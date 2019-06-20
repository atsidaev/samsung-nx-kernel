/* linux/drivers/spi/hs_spi.c
 *
 * Copyright (c) 2011 Samsung Electronics
 * Kyuchun han <kyuchun.han@samsung.com>
 *
 * DRIME4 SPI platform device driver using SPI framework
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <mach/d4_spi_regs.h>
#include <linux/pinctrl/pinconf-drime4.h>
#include <linux/pinctrl/consumer.h>
#include <mach/dma.h>
#include <linux/hs_spi.h>
#include <linux/spi/d4_spi_config.h>

#include <mach/map.h>

#include <linux/d4-pmu.h>

#ifdef CONFIG_DMA_ENGINE
#include <linux/dmaengine.h>
#include <linux/amba/pl330.h>
#endif

static DEFINE_MUTEX(spi_lock);
static LIST_HEAD(spi_list);

#define SUSPND	  (1<<0)
#define SPIBUSY   (1<<1)
#define RXBUSY	  (1<<2)
#define TXBUSY	  (1<<3)

#define FIFO_DEPTH			64
#define FIFO_THRESHOLD		32

enum spi_clk_select {
	CLKSEL81 = 0x1, CLKSEL100 = 0x2, CLKSEL27 = 0x3,
};

enum spi_rw_type {
	WRITE_TYPE, READ_TYPE, WRITE_READ_TYPE, UNKNOWN_TYPE
};

#define SPI_REG_DISABLE     0
#define SPI_REG_ENABLE      1
#define SPI_PENDING_ALL_CLEAR   0xFF

#define msecs_to_loops(t) (loops_per_jiffy / 1000 * HZ * t)

/**
 * struct hs_spi_data
 * @list: spi device list.
 * @reg: Poniter to the spi device register address.
 * @clk: Poniter to the spi device clock.
 * @pmx: Pointer to the spi pad selection.
 * @irq: variable for spi irq number.
 * @done: variable for spi irq completion.
 * @pdev: Pointer to spi platform device.
 * @spi_id: device id number.
 * @lock: Controller specific lock.
 * @rx_dmach: Controller's DMA channel for Rx.
 * @tx_dmach: Controller's DMA channel for Tx.
 * @sfr_start: BUS address of SPI controller regs.
 * @state: Set of FLAGS to indicate status.
 * @tx: tx buffer start address.
 * @tx_end: tx buffer end address.
 * @rwtype: communication type.
 * @read_len: variable for number of read bytes.
 * @wrtie_len: variable for number of write bytes.
 * @n_bytes: variable for number of total bytes.
 * @cur_bpw: Stores the active bits per word settings.
 * @int_handler: irq handler.
 * @waittime: waiting time for spi working.
 * @rx_chan: Pointer to read dma channel.
 * @tx_chan: Pointer to write dma channel.
 * @tx_dma: write dma channel.
 * @rx_dma: read dma channel.
  */
struct hs_spi_data {
	struct list_head list;
	void __iomem *regs;
	struct clk *clk;
	struct pinctrl *pmx;
	struct pinctrl_state	*pins_default;
	int irq;
	struct completion done;
	struct platform_device *pdev;
	unsigned char spi_id;
	/*	struct spi_device *tgl_spi; */

	spinlock_t lock;
	enum dma_ch rx_dmach;
	enum dma_ch tx_dmach;
	unsigned long sfr_start;
	unsigned int sfr_size;
	unsigned int state;
	void *tx;
	void *tx_end;
	void *rx;
	void *rx_end;
	/*	unsigned int temp_len;*/
	enum spi_rw_type rwtype;
	/* Read The Number of Bytes */
	unsigned int read_len;
	unsigned int read_len_temp;
	unsigned int temp_read;
	/*Write The Number of Bytes */
	unsigned int write_len;
	/*The Number of Bytes */
	unsigned int n_bytes;
	/*	unsigned cur_mode; */
	unsigned cur_bpw;
	unsigned set_bpw;
	/*	unsigned cur_speed; */
	void (*int_handler)(void *dev);
	unsigned int waittime;
	unsigned int burst_mode;
	unsigned int irq_cnt;
#ifdef CONFIG_DMA_ENGINE
struct dma_chan *rx_chan;
struct dma_chan *tx_chan;
dma_addr_t tx_dma;
dma_addr_t rx_dma;
#endif

};

#define SPI_MAX_CLK     50000000
#define SPI_MIN_CLK     13200


struct spi_clk_sel {
	u32 per;
	unsigned long clk_freq_mid;
	unsigned long clk_freq_max;
	unsigned long clk_freq_min;
};

struct spi_clk_sel spi_freq_table[] = {
		{ 0, 40500000, 50000000, 13500000 },
		{ 1, 20250000, 25000000, 6750000 },
		{ 2, 10125000, 12500000, 3375000 },
		{ 3, 5062500, 6250000, 1687500 },
		{ 4, 2531300, 3125000, 843800 },
		{ 5, 1265600, 1562500, 421900 },
		{ 6, 632800, 781300, 210900 },
		{ 7, 316400, 390600, 105500 },
		{ 8, 158200, 195300, 52700 },
		{ 9, 79100, 97700, 26400 },
		{ 10, 39600, 48800, 13200 },
		{ 0, 0, 0, 0 } };

struct port_select {
	unsigned char id;
	unsigned char port_num_clk;
	unsigned char port_num_di;
	unsigned char port_num_do;
	unsigned char port_num_cs;
};

struct port_select port_num[] = {
		{ 0, DRIME4_GPIO0(3), DRIME4_GPIO0(4), DRIME4_GPIO0(5), DRIME4_GPIO0(6) },
		{ 1, DRIME4_GPIO4(6), DRIME4_GPIO4(7), DRIME4_GPIO5(0), DRIME4_GPIO5(1) },
		{ 2, DRIME4_GPIO10(0), 	DRIME4_GPIO10(1), DRIME4_GPIO10(2), DRIME4_GPIO10(3) },
		{ 3, DRIME4_GPIO14(1), DRIME4_GPIO14(2), DRIME4_GPIO14(3), DRIME4_GPIO14(4) },
		{ 4, DRIME4_GPIO14(5), DRIME4_GPIO14(6), DRIME4_GPIO14(7), DRIME4_GPIO15(0) },
		{ 5, DRIME4_GPIO15(1), DRIME4_GPIO15(2), DRIME4_GPIO15(3), DRIME4_GPIO15(4) },
		{ 6, DRIME4_GPIO8(2), DRIME4_GPIO8(3), DRIME4_GPIO8(4), DRIME4_GPIO8(5) },
		{ 7, DRIME4_GPIO26(5), DRIME4_GPIO26(6), DRIME4_GPIO26(7), DRIME4_GPIO27(0) }, };

static void hs_spi_write_dummy_irq(void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;
	unsigned int remain_len, writen_len, val;
	unsigned int int_status, check_rfifo, check_wfifo;

	if (spi->write_len == spi->n_bytes) {
		writel(0x1, base + SPI_PENDING_CLR);
		return;
	}

	remain_len = spi->n_bytes - spi->write_len;

	val = readl(base + SPI_FIFO_CFG);
	val = SPI_GET_FIFO_CFG_TX_RDY_LVL(val);

	check_wfifo = readl(base + SPI_STATUS);
	int_status = SPI_STATUS_TX_FIFO_LVL(check_wfifo);
	check_rfifo = SPI_STATUS_RX_FIFO_LVL(check_wfifo);

	if (spi->write_len > spi->read_len)
		return;

	if (int_status < val) {
		if (remain_len > val)
			writen_len = val;
		else
			writen_len = remain_len;
	} else {
		writen_len = FIFO_DEPTH - int_status;
	}

	spi->write_len += writen_len;

	while (1) {
		if (writen_len) {
			switch (spi->cur_bpw) {
			case 8:
				__raw_writeb(0xFF, base + SPI_TX_DATA);
				writen_len -= 1;
				break;
			case 16:
				__raw_writew(0xFF, base + SPI_TX_DATA);
				writen_len -= 2;
				break;
			default:
				__raw_writel(0xFF, base + SPI_TX_DATA);
				writen_len -= 4;
				break;
			}
		} else
			break;
	}
}

static void hs_spi_read_dummy_irq(void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;
	unsigned int val;

	val = __raw_readl(base + SPI_FIFO_CFG);
	val = SPI_GET_FIFO_CFG_RX_RDY_LVL(val);
	val = __raw_readl(base + SPI_STATUS);

	if (SPI_STATUS_RX_FIFO_LVL(val)) {
		switch (spi->cur_bpw) {
		case 8:
			__raw_readb(base + SPI_RX_DATA);
			spi->read_len += 1;
			break;
		case 16:
			__raw_readw(base + SPI_RX_DATA);
			spi->read_len += 2;
			break;
		default:
			__raw_readl(base + SPI_RX_DATA);
			spi->read_len += 4;
			break;
		}

		return;
	}

	if ((SPI_STATUS_RX_FIFO_LVL(val) == 0) && (spi->read_len == spi->n_bytes)) {
		val = readl(base + SPI_PENDING_CLR);
		SPI_PEND_CLR_RX_TRIG_INT(val, SPI_REG_ENABLE);
		/*packet status clear*/
		SPI_PEND_CLR_TRAILING_CLR(val, SPI_REG_ENABLE);
		writel(val, base + SPI_PENDING_CLR);
	}
}


unsigned int temp_cnt = 0;
static void hs_spi_fw_read_irq(void *dev)
{
	unsigned int read_len, val;
	unsigned int int_status;
	unsigned int temp;
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;


	int_status = __raw_readl(base + SPI_STATUS);
	int_status = SPI_STATUS_RX_FIFO_LVL(int_status);

	read_len = int_status;
	if (read_len == 0) {

		if (spi->irq_cnt == 0) {
			if (spi->read_len < spi->n_bytes)  {
				gpio_direction_output(GPIO_AF_LED, 1);
				return;
			}
			val = __raw_readl(base + SPI_PENDING_CLR);
			SPI_PEND_CLR_RX_TRIG_INT(val, SPI_REG_ENABLE);
			/*packet status clear*/
			SPI_PEND_CLR_TRAILING_CLR(val, SPI_REG_ENABLE);
			__raw_writel(val, base + SPI_PENDING_CLR);
			printk(KERN_ALERT"=============handler fw_read done1=================: %d\n", spi->read_len);
			spi->read_len = spi->n_bytes;

			__raw_writel(0x0, base + SPI_CH_ON);
			__raw_writel(0x20, base + SPI_CH_CFG);
			__raw_writel(0x00, base + SPI_CH_CFG);
			gpio_direction_output(GPIO_AF_LED, 0);
			return;
		} else {
			spi->irq_cnt--;
		}
		return;
	} else {
		if (spi->read_len >= spi->n_bytes) {
			val = __raw_readl(base + SPI_PENDING_CLR);
			SPI_PEND_CLR_RX_TRIG_INT(val, SPI_REG_ENABLE);
			/*packet status clear*/
			SPI_PEND_CLR_TRAILING_CLR(val, SPI_REG_ENABLE);
			__raw_writel(val, base + SPI_PENDING_CLR);
			printk(KERN_ALERT"=============handler fw_read done2=================: %d\n", spi->read_len);
			spi->read_len = spi->n_bytes;
			__raw_writel(0x0, base + SPI_CH_ON);
			__raw_writel(0x20, base + SPI_CH_CFG);
			__raw_writel(0x00, base + SPI_CH_CFG);
			gpio_direction_output(GPIO_AF_LED, 0);
			return;
		}
	}

	spi->read_len += read_len;
	switch (spi->cur_bpw) {
	case 8:
		__raw_readsb(base + SPI_RX_DATA, spi->rx, read_len);
		break;
	case 16:
		__raw_readsw(base + SPI_RX_DATA, spi->rx, read_len / 2);
		break;
	default:
		__raw_readsl(base + SPI_RX_DATA, spi->rx, read_len / 4);
		break;
	}
	spi->rx += read_len;
	spi->irq_cnt = 100;
	gpio_direction_output(GPIO_AF_LED, 0);

}



static void hs_spi_read_irq(void *dev)
{
	unsigned int read_len, val;
	unsigned int int_status;

	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;

	if (spi->rx == spi->rx_end) {
		val = __raw_readl(base + SPI_PENDING_CLR);
		SPI_PEND_CLR_RX_TRIG_INT(val, SPI_REG_ENABLE);
		/*packet status clear*/
		SPI_PEND_CLR_TRAILING_CLR(val, SPI_REG_ENABLE);
		__raw_writel(val, base + SPI_PENDING_CLR);
		return;
	}

	int_status = __raw_readl(base + SPI_STATUS);
	int_status = SPI_STATUS_RX_FIFO_LVL(int_status);

	read_len = int_status;

	if (read_len == 0)
		return;

	spi->read_len += read_len;
	switch (spi->cur_bpw) {
	case 8:
		__raw_readsb(base + SPI_RX_DATA, spi->rx, read_len);
		break;
	case 16:
		__raw_readsw(base + SPI_RX_DATA, spi->rx, read_len / 2);
		break;
	default:
		__raw_readsl(base + SPI_RX_DATA, spi->rx, read_len / 4);
		break;
	}
	spi->rx += read_len;
}

