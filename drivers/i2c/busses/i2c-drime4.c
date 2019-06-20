/* linux/drivers/i2c/busses/i2c-drime4.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 I2C Controller
 *
 * This code is based on linux/drivers/i2c/busses/i2c-s3c2410.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/pinctrl/consumer.h>

#include <mach/i2c.h>
#include <mach/map.h>
/* Register definitions */
#define DRIME4_I2CCON	(0x00)
#define DRIME4_I2CSTAT	(0x04)
#define DRIME4_I2CADD	(0x08)
#define DRIME4_I2CDS	(0x0C)
#define DRIME4_I2CSTOP	(0x10)

#define DRIME4_I2CCON_IRQCLEAR		(1<<8)
#define DRIME4_I2CCON_ACKEN		(1<<7)
#define DRIME4_I2CCON_TXDIV_16		(0<<6)
#define DRIME4_I2CCON_TXDIV_256		(1<<6)
#define DRIME4_I2CCON_IRQEN		(1<<5)
#define DRIME4_I2CCON_IRQDIS		(0<<5)
#define DRIME4_I2CCON_IRQPEND		(1<<4)	/* Only for READ */
#define DRIME4_I2CCON_SCALE(x)		((x)&15)
#define DRIME4_I2CCON_SCALEMASK		(0xf)

#define DRIME4_I2CSTAT_MASTER_RX	(2<<6)
#define DRIME4_I2CSTAT_MASTER_TX	(3<<6)
#define DRIME4_I2CSTAT_SLAVE_RX		(0<<6)
#define DRIME4_I2CSTAT_SLAVE_TX		(1<<6)
#define DRIME4_I2CSTAT_MODEMASK		(3<<6)

#define DRIME4_I2CSTAT_START		(1<<5)
#define DRIME4_I2CSTAT_BUSBUSY		(1<<5)
#define DRIME4_I2CSTAT_TXRXEN		(1<<4)
#define DRIME4_I2CSTAT_ARBITR		(1<<3)
#define DRIME4_I2CSTAT_ASSLAVE		(1<<2)
#define DRIME4_I2CSTAT_ADDR0		(1<<1)
#define DRIME4_I2CSTAT_LASTBIT		(1<<0)

#define DRIME4_I2CLC_SDA_DELAY0		(0<<0)
#define DRIME4_I2CLC_SDA_DELAY5		(1<<0)
#define DRIME4_I2CLC_SDA_DELAY10	(2<<0)
#define DRIME4_I2CLC_SDA_DELAY15	(3<<0)
#define DRIME4_I2CLC_SDA_DELAY_MASK	(3<<0)

#define DRIME4_I2CSTOP_MASTER_DATA_SFT	(1<<2)
#define DRIME4_I2CSTOP_MASTER_DATA_REL	(1<<1)
#define DRIME4_I2CSTOP_MASTER_CLK_REL	(1<<0)

/* i2c controller state */

enum drime4_i2c_state {
	STATE_IDLE,
	STATE_START,
	STATE_READ,
	STATE_WRITE,
	STATE_STOP
};

struct drime4_i2c {
	spinlock_t		lock;
	wait_queue_head_t	wait;
	unsigned int		suspended:1;

	struct i2c_msg		*msg;
	unsigned int		msg_num;
	unsigned int		msg_idx;
	unsigned int		msg_ptr;

	unsigned int		tx_setup;
	unsigned int		irq;

	enum drime4_i2c_state	state;
	unsigned long		clkrate;

	void __iomem		*regs;
	struct clk		*clk;
	struct device		*dev;
	struct resource		*ioarea;
	struct i2c_adapter	adap;

	struct pinctrl		*pinctrl;
	struct pinctrl_state	*pinstate;

#ifdef CONFIG_CPU_FREQ
	struct notifier_block	freq_transition;
#endif
};

/* drime4_i2c_master_complete
 *
 * complete the message and wake up the caller, using the given return code,
 * or zero to mean ok.
*/

static inline void drime4_i2c_master_complete(struct drime4_i2c *i2c, int ret)
{
	dev_dbg(i2c->dev, "master_complete %d\n", ret);

	i2c->msg_ptr = 0;
	i2c->msg = NULL;
	i2c->msg_idx++;
	i2c->msg_num = 0;
	if (ret)
		i2c->msg_idx = ret;

	wake_up(&i2c->wait);
}

