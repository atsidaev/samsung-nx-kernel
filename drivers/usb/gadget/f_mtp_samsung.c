/*
 * drivers/usb/gadget/f_mtp.c
 *
 * Function Driver for USB MTP,
 * mtpg.c -- MTP Driver, for MTP development,
 *
 * Copyright (C) 2009 by Samsung Electronics,
 * Author:Deepak M.G. <deepak.guru@samsung.com>,
 * Author:Madhukar.J  <madhukar.j@samsung.com>,
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
 */

/*
 * f_mtp.c file is the driver for MTP device. Totally three
 * EndPoints will be configured in which 2 Bulk End Points
 * and 1 Interrupt End point. This driver will also register as
 * misc driver and exposes file operation funtions to user space.
 */

/* Includes */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/usb.h>
#include <linux/usb_usual.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>

#include <linux/sched.h>
#include <asm-generic/siginfo.h>

#include "f_mtp.h"
#include "gadget_chips.h"


//#include "config.c"
/*-------------------------------------------------------------------------*/
/*Only for Debug*/
#define DEBUG_MTP 1
/*#define CSY_TEST */

#define USB_DRIME4

#if DEBUG_MTP
#define DEBUG_MTP_SETUP
#define DEBUG_MTP_READ
#define DEBUG_MTP_WRITE

#else
#undef DEBUG_MTP_SETUP
#undef DEBUG_MTP_READ
#undef DEBUG_MTP_WRITE
#endif


#define DEBUG_MTP_SETUP
#define DEBUG_MTP_READ
//#define DEBUG_MTP_WRITE


#ifdef DEBUG_MTP_SETUP
#define DEBUG_MTPB(fmt,args...) printk(fmt, ##args)
#else
#define DEBUG_MTPB(fmt,args...) do {} while(0)
#endif

#ifdef DEBUG_MTP_READ
#define DEBUG_MTPR(fmt,args...) printk(fmt, ##args)
#else
#define DEBUG_MTPR(fmt,args...) do {} while(0)
#endif

#ifdef DEBUG_MTP_WRITE
#define DEBUG_MTPW(fmt,args...) printk(fmt, ##args)
#else
#define DEBUG_MTPW(fmt,args...) do {} while(0)
#endif
/*-------------------------------------------------------------------------*/

#define BULK_BUFFER_SIZE	 4096

/* number of rx and tx requests to allocate */
#define RX_REQ_MAX		 4
#define TX_REQ_MAX		 4

#define DRIVER_NAME		 "usb_mtp_gadget"


#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
/* soonyong.cho : Below name is used for samsung composite framework. */
#include <linux/usb/android_composite.h>
static const char longname[] = 	"mtp";
#else
static const char longname[] = 	"Gadget_MTP";
#endif

static DEFINE_MUTEX(mtp_lock);
static const char shortname[] = DRIVER_NAME;
static pid_t mtp_pid;	//static int mtp_pid;
typedef enum {
mtp_disable_desc = 0,  // 0
mtp_enable_desc  // 1
} mtp_desc_t;

struct mtp_ep_descs {
	struct usb_endpoint_descriptor	*bulk_in;
	struct usb_endpoint_descriptor	*bulk_out;
	struct usb_endpoint_descriptor	*int_in;
};

struct f_mtp {
	struct usb_function function;
	unsigned allocated_intf_num;

	struct mtp_ep_descs	fs;
	struct mtp_ep_descs	hs;

	struct usb_ep		*bulk_in;
	struct usb_ep		*bulk_out;
	struct usb_ep		*int_in;
	struct usb_request	*notify_req;

	struct list_head 	bulk_in_q;
	struct list_head 	bulk_out_q;

};

/* MTP Device Structure*/
struct mtpg_dev {
	struct f_mtp *mtp_func;
	spinlock_t		lock;

	int			online;
	int 			error;

	wait_queue_head_t 	read_wq;
	wait_queue_head_t 	write_wq;

	struct usb_request 	*read_req;
	unsigned char 		*read_buf;
	unsigned 		read_count;

	atomic_t 		read_excl;
	atomic_t 		write_excl;
	atomic_t 		ioctl_excl;
	atomic_t 		open_excl;
	atomic_t 		wintfd_excl;

	struct list_head 	*tx_idle;
	struct list_head 	*rx_idle;
	struct list_head 	rx_done;

	char cancel_io_buf[USB_PTPREQUEST_CANCELIO_SIZE+1];

};

static inline struct f_mtp *func_to_mtp(struct usb_function *f)
{
	return container_of(f, struct f_mtp, function);
}

/* Global mtpg_dev Structure
* the_mtpg variable be used between mtpg_open() and mtpg_function_bind() */
static struct mtpg_dev    *the_mtpg;

/* Three full-speed and high-speed endpoint descriptors: bulk-in, bulk-out,
 * and interrupt-in. */

#define INT_MAX_PACKET_SIZE 10

//static struct usb_interface_descriptor mtpg_interface_desc = {
struct usb_interface_descriptor mtpg_interface_desc = {
	.bLength =		sizeof mtpg_interface_desc,
	.bDescriptorType =	USB_DT_INTERFACE,
	.bNumEndpoints =	3,
	.bInterfaceClass =	USB_CLASS_STILL_IMAGE,
	.bInterfaceSubClass =	01,
	.bInterfaceProtocol =	01,
};

static struct usb_endpoint_descriptor fs_mtpg_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	/* wMaxPacketSize set by autoconfiguration */
};

static struct usb_endpoint_descriptor fs_mtpg_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	/* wMaxPacketSize set by autoconfiguration */
};

static struct usb_endpoint_descriptor int_fs_notify_desc = {
        .bLength =              USB_DT_ENDPOINT_SIZE,
        .bDescriptorType =      USB_DT_ENDPOINT,
        .bEndpointAddress =     USB_DIR_IN,
        .bmAttributes =         USB_ENDPOINT_XFER_INT,
        .wMaxPacketSize =       __constant_cpu_to_le16(64),
        .bInterval =            0x04,
};

