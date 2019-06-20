/* linux/drivers/usb/gadget/drime4_ss_udc.c
 *
 * Copyright 2011 Samsung Electronics Co., Ltd.
 *	http://www.samsung.com/
 *
 * DRIME4 SuperSpeed USB 3.0 Device Controlle driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/kref.h>
#include <linux/kthread.h>
#include <linux/limits.h>
#include <linux/timer.h>


#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include <mach/clock.h>

#include "drime4_ss_udc.h"
//#include <mach/version_information.h>



#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
static int drime4_get_usb_mode(void);
static int drime4_change_usb_mode(int mode);
//#include <plat/udc-hs.h>
/* It must be changed when we receive new otg host driver from MCCI */
//extern struct platform_driver s5pc110_otg_driver;
#endif

static struct timer_list usb_sp_timer;



#define CONFIG_USB_GADGET_DRIME4_SS_UDC_SSMODE

unsigned int  reset_flag = 0;
unsigned int  timerflag  = 0;

static void drime4_ss_udc_enqueue_setup(struct drime4_ss_udc *udc);
static int drime4_ss_udc_ep_queue(struct usb_ep *ep, struct usb_request *req,
			      gfp_t gfp_flags);
static void drime4_ss_udc_kill_all_requests(struct drime4_ss_udc *udc,
			      struct drime4_ss_udc_ep *ep,
			      int result, bool force);
static void drime4_ss_udc_ep_activate(struct drime4_ss_udc *udc,
				      struct drime4_ss_udc_ep *udc_ep);
static void drime4_ss_udc_ep_deactivate(struct drime4_ss_udc *udc,
					struct drime4_ss_udc_ep *udc_ep);
static int drime4_ss_udc_ep_dequeue(struct usb_ep *ep, struct usb_request *req);
static void drime4_ss_udc_complete_request(struct drime4_ss_udc *udc,
				       struct drime4_ss_udc_ep *udc_ep,
				       struct drime4_ss_udc_req *udc_req,
				       int result);
static int drime4_ss_udc_pullup(struct usb_gadget *gadget, int is_on);
static int drime4_ss_udc_start(struct usb_gadget *gadget,
		struct usb_gadget_driver *driver);
static int drime4_ss_udc_stop(struct usb_gadget *gadget,
		struct usb_gadget_driver *driver);

static int drime4_ss_udc_corereset(struct drime4_ss_udc *udc);
static void drime4_ss_udc_init(struct drime4_ss_udc *udc);
static int drime4_ss_udc_ep_disable(struct usb_ep *ep);

int drime4_udc_soft_connect(struct drime4_ss_udc *udc);
void drime4_udc_soft_disconnect(struct drime4_ss_udc *udc);

static int __devinit drime4_ss_udc_initep(struct drime4_ss_udc *udc,
				       struct drime4_ss_udc_ep *udc_ep,
				       int epnum);

static struct usb_request *drime4_ss_udc_ep_alloc_request(struct usb_ep *ep,
						      gfp_t flags);

static struct drime4_ss_udc *our_udc;
static struct platform_device *our_platform_pdev;	// HSW +

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
int drime4_vbus_enable(struct usb_gadget *gadget, int enable);
#endif

static struct usb_gadget_ops drime4_ss_udc_gadget_ops = {
	.pullup		= drime4_ss_udc_pullup,
	.udc_start	= drime4_ss_udc_start,
	.udc_stop	= drime4_ss_udc_stop,
#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
	.vbus_session = drime4_vbus_enable,
#endif
};


// return : 
//	1:ON, 2:OFF
static int drime4_ss_udc_get_clk_status(void)
{
	struct drime4_ss_udc *dev = our_udc;
	struct platform_device *pdev = our_platform_pdev;
	struct device *usb_dev = &pdev->dev;

	dev->clk = clk_get(usb_dev, "usb3");
	if (IS_ERR(dev->clk)) 
	{
		printk(KERN_EMERG "drime4_ss_udc_resume() cannot get UDC clock\n");
		return -1;
	}

	return (int)dev->clk->state;
}


// param : 
//	clk_ctrl -> 1:ON, 2:OFF
static void drime4_ss_udc_clk_ctrl(int clk_ctrl)
{
	struct drime4_ss_udc *dev = our_udc;
	struct platform_device *pdev = our_platform_pdev;
	struct device *usb_dev = &pdev->dev;

	dev->clk = clk_get(usb_dev, "usb3");
	if (IS_ERR(dev->clk)) 
	{
		printk(KERN_EMERG "drime4_ss_udc_resume() cannot get UDC clock\n");
		return -1;
	}

	printk(KERN_INFO "drime4_ss_udc_clk_ctrl() 00 dev->clk->state = %d, dev->clk->refcnt = %d, clk_ctrl = %d\n", dev->clk->state, dev->clk->refcnt, clk_ctrl);

	// clk_ctrl -> 0:Uninitialize, 1:ON, 2:OFF
	if((int)dev->clk->state == clk_ctrl || clk_ctrl == 0 || clk_ctrl > 2)
	{
		return;
	}

	// 0:Uninitialize, 1:ON, 2:OFF
	switch(clk_ctrl)
	{
		case 1:
			clk_enable(dev->clk);
			break;
		case 2:
			clk_disable(dev->clk);
			clk_put(dev->clk);
			break;
		default:
			break;
	}

	mdelay(1);
	printk(KERN_INFO "drime4_ss_udc_clk_ctrl() 99 dev->clk->state = %d, dev->clk->refcnt = %d, clk_ctrl = %d\n", dev->clk->state, dev->clk->refcnt, clk_ctrl);
}

static bool drime4_ss_udc_poll_bit_set(void __iomem *ptr, u32 val, int timeout)
{
	u32 reg;

	do {
		reg = readl(ptr);
	} while (!(reg & val) && timeout-- > 0);

	if (reg & val)
		return true;

	return false;
}

static bool drime4_ss_udc_poll_bit_clear(void __iomem *ptr, u32 val,
	int timeout)
{
	u32 reg;

	do {
		reg = readl(ptr);
	} while ((reg & val) && timeout-- > 0);

	if (reg & val)
		return false;

	return true;
}

static void drime4_ss_udc_run_stop(struct drime4_ss_udc *udc, int is_on)
{
	bool res = false;

	if (is_on == 1) {

		__orr32(udc->regs + DRIME4_USB3_DCTL,
			DRIME4_USB3_DCTL_Run_Stop);
		res = drime4_ss_udc_poll_bit_clear(udc->regs + DRIME4_USB3_DSTS,
						   DRIME4_USB3_DSTS_DevCtrlHlt,
						   2000);
	} else if(is_on == 0) {

		__bic32(udc->regs + DRIME4_USB3_DCTL,
			DRIME4_USB3_DCTL_Run_Stop);
		res = drime4_ss_udc_poll_bit_set(udc->regs + DRIME4_USB3_DSTS,
						   DRIME4_USB3_DSTS_DevCtrlHlt,
						   2000);
	}

	if (!res)
		dev_err(udc->dev, "Failed %sConnect by software\n",
			is_on ? "" : "dis-");
}

static int drime4_ss_udc_pullup(struct usb_gadget *gadget, int is_on)
{

	struct drime4_ss_udc *udc =
		container_of(gadget, struct drime4_ss_udc, gadget);

	if (is_on)
	{
		drime4_ss_udc_run_stop(udc, is_on);
	}
	else
	{
		drime4_ss_udc_run_stop(udc, is_on);
	}
    return 0;

}

static int drime4_ss_udc_start(struct usb_gadget *gadget,
		struct usb_gadget_driver *driver)
{
	int ret;
	struct drime4_ss_udc *udc =
		container_of(gadget, struct drime4_ss_udc, gadget);

	if (udc->driver) {
		dev_err(udc->dev, "%s is already bound to %s\n",
				udc->gadget.name,
				udc->driver->driver.name);
		return -EBUSY;
	}

	udc->driver = driver;
	udc->gadget.dev.driver = &driver->driver;
	udc->gadget.dev.dma_mask = udc->dev->dma_mask;
	udc->gadget.speed = USB_SPEED_UNKNOWN;

	/* we must now enable ep0 ready for host detection and then
	 * set configuration. */

	ret = drime4_ss_udc_corereset(udc);
	if (ret) {
		dev_err(udc->dev, "%s coreset failed\n",
				udc->gadget.name);
		return ret;
	}

	drime4_ss_udc_init(udc);
        udc->last_rst = jiffies;
	udc->ep0_state = EP0_SETUP_PHASE;

	drime4_ss_udc_enqueue_setup(udc);
	/* report to the user, and return */

	/* static analysis bug fix 2012-09-10 Han Seungwon */
	//dev_info(udc->dev, " drime4_ss_udc_start : bound driver %s\n", driver->driver.name);
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	return 0;

}

static int drime4_ss_udc_stop(struct usb_gadget *gadget,
		struct usb_gadget_driver *driver)
{
	int ep;
	struct drime4_ss_udc *udc =
		container_of(gadget, struct drime4_ss_udc, gadget);

	if (!udc)
		return -ENODEV;

	/* all endpoints should be shutdown */
	for (ep = 0; ep < DRIME4_USB3_EPS; ep++)
		drime4_ss_udc_ep_disable(&udc->eps[ep].ep);

	call_gadget(udc, disconnect);

	udc->driver = NULL;
	udc->gadget.speed = USB_SPEED_UNKNOWN;

	/* static analysis bug fix 2012-09-10 Han Seungwon */
	//dev_info(udc->dev, "unregistered gadget driver '%s'\n", driver->driver.name);
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	return 0;

}

static bool drime4_ss_udc_issue_cmd(struct drime4_ss_udc *udc,
				 struct drime4_ss_udc_ep_command *epcmd)
{
	bool res;
	u32 depcmd;

	/* If some of parameters are not in use, we will write it anyway
	   for simplification */
	writel(epcmd->param0, udc->regs + DRIME4_USB3_DEPCMDPAR0(epcmd->ep));
	writel(epcmd->param1, udc->regs + DRIME4_USB3_DEPCMDPAR1(epcmd->ep));
	writel(epcmd->param2, udc->regs + DRIME4_USB3_DEPCMDPAR2(epcmd->ep));

	depcmd = epcmd->cmdtyp | epcmd->cmdflags;
	writel(depcmd, udc->regs + DRIME4_USB3_DEPCMD(epcmd->ep));

	res = drime4_ss_udc_poll_bit_clear(udc->regs +
					   DRIME4_USB3_DEPCMD(epcmd->ep),
					   DRIME4_USB3_DEPCMDx_CmdAct,
					   1000);
	return res;
}

/**
 * get_ep_head - return the first request on the endpoint
 * @udc_ep: The controller endpoint to get
 *
 * Get the first request on the endpoint.
*/
static struct drime4_ss_udc_req *get_ep_head(struct drime4_ss_udc_ep *udc_ep)
{
	if (list_empty(&udc_ep->queue))
		return NULL;

	return list_first_entry(&udc_ep->queue,
			struct drime4_ss_udc_req, queue);
}

/**
 * drime4_ss_udc_map_dma - map the DMA memory being used for the request
 * @udc: The device state.
 * @udc_ep: The endpoint the request is on.
 * @req: The request being processed.
 *
 * We've been asked to queue a request, so ensure that the memory buffer
 * is correctly setup for DMA. If we've been passed an extant DMA address
 * then ensure the buffer has been synced to memory. If our buffer has no
 * DMA memory, then we map the memory and mark our request to allow us to
 * cleanup on completion.
*/
static int drime4_ss_udc_map_dma(struct drime4_ss_udc *udc,
			     struct drime4_ss_udc_ep *udc_ep,
			     struct usb_request *req)
{
	enum dma_data_direction dir;
	struct drime4_ss_udc_req *udc_req = our_req(req);

	dir = udc_ep->dir_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE;

	/* if the length is zero, ignore the DMA data */
	if (udc_req->req.length == 0)
		return 0;

	if (req->dma == DMA_ADDR_INVALID) {
		dma_addr_t dma;

		dma = dma_map_single(udc->dev,
					req->buf, req->length, dir);

		if (unlikely(dma_mapping_error(udc->dev, dma)))
			goto dma_error;

		udc_req->mapped = 1;
		req->dma = dma;
	} else
		dma_sync_single_for_device(udc->dev,
				req->dma, req->length, dir);

	return 0;

dma_error:
	dev_err(udc->dev, "%s: failed to map buffer %p, %d bytes\n",
		__func__, req->buf, req->length);

	return -EIO;
}

/**
 * drime4_ss_udc_unmap_dma - unmap the DMA memory being used for the request
 * @udc: The device state.
 * @udc_ep: The endpoint for the request
 * @udc_req: The request being processed.
 *
 * This is the reverse of drime4_ss_udc_map_dma(), called for the completion
 * of a request to ensure the buffer is ready for access by the caller.
*/
static void drime4_ss_udc_unmap_dma(struct drime4_ss_udc *udc,
				struct drime4_ss_udc_ep *udc_ep,
				struct drime4_ss_udc_req *udc_req)
{
	struct usb_request *req = &udc_req->req;
	enum dma_data_direction dir;

	dir = udc_ep->dir_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE;

	/* ignore this if we're not moving any data */
	if (udc_req->req.length == 0)
		return;

	if (udc_req->mapped) {
		/* we mapped this, so unmap and remove the dma */

		dma_unmap_single(udc->dev, req->dma, req->length, dir);

		req->dma = DMA_ADDR_INVALID;
		udc_req->mapped = 0;
	}
}

/**
 * ep_from_windex - convert control wIndex value to endpoint
 * @udc: The driver state.
 * @windex: The control request wIndex field (in host order).
 *
 * Convert the given wIndex into a pointer to an driver endpoint
 * structure, or return NULL if it is not a valid endpoint.
*/
static struct drime4_ss_udc_ep *ep_from_windex(struct drime4_ss_udc *udc,
					   u32 windex)
{
	struct drime4_ss_udc_ep *ep = &udc->eps[windex & 0x7F];
	int dir = (windex & USB_DIR_IN) ? 1 : 0;
	int idx = windex & 0x7F;

	if (windex >= 0x100)
		return NULL;

	if (idx > DRIME4_USB3_EPS)
		return NULL;

	if (idx && ep->dir_in != dir)
		return NULL;

	return ep;
}