static void hs_spi_write_irq(void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;
	unsigned int remain_len, writen_len, val;

	if (spi->tx == spi->tx_end) {
		__raw_writel(0x1, base + SPI_PENDING_CLR);
		return;
	}

	remain_len = spi->tx_end - spi->tx;

	val = __raw_readl(base + SPI_FIFO_CFG);
	val = SPI_GET_FIFO_CFG_TX_RDY_LVL(val);
	if (remain_len > val)
		writen_len = val;
	else
		writen_len = remain_len;

	switch (spi->cur_bpw) {
	case 8:
		__raw_writesb(base + SPI_TX_DATA, spi->tx, writen_len);
		break;
	case 16:
		__raw_writesw(base + SPI_TX_DATA, spi->tx, writen_len / 2);
		break;
	default:
		__raw_writesl(base + SPI_TX_DATA, spi->tx, writen_len / 4);
		break;
	}
	spi->tx += writen_len;
	spi->write_len += writen_len;
}

static void spi_read_top_irq(void *dev)
{
	struct hs_spi_data *spi = dev;

	if (spi->rwtype == WRITE_TYPE)
		hs_spi_read_dummy_irq(spi);
	else
		hs_spi_read_irq(spi);
}

static void spi_write_top_irq(void *dev)
{
	struct hs_spi_data *spi = dev;

	if (spi->rwtype == READ_TYPE)
		hs_spi_write_dummy_irq(spi);
	else
		hs_spi_write_irq(spi);
}


static void spi_readwrite_irq(void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;

	unsigned int val;
	unsigned int temp;

	val = __raw_readl(base + SPI_PENDING_CLR);

	temp = SPI_GET_PEND_CLR_RX_TRIG_INT(val);

	if (temp == 1)
		spi_read_top_irq(spi);

	temp = SPI_GET_PEND_CLR_TX_TRIG_INT(val);

	if (temp == 1)
		spi_write_top_irq(spi);

}


void hs_spi_slave_write_irq(void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;
	unsigned int remain_len, writen_len, val;

	if (spi->tx == spi->tx_end) {
		__raw_writel(0x1, base + SPI_PENDING_CLR);
		return;
	}

	remain_len = spi->tx_end - spi->tx;

/*

	val = __raw_readl(base + SPI_FIFO_CFG);
	val = SPI_GET_FIFO_CFG_TX_RDY_LVL(val);
	if (remain_len > val)
		writen_len = val;
	else
		writen_len = remain_len;

*/

	val = __raw_readl(base + SPI_STATUS);
	val = FIFO_DEPTH - SPI_STATUS_TX_FIFO_LVL(val);
	if (val == FIFO_DEPTH) {
		return;
	}
	if (remain_len > val)
		writen_len = val;
	else
		writen_len = remain_len;

	switch (spi->cur_bpw) {
	case 8:
		__raw_writesb(base + SPI_TX_DATA, spi->tx, writen_len);
		spi->read_len += writen_len;
		break;
	case 16:
		__raw_writesw(base + SPI_TX_DATA, spi->tx, writen_len / 2);
		spi->read_len += (writen_len / 2);
		break;
	default:
		__raw_writesl(base + SPI_TX_DATA, spi->tx, writen_len / 4);
		spi->read_len += (writen_len / 4);
		break;
	}
	spi->tx += writen_len;
}

void spi_slave_write_irq(void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;

	unsigned int val;
	unsigned int temp;

	val = __raw_readl(base + SPI_PENDING_CLR);

	temp = SPI_GET_PEND_CLR_TX_TRIG_INT(val);

	if (temp == 1)
		hs_spi_slave_write_irq(spi);

}

void hs_spi_int_done(struct hs_spi_data *spi)
{
	void __iomem *base = spi->regs;
	spi->n_bytes = 0;
	__raw_writel(0x20, base + SPI_CH_CFG);
	__raw_writel(0x10, base + SPI_CH_CFG);

	complete(&spi->done);
}

static void hs_spi_slave_read_irq(void *dev)
{
	unsigned int read_len, val, read_len_plus;
	unsigned int int_status;
	unsigned char first_data;
	unsigned int temp;
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;


	int_status = __raw_readl(base + SPI_STATUS);
	int_status = SPI_STATUS_RX_FIFO_LVL(int_status);

	read_len = int_status;

	if ((spi->n_bytes == spi->read_len_temp) && (spi->n_bytes != 0)) {
		__raw_readsb(base + SPI_RX_DATA, spi->rx, 1);
		val = __raw_readl(base + SPI_PENDING_CLR);
		SPI_PEND_CLR_RX_TRIG_INT(val, SPI_REG_ENABLE);
		/*packet status clear*/
		SPI_PEND_CLR_TRAILING_CLR(val, SPI_REG_ENABLE);
		__raw_writel(val, base + SPI_PENDING_CLR);
		/*spi->n_bytes = spi->read_len;*/
		spi->read_len = spi->read_len_temp;
		/*channel off*/
		__raw_writel(0x0, base + SPI_CH_ON);
		__raw_writel(0x20, base + SPI_CH_CFG);
		__raw_writel(0x00, base + SPI_CH_CFG);
/*		printk(KERN_ALERT"handler done\n");*/
		return;
	}

	if (read_len != 0) {
		switch (spi->cur_bpw) {
		case 8:
			if (spi->read_len_temp == 0) {
				__raw_readsb(base + SPI_RX_DATA, spi->rx, 2);
				temp = *((unsigned short*)(spi->rx));
				spi->n_bytes += temp;
				spi->read_len_temp = spi->temp_read;
				/*printk(KERN_ALERT"spi->read_len_temp:%d, %d\n", temp, spi->n_bytes);*/
				spi->read_len_temp += 2;
				read_len_plus = 2;
			} else {
				__raw_readsb(base + SPI_RX_DATA, spi->rx, read_len);
				spi->read_len_temp += read_len;
				read_len_plus = read_len;
			}
			break;
		case 16:
			__raw_readsw(base + SPI_RX_DATA, spi->rx, read_len / 2);
			break;
		default:
			__raw_readsl(base + SPI_RX_DATA, spi->rx, read_len / 4);
			break;
		}
		spi->rx += read_len_plus;

	}

}


static void spi_slave_read_irq(void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;

	unsigned int val;
	unsigned int temp;
	val = __raw_readl(base + SPI_PENDING_CLR);
	temp = SPI_GET_PEND_CLR_RX_TRIG_INT(val);

	if (temp == 1)
		hs_spi_slave_read_irq(spi);
}

static void spi_slave_burst_read_irq(void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;

	unsigned int val;
	unsigned int temp;
	val = __raw_readl(base + SPI_PENDING_CLR);
	temp = SPI_GET_PEND_CLR_RX_TRIG_INT(val);

	hs_spi_fw_read_irq(spi);
}




struct hs_spi_data *hs_spi_request(int hs_spi_id)
{
	struct hs_spi_data *hs_spi;
	int found = 0;

	mutex_lock(&spi_lock);

	list_for_each_entry(hs_spi, &spi_list, list) {
		if (hs_spi->spi_id == hs_spi_id) {
			found = 1;
			break;
		}
	}
	if (found != 1)
		hs_spi = NULL;

	mutex_unlock(&spi_lock);
	return hs_spi;
}
EXPORT_SYMBOL(hs_spi_request);

static unsigned int drime4_spi_freq(struct d4_hs_spi_config *spi,
		enum d4_spi_mclk bclk)
{
	unsigned int spi_max_clk, reval, cnt, remval;

	cnt = 0;
	spi_max_clk = bclk * 1000000;

	reval = spi_max_clk / spi->speed_hz;
	remval = spi_max_clk % spi->speed_hz;

	if (reval == 0)
		return reval;

	if (remval) {
		if ((reval % 2) == 0)
			reval = reval - 2;
		else
			reval = reval - 1;
	}

	while (1) {
		reval = (reval / 2);
		cnt++;
		if ((reval == 0) || (reval == 1))
			break;
	}
	reval = (cnt);
	return reval;
}

static unsigned int drime4_spi_get_per(struct d4_hs_spi_config *spi_info)
{
	u32 psr_min, psr_max, psr_mid;
	u32 speed, clkmin, clkmid, clkmax;
	unsigned int cur_sclk = 0;
	struct d4_hs_spi_config *spi = spi_info;
	unsigned int psr;

	if (spi->speed_hz > SPI_MAX_CLK) {
		/* max possible */
		cur_sclk = CLKSEL100;
		speed = cur_sclk << 4;
	} else if (spi->speed_hz < SPI_MIN_CLK) {
		/* min possible */
		cur_sclk = CLKSEL27;
		speed = cur_sclk << 4;
	} else {
		/* normal type set prescale */
		psr_mid = drime4_spi_freq(spi, D4_SPI_MST_CLK_81M);

		clkmid = spi_freq_table[psr_mid].clk_freq_mid;

		psr_max = drime4_spi_freq(spi, D4_SPI_MST_CLK_100M);
		clkmax = spi_freq_table[psr_max].clk_freq_max;

		psr_min = drime4_spi_freq(spi, D4_SPI_MST_CLK_27M);
		clkmin = spi_freq_table[psr_min].clk_freq_min;

		speed = max3(clkmid, clkmin, clkmax);

		if (speed == clkmid) {
			cur_sclk = CLKSEL81;
			psr = psr_mid;
		} else if (speed == clkmax) {
			cur_sclk = CLKSEL100;
			psr = psr_max;
		} else {
			cur_sclk = CLKSEL27;
			psr = psr_min;
		}
		speed = ((cur_sclk << 4) | psr);

	}
	return speed;
}

void hs_spi_close(struct hs_spi_data *spi)
{
	void __iomem *base = spi->regs;
	unsigned int val;

	val = 0;
	SPI_CH_ON_TX_CH(val, SPI_REG_DISABLE);
	SPI_CH_ON_RX_CH(val, SPI_REG_DISABLE);
	__raw_writel(val, base + SPI_CH_ON);

}
EXPORT_SYMBOL(hs_spi_close);

static void hs_spi_reset(struct hs_spi_data *spi)
{
	void __iomem *base = spi->regs;
	unsigned int val;

	val = __raw_readl(base + SPI_CH_CFG);

	SPI_CH_CFG_SW_RST(val, SPI_REG_ENABLE);
	__raw_writel(val, base + SPI_CH_CFG);

	SPI_CH_CFG_SW_RST(val, SPI_REG_DISABLE);
	__raw_writel(val, base + SPI_CH_CFG);
}


static void hs_spi_pad_val_set(struct hs_spi_data *spi)
{
	void __iomem *base = spi->regs;
	unsigned int val;

	val = __raw_readl(base + SPI_CH_CFG);
	gpio_direction_output(port_num[spi->spi_id].port_num_clk, ((val >> 3) & 0x1));

	val = __raw_readl(base + SPI_SIG_CTRL);
	gpio_direction_output(port_num[spi->spi_id].port_num_cs, ~((val >> 2) & 0x1));
}

void hs_spi_fix(struct hs_spi_data *spi, enum hs_spi_fix val)
{
	if (val == SH_SPI_FIX_EN) {

		pinctrl_free_gpio(port_num[spi->spi_id].port_num_clk);

		pinctrl_free_gpio(port_num[spi->spi_id].port_num_di);
		pinctrl_free_gpio(port_num[spi->spi_id].port_num_do);

		pinctrl_free_gpio(port_num[spi->spi_id].port_num_cs);
		spi->pmx = devm_pinctrl_get(&spi->pdev->dev);
		spi->pins_default = pinctrl_lookup_state(spi->pmx, PINCTRL_STATE_DEFAULT);
		pinctrl_select_state(spi->pmx, spi->pins_default);
	} else {
		devm_pinctrl_put(spi->pmx);
		pinctrl_request_gpio(port_num[spi->spi_id].port_num_clk);

		pinctrl_request_gpio(port_num[spi->spi_id].port_num_di);
		pinctrl_request_gpio(port_num[spi->spi_id].port_num_do);

		pinctrl_request_gpio(port_num[spi->spi_id].port_num_cs);
	}
}
EXPORT_SYMBOL(hs_spi_fix);

void hs_spi_pin_set(struct hs_spi_data *spi, struct spi_pin_info *pad_info)
{

	if (pad_info->fix_val == SH_SPI_FIX_EN) {
		if (pad_info->pad_clk == SH_SPI_FIX_EN) {
			pinctrl_free_gpio(port_num[spi->spi_id].port_num_clk);
		}
		if (pad_info->pad_di == SH_SPI_FIX_EN) {
			pinctrl_free_gpio(port_num[spi->spi_id].port_num_di);
		}
		if (pad_info->pad_do == SH_SPI_FIX_EN) {
			pinctrl_free_gpio(port_num[spi->spi_id].port_num_do);
		}
		if (pad_info->pad_cs == SH_SPI_FIX_EN) {
			pinctrl_free_gpio(port_num[spi->spi_id].port_num_cs);
		}
		spi->pmx = devm_pinctrl_get(&spi->pdev->dev);
		if (IS_ERR(spi->pmx)) {
			printk("get spi->pmx is failed \n");
		} else {
			spi->pins_default = pinctrl_lookup_state(spi->pmx, PINCTRL_STATE_DEFAULT);
			pinctrl_select_state(spi->pmx, spi->pins_default);
		}
	} else {
		devm_pinctrl_put(spi->pmx);

		if (pad_info->pad_clk == SH_SPI_FIX_EN) {
			pinctrl_request_gpio(port_num[spi->spi_id].port_num_clk);
		}
		if (pad_info->pad_di == SH_SPI_FIX_EN) {
			pinctrl_request_gpio(port_num[spi->spi_id].port_num_di);
		}
		if (pad_info->pad_do == SH_SPI_FIX_EN) {
			pinctrl_request_gpio(port_num[spi->spi_id].port_num_do);
		}
		if (pad_info->pad_cs == SH_SPI_FIX_EN) {
			pinctrl_request_gpio(port_num[spi->spi_id].port_num_cs);
		}
	}
}
EXPORT_SYMBOL(hs_spi_pin_set);