static struct usb_descriptor_header *fs_mtpg_desc[] = {
	(struct usb_descriptor_header *) &mtpg_interface_desc,
	(struct usb_descriptor_header *) &fs_mtpg_in_desc,
	(struct usb_descriptor_header *) &fs_mtpg_out_desc,
	(struct usb_descriptor_header *) &int_fs_notify_desc,
	NULL,
};

static struct usb_endpoint_descriptor hs_mtpg_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	/* bEndpointAddress copied from fs_mtpg_in_desc during mtpg_function_bind() */
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_mtpg_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	/* bEndpointAddress copied from fs_mtpg_out_desc during mtpg_function_bind() */
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
	.bInterval =		1,	/* NAK every 1 uframe */ 
};

static struct usb_endpoint_descriptor int_hs_notify_desc = {
        .bLength =              USB_DT_ENDPOINT_SIZE,
        .bDescriptorType =      USB_DT_ENDPOINT,
        .bEndpointAddress =     USB_DIR_IN,
        .bmAttributes =         USB_ENDPOINT_XFER_INT,
        .wMaxPacketSize =       __constant_cpu_to_le16(64),
	.bInterval =            INT_MAX_PACKET_SIZE + 4,
};

static struct usb_descriptor_header *hs_mtpg_desc[] = {
	(struct usb_descriptor_header *) &mtpg_interface_desc,
	(struct usb_descriptor_header *) &hs_mtpg_in_desc,
	(struct usb_descriptor_header *) &hs_mtpg_out_desc,
	(struct usb_descriptor_header *) &int_hs_notify_desc,
	NULL
};

/* string IDs are assigned dynamically */
#define F_MTP_IDX			0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

/* default serial number takes at least two packets */
static char serial[] = "0123456789.0123456789.0123456789";

static struct usb_string strings_dev_mtp[] = {
	[F_MTP_IDX].s = "Samsung MTP",
	[STRING_PRODUCT_IDX].s = longname,
	[STRING_SERIAL_IDX].s = serial,
	{  },			/* end of list */
};

static struct usb_gadget_strings stringtab_mtp = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev_mtp,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_mtp,
	NULL,
};

/** added by wonjung1.kim**/
extern struct usb_endpoint_descriptor * usb_find_endpoint(
	struct usb_descriptor_header **src,
	struct usb_descriptor_header **copy,
	struct usb_endpoint_descriptor *match
);


/* -------------------------------------------------------------------------
 *	Main Functionalities Start!
 * ------------------------------------------------------------------------- */
static inline int _lock(atomic_t *excl)
{

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	if (atomic_inc_return(excl) == 1) {
		return 0;
	} else {
		atomic_dec(excl);
		return -1;
	}
}

static inline void _unlock(atomic_t *excl)
{
	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);
	atomic_dec(excl);
}

/* add a request to the tail of a list */
static void inline req_put(struct mtpg_dev *dev, struct list_head *head,
		struct usb_request *req)
{
	unsigned long flags;

	if (!dev || !req)
		return;

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	spin_lock_irqsave(&dev->lock, flags);
	if (head)
		list_add_tail(&req->list, head);
	spin_unlock_irqrestore(&dev->lock, flags);
}

/* remove a request from the head of a list */
static struct usb_request *req_get(struct mtpg_dev *dev, struct list_head *head)
{
	unsigned long flags;
	struct usb_request *req;