/**
 * drime4_ss_udc_ep_enable - enable the given endpoint
 * @ep: The USB endpint to configure
 * @desc: The USB endpoint descriptor to configure with.
 *
 * This is called from the USB gadget code's usb_ep_enable().
*/
static int drime4_ss_udc_ep_enable(struct usb_ep *ep,
			       const struct usb_endpoint_descriptor *desc)
{
	struct drime4_ss_udc_ep *udc_ep = our_ep(ep);
	struct drime4_ss_udc *udc = udc_ep->parent;
	unsigned long flags;
	int dir_in;
	int epnum;

    
	dev_info(udc->dev,
		"%s: ep %s: a 0x%02x, attr 0x%02x, mps 0x%04x, intr %d\n",
		__func__, ep->name, desc->bEndpointAddress, desc->bmAttributes,
		desc->wMaxPacketSize, desc->bInterval);

	/* not to be called for EP0 */
	WARN_ON(udc_ep->epnum == 0);

	epnum = (desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);
	if (epnum != udc_ep->epnum) {
		dev_err(udc->dev, "%s: EP %d number mismatch!\n", __func__, epnum);
		return -EINVAL;
	}

	dir_in = (desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) ? 1 : 0;
	if (dir_in != udc_ep->dir_in) {
		dev_err(udc->dev, "%s: EP direction mismatch!\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&udc_ep->lock, flags);

	/* update the endpoint state */
	udc_ep->ep.maxpacket = le16_to_cpu(desc->wMaxPacketSize);
	udc_ep->type = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;

	switch (udc_ep->type) {
	case USB_ENDPOINT_XFER_ISOC:
		dev_err(udc->dev, "no current ISOC support\n");
		return -EINVAL;

	case USB_ENDPOINT_XFER_BULK:
		dev_dbg(udc->dev, "Bulk endpoint\n");
		break;

	case USB_ENDPOINT_XFER_INT:
		dev_dbg(udc->dev, "Interrupt endpoint\n");
		break;

	case USB_ENDPOINT_XFER_CONTROL:
		dev_dbg(udc->dev, "Control endpoint\n");
		break;
	}

	drime4_ss_udc_ep_activate(udc, udc_ep);

	spin_unlock_irqrestore(&udc_ep->lock, flags);
	return 0;
}

/**
 * drime4_ss_udc_ep_disable - disable the given endpoint
 * @ep: The USB endpint to configure
 * @desc: The USB endpoint descriptor to configure with.
 *
 * This is called from the USB gadget code's usb_ep_disable().
*/
static int drime4_ss_udc_ep_disable(struct usb_ep *ep)
{
	struct drime4_ss_udc_ep *udc_ep = our_ep(ep);
	struct drime4_ss_udc *udc = udc_ep->parent;
	unsigned long flags;

	dev_info(udc->dev, "%s: ep%d%s\n", __func__,
			  udc_ep->epnum, udc_ep->dir_in ? "in" : "out");

#if 1
	if(drime4_ss_udc_get_clk_status() == 2)	// UDC CLK OFF
	{
		drime4_ss_udc_clk_ctrl(1);
	}
#endif


	if (ep == &udc->eps[0].ep) {
		dev_err(udc->dev, "%s: called for ep0\n", __func__);
		return -EINVAL;
	}
	spin_lock_irqsave(&udc_ep->lock, flags);
	drime4_ss_udc_ep_deactivate(udc, udc_ep);
	spin_unlock_irqrestore(&udc_ep->lock, flags);
	/* terminate all requests with shutdown */
	drime4_ss_udc_kill_all_requests(udc, udc_ep, -ESHUTDOWN, false);

#if 0
	if(drime4_ss_udc_get_clk_status() == 1)	// UDC CLK ON
	{
		drime4_ss_udc_clk_ctrl(2);
	}
#endif

	return 0;
}

/**
 * drime4_ss_udc_ep_alloc_request - allocate a request object
 * @ep: USB endpoint to allocate request for.
 * @flags: Allocation flags
 *
 * Allocate a new USB request structure appropriate for the specified endpoint
 */
static struct usb_request *drime4_ss_udc_ep_alloc_request(struct usb_ep *ep,
						      gfp_t flags)
{
	struct drime4_ss_udc_req *req;

	req = kzalloc(sizeof(struct drime4_ss_udc_req), flags);
	if (!req)
		return NULL;

	INIT_LIST_HEAD(&req->queue);

	req->req.dma = DMA_ADDR_INVALID;

	return &req->req;
}

static void drime4_ss_udc_ep_free_request(struct usb_ep *ep,
				      struct usb_request *req)
{
	struct drime4_ss_udc_ep *udc_ep = our_ep(ep);
	struct drime4_ss_udc *udc = udc_ep->parent;
	struct drime4_ss_udc_req *udc_req = our_req(req);

	dev_dbg(udc->dev, "%s: ep%d, req %p\n", __func__, udc_ep->epnum, req);

	kfree(udc_req);
}

static int drime4_ss_udc_ep_sethalt(struct usb_ep *ep, int value)
{
	struct drime4_ss_udc_ep *udc_ep = our_ep(ep);
	struct drime4_ss_udc *udc = udc_ep->parent;
	struct drime4_ss_udc_ep_command *epcmd = &udc->epcmd;
	int index = get_phys_epnum(udc_ep);
	unsigned long irqflags;
	bool res;

	dev_info(udc->dev, "%s(ep %p %s, %d)\n", __func__, ep, ep->name, value);

	spin_lock_irqsave(&udc_ep->lock, irqflags);

	epcmd->ep = index;
	if (value)
		epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPSSTALL;
	else
		epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPCSTALL;

	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

	res = drime4_ss_udc_issue_cmd(udc, epcmd);
	if (!res) {
		dev_err(udc->dev, "Failed to set/clear stall\n");
		return -EINVAL;
	}

	/* If EP0, do it for another direction */
	if (udc_ep->epnum == 0) {
		epcmd->ep = ~udc_ep->dir_in;

		res = drime4_ss_udc_issue_cmd(udc, epcmd);
		if (!res) {
			dev_err(udc->dev, "Failed to set/clear stall\n");
			return -EINVAL;
		}

		udc->ep0_state = EP0_STALL;
	}

	/* If everything is Ok, we mark endpoint as halted */
	udc_ep->halted = value ? 1 : 0;

	spin_unlock_irqrestore(&udc_ep->lock, irqflags);

	return 0;
}

static struct usb_ep_ops drime4_ss_udc_ep_ops = {
	.enable		= drime4_ss_udc_ep_enable,
	.disable	= drime4_ss_udc_ep_disable,
	.alloc_request	= drime4_ss_udc_ep_alloc_request,
	.free_request	= drime4_ss_udc_ep_free_request,
	.queue		= drime4_ss_udc_ep_queue,
	.dequeue	= drime4_ss_udc_ep_dequeue,
	.set_halt	= drime4_ss_udc_ep_sethalt,
};

/**
 * drime4_ss_udc_complete_oursetup - setup completion callback
 * @ep: The endpoint the request was on.
 * @req: The request completed.
 *
 * Called on completion of any requests the driver itself
 * submitted that need cleaning up.
 */
static void drime4_ss_udc_complete_oursetup(struct usb_ep *ep,
					struct usb_request *req)
{
	struct drime4_ss_udc_ep *udc_ep = our_ep(ep);
	struct drime4_ss_udc *udc = udc_ep->parent;

	dev_dbg(udc->dev, "%s: ep %p, req %p\n", __func__, ep, req);

	drime4_ss_udc_ep_free_request(ep, req);
}

/**
 * drime4_ss_udc_send_reply - send reply to control request
 * @udc: The device state
 * @ep: Endpoint 0
 * @buff: Buffer for request
 * @length: Length of reply.
 *
 * Create a request and queue it on the given endpoint. This is useful as
 * an internal method of sending replies to certain control requests, etc.
 */
static int drime4_ss_udc_send_reply(struct drime4_ss_udc *udc,
				struct drime4_ss_udc_ep *ep,
				void *buff,
				int length)
{
	struct usb_request *req;
	struct drime4_ss_udc_req *udc_req;
	int ret;

	dev_dbg(udc->dev, "%s: buff %p, len %d\n", __func__, buff, length);

	req = drime4_ss_udc_ep_alloc_request(&ep->ep, GFP_ATOMIC);
	udc->ep0_reply = req;
	if (!req) {
		dev_warn(udc->dev, "%s: cannot alloc req\n", __func__);
		return -ENOMEM;
	}

	udc_req = our_req(req);

	req->buf = udc->ep0_buff;
	req->length = length;
	req->zero = 1; /* always do zero-length final transfer */
	req->complete = drime4_ss_udc_complete_oursetup;

	if (length && (buff != req->buf))
		memcpy(req->buf, buff, length);
	else
		ep->sent_zlp = 1;

	ret = drime4_ss_udc_ep_queue(&ep->ep, req, GFP_ATOMIC);
	if (ret) {
		dev_warn(udc->dev, "%s: cannot queue req\n", __func__);
		return ret;
	}

	return 0;
}

/**
 * drime4_ss_udc_start_req - start a USB request from an endpoint's queue
 * @udc: The controller state.
 * @udc_ep: The endpoint to process a request for
 * @udc_req: The request to start.
 * @continuing: True if we are doing more for the current request.
 *
 * Start the given request running by setting the endpoint registers
 * appropriately, and writing any data to the FIFOs.
 */
static void drime4_ss_udc_start_req(struct drime4_ss_udc *udc,
				 struct drime4_ss_udc_ep *udc_ep,
				 struct drime4_ss_udc_req *udc_req,
				 bool continuing)
{
	struct drime4_ss_udc_ep_command *epcmd = &udc->epcmd;
	struct usb_request *ureq = &udc_req->req;
	int trb_type = 1;
	int epnum = udc_ep->epnum;
	int xfer_length;
	bool res;

	dev_dbg(udc->dev, "%s: ep%d%s, req %p\n", __func__, epnum,
			   udc_ep->dir_in ? "in" : "out", ureq);

	udc_ep->req = udc_req;

	/* If endpoint is stalled, we will restart request later */
	if (udc_ep->halted) {
		dev_warn(udc->dev, "%s: ep%d is stalled\n", __func__, epnum);
		return;
	}

	/* Get type of TRB */
	if (epnum == 0 && !continuing)
		switch (udc->ep0_state) {
		case EP0_SETUP_PHASE:
			trb_type = 2;
			break;

		case EP0_DATA_PHASE:
			trb_type = 5;
			break;

		case EP0_STATUS_PHASE_2:
			trb_type = 3;
			break;

		case EP0_STATUS_PHASE_3:
			trb_type = 4;
			break;
		default:
			dev_warn(udc->dev, "%s: Erroneous EP0 state (%d)",
					   __func__, udc->ep0_state);
			return;
			break;
		}
	else
		trb_type = 1;

	/* Get transfer length */
	if (udc_ep->dir_in)
		xfer_length = ureq->length;
	else
		xfer_length = (ureq->length + udc_ep->ep.maxpacket - 1) &
			~(udc_ep->ep.maxpacket - 1);

	/* Fill TRB */
	udc_ep->trb->buff_ptr_low = (u32) ureq->dma;
	udc_ep->trb->buff_ptr_high = 0;
	udc_ep->trb->param1 =
			DRIME4_USB3_TRB_BUFSIZ(xfer_length);
	udc_ep->trb->param2 = DRIME4_USB3_TRB_IOC | DRIME4_USB3_TRB_LST |
			DRIME4_USB3_TRB_HWO | DRIME4_USB3_TRB_TRBCTL(trb_type);

	/* Start Transfer */
	epcmd->ep = get_phys_epnum(udc_ep);
	epcmd->param0 = 0;
	epcmd->param1 = udc_ep->trb_dma;
	epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPSTRTXFER;
	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

	res = drime4_ss_udc_issue_cmd(udc, epcmd);
	if (!res)
		dev_err(udc->dev, "Failed to start transfer\n");

	udc_ep->tri = (readl(udc->regs + DRIME4_USB3_DEPCMD(epcmd->ep)) >>
				DRIME4_USB3_DEPCMDx_EventParam_SHIFT) &
				DRIME4_USB3_DEPCMDx_XferRscIdx_LIMIT;
}

/**
 * drime4_ss_udc_process_req_featire - process request {SET,CLEAR}_FEATURE
 * @udc: The device state
 * @ctrl: USB control request
 */
static int drime4_ss_udc_process_req_feature(struct drime4_ss_udc *udc,
					 struct usb_ctrlrequest *ctrl)
{
	struct drime4_ss_udc_req *udc_req;
	bool restart;
	bool set = (ctrl->bRequest == USB_REQ_SET_FEATURE);
	struct drime4_ss_udc_ep *ep;

	dev_dbg(udc->dev, "%s: %s_FEATURE\n",
		__func__, set ? "SET" : "CLEAR");

	if (ctrl->bRequestType == USB_RECIP_ENDPOINT) {
		ep = ep_from_windex(udc, le16_to_cpu(ctrl->wIndex));
		if (!ep) {
			dev_dbg(udc->dev, "%s: no endpoint for 0x%04x\n",
				__func__, le16_to_cpu(ctrl->wIndex));
			return -ENOENT;
		}

		switch (le16_to_cpu(ctrl->wValue)) {
		case USB_ENDPOINT_HALT:
			drime4_ss_udc_ep_sethalt(&ep->ep, set);

			if (!set) {
				/* If we have pending request, then start it */
				restart = !list_empty(&ep->queue);
				if (restart) {
					udc_req = get_ep_head(ep);
					drime4_ss_udc_start_req(udc, ep,
								udc_req, false);
				}
			}
			break;

		default:
			return -ENOENT;
		}
	} else
		return -ENOENT;  /* currently only deal with endpoint */

	return 1;
}

/**
 * drime4_ss_udc_process_req_status - process request GET_STATUS
 * @udc: The device state
 * @ctrl: USB control request
 */
static int drime4_ss_udc_process_req_status(struct drime4_ss_udc *udc,
					struct usb_ctrlrequest *ctrl)
{
	struct drime4_ss_udc_ep *ep0 = &udc->eps[0];
	struct drime4_ss_udc_ep *ep;
	__le16 reply;
	int ret;

	dev_dbg(udc->dev, "%s: USB_REQ_GET_STATUS\n", __func__);

	if (!ep0->dir_in) {
		dev_warn(udc->dev, "%s: direction out?\n", __func__);
		return -EINVAL;
	}

	switch (ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		reply = cpu_to_le16(0); /* bit 0 => self powered,
					 * bit 1 => remote wakeup */
		break;

	case USB_RECIP_INTERFACE:
		/* currently, the data result should be zero */
		reply = cpu_to_le16(0);
		break;

	case USB_RECIP_ENDPOINT:
		ep = ep_from_windex(udc, le16_to_cpu(ctrl->wIndex));
		if (!ep)
			return -ENOENT;

		reply = cpu_to_le16(ep->halted ? 1 : 0);
		break;

	default:
		return 0;
	}

	if (le16_to_cpu(ctrl->wLength) != 2)
		return -EINVAL;

	ret = drime4_ss_udc_send_reply(udc, ep0, &reply, 2);
	if (ret) {
		dev_err(udc->dev, "%s: failed to send reply\n", __func__);
		return ret;
	}

	return 1;
}

/**
 * drime4_ss_udc_process_control - process a control request
 * @udc: The device state
 * @ctrl: The control request received
 *
 * The controller has received the SETUP phase of a control request, and
 * needs to work out what to do next (and whether to pass it on to the
 * gadget driver).
 */
static void drime4_ss_udc_process_control(struct drime4_ss_udc *udc,
				      struct usb_ctrlrequest *ctrl)
{
	struct drime4_ss_udc_ep *ep0 = &udc->eps[0];
	int ret = 0;

	dev_dbg(udc->dev, "ctrl Req=%02x, Type=%02x, V=%04x, L=%04x\n",
		 ctrl->bRequest, ctrl->bRequestType,
		 ctrl->wValue, ctrl->wLength);

	/* record the direction of the request, for later use when enquing
	 * packets onto EP0. */

	ep0->dir_in = (ctrl->bRequestType & USB_DIR_IN) ? 1 : 0;
	dev_dbg(udc->dev, "ctrl: dir_in=%d\n", ep0->dir_in);

	/* if we've no data with this request, then the last part of the
	 * transaction is going to implicitly be IN. */
	ep0->sent_zlp = 0;

	mdelay(1);

	if (ctrl->wLength == 0) {
		ep0->dir_in = 1;
		udc->ep0_three_stage = 0;
		udc->ep0_state = EP0_STATUS_PHASE_2;
		//printk(KERN_EMERG "%s:%d\n", __func__, __LINE__);
	} else
		udc->ep0_three_stage = 1;

	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		switch (ctrl->bRequest) {
		case USB_REQ_SET_ADDRESS:
			__bic32(udc->regs + DRIME4_USB3_DCFG,
				DRIME4_USB3_DCFG_DevAddr_MASK);
			__orr32(udc->regs + DRIME4_USB3_DCFG,
				DRIME4_USB3_DCFG_DevAddr(ctrl->wValue));

			dev_info(udc->dev, "new address %d\n", ctrl->wValue);

			udc->ep0_state = EP0_WAIT_NRDY;
			return;

		case USB_REQ_GET_STATUS:
			ret = drime4_ss_udc_process_req_status(udc, ctrl);
			break;

		case USB_REQ_CLEAR_FEATURE:
		case USB_REQ_SET_FEATURE:
			ret = drime4_ss_udc_process_req_feature(udc, ctrl);

			udc->ep0_state = EP0_WAIT_NRDY;
			break;
		}
	}

	/* as a fallback, try delivering it to the driver to deal with */

	if (ret == 0 && udc->driver) {
		ret = udc->driver->setup(&udc->gadget, ctrl);
		if (ret < 0)
			dev_dbg(udc->dev, "driver->setup() ret %d\n", ret);
	}

	/* the request is either unhandlable, or is not formatted correctly
	 * so respond with a STALL for the status stage to indicate failure.
	 */

	if (ret < 0) {
		struct drime4_ss_udc_ep_command *epcmd = &udc->epcmd;
		bool res;

		dev_dbg(udc->dev, "ep0 stall (dir=%d)\n", ep0->dir_in);
		/* FIXME */
		epcmd->ep = (ep0->dir_in) ? 1 : 0;
		epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPSSTALL;
		epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

		res = drime4_ss_udc_issue_cmd(udc, epcmd);
		if (!res)
			dev_err(udc->dev, "Failed to set/clear stall\n");

		udc->ep0_state = EP0_SETUP_PHASE;
		drime4_ss_udc_enqueue_setup(udc);
	}
}

/**
 * drime4_ss_udc_complete_setup - completion of a setup transfer
 * @ep: The endpoint the request was on.
 * @req: The request completed.
 *
 * Called on completion of any requests the driver itself submitted for
 * EP0 setup packets
 */
static void drime4_ss_udc_complete_setup(struct usb_ep *ep,
				      struct usb_request *req)
{
	struct drime4_ss_udc_ep *udc_ep = our_ep(ep);
	struct drime4_ss_udc *udc = udc_ep->parent;

	if (req->status < 0) {
		dev_dbg(udc->dev, "%s: failed %d\n", __func__, req->status);
		return;
	}

	drime4_ss_udc_process_control(udc, req->buf);
}

static int drime4_ss_udc_ep_queue(struct usb_ep *ep, struct usb_request *req,
			      gfp_t gfp_flags)
{
	struct drime4_ss_udc_req *udc_req = our_req(req);
	struct drime4_ss_udc_ep *udc_ep = our_ep(ep);
	struct drime4_ss_udc *udc = udc_ep->parent;
	unsigned long irqflags;
	bool first;
	int ret;

	dev_dbg(udc->dev, "%s: ep%d%s (%p): %d@%p, noi=%d, zero=%d, snok=%d\n",
			  __func__, udc_ep->epnum,
			  udc_ep->dir_in ? "in" : "out", req,
			  req->length, req->buf, req->no_interrupt,
			  req->zero, req->short_not_ok);

	/* initialise status of the request */
	INIT_LIST_HEAD(&udc_req->queue);

	req->actual = 0;
	req->status = -EINPROGRESS;

	/* Sync the buffers as necessary */
	if (req->buf == udc->ctrl_buff)
		req->dma = udc->ctrl_buff_dma;
	else {
		ret = drime4_ss_udc_map_dma(udc, udc_ep, req);
		if (ret)
			return ret;
	}

	spin_lock_irqsave(&udc_ep->lock, irqflags);

	first = list_empty(&udc_ep->queue);
	list_add_tail(&udc_req->queue, &udc_ep->queue);

	if (first && !udc_ep->not_ready)
		drime4_ss_udc_start_req(udc, udc_ep, udc_req, false);

	spin_unlock_irqrestore(&udc_ep->lock, irqflags);

	return 0;
}

/**
 * on_list - check request is on the given endpoint
 * @ep: The endpoint to check.
 * @test: The request to test if it is on the endpoint.
*/
static bool on_list(struct drime4_ss_udc_ep *udc_ep,
		    struct drime4_ss_udc_req *test)
{
	struct drime4_ss_udc_req *udc_req, *treq;

	list_for_each_entry_safe(udc_req, treq, &udc_ep->queue, queue) {
		if (udc_req == test)
			return true;
	}

	return false;
}

/**
 * drime4_ss_udc_ep_dequeue - dequeue a request from an endpoint
 * @ep: The endpoint the request was on.
 * @req: The request to dequeue.
 *
 * Dequeue a request and call its completion routine.
 */
static int drime4_ss_udc_ep_dequeue(struct usb_ep *ep, struct usb_request *req)
{
	struct drime4_ss_udc_req *udc_req = our_req(req);
	struct drime4_ss_udc_ep *udc_ep = our_ep(ep);
	struct drime4_ss_udc *udc = udc_ep->parent;
	unsigned long flags;

	dev_dbg(udc->dev, "%s: ep%d%s (%p)\n", __func__,
			  udc_ep->epnum, udc_ep->dir_in ? "in" : "out", req);

	spin_lock_irqsave(&udc_ep->lock, flags);

	if (!on_list(udc_ep, udc_req)) {
		spin_unlock_irqrestore(&udc_ep->lock, flags);
		return -EINVAL;
	}

	drime4_ss_udc_complete_request(udc, udc_ep, udc_req, -ECONNRESET);
	spin_unlock_irqrestore(&udc_ep->lock, flags);

	return 0;
}

static void drime4_ss_udc_enqueue_status(struct drime4_ss_udc *udc)
{
	struct usb_request *req = udc->ctrl_req;
	struct drime4_ss_udc_req *udc_req = our_req(req);
	int ret;

	dev_dbg(udc->dev, "%s: queueing status request\n", __func__);

	req->zero = 0;
	req->length = 0;
	req->buf = udc->ctrl_buff;
	req->complete = NULL;

	if (!list_empty(&udc_req->queue)) {
		dev_info(udc->dev, "%s already queued???\n", __func__);
		return;
	}

	ret = drime4_ss_udc_ep_queue(&udc->eps[0].ep, req, GFP_ATOMIC);
	if (ret < 0) {
		dev_err(udc->dev, "%s: failed queue (%d)\n", __func__, ret);
		/* Don't think there's much we can do other than watch the
		 * driver fail. */
	}
}

/**
 * drime4_ss_udc_enqueue_setup - start a request for EP0 packets
 * @udc: The device state.
 *
 * Enqueue a request on EP0 if necessary to received any SETUP packets
 * received from the host.
 */
static void drime4_ss_udc_enqueue_setup(struct drime4_ss_udc *udc)
{
	struct usb_request *req = udc->ctrl_req;
	struct drime4_ss_udc_req *udc_req = our_req(req);
	int ret;

	dev_dbg(udc->dev, "%s: queueing setup request\n", __func__);

	req->zero = 0;
	req->length = DRIME4_USB3_CTRL_BUFF_SIZE;
	req->buf = udc->ctrl_buff;
	req->complete = drime4_ss_udc_complete_setup;

	if (!list_empty(&udc_req->queue)) {
		dev_dbg(udc->dev, "%s already queued???\n", __func__);
		return;
	}

	udc->eps[0].dir_in = 0;

	ret = drime4_ss_udc_ep_queue(&udc->eps[0].ep, req, GFP_ATOMIC);
	if (ret < 0) {
		dev_err(udc->dev, "%s: failed queue (%d)\n", __func__, ret);
		/* Don't think there's much we can do other than watch the
		 * driver fail. */
	}
}

/**
 * drime4_ss_udc_complete_request - complete a request given to us
 * @udc: The device state.
 * @udc_ep: The endpoint the request was on.
 * @udc_req: The request to complete.
 * @result: The result code (0 => Ok, otherwise errno)
 *
 * The given request has finished, so call the necessary completion
 * if it has one and then look to see if we can start a new request
 * on the endpoint.
 *
 * Note, expects the ep to already be locked as appropriate.
*/
static void drime4_ss_udc_complete_request(struct drime4_ss_udc *udc,
				       struct drime4_ss_udc_ep *udc_ep,
				       struct drime4_ss_udc_req *udc_req,
				       int result)
{
	bool restart;

	if (!udc_req) {
		dev_dbg(udc->dev, "%s: nothing to complete\n", __func__);
		return;
	}

	dev_dbg(udc->dev, "complete: ep %p %s, req %p, %d => %p\n",
		udc_ep, udc_ep->ep.name, udc_req, result,
		udc_req->req.complete);

	/* only replace the status if we've not already set an error
	 * from a previous transaction */

	if (udc_req->req.status == -EINPROGRESS)
		udc_req->req.status = result;
	udc_ep->req = NULL;
	udc_ep->tri = 0;
	list_del_init(&udc_req->queue);

	if (udc_req->req.buf != udc->ctrl_buff) 
		drime4_ss_udc_unmap_dma(udc, udc_ep, udc_req);
        
	if (udc_ep->epnum == 0) {
		switch (udc->ep0_state) {
		case EP0_SETUP_PHASE:
			udc->ep0_state = EP0_DATA_PHASE;
			break;
		case EP0_DATA_PHASE:
			udc->ep0_state = EP0_WAIT_NRDY;
			break;
		case EP0_STATUS_PHASE_2:
		case EP0_STATUS_PHASE_3:
			udc->ep0_state = EP0_SETUP_PHASE;
			break;
		default:
			dev_err(udc->dev, "%s: Erroneous EP0 state (%d)",
					   __func__, udc->ep0_state);
			/* Will try to repair from it */
			udc->ep0_state = EP0_SETUP_PHASE;
			return;
			break;
		}
	}

	/* call the complete request with the locks off, just in case the
	 * request tries to queue more work for this endpoint. */

	if (udc_req->req.complete) {
		spin_unlock(&udc_ep->lock);
		udc_req->req.complete(&udc_ep->ep, &udc_req->req);
		spin_lock(&udc_ep->lock);
	}

	/* Look to see if there is anything else to do. Note, the completion
	 * of the previous request may have caused a new request to be started
	 * so be careful when doing this. */

	if (!udc_ep->req && result >= 0) {
		restart = !list_empty(&udc_ep->queue);
		if (restart) {
			udc_req = get_ep_head(udc_ep);
			drime4_ss_udc_start_req(udc, udc_ep, udc_req, false);
		}
	}
}

/**
 * drime4_ss_udc_kill_all_requests - remove all requests from the endpoint's queue
 * @udc: The device state.
 * @ep: The endpoint the requests may be on.
 * @result: The result code to use.
 * @force: Force removal of any current requests
 *
 * Go through the requests on the given endpoint and mark them
 * completed with the given result code.
 */
static void drime4_ss_udc_kill_all_requests(struct drime4_ss_udc *udc,
			      struct drime4_ss_udc_ep *ep,
			      int result, bool force)
{
	struct drime4_ss_udc_req *req, *treq;
	unsigned long flags;

	dev_dbg(udc->dev, "%s: ep%d\n", __func__, ep->epnum);

	spin_lock_irqsave(&ep->lock, flags);

	list_for_each_entry_safe(req, treq, &ep->queue, queue) {

		drime4_ss_udc_complete_request(udc, ep, req,
					   result);
	}

	spin_unlock_irqrestore(&ep->lock, flags);
}

/**
 * drime4_ss_udc_complete_request_lock - complete a request given to us (locked)
 * @udc: The device state.
 * @udc_ep: The endpoint the request was on.
 * @udc_req: The request to complete.
 * @result: The result code (0 => Ok, otherwise errno)
 *
 * See drime4_ss_udc_complete_request(), but called with the endpoint's
 * lock held.
*/
static void drime4_ss_udc_complete_request_lock(struct drime4_ss_udc *udc,
					    struct drime4_ss_udc_ep *udc_ep,
					    struct drime4_ss_udc_req *udc_req,
					    int result)
{
	unsigned long flags;

	spin_lock_irqsave(&udc_ep->lock, flags);
	drime4_ss_udc_complete_request(udc, udc_ep, udc_req, result);
	spin_unlock_irqrestore(&udc_ep->lock, flags);
}

/**
 * drime4_ss_udc_complete_in - complete IN transfer
 * @udc: The device state.
 * @udc_ep: The endpoint that has just completed.
 *
 * An IN transfer has been completed, update the transfer's state and then
 * call the relevant completion routines.
 */
static void drime4_ss_udc_complete_in(struct drime4_ss_udc *udc,
				  struct drime4_ss_udc_ep *udc_ep)
{
	struct drime4_ss_udc_req *udc_req = udc_ep->req;
	struct usb_request *req = &udc_req->req;
	static struct drime4_ss_udc_req *backup_udc_req;
	int size_left;

	dev_dbg(udc->dev, "%s: ep%d, req %p\n", __func__, udc_ep->epnum, req);

	if (!udc_req) {
		dev_dbg(udc->dev, "XferCompl but no req\n");
		return;
	}

	if (udc_ep->trb->param2 & DRIME4_USB3_TRB_HWO) {
		dev_dbg(udc->dev, "%s: HWO bit set!\n", __func__);
		return;
	}

	size_left = udc_ep->trb->param1 & DRIME4_USB3_TRB_BUFSIZ_MASK;
	udc_req->req.actual = udc_req->req.length - size_left;

        if (size_left)
        {
		dev_dbg(udc->dev, "%s: BUFSIZ is not zero (%d)",  __func__, size_left);
	}
	drime4_ss_udc_complete_request_lock(udc, udc_ep, udc_req, 0);

	backup_udc_req = udc_req;
}


/**
 * drime4_ss_udc_complete_out - complete OUT transfer
 * @udc: The device instance.
 * @epnum: The endpoint that has just completed.
*/
static void drime4_ss_udc_complete_out(struct drime4_ss_udc *udc,
				       struct drime4_ss_udc_ep *udc_ep)
{
	struct drime4_ss_udc_req *udc_req = udc_ep->req;
	struct usb_request *req = &udc_req->req;
	int len, size_left;

	dev_dbg(udc->dev, "%s: ep%d, req %p\n", __func__, udc_ep->epnum, req);

	if (!udc_req) {
		dev_dbg(udc->dev, "%s: no request active\n", __func__);
		return;
	}

	if (udc_ep->trb->param2 & DRIME4_USB3_TRB_HWO) {
		dev_dbg(udc->dev, "%s: HWO bit set!\n", __func__);
		return;
	}

	size_left = udc_ep->trb->param1 & DRIME4_USB3_TRB_BUFSIZ_MASK;
	len = (req->length + udc_ep->ep.maxpacket - 1) &
		~(udc_ep->ep.maxpacket - 1);
	udc_req->req.actual = len - size_left;

	if (size_left)
		dev_dbg(udc->dev, "%s: BUFSIZ is not zero (%d)",
			 __func__, size_left);

	drime4_ss_udc_complete_request_lock(udc, udc_ep, udc_req, 0);
}

static void drime4_ss_udc_irq_connectdone(struct drime4_ss_udc *udc)
{
	struct drime4_ss_udc_ep_command *epcmd = &udc->epcmd;
	u32 reg, speed;
	int mps0, mps;
	int i;
	bool res;

	dev_dbg(udc->dev, "%s\n", __func__);

	reg = readl(udc->regs + DRIME4_USB3_DSTS);
	speed = reg & DRIME4_USB3_DSTS_ConnectSpd_MASK;

	/* Suspend the inactive Phy */
	if (speed == USB_SPEED_SUPER)
		__orr32(udc->regs + DRIME4_USB3_GUSB2PHYCFG(0),
			DRIME4_USB3_GUSB2PHYCFGx_SusPHY);
	else
		__orr32(udc->regs + DRIME4_USB3_GUSB3PIPECTL(0),
			DRIME4_USB3_GUSB3PIPECTLx_SuspSSPhy);

	switch (speed) {
	/* High-speed */
	case 0:
		udc->gadget.speed = USB_SPEED_HIGH;
		mps0 = EP0_HS_MPS;
		mps = EP_HS_MPS;
		break;
	/* Full-speed */
	case 1:
	case 3:
		udc->gadget.speed = USB_SPEED_FULL;
		mps0 = EP0_FS_MPS;
		mps = EP_FS_MPS;
		break;
	/* Low-speed */
	case 2:
		udc->gadget.speed = USB_SPEED_LOW;
		mps0 = EP0_LS_MPS;
		mps = EP_LS_MPS;
		break;
	/* SuperSpeed */
	case 4:
		udc->gadget.speed = USB_SPEED_SUPER;
		mps0 = EP0_SS_MPS;
		mps = EP_SS_MPS;
		break;
		/* static analysis bug fix 2012-09-10 Han Seungwon */
	default:
		udc->gadget.speed = USB_SPEED_LOW;
		mps0 = EP0_LS_MPS;
		mps = EP_LS_MPS;
		break;
		/* static analysis bug fix 2012-09-10 Han Seungwon */
	}

	udc->eps[0].ep.maxpacket = mps0;
	for (i = 1; i < DRIME4_USB3_EPS; i++)
		udc->eps[i].ep.maxpacket = mps;

	epcmd->ep = 0;
	epcmd->param0 = DRIME4_USB3_DEPCMDPAR0x_MPS(mps0);
	epcmd->param1 = DRIME4_USB3_DEPCMDPAR1x_XferNRdyEn |
			DRIME4_USB3_DEPCMDPAR1x_XferCmplEn;
	epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPCFG;
	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

	res = drime4_ss_udc_issue_cmd(udc, epcmd);
	if (!res)
		dev_err(udc->dev, "Failed to configure physical EP0\n");

	epcmd->ep = 1;
	epcmd->param1 = DRIME4_USB3_DEPCMDPAR1x_EpDir |
			DRIME4_USB3_DEPCMDPAR1x_XferNRdyEn |
			DRIME4_USB3_DEPCMDPAR1x_XferCmplEn;
	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

	res = drime4_ss_udc_issue_cmd(udc, epcmd);
	if (!res)
		dev_err(udc->dev, "Failed to configure physical EP1\n");
}

static void drime4_ss_udc_irq_usbrst(struct drime4_ss_udc *udc)
{
	struct drime4_ss_udc_ep_command *epcmd = &udc->epcmd;
	struct drime4_ss_udc_ep *ep;
	bool res;
	int epnum;
    int size_left;
    struct drime4_ss_udc_req *udc_req;

	dev_dbg(udc->dev, "%s\n", __func__);

	epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPENDXFER;

	/* End transfer, kill all requests and clear STALL on the
	   non-EP0 endpoints */
	for (epnum = 1; epnum < DRIME4_USB3_EPS; epnum++) {

		ep = &udc->eps[epnum];
		//printk(KERN_EMERG "drime4_ss_udc_irq_usbrst 00 \n");
		epcmd->ep = get_phys_epnum(ep);
                /*  Checking of  device halted and tri stete is not required at thius stage
                    It will be checked at the time of actual request transfer */
		if (ep->tri) {
                        if (time_after(jiffies, udc->last_rst + msecs_to_jiffies(200))) {
			    epcmd->cmdflags = (ep->tri <<
				DRIME4_USB3_DEPCMDx_CommandParam_SHIFT) |
				DRIME4_USB3_DEPCMDx_HiPri_ForceRM |
				DRIME4_USB3_DEPCMDx_CmdIOC |
				DRIME4_USB3_DEPCMDx_CmdAct;
 
		         	res = drime4_ss_udc_issue_cmd(udc, epcmd);
		                if (!res) {
			        	dev_err(udc->dev, "Failed to end transfer\n");
			        	ep->not_ready = true;
			        }
                         
			        ep->tri = 0;
                                udc->last_rst = jiffies;
                      }
		}
		ep->not_ready = false;
		drime4_ss_udc_kill_all_requests(udc, ep, -ECONNRESET, true);
	//	if (ep->halted)
	//		drime4_ss_udc_ep_sethalt(&ep->ep, 0);
	}

	/* Set device address to 0 */
	__bic32(udc->regs + DRIME4_USB3_DCFG, DRIME4_USB3_DCFG_DevAddr_MASK);
}

static void drime4_ss_udc_irq_disconnect(struct drime4_ss_udc *udc)
{
        struct drime4_ss_udc_ep *ep;
        int epnum;

        dev_dbg(udc->dev, "%s\n", __func__);

        /* End transfer, kill all requests and clear STALL on the
           non-EP0 endpoints */
        for (epnum = 1; epnum < DRIME4_USB3_EPS; epnum++) {

                ep = &udc->eps[epnum];
                drime4_ss_udc_kill_all_requests(udc, ep, -ESHUTDOWN, true);
        }
}

/**
 * drime4_ss_udc_handle_depevt - handle endpoint-specific event
 * @udc: The driver state
 * @event: event to handle
 *
*/
static void drime4_ss_udc_handle_depevt(struct drime4_ss_udc *udc, u32 event)
{
	int index = (event & DRIME4_USB3_DEPEVT_EPNUM_MASK) >> 1;
	int dir_in = index & 1;
	int epnum = get_usb_epnum(index);
	/* We will need it in future */
	struct drime4_ss_udc_ep *udc_ep = &udc->eps[epnum];
	struct drime4_ss_udc_ep_command *epcmd, *tepcmd;
	struct drime4_ss_udc_req *udc_req;
	bool restart, res;

    //printk("4");
	switch (event & DRIME4_USB3_DEPEVT_EVENT_MASK) {
	case DRIME4_USB3_DEPEVT_EVENT_XferNotReady:
		dev_dbg(udc->dev, "Xfer Not Ready: ep%d%s\n",
				  epnum, dir_in ? "in" : "out");
		//printk(KERN_EMERG "%s:%d Xfer Not Ready: ep%d%s\n", __func__, __LINE__, epnum, dir_in ? "in" : "out");
		//mdelay(1);
		if (epnum == 0 && udc->ep0_state == EP0_WAIT_NRDY) {
			udc_ep->dir_in = dir_in;

			if (udc->ep0_three_stage)
				udc->ep0_state = EP0_STATUS_PHASE_3;
			else
				udc->ep0_state = EP0_STATUS_PHASE_2;

			drime4_ss_udc_enqueue_status(udc);
		}
		break;

	case DRIME4_USB3_DEPEVT_EVENT_XferComplete:
		dev_dbg(udc->dev, "Xfer Complete: ep%d%s\n",
				  epnum, dir_in ? "in" : "out");
		if (dir_in) {
			/* Handle "transfer complete" for IN EPs */
			drime4_ss_udc_complete_in(udc, udc_ep);
		} else {
			/* Handle "transfer complete" for OUT EPs */
			drime4_ss_udc_complete_out(udc, udc_ep);
		}

		if (epnum == 0 && udc->ep0_state == EP0_SETUP_PHASE)
			drime4_ss_udc_enqueue_setup(udc);

		break;

	case DRIME4_USB3_DEPEVT_EVENT_EPCmdCmplt:
		dev_dbg(udc->dev, "EP Cmd complete: ep%d%s\n",
				  epnum, dir_in ? "in" : "out");

		udc_ep->not_ready = false;

		/* Issue all pending commands for endpoint */
		list_for_each_entry_safe(epcmd, tepcmd,
					 &udc_ep->cmd_queue, queue) {

			dev_dbg(udc->dev, "Pending command %02xh for ep%d%s\n",
					 epcmd->cmdtyp, epnum,
					 dir_in ? "in" : "out");

			res = drime4_ss_udc_issue_cmd(udc, epcmd);
			if (!res)
				dev_err(udc->dev, "Failed to issue command\n");

			list_del_init(&epcmd->queue);
			kfree(epcmd);
		}

		/* If we have pending request, then start it */
		restart = !list_empty(&udc_ep->queue);
		if (restart) {
			udc_req = get_ep_head(udc_ep);
			drime4_ss_udc_start_req(udc, udc_ep,
						udc_req, false);
		}

		break;
	}

}
#include <linux/jack.h>
static void usb_sp_timer_callback(unsigned long data )
{
	//printk(KERN_EMERG "%s:%d : ULStChng reset_flag %d, 
     //      timerflg %d\n", __func__, __LINE__, reset_flag, timerflag);
    if (!reset_flag){
        //printk(KERN_EMERG "The Special Charger Detected ----\n");
        jack_event_handler("usb_discont", 2);
    }   
}
/**
 * drime4_ss_udc_handle_devt - handle device-specific event
 * @udc: The driver state
 * @event: event to handle
 *
*/


static void drime4_ss_udc_handle_devt(struct drime4_ss_udc *udc, u32 event)
{
    int ret =0;
	//dev_info(udc->dev, "111");
	//printk("%s:%d event:%d\n", __func__, __LINE__, event & DRIME4_USB3_DEVT_EVENT_MASK);
	mdelay(1);
	switch (event & DRIME4_USB3_DEVT_EVENT_MASK) {
	case DRIME4_USB3_DEVT_EVENT_ULStChng:
		//printk(KERN_EMERG "%s:%d : ULStChng reset_flag %d, 
        //          timerflg %d\n", __func__, __LINE__, reset_flag, timerflag);
		dev_dbg(udc->dev, "USB-Link State Change");
		ret = mod_timer(&usb_sp_timer, jiffies + msecs_to_jiffies(600) );
		break;

	case DRIME4_USB3_DEVT_EVENT_ConnectDone:
		//printk(KERN_EMERG "%s:%d\n", __func__, __LINE__);
		dev_dbg(udc->dev, "Connection Done");
#ifdef CONFIG_BATTERY_SAMSUNG
		drime4_ss_udc_cable_connect(udc);
#endif
		drime4_ss_udc_irq_connectdone(udc);
		break;

	case DRIME4_USB3_DEVT_EVENT_USBRst:
		//printk(KERN_EMERG "%s:%d : USBR streset_flag %d, 
        //      timerflg %d\n", __func__, __LINE__, reset_flag, timerflag);
		dev_info(udc->dev, "USB Reset");
        reset_flag = 1;
        jack_event_handler("usb_discont", 1);
		mdelay(20);	//mdelay(32);
		drime4_ss_udc_irq_usbrst(udc);
		break;

	case DRIME4_USB3_DEVT_EVENT_DisconnEvt:
		//printk(KERN_EMERG "%s:%d\n", __func__, __LINE__);
		dev_info(udc->dev, "Disconnection Detected");
                drime4_ss_udc_irq_disconnect(udc);

		call_gadget(udc, disconnect);
#ifdef CONFIG_BATTERY_SAMSUNG
		drime4_ss_udc_cable_disconnect(udc);
#endif
		break;
	default:
		printk(KERN_EMERG "UNKNOWN %s:%d\n", __func__, __LINE__);
		break;
	}
}

static void drime4_ss_udc_handle_otgevt(struct drime4_ss_udc *udc, u32 event)
{
    	//dev_info(udc->dev, "222");

}

static void drime4_ss_udc_handle_gevt(struct drime4_ss_udc *udc, u32 event)
{
    	//dev_info(udc->dev, "333");
}
/**
 * drime4_ss_udc_irq - handle device interrupt
 * @irq: The IRQ number triggered
 * @pw: The pw value when registered the handler.
 */
static int indx;
static u32 print_cnt = 0;
static irqreturn_t drime4_ss_udc_irq(int irq, void *pw)
{
	struct drime4_ss_udc *udc = pw;
	
	int gevntcount;
	static u32 event;
	u32 ecode1, ecode2;
	
	gevntcount = readl(udc->regs + DRIME4_USB3_GEVNTCOUNT(0)) &
			DRIME4_USB3_GEVNTCOUNTx_EVNTCount_MASK;
	/* TODO: what if number of events more then buffer size? */

	//dev_info(udc->dev, "INTERRUPT (%d)\n", gevntcount >> 2);

	while (gevntcount) {
		event = udc->event_buff[indx++];
		if(print_cnt < 60)
		{
			//printk(KERN_INFO "drime4_ss_udc_irq() event = %x\n", event);
			//mdelay(10);
			print_cnt++;
		}

		//dev_info(udc->dev, "event = 0x%08x\n", event);

		ecode1 = event & 0x01;

		if (ecode1 == 0)
		{
			/* Device Endpoint-Specific Event */
			//printk(KERN_EMERG "%s:%d\n", __func__, __LINE__);
			drime4_ss_udc_handle_depevt(udc, event);
		}
		else {
			ecode2 = (event & 0xfe) >> 1;

			switch (ecode2) {
			/* Device-Specific Event */
			case 0x00:
				//printk(KERN_EMERG "%s:%d\n", __func__, __LINE__);
				drime4_ss_udc_handle_devt(udc, event);
			break;

			/* OTG Event */
			case 0x01:
				//printk(KERN_EMERG "%s:%d\n", __func__, __LINE__);
				drime4_ss_udc_handle_otgevt(udc, event);
			break;

			/* Other Core Event */
			case 0x03:
			case 0x04:
				//printk(KERN_EMERG "%s:%d\n", __func__, __LINE__);
				drime4_ss_udc_handle_gevt(udc, event);
			break;

			/* Unknown Event Type */
			default:
				//printk(KERN_EMERG "%s:%d\n", __func__, __LINE__);
				dev_info(udc->dev, "Unknown event type\n");
			break;
			}
		}
		/* We processed 1 event (4 bytes) */
		writel(4, udc->regs + DRIME4_USB3_GEVNTCOUNT(0));

		if (indx > (DRIME4_USB3_EVENT_BUFF_WSIZE - 1))
			indx = 0;

		gevntcount -= 4;
	}

	/* Do we need to read GEVENTCOUNT here and retry? */

	return IRQ_HANDLED;
}

/**
 * drime4_ss_udc_initep - initialise a single endpoint
 * @udc: The device state.
 * @udc_ep: The endpoint to be initialised.
 * @epnum: The endpoint number
 *
 * Initialise the given endpoint (as part of the probe and device state
 * creation) to give to the gadget driver. Setup the endpoint name, any
 * direction information and other state that may be required.
 */
unsigned long g_ep0_trb_dma;
static int __devinit drime4_ss_udc_initep(struct drime4_ss_udc *udc,
				       struct drime4_ss_udc_ep *udc_ep,
				       int epnum)
{

	char *dir;

	if (epnum == 0)
		dir = "";
	else if ((epnum % 2) == 0) {
		dir = "out";
	} else {
		dir = "in";
		udc_ep->dir_in = 1;
	}

	udc_ep->epnum = epnum;

	snprintf(udc_ep->name, sizeof(udc_ep->name), "ep%d%s", epnum, dir);

	INIT_LIST_HEAD(&udc_ep->queue);
	INIT_LIST_HEAD(&udc_ep->cmd_queue);
	INIT_LIST_HEAD(&udc_ep->ep.ep_list);

	spin_lock_init(&udc_ep->lock);

	/* add to the list of endpoints known by the gadget driver */
	if (epnum)
		list_add_tail(&udc_ep->ep.ep_list, &udc->gadget.ep_list);

	udc_ep->parent = udc;
	udc_ep->ep.name = udc_ep->name;
#if defined(CONFIG_USB_GADGET_DRIME4_SS_UDC_SSMODE)
	udc_ep->ep.maxpacket = epnum ? EP_SS_MPS : EP0_SS_MPS;
#else
	udc_ep->ep.maxpacket = epnum ? EP_HS_MPS : EP0_HS_MPS;
#endif
	udc_ep->ep.ops = &drime4_ss_udc_ep_ops;
	udc_ep->trb = dma_alloc_coherent(NULL,
					 sizeof(struct drime4_ss_udc_trb),
					 &udc_ep->trb_dma,
					 GFP_KERNEL);
	if (!udc_ep->trb)
		return -ENOMEM;

	memset(udc_ep->trb, 0, sizeof(struct drime4_ss_udc_trb));

	if (epnum == 0) {
		udc->ep0_state = EP0_UNCONNECTED;
#ifdef CONFIG_BOOTLOADER_SNAPSHOT
		g_ep0_trb_dma = udc_ep->trb_dma;
#endif
	}

	return 0;
}

static void drime4_ss_udc_phy_set(struct platform_device *pdev)
{
	u32 reg;
	struct drime4_ss_udc *udc = platform_get_drvdata(pdev);
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	if(udc == NULL)	return -1;
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	/* The reset values:
	 *	GUSB2PHYCFG(0)	= 0x00002400
	 *	GUSB3PIPECTL(0)	= 0x00260002
	 */
	/* PHY initialization */

	writel(0, udc->phy_regs + DRIME4_PHY_CFG1);
	reg = DRIME4_PHY_CFG4_REFSSP_EN | DRIME4_PHY_CFG4_RETENABLE_EN	|
		DRIME4_PHY_CFG4_MPLL_100M;
	writel(reg, udc->phy_regs + DRIME4_PHY_CFG4);
	reg = DRIME4_PHY_CFG3_FSEL_100M | DRIME4_PHY_CFG3_REFCLK_PAD |
		DRIME4_PHY_CFG3_COMMON_ONN;
	writel(reg, udc->phy_regs + DRIME4_PHY_CFG3);
	/* Wait 1ms for pll locked*/
	udelay(1000);

	reg = DRIME4_LINK_CTRL0_BUS_RST | DRIME4_LINK_CTRL0_CORE_RST |
		DRIME4_LINK_CTRL0_POWERON_RST;
	writel(reg, udc->phy_regs + DRIME4_LINK_CTRL0);

	__orr32(udc->regs + DRIME4_USB3_GCTL, DRIME4_USB3_GCTL_CoreSoftReset);
	__orr32(udc->regs + DRIME4_USB3_GUSB2PHYCFG(0),
			    DRIME4_USB3_GUSB2PHYCFGx_PHYSoftRst);
	__orr32(udc->regs + DRIME4_USB3_GUSB3PIPECTL(0),
			    DRIME4_USB3_GUSB3PIPECTLx_PHYSoftRst);

	__bic32(udc->regs + DRIME4_USB3_GUSB2PHYCFG(0),
			    DRIME4_USB3_GUSB2PHYCFGx_PHYSoftRst);
	__bic32(udc->regs + DRIME4_USB3_GUSB3PIPECTL(0),
			    DRIME4_USB3_GUSB3PIPECTLx_PHYSoftRst);
	__bic32(udc->regs + DRIME4_USB3_GCTL, DRIME4_USB3_GCTL_CoreSoftReset);


	__bic32(udc->regs + DRIME4_USB3_GUSB2PHYCFG(0),
		DRIME4_USB3_GUSB2PHYCFGx_SusPHY |
		DRIME4_USB3_GUSB2PHYCFGx_EnblSlpM |
		DRIME4_USB3_GUSB2PHYCFGx_USBTrdTim_MASK);
	__orr32(udc->regs + DRIME4_USB3_GUSB2PHYCFG(0),
		DRIME4_USB3_GUSB2PHYCFGx_USBTrdTim(9));

	__bic32(udc->regs + DRIME4_USB3_GUSB3PIPECTL(0),
			    DRIME4_USB3_GUSB3PIPECTLx_SuspSSPhy);

	dev_dbg(udc->dev, "GUSB2PHYCFG(0)=0x%08x, GUSB3PIPECTL(0)=0x%08x",
		readl(udc->regs + DRIME4_USB3_GUSB2PHYCFG(0)),
		readl(udc->regs + DRIME4_USB3_GUSB3PIPECTL(0)));
}

/**
 * drime4_ss_udc_corereset - issue softreset to the core
 * @udc: The device state
 *
 * Issue a soft reset to the core, and await the core finishing it.
*/
static int drime4_ss_udc_corereset(struct drime4_ss_udc *udc)
{
	bool res;

	/* issue soft reset */
	__orr32(udc->regs + DRIME4_USB3_DCTL, DRIME4_USB3_DCTL_CSftRst);

	res = drime4_ss_udc_poll_bit_clear(udc->regs + DRIME4_USB3_DCTL,
					DRIME4_USB3_DCTL_CSftRst,
					1000);
	if (!res) {
		dev_err(udc->dev, "Failed to get CSftRst asserted\n");
		return -EINVAL;
	}

	return 0;
}


int drime4_udc_soft_connect(struct drime4_ss_udc *udc)
{
    bool res;
    res = drime4_ss_udc_poll_bit_clear(udc->regs + DRIME4_USB3_DCTL,
					DRIME4_USB3_DCTL_CSftRst,
					1000);
	if (!res) {
		dev_err(udc->dev, "Failed to get CSftRst asserted\n");
		return -EINVAL;
	}

	return 0;


}

void drime4_udc_soft_disconnect(struct drime4_ss_udc *udc)
{

	/* issue soft reset */
	__orr32(udc->regs + DRIME4_USB3_DCTL, DRIME4_USB3_DCTL_CSftRst);

}

/**
 * drime4_ss_udc_ep0_activate - activate USB endpoint 0
 * @udc: The device state
 *
 * Configure physical endpoints 0 & 1.
 */
static void drime4_ss_udc_ep0_activate(struct drime4_ss_udc *udc)
{
	struct drime4_ss_udc_ep_command *epcmd = &udc->epcmd;
	bool res;

	/* Start New Configuration */
	epcmd->ep = 0;
	epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPSTARTCFG;
	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;


	res = drime4_ss_udc_issue_cmd(udc, epcmd);
	if (!res)
		dev_err(udc->dev, "Failed to start new configuration\n");

	/* Configure Physical Endpoint 0 */
	epcmd->ep = 0;
	epcmd->param0 = DRIME4_USB3_DEPCMDPAR0x_MPS(0x200);
	epcmd->param1 = DRIME4_USB3_DEPCMDPAR1x_XferNRdyEn |
			DRIME4_USB3_DEPCMDPAR1x_XferCmplEn;
	epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPCFG;
	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

	res = drime4_ss_udc_issue_cmd(udc, epcmd);
	if (!res)
		dev_err(udc->dev, "Failed to configure physical EP0\n");

	/* Configure Physical Endpoint 1 */
	epcmd->ep = 1;
	epcmd->param0 = DRIME4_USB3_DEPCMDPAR0x_MPS(0x200);
	epcmd->param1 = DRIME4_USB3_DEPCMDPAR1x_EpDir |
			DRIME4_USB3_DEPCMDPAR1x_XferNRdyEn |
			DRIME4_USB3_DEPCMDPAR1x_XferCmplEn;
	epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPCFG;
	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

	res = drime4_ss_udc_issue_cmd(udc, epcmd);
	if (!res)
		dev_err(udc->dev, "Failed to configure physical EP1\n");

	/* Configure Pysical Endpoint 0 Transfer Resource */
	epcmd->ep = 0;
	epcmd->param0 = DRIME4_USB3_DEPCMDPAR0x_NumXferRes(1);
	epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPXFERCFG;
	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

	res = drime4_ss_udc_issue_cmd(udc, epcmd);
	if (!res)
		dev_err(udc->dev,
			"Failed to configure physical EP0 transfer resource\n");

	/* Configure Pysical Endpoint 1 Transfer Resource */
	epcmd->ep = 1;
	epcmd->param0 = DRIME4_USB3_DEPCMDPAR0x_NumXferRes(1);
	epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPXFERCFG;
	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

	res = drime4_ss_udc_issue_cmd(udc, epcmd);
	if (!res)
		dev_err(udc->dev,
			"Failed to configure physical EP1 transfer resource\n");

	/* Enable Physical Endpoints 0 and 1 */
	writel(3, udc->regs + DRIME4_USB3_DALEPENA);
}

/**
 * drime4_ss_udc_ep_activate - activate USB endpoint
 * @udc: The device state
 * @udc_ep: The endpoint to activate.
 *
 * Configure physical endpoints > 1.
 */
static void drime4_ss_udc_ep_activate(struct drime4_ss_udc *udc,
				      struct drime4_ss_udc_ep *udc_ep)
{
	struct drime4_ss_udc_ep_command *epcmd = &udc->epcmd;
	int epnum = udc_ep->epnum;
	int maxburst = udc_ep->ep.maxburst;
	bool res;

	if (!udc->eps_enabled) {
		udc->eps_enabled = true;

		/* Start New Configuration */
		epcmd->ep = 0;
		epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPSTARTCFG;
		epcmd->cmdflags =
			(2 << DRIME4_USB3_DEPCMDx_CommandParam_SHIFT) |
			DRIME4_USB3_DEPCMDx_CmdAct;

		res = drime4_ss_udc_issue_cmd(udc, epcmd);
		if (!res)
			dev_err(udc->dev, "Failed to start new configuration\n");
	}

	if (udc_ep->not_ready) {
		epcmd = kzalloc(sizeof(struct drime4_ss_udc_ep_command),
				GFP_ATOMIC);
		if (!epcmd) {
			/* Will try to issue command immediately */
			epcmd = &udc->epcmd;
			udc_ep->not_ready = false;
		}
	}

	epcmd->ep = get_phys_epnum(udc_ep);
	epcmd->param0 = DRIME4_USB3_DEPCMDPAR0x_EPType(udc_ep->type) |
			DRIME4_USB3_DEPCMDPAR0x_MPS(udc_ep->ep.maxpacket) |
			DRIME4_USB3_DEPCMDPAR0x_BrstSiz(maxburst);
	if (udc_ep->dir_in)
		epcmd->param0 |= DRIME4_USB3_DEPCMDPAR0x_FIFONum(epnum);
	epcmd->param1 = DRIME4_USB3_DEPCMDPAR1x_EpNum(epnum) |
			(udc_ep->dir_in ? DRIME4_USB3_DEPCMDPAR1x_EpDir : 0) |
			DRIME4_USB3_DEPCMDPAR1x_XferNRdyEn |
			DRIME4_USB3_DEPCMDPAR1x_XferCmplEn;
	epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPCFG;
	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

	if (udc_ep->not_ready)
		list_add_tail(&epcmd->queue, &udc_ep->cmd_queue);
	else {
		res = drime4_ss_udc_issue_cmd(udc, epcmd);
		if (!res)
			dev_err(udc->dev, "Failed to configure physical EP\n");
	}

	/* Configure Pysical Endpoint Transfer Resource */
	if (udc_ep->not_ready) {
		epcmd = kzalloc(sizeof(struct drime4_ss_udc_ep_command),
				GFP_ATOMIC);
		if (!epcmd) {
			epcmd = &udc->epcmd;
			udc_ep->not_ready = false;
		}
	}

	epcmd->ep = get_phys_epnum(udc_ep);
	epcmd->param0 = DRIME4_USB3_DEPCMDPAR0x_NumXferRes(1);
	epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPXFERCFG;
	epcmd->cmdflags = DRIME4_USB3_DEPCMDx_CmdAct;

	if (udc_ep->not_ready)
		list_add_tail(&epcmd->queue, &udc_ep->cmd_queue);
	else {
		res = drime4_ss_udc_issue_cmd(udc, epcmd);
		if (!res)
			dev_err(udc->dev, "Failed to configure physical EP "
					  "transfer resource\n");
	}

	/* Enable Physical Endpoint */
	__orr32(udc->regs + DRIME4_USB3_DALEPENA, 1 << epcmd->ep);
}

/**
 * drime4_ss_udc_ep_deactivate - deactivate USB endpoint
 * @udc: The device state.
 * @udc_ep: The endpoint to deactivate.
 *
 * End any active transfer and disable endpoint.
 */
static void drime4_ss_udc_ep_deactivate(struct drime4_ss_udc *udc,
					struct drime4_ss_udc_ep *udc_ep)
{
	struct drime4_ss_udc_ep_command *epcmd = &udc->epcmd;
	int index = get_phys_epnum(udc_ep);

	//printk(KERN_EMERG "%s:%d\n", __func__, __LINE__);

	udc->eps_enabled = false;

	if (udc_ep->tri && !udc_ep->not_ready) {
		bool res;

		epcmd->ep = get_phys_epnum(udc_ep);
		epcmd->cmdtyp = DRIME4_USB3_DEPCMDx_CmdTyp_DEPENDXFER;
		epcmd->cmdflags = (udc_ep->tri <<
			DRIME4_USB3_DEPCMDx_CommandParam_SHIFT) |
			DRIME4_USB3_DEPCMDx_HiPri_ForceRM |
			DRIME4_USB3_DEPCMDx_CmdIOC |
			DRIME4_USB3_DEPCMDx_CmdAct;

		res = drime4_ss_udc_issue_cmd(udc, epcmd);
		if (!res) {
			dev_err(udc->dev, "Failed to end transfer\n");
			udc_ep->not_ready = true;
		}

		udc_ep->tri = 0;
	}

	__bic32(udc->regs + DRIME4_USB3_DALEPENA, 1 << index);
}

static void drime4_ss_udc_init(struct drime4_ss_udc *udc)
{
	u32 reg;
	u16 release;

	reg = readl(udc->regs + DRIME4_USB3_GSNPSID);
	release = reg & 0xffff;
	//dev_info(udc->dev, "drime4_ss_udc_init - Core ID Number: 0x%04x\n", reg >> 16);
	//dev_info(udc->dev, "drime4_ss_udc_init - Release Number: 0x%04x\n", release);

	/*
	 * WORKAROUND: DWC3 revisions <1.90a have a bug
	 * when The device fails to connect at SuperSpeed
	 * and falls back to high-speed mode which causes
	 * the device to enter in a Connect/Disconnect loop
	 */
	if (release < 0x190a)
		__orr32(udc->regs + DRIME4_USB3_GCTL,
			DRIME4_USB3_GCTL_U2RSTECN);

	writel(DRIME4_USB3_GSBUSCFG0_INCR16BrstEna,
	       udc->regs + DRIME4_USB3_GSBUSCFG0);
	writel(DRIME4_USB3_GSBUSCFG1_BREQLIMIT(3),
	       udc->regs + DRIME4_USB3_GSBUSCFG1);

	/* Event buffer */
	writel(0, udc->regs + DRIME4_USB3_GEVNTADR_63_32(0));
	writel(udc->event_buff_dma,
		udc->regs + DRIME4_USB3_GEVNTADR_31_0(0));
	/* Set Event Buffer size */
	writel(DRIME4_USB3_EVENT_BUFF_BSIZE,
		udc->regs + DRIME4_USB3_GEVNTSIZ(0));

	writel(DRIME4_USB3_DCFG_NumP(1) | DRIME4_USB3_DCFG_PerFrInt(2) |
#if defined(CONFIG_USB_GADGET_DRIME4_SS_UDC_SSMODE)
	       DRIME4_USB3_DCFG_DevSpd(4),
#else
	       DRIME4_USB3_DCFG_DevSpd(0),
#endif
	       udc->regs + DRIME4_USB3_DCFG);

	/* Flush any pending events */
	__orr32(udc->regs + DRIME4_USB3_GEVNTSIZ(0),
		DRIME4_USB3_GEVNTSIZx_EvntIntMask);

	reg = readl(udc->regs + DRIME4_USB3_GEVNTCOUNT(0));
	writel(reg, udc->regs + DRIME4_USB3_GEVNTCOUNT(0));

	__bic32(udc->regs + DRIME4_USB3_GEVNTSIZ(0),
		DRIME4_USB3_GEVNTSIZx_EvntIntMask);

	/* Enable events */
	writel(DRIME4_USB3_DEVTEN_ULStCngEn | DRIME4_USB3_DEVTEN_ConnectDoneEn |
		DRIME4_USB3_DEVTEN_USBRstEn | DRIME4_USB3_DEVTEN_DisconnEvtEn,
		udc->regs + DRIME4_USB3_DEVTEN);

	drime4_ss_udc_ep0_activate(udc);

	/* Start the device controller operation */
	__orr32(udc->regs + DRIME4_USB3_DCTL, DRIME4_USB3_DCTL_Run_Stop);
}

static void drime4_ss_udc_free_all_trb(struct drime4_ss_udc *udc)
{
	int epnum;

	for (epnum = 0; epnum < DRIME4_USB3_EPS; epnum++) {
		struct drime4_ss_udc_ep *udc_ep = &udc->eps[epnum];

		if (udc_ep->trb_dma)
			dma_free_coherent(NULL,
					  sizeof(struct drime4_ss_udc_trb),
					  udc_ep->trb,
					  udc_ep->trb_dma);
	}
}



/** added by wonjung1.kim **/

/*
 *	usb_gadget_register_driver
 */
int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct drime4_ss_udc *udc = our_udc;
	int		retval;

	//dprintk(DEBUG_NORMAL, "usb_gadget_register_driver() '%s'\n",driver->driver.name);
	dev_info(udc->dev, "usb_gadget_register_driver= %s\n", driver->driver.name);
	//printk(KERN_EMERG "[usb_gadget_register_driver] %s\n", driver->driver.name);

#if 0
	/* Sanity checks */
	if (!udc)
		return -ENODEV;
#endif	/* static analysis bug fix 2012-09-10 Han Seungwon */

	if (udc->driver)
		return -EBUSY;

#if (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,39))
	if (!driver->bind || !driver->setup || driver->speed < USB_SPEED_FULL)
	{
		printk(KERN_EMERG "Invalid driver: bind %p setup %p speed %d\n", 	driver->bind, driver->setup, driver->speed);
		return -EINVAL;
	}
#elif (LINUX_VERSION_CODE == KERNEL_VERSION(3,5,0))
	if (!driver->bind || !driver->setup || driver->max_speed < USB_SPEED_FULL)
	{
		printk(KERN_EMERG "Invalid driver: bind %p setup %p max_speed %d\n", driver->bind, driver->setup, driver->max_speed);
		return -EINVAL;
	}
#else
	if (!driver->bind || !driver->setup || driver->speed < USB_SPEED_FULL)
	{
		printk(KERN_EMERG "Invalid driver: bind %p setup %p speed %d\n", 	driver->bind, driver->setup, driver->speed);
		return -EINVAL;
	}
#endif

#if defined(MODULE)
	if (!driver->unbind) {
		printk(KERN_EMERG "Invalid driver: no unbind method\n");
		return -EINVAL;
	}
#endif

	/* Hook the driver */
	udc->driver = driver;
	udc->gadget.dev.driver = &driver->driver;
	printk(KERN_ERR "TESTED 1 ++++++++++++\n");

	/* Bind the driver */
	if ((retval = usb_add_gadget_udc(&udc->gadget.dev, &udc->gadget)) != 0) {
    //if ((retval = device_add(&udc->gadget.dev)) != 0) {  // 39 Version, android init
	//if ((retval = device_add(&udc->gadget.dev)) != 0) {
		printk(KERN_EMERG "Error in device_add() : %d\n",retval);
		goto register_error;
	}
	printk(KERN_EMERG "TESTED 2 ++++++++++++\n");

	//dprintk(DEBUG_NORMAL, "binding gadget driver '%s'\n",driver->driver.name);

	if ((retval = driver->bind (&udc->gadget)) != 0) {
		device_del(&udc->gadget.dev);
		goto register_error;
	}
	printk(KERN_EMERG "TESTED 3 ++++++++++++\n");

	/* Enable udc */
	//s3c2410_udc_enable(udc);
	drime4_ss_udc_start(&udc->gadget, udc->driver);
    // need to be implemented.
	printk(KERN_EMERG "TESTED  4++++++++++++\n");

    enable_irq(udc->irq);

	return 0;

register_error:
	printk(KERN_EMERG "TESTED  5++++++++++++\n");
	udc->driver = NULL;
	udc->gadget.dev.driver = NULL;
	return retval;
}
#if 0
/*
 *	usb_gadget_unregister_driver
 */