static inline void drime4_i2c_disable_ack(struct drime4_i2c *i2c)
{
	unsigned long tmp;

	tmp = readl(i2c->regs + DRIME4_I2CCON);
	writel(tmp & ~DRIME4_I2CCON_ACKEN, i2c->regs + DRIME4_I2CCON);
}

static inline void drime4_i2c_enable_ack(struct drime4_i2c *i2c)
{
	unsigned long tmp;

	tmp = readl(i2c->regs + DRIME4_I2CCON);
	writel(tmp | DRIME4_I2CCON_ACKEN, i2c->regs + DRIME4_I2CCON);
}

/* irq clear/enable/disable functions */
static inline void drime4_i2c_clear_irq(struct drime4_i2c *i2c)
{
	unsigned long tmp;

	tmp = readl(i2c->regs + DRIME4_I2CCON);
	writel(tmp | DRIME4_I2CCON_IRQCLEAR, i2c->regs + DRIME4_I2CCON);
}

static inline void drime4_i2c_disable_irq(struct drime4_i2c *i2c)
{
	unsigned long tmp;

	tmp = readl(i2c->regs + DRIME4_I2CCON);
	writel(tmp & ~DRIME4_I2CCON_IRQEN, i2c->regs + DRIME4_I2CCON);
}

static inline void drime4_i2c_enable_irq(struct drime4_i2c *i2c)
{
	unsigned long tmp;

	tmp = readl(i2c->regs + DRIME4_I2CCON);
	writel(tmp | DRIME4_I2CCON_IRQEN, i2c->regs + DRIME4_I2CCON);
}

static inline void drime4_i2c_pendclear_irq(struct drime4_i2c *i2c)
{
	unsigned long tmp;

	tmp = readl(i2c->regs + DRIME4_I2CCON);
	tmp &= ~DRIME4_I2CCON_IRQPEND;
	writel(tmp, i2c->regs + DRIME4_I2CCON);
}

/* drime4_i2c_message_start
 *
 * put the start of a message onto the bus
*/

static void drime4_i2c_message_start(struct drime4_i2c *i2c,
				      struct i2c_msg *msg)
{
	unsigned int addr = (msg->addr & 0x7f) << 1;
	unsigned long stat;
	unsigned long i2ccon;

	stat = 0;
	stat |=  DRIME4_I2CSTAT_TXRXEN;

	if (msg->flags & I2C_M_RD) {
		stat |= DRIME4_I2CSTAT_MASTER_RX;
		addr |= 1;
	} else
		stat |= DRIME4_I2CSTAT_MASTER_TX;

	if (msg->flags & I2C_M_REV_DIR_ADDR)
		addr ^= 1;

	/* todo - check for wether ack wanted or not */
	drime4_i2c_enable_ack(i2c);

	i2ccon = readl(i2c->regs + DRIME4_I2CCON);
	writel(stat, i2c->regs + DRIME4_I2CSTAT);

	dev_dbg(i2c->dev, "START: %08lx to I2CSTAT, %02x to DS\n", stat, addr);
	writeb(addr, i2c->regs + DRIME4_I2CDS);

	/* delay here to ensure the data byte has gotten onto the bus
	 * before the transaction is started */

	ndelay(i2c->tx_setup);

	dev_dbg(i2c->dev, "i2ccon, %08lx\n", i2ccon);
	writel(i2ccon, i2c->regs + DRIME4_I2CCON);

	stat |= DRIME4_I2CSTAT_START;
	writel(stat, i2c->regs + DRIME4_I2CSTAT);
}

static inline void drime4_i2c_stop(struct drime4_i2c *i2c, int ret)
{
	unsigned long i2cstat = readl(i2c->regs + DRIME4_I2CSTAT);

	dev_dbg(i2c->dev, "STOP\n");

	writel(0x1, i2c->regs + DRIME4_I2CSTOP);

	drime4_i2c_pendclear_irq(i2c);
	/* TODO: Recieve interrupt clear information from Slave */

	/* stop the transfer */
	i2cstat = readl(i2c->regs + DRIME4_I2CSTAT);
	i2cstat &= ~(DRIME4_I2CSTAT_START | DRIME4_I2CSTAT_TXRXEN);
	writel(i2cstat, i2c->regs + DRIME4_I2CSTAT);

	i2c->state = STATE_STOP;

	drime4_i2c_master_complete(i2c, ret);
	drime4_i2c_disable_irq(i2c);
}