	if (!dev)
		return 0;

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	spin_lock_irqsave(&dev->lock, flags);
	if (!head)
		req = NULL;
	else {
		if (list_empty(head)) {
			req = NULL;
		}
		else {
			req = list_first_entry(head, struct usb_request, list);
			list_del(&req->list);
		}
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	return req;
}

static int mtp_send_signal(int value)
{
	int ret;
	struct siginfo info;
	struct task_struct *t;
	memset(&info, 0, sizeof(struct siginfo));
	info.si_signo = SIG_SETUP;
	info.si_code = SI_QUEUE;
	info.si_int = value;
	rcu_read_lock();
	t = find_task_by_vpid(mtp_pid);
	if(t == NULL){
		printk("no such pid\n");
		rcu_read_unlock();
		return -ENODEV;
	}

	rcu_read_unlock();
	ret = send_sig_info(SIG_SETUP, &info, t);    //send the signal
	if (ret < 0) {
		printk("error sending signal !!!!!!!!\n");
		return ret;
	}
	return 0;

}

static int mtpg_open(struct inode *ip, struct file *fp)
{
	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	if (_lock(&the_mtpg->open_excl)){
		printk("mtpg_open fn -- mtpg device busy\n");
		return -EBUSY;
	}

	fp->private_data = the_mtpg;

	/* clear the error latch */

	DEBUG_MTPB("[%s] mtpg_open and clearing the error = 0 \n", __func__);

	the_mtpg->error = 0;

	return 0;
}

static ssize_t mtpg_read(struct file *fp, char __user *buf,
				size_t count, loff_t *pos)
{
	struct mtpg_dev *dev = fp->private_data;
	struct usb_request *req;
	int r = count, xfer;
	int ret;

	DEBUG_MTPR("*******[%s] and count = (%d)\n",__func__, count);

	if (_lock(&dev->read_excl)){
		printk("mtpg_read fn -- mtpg device busy\n");
		DEBUG_MTPR("*******[%s] mtpg_read fn -- mtpg device busy\n",__func__, count);
		return -EBUSY;
	}

	while (!(dev->online || dev->error)) {
		DEBUG_MTPR("******[%s] and line is = %d \n", __FUNCTION__,__LINE__);
		ret = wait_event_interruptible(dev->read_wq,(dev->online || dev->error));
        //init_waitqueue_head(&dev->read_wq);

		if (ret < 0) {
			_unlock(&dev->read_excl);
			printk("-------line is = %d,mtp_read ret<0 \n",__LINE__);
			return ret;
		}
	}
	DEBUG_MTPR("******[%s] and line is = %d \n", __FUNCTION__,__LINE__);

	while (count > 0) {
		DEBUG_MTPR("*********[%s] and line is = %d \n", __FUNCTION__,__LINE__);

		if (dev->error) {
			r = -EIO;
			printk("*******[%s]\t%d: dev->error so break r=%d\n",__FUNCTION__,__LINE__,r);
			break;
		}

		/* if we have idle read requests, get them queued */
		DEBUG_MTPR("*********[%s]\t%d: get request \n", __FUNCTION__,__LINE__);
		while (!!(req = req_get(dev, dev->rx_idle))) {
requeue_req:
			mutex_lock(&mtp_lock);
			if (!dev->mtp_func)
				ret = -ENODEV;
			else {
				DEBUG_MTPR("[%s]\t%d: ---------- usb-ep-queue \n", __FUNCTION__,__LINE__);
				req->length = BULK_BUFFER_SIZE;
				//ret = usb_ep_queue(dev->mtp_func->bulk_out, req, GFP_ATOMIC);
				ret = usb_ep_queue(dev->mtp_func->bulk_out, req, GFP_KERNEL);	// HSW TEST M
			}
			mutex_unlock(&mtp_lock);

			if (ret < 0) {
				r = ret;
				dev->error = 1;
				req_put(dev, dev->rx_idle, req);
				printk("*****[%s] \t line %d, RETURN ERROR r = %d !!!!!!!!! \n", __FUNCTION__,__LINE__,r);
				goto fail;
			} else {
				DEBUG_MTPR("********* [%s] rx req queue %p\n",__FUNCTION__, req);
			}
		}

		DEBUG_MTPR("*******[%s]\t%d: read_count = %d\n", __FUNCTION__,__LINE__, dev->read_count);

		/* if we have data pending, give it to userspace */
		if (dev->read_count > 0) {
			DEBUG_MTPR("*******[%s]\t%d: read_count = %d\n", __FUNCTION__,__LINE__, dev->read_count);
			if (dev->read_count < count) {
				xfer = dev->read_count;
			}
			else {
				xfer = count;
			}

			mutex_lock(&mtp_lock);
			if (!dev->mtp_func) {
				mutex_unlock(&mtp_lock);
				r = -ENODEV;
				break;
			} else if (copy_to_user(buf, dev->read_buf, xfer)) {
				mutex_unlock(&mtp_lock);
				r = -EFAULT;
				DEBUG_MTPR("*****[%s]\t%d: copy-to-user failed so RET r = %d!!!!!!!\n",__FUNCTION__,__LINE__,r);
				break;
			}
			mutex_unlock(&mtp_lock);

			dev->read_buf += xfer;
			dev->read_count -= xfer;
			buf += xfer;
			count -= xfer;

			/* if we've emptied the buffer, release the request */
			if (dev->read_count == 0) {
				DEBUG_MTPR("******[%s] and line is = %d \n", __FUNCTION__,__LINE__);
				req_put(dev, dev->rx_idle, dev->read_req);
				dev->read_req = NULL;	
			}

			/* Updating the buffer size and returnung from mtpg_read */
			r = xfer;
			DEBUG_MTPR("***** [%s] \t %d: returning lenght %d\n", __FUNCTION__,__LINE__,r);
			goto fail;
		}

		/* wait for a request to complete */
		req = 0;
		DEBUG_MTPR("*******[%s] and line is = %d \n", __FUNCTION__,__LINE__);

		ret = wait_event_interruptible(dev->read_wq, (!!(req = req_get(dev, &dev->rx_done)) || dev->error));

		DEBUG_MTPR("*******[%s]\t%d: dev->error %d and req = %p \n", __FUNCTION__,__LINE__,dev->error, req);

		if (req) {
			/* if we got a 0-len one we need to put it back into
			** service.  if we made it the current read req we'd
			** be stuck forever
			*/
			if (req->actual == 0)
				goto requeue_req;

			dev->read_req = req;
			dev->read_count = req->actual;
			dev->read_buf = req->buf;

			DEBUG_MTPR("******[%s]\t%d: rx_req=%p req->actual=%d \n",__FUNCTION__,__LINE__, req, req->actual);
		}

		if (ret < 0) {
			r = ret;
			DEBUG_MTPR("***** [%s]\t%d after ret=%d so break return = %d\n",__FUNCTION__,__LINE__, ret, r);
			break;
		}
	}

fail:
	_unlock(&dev->read_excl);

	DEBUG_MTPR("******[%s]\t%d: RETURNING Bact to USpace r=%d + + + +  + + + \n",__FUNCTION__,__LINE__,r);
	return r;

}

static ssize_t mtpg_write(struct file *fp, const char __user *buf,
				 size_t count, loff_t *pos)
{
	struct mtpg_dev *dev = fp->private_data;
	struct usb_request *req = 0;
	int r = count, xfer;
	int ret;


	if (_lock(&dev->write_excl)) {
		DEBUG_MTPW("USB/Driver/f_mtp.c mtpg_write dev->write_exel _lock\n");
		return -EBUSY;
		}

	while (count > 0) {
		if (dev->error) {
			r = -EIO;
			printk("***** [%s]\t%d count > 0 but dev->error so break !!!!!!\n",__FUNCTION__, __LINE__);
			break;
		}

		/* get an idle tx request to use */
		req = 0;

		ret = wait_event_interruptible(dev->write_wq, (!!(req = req_get(dev, dev->tx_idle)) || dev->error));

		DEBUG_MTPW("[%s]\t%d: count : %d, dev->error = %d\n",__FUNCTION__, __LINE__, count, dev->error);

		if (ret < 0) {
			r = ret;
			printk("***** [%s]\t%d ret = %d !!!!!!\n",__FUNCTION__, __LINE__,r);
			break;
		}

		if (req) {
			if (count > BULK_BUFFER_SIZE) {
				xfer = BULK_BUFFER_SIZE;
			}
			else{
				xfer = count;
			}

			DEBUG_MTPW("***** [%s]\t%d copy_from_user length %d \n",__FUNCTION__, __LINE__,xfer);

			mutex_lock(&mtp_lock);
			if (!dev->mtp_func) {
				mutex_unlock(&mtp_lock);
				r = -ENODEV;
				break;
			} else if (copy_from_user(req->buf, buf, xfer)) {
				mutex_unlock(&mtp_lock);
				r = -EFAULT;
				break;
			}

			req->length = xfer;
			ret = usb_ep_queue(dev->mtp_func->bulk_in, req, GFP_ATOMIC);
			mutex_unlock(&mtp_lock);

			if (ret < 0) {
				DEBUG_MTPW("********** mtpg_write after ep_queue ret < 0 brk\n");
				dev->error = 1;
				r = ret;
				DEBUG_MTPW("***** [%s]\t%d after ep_queue ret=%d so break return = %d\n",__FUNCTION__,__LINE__, ret, r);
				break;
			}

			buf += xfer;
			count -= xfer;

			/* zero this so we don't try to free it on error exit */
			req = NULL;
		}
	}

	if (req){
		DEBUG_MTPW("[%s] \t%d  req_put \n", __FUNCTION__,__LINE__ );
		req_put(dev, dev->tx_idle, req);
	}
	_unlock(&dev->write_excl);

	DEBUG_MTPW("[%s]\t%d  RETURN back to USpace r=%d + + + + + + + + + + \n", __FUNCTION__,__LINE__,r );
	return r;
}

/*Fixme for Interrupt Transfer*/
static void interrupt_complete(struct usb_ep *ep, struct usb_request *req )
{
	DEBUG_MTPW("[%s]\t%d******** Finished Writing Interrupt Data \n",  __FUNCTION__,__LINE__);
}

static ssize_t interrupt_write(struct file *fd, const char __user *buf, size_t count)
{
	struct mtpg_dev *dev = fd->private_data;
	struct usb_request *req;
	int  ret;

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);
	req = dev->mtp_func->notify_req;