int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct drime4_ss_udc *udc = our_udc;

	if (!udc)
		return -ENODEV;

	if (!driver || driver != udc->driver || !driver->unbind)
		return -EINVAL;

	//dprintk(DEBUG_NORMAL, "usb_gadget_unregister_driver() '%s'\n",driver->driver.name);

	/* report disconnect */
	if (driver->disconnect)
		driver->disconnect(&udc->gadget);

	driver->unbind(&udc->gadget);

	device_del(&udc->gadget.dev);
	udc->driver = NULL;

	/* Disable udc */
	//s3c2410_udc_disable(udc);
    // need to be implemented.

	return 0;
}

#endif

unsigned long g_event_buff_dma;
unsigned long g_ctrl_buff_dma;
static int __devinit drime4_ss_udc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct drime4_ss_udc *udc;
	struct resource *res;
	int epnum;
	int ret=0;
	int irq;


	dev_info(dev, " drime4_ss_udc_probe \n ");
	udc = devm_kzalloc(dev, sizeof(struct drime4_ss_udc) +
			sizeof(struct drime4_ss_udc_ep) * DRIME4_USB3_EPS,
			GFP_KERNEL);
	if (!udc) 
	{
		dev_err(dev, "cannot get memory\n");
		return -ENOMEM;
	}


	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) 
	{
		dev_err(dev, "cannot find register resource 0\n");
		return -EINVAL;
	}

	udc->phy_regs_res = devm_request_mem_region(dev, res->start, resource_size(res), dev_name(dev));
	if (!udc->phy_regs_res) 
	{
		dev_err(dev, "cannot reserve registers\n");
		return -ENOENT;
	}

	udc->phy_regs = devm_ioremap(dev, res->start, resource_size(res));
	if (!udc->phy_regs) 
	{
		dev_err(dev, "cannot map registers\n");
		return -ENXIO;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) 
	{
		dev_err(dev, "cannot find register resource 0\n");
		return -EINVAL;
	}

	udc->regs_res = devm_request_mem_region(dev, res->start, resource_size(res), dev_name(dev));
	if (!udc->regs_res) 
	{
		dev_err(dev, "cannot reserve registers\n");
		return -ENOENT;
	}

	udc->regs = devm_ioremap(dev, res->start, resource_size(res));
	if (!udc->regs) 
	{
		dev_err(dev, "cannot map registers\n");
		return -ENXIO;
	}

	udc->event_buff = dma_alloc_coherent(NULL,
					     DRIME4_USB3_EVENT_BUFF_BSIZE,
					     &udc->event_buff_dma,
					     GFP_KERNEL);
	if (!udc->event_buff) 
	{
		dev_err(dev, "cannot get memory for event buffer\n");
		return -ENOMEM;
	}
	memset(udc->event_buff, 0, DRIME4_USB3_EVENT_BUFF_BSIZE);

