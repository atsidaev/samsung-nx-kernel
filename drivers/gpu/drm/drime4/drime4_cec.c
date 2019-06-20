/**
 * @file d4_hdmi_cec.c
 * @brief DRIMe4 HDMI CEC Interface / Control
 * @author Somabha Bhattacharjya <b.somabha@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/io.h>
#include <linux/platform_device.h>

//#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/sched.h>
//#include <linux/input.h>

#include <mach/hdmi/video/d4_hdmi_video.h>
#include <video/hdmi/d4_hdmi_cec_ioctl.h>
//#include <video/hdmi/d4_hdmi_video_type.h>
#include "drime4_cec_regs.h"
#include "drime4_hdmi.h"
#include "../../../video/drime4/hdmi/video/d4_hdmi.h"

#define CEC_MESSAGE_BROADCAST_MASK	0x0F
#define CEC_MESSAGE_BROADCAST		0x0F
#define CEC_FILTER_THRESHOLD		0x15

/**
 * @enum edp_hdmi_cec_state
 * @brief Defines all possible states of CEC software state machine
 */
enum hdmi_cec_state {
	STATE_RX, STATE_TX, STATE_DONE, STATE_ERROR
};

/**
 * @struct hdmi_cec_rx_struct
 * @brief Holds CEC Rx state and data
 */
struct hdmi_cec_rx_struct {
	spinlock_t lock;
	wait_queue_head_t waitq;
	atomic_t state;
	u8 *buffer;
	unsigned int size;
//	struct input_dev *input;
//	struct tasklet_struct input_tasklet;
};

/**
 * @struct cec_tx_struct
 * @brief Holds CEC Tx state and data
 */
struct cec_tx_struct {
	wait_queue_head_t waitq;
	atomic_t state;
};

#if 0
struct cec_remote_button {
	int code;
	int type;
};
struct platform_cec_dev_data {
	struct cec_remote_button *cec_btn;
	int butsize;
};
#endif

static void __iomem *regs;
static struct resource *ioarea;

static struct hdmi_cec_rx_struct cec_rx_struct;
static struct cec_tx_struct cec_tx_struct;
static unsigned int retransmission = 5;

static int drime4_cec_open(struct inode *inode, struct file *file);
static int drime4_cec_release(struct inode *inode, struct file *file);
static ssize_t drime4_cec_read(struct file *file, char __user *buffer,
		size_t count, loff_t *ppos);
static ssize_t drime4_cec_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos);
static long drime4_cec_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg);
static unsigned int drime4_cec_poll(struct file *file, poll_table *wait);

static irqreturn_t drime4_cec_irq_handler(int irq, void *dev_id);

static inline void drime4_cec_set_divider(void);
static inline void drime4_cec_disable_rx(void);
static inline void drime4_cec_enable_rx(void);
static inline void drime4_cec_mask_rx_interrupts(void);
static inline void drime4_cec_unmask_rx_interrupts(void);
static inline void drime4_cec_mask_tx_interrupts(void);
static inline void drime4_cec_unmask_tx_interrupts(void);
static inline void drime4_cec_set_rx_state(enum hdmi_cec_state state);
static inline void drime4_cec_set_tx_state(enum hdmi_cec_state state);

//static void send_deferred_input(unsigned long _cec_rx_struct);

static const struct file_operations drime4_cec_fops = { .owner = THIS_MODULE,
		.open = drime4_cec_open, .release = drime4_cec_release, .read =
				drime4_cec_read, .write = drime4_cec_write, .unlocked_ioctl =
				drime4_cec_ioctl, .poll = drime4_cec_poll, };

static struct miscdevice drime4_cec_misc_device = { .minor = MISC_DYNAMIC_MINOR,
		.name = HDMI_CEC_MODULE_NAME, .fops = &drime4_cec_fops, };

/**
 * @brief   CEC Device Open function
 * @fn      int drime4_cec_open(struct inode *inode, struct file *file)
 * @param   *inode	[in] inode struct
 * @param	*file	[in] file struct
 * @return  int
 *
 */
int drime4_cec_open(struct inode *inode, struct file *file)
{
	drime4_cec_set_divider();

	/* setup filter */
	writeb(CEC_FILTER_THRESHOLD, regs + CEC_FILTER_TH);
	writeb(CEC_FILTER_EN | CEC_FILTER_CUR_VAL, regs + CEC_FILTER_CTRL);

	/* reset CEC Rx */
	writeb(CEC_RX_CTRL_RESET, regs + CEC_RX_CTRL);
	/* reset CEC Tx */
	writeb(CEC_TX_CTRL_RESET, regs + CEC_TX_CTRL);

	drime4_cec_unmask_tx_interrupts();
	drime4_cec_set_rx_state(STATE_RX);
	drime4_cec_unmask_rx_interrupts();
	drime4_cec_enable_rx();
	return 0;
}