/* helper functions to determine the current state in the set of
 * messages we are sending */

/* is_lastmsg()
 *
 * returns TRUE if the current message is the last in the set
*/

static inline int is_lastmsg(struct drime4_i2c *i2c)
{
	return i2c->msg_idx >= (i2c->msg_num - 1);
}

/* is_msglast
 *
 * returns TRUE if we this is the last byte in the current message
*/

static inline int is_msglast(struct drime4_i2c *i2c)
{
	return i2c->msg_ptr == i2c->msg->len-1;
}

/* is_msgend
 *
 * returns TRUE if we reached the end of the current message
*/

static inline int is_msgend(struct drime4_i2c *i2c)
{
	return i2c->msg_ptr >= i2c->msg->len;
}

/* i2c_drime4_irq_nextbyte
 *
 * process an interrupt and work out what to do
 */

static int i2c_drime4_irq_nextbyte(struct drime4_i2c *i2c,
	unsigned long i2cstat)
{
	unsigned char byte;
	int ret = 0;

	switch (i2c->state) {

	case STATE_IDLE:
		dev_err(i2c->dev, "%s: called in STATE_IDLE\n", __func__);
		goto out;
		break;

	case STATE_STOP:
		dev_err(i2c->dev, "%s: called in STATE_STOP\n", __func__);
		drime4_i2c_disable_irq(i2c);
		goto out_ack;

	case STATE_START:
		/* last thing we did was send a start condition on the
		 * bus, or started a new i2c message
		 */

		if (i2cstat & DRIME4_I2CSTAT_LASTBIT &&
		    !(i2c->msg->flags & I2C_M_IGNORE_NAK)) {
			/* ack was not received... */

			dev_dbg(i2c->dev, "ack was not received\n");
			drime4_i2c_stop(i2c, -ENXIO);
			goto out_ack;
		}

		if (i2c->msg->flags & I2C_M_RD)
			i2c->state = STATE_READ;
		else
			i2c->state = STATE_WRITE;

		/* terminate the transfer if there is nothing to do
		 * as this is used by the i2c probe to find devices. */

		if (is_lastmsg(i2c) && i2c->msg->len == 0) {
			drime4_i2c_stop(i2c, 0);
			goto out_ack;
		}

		if (i2c->state == STATE_READ)
			goto prepare_read;

		/* fall through to the write state, as we will need to
		 * send a byte as well */

	case STATE_WRITE:
		/* we are writing data to the device... check for the
		 * end of the message, and if so, work out what to do
		 */

		if (!(i2c->msg->flags & I2C_M_IGNORE_NAK)) {
			if (i2cstat & DRIME4_I2CSTAT_LASTBIT) {
				dev_dbg(i2c->dev, "WRITE: No Ack\n");

				drime4_i2c_stop(i2c, -ECONNREFUSED);
				goto out_ack;
			}
		}

 retry_write:

		if (!is_msgend(i2c)) {
			byte = i2c->msg->buf[i2c->msg_ptr++];
			writeb(byte, i2c->regs + DRIME4_I2CDS);

			/* delay after writing the byte to allow the
			 * data setup time on the bus, as writing the
			 * data to the register causes the first bit
			 * to appear on SDA, and SCL will change as
			 * soon as the interrupt is acknowledged */

			ndelay(i2c->tx_setup);

		} else if (!is_lastmsg(i2c)) {
			/* we need to go to the next i2c message */

			dev_dbg(i2c->dev, "WRITE: Next Message\n");

			i2c->msg_ptr = 0;
			i2c->msg_idx++;
			i2c->msg++;

			/* check to see if we need to do another message */
			if (i2c->msg->flags & I2C_M_NOSTART) {

				if (i2c->msg->flags & I2C_M_RD) {
					/* cannot do this, the controller
					 * forces us to send a new START
					 * when we change direction */

					drime4_i2c_stop(i2c, -EINVAL);
				}

				goto retry_write;
			} else {
				/* send the new start */
				drime4_i2c_message_start(i2c, i2c->msg);
				i2c->state = STATE_START;
			}

		} else {
			/* send stop */

			drime4_i2c_stop(i2c, 0);
		}
		break;

	case STATE_READ:
		/* we have a byte of data in the data register, do
		 * something with it, and then work out wether we are
		 * going to do any more read/write
		 */

		byte = readb(i2c->regs + DRIME4_I2CDS);
		i2c->msg->buf[i2c->msg_ptr++] = byte;

 prepare_read:
		if (is_msglast(i2c)) {
			/* last byte of buffer */

			if (is_lastmsg(i2c)) {
				writeb(0x04, i2c->regs + DRIME4_I2CSTOP);
				drime4_i2c_disable_ack(i2c);
			}

		} else if (is_msgend(i2c)) {
			/* ok, we've read the entire buffer, see if there
			 * is anything else we need to do */

			if (is_lastmsg(i2c)) {
				/* last message, send stop and complete */
				dev_dbg(i2c->dev, "READ: Send Stop\n");

				drime4_i2c_stop(i2c, 0);
			} else {
				/* go to the next transfer */
				dev_dbg(i2c->dev, "READ: Next Transfer\n");

				i2c->msg_ptr = 0;
				i2c->msg_idx++;
				i2c->msg++;
			}
		}

		break;
	}

	/* acknowlegde the IRQ and get back on with the work */

 out_ack:
	drime4_i2c_pendclear_irq(i2c);
 out:
	return ret;
}