#ifdef CONFIG_BOOTLOADER_SNAPSHOT
	g_event_buff_dma = udc->event_buff_dma;
#endif

	udc->ctrl_buff = dma_alloc_coherent(NULL,
					    DRIME4_USB3_CTRL_BUFF_SIZE,
					    &udc->ctrl_buff_dma,
					    GFP_KERNEL);
	if (!udc->ctrl_buff) 
	{
		dev_err(dev, "cannot get memory for control buffer\n");
		ret = -ENOMEM;
		goto err_mem;
	}
	memset(udc->ctrl_buff, 0, DRIME4_USB3_CTRL_BUFF_SIZE);

#ifdef CONFIG_BOOTLOADER_SNAPSHOT
	g_ctrl_buff_dma = udc->ctrl_buff_dma;
#endif

	udc->ep0_buff = devm_kzalloc(dev, 512, GFP_KERNEL);
	if (!udc->ep0_buff) 
	{
		dev_err(dev, "cannot get memory for EP0 buffer\n");
		ret = -ENOMEM;
		goto err_mem;
	}

	udc->dev = dev;

//	device_initialize(&udc->gadget.dev);

	platform_set_drvdata(pdev, udc);
    setup_timer(&usb_sp_timer, usb_sp_timer_callback, 0);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) 
	{
		dev_err(dev, "cannot find irq\n");
		goto err_mem;
	}
	udc->irq = irq;

	ret = devm_request_irq(dev, irq, drime4_ss_udc_irq, 0,
		dev_name(dev), udc);
	if (ret < 0) 
	{
		dev_err(dev, "cannot claim IRQ\n");
		goto err_mem;
	}



	dev_set_name(&udc->gadget.dev, "gadget");