static void hs_spi_pad_set(struct hs_spi_data *spi, enum hs_spi_fix val)
{

	if (val == SH_SPI_FIX_EN) {

		pinctrl_free_gpio(port_num[spi->spi_id].port_num_clk);
/*
		pinctrl_free_gpio(port_num[spi->spi_id].port_num_di);
		pinctrl_free_gpio(port_num[spi->spi_id].port_num_do);
*/
		pinctrl_free_gpio(port_num[spi->spi_id].port_num_cs);
		spi->pmx = devm_pinctrl_get(&spi->pdev->dev);
		spi->pins_default = pinctrl_lookup_state(spi->pmx, PINCTRL_STATE_DEFAULT);
		pinctrl_select_state(spi->pmx, spi->pins_default);
	} else {
		devm_pinctrl_put(spi->pmx);
		pinctrl_request_gpio(port_num[spi->spi_id].port_num_clk);
/*
		pinctrl_request_gpio(port_num[spi->spi_id].port_num_di);
		pinctrl_request_gpio(port_num[spi->spi_id].port_num_do);
*/
		pinctrl_request_gpio(port_num[spi->spi_id].port_num_cs);
	}
}


static void hs_spi_cnt_delay_set(struct hs_spi_data *spi, struct d4_hs_spi_config *spi_info)
{

	void __iomem *base = spi->regs;
	u32 predelay, postdelay, holddelay, cur_clk;
	u32 val;
	u32 tpsr;
	u32 tsclk;

	predelay = spi_info->delayconf.pre_delay;
	postdelay = spi_info->delayconf.post_delay;
	holddelay = spi_info->delayconf.hold_delay;

	val = __raw_readw(base + SPI_CLK_PS);
	tpsr = (val & 0x0F);
	tsclk = (val & 0xF0)>>4;

	if (tsclk == CLKSEL81)
		cur_clk = spi_freq_table[tpsr].clk_freq_mid;
	else if (tsclk == CLKSEL100)
		cur_clk = spi_freq_table[tpsr].clk_freq_max;
	else
		cur_clk = spi_freq_table[tpsr].clk_freq_min;

	if (predelay != 0)
		predelay = (((predelay * cur_clk) / 1000000)*2) - 2;
	if (postdelay != 0)
		postdelay = (((postdelay * cur_clk) / 1000000)*2) - 2;
	if (holddelay != 0)
		holddelay = (((holddelay * cur_clk) / 1000000) * 2) - 3;

	if (predelay > 0x3F)
		predelay = 0x3F;

	if (postdelay > 0x3F)
		postdelay = 0x3F;

	if (holddelay > 0x3F)
		holddelay = 0x3F;

	val = __raw_readl(base + SPI_SIG_CTRL);
	SPI_SIG_CTRL_DELAY_DIS(val, SPI_REG_ENABLE);
	SPI_SIG_CTRL_CNT_PRE(val, predelay);
	SPI_SIG_CTRL_CNT_POST(val, postdelay);
	SPI_SIG_CTRL_CNT_HOLD(val, holddelay);
	__raw_writel(val, base + SPI_SIG_CTRL);
}


void hs_spi_get_phy_info(struct hs_spi_data *spi, struct spi_phys_reg_info *info)
{
	info->reg_start_addr = spi->sfr_start;
	info->reg_size = spi->sfr_size;
}
EXPORT_SYMBOL(hs_spi_get_phy_info);


void hs_spi_pad_reset(struct hs_spi_data *spi)
{
	hs_spi_pad_val_set(spi);
	hs_spi_pad_set(spi, SH_SPI_FIX_DIS);
	udelay(100);
	hs_spi_reset(spi);
	hs_spi_fix(spi, SH_SPI_FIX_EN);
}
EXPORT_SYMBOL(hs_spi_pad_reset);

void hs_spi_burst_set(struct hs_spi_data *spi, unsigned int val)
{
	if (val == SPI_REG_ENABLE) {
		pinctrl_put(spi->pmx);
		gpio_direction_output(port_num[spi->spi_id].port_num_cs, 0);
		pinctrl_request_gpio(port_num[spi->spi_id].port_num_cs);
	} else {
		gpio_direction_output(port_num[spi->spi_id].port_num_cs, 1);
		pinctrl_free_gpio(port_num[spi->spi_id].port_num_cs);
		spi->pmx = devm_pinctrl_get(&spi->pdev->dev);
		spi->pins_default = pinctrl_lookup_state(spi->pmx, PINCTRL_STATE_DEFAULT);
		pinctrl_select_state(spi->pmx, spi->pins_default);
	}
}
EXPORT_SYMBOL(hs_spi_burst_set);
/**
 * @brief  사용 할 SPI의 기본 설정을 Setting 하는 함수
 * @fn     void hs_spi_config(struct hs_spi_data *spi, struct d4_hs_spi_config *spi_info)
 * @param  *spi				:[in]사용 할 SPI의 정보를 담고있는 포인터
 * @param  *spi_info	:[in] SPI channel 설정에 필요한 정보를 담고 있는 포인터
 * @return void
 * @author kyuchun.han
 * @note
 */
void hs_spi_config(struct hs_spi_data *spi, struct d4_hs_spi_config *spi_info)
{
	unsigned int val;
	void __iomem *base;
	struct d4_hs_spi_config *ch;

	if (spi == NULL)
		return;

	base = spi->regs;

	if (spi_info == NULL)
			return;

	ch = spi_info;

	/*Set Prescale */
	if (((ch->setup_select & 0x01) == SH_SPI_SPEED) || ((ch->setup_select & 0x3F) == SH_SPI_ALL)) {
		val = drime4_spi_get_per(ch);
		__raw_writel(val, base + SPI_CLK_PS);
	}

	/*SPI CPHA_CPOL MODE SETUP*/
	if (((ch->setup_select & 0x02) == SH_SPI_WAVEMODE) || ((ch->setup_select & 0x3F) == SH_SPI_ALL)) {
		val = __raw_readl(base + SPI_CH_CFG);
		SPI_CH_CFG_CPHA(val, (spi_info->mode&0x1));
		SPI_CH_CFG_CPOL(val, ((spi_info->mode>>0x1)&0x1));
		SPI_CH_CFG_MST_SLV(val, 0);
		__raw_writel(val, base + SPI_CH_CFG);
	}

	if (((ch->setup_select & 0x04) == SH_SPI_BPW) || ((ch->setup_select & 0x3F) == SH_SPI_ALL)) {
		/*FIFO WIDTH SETUP*/
		if (spi_info->bpw <= 8)
			spi->cur_bpw = 8;
		else if ((spi_info->bpw > 8) && (spi_info->bpw <= 16))
			spi->cur_bpw = 16;
		else
			spi->cur_bpw = 32;

		val = __raw_readl(base + SPI_FIFO_CFG);
		SPI_FIFO_CFG_FIFO_WIDTH(val, (spi->cur_bpw>>4));
		SPI_FIFO_CFG_CH_WIDTH(val, (spi->cur_bpw>>4));
		__raw_writel(val, base + SPI_FIFO_CFG);
		spi->set_bpw = spi_info->bpw;
	}


	if (((ch->setup_select & 0x08) == SH_SPI_INVERT) || ((ch->setup_select & 0x3F) == SH_SPI_ALL)) {
			/* set invert enable */
		if (spi_info->ss_inven) {
			val = readl(base + SPI_SIG_CTRL);
			SPI_SIG_CTRL_SS_INVEN(val, spi_info->ss_inven);
			__raw_writel(val, base + SPI_SIG_CTRL);
		}
	}

	/* default set auto mode & manual disable */
	val = __raw_readl(base + SPI_SIG_CTRL);
	SPI_SIG_CTRL_SS_MODE(val, SPI_REG_ENABLE);
	__raw_writel(val, base + SPI_SIG_CTRL);
	spi->burst_mode = SPI_REG_DISABLE;

	if (((ch->setup_select & 0x10) == SH_SPI_BURST) || ((ch->setup_select & 0x3F) == SH_SPI_ALL)) {
		/* set toggle disable */
		if (spi_info->spi_ttype == D4_SPI_TRAN_BURST_ON) {
			val = __raw_readl(base + SPI_SIG_CTRL);
			SPI_SIG_CTRL_SS_TOGG_DIS(val, D4_SPI_TOGG_DISABLE_ON);
			__raw_writel(val, base + SPI_SIG_CTRL);
			spi->burst_mode = SPI_REG_ENABLE;

		} else {
			val = __raw_readl(base + SPI_SIG_CTRL);
			SPI_SIG_CTRL_SS_TOGG_DIS(val, D4_SPI_TOGG_DISABLE_OFF);
			__raw_writel(val, base + SPI_SIG_CTRL);
			spi->burst_mode = SPI_REG_DISABLE;
		}
	}

	if (((ch->setup_select & 0x20) == SH_SPI_WAITTIME) || ((ch->setup_select & 0x3F) == SH_SPI_ALL)) {
		/* waiting time set*/
		spi->waittime = msecs_to_loops(spi_info->waittime);
	}

	if (((ch->setup_select & 0x40) == HS_SPI_DELAY) || ((ch->setup_select & 0x3F) == SH_SPI_ALL)) {
		/* delay time set*/
		hs_spi_cnt_delay_set(spi, spi_info);
	}

	hs_spi_pad_val_set(spi);
	hs_spi_pad_set(spi, SH_SPI_FIX_DIS);
	hs_spi_reset(spi);
	hs_spi_pad_set(spi, SH_SPI_FIX_EN);
}
EXPORT_SYMBOL(hs_spi_config);

/**
 * @brief  사용 할 SPI의 기본 설정을 Setting을 초기화 하는 함수
 * @fn     void hs_spi_config_clear(struct hs_spi_data *spi)
 * @param  *spi				:[in]사용 하고 있는  SPI의 정보를 담고있는 포인터
 * @return void
 * @author kyuchun.han
 * @note
 */
void hs_spi_config_clear(struct hs_spi_data *spi)
{
	void __iomem *base = spi->regs;
	u32 val;

	__raw_writel(SPI_REG_DISABLE, base + SPI_PACKET_CNT);
	__raw_writel(SPI_REG_DISABLE, base + SPI_FIFO_CFG);
	__raw_writel(SPI_REG_DISABLE, base + SPI_SIG_CTRL);
	__raw_writel(SPI_REG_DISABLE, base + SPI_CH_CFG);
	__raw_writel(SPI_REG_DISABLE, base + SPI_CLK_PS);


	val = __raw_readl(base + SPI_SIG_CTRL);
	SPI_SIG_CTRL_DELAY_DIS(val, SPI_REG_DISABLE);
	SPI_SIG_CTRL_CNT_PRE(val, 0);
	SPI_SIG_CTRL_CNT_POST(val, 0);
	SPI_SIG_CTRL_CNT_HOLD(val, 0);
	__raw_writel(val, base + SPI_SIG_CTRL);
}
EXPORT_SYMBOL(hs_spi_config_clear);

static void spi_fifo_clear(void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;

	while (SPI_STATUS_RX_FIFO_LVL(__raw_readl(base + SPI_STATUS)) != 0)
		__raw_readw(base + SPI_RX_DATA);
	__raw_writel(SPI_PENDING_ALL_CLEAR, base + SPI_PENDING_CLR);
}

static void spi_int_lvl_set(void *data)
{
	struct hs_spi_data *spi = data;
	void __iomem *base = spi->regs;
	unsigned int val, setval;
	unsigned int div_val;

	unsigned int lens = spi->n_bytes;

	val = __raw_readl(base + SPI_FIFO_CFG);

	div_val = (spi->cur_bpw >> 3);

	if (lens / FIFO_DEPTH) {
		setval = FIFO_THRESHOLD;
	} else if (lens == div_val) {
		setval = div_val;
	} else {
		setval = lens / div_val;
		if (setval & 0x1)
			setval = setval - 1;
	}

	SPI_FIFO_CFG_TX_RDY_LVL(val, setval);
	SPI_FIFO_CFG_RX_RDY_LVL(val, setval);
	__raw_writel(val, base + SPI_FIFO_CFG);
}


static void spi_int_lvl_rset(void *data)
{
	struct hs_spi_data *spi = data;
	void __iomem *base = spi->regs;
	unsigned int val;

	val = __raw_readl(base + SPI_FIFO_CFG);

	SPI_FIFO_CFG_RX_RDY_LVL(val, 3);
	__raw_writel(val, base + SPI_FIFO_CFG);
}