/* drime4_i2c_irq
 *
 * top level IRQ servicing routine
*/

static irqreturn_t drime4_i2c_irq(int irqno, void *dev_id)
{
	struct drime4_i2c *i2c = dev_id;
	unsigned long status;
	unsigned long tmp;

	drime4_i2c_clear_irq(i2c);

	status = readl(i2c->regs + DRIME4_I2CSTAT);

	if (status & DRIME4_I2CSTAT_ARBITR) {
		/* deal with arbitration loss */
		dev_err(i2c->dev, "deal with arbitration loss\n");
	}

	if (i2c->state == STATE_IDLE) {
		dev_dbg(i2c->dev, "IRQ: error i2c->state == IDLE\n");

		tmp = readl(i2c->regs + DRIME4_I2CCON);
		tmp &= ~DRIME4_I2CCON_IRQPEND;
		writel(tmp, i2c->regs + DRIME4_I2CCON);
		goto out;
	}

	/* pretty much this leaves us with the fact that we've
	 * transmitted or received whatever byte we last sent */

	i2c_drime4_irq_nextbyte(i2c, status);

 out:
	return IRQ_HANDLED;
}


/* drime4_i2c_set_master
 *
 * get the i2c bus for a master transaction
*/

static int drime4_i2c_set_master(struct drime4_i2c *i2c)
{
	unsigned long i2cstat;
	int timeout = 400;

	while (timeout-- > 0) {
		i2cstat = readl(i2c->regs + DRIME4_I2CSTAT);

		if (!(i2cstat & DRIME4_I2CSTAT_BUSBUSY))
			return 0;

		usleep_range(1000, 1000);
	}

	return -ETIMEDOUT;
}

/* drime4_i2c_doxfer
 *
 * this starts an i2c transfer
*/

static int drime4_i2c_doxfer(struct drime4_i2c *i2c,
			      struct i2c_msg *msgs, int num)
{
	unsigned long i2cstat, timeout;
	int spins = 20;
	int ret;

	if (i2c->suspended)
		return -EIO;

	ret = drime4_i2c_set_master(i2c);
	if (ret != 0) {
		dev_err(i2c->dev, "cannot get bus (error %d)\n", ret);
		ret = -EAGAIN;
		goto out;
	}

	spin_lock_irq(&i2c->lock);

	i2c->msg     = msgs;
	i2c->msg_num = num;
	i2c->msg_ptr = 0;
	i2c->msg_idx = 0;
	i2c->state   = STATE_START;

	drime4_i2c_enable_irq(i2c);
	drime4_i2c_message_start(i2c, msgs);
	spin_unlock_irq(&i2c->lock);

	timeout = wait_event_timeout(i2c->wait, i2c->msg_num == 0, HZ * 5);

