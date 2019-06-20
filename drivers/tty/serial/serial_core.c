/*
 *  Driver core for serial ports
 *
 *  Based on drivers/char/serial.c, by Linus Torvalds, Theodore Ts'o.
 *
 *  Copyright 1999 ARM Limited
 *  Copyright (C) 2000-2001 Deep Blue Solutions Ltd.
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
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <linux/serial.h> /* for serial_state and serial_icounter_struct */
#include <linux/serial_core.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/syscalls.h>

#include <asm/irq.h>
#include <asm/uaccess.h>
#include <linux/io.h>
#include <mach/map.h>
#include <mach/watchdog-reset.h>


/*
 * This routine is used by the interrupt handler to schedule processing in
 * the software interrupt portion of the driver.
 */
#define _USE_RELEASE_LOG_

void uart_save_log_to_card(int saveType , char *func);

#ifdef _USE_RELEASE_LOG_
#define DEBUG_MODE		0
#define RELEASE_MODE	1
#define LOG_BUF_SIZE	(128 * 1024)

static int uart_mode = DEBUG_MODE;

static unsigned long r_log_buf_size;
unsigned char *r_log_buf;
static unsigned long r_log_idx;
static int r_log_overall;
static int isKeyLogSave;
unsigned long is_poweroff_status;
unsigned long is_poweroff_halt_status;

struct work_struct log_queue;

static void make_log_file(struct work_struct *work)
{
	uart_save_log_to_card(0, (char *)__FUNCTION__);

	printk("!!!!!!!!!!!!!!!!!!!!!!!!\n log save end\n!!!!!!!!!!!!!!!!!!!!!!!1\n");
}
#endif

/*
 * This is used to lock changes in serial line configuration.
 */
static DEFINE_MUTEX(port_mutex);

/*
 * lockdep: port->lock is initialized in two places, but we
 *          want only one lock-class:
 */
static struct lock_class_key port_lock_key;

#define HIGH_BITS_OFFSET	((sizeof(long)-sizeof(int))*8)

#ifdef CONFIG_SERIAL_CORE_CONSOLE
#define uart_console(port)	((port)->cons && (port)->cons->index == (port)->line)
#else
#define uart_console(port)	(0)
#endif

static void uart_change_speed(struct tty_struct *tty, struct uart_state *state,
					struct ktermios *old_termios);
static void uart_wait_until_sent(struct tty_struct *tty, int timeout);
static void uart_change_pm(struct uart_state *state, int pm_state);

static void uart_port_shutdown(struct tty_port *port);

/*
 * This routine is used by the interrupt handler to schedule processing in
 * the software interrupt portion of the driver.
 */
void uart_write_wakeup(struct uart_port *port)
{
	struct uart_state *state = port->state;
	/*
	 * This means you called this function _after_ the port was
	 * closed.  No cookie for you.
	 */
	BUG_ON(!state);
	tty_wakeup(state->port.tty);
}

static void uart_stop(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port = state->uart_port;
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);
	port->ops->stop_tx(port);
	spin_unlock_irqrestore(&port->lock, flags);
}

static void __uart_start(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port = state->uart_port;

	if (!uart_circ_empty(&state->xmit) && state->xmit.buf &&
	    !tty->stopped && !tty->hw_stopped)
		port->ops->start_tx(port);
}

static void uart_start(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port = state->uart_port;
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);
	__uart_start(tty);
	spin_unlock_irqrestore(&port->lock, flags);
}

static inline void
uart_update_mctrl(struct uart_port *port, unsigned int set, unsigned int clear)
{
	unsigned long flags;
	unsigned int old;

	spin_lock_irqsave(&port->lock, flags);
	old = port->mctrl;
	port->mctrl = (old & ~clear) | set;
	if (old != port->mctrl)
		port->ops->set_mctrl(port, port->mctrl);
	spin_unlock_irqrestore(&port->lock, flags);
}

#define uart_set_mctrl(port, set)	uart_update_mctrl(port, set, 0)
#define uart_clear_mctrl(port, clear)	uart_update_mctrl(port, 0, clear)

/*
 * Startup the port.  This will be called once per open.  All calls
 * will be serialised by the per-port mutex.
 */
static int uart_port_startup(struct tty_struct *tty, struct uart_state *state,
		int init_hw)
{
	struct uart_port *uport = state->uart_port;
	struct tty_port *port = &state->port;
	unsigned long page;
	int retval = 0;

	if (uport->type == PORT_UNKNOWN)
		return 1;

	/*
	 * Initialise and allocate the transmit and temporary
	 * buffer.
	 */
	if (!state->xmit.buf) {
		/* This is protected by the per port mutex */
		page = get_zeroed_page(GFP_KERNEL);
		if (!page)
			return -ENOMEM;

		state->xmit.buf = (unsigned char *) page;
		uart_circ_clear(&state->xmit);
	}

	retval = uport->ops->startup(uport);
	if (retval == 0) {
		if (uart_console(uport) && uport->cons->cflag) {
			tty->termios->c_cflag = uport->cons->cflag;
			uport->cons->cflag = 0;
		}
		/*
		 * Initialise the hardware port settings.
		 */
		uart_change_speed(tty, state, NULL);

		if (init_hw) {
			/*
			 * Setup the RTS and DTR signals once the
			 * port is open and ready to respond.
			 */
			if (tty->termios->c_cflag & CBAUD)
				uart_set_mctrl(uport, TIOCM_RTS | TIOCM_DTR);
		}

		if (port->flags & ASYNC_CTS_FLOW) {
			spin_lock_irq(&uport->lock);
			if (!(uport->ops->get_mctrl(uport) & TIOCM_CTS))
				tty->hw_stopped = 1;
			spin_unlock_irq(&uport->lock);
		}
	}

	/*
	 * This is to allow setserial on this port. People may want to set
	 * port/irq/type and then reconfigure the port properly if it failed
	 * now.
	 */
	if (retval && capable(CAP_SYS_ADMIN))
		return 1;

	return retval;
}

static int uart_startup(struct tty_struct *tty, struct uart_state *state,
		int init_hw)
{
	struct tty_port *port = &state->port;
	int retval;

	if (port->flags & ASYNC_INITIALIZED)
		return 0;

	/*
	 * Set the TTY IO error marker - we will only clear this
	 * once we have successfully opened the port.
	 */
	set_bit(TTY_IO_ERROR, &tty->flags);

	retval = uart_port_startup(tty, state, init_hw);
	if (!retval) {
		set_bit(ASYNCB_INITIALIZED, &port->flags);
		clear_bit(TTY_IO_ERROR, &tty->flags);
	} else if (retval > 0)
		retval = 0;

	return retval;
}

/*
 * This routine will shutdown a serial port; interrupts are disabled, and
 * DTR is dropped if the hangup on close termio flag is on.  Calls to
 * uart_shutdown are serialised by the per-port semaphore.
 */
static void uart_shutdown(struct tty_struct *tty, struct uart_state *state)
{
	struct uart_port *uport = state->uart_port;
	struct tty_port *port = &state->port;

	/*
	 * Set the TTY IO error marker
	 */
	if (tty)
		set_bit(TTY_IO_ERROR, &tty->flags);

	if (test_and_clear_bit(ASYNCB_INITIALIZED, &port->flags)) {
		/*
		 * Turn off DTR and RTS early.
		 */
		if (!tty || (tty->termios->c_cflag & HUPCL))
			uart_clear_mctrl(uport, TIOCM_DTR | TIOCM_RTS);

		uart_port_shutdown(port);
	}

	/*
	 * It's possible for shutdown to be called after suspend if we get
	 * a DCD drop (hangup) at just the right time.  Clear suspended bit so
	 * we don't try to resume a port that has been shutdown.
	 */
	clear_bit(ASYNCB_SUSPENDED, &port->flags);

	/*
	 * Free the transmit buffer page.
	 */
	if (state->xmit.buf) {
		free_page((unsigned long)state->xmit.buf);
		state->xmit.buf = NULL;
	}
}

/**
 *	uart_update_timeout - update per-port FIFO timeout.
 *	@port:  uart_port structure describing the port
 *	@cflag: termios cflag value
 *	@baud:  speed of the port
 *
 *	Set the port FIFO timeout value.  The @cflag value should
 *	reflect the actual hardware settings.
 */
void
uart_update_timeout(struct uart_port *port, unsigned int cflag,
		    unsigned int baud)
{
	unsigned int bits;

	/* byte size and parity */
	switch (cflag & CSIZE) {
	case CS5:
		bits = 7;
		break;
	case CS6:
		bits = 8;
		break;
	case CS7:
		bits = 9;
		break;
	default:
		bits = 10;
		break; /* CS8 */
	}

	if (cflag & CSTOPB)
		bits++;
	if (cflag & PARENB)
		bits++;

	/*
	 * The total number of bits to be transmitted in the fifo.
	 */
	bits = bits * port->fifosize;

	/*
	 * Figure the timeout to send the above number of bits.
	 * Add .02 seconds of slop
	 */
	port->timeout = (HZ * bits) / baud + HZ/50;
}

EXPORT_SYMBOL(uart_update_timeout);

/**
 *	uart_get_baud_rate - return baud rate for a particular port
 *	@port: uart_port structure describing the port in question.
 *	@termios: desired termios settings.
 *	@old: old termios (or NULL)
 *	@min: minimum acceptable baud rate
 *	@max: maximum acceptable baud rate
 *
 *	Decode the termios structure into a numeric baud rate,
 *	taking account of the magic 38400 baud rate (with spd_*
 *	flags), and mapping the %B0 rate to 9600 baud.
 *
 *	If the new baud rate is invalid, try the old termios setting.
 *	If it's still invalid, we try 9600 baud.
 *
 *	Update the @termios structure to reflect the baud rate
 *	we're actually going to be using. Don't do this for the case
 *	where B0 is requested ("hang up").
 */
unsigned int
uart_get_baud_rate(struct uart_port *port, struct ktermios *termios,
		   struct ktermios *old, unsigned int min, unsigned int max)
{
	unsigned int try, baud, altbaud = 38400;
	int hung_up = 0;
	upf_t flags = port->flags & UPF_SPD_MASK;

	if (flags == UPF_SPD_HI)
		altbaud = 57600;
	else if (flags == UPF_SPD_VHI)
		altbaud = 115200;
	else if (flags == UPF_SPD_SHI)
		altbaud = 230400;
	else if (flags == UPF_SPD_WARP)
		altbaud = 460800;

	for (try = 0; try < 2; try++) {
		baud = tty_termios_baud_rate(termios);

		/*
		 * The spd_hi, spd_vhi, spd_shi, spd_warp kludge...
		 * Die! Die! Die!
		 */
		if (baud == 38400)
			baud = altbaud;

		/*
		 * Special case: B0 rate.
		 */
		if (baud == 0) {
			hung_up = 1;
			baud = 9600;
		}

		if (baud >= min && baud <= max)
			return baud;

		/*
		 * Oops, the quotient was zero.  Try again with
		 * the old baud rate if possible.
		 */
		termios->c_cflag &= ~CBAUD;
		if (old) {
			baud = tty_termios_baud_rate(old);
			if (!hung_up)
				tty_termios_encode_baud_rate(termios,
								baud, baud);
			old = NULL;
			continue;
		}

		/*
		 * As a last resort, if the range cannot be met then clip to
		 * the nearest chip supported rate.
		 */
		if (!hung_up) {
			if (baud <= min)
				tty_termios_encode_baud_rate(termios,
							min + 1, min + 1);
			else
				tty_termios_encode_baud_rate(termios,
							max - 1, max - 1);
		}
	}
	/* Should never happen */
	WARN_ON(1);
	return 0;
}