static void spi_int_lvl_wset(void *data)
{
	struct hs_spi_data *spi = data;
	void __iomem *base = spi->regs;
	unsigned int val, setval;
	unsigned int div_val;

	unsigned int lens = spi->n_bytes;

	val = __raw_readl(base + SPI_FIFO_CFG);

	div_val = (spi->cur_bpw >> 3);

	if (lens / FIFO_DEPTH) {
		setval = FIFO_THRESHOLD;
	} else if (lens == div_val) {
		setval = div_val;
	} else {
		setval = lens / div_val;
		if (setval & 0x1)
			setval = setval - 1;
	}

	SPI_FIFO_CFG_TX_RDY_LVL(val, setval);
	__raw_writel(val, base + SPI_FIFO_CFG);
}


/**
 * @brief  SPI의 Interrupt 기능을 사용하여 Read를 수행하는 함수
 * @fn     int hs_spi_polling_read(struct hs_spi_data *spi, struct spi_data_info *spi_data)
 * @param  *spi				:[in]사용 할 SPI의 정보를 담고있는 포인터
 * @param  *spi_data	:[in]사용 할 SPI channel을 통해 Write되거나 Read될 Data의 정보를 담고 있는 포인터
 * @return int					:[out] 함수 동작의 상태정보 ( 0: SUCCESS)
 * @author kyuchun.han
 * @note
 */
int hs_spi_interrupt_read(struct hs_spi_data *spi,
		struct spi_data_info *spi_data)
{
	int retval;
	unsigned int chen, fifo_lvl, write_len, int_en;
	void __iomem *base = spi->regs;

	retval = 0;

	/*Interrupt all disable*/
	__raw_writel(SPI_REG_DISABLE, base + SPI_INT_EN);
	spi->int_handler = spi_readwrite_irq;

	spi->read_len = 0;
	spi->n_bytes = (spi_data->data_len * (spi->cur_bpw / 8));
	spi->rx = spi_data->rbuffer;
	spi->rx_end = spi->rx + spi->n_bytes;
	spi->rwtype = READ_TYPE;

	spi_fifo_clear(spi);
	spi_int_lvl_wset(spi);

	fifo_lvl = __raw_readl(base + SPI_FIFO_CFG);
	write_len = SPI_GET_FIFO_CFG_RX_RDY_LVL(fifo_lvl);

	spi->write_len = write_len;

	while (1) {
		if (write_len) {
			switch (spi->cur_bpw) {
			case 8:
				__raw_writeb(0xFF, base + SPI_TX_DATA);
				write_len -= 1;
				break;
			case 16:
				__raw_writew(0xFF, base + SPI_TX_DATA);
				write_len -= 2;
				break;
			default:
				__raw_writel(0xFF, base + SPI_TX_DATA);
				write_len -= 4;
				break;
			}
		} else
			break;
	}

	int_en = 0;
	SPI_INT_EN_RX_FIFO_TRIG(int_en, SPI_REG_ENABLE);
	SPI_INT_EN_TX_FIFO_TRIG(int_en, SPI_REG_ENABLE);
	__raw_writel(int_en, base + SPI_INT_EN);
	chen = 0;
	SPI_CH_ON_TX_CH(chen, SPI_REG_ENABLE);
	SPI_CH_ON_RX_CH(chen, SPI_REG_ENABLE);
	__raw_writel(chen, base + SPI_CH_ON);

	if (!wait_for_completion_timeout(&spi->done,
			msecs_to_jiffies(spi->waittime)))
		retval = -EIO;

	if (retval)
		spi_fifo_clear(spi);


	__raw_writel(SPI_REG_DISABLE, base + SPI_INT_EN);
	__raw_writel(SPI_REG_DISABLE, base + SPI_CH_ON);
	return retval;
}
EXPORT_SYMBOL(hs_spi_interrupt_read);

int hs_spi_slave_interrupt_read(struct hs_spi_data *spi,
		struct spi_data_info *spi_data, int ReadyBusy, int notification, int rb_val, int noti_val)
{
	int retval;
	unsigned int chen, fifo_lvl, write_len, int_en;
	unsigned int wait_time, temp;
	void __iomem *base = spi->regs;

	retval = 0;

	/*Interrupt all disable*/
	__raw_writel(SPI_REG_DISABLE, base + SPI_INT_EN);
	spi->int_handler = spi_slave_read_irq;

	spi->read_len = 0;
	spi->read_len_temp = 0;
	spi->temp_read = 0;
	/*spi->n_bytes = (spi_data->data_len * (spi->cur_bpw / 8));*/
	spi->n_bytes = 0;
	spi->rx = spi_data->rbuffer;
	spi->rx_end = spi->rx + spi->n_bytes;
	spi->rwtype = READ_TYPE;
	spi->irq_cnt = 1000;
	spi_fifo_clear(spi);
	spi_int_lvl_rset(spi);

	int_en = 0;
	SPI_INT_EN_RX_FIFO_TRIG(int_en, SPI_REG_ENABLE);
	__raw_writel(int_en, base + SPI_INT_EN);
	chen = 0;
	SPI_CH_ON_RX_CH(chen, SPI_REG_ENABLE);
	__raw_writel(chen, base + SPI_CH_ON);

	gpio_direction_output(notification, noti_val);
	gpio_direction_output(ReadyBusy, rb_val);
	wait_for_completion_interruptible(&spi->done);
	/*wait_for_completion(&spi->done);*/

	wait_time = spi->waittime;
	do {
		temp = __raw_readl(base + SPI_STATUS);
		temp = temp >> 15;
	} while ((temp) && --wait_time);

	if (temp) {
		dev_err(&spi->pdev->dev, "SPI SLAVE INTERRUPT READ ERROR\n");
		retval = -EIO;
	}

	spi_fifo_clear(spi);

	__raw_writel(SPI_REG_DISABLE, base + SPI_INT_EN);
	__raw_writel(SPI_REG_DISABLE, base + SPI_CH_ON);
	spi_data->data_len = (spi->n_bytes * (spi->cur_bpw / 8));
	return retval;
}

int hs_spi_slave_interrupt_write(struct hs_spi_data *spi,
		struct spi_data_info *spi_data, int ReadyBusy, int notification, int rb_val, int noti_val)
{
	void __iomem *base = spi->regs;

	int rtval;
	unsigned int writen_len, int_en, chen, wait_time, temp;

	rtval = 0;
	spi_fifo_clear(spi);

	/*Interrupt all disable*/
	__raw_writel(SPI_REG_DISABLE, base + SPI_INT_EN);

	spi->int_handler = spi_slave_write_irq;
	spi->read_len = 0;
	spi->n_bytes = (spi_data->data_len * (spi->cur_bpw / 8));
	spi->tx = spi_data->wbuffer;
	spi->tx_end = spi->tx + spi->n_bytes;
	spi->rwtype = WRITE_TYPE;


	if (spi->n_bytes / FIFO_DEPTH)
		/*data over fifo */
		writen_len = FIFO_DEPTH;
	else
		writen_len = spi->n_bytes;


	switch (spi->cur_bpw) {
	case 8:
		__raw_writesb(base + SPI_TX_DATA, spi->tx, writen_len);
		break;
	case 16:
		__raw_writesw(base + SPI_TX_DATA, spi->tx, writen_len / 2);
		break;
	default:
		__raw_writesl(base + SPI_TX_DATA, spi->tx, writen_len / 4);
		break;
	}
	spi->tx += writen_len;
	spi->read_len = writen_len;

	spi_int_lvl_wset(spi);
	int_en = 0;
	SPI_INT_EN_TX_FIFO_TRIG(int_en, SPI_REG_ENABLE);
	/*SPI_INT_EN_RX_FIFO_TRIG(int_en, SPI_REG_ENABLE);*/
	writel(int_en, base + SPI_INT_EN);

	chen = 0;
	SPI_CH_ON_TX_CH(chen, SPI_REG_ENABLE);
/*	SPI_CH_ON_RX_CH(chen, SPI_REG_ENABLE);*/
	writel(chen, base + SPI_CH_ON);

	gpio_direction_output(notification, noti_val);
	gpio_direction_output(ReadyBusy, rb_val);
	wait_for_completion_interruptible(&spi->done);
	/*wait_for_completion(&spi->done);*/
	wait_time = spi->waittime;
	do {
		temp = __raw_readl(base + SPI_STATUS);
		temp = (temp >> 8) & 0x3F;
	} while ((temp) && --wait_time);

	if (temp) {
		dev_err(&spi->pdev->dev, "SPI SLAVE INTERRUPT WRITE&READ ERROR\n");
		rtval = -EIO;

	}
	udelay(100);
	writel(SPI_REG_DISABLE, base + SPI_INT_EN);
	writel(SPI_REG_DISABLE, base + SPI_CH_ON);
	return rtval;
}


int hs_spi_slave_interrupt_burst_read(struct hs_spi_data *spi,
		struct spi_data_info *spi_data, int ReadyBusy, int notification, int rb_val, int noti_val)
{
	int retval;
	unsigned int chen, fifo_lvl, write_len, int_en;
	void __iomem *base = spi->regs;

	retval = 0;


	/*Interrupt all disable*/
	__raw_writel(SPI_REG_DISABLE, base + SPI_INT_EN);
	spi->int_handler = spi_slave_burst_read_irq;

	spi->read_len = 0;
	spi->n_bytes = (spi_data->data_len * (spi->cur_bpw / 8));
	spi->rx = spi_data->rbuffer;
	spi->rx_end = spi->rx + spi->n_bytes;
	spi->rwtype = READ_TYPE;
	spi->irq_cnt = 1000;
	spi_fifo_clear(spi);
	spi_int_lvl_rset(spi);
	printk(KERN_ALERT"==================hs_spi_slave_interrupt_burst_read:%d\n", spi->n_bytes);

	int_en = 0;
	SPI_INT_EN_RX_FIFO_TRIG(int_en, SPI_REG_ENABLE);
	__raw_writel(int_en, base + SPI_INT_EN);
	chen = 0;
	SPI_CH_ON_RX_CH(chen, SPI_REG_ENABLE);
	__raw_writel(chen, base + SPI_CH_ON);

	gpio_direction_output(notification, noti_val);
	gpio_direction_output(ReadyBusy, rb_val);
	wait_for_completion(&spi->done);

	printk(KERN_ALERT"==================burst_read done:%d\n", spi->read_len);

	__raw_writel(SPI_REG_DISABLE, base + SPI_INT_EN);
	__raw_writel(SPI_REG_DISABLE, base + SPI_CH_ON);

	spi_data->data_len = (spi->n_bytes * (spi->cur_bpw / 8));

	return retval;
}
EXPORT_SYMBOL(hs_spi_slave_interrupt_burst_read);


/**
 * @brief  SPI의 Interrupt 기능을 사용하여 Write를 수행하는 함수
 * @fn     int hs_spi_polling_read(struct hs_spi_data *spi, struct spi_data_info *spi_data)
 * @param  *spi				:[in]사용 할 SPI의 정보를 담고있는 포인터
 * @param  *spi_data	:[in]사용 할 SPI channel을 통해 Write되거나 Read될 Data의 정보를 담고 있는 포인터
 * @return int					:[out] 함수 동작의 상태정보 ( 0: SUCCESS)
 * @author kyuchun.han
 * @note
 */
int hs_spi_interrupt_write(struct hs_spi_data *spi,
		struct spi_data_info *spi_data)
{
	void __iomem *base = spi->regs;

	int rtval;
	unsigned int writen_len, int_en, chen;

	rtval = 0;

	spi_fifo_clear(spi);

	/*Interrupt all disable*/
	__raw_writel(SPI_REG_DISABLE, base + SPI_INT_EN);

	spi->int_handler = spi_readwrite_irq;
	spi->read_len = 0;
	spi->n_bytes = (spi_data->data_len * (spi->cur_bpw / 8));
	spi->tx = spi_data->wbuffer;
	spi->tx_end = spi->tx + spi->n_bytes;
	spi->rwtype = WRITE_TYPE;
	spi_int_lvl_set(spi);

	if (spi->n_bytes / FIFO_DEPTH)
		/*data over fifo */
		writen_len = FIFO_DEPTH;
	else
		writen_len = spi->n_bytes;

	switch (spi->cur_bpw) {
	case 8:
		__raw_writesb(base + SPI_TX_DATA, spi->tx, writen_len);
		break;
	case 16:
		__raw_writesw(base + SPI_TX_DATA, spi->tx, writen_len / 2);
		break;
	default:
		__raw_writesl(base + SPI_TX_DATA, spi->tx, writen_len / 4);
		break;
	}
	spi->tx += writen_len;

	int_en = 0;
	SPI_INT_EN_TX_FIFO_TRIG(int_en, SPI_REG_ENABLE);
	SPI_INT_EN_RX_FIFO_TRIG(int_en, SPI_REG_ENABLE);
	writel(int_en, base + SPI_INT_EN);

	chen = 0;
	SPI_CH_ON_TX_CH(chen, SPI_REG_ENABLE);
	SPI_CH_ON_RX_CH(chen, SPI_REG_ENABLE);
	writel(chen, base + SPI_CH_ON);