	ret = i2c->msg_idx;

	/* having these next two as dev_err() makes life very
	 * noisy when doing an i2cdetect */

	if (timeout == 0)
		dev_dbg(i2c->dev, "timeout\n");
	else if (ret != num)
		dev_dbg(i2c->dev, "incomplete xfer (%d)\n", ret);

	/* ensure the stop has been through the bus */

	dev_dbg(i2c->dev, "waiting for bus idle\n");

	/* first, try busy waiting briefly */
	do {
		i2cstat = readl(i2c->regs + DRIME4_I2CSTAT);
	} while ((i2cstat & DRIME4_I2CSTAT_START) && --spins);

	/* if that timed out sleep */
	if (!spins) {
		usleep_range(1000, 1000);
		i2cstat = readl(i2c->regs + DRIME4_I2CSTAT);
	}

	if (i2cstat & DRIME4_I2CSTAT_START)
		dev_warn(i2c->dev, "timeout waiting for bus idle\n");

 out:
	return ret;
}

/* drime4_i2c_xfer
 *
 * first port of call from the i2c bus code when an message needs
 * transferring across the i2c bus.
*/

static int drime4_i2c_xfer(struct i2c_adapter *adap,
			struct i2c_msg *msgs, int num)
{
	struct drime4_i2c *i2c = (struct drime4_i2c *)adap->algo_data;
	int retry;
	int ret;

	clk_enable(i2c->clk);

	for (retry = 0; retry < adap->retries; retry++) {

		ret = drime4_i2c_doxfer(i2c, msgs, num);

		if (ret != -EAGAIN) {
			clk_disable(i2c->clk);
			return ret;
		}

		dev_dbg(i2c->dev, "Retrying transmission (%d)\n", retry);

		udelay(100);
	}

	clk_disable(i2c->clk);
	return -EREMOTEIO;
}

/* declare our i2c functionality */
static u32 drime4_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}

/* i2c bus registration info */

static const struct i2c_algorithm drime4_i2c_algorithm = {
	.master_xfer		= drime4_i2c_xfer,
	.functionality		= drime4_i2c_func,
};

/* drime4_i2c_calcdivisor
 *
 * return the divisor settings for a given frequency
*/

static int drime4_i2c_calcdivisor(unsigned long clkin, unsigned int wanted,
				   unsigned int *div1, unsigned int *divs)
{
	unsigned int calc_divs = clkin / wanted;
	unsigned int calc_div1;

	if (calc_divs / 16 > 16)
		calc_div1 = 256;
	else
		calc_div1 = 16;

	calc_divs += calc_div1 - 1;
	calc_divs /= calc_div1;

	if(calc_divs == 0)
		calc_divs = 1;
	if(calc_divs > 16)
		calc_divs = 16;

	*divs = calc_divs;
	*div1 = calc_div1;

	return clkin /(calc_divs * calc_div1);
}

/* drime4_i2c_clockrate
 *
 * work out a divisor for the user requested frequency setting,
 * either by the requested frequency, or scanning the acceptable
 * range of frequencies until something is found
*/

static int drime4_i2c_clockrate(struct drime4_i2c *i2c, unsigned int *got)
{
	struct drime4_platform_i2c *pdata = i2c->dev->platform_data;
	/* FIXME: drime4 i2c clock */
	unsigned long clkin = clk_get_rate(i2c->clk);
	unsigned int divs, div1;
	unsigned long target_frequency;
	u32 i2ccon;
	int freq;

	i2c->clkrate = clkin;
	clkin /= 1000;		/* clkin now in KHz */

	dev_dbg(i2c->dev, "pdata desired frequency %lu\n", pdata->frequency);

	target_frequency = pdata->frequency ? pdata->frequency : 100000;

	target_frequency /= 1000; /* Target frequency now in KHz */

	freq = drime4_i2c_calcdivisor(clkin, target_frequency, &div1, &divs);

	if (freq > target_frequency) {
		dev_err(i2c->dev,
			"Unable to achieve desired frequency %luKHz."	\
			" Lowest achievable %dKHz\n", target_frequency, freq);
		return -EINVAL;
	}

	*got = freq;

	i2ccon = readl(i2c->regs + DRIME4_I2CCON);
	i2ccon &= ~(DRIME4_I2CCON_SCALEMASK | DRIME4_I2CCON_TXDIV_256);
	i2ccon |= (divs - 1);

	if (div1 == 256)
		i2ccon |= DRIME4_I2CCON_TXDIV_256;

	writel(i2ccon, i2c->regs + DRIME4_I2CCON);
/* FIXME: Should drime4 execute this code? */
#if 0
	if (drime4_i2c_is2440(i2c)) {
		unsigned long sda_delay;

		if (pdata->sda_delay) {
			sda_delay = clkin * pdata->sda_delay;
			sda_delay = DIV_ROUND_UP(sda_delay, 1000000);
			sda_delay = DIV_ROUND_UP(sda_delay, 5);
			if (sda_delay > 3)
				sda_delay = 3;
			sda_delay |= S3C2410_IICLC_FILTER_ON;
		} else
			sda_delay = 0;

		dev_dbg(i2c->dev, "IICLC=%08lx\n", sda_delay);
		writel(sda_delay, i2c->regs + S3C2440_IICLC);
	}
#endif

	return 0;
}