#if (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,39))
	udc->gadget.is_dualspeed = 1;
#endif
	udc->gadget.ops = &drime4_ss_udc_gadget_ops;
	udc->gadget.name = dev_name(dev);

	udc->gadget.dev.parent = dev;
	udc->gadget.dev.dma_mask = dev->dma_mask;

	/* setup endpoint information */

	INIT_LIST_HEAD(&udc->gadget.ep_list);
	udc->gadget.ep0 = &udc->eps[0].ep;

	/* allocate EP0 request */

	udc->ctrl_req = drime4_ss_udc_ep_alloc_request(&udc->eps[0].ep, GFP_KERNEL);
	if (!udc->ctrl_req) 
	{
		dev_err(dev, "failed to allocate ctrl req\n");
		goto err_mem;
	}

	/* reset the system */
	udc->clk = clk_get(dev, "usb3");
	if (IS_ERR(udc->clk)) 
	{
		dev_err(dev, "cannot get UDC clock\n");
		ret = -EINVAL;
		goto err_ep;
	}
	clk_enable(udc->clk);
	drime4_ss_udc_phy_set(pdev);

	/* initialise the endpoints now the core has been initialised */
	for (epnum = 0; epnum < DRIME4_USB3_EPS; epnum++) 
	{
		ret = drime4_ss_udc_initep(udc, &udc->eps[epnum], epnum);
		if (ret < 0) 
		{
			dev_err(dev, "cannot get memory for TRB\n");
			goto err_initep;
		}
	}