	if (!wait_for_completion_timeout(&spi->done, spi->waittime))
		rtval = -EIO;

	if (rtval)
		spi_fifo_clear(spi);


	writel(SPI_REG_DISABLE, base + SPI_INT_EN);
	writel(SPI_REG_DISABLE, base + SPI_CH_ON);
	return rtval;
}
EXPORT_SYMBOL(hs_spi_interrupt_write);


/**
 * @brief  SPI의 Interrupt 기능을 사용하여 Write와  Read를 동시에 수행하는 함수
 * @fn     int hs_spi_polling_read(struct hs_spi_data *spi, struct spi_data_info *spi_data)
 * @param  *spi				:[in]사용 할 SPI의 정보를 담고있는 포인터
 * @param  *spi_data	:[in]사용 할 SPI channel을 통해 Write되거나 Read될 Data의 정보를 담고 있는 포인터
 * @return int					:[out] 함수 동작의 상태정보 ( 0: SUCCESS)
 * @author kyuchun.han
 * @note
 */
int hs_spi_interrupt_rw(struct hs_spi_data *spi, struct spi_data_info *spi_data)
{
	void __iomem *base = spi->regs;
	int rtval;
	unsigned int writen_len, int_en, chen;
	rtval = 0;

	spi_fifo_clear(spi);

	/*Interrupt all disable*/
	__raw_writel(SPI_REG_DISABLE, base + SPI_INT_EN);

	spi->int_handler = spi_readwrite_irq;
	spi->read_len = 0;
	spi->n_bytes = (spi_data->data_len * (spi->cur_bpw / 8));
	spi->tx = spi_data->wbuffer;
	spi->tx_end = spi->tx + spi->n_bytes;

	spi->rx = spi_data->rbuffer;
	spi->rx_end = spi->rx + spi->n_bytes;
	spi->rwtype = WRITE_READ_TYPE;

	spi_int_lvl_set(spi);

	if (spi->n_bytes / FIFO_DEPTH)
		/*data over fifo */
		writen_len = FIFO_DEPTH;
	else
		writen_len = spi->n_bytes;

	switch (spi->cur_bpw) {
	case 8:
		__raw_writesb(base + SPI_TX_DATA, spi->tx, writen_len);
		break;
	case 16:
		__raw_writesw(base + SPI_TX_DATA, spi->tx, writen_len / 2);
		break;
	default:
		__raw_writesl(base + SPI_TX_DATA, spi->tx, writen_len / 4);
		break;
	}
	spi->tx += writen_len;

	int_en = 0;
	SPI_INT_EN_TX_FIFO_TRIG(int_en, SPI_REG_ENABLE);
	SPI_INT_EN_RX_FIFO_TRIG(int_en, SPI_REG_ENABLE);
	writel(int_en, base + SPI_INT_EN);

	chen = 0;
	SPI_CH_ON_TX_CH(chen, SPI_REG_ENABLE);
	SPI_CH_ON_RX_CH(chen, SPI_REG_ENABLE);
	writel(chen, base + SPI_CH_ON);

	if (!wait_for_completion_timeout(&spi->done, spi->waittime))
		rtval = -EIO;

	if (rtval)
		spi_fifo_clear(spi);


	writel(SPI_REG_DISABLE, base + SPI_INT_EN);
	writel(SPI_REG_DISABLE, base + SPI_CH_ON);
	return rtval;
}
EXPORT_SYMBOL(hs_spi_interrupt_rw);


/**
 * @brief  SPI의 Polling 기능을 사용하여 Read 하는 함수
 * @fn     int hs_spi_polling_read(struct hs_spi_data *spi, struct spi_data_info *spi_data)
 * @param  *spi				:[in]사용 할 SPI의 정보를 담고있는 포인터
 * @param  *spi_data	:[in]사용 할 SPI channel을 통해 Write되거나 Read될 Data의 정보를 담고 있는 포인터
 * @return int					:[out] 함수 동작의 상태정보 ( 0: SUCCESS)
 * @author kyuchun.han
 * @note
 */
int hs_spi_polling_read(struct hs_spi_data *spi, struct spi_data_info *spi_data)
{
	void __iomem *base = spi->regs;
	unsigned int val;
	unsigned int cnt;
	unsigned int temp;
	int rtval;
	unsigned int wait_time = spi->waittime;
	rtval = 0;

	spi_fifo_clear(spi);

	/* set valid bit for controller */

	if (spi->set_bpw < spi->cur_bpw) {
		val = 0;
		SPI_VALID_BITS_EN(val, SPI_REG_ENABLE);
		SPI_VALID_BITS_SET(val, spi->set_bpw);
		__raw_writel(val, base + SPI_VALID_BITS);
	}

	val = 0;
	SPI_CH_ON_TX_CH(val, SPI_REG_ENABLE);
	SPI_CH_ON_RX_CH(val, SPI_REG_ENABLE);
	__raw_writel(val, base + SPI_CH_ON);

#if 0
	if (spi->burst_mode == SPI_REG_ENABLE) {
		val = __raw_readl(base + SPI_SIG_CTRL);
		SPI_SIG_CTRL_SS_MODE(val, SPI_REG_DISABLE);
		__raw_writel(val, base + SPI_SIG_CTRL);
	}
#endif
	if (spi->burst_mode == SPI_REG_ENABLE) {
		devm_pinctrl_put(spi->pmx);
		gpio_direction_output(port_num[spi->spi_id].port_num_cs, 0);
		pinctrl_request_gpio(port_num[spi->spi_id].port_num_cs);
	}

	for (cnt = 0; cnt < spi_data->data_len; cnt++) {
		__raw_writel(0xFFFF, base + SPI_TX_DATA);

		do {
			temp = __raw_readl(base + SPI_STATUS);
			temp = (temp >> 1) & 0x1;
		} while (!temp && --wait_time);

		if (!wait_time) {
			rtval = -EIO;
			goto out_func;
		}

		switch (spi->cur_bpw) {
		case 8:
			*((u8 *) (spi_data->rbuffer) + cnt) = __raw_readb(base
					+ SPI_RX_DATA);
			break;
		case 16:
			*((u16 *) (spi_data->rbuffer) + cnt) = __raw_readw(base
					+ SPI_RX_DATA);
			break;

		default:
			*((u32 *) (spi_data->rbuffer) + cnt) = __raw_readl(base
					+ SPI_RX_DATA);
			break;
		}
	}

	wait_time = spi->waittime;
	do {
		temp = __raw_readl(base + SPI_STATUS);
		temp = temp >> 15;
	} while ((temp) && --wait_time);

	if (temp) {
		dev_err(&spi->pdev->dev, "SPI POLLING WRITE&READ ERROR\n");
		rtval = -EIO;

	}

#if 0
	if (spi->burst_mode == SPI_REG_ENABLE) {
		val = __raw_readl(base + SPI_SIG_CTRL);
		SPI_SIG_CTRL_SS_MODE(val, SPI_REG_ENABLE);
		__raw_writel(val, base + SPI_SIG_CTRL);
	}
#endif
	if (spi->burst_mode == SPI_REG_ENABLE) {
		gpio_direction_output(port_num[spi->spi_id].port_num_cs, 1);
		pinctrl_free_gpio(port_num[spi->spi_id].port_num_cs);
		spi->pmx = devm_pinctrl_get(&spi->pdev->dev);
		spi->pins_default = pinctrl_lookup_state(spi->pmx, PINCTRL_STATE_DEFAULT);
		pinctrl_select_state(spi->pmx, spi->pins_default);
	}

out_func:
	__raw_writel(0xFF, base + SPI_PENDING_CLR);
	return rtval;
}
EXPORT_SYMBOL(hs_spi_polling_read);


/**
 * @brief  SPI의 Polling 기능을 사용하여 Write 하는 함수
 * @fn     int hs_spi_polling_write(struct hs_spi_data *spi, struct spi_data_info *spi_data)
 * @param  *spi				:[in]사용 할 SPI의 정보를 담고있는 포인터
 * @param  *spi_data	:[in]사용 할 SPI channel을 통해 Write되거나 Read될 Data의 정보를 담고 있는 포인터
 * @return int					:[out] 함수 동작의 상태정보 ( 0: SUCCESS)
 * @author kyuchun.han
 * @note
 */
int hs_spi_polling_write(struct hs_spi_data *spi,
		struct spi_data_info *spi_data)
{
	void __iomem *base = spi->regs;
	unsigned int val;
	unsigned int cnt;
	unsigned int temp;
	int rtval;
	unsigned int wait_time = spi->waittime;
	rtval = 0;

	spi_fifo_clear(spi);

	/* set valid bit for controller */

	if (spi->set_bpw < spi->cur_bpw) {
		val = 0;
		SPI_VALID_BITS_EN(val, SPI_REG_ENABLE);
		SPI_VALID_BITS_SET(val, spi->set_bpw);
		__raw_writel(val, base + SPI_VALID_BITS);
	}


	val = 0;
	SPI_CH_ON_TX_CH(val, SPI_REG_ENABLE);
	SPI_CH_ON_RX_CH(val, SPI_REG_ENABLE);
	__raw_writel(val, base + SPI_CH_ON);


	if (spi->burst_mode == SPI_REG_ENABLE) {
		devm_pinctrl_put(spi->pmx);
		gpio_direction_output(port_num[spi->spi_id].port_num_cs, 0);
		pinctrl_request_gpio(port_num[spi->spi_id].port_num_cs);
	}

	for (cnt = 0; cnt < spi_data->data_len; cnt++) {
		switch (spi->cur_bpw) {
		case 8:
			__raw_writeb(*((u8 *) (spi_data->wbuffer) + cnt), base
					+ SPI_TX_DATA);
			break;
		case 16:
			__raw_writew(*((u16 *) (spi_data->wbuffer) + cnt), base
					+ SPI_TX_DATA);
			break;
		default:
			__raw_writel(*((u32 *) (spi_data->wbuffer) + cnt), base
					+ SPI_TX_DATA);
			break;
		}

		do {
			temp = __raw_readl(base + SPI_STATUS);
			temp = (temp >> 1) & 0x1;
		} while (!temp && --wait_time);

		if (!wait_time) {
			rtval = -EIO;
			goto out_func;
		}

#if 0
		switch (spi->cur_bpw) {
		case 8:
			__raw_readb(base + SPI_RX_DATA);
			break;
		case 16:
			__raw_readw(base + SPI_RX_DATA);
			break;
		default:
			__raw_readl(base + SPI_RX_DATA);
			break;
		}
#else
		__raw_readl(base + SPI_RX_DATA);
#endif
	}

	wait_time = spi->waittime;
	do {
		temp = __raw_readl(base + SPI_STATUS);
		temp = temp >> 15;
	} while ((temp) && --wait_time);

	if (temp) {
		dev_err(&spi->pdev->dev, "SPI POLLING WRITE&READ ERROR\n");
		rtval = -EIO;

	}

	if (spi->burst_mode == SPI_REG_ENABLE) {
		gpio_direction_output(port_num[spi->spi_id].port_num_cs, 1);
		pinctrl_free_gpio(port_num[spi->spi_id].port_num_cs);
		spi->pmx = devm_pinctrl_get(&spi->pdev->dev);
		spi->pins_default = pinctrl_lookup_state(spi->pmx, PINCTRL_STATE_DEFAULT);
		pinctrl_select_state(spi->pmx, spi->pins_default);
	}

out_func:
	__raw_writel(0xFF, base + SPI_PENDING_CLR);
	return rtval;
}
EXPORT_SYMBOL(hs_spi_polling_write);


/**
 * @brief  SPI의 Polling 기능을 사용하여 Write와 Read를 동시에 수행하는 함수
 * @fn     int hs_spi_polling_read(struct hs_spi_data *spi, struct spi_data_info *spi_data)
 * @param  *spi				:[in]사용 할 SPI의 정보를 담고있는 포인터
 * @param  *spi_data	:[in]사용 할 SPI channel을 통해 Write되거나 Read될 Data의 정보를 담고 있는 포인터
 * @return int					:[out] 함수 동작의 상태정보 ( 0: SUCCESS)
 * @author kyuchun.han
 * @note
 */