	if( !req )
		printk("Alloc has failed \n");

	if(_lock(&dev->wintfd_excl)) {
		printk("write failed on interrupt endpoint \n");
		return -EBUSY;
	}

	if(copy_from_user(req->buf, buf, count)) {
		printk("copy from user has failed\n");
		return -EIO;
	}

	req->length = count;
	req->complete = interrupt_complete;

	ret = usb_ep_queue(dev->mtp_func->int_in, req, GFP_ATOMIC);	

	if( ret < 0 )
		return -EIO;

	_unlock(&dev->wintfd_excl);
	return ret;
}

/*Fixme for enabling and disabling the MTP*/
static long  mtpg_ioctl(struct file *fd, unsigned int code, unsigned long arg)
{

	struct mtpg_dev		*dev = fd->private_data;
	struct usb_composite_dev *cdev;
	struct usb_request	*req;
	int status = 0;
	int size = 0;
	int ret_value = 0;
	int max_pkt = 0;

	char *buf_ptr = NULL;
	char buf[USB_PTPREQUEST_GETSTATUS_SIZE+1] = {0};

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

 	if (code == SET_MTP_USER_PID) {
#if 1
		mtp_pid = arg;
		DEBUG_MTPB("[%s] SET_MTP_USER_PID; pid = %d \tline = [%d] \n", __func__,mtp_pid,__LINE__);
#else
		copy_from_user(&mtp_pid, arg, sizeof(pid_t));
		DEBUG_MTPB("[%s] SET_MTP_USER_PID; pid = %d \tline = [%d] \n", __func__,mtp_pid,__LINE__);
#endif

		return status;
	}

	mutex_lock(&mtp_lock);
	if (!dev->mtp_func) {
		DEBUG_MTPB("mtpg_ioctl fail, usb not yet enabled for MTP\n");
		mutex_unlock(&mtp_lock);
		return -ENODEV;
	}
	cdev = dev->mtp_func->function.config->cdev;
	req = cdev->req;

	printk("[%s L:%d] code = %d \n", __func__, __LINE__, code);
	
	switch (code) {
		case MTP_CLEAR_HALT:
			status = usb_ep_clear_halt (dev->mtp_func->bulk_in);
			status = usb_ep_clear_halt (dev->mtp_func->bulk_out);
			break;

		case MTP_WRITE_INT_DATA:
			DEBUG_MTPB("[%s] \t %d MTP intrpt_Write \n",__func__,__LINE__);
			ret_value = interrupt_write(fd, (const char *)arg, MTP_MAX_PACKET_LEN_FROM_APP );
			if(ret_value < 0){
				printk("[%s] \t %d interrupt-fd failed \n", __func__,__LINE__);
				status = -EIO;
			}
			else {
				printk("[%s] \t %d interrupt fd success \n", __func__,__LINE__);
				status = MTP_MAX_PACKET_LEN_FROM_APP;
			}
			break;

		case GET_SETUP_DATA:
			buf_ptr = (char *)arg;
			DEBUG_MTPB("[%s] GET_SETUP_DATA\tline = [%d] \n", __func__,__LINE__);
			if (copy_to_user(buf_ptr, dev->cancel_io_buf, USB_PTPREQUEST_CANCELIO_SIZE)) {
				status = -EIO;
				DEBUG_MTPR("*****[%s]\t%d: copy-to-user failed!!!!\n",__FUNCTION__,__LINE__);
			}
			break;
		case SEND_RESET_ACK:
			//req->zero = 1;
			req->length = 0;
			//printk("[%s] SEND_RESET_ACK and usb_ep_queu 0 data size = %d\tline = [%d] \n", __func__,size,__LINE__);
			status = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
			if (status < 0)
					DEBUG_MTPB("[%s] Error at usb_ep_queue\tline = [%d] \n", __func__,__LINE__);
			break;
		case SET_SETUP_DATA:
			buf_ptr = (char *)arg;
			if (copy_from_user(buf, buf_ptr, USB_PTPREQUEST_GETSTATUS_SIZE)) {
				status = -EIO;
				DEBUG_MTPR("*****[%s]\t%d: copy-from-user failed!!!!\n",__FUNCTION__,__LINE__);
				break;
			}
			size = buf[0];
			DEBUG_MTPB("[%s] SET_SETUP_DATA; data size = %d\tline = [%d] \n", __func__,size,__LINE__);
			memcpy(req->buf, buf, size);
			req->zero = 0;
			req->length = size;
			status = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
			if (status < 0)
					DEBUG_MTPB("[%s] Error at usb_ep_queue\tline = [%d] \n", __func__,__LINE__);
			break;
		case SET_ZLP_DATA:
			//req->zero = 1;
			status = wait_event_interruptible(dev->write_wq, (!!(req = req_get(dev, dev->tx_idle)) || dev->error));
			if (status < 0 || dev->error == 1) {
				printk("***** [%s]\t%d status = %d !!!!!\n", __func__,__LINE__,status);
				break;
			}
			/* static analysis bug fix 2012-09-27 Han Seungwon */
			if(req == NULL)
			{
				break;
			}
			req->length = 0;
			printk("[%s] SEND_ZLP_DATA and usb_ep_queu 0 data size = %d\tline = [%d] \n", __func__,size,__LINE__);
			status = usb_ep_queue(dev->mtp_func->bulk_in, req, GFP_ATOMIC);
			if (status < 0) {
				printk("[%s] Error at usb_ep_queue\tline = [%d] \n", __func__,__LINE__);
			} else {
				printk("[%s] usb_ep_queue passed and status = %d\tline = [%d] \n", __func__,__LINE__,status);
				status =20;
			}
			break;
		case GET_HIGH_FULL_SPEED:
			printk("[%s] GET_HIGH_FULL_SPEED and \tline = [%d] \n", __func__,__LINE__);
			max_pkt = dev->mtp_func->bulk_in->maxpacket;
			printk("[%s]  line = %d max_pkt = [%d] \n", __func__,__LINE__, max_pkt);
			if(max_pkt == 64)
				status = 64;
			else
				status =512;

			break;

		default:
			status = -ENOTTY;
	}
	mutex_unlock(&mtp_lock);
	DEBUG_MTPB("[%s] \tline = [%d] ioctl code %d\n", __func__,__LINE__,code);

	return status;
}