int drime4_cec_release(struct inode *inode, struct file *file)
{
	return 0;
}

ssize_t drime4_cec_read(struct file *file, char __user *buffer, size_t count,
		loff_t *ppos)
{
	ssize_t retval;
	if (wait_event_interruptible(cec_rx_struct.waitq,
			atomic_read(&cec_rx_struct.state) == STATE_DONE)) {
		return -ERESTARTSYS;
	}
	spin_lock_irq(&cec_rx_struct.lock);

	if (cec_rx_struct.size > count) {
		spin_unlock_irq(&cec_rx_struct.lock);
		return -1;
	}

	if (copy_to_user(buffer, cec_rx_struct.buffer, cec_rx_struct.size)) {
		spin_unlock_irq(&cec_rx_struct.lock);
		return -EFAULT;
	}
	retval = cec_rx_struct.size;

	drime4_cec_set_rx_state(STATE_RX);
	spin_unlock_irq(&cec_rx_struct.lock);
	return retval;
}

ssize_t drime4_cec_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	char *data;
	unsigned char reg = 0x00;
	int i = 0;

	/* check data size */
	if (count > CEC_TX_BUFF_SIZE || count == 0)
		return -1;

	data = kmalloc(count, GFP_KERNEL);
	if (!data) {
		return -1;
	}

	if (copy_from_user(data, buffer, count)) {
		kfree(data);
		return -EFAULT;
	}

	/* copy packet to hardware buffer */
	while (i < count) {
		writeb(data[i], regs + CEC_TX_BUFF0 + (i * 4));
		i++;
	}

	/* set number of bytes to transfer */
	writeb(count, regs + CEC_TX_BYTES);

	drime4_cec_set_tx_state(STATE_TX);

	/* start transfer */
	reg |= CEC_TX_CTRL_START;

	/* if message is broadcast message - set corresponding bit */
	if ((data[0] & CEC_MESSAGE_BROADCAST_MASK) == CEC_MESSAGE_BROADCAST)
		reg |= CEC_TX_CTRL_BCAST;
	else
		reg &= ~CEC_TX_CTRL_BCAST;

	/* set number of retransmissions */
	reg |= (retransmission<<4);
	writeb(reg, regs + CEC_TX_CTRL);
	kfree(data);

	/* wait for interrupt */
	if (wait_event_interruptible(cec_tx_struct.waitq,
			(atomic_read(&cec_tx_struct.state) != STATE_TX))) {
		printk("ret : -512\n");
		return -ERESTARTSYS;
	}
	if (atomic_read(&cec_tx_struct.state) == STATE_ERROR) {
		printk("ret : -1\n");
		return -1;
	}

	return count;
}

/**
 * @brief CEC IOCTL interface processing function
 * @fn	int cec_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
 * @param	*filp	[in] file struct
 * @param   cmd		[in] cmd
 * @param   arg		[in] arg
 * @return On error returns a negative error, zero otherwise. <br>
 */