#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
	atomic_set(&udc->usb_status, -1); /* -1 means that it is not ready. */
	udc->get_usb_mode = drime4_get_usb_mode;
	udc->change_usb_mode = drime4_change_usb_mode;
	mutex_init(&udc->mutex);

	//if (pdev->dev.platform_data) {
	//	dev->ndev = pdev->dev.platform_data;
	//	printk("Register host notify driver : %s\n", dev->ndev->name);
	//	host_notify_dev_register(dev->ndev);
	//}
#endif

	our_udc = udc;

#if 1 // 39
	ret = device_register(&udc->gadget.dev);
	if (ret) 
	{
		dev_err(udc->dev, "failed to register gadget device\n");
		put_device(&udc->gadget.dev);
		goto err_initep;
	}
#endif

#if 1  //   39
	ret = usb_add_gadget_udc(dev, &udc->gadget);
	if (ret) 
	{
		dev_err(udc->dev, "failed to register udc\n");
		goto err_dev;
	}
#endif

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
	//disable_irq(udc->irq);
#endif

	our_platform_pdev = pdev;
	dev_info(dev, " drime4_ss_udc_probe ENDDDD \n ");

	return 0;

err_dev:
	device_unregister(&udc->gadget.dev);

err_initep:
	drime4_ss_udc_free_all_trb(udc);
	clk_disable(udc->clk);
	clk_put(udc->clk);