static int mtpg_release_device(struct inode *ip, struct file *fp)
{
	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	if(the_mtpg != NULL)
		_unlock(&the_mtpg->open_excl);

	return 0;
}

/* file operations for MTP device /dev/usb_mtp_gadget */
static struct file_operations mtpg_fops = {
	.owner   = THIS_MODULE,
	.read    = mtpg_read,
	.write   = mtpg_write,
	.open    = mtpg_open,
	.unlocked_ioctl = mtpg_ioctl,
	.release = mtpg_release_device,
};

static struct miscdevice mtpg_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = shortname,
	.fops = &mtpg_fops,
};

struct usb_request *alloc_ep_req(struct usb_ep *ep, unsigned len, gfp_t kmalloc_flags)
{
	struct usb_request	*req;

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	req = usb_ep_alloc_request(ep, kmalloc_flags);
	if (req) {
		req->length = len;
		req->buf = kmalloc(len, kmalloc_flags);
		if (!req->buf) {
			usb_ep_free_request(ep, req);
			req = NULL;
		}
	}
	return req;
}

static void mtpg_request_free(struct usb_request *req, struct usb_ep *ep)
{

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	if (req) {
		kfree(req->buf);
		usb_ep_free_request(ep, req);
	}
}

static struct usb_request *mtpg_request_new(struct usb_ep *ep, int buffer_size)
{

	struct usb_request *req = usb_ep_alloc_request(ep, GFP_KERNEL);

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	if (!req) {
		printk("******* %s \t line %d ERROR !!! \n",__FUNCTION__,__LINE__);
		return NULL;
	}

	/* now allocate buffers for the requests */
	req->buf = kmalloc(buffer_size, GFP_KERNEL);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}

	return req;
}

static void mtpg_complete_in(struct usb_ep *ep, struct usb_request *req)
{
	struct mtpg_dev *dev = the_mtpg;
	struct f_mtp *mtp_func = ep->driver_data;

	DEBUG_MTPB("[%s] \t %d req->status is = %d\n", __FUNCTION__,__LINE__, req->status);

	if (req->status != 0)
		dev->error = 1;

	req_put(dev, &mtp_func->bulk_in_q, req);
	wake_up(&dev->write_wq);
}

static void mtpg_complete_out(struct usb_ep *ep, struct usb_request *req)
{
	struct mtpg_dev *dev = the_mtpg;
	struct f_mtp *mtp_func = ep->driver_data;

	DEBUG_MTPB("[%s] \tline = [%d]req->status is = %d \n", __func__,__LINE__, req->status);
	if (req->status != 0) {
		dev->error = 1;

		DEBUG_MTPB(" [%s] \t %d dev->error is = %d for rx_idle\n", __FUNCTION__,__LINE__, dev->error);
		req_put(dev, &mtp_func->bulk_out_q, req);
	}
	else {
		DEBUG_MTPB("[%s] \t %d for rx_done \n", __FUNCTION__,__LINE__);
		req_put(dev, &dev->rx_done, req);
	}
	wake_up(&dev->read_wq);
}

/* create_bulk_endpoints() must be used after function binded */
static int create_bulk_endpoints(struct f_mtp *mtp_func,
								struct usb_endpoint_descriptor *in_desc,
								struct usb_endpoint_descriptor *out_desc,
								struct usb_endpoint_descriptor *intr_desc)
{
	struct usb_composite_dev *cdev = mtp_func->function.config->cdev;
	struct usb_request *req;
	struct mtpg_dev *dev = the_mtpg;
	struct usb_ep *ep;
	int i;

	ep = usb_ep_autoconfig(cdev->gadget, in_desc);
	if (!ep){
		printk(KERN_ERR "Error in usb_ep_autoconfig for IN DESC Failed !!!!!!!!!! \n");
		return -ENODEV;
	}
	ep->driver_data = cdev;		/* claim the endpoint */
	mtp_func->bulk_in = ep;

	ep = usb_ep_autoconfig(cdev->gadget, out_desc);
	if (!ep){
		printk(KERN_ERR "Error in usb_ep_autoconfig for OUT DESC Failed !!!!!!!!!! \n");
		return -ENODEV;
	}
	ep->driver_data = cdev;		/* claim the endpoint */
	mtp_func->bulk_out = ep;

	/* Interrupt Support for MTP */
	ep = usb_ep_autoconfig(cdev->gadget, intr_desc);
	if (!ep){
		printk(KERN_ERR "Error in usb_ep_autoconfig for INT IN DESC Failed !!!!!!!!!! \n");
		return -ENODEV;
	}
	ep->driver_data = cdev;
	mtp_func->int_in = ep;

	mtp_func->notify_req = alloc_ep_req(mtp_func->int_in,
			sizeof(struct usb_mtp_ctrlrequest) + 2,
			GFP_ATOMIC);
	if (!mtp_func->notify_req)
		return -ENOMEM;