long drime4_cec_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned int laddr;

	switch (cmd) {

	case CEC_IOC_SETLADDR:
		if (get_user(laddr, (unsigned int __user *) arg))
			return -EFAULT;
		writeb(laddr & 0x0F, regs + CEC_LOGIC_ADDR);
		break;
	case CEC_IOC_SETRETRYCNT:
		if (get_user(retransmission, (unsigned int __user *) arg))
			return -EFAULT;
		if (retransmission) {
			drime4_cec_unmask_rx_interrupts();
			drime4_cec_enable_rx();
		} else {
			drime4_cec_mask_rx_interrupts();
			drime4_cec_disable_rx();
		}
		
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/**
 * @brief Poll interface handler function
 * @fn	unsigned int cec_poll(struct file *file, poll_table *wait)
 * @param *file	[in]	driver file discriptor
 * @param *wait	[in]	poll table
 * @return On error returns a negative error, zero otherwise. <br>
 */
unsigned int drime4_cec_poll(struct file *file, poll_table *wait)
{
	poll_wait(file, &(cec_rx_struct.waitq), wait);
	if (atomic_read(&(cec_rx_struct.state)) == STATE_DONE)
		return POLLIN | POLLRDNORM;
	return 0;
}

/**
 * @brief CEC interrupt handler
 *		Handles interrupt requests from CEC hardware.
 *		Action depends on current state of CEC hardware.
 * @fn	irqreturn_t drime4_cec_irq_handler(int irq, void *dev_id)
 * @param	irq	[in] interrupt number
 * @param   *dev_id		[in] device id
 * @return  irqreturn_t
 */
irqreturn_t drime4_cec_irq_handler(int irq, void *dev_id)
{
	unsigned char status;
	status = readb(regs + CEC_TX_STATUS_0);
	if (status & CEC_STATUS_TX_DONE) {
		if (status & CEC_STATUS_TX_ERROR)
			drime4_cec_set_tx_state(STATE_ERROR);
		else
			drime4_cec_set_tx_state(STATE_DONE);

		/* clear interrupt pending bit */
		writeb(CEC_IRQ_TX_DONE | CEC_IRQ_TX_ERROR,
				regs + CEC_INTR_CLEAR);

		wake_up_interruptible(&(cec_tx_struct.waitq));
	}
	status = readb(regs + CEC_RX_STATUS_0);
	if (status & CEC_STATUS_RX_DONE) {
		if (status & CEC_STATUS_RX_ERROR) {
			/* reset CEC Rx */
			writeb(CEC_RX_CTRL_RESET, regs + CEC_RX_CTRL);
		} else {
			unsigned int size, i = 0;
			/* copy data from internal buffer */
			size = readb(regs + CEC_RX_STATUS_1);
			spin_lock(&cec_rx_struct.lock);

			while (i < size) {
				if (i >= CEC_RX_BUFF_SIZE) {
					printk(KERN_WARNING "\n[CEC_TX_BUFF_SIZE exceed!! %d, %d]\n", i, CEC_TX_BUFF_SIZE);
					break;
				}
				cec_rx_struct.buffer[i] = readb(regs + CEC_RX_BUFF0 + (i * 4));
				i++;
			}

//			tasklet_schedule(&(cec_rx_struct.input_tasklet)); /* schedule deferred tasklet*/

			cec_rx_struct.size = size;
			drime4_cec_set_rx_state(STATE_DONE);
			spin_unlock(&cec_rx_struct.lock);
			/* clear interrupt pending bit */
			writeb(CEC_IRQ_RX_DONE | CEC_IRQ_RX_ERROR,
					regs + CEC_INTR_CLEAR);
			wake_up_interruptible(&(cec_rx_struct.waitq));
		}
		drime4_cec_enable_rx();		
	}
	return IRQ_HANDLED;
}

/**
 * @brief CEC device driver probe
 * fn static int __devinit hdmi_cec_probe(struct platform_device *pdev)
 * param  struct platform_device *pdev
 * return error state
 */
static int __devinit drime4_hdmi_cec_probe(struct platform_device *pdev)
{
	u8 *buffer = NULL;
	int ret = 0;
	int i;
	struct input_dev *input;
//	struct platform_cec_dev_data *cecdata = pdev->dev.platform_data;

#if 0
	input = input_allocate_device();
	if(input == NULL)
	{
		return -1;
	}
	
	input->name = "cmd_cec";

	__set_bit(EV_KEY, input->evbit);

	for (i = 0; i < cecdata->butsize; i++) {
		struct cec_remote_button *cecbtn = &cecdata->cec_btn[i];

		input_set_capability(input, cecbtn->type, cecbtn->code);
	}
#endif

	ret = misc_register(&drime4_cec_misc_device);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		goto error;
	}

	ioarea = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!ioarea) {
		dev_err(&pdev->dev, "failed to get IO memory region.\n");
		ret = -EINVAL;
		goto error;
	}

#if 0
	ioarea = request_mem_region(ioarea->start, resource_size(ioarea), pdev->name);
	if (ioarea == NULL) {
		dev_err(&pdev->dev, "Failed to request IO\n");
		goto error;
	}
#endif

	regs = ioremap(ioarea->start, resource_size(ioarea));
	if (regs == NULL) {
		dev_err(&pdev->dev, "cannot map IO\n");
		goto error;
	}

	init_waitqueue_head(&(cec_rx_struct.waitq));
	spin_lock_init(&cec_rx_struct.lock);
	init_waitqueue_head(&(cec_tx_struct.waitq));

	buffer = kmalloc(CEC_TX_BUFF_SIZE, GFP_KERNEL);
	if (!buffer) {
		dev_err(&pdev->dev, "CEC: kmalloc() failed!\n");
		goto error;
	}
	cec_rx_struct.buffer = buffer;
	cec_rx_struct.size = 0;
//	cec_rx_struct.input = input;