EXPORT_SYMBOL(uart_get_baud_rate);

/**
 *	uart_get_divisor - return uart clock divisor
 *	@port: uart_port structure describing the port.
 *	@baud: desired baud rate
 *
 *	Calculate the uart clock divisor for the port.
 */
unsigned int
uart_get_divisor(struct uart_port *port, unsigned int baud)
{
	unsigned int quot;

	/*
	 * Old custom speed handling.
	 */
	if (baud == 38400 && (port->flags & UPF_SPD_MASK) == UPF_SPD_CUST)
		quot = port->custom_divisor;
	else
		quot = DIV_ROUND_CLOSEST(port->uartclk, 16 * baud);

	return quot;
}

EXPORT_SYMBOL(uart_get_divisor);

/* FIXME: Consistent locking policy */
static void uart_change_speed(struct tty_struct *tty, struct uart_state *state,
					struct ktermios *old_termios)
{
	struct tty_port *port = &state->port;
	struct uart_port *uport = state->uart_port;
	struct ktermios *termios;

	/*
	 * If we have no tty, termios, or the port does not exist,
	 * then we can't set the parameters for this port.
	 */
	if (!tty || !tty->termios || uport->type == PORT_UNKNOWN)
		return;

	termios = tty->termios;

	/*
	 * Set flags based on termios cflag
	 */
	if (termios->c_cflag & CRTSCTS)
		set_bit(ASYNCB_CTS_FLOW, &port->flags);
	else
		clear_bit(ASYNCB_CTS_FLOW, &port->flags);

	if (termios->c_cflag & CLOCAL)
		clear_bit(ASYNCB_CHECK_CD, &port->flags);
	else
		set_bit(ASYNCB_CHECK_CD, &port->flags);

	uport->ops->set_termios(uport, termios, old_termios);
}

static inline int __uart_put_char(struct uart_port *port,
				struct circ_buf *circ, unsigned char c)
{
	unsigned long flags;
	int ret = 0;

	if (!circ->buf)
		return 0;

	spin_lock_irqsave(&port->lock, flags);
	if (uart_circ_chars_free(circ) != 0) {
		circ->buf[circ->head] = c;
		circ->head = (circ->head + 1) & (UART_XMIT_SIZE - 1);
		ret = 1;
	}
	spin_unlock_irqrestore(&port->lock, flags);
	return ret;
}

static int uart_put_char(struct tty_struct *tty, unsigned char ch)
{
	struct uart_state *state = tty->driver_data;

	return __uart_put_char(state->uart_port, &state->xmit, ch);
}

static void uart_flush_chars(struct tty_struct *tty)
{
	uart_start(tty);
}

static int uart_write(struct tty_struct *tty,
					const unsigned char *buf, int count)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	struct circ_buf *circ;
	unsigned long flags;
	int c, ret = 0;

#ifdef _USE_RELEASE_LOG_
	int r_count = 0;
	const unsigned char *r_buf = NULL;
#endif

	/*
	 * This means you called this function _after_ the port was
	 * closed.  No cookie for you.
	 */
	if (!state) {
		WARN_ON(1);
		return -EL3HLT;
	}

	port = state->uart_port;
	circ = &state->xmit;

	if (!circ->buf)
		return 0;

#ifdef _USE_RELEASE_LOG_
	if (count > 3) {
		/* is there log level */
		if (buf[0] == '<' && buf[1] == '0' && buf[2] == '>') {
			uart_mode = RELEASE_MODE;
		} else {
			uart_mode = DEBUG_MODE;
		}
	} else {
		uart_mode = DEBUG_MODE;
	}

	r_count = count;
	r_buf = buf;
	if (r_log_idx + r_count > r_log_buf_size) {
		memcpy(r_log_buf + r_log_idx, r_buf, r_log_buf_size-r_log_idx);
		r_count -= r_log_buf_size-r_log_idx;
		r_buf += r_log_buf_size-r_log_idx;
		r_log_idx = 0;
		r_log_overall = 1;
	}
	memcpy(r_log_buf + r_log_idx, r_buf, r_count);
	r_log_idx = (r_log_idx + r_count) & (r_log_buf_size - 1);

#if 0
	if (r_log_idx >= 1) {
		if (']' != r_log_buf[r_log_idx-1] || 14 != r_count) {
			r_log_buf[r_log_idx++] = '\n';
		}
	}
#endif

    if (uart_mode == RELEASE_MODE) {
		return count;
	}
#endif

	spin_lock_irqsave(&port->lock, flags);
	while (1) {
		c = CIRC_SPACE_TO_END(circ->head, circ->tail, UART_XMIT_SIZE);
		if (count < c)
			c = count;
		if (c <= 0)
			break;
		memcpy(circ->buf + circ->head, buf, c);
		circ->head = (circ->head + c) & (UART_XMIT_SIZE - 1);
		buf += c;
		count -= c;
		ret += c;
	}
	spin_unlock_irqrestore(&port->lock, flags);

	uart_start(tty);
	return ret;
}

static int uart_write_room(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&state->uart_port->lock, flags);
	ret = uart_circ_chars_free(&state->xmit);
	spin_unlock_irqrestore(&state->uart_port->lock, flags);
	return ret;
}

static int uart_chars_in_buffer(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&state->uart_port->lock, flags);
	ret = uart_circ_chars_pending(&state->xmit);
	spin_unlock_irqrestore(&state->uart_port->lock, flags);
	return ret;
}

static void uart_flush_buffer(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	unsigned long flags;

	/*
	 * This means you called this function _after_ the port was
	 * closed.  No cookie for you.
	 */
	if (!state) {
		WARN_ON(1);
		return;
	}

	port = state->uart_port;
	pr_debug("uart_flush_buffer(%d) called\n", tty->index);

	spin_lock_irqsave(&port->lock, flags);
	uart_circ_clear(&state->xmit);
	if (port->ops->flush_buffer)
		port->ops->flush_buffer(port);
	spin_unlock_irqrestore(&port->lock, flags);
	tty_wakeup(tty);
}

/*
 * This function is used to send a high-priority XON/XOFF character to
 * the device
 */
static void uart_send_xchar(struct tty_struct *tty, char ch)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port = state->uart_port;
	unsigned long flags;

	if (port->ops->send_xchar)
		port->ops->send_xchar(port, ch);
	else {
		port->x_char = ch;
		if (ch) {
			spin_lock_irqsave(&port->lock, flags);
			port->ops->start_tx(port);
			spin_unlock_irqrestore(&port->lock, flags);
		}
	}
}

static void uart_throttle(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;

	if (I_IXOFF(tty))
		uart_send_xchar(tty, STOP_CHAR(tty));

	if (tty->termios->c_cflag & CRTSCTS)
		uart_clear_mctrl(state->uart_port, TIOCM_RTS);
}

static void uart_unthrottle(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port = state->uart_port;

	if (I_IXOFF(tty)) {
		if (port->x_char)
			port->x_char = 0;
		else
			uart_send_xchar(tty, START_CHAR(tty));
	}

	if (tty->termios->c_cflag & CRTSCTS)
		uart_set_mctrl(port, TIOCM_RTS);
}

static int uart_get_info(struct uart_state *state,
			 struct serial_struct __user *retinfo)
{
	struct uart_port *uport = state->uart_port;
	struct tty_port *port = &state->port;
	struct serial_struct tmp;

	memset(&tmp, 0, sizeof(tmp));

	/* Ensure the state we copy is consistent and no hardware changes
	   occur as we go */
	mutex_lock(&port->mutex);

	tmp.type	    = uport->type;
	tmp.line	    = uport->line;
	tmp.port	    = uport->iobase;
	if (HIGH_BITS_OFFSET)
		tmp.port_high = (long) uport->iobase >> HIGH_BITS_OFFSET;
	tmp.irq		    = uport->irq;
	tmp.flags	    = uport->flags;
	tmp.xmit_fifo_size  = uport->fifosize;
	tmp.baud_base	    = uport->uartclk / 16;
	tmp.close_delay	    = jiffies_to_msecs(port->close_delay) / 10;
	tmp.closing_wait    = port->closing_wait == ASYNC_CLOSING_WAIT_NONE ?
				ASYNC_CLOSING_WAIT_NONE :
				jiffies_to_msecs(port->closing_wait) / 10;
	tmp.custom_divisor  = uport->custom_divisor;
	tmp.hub6	    = uport->hub6;
	tmp.io_type         = uport->iotype;
	tmp.iomem_reg_shift = uport->regshift;
	tmp.iomem_base      = (void *)(unsigned long)uport->mapbase;

	mutex_unlock(&port->mutex);

	if (copy_to_user(retinfo, &tmp, sizeof(*retinfo)))
		return -EFAULT;
	return 0;
}

static int uart_set_info(struct tty_struct *tty, struct uart_state *state,
			 struct serial_struct __user *newinfo)
{
	struct serial_struct new_serial;
	struct uart_port *uport = state->uart_port;
	struct tty_port *port = &state->port;
	unsigned long new_port;
	unsigned int change_irq, change_port, closing_wait;
	unsigned int old_custom_divisor, close_delay;
	upf_t old_flags, new_flags;
	int retval = 0;

	if (copy_from_user(&new_serial, newinfo, sizeof(new_serial)))
		return -EFAULT;

	new_port = new_serial.port;
	if (HIGH_BITS_OFFSET)
		new_port += (unsigned long) new_serial.port_high << HIGH_BITS_OFFSET;

	new_serial.irq = irq_canonicalize(new_serial.irq);
	close_delay = msecs_to_jiffies(new_serial.close_delay * 10);
	closing_wait = new_serial.closing_wait == ASYNC_CLOSING_WAIT_NONE ?
			ASYNC_CLOSING_WAIT_NONE :
			msecs_to_jiffies(new_serial.closing_wait * 10);

	/*
	 * This semaphore protects port->count.  It is also
	 * very useful to prevent opens.  Also, take the
	 * port configuration semaphore to make sure that a
	 * module insertion/removal doesn't change anything
	 * under us.
	 */
	mutex_lock(&port->mutex);

	change_irq  = !(uport->flags & UPF_FIXED_PORT)
		&& new_serial.irq != uport->irq;

	/*
	 * Since changing the 'type' of the port changes its resource
	 * allocations, we should treat type changes the same as
	 * IO port changes.
	 */
	change_port = !(uport->flags & UPF_FIXED_PORT)
		&& (new_port != uport->iobase ||
		    (unsigned long)new_serial.iomem_base != uport->mapbase ||
		    new_serial.hub6 != uport->hub6 ||
		    new_serial.io_type != uport->iotype ||
		    new_serial.iomem_reg_shift != uport->regshift ||
		    new_serial.type != uport->type);

	old_flags = uport->flags;
	new_flags = new_serial.flags;
	old_custom_divisor = uport->custom_divisor;