int hs_spi_polling_rw(struct hs_spi_data *spi, struct spi_data_info *spi_data)
{
	void __iomem *base = spi->regs;
	unsigned int val;
	unsigned int cnt;
	unsigned int temp;
	int rtval;
	unsigned int wait_time = spi->waittime;
	rtval = 0;

	spi_fifo_clear(spi);

	/* set valid bit for controller */

	if (spi->set_bpw < spi->cur_bpw) {
		val = 0;
		SPI_VALID_BITS_EN(val, SPI_REG_ENABLE);
		SPI_VALID_BITS_SET(val, spi->set_bpw);
		__raw_writel(val, base + SPI_VALID_BITS);
	}

	val = 0;
	SPI_CH_ON_TX_CH(val, SPI_REG_ENABLE);
	SPI_CH_ON_RX_CH(val, SPI_REG_ENABLE);
	__raw_writel(val, base + SPI_CH_ON);

#if 0
	if (spi->burst_mode == SPI_REG_ENABLE) {
		val = __raw_readl(base + SPI_SIG_CTRL);
		SPI_SIG_CTRL_SS_MODE(val, SPI_REG_DISABLE);
		__raw_writel(val, base + SPI_SIG_CTRL);
	}
#endif
	if (spi->burst_mode == SPI_REG_ENABLE) {
		devm_pinctrl_put(spi->pmx);
		gpio_direction_output(port_num[spi->spi_id].port_num_cs, 0);
		pinctrl_request_gpio(port_num[spi->spi_id].port_num_cs);
	}

	for (cnt = 0; cnt < spi_data->data_len; cnt++) {
		switch (spi->cur_bpw) {
		case 8:
			__raw_writeb(*((u8 *) (spi_data->wbuffer) + cnt), base
					+ SPI_TX_DATA);
			break;
		case 16:
			__raw_writew(*((u16 *) (spi_data->wbuffer) + cnt), base
					+ SPI_TX_DATA);
			break;
		default:
			__raw_writel(*((u32 *) (spi_data->wbuffer) + cnt), base
					+ SPI_TX_DATA);
			break;
		}

		do {
			temp = __raw_readl(base + SPI_STATUS);
			temp = (temp >> 1) & 0x1;
		} while (!temp && --wait_time);

		if (!wait_time) {
			rtval = -EIO;
			goto out_func;
		}

		switch (spi->cur_bpw) {
		case 8:
			*((u8 *) (spi_data->rbuffer) + cnt) = __raw_readl(base
					+ SPI_RX_DATA);
			break;
		case 16:
			*((u16 *) (spi_data->rbuffer) + cnt) = __raw_readl(base
					+ SPI_RX_DATA);
			break;
		default:
			*((u32 *) (spi_data->rbuffer) + cnt) = __raw_readl(base
					+ SPI_RX_DATA);
			break;
		}

	}

	wait_time = spi->waittime;
	do {
		temp = __raw_readl(base + SPI_STATUS);
		temp = temp >> 15;
	} while ((temp) && --wait_time);

	if (temp) {
		dev_err(&spi->pdev->dev, "SPI POLLING WRITE&READ ERROR\n");
		rtval = -EIO;

	}

#if 0
	if (spi->burst_mode == SPI_REG_ENABLE) {
		val = __raw_readl(base + SPI_SIG_CTRL);
		SPI_SIG_CTRL_SS_MODE(val, SPI_REG_ENABLE);
		__raw_writel(val, base + SPI_SIG_CTRL);
	}
#endif
	if (spi->burst_mode == SPI_REG_ENABLE) {
		gpio_direction_output(port_num[spi->spi_id].port_num_cs, 1);
		pinctrl_free_gpio(port_num[spi->spi_id].port_num_cs);
		spi->pmx = devm_pinctrl_get(&spi->pdev->dev);
		spi->pins_default = pinctrl_lookup_state(spi->pmx, PINCTRL_STATE_DEFAULT);
		pinctrl_select_state(spi->pmx, spi->pins_default);
	}

out_func:
	__raw_writel(0xFF, base + SPI_PENDING_CLR);
	return rtval;
}
EXPORT_SYMBOL(hs_spi_polling_rw);


#define XFER_DMAADDR_INVALID DMA_BIT_MASK(32)

static void drime4_spi_dma_irq(void *data)
{
	struct hs_spi_data *spi = data;
	unsigned long flags;
	spin_lock_irqsave(&spi->lock, flags);
	spi->state &= ~RXBUSY;
	spi->state &= ~TXBUSY;
	complete(&spi->done);
	spin_unlock_irqrestore(&spi->lock, flags);
}


static void drime4_spi_slave_read_dma_irq(void *data)
{
	struct hs_spi_data *spi = data;
	unsigned long flags;
	spin_lock_irqsave(&spi->lock, flags);
	spi->state &= ~RXBUSY;
	complete(&spi->done);
	printk(KERN_ALERT"=========drime4_spi_slave_read_dma_irq==================\n");
	spin_unlock_irqrestore(&spi->lock, flags);
}


static int hs_spi_map_mssg(struct hs_spi_data *spi,
		struct spi_data_info *spi_data)
{
	struct device *dev = &spi->pdev->dev;

	if (spi_data->wbuffer != NULL) {
		spi->tx_dma = dma_map_single(dev, (void *) spi_data->wbuffer,
				spi_data->data_len, DMA_TO_DEVICE);
		if (dma_mapping_error(dev, spi->tx_dma)) {
			dev_err(dev, "dma_map_single Tx failed\n");
			spi->tx_dma = XFER_DMAADDR_INVALID;
			return -ENOMEM;
		}
	}

	if (spi_data->rbuffer != NULL) {
		spi->rx_dma = dma_map_single(dev, spi_data->rbuffer,
				spi_data->data_len, DMA_FROM_DEVICE);
		if (dma_mapping_error(dev, spi->rx_dma)) {
			dev_err(dev, "dma_map_single Rx failed\n");
			dma_unmap_single(dev, spi->tx_dma, spi_data->data_len,
					DMA_TO_DEVICE);
			spi->tx_dma = XFER_DMAADDR_INVALID;
			spi->rx_dma = XFER_DMAADDR_INVALID;
			return -ENOMEM;
		}
	}
	return 0;
}


static int hs_spi_map_slave_mssg(struct hs_spi_data *spi,
		struct spi_data_info *spi_data)
{
	struct device *dev = &spi->pdev->dev;

	if (spi_data->rbuffer != NULL) {
		spi->rx_dma = dma_map_single(dev, spi_data->rbuffer,
				spi_data->data_len, DMA_FROM_DEVICE);

		printk("========hs_spi_map_slave_mssg====  :0x%x\n", spi->rx_dma);
		if (dma_mapping_error(dev, spi->rx_dma)) {
			dev_err(dev, "dma_map_single Rx failed\n");
			dma_unmap_single(dev, spi->tx_dma, spi_data->data_len,
					DMA_TO_DEVICE);
			spi->tx_dma = XFER_DMAADDR_INVALID;
			spi->rx_dma = XFER_DMAADDR_INVALID;
			return -ENOMEM;
		}
	}
	return 0;
}


static void terminate_dma(struct hs_spi_data *spi)
{
	struct dma_chan *rxchan = spi->rx_chan;
	struct dma_chan *txchan = spi->tx_chan;

	dmaengine_terminate_all(rxchan);
	dmaengine_terminate_all(txchan);
}
#if 0
static bool dma_filter(struct dma_chan *chan, void *param)
{
#if 0
	struct dma_pl330_peri *peri = (struct dma_pl330_peri *) chan->private;
	unsigned dma_ch = (unsigned) param;

	if (peri->peri_id != dma_ch)
		return false;
#else
	u8 *peri_id;
	peri_id = chan->private;
	return *peri_id == (unsigned)param;

#endif
}
#endif

static int hs_acquire_slave_dma(struct hs_spi_data *spi)
{
	dma_cap_mask_t mask;

	/* Tru to acquire a generic DMA engine slave channel */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);

	spi->rx_chan
			= dma_request_channel(mask, pl330_filter, (void *) spi->rx_dmach);
	if (!spi->rx_chan) {
		printk("no RX DMA channel!\n");

		goto err_no_rxchan;
	}

	dev_dbg(&spi->pdev->dev, "setup for DMA on RX %s\n", 	dma_chan_name(spi->rx_chan));
	return 0;

err_no_rxchan:
	return -ENODEV;
}


static int hs_acquire_dma(struct hs_spi_data *spi)
{
	dma_cap_mask_t mask;

	/* Tru to acquire a generic DMA engine slave channel */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);
	/*
	 * We need both RX and TX channels to do DMA, else do none
	 * of them.
	 */
	spi->rx_chan
			= dma_request_channel(mask, pl330_filter, (void *) spi->rx_dmach);
	if (!spi->rx_chan) {
		dev_err(&spi->pdev->dev, "no RX DMA channel!\n");
		goto err_no_rxchan;
	}

	spi->tx_chan
			= dma_request_channel(mask, pl330_filter, (void *) spi->tx_dmach);
	if (!spi->tx_chan) {
		dev_err(&spi->pdev->dev, "no TX DMA channel!\n");
		goto err_no_txchan;
	}
	//printk("test:0x%x, 0x%x\n", spi->rx_chan, spi->tx_chan);
	dev_dbg(&spi->pdev->dev, "setup for DMA on RX %s, TX %s\n",
			dma_chan_name(spi->rx_chan),
			dma_chan_name(spi->tx_chan));

	return 0;

err_no_txchan:
	dma_release_channel(spi->rx_chan);
	spi->rx_chan = NULL;

err_no_rxchan:
	return -ENODEV;
}

static int wait_for_dma(struct hs_spi_data *spi)
{
	void __iomem *base = spi->regs;
	u32 status;
	u32 chcfg;
	u32 val;
	status = 0;

	if (!wait_for_completion_timeout(&spi->done,
			msecs_to_jiffies(spi->waittime))) {
		dev_err(&spi->pdev->dev, "dma waiting error\n");
		status = -EIO;
	}
	val = msecs_to_loops(spi->waittime);
	do {
		chcfg = __raw_readl(base + SPI_STATUS);
		chcfg = SPI_STATUS_RX_FIFO_LVL(chcfg);
	} while ((chcfg) && --val);

	if (chcfg) {
		dev_err(&spi->pdev->dev, "DMA transfer done fail\n");
		status = -EIO;
	}
	return status;
}


static int hs_spi_dma_slave_read_transfer(struct hs_spi_data *spi,
		struct spi_data_info *spi_data)
{
	void __iomem *base = spi->regs;

	struct dma_slave_config slave_config;
	struct scatterlist tx_sg;
	struct scatterlist rx_sg;
	struct dma_async_tx_descriptor *tx_desc;
	struct dma_async_tx_descriptor *rx_desc;
	u32 modecfg, chcfg, chon;

	int  chan;
	int status = 0;


	spi_fifo_clear(spi);
	modecfg = __raw_readl(base + SPI_FIFO_CFG);
	SPI_FIFO_CFG_RX_DMA(modecfg, 0);
	chcfg = __raw_readl(base + SPI_CH_CFG);
	memset(&slave_config, 0, sizeof(slave_config));

	chon = 0;

	/*rx setup*/
	spi->state |= RXBUSY;
	SPI_FIFO_CFG_RX_DMA(modecfg, 1);
	SPI_CH_ON_RX_CH(chon, 1);

	slave_config.direction = DMA_FROM_DEVICE;
	slave_config.src_addr = spi->sfr_start + SPI_RX_DATA;
	slave_config.dst_addr = spi->rx_dma;
	slave_config.src_addr_width = spi->cur_bpw / 8;
	dmaengine_slave_config(spi->rx_chan, &slave_config);

	sg_init_table(&rx_sg, 1);
	sg_set_page(&rx_sg, pfn_to_page(PFN_DOWN(spi->rx_dma)), spi_data->data_len,
			offset_in_page(spi->rx_dma));
	sg_dma_len(&rx_sg) = spi_data->data_len;
	sg_dma_address(&rx_sg) = spi->rx_dma;

	rx_desc = spi->rx_chan->device->device_prep_slave_sg(spi->rx_chan, &rx_sg,
			1, DMA_FROM_DEVICE, DMA_PREP_INTERRUPT, NULL);
	rx_desc->callback = drime4_spi_slave_read_dma_irq;
	rx_desc->callback_param = spi;
	dmaengine_submit(rx_desc);
	dma_async_issue_pending(spi->rx_chan);

	__raw_writel(modecfg, base + SPI_FIFO_CFG);
	__raw_writel(chcfg, base + SPI_CH_CFG);
	__raw_writel(chon, base + SPI_CH_ON);


	//gpio_direction_output(GPIO_SLAVE_RB, 1);
	wait_for_completion(&spi->done);

/*	if (status) {
		dev_err(&spi->pdev->dev, "DMA Tranfer Fail: "
			"rx-fifo-lvl%d tx-fifo-lvl%d\n",
				SPI_STATUS_RX_FIFO_LVL(__raw_readl(base + SPI_STATUS)),
				SPI_STATUS_TX_FIFO_LVL(__raw_readl(base + SPI_STATUS)));
		if (spi_data->wbuffer != NULL && (spi->state & TXBUSY))
			dmaengine_terminate_all(spi->tx_chan);
		if (spi_data->rbuffer != NULL && (spi->state & RXBUSY))
			dmaengine_terminate_all(spi->rx_chan);

	}*/

	__raw_writel(SPI_PENDING_ALL_CLEAR, base + SPI_PENDING_CLR);

	modecfg = __raw_readl(base + SPI_FIFO_CFG);
	SPI_FIFO_CFG_RX_DMA(modecfg, 0);
	__raw_writel(modecfg, base + SPI_FIFO_CFG);
	__raw_writel(SPI_REG_DISABLE, base + SPI_CH_ON);
	return status;
}


static int hs_spi_dma_wr_transfer(struct hs_spi_data *spi,
		struct spi_data_info *spi_data)
{
	void __iomem *base = spi->regs;

	struct dma_slave_config slave_config;
	struct scatterlist tx_sg;
	struct scatterlist rx_sg;
	struct dma_async_tx_descriptor *tx_desc;
	struct dma_async_tx_descriptor *rx_desc;
	u32 modecfg, chcfg, chon;
#if 0
	unsigned int value;
	unsigned int addr;
#endif
	int  chan;
	int status = 0;