#if 0
	ret = input_register_device(input);
	if (ret) {
		dev_err(&pdev->dev, "Could not register input device\n");
		/* input to evdev is not primary task of CEC. If necessary in future,
		 we can uncomment the following line */
		/* goto error; */
	}

	/* initialize the tasklet for deferred task */
	tasklet_init(&cec_rx_struct.input_tasklet, send_deferred_input,
			(unsigned long) &cec_rx_struct);
	
#endif
	if (request_irq(IRQ_HDMI, drime4_cec_irq_handler, IRQF_SHARED,
			HDMI_CEC_MODULE_NAME, (void*)pdev)) {
		printk(KERN_WARNING "CEC: IRQ %d is not free.\n", IRQ_HDMI);
		return -EIO;
	}
	return 0;

error:
    if (ioarea) {
		release_mem_region(ioarea->start, resource_size(ioarea));
		release_resource(ioarea);
	}
	if (regs)
		iounmap(regs);
	if (cec_rx_struct.buffer)
		kfree(cec_rx_struct.buffer);
//	input_unregister_device(input);
	misc_deregister(&drime4_cec_misc_device);

	return -EBUSY;
}

/**
 * brief CEC device driver resource release function
 * fn static int hdmi_cec_remove(struct platform_device *pdev)
 * param struct platform_device *pdev
 * return 0
 */
static int drime4_hdmi_cec_remove(struct platform_device *pdev)
{
	/* Unregister from the input subsystem */
//	input_unregister_device(cec_rx_struct.input);
	free_irq(IRQ_HDMI, (void*)pdev);
	kfree(cec_rx_struct.buffer);
	iounmap(regs);
	release_mem_region(ioarea->start, resource_size(ioarea));
	release_resource(ioarea);
	misc_deregister(&drime4_cec_misc_device);

	return 0;
}

/**
 * @brief Set CEC divider value.
 * @fn void drime4_cec_set_divider(void)
 * @param void
 * @return void <br>
 */
void drime4_cec_set_divider(void)
{
	/**
	 * compute divider value
	 * (CEC_DIVISOR) * (clock cycle time) = 0.05ms \n
	 * Ex.:	54MHz   0x0A8C (2700)
	 * 		30MHz   0x05DC (1500)
	 * 		133MHz  0x19FA (6650)
	 *		162MHz  0x1FA4 (8100)
	 * 		200Mhz  0x2710
	 */
	writeb(0x1F, regs + CEC_DIVISOR_1);
	writeb(0xA4, regs + CEC_DIVISOR_0);
}

/**
 * @brief Disable CEC Rx engine
 * @fn void drime4_cec_disable_rx(void)
 * @param void
 * @return void <br>
 */
void drime4_cec_disable_rx(void)
{
	unsigned char reg;
	reg = readb(regs + CEC_RX_CTRL);
	reg &= ~CEC_RX_CTRL_ENABLE;
	writeb(reg, regs + CEC_RX_CTRL);
}


/**
 * @brief Enable CEC Rx engine
 * @fn void drime4_cec_enable_rx(void)
 * @param void
 * @return void <br>
 */
void drime4_cec_enable_rx(void)
{
	unsigned char reg;
	reg = readb(regs + CEC_RX_CTRL);
	reg |= CEC_RX_CTRL_ENABLE | CEC_RX_CTRL_CHECK_START_BIT_ERROR |
		CEC_RX_CTRL_CHECK_LOW_TIME_ERROR | CEC_RX_CTRL_CHECK_SAMPLING_ERROR;
	writeb(reg, regs + CEC_RX_CTRL);
}

/**
 * @brief Mask CEC Rx interrupts
 * @fn void drime4_cec_mask_rx_interrupts(void)
 * @param void
 * @return void <br>
 */
void drime4_cec_mask_rx_interrupts(void)
{
	unsigned char reg;
	reg = readb(regs + CEC_INTR_MASK);
	reg |= CEC_IRQ_RX_DONE;
	reg |= CEC_IRQ_RX_ERROR;
	writeb(reg, regs + CEC_INTR_MASK);
}

/**
 * @brief Unmask CEC Rx interrupts
 * @fn void drime4_cec_unmask_rx_interrupts(void)
 * @param void
 * @return void <br>
 */
void drime4_cec_unmask_rx_interrupts(void)
{
	unsigned char reg;
	reg = readb(regs + CEC_INTR_MASK);
	reg &= ~CEC_IRQ_RX_DONE;
	reg &= ~CEC_IRQ_RX_ERROR;
	writeb(reg, regs + CEC_INTR_MASK);
}