err_ep:

	drime4_ss_udc_ep_free_request(&udc->eps[0].ep, udc->ctrl_req);
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	kfree(udc->ep0_buff);
	/* static analysis bug fix 2012-09-10 Han Seungwon */
err_mem:
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	//kfree(udc->ep0_buff);
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	if (udc->ctrl_buff)
	{
		dma_free_coherent(NULL, DRIME4_USB3_CTRL_BUFF_SIZE, udc->ctrl_buff, udc->ctrl_buff_dma);
	}
	if (udc->event_buff)
	{
		dma_free_coherent(NULL, DRIME4_USB3_EVENT_BUFF_BSIZE, udc->event_buff, udc->event_buff_dma);
	}
	return ret;
}

static int __devexit drime4_ss_udc_remove(struct platform_device *pdev)
{
	struct drime4_ss_udc *udc = platform_get_drvdata(pdev);
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	if(udc == NULL)	return -1;
	/* static analysis bug fix 2012-09-10 Han Seungwon */

	//by wonjung1.kim
	usb_del_gadget_udc(&udc->gadget);

	clk_disable(udc->clk);
	clk_put(udc->clk);

	drime4_ss_udc_free_all_trb(udc);
	dma_free_coherent(NULL, DRIME4_USB3_CTRL_BUFF_SIZE, udc->ctrl_buff, udc->ctrl_buff_dma);
	dma_free_coherent(NULL, DRIME4_USB3_EVENT_BUFF_BSIZE, udc->event_buff, udc->event_buff_dma);

	return 0;
}

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE

#if 1
/*
 *	udc_reinit - initialize software state
 */
static void udc_reinit(struct drime4_ss_udc *dev)
{
	unsigned int i;


	/* device/ep0 records init */
	INIT_LIST_HEAD(&dev->gadget.ep_list);
	INIT_LIST_HEAD(&dev->gadget.ep0->ep_list);
	dev->ep0_state = EP0_WAIT_NRDY; //WAIT_FOR_SETUP;	//dev->ep0_state = EP0_WAIT_NRDY; //WAIT_FOR_SETUP;

	/* basic endpoint records init */
	for (i = 0; i < DRIME4_USB3_EPS; i++) 
	{
		struct drime4_ss_udc_ep *ep = &dev->eps[i];

		if (i != 0)
		{
			list_add_tail(&ep->ep.ep_list, &dev->gadget.ep_list);
		}

		ep->ep.desc = 0;
		ep->halted = 0;
		INIT_LIST_HEAD(&ep->queue);
		ep->req = 0;
	}

	/* the rest was statically initialized, and is read-only */
}
#endif

int drime4_vbus_enable(struct usb_gadget *gadget, int enable)
{
	unsigned long flags;
	//struct s3c_udc *dev = the_controller;
	//struct drime4_ss_udc *udc = our_udc;

	struct drime4_ss_udc *udc = container_of(gadget, struct drime4_ss_udc, gadget);


	u32 dev_gctl;
	struct usb_composite_dev *cdev;

	/* static analysis bug fix 2012-09-10 Han Seungwon */
	//dev_err(udc->dev, "udc->enabled=%d,=%d\n", udc->udc_enabled, enable);
	/* static analysis bug fix 2012-09-10 Han Seungwon */

	/* static analysis bug fix 2012-09-18 Han Seungwon */
	if(gadget == NULL)
	{
		printk(KERN_ERR "[%s] gadget is Null \n", __func__);
		return 0;
	}
	/* static analysis bug fix 2012-09-18 Han Seungwon */

	if (udc->udc_enabled != enable) 
	{
		udc->udc_enabled = enable;
		if (!enable) 
		{
			spin_lock_irqsave(&udc->lock, flags);
			dev_gctl = readl(DRIME4_USB3_GCTL);
			dev_err(udc->dev, "drime4_vbus_enable 1 \n");
			if ((dev_gctl & DRIME4_USB3_GCTL_PwrDnScale(1))==0) 
			{
				if (udc->driver) 
				{
					dev_err(udc->dev, "drime4_vbus_enable SUB1 \n");

					cdev=get_gadget_data(&udc->gadget);
					//cdev->mute_switch = (bool) 0;
					spin_unlock(&udc->lock);
					udc->driver->disconnect(&udc->gadget);
					spin_lock(&udc->lock);
				}
			}
			dev_err(udc->dev, "drime4_vbus_enable 2 \n");

			//stop_activity(dev, dev->driver);
			drime4_ss_udc_stop(&udc->gadget, udc->driver);
			dev_err(udc->dev, "drime4_vbus_enable 3 \n");

			spin_unlock_irqrestore(&udc->lock, flags);
			//udc_disable(dev);
			//drime4_ss_udc_pullup(&udc->gadget, 0);
			drime4_udc_soft_disconnect(udc);
			dev_err(udc->dev, "drime4_vbus_enable 4 \n");

			//s3c_udc_power(dev, 0);
		} 
		else 
		{
		
			dev_err(udc->dev, "drime4_vbus_enable 5 \n");
			//clk_enable(udc->clk);

			//s3c_udc_power(dev, 1);
			drime4_ss_udc_start(&udc->gadget, udc->driver);
			//udc_reinit(dev);
			//udc_enable(dev);
			//call_gadget(udc, connect);

			//dev_info(udc->dev, "A- drime4_ss_udc_start\n");
			dev_err(udc->dev, "drime4_vbus_enable 6 \n");
			udc_reinit(udc);
			//drime4_ss_udc_pullup(&udc->gadget, 1);
			//drime4_udc_soft_connect(udc);

			//udc->driver->connect(&udc->gadget);
			// dev_info(udc->dev, "A- drime4_ss_udc_pullup\n");

		}
	} 
	else
	{
		//dev_err(&udc->gadget.dev, "%s, udc : %d, en : %d\n", __func__, udc->udc_enabled, enable);	/* static analysis bug fix 2012-09-18 Han Seungwon */
	}

	return 0;
}



/* Description : Get host mode
 * Return value :
 *                -> USB_CABLE_DETACHED   : disabled udc
 *		  -> USB_CABLE_ATTACHED   : enabled udc
 *		  -> USB_OTGHOST_DETACHED : disabled otg host
 *		  -> USB_OTGHOST_ATTACHED : enabled otg host
                  ->                  -1  : I don't know yet. USB Switch should set usb mode first.
 * Written by SoonYong, Cho (Tue 16, Nov 2010)
 */
static int drime4_get_usb_mode()
{
	//struct s3c_udc *dev = the_controller;
	struct drime4_ss_udc *udc = our_udc;

	dev_err(udc->dev, "current = %d\n", atomic_read(&udc->usb_status));

	return atomic_read(&udc->usb_status);
}

/* Description : Change usb mode.
 * Parameters  : int mode
 *                -> USB_CABLE_DETACHED   : disable udc
 *		  -> USB_CABLE_ATTACHED   : enable udc
 *		  -> USB_OTGHOST_DETACHED : disable otg host
 *		  -> USB_OTGHOST_ATTACHED : enable otg host
 * Return value : 0 (success), -1 (Failed)
 * Written by SoonYong, Cho (Tue 16, Nov 2010)
 */