	if (!capable(CAP_SYS_ADMIN)) {
		retval = -EPERM;
		if (change_irq || change_port ||
		    (new_serial.baud_base != uport->uartclk / 16) ||
		    (close_delay != port->close_delay) ||
		    (closing_wait != port->closing_wait) ||
		    (new_serial.xmit_fifo_size &&
		     new_serial.xmit_fifo_size != uport->fifosize) ||
		    (((new_flags ^ old_flags) & ~UPF_USR_MASK) != 0))
			goto exit;
		uport->flags = ((uport->flags & ~UPF_USR_MASK) |
			       (new_flags & UPF_USR_MASK));
		uport->custom_divisor = new_serial.custom_divisor;
		goto check_and_exit;
	}

	/*
	 * Ask the low level driver to verify the settings.
	 */
	if (uport->ops->verify_port)
		retval = uport->ops->verify_port(uport, &new_serial);

	if ((new_serial.irq >= nr_irqs) || (new_serial.irq < 0) ||
	    (new_serial.baud_base < 9600))
		retval = -EINVAL;

	if (retval)
		goto exit;

	if (change_port || change_irq) {
		retval = -EBUSY;

		/*
		 * Make sure that we are the sole user of this port.
		 */
		if (tty_port_users(port) > 1)
			goto exit;

		/*
		 * We need to shutdown the serial port at the old
		 * port/type/irq combination.
		 */
		uart_shutdown(tty, state);
	}

	if (change_port) {
		unsigned long old_iobase, old_mapbase;
		unsigned int old_type, old_iotype, old_hub6, old_shift;

		old_iobase = uport->iobase;
		old_mapbase = uport->mapbase;
		old_type = uport->type;
		old_hub6 = uport->hub6;
		old_iotype = uport->iotype;
		old_shift = uport->regshift;

		/*
		 * Free and release old regions
		 */
		if (old_type != PORT_UNKNOWN)
			uport->ops->release_port(uport);

		uport->iobase = new_port;
		uport->type = new_serial.type;
		uport->hub6 = new_serial.hub6;
		uport->iotype = new_serial.io_type;
		uport->regshift = new_serial.iomem_reg_shift;
		uport->mapbase = (unsigned long)new_serial.iomem_base;

		/*
		 * Claim and map the new regions
		 */
		if (uport->type != PORT_UNKNOWN) {
			retval = uport->ops->request_port(uport);
		} else {
			/* Always success - Jean II */
			retval = 0;
		}

		/*
		 * If we fail to request resources for the
		 * new port, try to restore the old settings.
		 */
		if (retval && old_type != PORT_UNKNOWN) {
			uport->iobase = old_iobase;
			uport->type = old_type;
			uport->hub6 = old_hub6;
			uport->iotype = old_iotype;
			uport->regshift = old_shift;
			uport->mapbase = old_mapbase;
			retval = uport->ops->request_port(uport);
			/*
			 * If we failed to restore the old settings,
			 * we fail like this.
			 */
			if (retval)
				uport->type = PORT_UNKNOWN;

			/*
			 * We failed anyway.
			 */
			retval = -EBUSY;
			/* Added to return the correct error -Ram Gupta */
			goto exit;
		}
	}

	if (change_irq)
		uport->irq      = new_serial.irq;
	if (!(uport->flags & UPF_FIXED_PORT))
		uport->uartclk  = new_serial.baud_base * 16;
	uport->flags            = (uport->flags & ~UPF_CHANGE_MASK) |
				 (new_flags & UPF_CHANGE_MASK);
	uport->custom_divisor   = new_serial.custom_divisor;
	port->close_delay     = close_delay;
	port->closing_wait    = closing_wait;
	if (new_serial.xmit_fifo_size)
		uport->fifosize = new_serial.xmit_fifo_size;
	if (port->tty)
		port->tty->low_latency =
			(uport->flags & UPF_LOW_LATENCY) ? 1 : 0;

 check_and_exit:
	retval = 0;
	if (uport->type == PORT_UNKNOWN)
		goto exit;
	if (port->flags & ASYNC_INITIALIZED) {
		if (((old_flags ^ uport->flags) & UPF_SPD_MASK) ||
		    old_custom_divisor != uport->custom_divisor) {
			/*
			 * If they're setting up a custom divisor or speed,
			 * instead of clearing it, then bitch about it. No
			 * need to rate-limit; it's CAP_SYS_ADMIN only.
			 */
			if (uport->flags & UPF_SPD_MASK) {
				char buf[64];
				printk(KERN_NOTICE
				       "%s sets custom speed on %s. This "
				       "is deprecated.\n", current->comm,
				       tty_name(port->tty, buf));
			}
			uart_change_speed(tty, state, NULL);
		}
	} else
		retval = uart_startup(tty, state, 1);
 exit:
	mutex_unlock(&port->mutex);
	return retval;
}

/**
 *	uart_get_lsr_info	-	get line status register info
 *	@tty: tty associated with the UART
 *	@state: UART being queried
 *	@value: returned modem value
 *
 *	Note: uart_ioctl protects us against hangups.
 */
static int uart_get_lsr_info(struct tty_struct *tty,
			struct uart_state *state, unsigned int __user *value)
{
	struct uart_port *uport = state->uart_port;
	unsigned int result;

	result = uport->ops->tx_empty(uport);

	/*
	 * If we're about to load something into the transmit
	 * register, we'll pretend the transmitter isn't empty to
	 * avoid a race condition (depending on when the transmit
	 * interrupt happens).
	 */
	if (uport->x_char ||
	    ((uart_circ_chars_pending(&state->xmit) > 0) &&
	     !tty->stopped && !tty->hw_stopped))
		result &= ~TIOCSER_TEMT;

	return put_user(result, value);
}

static int uart_tiocmget(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port = &state->port;
	struct uart_port *uport = state->uart_port;
	int result = -EIO;

	mutex_lock(&port->mutex);
	if (!(tty->flags & (1 << TTY_IO_ERROR))) {
		result = uport->mctrl;
		spin_lock_irq(&uport->lock);
		result |= uport->ops->get_mctrl(uport);
		spin_unlock_irq(&uport->lock);
	}
	mutex_unlock(&port->mutex);

	return result;
}

static int
uart_tiocmset(struct tty_struct *tty, unsigned int set, unsigned int clear)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *uport = state->uart_port;
	struct tty_port *port = &state->port;
	int ret = -EIO;

	mutex_lock(&port->mutex);
	if (!(tty->flags & (1 << TTY_IO_ERROR))) {
		uart_update_mctrl(uport, set, clear);
		ret = 0;
	}
	mutex_unlock(&port->mutex);
	return ret;
}

static int uart_break_ctl(struct tty_struct *tty, int break_state)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port = &state->port;
	struct uart_port *uport = state->uart_port;

	mutex_lock(&port->mutex);

	if (uport->type != PORT_UNKNOWN)
		uport->ops->break_ctl(uport, break_state);

	mutex_unlock(&port->mutex);
	return 0;
}

static int uart_do_autoconfig(struct tty_struct *tty,struct uart_state *state)
{
	struct uart_port *uport = state->uart_port;
	struct tty_port *port = &state->port;
	int flags, ret;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	/*
	 * Take the per-port semaphore.  This prevents count from
	 * changing, and hence any extra opens of the port while
	 * we're auto-configuring.
	 */
	if (mutex_lock_interruptible(&port->mutex))
		return -ERESTARTSYS;

	ret = -EBUSY;
	if (tty_port_users(port) == 1) {
		uart_shutdown(tty, state);

		/*
		 * If we already have a port type configured,
		 * we must release its resources.
		 */
		if (uport->type != PORT_UNKNOWN)
			uport->ops->release_port(uport);

		flags = UART_CONFIG_TYPE;
		if (uport->flags & UPF_AUTO_IRQ)
			flags |= UART_CONFIG_IRQ;

		/*
		 * This will claim the ports resources if
		 * a port is found.
		 */
		uport->ops->config_port(uport, flags);

		ret = uart_startup(tty, state, 1);
	}
	mutex_unlock(&port->mutex);
	return ret;
}

/*
 * Wait for any of the 4 modem inputs (DCD,RI,DSR,CTS) to change
 * - mask passed in arg for lines of interest
 *   (use |'ed TIOCM_RNG/DSR/CD/CTS for masking)
 * Caller should use TIOCGICOUNT to see which one it was
 *
 * FIXME: This wants extracting into a common all driver implementation
 * of TIOCMWAIT using tty_port.
 */
static int
uart_wait_modem_status(struct uart_state *state, unsigned long arg)
{
	struct uart_port *uport = state->uart_port;
	struct tty_port *port = &state->port;
	DECLARE_WAITQUEUE(wait, current);
	struct uart_icount cprev, cnow;
	int ret;

	/*
	 * note the counters on entry
	 */
	spin_lock_irq(&uport->lock);
	memcpy(&cprev, &uport->icount, sizeof(struct uart_icount));

	/*
	 * Force modem status interrupts on
	 */
	uport->ops->enable_ms(uport);
	spin_unlock_irq(&uport->lock);

	add_wait_queue(&port->delta_msr_wait, &wait);
	for (;;) {
		spin_lock_irq(&uport->lock);
		memcpy(&cnow, &uport->icount, sizeof(struct uart_icount));
		spin_unlock_irq(&uport->lock);

		set_current_state(TASK_INTERRUPTIBLE);

		if (((arg & TIOCM_RNG) && (cnow.rng != cprev.rng)) ||
		    ((arg & TIOCM_DSR) && (cnow.dsr != cprev.dsr)) ||
		    ((arg & TIOCM_CD)  && (cnow.dcd != cprev.dcd)) ||
		    ((arg & TIOCM_CTS) && (cnow.cts != cprev.cts))) {
			ret = 0;
			break;
		}

		schedule();

		/* see if a signal did it */
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			break;
		}

		cprev = cnow;
	}

	current->state = TASK_RUNNING;
	remove_wait_queue(&port->delta_msr_wait, &wait);

	return ret;
}

/*
 * Get counter of input serial line interrupts (DCD,RI,DSR,CTS)
 * Return: write counters to the user passed counter struct
 * NB: both 1->0 and 0->1 transitions are counted except for
 *     RI where only 0->1 is counted.
 */
static int uart_get_icount(struct tty_struct *tty,
			  struct serial_icounter_struct *icount)
{
	struct uart_state *state = tty->driver_data;
	struct uart_icount cnow;
	struct uart_port *uport = state->uart_port;

	spin_lock_irq(&uport->lock);
	memcpy(&cnow, &uport->icount, sizeof(struct uart_icount));
	spin_unlock_irq(&uport->lock);

	icount->cts         = cnow.cts;
	icount->dsr         = cnow.dsr;
	icount->rng         = cnow.rng;
	icount->dcd         = cnow.dcd;
	icount->rx          = cnow.rx;
	icount->tx          = cnow.tx;
	icount->frame       = cnow.frame;
	icount->overrun     = cnow.overrun;
	icount->parity      = cnow.parity;
	icount->brk         = cnow.brk;
	icount->buf_overrun = cnow.buf_overrun;

	return 0;
}

/*
 * Called via sys_ioctl.  We can use spin_lock_irq() here.
 */