	for (i = 0; i < RX_REQ_MAX; i++) {
		req = mtpg_request_new(mtp_func->bulk_out, BULK_BUFFER_SIZE);
		if (!req){
			DEBUG_MTPB("[%s] \t %d mtpg_request_new error \n", __FUNCTION__,__LINE__);
			goto ep_alloc_fail_out;
		}
		req->complete = mtpg_complete_out;
		req_put(dev, &mtp_func->bulk_out_q, req);
	}

	for (i = 0; i < TX_REQ_MAX; i++) {
		req = mtpg_request_new(mtp_func->bulk_in, BULK_BUFFER_SIZE);
		if (!req){
			DEBUG_MTPB("[%s] \t %d mtpg_request_new error \n", __FUNCTION__,__LINE__);
			goto ep_alloc_fail_all;
		}
		req->complete = mtpg_complete_in;
		req_put(dev, &mtp_func->bulk_in_q, req);
	}

	return 0;

ep_alloc_fail_all:
	while (!!(req = req_get(dev, &mtp_func->bulk_in_q)))
		mtpg_request_free(req, mtp_func->bulk_in);

ep_alloc_fail_out:
	while (!!(req = req_get(dev, &mtp_func->bulk_out_q)))
		mtpg_request_free(req, mtp_func->bulk_out);

	mtpg_request_free(mtp_func->notify_req, mtp_func->int_in);

	return -ENOMEM;
}

static void mtpg_function_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct mtpg_dev	*dev = the_mtpg;
	struct f_mtp *mtp_func = func_to_mtp(f);
	struct usb_request *req;

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	dev->online = 0;
	dev->error = 1;

	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);
	usb_free_descriptors(f->descriptors);

	mutex_lock(&mtp_lock);

	while (!!(req = req_get(dev, &mtp_func->bulk_out_q))) {
		mtpg_request_free(req, mtp_func->bulk_out);
		}
	while (!!(req = req_get(dev, &mtp_func->bulk_in_q))) {
		mtpg_request_free(req, mtp_func->bulk_in);
		}
	while (!!(req = req_get(dev, &dev->rx_done))) {
		mtpg_request_free(req, mtp_func->bulk_out);
		}

	mtpg_request_free(mtp_func->notify_req, mtp_func->int_in);

	kfree(mtp_func);
	dev->mtp_func = NULL;
	dev->read_req = NULL;
	dev->read_buf = NULL;
	dev->read_count = 0;

	mutex_unlock(&mtp_lock);

	wake_up(&dev->read_wq);
	wake_up(&dev->write_wq);
}

static int is_mtp_setted = 0;
static int mtpg_function_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_mtp *mtp_func = func_to_mtp(f);
	int			rc , id;

	struct usb_gadget	*gadget = c->cdev->gadget;
	struct usb_ep		*ep;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	//printk(KERN_EMERG "mtpg_function_bind() 00\n");
	is_mtp_setted = 0;

	//DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);
	//printk("[%s] \tline = [%d] \n", __func__,__LINE__);

	id = usb_interface_id(c, f);
	if(id < 0) 
	{
		printk("Error in usb_string_id Failed !!! \n");
		return id;
	}

	mtpg_interface_desc.bInterfaceNumber = id;
	mtp_func->allocated_intf_num = id;


	rc = create_bulk_endpoints(mtp_func, &fs_mtpg_in_desc, &fs_mtpg_out_desc, &int_fs_notify_desc);
	if(rc) 
	{
		printk(KERN_ERR "mtpg unable to autoconfigure all endpoints\n");
		return rc;
	}

	f->descriptors = usb_copy_descriptors(fs_mtpg_desc);
	if(!f->descriptors)
	{
		goto desc_alloc_fail;
	}

	mtp_func->fs.bulk_in = usb_find_endpoint(fs_mtpg_desc, f->descriptors, &fs_mtpg_in_desc);
	mtp_func->fs.bulk_out = usb_find_endpoint(fs_mtpg_desc, f->descriptors, &fs_mtpg_out_desc);
	mtp_func->fs.int_in = usb_find_endpoint(fs_mtpg_desc, f->descriptors, 	&int_fs_notify_desc);

	if(gadget_is_dualspeed(cdev->gadget))
	{
		DEBUG_MTPB("[%s] \tdual speed line = [%d] \n", __func__,__LINE__);
		printk("[%s] \tdual speed line = [%d] \n", __func__,__LINE__);

		/* Assume endpoint addresses are the same for both speeds */
		hs_mtpg_in_desc.bEndpointAddress = fs_mtpg_in_desc.bEndpointAddress;
		hs_mtpg_out_desc.bEndpointAddress = fs_mtpg_out_desc.bEndpointAddress;
		int_hs_notify_desc.bEndpointAddress = int_fs_notify_desc.bEndpointAddress;

		f->hs_descriptors = usb_copy_descriptors(hs_mtpg_desc);
		if(!f->hs_descriptors)
		{
			goto desc_alloc_fail;
		}

		mtp_func->hs.bulk_in = usb_find_endpoint(hs_mtpg_desc, f->hs_descriptors, &hs_mtpg_in_desc);
		mtp_func->hs.bulk_out = usb_find_endpoint(hs_mtpg_desc, f->hs_descriptors, &hs_mtpg_out_desc);
		mtp_func->hs.int_in = usb_find_endpoint(hs_mtpg_desc, f->hs_descriptors, &int_hs_notify_desc);
	}

	return 0;

desc_alloc_fail:
	if (f->descriptors)
	{
		usb_free_descriptors(f->descriptors);
	}

	//printk(KERN_EMERG "mtpg_function_bind()\n");

	return -ENOMEM;
}