#ifdef CONFIG_CPU_FREQ

#define freq_to_i2c(_n) container_of(_n, struct drime4_i2c, freq_transition)

static int drime4_i2c_cpufreq_transition(struct notifier_block *nb,
					  unsigned long val, void *data)
{
	struct drime4_i2c *i2c = freq_to_i2c(nb);
	unsigned long flags;
	unsigned int got;
	int delta_f;
	int ret;

	delta_f = clk_get_rate(i2c->clk) - i2c->clkrate;

	/* if we're post-change and the input clock has slowed down
	 * or at pre-change and the clock is about to speed up, then
	 * adjust our clock rate. <0 is slow, >0 speedup.
	 */

	if ((val == CPUFREQ_POSTCHANGE && delta_f < 0) ||
	    (val == CPUFREQ_PRECHANGE && delta_f > 0)) {
		spin_lock_irqsave(&i2c->lock, flags);
		ret = drime4_i2c_clockrate(i2c, &got);
		spin_unlock_irqrestore(&i2c->lock, flags);

		if (ret < 0)
			dev_err(i2c->dev, "cannot find frequency\n");
		else
			dev_info(i2c->dev, "setting freq %d\n", got);
	}

	return 0;
}

static inline int drime4_i2c_register_cpufreq(struct drime4_i2c *i2c)
{
	i2c->freq_transition.notifier_call = drime4_i2c_cpufreq_transition;

	return cpufreq_register_notifier(&i2c->freq_transition,
					 CPUFREQ_TRANSITION_NOTIFIER);
}

static inline void drime4_i2c_deregister_cpufreq(struct drime4_i2c *i2c)
{
	cpufreq_unregister_notifier(&i2c->freq_transition,
				    CPUFREQ_TRANSITION_NOTIFIER);
}

#else
static inline int drime4_i2c_register_cpufreq(struct drime4_i2c *i2c)
{
	return 0;
}

static inline void drime4_i2c_deregister_cpufreq(struct drime4_i2c *i2c)
{
}
#endif

/* drime4_i2c_init
 *
 * initialise the controller, set the IO lines and frequency
*/

static int drime4_i2c_init(struct drime4_i2c *i2c)
{
	unsigned long i2con = DRIME4_I2CCON_IRQEN | DRIME4_I2CCON_ACKEN;
	struct drime4_platform_i2c *pdata;
	unsigned int freq;

	/* get the plafrom data */

	pdata = i2c->dev->platform_data;

	/* inititalise the gpio */

	if (pdata->cfg_gpio)
		pdata->cfg_gpio(to_platform_device(i2c->dev));

	/* write slave address */

	writeb(pdata->slave_addr, i2c->regs + DRIME4_I2CADD);

	dev_info(i2c->dev, "slave address 0x%02x\n", pdata->slave_addr);

	writel(i2con, i2c->regs + DRIME4_I2CCON);

	/* we need to work out the divisors for the clock... */

	if (drime4_i2c_clockrate(i2c, &freq) != 0) {
		writel(0, i2c->regs + DRIME4_I2CCON);
		dev_err(i2c->dev, "cannot meet bus frequency required\n");
		return -EINVAL;
	}

	/* todo - check that the i2c lines aren't being dragged anywhere */

	dev_info(i2c->dev, "bus frequency set to %d KHz\n", freq);
	dev_dbg(i2c->dev, "DRIME4_I2CCON=0x%02lx\n", i2con);

	return 0;
}