static int
uart_ioctl(struct tty_struct *tty, unsigned int cmd,
	   unsigned long arg)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port = &state->port;
	void __user *uarg = (void __user *)arg;
	int ret = -ENOIOCTLCMD;


	/*
	 * These ioctls don't rely on the hardware to be present.
	 */
	switch (cmd) {
	case TIOCGSERIAL:
		ret = uart_get_info(state, uarg);
		break;

	case TIOCSSERIAL:
		ret = uart_set_info(tty, state, uarg);
		break;

	case TIOCSERCONFIG:
		ret = uart_do_autoconfig(tty, state);
		break;

	case TIOCSERGWILD: /* obsolete */
	case TIOCSERSWILD: /* obsolete */
		ret = 0;
		break;
	}

	if (ret != -ENOIOCTLCMD)
		goto out;

	if (tty->flags & (1 << TTY_IO_ERROR)) {
		ret = -EIO;
		goto out;
	}

	/*
	 * The following should only be used when hardware is present.
	 */
	switch (cmd) {
	case TIOCMIWAIT:
		ret = uart_wait_modem_status(state, arg);
		break;
	}

	if (ret != -ENOIOCTLCMD)
		goto out;

	mutex_lock(&port->mutex);

	if (tty->flags & (1 << TTY_IO_ERROR)) {
		ret = -EIO;
		goto out_up;
	}

	/*
	 * All these rely on hardware being present and need to be
	 * protected against the tty being hung up.
	 */
	switch (cmd) {
	case TIOCSERGETLSR: /* Get line status register */
		ret = uart_get_lsr_info(tty, state, uarg);
		break;

	default: {
		struct uart_port *uport = state->uart_port;
		if (uport->ops->ioctl)
			ret = uport->ops->ioctl(uport, cmd, arg);
		break;
	}
	}
out_up:
	mutex_unlock(&port->mutex);
out:
	return ret;
}

static void uart_set_ldisc(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *uport = state->uart_port;

	if (uport->ops->set_ldisc)
		uport->ops->set_ldisc(uport, tty->termios->c_line);
}

static void uart_set_termios(struct tty_struct *tty,
						struct ktermios *old_termios)
{
	struct uart_state *state = tty->driver_data;
	unsigned long flags;
	unsigned int cflag = tty->termios->c_cflag;


	/*
	 * These are the bits that are used to setup various
	 * flags in the low level driver. We can ignore the Bfoo
	 * bits in c_cflag; c_[io]speed will always be set
	 * appropriately by set_termios() in tty_ioctl.c
	 */
#define RELEVANT_IFLAG(iflag)	((iflag) & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))
	if ((cflag ^ old_termios->c_cflag) == 0 &&
	    tty->termios->c_ospeed == old_termios->c_ospeed &&
	    tty->termios->c_ispeed == old_termios->c_ispeed &&
	    RELEVANT_IFLAG(tty->termios->c_iflag ^ old_termios->c_iflag) == 0) {
		return;
	}

	uart_change_speed(tty, state, old_termios);

	/* Handle transition to B0 status */
	if ((old_termios->c_cflag & CBAUD) && !(cflag & CBAUD))
		uart_clear_mctrl(state->uart_port, TIOCM_RTS | TIOCM_DTR);
	/* Handle transition away from B0 status */
	else if (!(old_termios->c_cflag & CBAUD) && (cflag & CBAUD)) {
		unsigned int mask = TIOCM_DTR;
		if (!(cflag & CRTSCTS) ||
		    !test_bit(TTY_THROTTLED, &tty->flags))
			mask |= TIOCM_RTS;
		uart_set_mctrl(state->uart_port, mask);
	}

	/* Handle turning off CRTSCTS */
	if ((old_termios->c_cflag & CRTSCTS) && !(cflag & CRTSCTS)) {
		spin_lock_irqsave(&state->uart_port->lock, flags);
		tty->hw_stopped = 0;
		__uart_start(tty);
		spin_unlock_irqrestore(&state->uart_port->lock, flags);
	}
	/* Handle turning on CRTSCTS */
	else if (!(old_termios->c_cflag & CRTSCTS) && (cflag & CRTSCTS)) {
		spin_lock_irqsave(&state->uart_port->lock, flags);
		if (!(state->uart_port->ops->get_mctrl(state->uart_port) & TIOCM_CTS)) {
			tty->hw_stopped = 1;
			state->uart_port->ops->stop_tx(state->uart_port);
		}
		spin_unlock_irqrestore(&state->uart_port->lock, flags);
	}
}

/*
 * In 2.4.5, calls to this will be serialized via the BKL in
 *  linux/drivers/char/tty_io.c:tty_release()
 *  linux/drivers/char/tty_io.c:do_tty_handup()
 */
static void uart_close(struct tty_struct *tty, struct file *filp)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port;
	struct uart_port *uport;
	unsigned long flags;

	if (!state)
		return;

	uport = state->uart_port;
	port = &state->port;

	pr_debug("uart_close(%d) called\n", uport->line);

	if (tty_port_close_start(port, tty, filp) == 0)
		return;

	/*
	 * At this point, we stop accepting input.  To do this, we
	 * disable the receive line status interrupts.
	 */
	if (port->flags & ASYNC_INITIALIZED) {
		unsigned long flags;
		spin_lock_irqsave(&uport->lock, flags);
		uport->ops->stop_rx(uport);
		spin_unlock_irqrestore(&uport->lock, flags);
		/*
		 * Before we drop DTR, make sure the UART transmitter
		 * has completely drained; this is especially
		 * important if there is a transmit FIFO!
		 */
		uart_wait_until_sent(tty, uport->timeout);
	}

	mutex_lock(&port->mutex);
	uart_shutdown(tty, state);
	uart_flush_buffer(tty);

	tty_ldisc_flush(tty);

	tty_port_tty_set(port, NULL);
	spin_lock_irqsave(&port->lock, flags);
	tty->closing = 0;

	if (port->blocked_open) {
		spin_unlock_irqrestore(&port->lock, flags);
		if (port->close_delay)
			msleep_interruptible(
					jiffies_to_msecs(port->close_delay));
		spin_lock_irqsave(&port->lock, flags);
	} else if (!uart_console(uport)) {
		spin_unlock_irqrestore(&port->lock, flags);
		uart_change_pm(state, 3);
		spin_lock_irqsave(&port->lock, flags);
	}

	/*
	 * Wake up anyone trying to open this port.
	 */
	clear_bit(ASYNCB_NORMAL_ACTIVE, &port->flags);
	clear_bit(ASYNCB_CLOSING, &port->flags);
	spin_unlock_irqrestore(&port->lock, flags);
	wake_up_interruptible(&port->open_wait);
	wake_up_interruptible(&port->close_wait);

	mutex_unlock(&port->mutex);
}

static void uart_wait_until_sent(struct tty_struct *tty, int timeout)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port = state->uart_port;
	unsigned long char_time, expire;

	if (port->type == PORT_UNKNOWN || port->fifosize == 0)
		return;

	/*
	 * Set the check interval to be 1/5 of the estimated time to
	 * send a single character, and make it at least 1.  The check
	 * interval should also be less than the timeout.
	 *
	 * Note: we have to use pretty tight timings here to satisfy
	 * the NIST-PCTS.
	 */
	char_time = (port->timeout - HZ/50) / port->fifosize;
	char_time = char_time / 5;
	if (char_time == 0)
		char_time = 1;
	if (timeout && timeout < char_time)
		char_time = timeout;

	/*
	 * If the transmitter hasn't cleared in twice the approximate
	 * amount of time to send the entire FIFO, it probably won't
	 * ever clear.  This assumes the UART isn't doing flow
	 * control, which is currently the case.  Hence, if it ever
	 * takes longer than port->timeout, this is probably due to a
	 * UART bug of some kind.  So, we clamp the timeout parameter at
	 * 2*port->timeout.
	 */
	if (timeout == 0 || timeout > 2 * port->timeout)
		timeout = 2 * port->timeout;

	expire = jiffies + timeout;

	pr_debug("uart_wait_until_sent(%d), jiffies=%lu, expire=%lu...\n",
		port->line, jiffies, expire);

	/*
	 * Check whether the transmitter is empty every 'char_time'.
	 * 'timeout' / 'expire' give us the maximum amount of time
	 * we wait.
	 */
	while (!port->ops->tx_empty(port)) {
		msleep_interruptible(jiffies_to_msecs(char_time));
		if (signal_pending(current))
			break;
		if (time_after(jiffies, expire))
			break;
	}
}

/*
 * This is called with the BKL held in
 *  linux/drivers/char/tty_io.c:do_tty_hangup()
 * We're called from the eventd thread, so we can sleep for
 * a _short_ time only.
 */
static void uart_hangup(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port = &state->port;
	unsigned long flags;

	pr_debug("uart_hangup(%d)\n", state->uart_port->line);

	mutex_lock(&port->mutex);
	if (port->flags & ASYNC_NORMAL_ACTIVE) {
		uart_flush_buffer(tty);
		uart_shutdown(tty, state);
		spin_lock_irqsave(&port->lock, flags);
		port->count = 0;
		clear_bit(ASYNCB_NORMAL_ACTIVE, &port->flags);
		spin_unlock_irqrestore(&port->lock, flags);
		tty_port_tty_set(port, NULL);
		wake_up_interruptible(&port->open_wait);
		wake_up_interruptible(&port->delta_msr_wait);
	}
	mutex_unlock(&port->mutex);
}

static int uart_port_activate(struct tty_port *port, struct tty_struct *tty)
{
	return 0;
}

static void uart_port_shutdown(struct tty_port *port)
{
	struct uart_state *state = container_of(port, struct uart_state, port);
	struct uart_port *uport = state->uart_port;

	/*
	 * clear delta_msr_wait queue to avoid mem leaks: we may free
	 * the irq here so the queue might never be woken up.  Note
	 * that we won't end up waiting on delta_msr_wait again since
	 * any outstanding file descriptors should be pointing at
	 * hung_up_tty_fops now.
	 */
	wake_up_interruptible(&port->delta_msr_wait);

	/*
	 * Free the IRQ and disable the port.
	 */
	uport->ops->shutdown(uport);

	/*
	 * Ensure that the IRQ handler isn't running on another CPU.
	 */
	synchronize_irq(uport->irq);
}

static int uart_carrier_raised(struct tty_port *port)
{
	struct uart_state *state = container_of(port, struct uart_state, port);
	struct uart_port *uport = state->uart_port;
	int mctrl;
	spin_lock_irq(&uport->lock);
	uport->ops->enable_ms(uport);
	mctrl = uport->ops->get_mctrl(uport);
	spin_unlock_irq(&uport->lock);
	if (mctrl & TIOCM_CAR)
		return 1;
	return 0;
}

static void uart_dtr_rts(struct tty_port *port, int onoff)
{
	struct uart_state *state = container_of(port, struct uart_state, port);
	struct uart_port *uport = state->uart_port;

	if (onoff)
		uart_set_mctrl(uport, TIOCM_DTR | TIOCM_RTS);
	else
		uart_clear_mctrl(uport, TIOCM_DTR | TIOCM_RTS);
}

/*
 * calls to uart_open are serialised by the BKL in
 *   fs/char_dev.c:chrdev_open()
 * Note that if this fails, then uart_close() _will_ be called.
 *
 * In time, we want to scrap the "opening nonpresent ports"
 * behaviour and implement an alternative way for setserial
 * to set base addresses/ports/types.  This will allow us to
 * get rid of a certain amount of extra tests.
 */