static int mtpg_function_set_alt(struct usb_function *f,
		unsigned intf, unsigned alt)
{
	struct f_mtp *mtp_func = func_to_mtp(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct mtpg_dev *dev = the_mtpg;
	int ret, rc=0;

	//static int temp = 0;
	//printk(KERN_EMERG "mtpg_function_set_alt() is_mtp_setted = %d\n", is_mtp_setted);

	if(is_mtp_setted > 0)
	{
		DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);
		//wake_up(&dev->read_wq);
		//wake_up(&dev->write_wq);
		return 0;
	}

	is_mtp_setted++;

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);
	if (intf == mtp_func->allocated_intf_num) 
	{
		// Set Interrupt EP
#ifndef USB_DRIME4
		if (mtp_func->int_in->driver_data)
		{
			usb_ep_disable(mtp_func->int_in); 
		}
#endif
	        DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);
	        /**changed by wonjung1.kim **/
	        //ret = usb_ep_enable(mtp_func->int_in, ep_choose(cdev->gadget,mtp_func->hs.int_in, mtp_func->fs.int_in));
	       /* Enable the endpoints */
	    	rc = config_ep_by_speed(cdev->gadget, &(mtp_func->function), mtp_func->int_in);
	    	if (rc)
		{
			usb_ep_disable(mtp_func->int_in);
			printk("[%s]Error in config_ep_by_speed the int in endpoint, ret = %d\n",__func__, rc);
			return rc;	//return ret;	/* static analysis bug fix 2012-09-18 Han Seungwon */
		}
        	ret = usb_ep_enable(mtp_func->int_in);
		if(ret) {
			usb_ep_disable(mtp_func->int_in);
			printk("[%s]Error in enabling the int in endpoint, ret = %d\n",__func__, ret);
			return ret;
		}
		mtp_func->int_in->driver_data = mtp_func;

		// Set Bulk_IN EP
#ifndef USB_DRIME4
		if(mtp_func->bulk_in->driver_data)
		{
			usb_ep_disable(mtp_func->bulk_in);
		}
#endif
        	/**changed by wonjung1.kim **/
		//ret = usb_ep_enable(mtp_func->bulk_in, ep_choose(cdev->gadget,mtp_func->hs.bulk_in, mtp_func->fs.bulk_in));
       	/* Enable the endpoints */
		rc = config_ep_by_speed(cdev->gadget, &(mtp_func->function), mtp_func->bulk_in);
		if (rc)
		{
			usb_ep_disable(mtp_func->bulk_in);
			printk("[%s]Error in config_ep_by_speed the bulk in endpoint, ret = %d\n",__func__, ret);
			return rc;	//return ret;	/* static analysis bug fix 2012-09-18 Han Seungwon */
		}
		ret = usb_ep_enable(mtp_func->bulk_in);
		if (ret) 
		{
			usb_ep_disable(mtp_func->bulk_in);
			printk("[%s] Enable Bulk-Out EP error!!! %d\n", __FUNCTION__, __LINE__);
			return ret;
		}
		mtp_func->bulk_in->driver_data = mtp_func;

		// Set Bulk_OUT EP
#ifndef USB_DRIME4
		if(mtp_func->bulk_out->driver_data)
		{
			usb_ep_disable(mtp_func->bulk_out);
		}
#endif
		/**changed by wonjung1.kim **/
		//ret = usb_ep_enable(mtp_func->bulk_out, ep_choose(cdev->gadget,mtp_func->hs.bulk_out, mtp_func->fs.bulk_in));
		/* Enable the endpoints */
		rc = config_ep_by_speed(cdev->gadget, &(mtp_func->function), mtp_func->bulk_out);
		if (rc)
		{
			usb_ep_disable(mtp_func->bulk_out);
			printk("[%s]Error in config_ep_by_speed the bulk_out endpoint, ret = %d\n",__func__, ret);
			return rc;	//return ret;	/* static analysis bug fix 2012-09-18 Han Seungwon */
		}
		ret = usb_ep_enable(mtp_func->bulk_out);
		if (ret) 
		{
			usb_ep_disable(mtp_func->bulk_out);
			printk("[%s] Enable Bulk-In EP error!!! %d\n", __FUNCTION__, __LINE__);
			return ret;
		}
		mtp_func->bulk_out->driver_data = mtp_func;

		dev->tx_idle = &mtp_func->bulk_in_q;
		dev->rx_idle = &mtp_func->bulk_out_q;
		dev->mtp_func = mtp_func;
		dev->online = 1;

		/* readers may be blocked waiting for us to go online */
		wake_up(&dev->read_wq);

		return 0;
	}
	else 
	{
		printk(KERN_ERR "[%s] error , intf = %u , alt = %u",__func__, intf, alt);
		return -EINVAL;
	}
}

static void mtpg_function_disable(struct usb_function *f)
{

	struct mtpg_dev	*dev = the_mtpg;
	struct f_mtp *mtp_func = func_to_mtp(f);

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	is_mtp_setted = 0;

	dev->online = 0;
	dev->error = 1;
#if 1
	//printk(KERN_EMERG "mtpg_function_disable()\n");
	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);
	spin_lock(&dev->lock);
	dev->tx_idle = NULL;
	dev->rx_idle = NULL;
	spin_unlock(&dev->lock);

	usb_ep_disable(mtp_func->int_in);
	mtp_func->int_in->driver_data = NULL;

	usb_ep_disable(mtp_func->bulk_in);
	mtp_func->bulk_in->driver_data = NULL;

	usb_ep_disable(mtp_func->bulk_out);
	mtp_func->bulk_out->driver_data = NULL;

	wake_up(&dev->read_wq);
	wake_up(&dev->write_wq);
#endif
}