/* drime4_i2c_probe
 *
 * called by the bus driver when a suitable device is found
*/

static int drime4_i2c_probe(struct platform_device *pdev)
{
	struct drime4_i2c *i2c;
	struct drime4_platform_i2c *pdata;
	struct resource *res;
	unsigned int pad_val;
	int ret;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data\n");
		return -EINVAL;
	}

	i2c = devm_kzalloc(&pdev->dev, sizeof(struct drime4_i2c), GFP_KERNEL);
	if (!i2c) {
		dev_err(&pdev->dev, "no memory for state\n");
		return -ENOMEM;
	}

	strlcpy(i2c->adap.name, "drime4-i2c", sizeof(i2c->adap.name));
	i2c->adap.owner   = THIS_MODULE;
	i2c->adap.algo    = &drime4_i2c_algorithm;
	i2c->adap.retries = 2;
	i2c->adap.class   = I2C_CLASS_HWMON | I2C_CLASS_SPD;
	i2c->tx_setup     = 50;

	spin_lock_init(&i2c->lock);
	init_waitqueue_head(&i2c->wait);

	/* map the registers */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "cannot find IO resource\n");
		ret = -ENOENT;
		goto err_free;
	}

	i2c->ioarea = devm_request_mem_region(&pdev->dev, res->start,
					resource_size(res), pdev->name);

	if (i2c->ioarea == NULL) {
		dev_err(&pdev->dev, "cannot request IO\n");
		ret = -ENXIO;
		goto err_free;
	}

	i2c->regs = devm_ioremap(&pdev->dev, res->start, resource_size(res));

	if (i2c->regs == NULL) {
		dev_err(&pdev->dev, "cannot map IO\n");
		ret = -ENXIO;
		goto err_free;
	}

	dev_dbg(&pdev->dev, "registers %p (%p, %p)\n",
		i2c->regs, i2c->ioarea, res);

	/* setup info block for the i2c core */

	i2c->adap.algo_data = i2c;
	i2c->adap.dev.parent = &pdev->dev;

	/* find the clock and enable it */

	i2c->dev = &pdev->dev;
	i2c->clk = clk_get(&pdev->dev, "i2c");
	if (IS_ERR(i2c->clk)) {
		dev_err(&pdev->dev, "cannot get clock\n");
		ret = -ENOENT;
		goto err_free;
	}

	dev_dbg(&pdev->dev, "clock source %p\n", i2c->clk);

	clk_enable(i2c->clk);

	pad_val = __raw_readl(DRIME4_VA_PLATFORM_CTRL + 0x420);

	pad_val &= ~(1 << pdev->id);

	__raw_writel(pad_val, DRIME4_VA_PLATFORM_CTRL + 0x420);

	/* setup pad configuration */
	i2c->pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(i2c->pinctrl))
		dev_dbg(&pdev->dev, "cannot find pinmux configuration\n");
	else {
		i2c->pinstate = pinctrl_lookup_state(i2c->pinctrl,
			PINCTRL_STATE_DEFAULT);

		ret = pinctrl_select_state(i2c->pinctrl, i2c->pinstate);
		if (ret) {
			dev_err(&pdev->dev, "cannot select pins state\n");
			goto err_clk;
		}
	}

	/* initialise the i2c controller */

	ret = drime4_i2c_init(i2c);
	if (ret != 0)
		goto err_clk;

	/* find the IRQ for this unit (note, this relies on the init call to
	 * ensure no current IRQs pending
	 */

	i2c->irq = ret = platform_get_irq(pdev, 0);
	if (ret <= 0) {
		dev_err(&pdev->dev, "cannot find IRQ\n");
		goto err_clk;
	}

	ret = devm_request_irq(&pdev->dev, i2c->irq, drime4_i2c_irq,
				IRQF_DISABLED, dev_name(&pdev->dev), i2c);

	if (ret != 0) {
		dev_err(&pdev->dev, "cannot claim IRQ %d\n", i2c->irq);
		goto err_clk;
	}

	ret = drime4_i2c_register_cpufreq(i2c);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register cpufreq notifier\n");
		goto err_clk;
	}

	/* Note, previous versions of the driver used i2c_add_adapter()
	 * to add the bus at any number. We now pass the bus number via
	 * the platform data, so if unset it will now default to always
	 * being bus 0.
	 */

	i2c->adap.nr = pdata->bus_num;

	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to add bus to i2c core\n");
		goto err_cpufreq;
	}

	platform_set_drvdata(pdev, i2c);

	dev_info(&pdev->dev, "%s: DRIME4 I2C adapter\n",
		dev_name(&i2c->adap.dev));
	/*
	clk_disable(i2c->clk);
	*/
	return 0;