static int uart_open(struct tty_struct *tty, struct file *filp)
{
	struct uart_driver *drv = (struct uart_driver *)tty->driver->driver_state;
	int retval, line = tty->index;
	struct uart_state *state = drv->state + line;
	struct tty_port *port = &state->port;

	pr_debug("uart_open(%d) called\n", line);

	/*
	 * We take the semaphore here to guarantee that we won't be re-entered
	 * while allocating the state structure, or while we request any IRQs
	 * that the driver may need.  This also has the nice side-effect that
	 * it delays the action of uart_hangup, so we can guarantee that
	 * state->port.tty will always contain something reasonable.
	 */
	if (mutex_lock_interruptible(&port->mutex)) {
		retval = -ERESTARTSYS;
		goto end;
	}

	port->count++;
	if (!state->uart_port || state->uart_port->flags & UPF_DEAD) {
		retval = -ENXIO;
		goto err_dec_count;
	}

	/*
	 * Once we set tty->driver_data here, we are guaranteed that
	 * uart_close() will decrement the driver module use count.
	 * Any failures from here onwards should not touch the count.
	 */
	tty->driver_data = state;
	state->uart_port->state = state;
	tty->low_latency = (state->uart_port->flags & UPF_LOW_LATENCY) ? 1 : 0;
	tty_port_tty_set(port, tty);

	/*
	 * If the port is in the middle of closing, bail out now.
	 */
	if (tty_hung_up_p(filp)) {
		retval = -EAGAIN;
		goto err_dec_count;
	}

	/*
	 * Make sure the device is in D0 state.
	 */
	if (port->count == 1)
		uart_change_pm(state, 0);

	/*
	 * Start up the serial port.
	 */
	retval = uart_startup(tty, state, 0);

	/*
	 * If we succeeded, wait until the port is ready.
	 */
	mutex_unlock(&port->mutex);
	if (retval == 0)
		retval = tty_port_block_til_ready(port, tty, filp);

end:
	return retval;
err_dec_count:
	port->count--;
	mutex_unlock(&port->mutex);
	goto end;
}

static const char *uart_type(struct uart_port *port)
{
	const char *str = NULL;

	if (port->ops->type)
		str = port->ops->type(port);

	if (!str)
		str = "unknown";

	return str;
}

#ifdef CONFIG_PROC_FS

static void uart_line_info(struct seq_file *m, struct uart_driver *drv, int i)
{
	struct uart_state *state = drv->state + i;
	struct tty_port *port = &state->port;
	int pm_state;
	struct uart_port *uport = state->uart_port;
	char stat_buf[32];
	unsigned int status;
	int mmio;

	if (!uport)
		return;

	mmio = uport->iotype >= UPIO_MEM;
	seq_printf(m, "%d: uart:%s %s%08llX irq:%d",
			uport->line, uart_type(uport),
			mmio ? "mmio:0x" : "port:",
			mmio ? (unsigned long long)uport->mapbase
			     : (unsigned long long)uport->iobase,
			uport->irq);

	if (uport->type == PORT_UNKNOWN) {
		seq_putc(m, '\n');
		return;
	}

	if (capable(CAP_SYS_ADMIN)) {
		mutex_lock(&port->mutex);
		pm_state = state->pm_state;
		if (pm_state)
			uart_change_pm(state, 0);
		spin_lock_irq(&uport->lock);
		status = uport->ops->get_mctrl(uport);
		spin_unlock_irq(&uport->lock);
		if (pm_state)
			uart_change_pm(state, pm_state);
		mutex_unlock(&port->mutex);

		seq_printf(m, " tx:%d rx:%d",
				uport->icount.tx, uport->icount.rx);
		if (uport->icount.frame)
			seq_printf(m, " fe:%d",
				uport->icount.frame);
		if (uport->icount.parity)
			seq_printf(m, " pe:%d",
				uport->icount.parity);
		if (uport->icount.brk)
			seq_printf(m, " brk:%d",
				uport->icount.brk);
		if (uport->icount.overrun)
			seq_printf(m, " oe:%d",
				uport->icount.overrun);

#define INFOBIT(bit, str) \
	if (uport->mctrl & (bit)) \
		strncat(stat_buf, (str), sizeof(stat_buf) - \
			strlen(stat_buf) - 2)
#define STATBIT(bit, str) \
	if (status & (bit)) \
		strncat(stat_buf, (str), sizeof(stat_buf) - \
		       strlen(stat_buf) - 2)

		stat_buf[0] = '\0';
		stat_buf[1] = '\0';
		INFOBIT(TIOCM_RTS, "|RTS");
		STATBIT(TIOCM_CTS, "|CTS");
		INFOBIT(TIOCM_DTR, "|DTR");
		STATBIT(TIOCM_DSR, "|DSR");
		STATBIT(TIOCM_CAR, "|CD");
		STATBIT(TIOCM_RNG, "|RI");
		if (stat_buf[0])
			stat_buf[0] = ' ';

		seq_puts(m, stat_buf);
	}
	seq_putc(m, '\n');
#undef STATBIT
#undef INFOBIT
}

static int uart_proc_show(struct seq_file *m, void *v)
{
	struct tty_driver *ttydrv = m->private;
	struct uart_driver *drv = ttydrv->driver_state;
	int i;

	seq_printf(m, "serinfo:1.0 driver%s%s revision:%s\n",
			"", "", "");
	for (i = 0; i < drv->nr; i++)
		uart_line_info(m, drv, i);
	return 0;
}

static int uart_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, uart_proc_show, PDE(inode)->data);
}

static const struct file_operations uart_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= uart_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

#if defined(CONFIG_SERIAL_CORE_CONSOLE) || defined(CONFIG_CONSOLE_POLL)
/*
 *	uart_console_write - write a console message to a serial port
 *	@port: the port to write the message
 *	@s: array of characters
 *	@count: number of characters in string to write
 *	@write: function to write character to port
 */
void uart_console_write(struct uart_port *port, const char *s,
			unsigned int count,
			void (*putchar)(struct uart_port *, int))
{
	unsigned int i;

	for (i = 0; i < count; i++, s++) {
		if (*s == '\n')
			putchar(port, '\r');
		putchar(port, *s);
	}
}
EXPORT_SYMBOL_GPL(uart_console_write);

/*
 *	Check whether an invalid uart number has been specified, and
 *	if so, search for the first available port that does have
 *	console support.
 */
struct uart_port * __init
uart_get_console(struct uart_port *ports, int nr, struct console *co)
{
	int idx = co->index;

	if (idx < 0 || idx >= nr || (ports[idx].iobase == 0 &&
				     ports[idx].membase == NULL))
		for (idx = 0; idx < nr; idx++)
			if (ports[idx].iobase != 0 ||
			    ports[idx].membase != NULL)
				break;

	co->index = idx;

	return ports + idx;
}

/**
 *	uart_parse_options - Parse serial port baud/parity/bits/flow contro.
 *	@options: pointer to option string
 *	@baud: pointer to an 'int' variable for the baud rate.
 *	@parity: pointer to an 'int' variable for the parity.
 *	@bits: pointer to an 'int' variable for the number of data bits.
 *	@flow: pointer to an 'int' variable for the flow control character.
 *
 *	uart_parse_options decodes a string containing the serial console
 *	options.  The format of the string is <baud><parity><bits><flow>,
 *	eg: 115200n8r
 */
void
uart_parse_options(char *options, int *baud, int *parity, int *bits, int *flow)
{
	char *s = options;

	*baud = simple_strtoul(s, NULL, 10);
	while (*s >= '0' && *s <= '9')
		s++;
	if (*s)
		*parity = *s++;
	if (*s)
		*bits = *s++ - '0';
	if (*s)
		*flow = *s;
}
EXPORT_SYMBOL_GPL(uart_parse_options);

struct baud_rates {
	unsigned int rate;
	unsigned int cflag;
};

static const struct baud_rates baud_rates[] = {
	{ 921600, B921600 },
	{ 460800, B460800 },
	{ 230400, B230400 },
	{ 115200, B115200 },
	{  57600, B57600  },
	{  38400, B38400  },
	{  19200, B19200  },
	{   9600, B9600   },
	{   4800, B4800   },
	{   2400, B2400   },
	{   1200, B1200   },
	{      0, B38400  }
};

/**
 *	uart_set_options - setup the serial console parameters
 *	@port: pointer to the serial ports uart_port structure
 *	@co: console pointer
 *	@baud: baud rate
 *	@parity: parity character - 'n' (none), 'o' (odd), 'e' (even)
 *	@bits: number of data bits
 *	@flow: flow control character - 'r' (rts)
 */
int
uart_set_options(struct uart_port *port, struct console *co,
		 int baud, int parity, int bits, int flow)
{
	struct ktermios termios;
	static struct ktermios dummy;
	int i;

	/*
	 * Ensure that the serial console lock is initialised
	 * early.
	 */
	spin_lock_init(&port->lock);
	lockdep_set_class(&port->lock, &port_lock_key);

	memset(&termios, 0, sizeof(struct ktermios));

	termios.c_cflag = CREAD | HUPCL | CLOCAL;

	/*
	 * Construct a cflag setting.
	 */
	for (i = 0; baud_rates[i].rate; i++)
		if (baud_rates[i].rate <= baud)
			break;

	termios.c_cflag |= baud_rates[i].cflag;

	if (bits == 7)
		termios.c_cflag |= CS7;
	else
		termios.c_cflag |= CS8;

	switch (parity) {
	case 'o': case 'O':
		termios.c_cflag |= PARODD;
		/*fall through*/
	case 'e': case 'E':
		termios.c_cflag |= PARENB;
		break;
	}

	if (flow == 'r')
		termios.c_cflag |= CRTSCTS;

	/*
	 * some uarts on other side don't support no flow control.
	 * So we set * DTR in host uart to make them happy
	 */
	port->mctrl |= TIOCM_DTR;

	port->ops->set_termios(port, &termios, &dummy);
	/*
	 * Allow the setting of the UART parameters with a NULL console
	 * too:
	 */
	if (co)
		co->cflag = termios.c_cflag;

	return 0;
}
EXPORT_SYMBOL_GPL(uart_set_options);
#endif /* CONFIG_SERIAL_CORE_CONSOLE */

/**
 * uart_change_pm - set power state of the port
 *
 * @state: port descriptor
 * @pm_state: new state
 *
 * Locking: port->mutex has to be held
 */
static void uart_change_pm(struct uart_state *state, int pm_state)
{
	struct uart_port *port = state->uart_port;

	if (state->pm_state != pm_state) {
		if (port->ops->pm)
			port->ops->pm(port, pm_state, state->pm_state);
		state->pm_state = pm_state;
	}
}

struct uart_match {
	struct uart_port *port;
	struct uart_driver *driver;
};

static int serial_match_port(struct device *dev, void *data)
{
	struct uart_match *match = data;
	struct tty_driver *tty_drv = match->driver->tty_driver;
	dev_t devt = MKDEV(tty_drv->major, tty_drv->minor_start) +
		match->port->line;

	return dev->devt == devt; /* Actually, only one tty per port */
}