/*PIMA15740-2000 spec: Class specific setup request for MTP*/
static void mtp_complete_cancel_io(struct usb_ep *ep,
		struct usb_request *req)
{
	int i;
	struct mtpg_dev	*dev = ep->driver_data;
	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);
	if (req->status != 0)
	{
		DEBUG_MTPB("[%s] req->status !=0 \tline = [%d] \n", __func__,__LINE__);
		return;
	}

	if(req->length != USB_PTPREQUEST_CANCELIO_SIZE)
	{
		DEBUG_MTPB("[%s] USB_PTPREQUEST_CANCELIO_SIZE, actual:%d , length:%d\tline = [%d] \n", __func__,req->actual,req->length,__LINE__);
		usb_ep_set_halt(ep);

	}
	else
	{
		memset(dev->cancel_io_buf, 0, USB_PTPREQUEST_CANCELIO_SIZE+1);
		memcpy(dev->cancel_io_buf, req->buf, USB_PTPREQUEST_CANCELIO_SIZE);
		/*Debugging*/
		for(i=0;i<USB_PTPREQUEST_CANCELIO_SIZE; i++)
			DEBUG_MTPB("[%s] cancel_io_buf[%d] = %x \tline = [%d] \n", __func__,i,dev->cancel_io_buf[i],__LINE__);
		mtp_send_signal(USB_PTPREQUEST_CANCELIO);
	}


}
static int mtpg_function_setup(struct usb_function *f,
					const struct usb_ctrlrequest *ctrl)
{
	struct mtpg_dev	*dev = the_mtpg;
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	int signal_request = 0;
	int value = -EOPNOTSUPP;
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u16			w_length = le16_to_cpu(ctrl->wLength);

	DEBUG_MTPB("[%s] \tline = [%d] -(f->disabled)=[%d] \n", __func__,__LINE__, (f->disabled));

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
	/* soonyong.cho :  do nothing if we are disabled */
	if (f->disabled)
	{
		return value;
	}
#endif

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);
	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {

		case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_PTPREQUEST_CANCELIO:
			DEBUG_MTPB("[%s] USB_PTPREQUEST_CANCELIO \tline = [%d] \n", __func__,__LINE__);
			DEBUG_MTPB("[%s] \tline = [%d]  w_value = %x,w_index = %x, w_length = %x\n",__func__, __LINE__, w_value, w_index, w_length);
			if (w_value == 0x00 && w_index == mtpg_interface_desc.bInterfaceNumber && w_length == 0x06)
			{
				DEBUG_MTPB("[%s] read USB_PTPREQUEST_CANCELIO data \tline = [%d] \n", __func__,__LINE__);
				value = w_length;
				cdev->gadget->ep0->driver_data = dev;
				req->complete = mtp_complete_cancel_io;
				req->zero = 0;
				req->length = value;
				value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
				if (value < 0)
					printk("[%s] \tline = [%d] Error at usb_ep_queue !!!!!!!\n", __func__, __LINE__);
			}
			return value;
			break;
		case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_PTPREQUEST_RESET:
			DEBUG_MTPB("[%s] USB_PTPREQUEST_RESET \tline = [%d] \n", __func__,__LINE__);
			signal_request = USB_PTPREQUEST_RESET;
			break;

		case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_PTPREQUEST_GETSTATUS:
			DEBUG_MTPB("[%s] USB_PTPREQUEST_GETSTATUS \tline = [%d] \n", __func__,__LINE__);
			signal_request = USB_PTPREQUEST_GETSTATUS;
			break;

		case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_PTPREQUEST_GETEVENT:
			DEBUG_MTPB("[%s] USB_PTPREQUEST_GETEVENT \tline = [%d] \n", __func__,__LINE__);
			signal_request = USB_PTPREQUEST_GETEVENT;
			break;
		default:
			DEBUG_MTPB("[%s] INVALID REQUEST \tline = [%d] \n", __func__,__LINE__);
			return value;

		}
	value = mtp_send_signal(signal_request);
	return value;
}

int mtp_function_bind_upper_config(struct usb_composite_dev *cdev)
{
	struct mtpg_dev	*mtpg;
	int		status;

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	mtpg = kzalloc(sizeof(*mtpg), GFP_KERNEL);
	if (!mtpg) {
		printk("[%s] \t mtpg_dev_alloc memory  failed !!!\n", __func__);
		return -ENOMEM;
	}

	if (strings_dev_mtp[F_MTP_IDX].id == 0) {
		status = usb_string_id(cdev);
		if (status < 0) {
			kfree(mtpg);
			return status;
		}
		strings_dev_mtp[F_MTP_IDX].id = status;
		mtpg_interface_desc.iInterface = 1;		//mtpg_interface_desc.iInterface = status;
	}

	spin_lock_init(&mtpg->lock);
	init_waitqueue_head(&mtpg->read_wq);
	init_waitqueue_head(&mtpg->write_wq);

	atomic_set(&mtpg->open_excl, 0);
	atomic_set(&mtpg->read_excl, 0);
	atomic_set(&mtpg->write_excl, 0);
	atomic_set(&mtpg->wintfd_excl, 0);

	INIT_LIST_HEAD(&mtpg->rx_done);

	/* the_mtpg must be set before calling usb_gadget_register_driver */
	the_mtpg = mtpg;

	status = misc_register(&mtpg_device);
	if (status != 0){
		printk("Error in misc_register of mtpg_device Failed !!!\n");
		kfree(mtpg);
		the_mtpg = NULL;
	}

	return status;
}

int mtp_function_add(struct usb_configuration *c)
{
	int		rc;
	struct f_mtp *mtp_func;

	DEBUG_MTPB("[%s] \tline = [%d] \n", __func__,__LINE__);

	if (!the_mtpg) {
		printk("Error There is no the_mtpg!!\n");
		return -ENODEV;
	}

	mtp_func = kzalloc(sizeof(*mtp_func), GFP_KERNEL);
	if (!mtp_func) {
		printk("[%s] \t mtp_func memory alloc failed !!!\n", __func__);
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&mtp_func->bulk_out_q);
	INIT_LIST_HEAD(&mtp_func->bulk_in_q);

	mtp_func->function.name = longname;
	mtp_func->function.strings = dev_strings;

	mtp_func->function.bind = mtpg_function_bind;
	mtp_func->function.unbind = mtpg_function_unbind;
	mtp_func->function.setup = mtpg_function_setup;
	mtp_func->function.set_alt = mtpg_function_set_alt;
	mtp_func->function.disable = mtpg_function_disable;

	rc = usb_add_function(c, &mtp_func->function);
	if (rc != 0){
		printk("Error in usb_add_function Failed !!!\n");
		kfree(mtp_func);
	}

	return rc;
}

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
/* soonyong.cho : It is for using samsung composite framework. */
static struct android_usb_function mtp_function = {
	.name = "mtp",
	.bind_upper_config = mtp_function_bind_upper_config,
	.bind_config = mtp_function_add,

};

static int __init init(void)
{
	printk(KERN_ERR "f_mtp samsung init\n");
	android_register_function(&mtp_function);
	return 0;
}
module_init(init);
#endif

MODULE_AUTHOR("Deepak And Madhukar");
MODULE_LICENSE("GPL");