/**
 * @brief Mask CEC Tx interrupts
 * @fn void drime4_cec_mask_tx_interrupts(void)
 * @param void
 * @return void <br>
 */
void drime4_cec_mask_tx_interrupts(void)
{
	unsigned char reg;
	reg = readb(regs + CEC_INTR_MASK);
	reg |= CEC_IRQ_TX_DONE;
	reg |= CEC_IRQ_TX_ERROR;
	writeb(reg, regs + CEC_INTR_MASK);
}

/**
 * @brief Unmask CEC Tx interrupts
 * @fn void drime4_cec_unmask_tx_interrupts(void)
 * @param void
 * @return void <br>
 */
void drime4_cec_unmask_tx_interrupts(void)
{
	unsigned char reg;
	reg = readb(regs + CEC_INTR_MASK);
	reg &= ~CEC_IRQ_TX_DONE;
	reg &= ~CEC_IRQ_TX_ERROR;
	writeb(reg, regs + CEC_INTR_MASK);
}

/**
 * @brief Change CEC Tx state to state
 * @fn void drime4_cec_set_tx_state(enum hdmi_cec_state state)
 * @param state [in] new CEC Tx state.
 * @return void <br>
 */
void drime4_cec_set_tx_state(enum hdmi_cec_state state)
{
	atomic_set(&cec_tx_struct.state, state);
}

/**
 * @brief Change CEC Rx state.
 * @fn void drime4_cec_set_rx_state(enum hdmi_cec_state state)
 * @param state [in] new CEC Rx state.
 * @return void <br>
 */
void drime4_cec_set_rx_state(enum hdmi_cec_state state)
{
	atomic_set(&cec_rx_struct.state, state);
}

#if 0
/*
 * This runs as a tasklet when sending an input key report
 */
static void send_deferred_input(unsigned long _cec_rx_struct)
{
	struct hdmi_cec_rx_struct *in_cec_rx_struct = (void *) _cec_rx_struct;
	unsigned int operation;

	if (in_cec_rx_struct->buffer[1] == 0x44) {

		operation = in_cec_rx_struct->buffer[2];

		if (operation == 0x00) {
			input_report_key(in_cec_rx_struct->input, 0xb5, 1);
			input_report_key(in_cec_rx_struct->input, 0xb5, 0);
		} else if (operation == 0x01) {
			input_report_key(in_cec_rx_struct->input, 0xb6, 1);
			input_report_key(in_cec_rx_struct->input, 0xb6, 0);
		} else if (operation == 0x02) {
			input_report_key(in_cec_rx_struct->input, 0xb7, 1);
			input_report_key(in_cec_rx_struct->input, 0xb7, 0);
		} else if (operation == 0x03) {
			input_report_key(in_cec_rx_struct->input, 0xb8, 1);
			input_report_key(in_cec_rx_struct->input, 0xb8, 0);
		} else if (operation == 0x04) {
			input_report_key(in_cec_rx_struct->input, 0xb9, 1);
			input_report_key(in_cec_rx_struct->input, 0xb9, 0);
		} else if (operation == 0x0D) {
			input_report_key(in_cec_rx_struct->input, 0xba, 1);
			input_report_key(in_cec_rx_struct->input, 0xba, 0);
		} else if (operation == 0x71) {
			input_report_key(in_cec_rx_struct->input, 0xbb, 1);
			input_report_key(in_cec_rx_struct->input, 0xbb, 0);
		} else if (operation == 0x72) {
			input_report_key(in_cec_rx_struct->input, 0xbc, 1);
			input_report_key(in_cec_rx_struct->input, 0xbc, 0);
		} else if (operation == 0x73) {
			input_report_key(in_cec_rx_struct->input, 0xbd, 1);
			input_report_key(in_cec_rx_struct->input, 0xbd, 0);
		} else if (operation == 0x74) {
			input_report_key(in_cec_rx_struct->input, 0xbe, 1);
			input_report_key(in_cec_rx_struct->input, 0xbe, 0);
		}
	} else {
		input_report_key(in_cec_rx_struct->input, in_cec_rx_struct->buffer[1],
				1);
		input_report_key(in_cec_rx_struct->input, in_cec_rx_struct->buffer[1],
				0);
	}

	input_sync(in_cec_rx_struct->input);

}
#endif

struct platform_driver drime4_cec_driver = {
		.probe = drime4_hdmi_cec_probe, .remove = drime4_hdmi_cec_remove,
		.driver = { .name = HDMI_CEC_MODULE_NAME, .owner = THIS_MODULE, }, };