static int drime4_change_usb_mode(int mode)
{
	//struct s3c_udc *dev = the_controller;
 	struct drime4_ss_udc *dev = our_udc;

	int retval = 0;

	mutex_lock(&dev->mutex);
	dev_err(dev->dev, "usb cable = %d\n", mode);
	printk(KERN_EMERG "drime4_change_usb_mode() 00 \n");

#if 0
	if(atomic_read(&dev->usb_status) == USB_OTGHOST_ATTACHED) {
		if(mode == USB_CABLE_DETACHED || mode == USB_CABLE_ATTACHED ||
				mode == USB_CABLE_DETACHED_WITHOUT_NOTI) {
			DEBUG("Skip requested mode (%d), current mode=%d\n",mode, atomic_read(&dev->usb_status));
			mutex_unlock(&dev->mutex);
			return -1;
		}

	}
	if(atomic_read(&dev->usb_status) == USB_CABLE_ATTACHED) {
		if(mode == USB_OTGHOST_DETACHED || mode == USB_OTGHOST_ATTACHED) {
#ifdef USE_WORKAROUND_MUIC_BUG
			DEBUG("Did not skip requested mode (%d), current mode=%d\n",mode, atomic_read(&dev->usb_status));
			retval = drime4_vbus_enable(NULL, 0);
#else
			DEBUG("Skip requested mode (%d), current mode=%d\n",mode, atomic_read(&dev->usb_status));
			mutex_unlock(&dev->mutex);
			return -1;
#endif
		}

	}
#endif
	if(atomic_read(&dev->usb_status) == mode) {
		dev_err(dev->dev, "Skip requested mode (%d), current mode=%d\n",mode, atomic_read(&dev->usb_status));
		mutex_unlock(&dev->mutex);
		return -1;
	}

	switch(mode) {
		case 0 : //USB_CABLE_DETACHED:
#ifdef USE_CPUFREQ_LOCK
			s5pv310_cpufreq_lock_free(DVFS_LOCK_ID_USB);
			DEBUG("cpufreq_lock_free\n");
#endif
			printk(KERN_EMERG "drime4_change_usb_mode() 01 \n");
			if(dev->udc_enabled == 0) {
				printk(KERN_EMERG "drime4_change_usb_mode() 02 \n");
				drime4_vbus_enable(NULL, 1);
				dev->gadget.speed = USB_SPEED_HIGH;
			}
			retval = drime4_vbus_enable(NULL, 0);
			atomic_set(&dev->usb_status, 0);//USB_CABLE_DETACHED);
			break;
#if 0
		case USB_CABLE_DETACHED_WITHOUT_NOTI:
			dev->gadget.speed = USB_SPEED_UNKNOWN;
			retval = s3c_vbus_enable(NULL, 0);
			break;
#endif
		case 1: //USB_CABLE_ATTACHED:
#ifdef USE_CPUFREQ_LOCK
			s5pv310_cpufreq_lock(DVFS_LOCK_ID_USB, CPU_L0);
			CSY_DBG_ESS("cpufreq_lock\n");
#endif
			retval = drime4_vbus_enable(NULL, 1);
			atomic_set(&dev->usb_status, 1);//USB_CABLE_ATTACHED);
			break;

#ifdef CONFIG_USB_S3C_OTG_HOST
		case USB_OTGHOST_DETACHED:
#ifdef USE_CPUFREQ_LOCK_FOR_OTG_HOST
			s5pv310_cpufreq_lock_free(DVFS_LOCK_ID_USB);
			DEBUG("cpufreq_lock_free\n");
#endif
			s3c_udc_power(dev, 0);
			dev->ndev->mode = NOTIFY_NONE_MODE;
			host_state_notify(dev->ndev, NOTIFY_HOST_REMOVE);
			if(atomic_read(&dev->usb_status)==USB_OTGHOST_ATTACHED) {
				platform_driver_unregister(&s5pc110_otg_driver);
				DEBUG("otg host - unregistered\n");
			}

			/* irq setup after old hardware state is cleaned up */
			retval = request_irq(IRQ_OTG, s3c_udc_irq, 0, driver_name, dev);
			if (retval != 0) {
				DEBUG("otg host - can't get irq %i, err %d\n", IRQ_OTG, retval);
				mutex_unlock(&dev->mutex);
				return -1;
			}
			atomic_set(&dev->usb_status, USB_OTGHOST_DETACHED);
			break;

		case USB_OTGHOST_ATTACHED:
#ifdef USE_CPUFREQ_LOCK_FOR_OTG_HOST
			s5pv310_cpufreq_lock(DVFS_LOCK_ID_USB, CPU_L0);
			DEBUG("cpufreq_lock\n");
#endif
			if(!atomic_read(&dev->usb_status)!=USB_OTGHOST_ATTACHED) {
				free_irq(IRQ_OTG, dev);
				s3c_udc_power(dev, 1);

				if (platform_driver_register(&s5pc110_otg_driver) < 0) {
					DEBUG("otg host : Fail register\n");
					atomic_set(&dev->usb_status , USB_OTGHOST_DETACHED);
					mutex_unlock(&dev->mutex);
					return -1;
				}
				else {
					DEBUG("otg host : registered \n");
					atomic_set(&dev->usb_status , USB_OTGHOST_ATTACHED);
				}
			}
			else {
				DEBUG("otg host : Already otg mode, please check status.\n");
			}
			atomic_set(&dev->usb_status, USB_OTGHOST_ATTACHED);
			dev->ndev->mode = NOTIFY_HOST_MODE;
			host_state_notify(dev->ndev, NOTIFY_HOST_ADD);
			break;
#endif /* CONFIG_USB_S3C_OTG_HOST */
	}
	dev_err(dev->dev, "change mode ret=%d\n",retval);
	mutex_unlock(&dev->mutex);
	return 0;
}
#endif /* CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE */


# ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
static int drime4_ss_udc_suspend(struct platform_device *pdev, pm_message_t state)
{
	//struct s3c_udc *dev = the_controller;
	struct drime4_ss_udc *dev = our_udc;

	//printk(KERN_EMERG "is_on = 900, disconnect\n");
	our_udc->driver->disconnect(&our_udc->gadget);

	indx = 1;

	dev_err(dev->dev, "udc suspend %d \n", (dev->udc_enabled) );
	//disable_irq(IRQ_OTG);
	disable_irq(dev->irq);


	if (dev->driver && dev->driver->suspend)
		dev->driver->suspend(&dev->gadget);

	if (dev->udc_enabled)
		usb_gadget_vbus_disconnect(&dev->gadget);

	return 0;
}

static int drime4_ss_udc_resume(struct platform_device *pdev)
{
	//SW_DBG_P0_HIGH();

	//struct s3c_udc *dev = the_controller;
	struct drime4_ss_udc *dev = our_udc;
	struct device *usb_dev = &pdev->dev;

    reset_flag =0;
    timerflag  =0;
	dev_err(dev->dev, "udc resume %d \n", (int)(&dev->usb_status));
	if (dev->driver && dev->driver->resume)
		dev->driver->resume(&dev->gadget);

	//enable_irq(IRQ_OTG);
	enable_irq(dev->irq);

	if(atomic_read(&dev->usb_status) == 1) //USB_CABLE_ATTACHED)
		usb_gadget_vbus_connect(&dev->gadget);

	/* write 0x0000040D to 0x30160004(DRIME4_PHY_CFG1) */
	/* write 0x00000000 to 0x30160014(DRIME4_PHY_CFG5) */
#if 1
	writel(0x0000040D, dev->phy_regs + DRIME4_PHY_CFG1);
	writel(0x00000000, dev->phy_regs + DRIME4_PHY_CFG5);
#endif


#if 0
	// USB3 CLK DISABLE
	dev->clk = clk_get(usb_dev, "usb3");
	if (IS_ERR(dev->clk)) 
	{
		printk(KERN_EMERG "drime4_ss_udc_resume() cannot get UDC clock\n");
		return -1;
	}
	clk_disable(dev->clk);

	//our_platform_pdev = pdev;
#else
	drime4_ss_udc_clk_ctrl(1);	// ON
	drime4_ss_udc_clk_ctrl(2);	// OFF
#endif
    setup_timer(&usb_sp_timer, usb_sp_timer_callback, 0);

	//SW_DBG_P0_LOW();
	return 0;
}
#else
#define drime4_ss_udc_suspend NULL
#define drime4_ss_udc_resume NULL
#endif


/* is_on : 0 - start udc
 *		 1 - stop udc
*/
void usb_drime4_ss_udc_run_stop(unsigned int is_on)
{
	struct drime4_ss_udc *dev = our_udc;
	struct platform_device *pdev = our_platform_pdev;

	struct device *usb_dev = &pdev->dev;

	if(is_on > 1)
	{
		return;
	}
#if 0
	dev->clk = clk_get(usb_dev, "usb3");
	if (IS_ERR(dev->clk)) 
	{
		printk(KERN_EMERG "drime4_ss_udc_resume() cannot get UDC clock\n");
		return;
	}
	
	if(is_on == 1)
	{
		clk_enable(dev->clk);
		mdelay(100);
		
		/* write 0x00000000 to 0x30160004(DRIME4_PHY_CFG1) */
		/* write 0x00000001 to 0x30160014(DRIME4_PHY_CFG5) */
		writel(0x00000000, dev->phy_regs + DRIME4_PHY_CFG1);
		writel(0x00000001, dev->phy_regs + DRIME4_PHY_CFG5);
		print_cnt = 0;
	}
#else
	dev->clk = clk_get(usb_dev, "usb3");
	if (IS_ERR(dev->clk)) 
	{
		printk(KERN_EMERG "drime4_ss_udc_resume() cannot get UDC clock\n");
		return;
	}
	
	if(is_on == 1)
	{
		drime4_ss_udc_clk_ctrl(1);	//ON
		//msleep(100);
		
		/* write 0x00000000 to 0x30160004(DRIME4_PHY_CFG1) */
		/* write 0x00000001 to 0x30160014(DRIME4_PHY_CFG5) */
		writel(0x00000000, dev->phy_regs + DRIME4_PHY_CFG1);
		writel(0x00000001, dev->phy_regs + DRIME4_PHY_CFG5);
		print_cnt = 0;
	}

#endif
	
	//dev_info(dev->dev, "usb_drime4_ss_udc_run_stop %d\n", is_on);
	drime4_ss_udc_run_stop(dev, is_on);
#if 0
	if(is_on == 0)
	{
		mdelay(100);
		clk_disable(dev->clk);
	}
#else
	if(is_on == 0)
	{
#if 1
		writel(0x0000040D, dev->phy_regs + DRIME4_PHY_CFG1);
		writel(0x00000000, dev->phy_regs + DRIME4_PHY_CFG5);
#endif

		msleep(100);
		drime4_ss_udc_clk_ctrl(2);	//OFF
	}
#endif
	if(is_on == 10)
	{
		printk(KERN_EMERG "is_on = 10, indx clear\n");
		printk(KERN_EMERG "is_on = 10, indx = %d\n", indx);
		indx = 1;
		printk(KERN_EMERG "is_on = 10, indx = %d\n", indx);
	}
	else if(is_on == 100)
	{
		printk(KERN_EMERG "is_on = 100, print our_udc\n");

		printk(KERN_EMERG "our_udc->dev = %p\n", our_udc->dev);
		printk(KERN_EMERG "our_udc->driver = %p\n", our_udc->driver);
		printk(KERN_EMERG "our_udc->regs = %p\n", our_udc->regs);
		printk(KERN_EMERG "our_udc->phy_regs = %p\n", our_udc->phy_regs);
		printk(KERN_EMERG "our_udc->regs_res = %p\n", our_udc->regs_res);
		printk(KERN_EMERG "our_udc->phy_regs_res = %p\n", our_udc->phy_regs_res);
		printk(KERN_EMERG "our_udc->irq = %d\n", our_udc->irq);
		printk(KERN_EMERG "our_udc->clk = %p\n", our_udc->clk);
		printk(KERN_EMERG "our_udc->event_buff = %p\n", our_udc->event_buff);
		printk(KERN_EMERG "our_udc->event_buff_dma = %p\n", our_udc->event_buff_dma);
		printk(KERN_EMERG "our_udc->eps_enabled = %d\n", our_udc->eps_enabled);
		printk(KERN_EMERG "our_udc->ep0_state = %d\n", our_udc->ep0_state);
		printk(KERN_EMERG "our_udc->ep0_three_stage = %d\n", our_udc->ep0_three_stage);
		printk(KERN_EMERG "our_udc->ep0_buff = %p\n", our_udc->ep0_buff);
		printk(KERN_EMERG "our_udc->ctrl_buff = %p\n", our_udc->ctrl_buff);
		printk(KERN_EMERG "our_udc->ctrl_buff_dma = %p\n", our_udc->ctrl_buff_dma);
		printk(KERN_EMERG "our_udc->ep0_reply = %p\n", our_udc->ep0_reply);
		printk(KERN_EMERG "our_udc->ctrl_req = %p\n", our_udc->ctrl_req);
		printk(KERN_EMERG "our_udc->status = %d\n", our_udc->status);
		printk(KERN_EMERG "our_udc->udc_vcc_d = %p\n", our_udc->udc_vcc_d);
		printk(KERN_EMERG "our_udc->udc_vcc_a = %p\n", our_udc->udc_vcc_a);
		printk(KERN_EMERG "our_udc->udc_enabled = %p\n", our_udc->udc_enabled);
		printk(KERN_EMERG "our_udc->get_usb_mode = %p\n", our_udc->get_usb_mode);
		printk(KERN_EMERG "our_udc->change_usb_mode = %p\n", our_udc->change_usb_mode);
		printk(KERN_EMERG "our_udc->eps = %p\n", our_udc->eps);
	}
	else if(is_on == 101)
	{
		printk(KERN_EMERG "is_on = 101, print our_platform_pdev\n");
		
		printk(KERN_EMERG "our_platform_pdev->dev.dma_mask = %p\n", our_platform_pdev->dev.dma_mask);
		printk(KERN_EMERG "our_platform_pdev->dev.coherent_dma_mask = %x\n", our_platform_pdev->dev.coherent_dma_mask);
	}
	else if(is_on == 102)
	{
		printk(KERN_EMERG "our_udc->ep0_state = %d\n", our_udc->ep0_state);
		printk(KERN_EMERG "our_udc->ep0_three_stage = %d\n", our_udc->ep0_three_stage);
		printk(KERN_EMERG "our_udc->status = %d\n", our_udc->status);

		our_udc->ep0_state = EP0_UNCONNECTED;
		our_udc->ep0_three_stage = 0;
		our_udc->status = 0;

		printk(KERN_EMERG "our_udc->ep0_state = %d\n", our_udc->ep0_state);
		printk(KERN_EMERG "our_udc->ep0_three_stage = %d\n", our_udc->ep0_three_stage);
		printk(KERN_EMERG "our_udc->status = %d\n", our_udc->status);
	}
	else if(is_on == 500)
	{
		printk(KERN_EMERG "is_on = 500, udc_reinit\n");
		udc_reinit(dev);
	}
	else if(is_on == 600)
	{
		printk(KERN_EMERG "is_on = 600, drime4_change_usb_mode(0)\n");
		drime4_change_usb_mode(0);
	}
	else if(is_on == 601)
	{
		printk(KERN_EMERG "is_on = 601, drime4_change_usb_mode(1)\n");
		drime4_change_usb_mode(1);
	}
	else if(is_on == 700)
	{
		printk(KERN_EMERG "is_on = 700, USB RESET\n");
		drime4_ss_udc_irq_usbrst(our_udc);
	}
	else if(is_on == 900)
	{
		printk(KERN_EMERG "is_on = 900, disconnect\n");
		our_udc->driver->disconnect(&our_udc->gadget);
	}
	//mdelay(100);
}
EXPORT_SYMBOL(usb_drime4_ss_udc_run_stop);

static struct platform_driver drime4_ss_udc_driver = {
	.driver		= {
		.name	= "drime4-dwc3",
		.owner	= THIS_MODULE,
	},
	.probe		= drime4_ss_udc_probe,
	.remove		= __devexit_p(drime4_ss_udc_remove),
	.suspend	= drime4_ss_udc_suspend,
	.resume		= drime4_ss_udc_resume,
};

static int __init drime4_ss_udc_modinit(void)
{
	return platform_driver_register(&drime4_ss_udc_driver);
}

static void __exit drime4_ss_udc_modexit(void)
{
	platform_driver_unregister(&drime4_ss_udc_driver);
}

//EXPORT_SYMBOL(usb_gadget_unregister_driver);
EXPORT_SYMBOL(usb_gadget_register_driver);

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(drime4_ss_udc_modinit);
#else
fast_dev_initcall(drime4_ss_udc_modinit);
#endif
module_exit(drime4_ss_udc_modexit);

MODULE_DESCRIPTION("DRIME4 SuperSpeed USB 3.0 Device Controller");
MODULE_AUTHOR("Anton Tikhomirov <av.tikhomirov@samsung.com>");
MODULE_LICENSE("GPL");