int uart_suspend_port(struct uart_driver *drv, struct uart_port *uport)
{
	struct uart_state *state = drv->state + uport->line;
	struct tty_port *port = &state->port;
	struct device *tty_dev;
	struct uart_match match = {uport, drv};

	mutex_lock(&port->mutex);

	tty_dev = device_find_child(uport->dev, &match, serial_match_port);
	if (device_may_wakeup(tty_dev)) {
		if (!enable_irq_wake(uport->irq))
			uport->irq_wake = 1;
		put_device(tty_dev);
		mutex_unlock(&port->mutex);
		return 0;
	}
	if (console_suspend_enabled || !uart_console(uport))
		uport->suspended = 1;

	if (port->flags & ASYNC_INITIALIZED) {
		const struct uart_ops *ops = uport->ops;
		int tries;

		if (console_suspend_enabled || !uart_console(uport)) {
			set_bit(ASYNCB_SUSPENDED, &port->flags);
			clear_bit(ASYNCB_INITIALIZED, &port->flags);

			spin_lock_irq(&uport->lock);
			ops->stop_tx(uport);
			ops->set_mctrl(uport, 0);
			ops->stop_rx(uport);
			spin_unlock_irq(&uport->lock);
		}

		/*
		 * Wait for the transmitter to empty.
		 */
		for (tries = 3; !ops->tx_empty(uport) && tries; tries--)
			msleep(10);
		if (!tries)
			printk(KERN_ERR "%s%s%s%d: Unable to drain "
					"transmitter\n",
			       uport->dev ? dev_name(uport->dev) : "",
			       uport->dev ? ": " : "",
			       drv->dev_name,
			       drv->tty_driver->name_base + uport->line);

		if (console_suspend_enabled || !uart_console(uport))
			ops->shutdown(uport);
	}

	/*
	 * Disable the console device before suspending.
	 */
	if (console_suspend_enabled && uart_console(uport))
		console_stop(uport->cons);

	if (console_suspend_enabled || !uart_console(uport))
		uart_change_pm(state, 3);

	mutex_unlock(&port->mutex);

	return 0;
}

int uart_resume_port(struct uart_driver *drv, struct uart_port *uport)
{
	struct uart_state *state = drv->state + uport->line;
	struct tty_port *port = &state->port;
	struct device *tty_dev;
	struct uart_match match = {uport, drv};
	struct ktermios termios;

	mutex_lock(&port->mutex);

	tty_dev = device_find_child(uport->dev, &match, serial_match_port);
	if (!uport->suspended && device_may_wakeup(tty_dev)) {
		if (uport->irq_wake) {
			disable_irq_wake(uport->irq);
			uport->irq_wake = 0;
		}
		mutex_unlock(&port->mutex);
		return 0;
	}
	uport->suspended = 0;

	/*
	 * Re-enable the console device after suspending.
	 */
	if (uart_console(uport)) {
		/*
		 * First try to use the console cflag setting.
		 */
		memset(&termios, 0, sizeof(struct ktermios));
		termios.c_cflag = uport->cons->cflag;

		/*
		 * If that's unset, use the tty termios setting.
		 */
		if (port->tty && port->tty->termios && termios.c_cflag == 0)
			termios = *(port->tty->termios);

		if (console_suspend_enabled)
			uart_change_pm(state, 0);
		uport->ops->set_termios(uport, &termios, NULL);
		if (console_suspend_enabled)
			console_start(uport->cons);
	}

	if (port->flags & ASYNC_SUSPENDED) {
		const struct uart_ops *ops = uport->ops;
		int ret;

		uart_change_pm(state, 0);
		spin_lock_irq(&uport->lock);
		ops->set_mctrl(uport, 0);
		spin_unlock_irq(&uport->lock);
		if (console_suspend_enabled || !uart_console(uport)) {
			/* Protected by port mutex for now */
			struct tty_struct *tty = port->tty;
			ret = ops->startup(uport);
			if (ret == 0) {
				if (tty)
					uart_change_speed(tty, state, NULL);
				spin_lock_irq(&uport->lock);
				ops->set_mctrl(uport, uport->mctrl);
				ops->start_tx(uport);
				spin_unlock_irq(&uport->lock);
				set_bit(ASYNCB_INITIALIZED, &port->flags);
			} else {
				/*
				 * Failed to resume - maybe hardware went away?
				 * Clear the "initialized" flag so we won't try
				 * to call the low level drivers shutdown method.
				 */
				uart_shutdown(tty, state);
			}
		}

		clear_bit(ASYNCB_SUSPENDED, &port->flags);
	}

	mutex_unlock(&port->mutex);

	return 0;
}

static inline void
uart_report_port(struct uart_driver *drv, struct uart_port *port)
{
	char address[64];

	switch (port->iotype) {
	case UPIO_PORT:
		snprintf(address, sizeof(address), "I/O 0x%lx", port->iobase);
		break;
	case UPIO_HUB6:
		snprintf(address, sizeof(address),
			 "I/O 0x%lx offset 0x%x", port->iobase, port->hub6);
		break;
	case UPIO_MEM:
	case UPIO_MEM32:
	case UPIO_AU:
	case UPIO_TSI:
		snprintf(address, sizeof(address),
			 "MMIO 0x%llx", (unsigned long long)port->mapbase);
		break;
	default:
		strlcpy(address, "*unknown*", sizeof(address));
		break;
	}

	printk(KERN_INFO "%s%s%s%d at %s (irq = %d) is a %s\n",
	       port->dev ? dev_name(port->dev) : "",
	       port->dev ? ": " : "",
	       drv->dev_name,
	       drv->tty_driver->name_base + port->line,
	       address, port->irq, uart_type(port));
}

static void
uart_configure_port(struct uart_driver *drv, struct uart_state *state,
		    struct uart_port *port)
{
	unsigned int flags;

	/*
	 * If there isn't a port here, don't do anything further.
	 */
	if (!port->iobase && !port->mapbase && !port->membase)
		return;

	/*
	 * Now do the auto configuration stuff.  Note that config_port
	 * is expected to claim the resources and map the port for us.
	 */
	flags = 0;
	if (port->flags & UPF_AUTO_IRQ)
		flags |= UART_CONFIG_IRQ;
	if (port->flags & UPF_BOOT_AUTOCONF) {
		if (!(port->flags & UPF_FIXED_TYPE)) {
			port->type = PORT_UNKNOWN;
			flags |= UART_CONFIG_TYPE;
		}
		port->ops->config_port(port, flags);
	}

	if (port->type != PORT_UNKNOWN) {
		unsigned long flags;

		uart_report_port(drv, port);

		/* Power up port for set_mctrl() */
		uart_change_pm(state, 0);

		/*
		 * Ensure that the modem control lines are de-activated.
		 * keep the DTR setting that is set in uart_set_options()
		 * We probably don't need a spinlock around this, but
		 */
		spin_lock_irqsave(&port->lock, flags);
		port->ops->set_mctrl(port, port->mctrl & TIOCM_DTR);
		spin_unlock_irqrestore(&port->lock, flags);

		/*
		 * If this driver supports console, and it hasn't been
		 * successfully registered yet, try to re-register it.
		 * It may be that the port was not available.
		 */
		if (port->cons && !(port->cons->flags & CON_ENABLED))
			register_console(port->cons);

		/*
		 * Power down all ports by default, except the
		 * console if we have one.
		 */
		if (!uart_console(port))
			uart_change_pm(state, 3);
	}
}

#ifdef CONFIG_CONSOLE_POLL

static int uart_poll_init(struct tty_driver *driver, int line, char *options)
{
	struct uart_driver *drv = driver->driver_state;
	struct uart_state *state = drv->state + line;
	struct uart_port *port;
	int baud = 9600;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	if (!state || !state->uart_port)
		return -1;

	port = state->uart_port;
	if (!(port->ops->poll_get_char && port->ops->poll_put_char))
		return -1;

	if (options) {
		uart_parse_options(options, &baud, &parity, &bits, &flow);
		return uart_set_options(port, NULL, baud, parity, bits, flow);
	}

	return 0;
}

static int uart_poll_get_char(struct tty_driver *driver, int line)
{
	struct uart_driver *drv = driver->driver_state;
	struct uart_state *state = drv->state + line;
	struct uart_port *port;

	if (!state || !state->uart_port)
		return -1;

	port = state->uart_port;
	return port->ops->poll_get_char(port);
}

static void uart_poll_put_char(struct tty_driver *driver, int line, char ch)
{
	struct uart_driver *drv = driver->driver_state;
	struct uart_state *state = drv->state + line;
	struct uart_port *port;

	if (!state || !state->uart_port)
		return;

	port = state->uart_port;
	port->ops->poll_put_char(port, ch);
}
#endif

static const struct tty_operations uart_ops = {
	.open		= uart_open,
	.close		= uart_close,
	.write		= uart_write,
	.put_char	= uart_put_char,
	.flush_chars	= uart_flush_chars,
	.write_room	= uart_write_room,
	.chars_in_buffer= uart_chars_in_buffer,
	.flush_buffer	= uart_flush_buffer,
	.ioctl		= uart_ioctl,
	.throttle	= uart_throttle,
	.unthrottle	= uart_unthrottle,
	.send_xchar	= uart_send_xchar,
	.set_termios	= uart_set_termios,
	.set_ldisc	= uart_set_ldisc,
	.stop		= uart_stop,
	.start		= uart_start,
	.hangup		= uart_hangup,
	.break_ctl	= uart_break_ctl,
	.wait_until_sent= uart_wait_until_sent,
#ifdef CONFIG_PROC_FS
	.proc_fops	= &uart_proc_fops,
#endif
	.tiocmget	= uart_tiocmget,
	.tiocmset	= uart_tiocmset,
	.get_icount	= uart_get_icount,
#ifdef CONFIG_CONSOLE_POLL
	.poll_init	= uart_poll_init,
	.poll_get_char	= uart_poll_get_char,
	.poll_put_char	= uart_poll_put_char,
#endif
};

static const struct tty_port_operations uart_port_ops = {
	.activate	= uart_port_activate,
	.shutdown	= uart_port_shutdown,
	.carrier_raised = uart_carrier_raised,
	.dtr_rts	= uart_dtr_rts,
};

/**
 *	uart_register_driver - register a driver with the uart core layer
 *	@drv: low level driver structure
 *
 *	Register a uart driver with the core driver.  We in turn register
 *	with the tty layer, and initialise the core driver per-port state.
 *
 *	We have a proc file in /proc/tty/driver which is named after the
 *	normal driver.
 *
 *	drv->port should be NULL, and the per-port structures should be
 *	registered using uart_add_one_port after this call has succeeded.
 */