	spi_fifo_clear(spi);

	modecfg = __raw_readl(base + SPI_FIFO_CFG);
	SPI_FIFO_CFG_TX_DMA(modecfg, 0);
	SPI_FIFO_CFG_RX_DMA(modecfg, 0);

	chcfg = __raw_readl(base + SPI_CH_CFG);

	memset(&slave_config, 0, sizeof(slave_config));

	/*tx setup*/

	spi->state |= TXBUSY;
	chon = 0;
	SPI_CH_ON_TX_CH(chon, SPI_REG_ENABLE);
	SPI_FIFO_CFG_TX_DMA(modecfg, SPI_REG_ENABLE);

	slave_config.direction = DMA_TO_DEVICE;
	slave_config.src_addr = spi->tx_dma;
	slave_config.dst_addr = spi->sfr_start + SPI_TX_DATA;
	slave_config.dst_addr_width = spi->cur_bpw / 8;
	dmaengine_slave_config(spi->tx_chan, &slave_config);

	sg_init_table(&tx_sg, 1);
	sg_set_page(&tx_sg, pfn_to_page(PFN_DOWN(spi->tx_dma)), spi_data->data_len,
			offset_in_page(spi->tx_dma));
	sg_dma_len(&tx_sg) = spi_data->data_len;
	sg_dma_address(&tx_sg) = spi->tx_dma;
	tx_desc = spi->tx_chan->device->device_prep_slave_sg(spi->tx_chan, &tx_sg,
			1, DMA_TO_DEVICE, DMA_PREP_INTERRUPT, NULL);
	/*
	 tx_desc->callback = drime4_spi_dma_txcb;
	 tx_desc->callback_param = sdd;
	 */
	dmaengine_submit(tx_desc);
	dma_async_issue_pending(spi->tx_chan);

	/*rx setup*/
	spi->state |= RXBUSY;
	SPI_FIFO_CFG_RX_DMA(modecfg, 1);
	SPI_CH_ON_RX_CH(chon, 1);

	slave_config.direction = DMA_FROM_DEVICE;
	slave_config.src_addr = spi->sfr_start + SPI_RX_DATA;
	slave_config.dst_addr = spi->rx_dma;
	slave_config.src_addr_width = spi->cur_bpw / 8;
	dmaengine_slave_config(spi->rx_chan, &slave_config);

	sg_init_table(&rx_sg, 1);
	sg_set_page(&rx_sg, pfn_to_page(PFN_DOWN(spi->rx_dma)), spi_data->data_len,
			offset_in_page(spi->rx_dma));
	sg_dma_len(&rx_sg) = spi_data->data_len;
	sg_dma_address(&rx_sg) = spi->rx_dma;

	rx_desc = spi->rx_chan->device->device_prep_slave_sg(spi->rx_chan, &rx_sg,
			1, DMA_FROM_DEVICE, DMA_PREP_INTERRUPT, NULL);
	rx_desc->callback = drime4_spi_dma_irq;
	rx_desc->callback_param = spi;
	dmaengine_submit(rx_desc);
	dma_async_issue_pending(spi->rx_chan);

	__raw_writel(modecfg, base + SPI_FIFO_CFG);
	__raw_writel(chcfg, base + SPI_CH_CFG);
	__raw_writel(chon, base + SPI_CH_ON);

	status = wait_for_dma(spi);

	if (status) {
		dev_err(&spi->pdev->dev, "DMA Tranfer Fail: "
			"rx-fifo-lvl%d tx-fifo-lvl%d\n",
				SPI_STATUS_RX_FIFO_LVL(__raw_readl(base + SPI_STATUS)),
				SPI_STATUS_TX_FIFO_LVL(__raw_readl(base + SPI_STATUS)));
		if (spi_data->wbuffer != NULL && (spi->state & TXBUSY))
			dmaengine_terminate_all(spi->tx_chan);
		if (spi_data->rbuffer != NULL && (spi->state & RXBUSY))
			dmaengine_terminate_all(spi->rx_chan);

	}
#if 0
		chan = dma_get_chan(spi->tx_chan);
		addr = dam_get_addr(spi->tx_chan);

		value = __raw_readl(addr + 0x20);
		value &= ~(1<<chan);
	__raw_writel(value, addr + 0x20);


	chan = dma_get_chan(spi->rx_chan);
	addr = dam_get_addr(spi->rx_chan);

	value = __raw_readl(addr + 0x20);
	value &= ~(1<<chan);
__raw_writel(value, addr + 0x20);
#endif

	__raw_writel(SPI_PENDING_ALL_CLEAR, base + SPI_PENDING_CLR);

	modecfg = __raw_readl(base + SPI_FIFO_CFG);
	SPI_FIFO_CFG_TX_DMA(modecfg, 0);
	SPI_FIFO_CFG_RX_DMA(modecfg, 0);
	__raw_writel(modecfg, base + SPI_FIFO_CFG);
	__raw_writel(SPI_REG_DISABLE, base + SPI_CH_ON);
	return status;
}

void hs_spi_slave_set(struct hs_spi_data *spi)
{
	unsigned int val;
	void __iomem *base = spi->regs;

	struct d4_hs_spi_config *ch = spi;
	val = __raw_readl(base + SPI_CH_CFG);
	SPI_CH_CFG_MST_SLV(val, 1);
	__raw_writel(val, base + SPI_CH_CFG);

	hs_spi_pad_reset(spi);

}

static void hs_spi_unmap_mssg(struct hs_spi_data *spi,
		struct spi_data_info *spi_data)
{
	struct device *dev = &spi->pdev->dev;

	if (spi_data->rbuffer != NULL && spi->rx_dma != XFER_DMAADDR_INVALID)
		dma_unmap_single(dev, spi->rx_dma, spi_data->data_len, DMA_FROM_DEVICE);

	if (spi_data->wbuffer != NULL && spi->tx_dma != XFER_DMAADDR_INVALID)
		dma_unmap_single(dev, spi->tx_dma, spi_data->data_len, DMA_TO_DEVICE);

}

static void hs_release_dma(struct hs_spi_data *spi)
{
	if (spi->state & TXBUSY || spi->state & RXBUSY)
		terminate_dma(spi);
	if (spi->tx_chan)
		dma_release_channel(spi->tx_chan);
	if (spi->rx_chan)
		dma_release_channel(spi->rx_chan);
}


static void hs_spi_unmap_slave_mssg(struct hs_spi_data *spi,
		struct spi_data_info *spi_data)
{
	struct device *dev = &spi->pdev->dev;

	if (spi_data->rbuffer != NULL && spi->rx_dma != XFER_DMAADDR_INVALID)
		dma_unmap_single(dev, spi->rx_dma, spi_data->data_len, DMA_FROM_DEVICE);
}

static void hs_release_slave_dma(struct hs_spi_data *spi)
{
	struct dma_chan *rxchan = spi->rx_chan;

	if (spi->state & RXBUSY)
		dmaengine_terminate_all(rxchan);

	if (spi->rx_chan)
		dma_release_channel(spi->rx_chan);
}




#if 0
static int hs_acquire_wdma(struct hs_spi_data *spi)
{
	dma_cap_mask_t mask;

	 Tru to acquire a generic DMA engine slave channel
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);

	 * We need both RX and TX channels to do DMA, else do none
	 * of them.


	spi->tx_chan
			= dma_request_channel(mask, pl330_filter, (void *) spi->tx_dmach);
	if (!spi->tx_chan) {
		dev_err(&spi->pdev->dev, "no TX DMA channel!\n");
		goto err_no_txchan;
	}

	return 0;

err_no_txchan:
	dma_release_channel(spi->tx_chan);
	return -ENODEV;
}


static int hs_spi_dma_w_transfer(struct hs_spi_data *spi,
		struct spi_data_info *spi_data, unsigned int dest)
{
	void __iomem *base = spi->regs;

	struct dma_slave_config slave_config;
	struct scatterlist tx_sg;
	struct scatterlist rx_sg;
	struct dma_async_tx_descriptor *tx_desc;
	struct dma_async_tx_descriptor *rx_desc;
	u32 modecfg, chcfg, chon;
	int status = 0;

	spi_fifo_clear(spi);

	modecfg = __raw_readl(base + SPI_FIFO_CFG);
	SPI_FIFO_CFG_TX_DMA(modecfg, 0);
	SPI_FIFO_CFG_RX_DMA(modecfg, 0);

	chcfg = __raw_readl(base + SPI_CH_CFG);

	memset(&slave_config, 0, sizeof(slave_config));

	tx setup

	spi->state |= TXBUSY;
	chon = 0;
	SPI_CH_ON_TX_CH(chon, SPI_REG_ENABLE);
	SPI_FIFO_CFG_TX_DMA(modecfg, SPI_REG_ENABLE);

	slave_config.direction = DMA_TO_DEVICE;
	slave_config.src_addr = spi->tx_dma;
	slave_config.dst_addr = dest;
	slave_config.dst_addr_width = 4;
	dmaengine_slave_config(spi->tx_chan, &slave_config);
#if 1
	sg_init_table(&tx_sg, 1);
	sg_set_page(&tx_sg, pfn_to_page(PFN_DOWN(spi->tx_dma)), spi_data->data_len,
			offset_in_page(spi->tx_dma));
	sg_dma_len(&tx_sg) = spi_data->data_len;
	sg_dma_address(&tx_sg) = spi->tx_dma;
	tx_desc = spi->tx_chan->device->device_prep_slave_sg(spi->tx_chan, &tx_sg,
			1, DMA_TO_DEVICE, DMA_PREP_INTERRUPT, NULL);
#else

	tx_desc = dmaengine_prep_dma_cyclic(spi->tx_chan,
			spi->tx_dma, 12, 12, DMA_TO_DEVICE);
#endif

	 tx_desc->callback = drime4_spi_dma_irq;
	 tx_desc->callback_param = spi;

	dmaengine_submit(tx_desc);
	dma_async_issue_pending(spi->tx_chan);



	__raw_writel(modecfg, base + SPI_FIFO_CFG);
	__raw_writel(chcfg, base + SPI_CH_CFG);
	__raw_writel(chon, base + SPI_CH_ON);

	udelay(2);
	d4_pmu_isoen_set(PMU_CODEC, PMU_CTRL_OFF);


	if (!wait_for_completion_timeout(&spi->done,
				msecs_to_jiffies(msecs_to_loops(2000)))) {
			dev_err(&spi->pdev->dev, "dma waiting error\n");
			status = -EIO;
		}


	if (status) {
		dev_err(&spi->pdev->dev, "DMA Tranfer Fail: "
			"rx-fifo-lvl%d tx-fifo-lvl%d\n",
				SPI_STATUS_RX_FIFO_LVL(__raw_readl(base + SPI_STATUS)),
				SPI_STATUS_TX_FIFO_LVL(__raw_readl(base + SPI_STATUS)));
		if (spi_data->wbuffer != NULL && (spi->state & TXBUSY))
			dmaengine_terminate_all(spi->tx_chan);
	}
	__raw_writel(SPI_PENDING_ALL_CLEAR, base + SPI_PENDING_CLR);

	modecfg = __raw_readl(base + SPI_FIFO_CFG);
	SPI_FIFO_CFG_TX_DMA(modecfg, 0);
	SPI_FIFO_CFG_RX_DMA(modecfg, 0);
	__raw_writel(modecfg, base + SPI_FIFO_CFG);
	__raw_writel(SPI_REG_DISABLE, base + SPI_CH_ON);
	return status;
}


int hs_spi_dma_transfer_pmu(struct hs_spi_data *spi, struct spi_data_info *spi_data, unsigned int dest)
{
	int status = 0;
	int reval;

	unsigned char *temp = NULL;
	unsigned char data_size;
	struct device *dev = &spi->pdev->dev;
	void __iomem *base = spi->regs;

	__raw_writel(0x32, base + SPI_CLK_PS);
	__raw_writel(0xA00000, base + SPI_FIFO_CFG);
	__raw_writel(0x2, base + SPI_SIG_CTRL);
	__raw_writel(0x20, base + SPI_CH_CFG);


	spi->tx_dma = dma_map_single(dev, (void *) spi_data->wbuffer,
				spi_data->data_len, DMA_TO_DEVICE);

	__raw_writel(0x00, base + SPI_CH_CFG);

	hs_acquire_wdma(spi);


	reval = hs_spi_dma_w_transfer(spi, spi_data, dest);

	status = reval;
	dma_unmap_single(dev, spi->tx_dma, spi_data->data_len, DMA_TO_DEVICE);

	dma_release_channel(spi->tx_chan);

	return status;
}
EXPORT_SYMBOL(hs_spi_dma_transfer_pmu);
#endif



/**
 * @brief  SPI의 DMA를 활용하여 data를 write/read 하기위해 사용되는 함수
 * @fn     int hs_spi_dma_transfer(struct 나hs_spi_data *spi, struct spi_data_info *spi_data)
 * @param  *spi				:[in]사용 할 SPI의 정보를 담고있는 포인터
 * @param  *spi_data	:[in]사용 할 SPI channel을 통해 Write되거나 Read될 Data의 정보를 담고 있는 포인터
 * @return int					:[out] 함수 동작의 상태정보 ( 0: SUCCESS)
 * @author kyuchun.han
 * @note
 */