err_cpufreq:
	drime4_i2c_deregister_cpufreq(i2c);

err_clk:
	clk_disable(i2c->clk);
	clk_put(i2c->clk);


err_free:
	devm_kfree(&pdev->dev, i2c);

	return ret;
}

/* drime4_i2c_remove
 *
 * called when device is removed from the bus
*/

static int drime4_i2c_remove(struct platform_device *pdev)
{
	struct drime4_i2c *i2c = platform_get_drvdata(pdev);

	drime4_i2c_deregister_cpufreq(i2c);

	i2c_del_adapter(&i2c->adap);

	clk_disable(i2c->clk);
	clk_put(i2c->clk);

	return 0;
}

static int drime4_i2c_suspend(struct platform_device *pdev)
{
	struct drime4_i2c *i2c = platform_get_drvdata(pdev);

	if (i2c == NULL)
		return -1;

	i2c->suspended = 1;
	return 0;
}


static int drime4_i2c_resume(struct platform_device *pdev)
{
	struct drime4_i2c *i2c = platform_get_drvdata(pdev);

	if (i2c == NULL)
		return -1;

	i2c->suspended = 0;
	drime4_i2c_init(i2c);
	return 0;
}

#if 0
#ifdef CONFIG_PM
static int drime4_i2c_resume(struct device *dev)
{
	unsigned int pad_val;
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_i2c *i2c = platform_get_drvdata(pdev);

	if (i2c == NULL)
		return -1;

	i2c->suspended = 0;
//	clk_enable(i2c->clk);
	drime4_i2c_init(i2c);
//	clk_disable(i2c->clk);

	return 0;
}

static int drime4_i2c_suspend_noirq(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_i2c *i2c = platform_get_drvdata(pdev);

	if (i2c == NULL)
		return -1;

	i2c->suspended = 1;
	return 0;
}
static const struct dev_pm_ops drime4_i2c_dev_pm_ops = {
	.suspend_noirq = drime4_i2c_suspend_noirq,
	.resume = drime4_i2c_resume,
};

#define DRIME4_DEV_PM_OPS (&drime4_i2c_dev_pm_ops)
#else
#define DRIME4_DEV_PM_OPS NULL
#endif
#endif

/* device driver for platform bus bits */

static struct platform_device_id drime4_driver_ids[] = {
	{
		.name		= "drime4-i2c",
	},
};
MODULE_DEVICE_TABLE(platform, drime4_driver_ids);

static struct platform_driver drime4_i2c_driver = {
	.probe		= drime4_i2c_probe,
	.remove		= drime4_i2c_remove,
	.suspend	= drime4_i2c_suspend,
	.resume 	= drime4_i2c_resume,
	.id_table	= drime4_driver_ids,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "d4-i2c",
//		.pm	= DRIME4_DEV_PM_OPS,
	},
};

static int __init i2c_adap_drime4_init(void)
{
	return platform_driver_register(&drime4_i2c_driver);
}
#ifndef CONFIG_SCORE_FAST_RESUME
subsys_initcall(i2c_adap_drime4_init);
#else
fast_subsys_initcall(i2c_adap_drime4_init);
#endif

static void __exit i2c_adap_drime4_exit(void)
{
	platform_driver_unregister(&drime4_i2c_driver);
}
module_exit(i2c_adap_drime4_exit);

MODULE_DESCRIPTION("DRIME4 I2C Bus driver");
MODULE_AUTHOR("Chanho Park, <chanho61.park@samsung.com>");
MODULE_LICENSE("GPL");