int uart_register_driver(struct uart_driver *drv)
{
	struct tty_driver *normal;
	int i, retval;

	BUG_ON(drv->state);

#ifdef _USE_RELEASE_LOG_
	r_log_buf = kmalloc(LOG_BUF_SIZE, GFP_KERNEL);
	r_log_buf_size = LOG_BUF_SIZE;
	INIT_WORK(&log_queue, make_log_file);
#endif
	/*
	 * Maybe we should be using a slab cache for this, especially if
	 * we have a large number of ports to handle.
	 */
	drv->state = kzalloc(sizeof(struct uart_state) * drv->nr, GFP_KERNEL);
	if (!drv->state)
		goto out;

	normal = alloc_tty_driver(drv->nr);
	if (!normal)
		goto out_kfree;

	drv->tty_driver = normal;

	normal->driver_name	= drv->driver_name;
	normal->name		= drv->dev_name;
	normal->major		= drv->major;
	normal->minor_start	= drv->minor;
	normal->type		= TTY_DRIVER_TYPE_SERIAL;
	normal->subtype		= SERIAL_TYPE_NORMAL;
	normal->init_termios	= tty_std_termios;
	normal->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	normal->init_termios.c_ispeed = normal->init_termios.c_ospeed = 9600;
	normal->flags		= TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	normal->driver_state    = drv;
	tty_set_operations(normal, &uart_ops);

	/*
	 * Initialise the UART state(s).
	 */
	for (i = 0; i < drv->nr; i++) {
		struct uart_state *state = drv->state + i;
		struct tty_port *port = &state->port;

		tty_port_init(port);
		port->ops = &uart_port_ops;
		port->close_delay     = HZ / 2;	/* .5 seconds */
		port->closing_wait    = 30 * HZ;/* 30 seconds */
	}

	retval = tty_register_driver(normal);
	if (retval >= 0)
		return retval;

	put_tty_driver(normal);
out_kfree:
	kfree(drv->state);
out:
	return -ENOMEM;
}

/**
 *	uart_unregister_driver - remove a driver from the uart core layer
 *	@drv: low level driver structure
 *
 *	Remove all references to a driver from the core driver.  The low
 *	level driver must have removed all its ports via the
 *	uart_remove_one_port() if it registered them with uart_add_one_port().
 *	(ie, drv->port == NULL)
 */
void uart_unregister_driver(struct uart_driver *drv)
{
	struct tty_driver *p = drv->tty_driver;
	tty_unregister_driver(p);
	put_tty_driver(p);
	kfree(drv->state);
	drv->state = NULL;
	drv->tty_driver = NULL;
}

struct tty_driver *uart_console_device(struct console *co, int *index)
{
	struct uart_driver *p = co->data;
	*index = co->index;
	return p->tty_driver;
}

/**
 *	uart_add_one_port - attach a driver-defined port structure
 *	@drv: pointer to the uart low level driver structure for this port
 *	@uport: uart port structure to use for this port.
 *
 *	This allows the driver to register its own uart_port structure
 *	with the core driver.  The main purpose is to allow the low
 *	level uart drivers to expand uart_port, rather than having yet
 *	more levels of structures.
 */
int uart_add_one_port(struct uart_driver *drv, struct uart_port *uport)
{
	struct uart_state *state;
	struct tty_port *port;
	int ret = 0;
	struct device *tty_dev;

	BUG_ON(in_interrupt());

	if (uport->line >= drv->nr)
		return -EINVAL;

	state = drv->state + uport->line;
	port = &state->port;

	mutex_lock(&port_mutex);
	mutex_lock(&port->mutex);
	if (state->uart_port) {
		ret = -EINVAL;
		goto out;
	}

	state->uart_port = uport;
	state->pm_state = -1;

	uport->cons = drv->cons;
	uport->state = state;

	/*
	 * If this port is a console, then the spinlock is already
	 * initialised.
	 */
	if (!(uart_console(uport) && (uport->cons->flags & CON_ENABLED))) {
		spin_lock_init(&uport->lock);
		lockdep_set_class(&uport->lock, &port_lock_key);
	}

	uart_configure_port(drv, state, uport);

	/*
	 * Register the port whether it's detected or not.  This allows
	 * setserial to be used to alter this ports parameters.
	 */
	tty_dev = tty_register_device(drv->tty_driver, uport->line, uport->dev);
	if (likely(!IS_ERR(tty_dev))) {
		device_set_wakeup_capable(tty_dev, 1);
	} else {
		printk(KERN_ERR "Cannot register tty device on line %d\n",
		       uport->line);
	}

	/*
	 * Ensure UPF_DEAD is not set.
	 */
	uport->flags &= ~UPF_DEAD;

 out:
	mutex_unlock(&port->mutex);
	mutex_unlock(&port_mutex);

	return ret;
}

/**
 *	uart_remove_one_port - detach a driver defined port structure
 *	@drv: pointer to the uart low level driver structure for this port
 *	@uport: uart port structure for this port
 *
 *	This unhooks (and hangs up) the specified port structure from the
 *	core driver.  No further calls will be made to the low-level code
 *	for this port.
 */
int uart_remove_one_port(struct uart_driver *drv, struct uart_port *uport)
{
	struct uart_state *state = drv->state + uport->line;
	struct tty_port *port = &state->port;

	BUG_ON(in_interrupt());

	if (state->uart_port != uport)
		printk(KERN_ALERT "Removing wrong port: %p != %p\n",
			state->uart_port, uport);

	mutex_lock(&port_mutex);

	/*
	 * Mark the port "dead" - this prevents any opens from
	 * succeeding while we shut down the port.
	 */
	mutex_lock(&port->mutex);
	uport->flags |= UPF_DEAD;
	mutex_unlock(&port->mutex);

	/*
	 * Remove the devices from the tty layer
	 */
	tty_unregister_device(drv->tty_driver, uport->line);

	if (port->tty)
		tty_vhangup(port->tty);

	/*
	 * Free the port IO and memory resources, if any.
	 */
	if (uport->type != PORT_UNKNOWN)
		uport->ops->release_port(uport);

	/*
	 * Indicate that there isn't a port here anymore.
	 */
	uport->type = PORT_UNKNOWN;

	state->uart_port = NULL;
	mutex_unlock(&port_mutex);

	return 0;
}

/*
 *	Are the two ports equivalent?
 */
int uart_match_port(struct uart_port *port1, struct uart_port *port2)
{
	if (port1->iotype != port2->iotype)
		return 0;

	switch (port1->iotype) {
	case UPIO_PORT:
		return (port1->iobase == port2->iobase);
	case UPIO_HUB6:
		return (port1->iobase == port2->iobase) &&
		       (port1->hub6   == port2->hub6);
	case UPIO_MEM:
	case UPIO_MEM32:
	case UPIO_AU:
	case UPIO_TSI:
		return (port1->mapbase == port2->mapbase);
	}
	return 0;
}
EXPORT_SYMBOL(uart_match_port);

/**
 *	uart_handle_dcd_change - handle a change of carrier detect state
 *	@uport: uart_port structure for the open port
 *	@status: new carrier detect status, nonzero if active
 */
void uart_handle_dcd_change(struct uart_port *uport, unsigned int status)
{
	struct uart_state *state = uport->state;
	struct tty_port *port = &state->port;
	struct tty_ldisc *ld = tty_ldisc_ref(port->tty);
	struct pps_event_time ts;

	if (ld && ld->ops->dcd_change)
		pps_get_ts(&ts);

	uport->icount.dcd++;
#ifdef CONFIG_HARD_PPS
	if ((uport->flags & UPF_HARDPPS_CD) && status)
		hardpps();
#endif

	if (port->flags & ASYNC_CHECK_CD) {
		if (status)
			wake_up_interruptible(&port->open_wait);
		else if (port->tty)
			tty_hangup(port->tty);
	}

	if (ld && ld->ops->dcd_change)
		ld->ops->dcd_change(port->tty, status, &ts);
	if (ld)
		tty_ldisc_deref(ld);
}
EXPORT_SYMBOL_GPL(uart_handle_dcd_change);

/**
 *	uart_handle_cts_change - handle a change of clear-to-send state
 *	@uport: uart_port structure for the open port
 *	@status: new clear to send status, nonzero if active
 */
void uart_handle_cts_change(struct uart_port *uport, unsigned int status)
{
	struct tty_port *port = &uport->state->port;
	struct tty_struct *tty = port->tty;

	uport->icount.cts++;

	if (port->flags & ASYNC_CTS_FLOW) {
		if (tty->hw_stopped) {
			if (status) {
				tty->hw_stopped = 0;
				uport->ops->start_tx(uport);
				uart_write_wakeup(uport);
			}
		} else {
			if (!status) {
				tty->hw_stopped = 1;
				uport->ops->stop_tx(uport);
			}
		}
	}
}
EXPORT_SYMBOL_GPL(uart_handle_cts_change);

/**
 * uart_insert_char - push a char to the uart layer
 *
 * User is responsible to call tty_flip_buffer_push when they are done with
 * insertion.
 *
 * @port: corresponding port
 * @status: state of the serial port RX buffer (LSR for 8250)
 * @overrun: mask of overrun bits in @status
 * @ch: character to push
 * @flag: flag for the character (see TTY_NORMAL and friends)
 */
void uart_insert_char(struct uart_port *port, unsigned int status,
		 unsigned int overrun, unsigned int ch, unsigned int flag)
{
	struct tty_struct *tty = port->state->port.tty;

	if ((status & port->ignore_status_mask & ~overrun) == 0)
		tty_insert_flip_char(tty, ch, flag);

	/*
	 * Overrun is special.  Since it's reported immediately,
	 * it doesn't affect the current character.
	 */
	if (status & ~port->ignore_status_mask & overrun)
		tty_insert_flip_char(tty, 0, TTY_OVERRUN);
}


static const char * escape_codes[] = 
{
	"\033[1m",		
	"\033[1;35m",		
	"\033[30m",		/* 1. black   */
	"\033[31m",		/* 2. red     */
	"\033[32m",		/* 3. green   */
	"\033[33m",		/* 4. yellow  */
	"\033[34m",		/* 5. blue    */
	"\033[35m",		/* 6. magenta */
	"\033[36m",		/* 7. cyan    */
	"\033[37m",		/* 8. white   */
	"\033[43;30m",	/*            */
	"\033[44m",		/*            */
	"\033[0m",		/* escape end */
	"\033[m"		/* escape end */
};

static int num_escape_codes = sizeof(escape_codes) / sizeof(char *);

/**
 * remove escape codes
 */
void remove_escape_codes(char * log_buffer, unsigned long log_buffer_size)
{
	int matched_len = 0;
	int i = 0;
	int f_matched = 0;

	char * ptr = log_buffer;
	char * buffer_end = log_buffer + log_buffer_size - 1;
	char * src;
	char * tar;
	
	while (ptr <= (buffer_end - 4))
	{
		if (*ptr == '\033')
		{
			f_matched = 0;
			for (i = 0; i < num_escape_codes; i++)
			{
				matched_len = strlen(escape_codes[i]);
				if (strncmp(ptr, escape_codes[i], matched_len) == 0)
				{
					f_matched = 1;
					break;
				}
			}
			if (f_matched)
			{
				tar = ptr;
				src = ptr + matched_len;
				while (src <= buffer_end)
					*tar++ = *src++;
				memset(buffer_end - matched_len, 0, matched_len);
			}
		}
		ptr++;
	}
}

#if defined(CONFIG_DRM_DRIME4)
extern void display_fault(void);

#define DRIME4_WDOGREG(x) ((x) + DRIME4_VA_WDT)
#define DRIME4_WDTCR		 DRIME4_WDOGREG(0x0)
#define	DRIME4_WDTPSR		DRIME4_WDOGREG(0x4)
#define DRIME4_WDTLDR		DRIME4_WDOGREG(0x8)
#define DRIME4_WDTVLR		DRIME4_WDOGREG(0xC)
#define DRIME4_WDTISR		DRIME4_WDOGREG(0x10)