int hs_spi_dma_transfer(struct hs_spi_data *spi, struct spi_data_info *spi_data)
{
	int status = 0;
	int reval;
	unsigned char *temp = NULL;
	unsigned char data_size;
	data_size = (spi_data->data_len * spi->cur_bpw) / 8;

	temp = kzalloc(data_size, GFP_KERNEL);
	if (temp == NULL) {
		status = -ENOMEM;
		return status;
	}

	if (spi_data->wbuffer != NULL) {
		memset(temp, 0xFF, data_size);
		spi_data->rbuffer = temp;
	} else {
		memset(temp, 0xFF, data_size);
		spi_data->wbuffer = temp;
	}

	if (hs_spi_map_mssg(spi, spi_data)) {
		dev_err(&spi->pdev->dev, "Xfer: Unable to map message buffers!\n");
		status = -ENOMEM;
		return status;
	}

	if (hs_acquire_dma(spi)) {
		dev_err(&spi->pdev->dev, "Unable to dma channel\n");
		status = -ENOMEM;
		return status;
	}

	reval = hs_spi_dma_wr_transfer(spi, spi_data);


	hs_spi_unmap_mssg(spi, spi_data);
	hs_release_dma(spi);

	if (temp != NULL)
		kfree(temp);

	if (reval != 0) {
		status = -ENOMEM;
		dev_err(&spi->pdev->dev, "dma channel transfer fail\n");
	}

	return status;
}
EXPORT_SYMBOL(hs_spi_dma_transfer);





int hs_spi_slave_interrupt_burst_dma_read(struct hs_spi_data *spi,
		struct spi_data_info *spi_data)
{
	int status = 0;
	int reval;
	unsigned char *temp = NULL;
	unsigned char data_size;
	data_size = (spi_data->data_len * spi->cur_bpw) / 8;



/*
	temp = kzalloc(data_size, GFP_KERNEL);
	if (temp == NULL) {
		status = -ENOMEM;
		return status;
	}

	spi_data->rbuffer = temp;
*/

/*
	if (hs_spi_map_slave_mssg(spi, spi_data)) {
		dev_err(&spi->pdev->dev, "Xfer: Unable to map message buffers!\n");
		status = -ENOMEM;
		return status;
	}
*/

	spi->rx_dma = spi_data->rbuffer;
	if (hs_acquire_slave_dma(spi)) {
		dev_err(&spi->pdev->dev, "Unable to dma channel\n");
		status = -ENOMEM;
		return status;
	}

	reval = hs_spi_dma_slave_read_transfer(spi, spi_data);


	hs_spi_unmap_slave_mssg(spi, spi_data);
	hs_release_slave_dma(spi);


	if (reval != 0) {
		status = -ENOMEM;
		dev_err(&spi->pdev->dev, "dma channel transfer fail\n");
	}

	return status;
}

EXPORT_SYMBOL(hs_spi_slave_interrupt_burst_dma_read);




static int hs_spi_check_done(void *dev)
{

	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;

	unsigned int chcfg, val;
	int rtval;

	/*test*/
/*

	if (spi->read_len == spi->n_bytes) {
		val = __raw_readl(base + SPI_PENDING_CLR);
		SPI_PEND_CLR_TX_DONE_CLR(val, SPI_REG_ENABLE);
		__raw_writel(val, base + SPI_PENDING_CLR);
		return 0;
	}
*/
	rtval = 0;
	chcfg = __raw_readl(base + SPI_PENDING_CLR);

	chcfg = chcfg & 0x3;

	if (chcfg)
		return -1;

	if (spi->read_len == spi->n_bytes) {
		chcfg = __raw_readl(base + SPI_STATUS);
		chcfg = SPI_STATUS_RX_FIFO_LVL(chcfg);

		if (chcfg == 0) {
			val = __raw_readl(base + SPI_PENDING_CLR);
			SPI_PEND_CLR_TX_DONE_CLR(val, SPI_REG_ENABLE);
			__raw_writel(val, base + SPI_PENDING_CLR);
			rtval = 0;
		} else
			rtval = -1;
	} else
		rtval = -1;

	return rtval;
}

static irqreturn_t hs_spi_irq(int irq, void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;

	spi->int_handler(spi);

	if (hs_spi_check_done(spi) == 0) {
		__raw_writel(SPI_PENDING_ALL_CLEAR, base + SPI_PENDING_CLR);
		__raw_writel(SPI_REG_DISABLE, base + SPI_INT_EN);
		complete(&spi->done);
	} else {
		if (spi->read_len > spi->n_bytes) {
				printk("spi read len:%d\n", spi->read_len);
		}
	}

	return IRQ_HANDLED;
}

static int hs_spi_register(struct hs_spi_data *data)
{
	mutex_lock(&spi_lock);
	list_add_tail(&data->list, &spi_list);
	mutex_unlock(&spi_lock);
	return 0;
}

static int hs_spi_unregister(struct hs_spi_data *data)
{
	mutex_lock(&spi_lock);
	list_del(&data->list);
	mutex_unlock(&spi_lock);
	return 0;
}


static void hs_spi_hw_init(void *dev)
{
	struct hs_spi_data *spi = dev;
	void __iomem *base = spi->regs;
	__raw_writel(0x20, base + SPI_CH_CFG);
	__raw_writel(0x00, base + SPI_CH_CFG);
}

static int __init hs_spi_probe(struct platform_device *pdev)
{
	struct resource *mem_res, *dmatx_res, *dmarx_res;
	struct hs_spi_data *sdata;
	int ret;
	unsigned long pin_config;

	if (pdev->id < 0) {
		dev_err(&pdev->dev,
				"Invalid platform device id-%d\n", pdev->id);
		return -ENODEV;
	}

	if (pdev->dev.platform_data == NULL) {
		dev_err(&pdev->dev, "platform_data missing!\n");
		return -ENODEV;
	}

	sdata = devm_kzalloc(&pdev->dev, sizeof(struct hs_spi_data), GFP_KERNEL);
	if (sdata == NULL) {
		dev_err(&pdev->dev, "failed to allocate ht_pwm_device\n");
		return -ENOMEM;
	}

	sdata->pdev = pdev;
	sdata->spi_id = pdev->id;

	/* Check for availability of necessary resource */

	dmatx_res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	if (dmatx_res == NULL) {
		dev_err(&pdev->dev, "Unable to get SPI-Tx dma resource\n");
		ret = -ENXIO;
		goto err_free;
	}

	dmarx_res = platform_get_resource(pdev, IORESOURCE_DMA, 1);
	if (dmarx_res == NULL) {
		dev_err(&pdev->dev, "Unable to get SPI-Rx dma resource\n");
		ret = -ENXIO;
		goto err_free;
	}

	sdata->tx_dmach = dmatx_res->start;
	sdata->rx_dmach = dmarx_res->start;

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem_res == NULL) {
		dev_err(&pdev->dev, "Unable to get SPI MEM resource\n");
		ret = -ENXIO;
		goto err_free;
	}
	sdata->sfr_start = mem_res->start;
	sdata->sfr_size =  (unsigned int) resource_size(mem_res);

	sdata->clk = clk_get(&pdev->dev, "spi");

	if (sdata->clk == -2) {
		ret = -ENXIO;
		goto err_free;
	}

	clk_enable(sdata->clk);

/*

	if (pdev->id == 7) {
		pin_config = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
				PIN_CONFIG_DRIVE_STRENGTH_X1);
		ret = pin_config_group_set("drime4-pinmux", "spi1grp", pin_config);

		pin_config = to_config_packed(PIN_CONFIG_PULLUP_DOWN_SEL,
				PIN_CONFIG_PULLUP_SEL);
				ret = pin_config_group_set("drime4-pinmux", "spi1grp", pin_config);

		pin_config = to_config_packed(PIN_CONFIG_PULLUP_DOWN,
				PIN_CONFIG_PULLUP_DOWN_ENABLE);
		ret = pin_config_group_set("drime4-pinmux", "spi1grp", pin_config);
	}
*/

	sdata->pmx = devm_pinctrl_get(&pdev->dev);
	sdata->pins_default = pinctrl_lookup_state(sdata->pmx, PINCTRL_STATE_DEFAULT);
	pinctrl_select_state(sdata->pmx, sdata->pins_default);

	if (request_mem_region(mem_res->start, resource_size(mem_res),
					pdev->name) == NULL) {
		dev_err(&pdev->dev, "Req mem region failed\n");
		ret = -ENXIO;
		goto err_free_clk;
	}

	sdata->regs = ioremap(mem_res->start, resource_size(mem_res));
	if (sdata->regs == NULL) {
		dev_err(&pdev->dev, "Unable to remap IO\n");
		ret = -ENXIO;
		goto err_ioremap;
	}

	init_completion(&sdata->done);

	/* reset */
	hs_spi_hw_init(sdata);

	sdata->irq = platform_get_irq(pdev, 0);
	if (sdata->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		ret = -ENOENT;
		goto err_ioremap;
	}

	ret = request_irq(sdata->irq, hs_spi_irq, 0, pdev->name, sdata);
	if (ret) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_irq;
	}

	ret = hs_spi_register(sdata);
	if (ret) {
		dev_err(&pdev->dev, "failed to register spi\n");
		goto err_irq;
	}
	ret = gpio_request(port_num[sdata->spi_id].port_num_clk, "");
	if (ret) {
			dev_err(&pdev->dev, "failed to gpio request clk\n");
			goto err_irq;
		}
	ret = gpio_request(port_num[sdata->spi_id].port_num_cs, "");
	if (ret) {
			dev_err(&pdev->dev, "failed to gpio request cs\n");
			goto err_irq;
		}
	platform_set_drvdata(pdev, sdata);
	return ret;

err_irq:
				iounmap((void *) sdata->regs);

err_ioremap:
				release_mem_region(mem_res->start, resource_size(mem_res));

err_free_clk:
				clk_put(sdata->clk);
				devm_pinctrl_put(sdata->pmx);
err_free:
	    devm_kfree(&pdev->dev, sdata);
	    return ret;
}

static int hs_spi_remove(struct platform_device *pdev)
{
	struct hs_spi_data *sdata = platform_get_drvdata(pdev);
	struct resource *mem_res;
	unsigned long flags;

	if (sdata == NULL)
		return -1;

	spin_lock_irqsave(&sdata->lock, flags);

	spin_unlock_irqrestore(&sdata->lock, flags);

	clk_disable(sdata->clk);
	clk_put(sdata->clk);

	iounmap((void *) sdata->regs);

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem_res != NULL)
		release_mem_region(mem_res->start, resource_size(mem_res));

	platform_set_drvdata(pdev, NULL);
	hs_spi_unregister(sdata);
	devm_kfree(&pdev->dev, sdata);
	return 0;
}

#ifdef CONFIG_PM
static int hs_spi_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct hs_spi_data *sdata = platform_get_drvdata(pdev);
	unsigned long flags;

	if (sdata == NULL)
		return -1;


	spin_lock_irqsave(&sdata->lock, flags);
	spin_unlock_irqrestore(&sdata->lock, flags);

	/* Disable the clock */
	clk_disable(sdata->clk);

#if 0
	gpio_free(port_num[sdata->spi_id].port_num_clk);
	gpio_free(port_num[sdata->spi_id].port_num_cs);
#endif	
	return 0;
}

static int hs_spi_resume(struct platform_device *pdev)
{
	unsigned long flags;
	struct hs_spi_data *sdata;
	int ret;

	sdata = platform_get_drvdata(pdev);

	if (sdata == NULL)
		return -1;

	clk_enable(sdata->clk);
	spin_lock_irqsave(&sdata->lock, flags);
	spin_unlock_irqrestore(&sdata->lock, flags);

#if 0
	ret = gpio_request(port_num[sdata->spi_id].port_num_clk, "");
	if (ret) {
			dev_err(&pdev->dev, "failed to gpio request clk\n");
			return -1;
		}
	ret = gpio_request(port_num[sdata->spi_id].port_num_cs, "");
	if (ret) {
			dev_err(&pdev->dev, "failed to gpio request cs\n");
			return -1;
		}
#endif

	return 0;
}
#else
#define hs_spi_suspend	NULL
#define hs_spi_resume	NULL
#endif /* CONFIG_PM */

static struct platform_driver
		hs_spi_driver = { .driver = {
			.owner = THIS_MODULE,
			.name = SPI_MODULE_NAME,
		}, .remove = hs_spi_remove, .suspend = hs_spi_suspend,
				.resume = hs_spi_resume, };
MODULE_ALIAS("platform:drime4-spi");

static int __init hs_spi_init(void)
{
	return platform_driver_probe(&hs_spi_driver, hs_spi_probe);
}

#ifndef CONFIG_SCORE_FAST_RESUME
subsys_initcall(hs_spi_init);
#else
fast_subsys_initcall(hs_spi_init);
#endif

static void __exit hs_spi_exit(void)
{
	platform_driver_unregister(&hs_spi_driver);
}
module_exit(hs_spi_exit);

MODULE_AUTHOR("kyuchun han, <kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("drime4 SPI Controller Driver");
MODULE_LICENSE("GPL");