#define ENABLE_WATCHDOG_PATH		"/etc/watchdog_enable"

void reboot_exec(int sec)
{
	/* disable watchdog */
	__raw_writel(0, DRIME4_WDTCR);

	/* Scale Set */
	__raw_writel(0xFFFF, DRIME4_WDTPSR);
	__raw_writel(sec*63, DRIME4_WDTLDR);

	/* watchdog reset */
	__raw_writel(0x0007, DRIME4_WDTCR);
}
#endif

void uart_save_log_to_card(int saveType , char *func)
{
#ifdef _USE_RELEASE_LOG_
	unsigned char *buf;
	/* char *file_path[50] = {' '}; */
#if 0
	int int_fd = -1;
	int sys_fd = -1;
#endif
	int ext_fd = -1;
	int val;
	mm_segment_t old_fs = get_fs();
	char dlogfile_name[100] = {0,};
	char logfile_name[100] = {0,};
	//int logfile_fd = -1;
	//unsigned int log_file_check = 0;

	struct timeval my_time;   
	struct tm my_date;

	char log_end_mark[50] = {0,};
	struct timespec ts;
	char thread_name[80] = {0,};

	ktime_get_ts(&ts);
	buf = r_log_buf;

	set_fs(KERNEL_DS);

	if (isKeyLogSave) {
		isKeyLogSave = 0;
	} else {
#if defined(CONFIG_DRM_DRIME4)
		if (sys_access(ENABLE_WATCHDOG_PATH, 0) == 0) {
			is_poweroff_halt_status = 1;
			display_fault();
			reboot_exec(2);
			while(1);
		}
#endif
	}

	do_gettimeofday(&my_time);   
	time_to_tm(my_time.tv_sec, 0, &my_date);   
        
	printk(KERN_INFO "current  = %dy %dm %dd %dh %dm %ds, %dtz\n", \
		my_date.tm_year + 1900, my_date.tm_mon + 1, \
		my_date.tm_mday, my_date.tm_hour - sys_tz.tz_minuteswest / 60, \
		my_date.tm_min, my_date.tm_sec, sys_tz.tz_minuteswest);

#if 0
	/* Check Log file */
	for(log_file_check = 0 ; log_file_check < 1024 ; log_file_check++)
	{
		sprintf(logfile_name, "/sdcard/$log_%d.inf", log_file_check);
		logfile_fd = sys_open(logfile_name, O_RDWR, S_IRWXU & ~(S_IRUSR));
		if (logfile_fd >= 0)
		{
			sys_close(logfile_fd);
			logfile_fd = -1;
		}
		else
		{
			logfile_fd = -1;
			sprintf(logfile_name, "/sdcard/$log_%d.inf", log_file_check);
			sprintf(dlogfile_name, "/sdcard/$dlog_%d.inf", log_file_check);
			break;
		}
	}
#else
	sprintf(logfile_name, "/sdcard/%04d%02d%02d_%02d%02d%02d_log.inf",
		my_date.tm_year + 1900, my_date.tm_mon + 1, \
		my_date.tm_mday, my_date.tm_hour - sys_tz.tz_minuteswest / 60, \
		my_date.tm_min, my_date.tm_sec);
	sprintf(dlogfile_name, "/sdcard/%04d%02d%02d_%02d%02d%02d_dlog.inf",
		my_date.tm_year + 1900, my_date.tm_mon + 1, \
		my_date.tm_mday, my_date.tm_hour - sys_tz.tz_minuteswest / 60, \
		my_date.tm_min, my_date.tm_sec);
#endif
	
	val = __raw_readl(DRIME4_VA_GPIO + 0x183FC);		// GPIO24(1)
	val |= (1<<1);
	__raw_writel(val, DRIME4_VA_GPIO + 0x183FC);		// GPIO24(1)	

	remove_escape_codes(r_log_buf, r_log_buf_size);

	ext_fd = sys_open(logfile_name,
						O_RDWR|O_CREAT|O_TRUNC|O_SYNC,
						S_IRWXU & ~(S_IRUSR));

	if (ext_fd < 0) {
		sprintf(logfile_name, "/mnt/ubi1/rw_log.inf");
		sprintf(dlogfile_name, "/mnt/ubi1/rw_dlog.inf");
		
		ext_fd = sys_open(logfile_name,
							O_RDWR|O_CREAT|O_TRUNC|O_SYNC,
							S_IRWXU & ~(S_IRUSR));
	}

	{
		char *argv[] = { "/usr/bin/dlogutil", "-f", dlogfile_name, NULL };	//char *argv[] = { "/usr/bin/dlogutil", "-f", dlogfile_name, NULL };	//char *argv[] = { "/usr/bin/dlogutil", "-f", "/sdcard/$dlog.inf", NULL };
		static char *envp[] = {
				"HOME=/",
				"TERM=linux",
				"PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
		int ret = -EACCES;

		ret = call_usermodehelper(argv[0], argv, envp, UMH_NO_WAIT);
		printk(KERN_EMERG "ret = %d\n", ret);
	}

	if (r_log_overall) {
		r_log_buf[r_log_buf_size-1] = '\n';
		buf = r_log_buf + r_log_idx;

#if 0
		/* add end mark in log file. */
		if (r_log_idx + 8 < r_log_buf_size-1) {
			r_log_buf[r_log_idx] = '[';
			r_log_buf[r_log_idx+1] = 'L';
			r_log_buf[r_log_idx+2] = 'O';
			r_log_buf[r_log_idx+3] = 'G';
			r_log_buf[r_log_idx+4] = '_';
			r_log_buf[r_log_idx+5] = 'E';
			r_log_buf[r_log_idx+6] = 'N';
			r_log_buf[r_log_idx+7] = 'D';
			r_log_buf[r_log_idx+8] = ']';
		} else {
			r_log_buf[r_log_buf_size-10] = '[';
			r_log_buf[r_log_buf_size-9] = 'L';
			r_log_buf[r_log_buf_size-8] = 'O';
			r_log_buf[r_log_buf_size-7] = 'G';
			r_log_buf[r_log_buf_size-6] = '_';
			r_log_buf[r_log_buf_size-5] = 'E';
			r_log_buf[r_log_buf_size-4] = 'N';
			r_log_buf[r_log_buf_size-3] = 'D';
			r_log_buf[r_log_buf_size-2] = ']';
		}

		if (int_fd >= 0) {
			sys_write(int_fd, buf, r_log_buf_size-r_log_idx);
			sys_write(int_fd, r_log_buf, r_log_idx);
		}

		if (sys_fd >= 0) {
			sys_write(sys_fd, buf, r_log_buf_size-r_log_idx);
			sys_write(sys_fd, r_log_buf, r_log_idx);
		}
#endif
		if (ext_fd >= 0) {
			sys_write(ext_fd, buf, r_log_buf_size-r_log_idx);
			sys_write(ext_fd, r_log_buf, r_log_idx);
		}
	} else {
		if (r_log_idx < r_log_buf_size - 1) {
#if 0
			if (int_fd >= 0)
				sys_write(int_fd, buf, r_log_idx);
			if (sys_fd >= 0)
				sys_write(sys_fd, buf, r_log_idx);
#endif			
			if (ext_fd >= 0)
				sys_write(ext_fd, buf, r_log_idx);
		} else {
			r_log_buf[r_log_buf_size-1] = '\n';
#if 0
			if (int_fd >= 0)
				sys_write(int_fd, buf, r_log_buf_size);
			if (sys_fd >= 0)
				sys_write(sys_fd, buf, r_log_buf_size);
#endif
			if (ext_fd >= 0)
				sys_write(ext_fd, buf, r_log_buf_size);
		}
	}
#if 0
	if (int_fd >= 0) {
		sys_close(int_fd); int_fd = -1;
	}
	if (sys_fd >= 0) {
		sys_close(sys_fd); sys_fd = -1;
	}
#endif
	get_task_comm(thread_name,current);
	sprintf(log_end_mark, "\r\n[%d.%ld] serial log saved : %s tid(%d %s)", ts.tv_sec,ts.tv_nsec,func, sys_gettid(),thread_name);

	sys_write(ext_fd, log_end_mark, strlen(log_end_mark));

	if (ext_fd >= 0) {
		sys_close(ext_fd); ext_fd = -1;
	}

	msleep(5000);

	sys_sync();

	
	printk(KERN_ALERT "[%d.%ld] serial log saved : %s tid(%d %s) \n", ts.tv_sec,ts.tv_nsec,func, sys_gettid(),thread_name);

	set_fs(old_fs);
	{
		char *argv[] = { "/bin/sync", NULL, NULL };
		static char *envp[] = {
				"HOME=/",
				"TERM=linux",
				"PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
		int ret = -EACCES;

		ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
		//printk(KERN_EMERG "ret = %d\n", ret);
	}

	{
		char *argv[] = { "/usr/bin/killall", "dlogutil", NULL };
		static char *envp[] = {
				"HOME=/",
				"TERM=linux",
				"PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
		int ret = -EACCES;

		ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
		//printk(KERN_EMERG "ret = %d\n", ret);
	}
		
	val = __raw_readl(DRIME4_VA_GPIO + 0x183FC);		// GPIO24(1)
	val &= ~(1<<1);
	__raw_writel(val, DRIME4_VA_GPIO + 0x183FC);		// GPIO24(1)
#endif
}

void uart_save_log(void)
{
#ifdef _USE_RELEASE_LOG_
	isKeyLogSave = 1;
	schedule_work(&log_queue);
#endif
}

#define R_LOG_LEVEL	4
int uart_log_save_print(unsigned char *buf, int count)
{
#ifdef _USE_RELEASE_LOG_
	int r_count = 0, level = 0, ret = 0;
	unsigned char *r_buf = NULL;

	if (r_log_buf == NULL)
		return ret;

	r_buf = buf;
	r_count = count;
	if (count > 3) {
		/* is there log level */
		if (r_buf[0] == '<' && r_buf[1] >= '0' && r_buf[1] <= '7' && r_buf[2] == '>') {
			level = r_buf[1] - '0';
		}

		if (R_LOG_LEVEL < level) {
			return ret;
		}
	}

	if (r_log_idx + r_count > r_log_buf_size) {
		memcpy(r_log_buf + r_log_idx, r_buf, r_log_buf_size-r_log_idx);
		r_count -= r_log_buf_size-r_log_idx;
		r_buf += r_log_buf_size-r_log_idx;
		r_log_idx = 0;
		r_log_overall = 1;
	}
	memcpy(r_log_buf + r_log_idx, r_buf, r_count);
	r_log_idx = (r_log_idx + r_count) & (r_log_buf_size - 1);

	return ret;
#else
	return 0;
#endif
}

EXPORT_SYMBOL_GPL(uart_insert_char);

EXPORT_SYMBOL(uart_write_wakeup);
EXPORT_SYMBOL(uart_register_driver);
EXPORT_SYMBOL(uart_unregister_driver);
EXPORT_SYMBOL(uart_suspend_port);
EXPORT_SYMBOL(uart_resume_port);
EXPORT_SYMBOL(uart_add_one_port);
EXPORT_SYMBOL(uart_remove_one_port);

MODULE_DESCRIPTION("Serial driver core");
MODULE_LICENSE("GPL");
