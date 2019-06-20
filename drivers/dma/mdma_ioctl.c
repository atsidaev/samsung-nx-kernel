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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <mach/dma.h>
#include <linux/miscdevice.h>

#include <mach/map.h>
#include <linux/fb.h>
#include <linux/mdma_ioctl.h>

#ifdef CONFIG_DMA_ENGINE
#include <linux/dmaengine.h>
#include <linux/amba/pl330.h>
#endif

enum dma_use {
	MDMA_USED,
	MDMA_FREE
};

/**
 * struct mdma_data
 * @device_entry: mdma channel list.
 * @done: variable for mdma irq completion.
 * @waittime: waiting time for mdma working.
 * @ch_used: used channel.
 * @len: transmition number of size(byte).
 * @chan: Pointer to write dma channel.
 * @src_dma: source address.
 * @dst_dma: destnation address.
  */
struct mdma_data {
	struct list_head device_entry;
	struct completion done;
	unsigned int waittime;
	enum dma_use ch_used;
	unsigned int len;
#ifdef CONFIG_DMA_ENGINE
	struct dma_chan *chan;
	dma_addr_t src_dma;
	dma_addr_t dst_dma;
#endif

};

static DEFINE_MUTEX(mdma_lock);
static LIST_HEAD(device_list);

static unsigned int n_channels;


static char mdma_channel[20];
static char mdma_device[20];

static struct miscdevice *gmdma_dev;

static bool mdma_match_channel(struct dma_chan *chan)
{
	if (mdma_channel[0] == '\0')
		return true;
	return strcmp(dma_chan_name(chan), mdma_channel) == 0;
}

static bool mdma_match_device(struct dma_device *device)
{
	if (mdma_device[0] == '\0')
		return true;
	return strcmp(dev_name(device->dev), mdma_device) == 0;
}

static bool mdma_filter(struct dma_chan *chan, void *param)
{
	if (!mdma_match_channel(chan) || !mdma_match_device(chan->device))
		return false;
	else
		return true;
}




static void drime4_mdma_irq(void *data)
{
	struct mdma_data *mdma = data;
	complete(&mdma->done);
}


static int drime4_mdma_memcpy(struct mdma_data *mdma)
{
	struct dma_device *dev;
	struct dma_chan *chan;
	unsigned long flags;

	int status = 0;
	struct dma_async_tx_descriptor *tx = NULL;


	flags = DMA_CTRL_ACK |
		DMA_COMPL_SRC_UNMAP_SINGLE |
		DMA_COMPL_DEST_UNMAP_SINGLE|DMA_PREP_INTERRUPT;

	chan = mdma->chan;
	dev = chan->device;

	tx = dev->device_prep_dma_memcpy(chan, mdma->dst_dma, mdma->src_dma, mdma->len, flags);
	tx->callback = drime4_mdma_irq;
	tx->callback_param = mdma;

	dmaengine_submit(tx);
	dma_async_issue_pending(chan);
/*	wait_for_completion_timeout(&mdma->done, msecs_to_jiffies(10000));*/
	wait_for_completion(&mdma->done);

	return status;
}

struct mdma_data *drime4_mdma_request(void)
{
	struct mdma_data *mdma;
	int found = 0;

	mutex_lock(&mdma_lock);

	list_for_each_entry(mdma, &device_list, device_entry) {

		if (mdma->ch_used == MDMA_FREE) {
			found = 1;
			mdma->ch_used = MDMA_USED;
			printk("MDMA Request found channel:%d\n", mdma->chan->chan_id);
			break;
		}
	}

	if (found != 1)
		mdma = NULL;

	mutex_unlock(&mdma_lock);
	return mdma;
}

static int drime4_mdma_copy(unsigned long long src, unsigned long long dst, unsigned int len)
{
	struct mdma_data *mdma;
	int status = 0;
	mdma = drime4_mdma_request();
	if (!mdma) {
		printk("MDMA Request Fail\n");
	return -1;
	} else {
		printk("MDMA Request Success:%d\n", mdma->ch_used);
	}

	mdma->src_dma = src;
	mdma->dst_dma = dst;
	mdma->len = len;

	status = drime4_mdma_memcpy(mdma);
	mdma->ch_used = MDMA_FREE;
	return status;
}


static long mdmadev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret;
	u32 size;
	struct mdma_set_data mdma_data;
	/*struct mdma_data *mdata = (struct mdma_data *) filp->private_data;*/

	ret = 0;
	size = _IOC_SIZE(cmd);

	switch (cmd) {
	case MDMA_IOCTL_COPY:
		ret = copy_from_user((void *)&mdma_data, (const void *)arg, size);

		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		drime4_mdma_copy(mdma_data.mdma_src, mdma_data.mdma_dst, mdma_data.mdma_len);
		break;

	default:
		break;
	}
	return ret;
}


static int mdmadev_open(struct inode *inode, struct file *filp)
{
	return 0;
}


static int mdmadev_release(struct inode *inode, struct file *filp)
{
	return 0;
}


const struct file_operations mdmadev_ops = { .open = mdmadev_open,
		.release = mdmadev_release, .unlocked_ioctl = mdmadev_ioctl };

static int drmie4_mdmadev_set(struct miscdevice *mdmadev, const char *fmt, ...)
{
	va_list vargs;
	va_start(vargs, fmt);
	mdmadev->name = kvasprintf(GFP_KERNEL, fmt, vargs);
	if (mdmadev->name == NULL)
		return -1;
	mdmadev->minor = MISC_DYNAMIC_MINOR;
	mdmadev->fops = &mdmadev_ops;
	return 1;
}


static int drime4_mdma_misc_set(void)
{
	int ret;
	struct miscdevice *mdmadev;
	mdmadev = kzalloc(sizeof(*mdmadev), GFP_KERNEL);

	if (!mdmadev) {
		printk("no memory for state\n");
		return -ENOMEM;
	}
	ret = drmie4_mdmadev_set(mdmadev, "mdmadev");

	if (ret == -1) {
		kfree(mdmadev);
		return -ENODEV;
	}
	ret = misc_register(mdmadev);
	if (ret < 0) {
		printk("failed to register MDMA ioctl\n");
		kfree(mdmadev->name);
		kfree(mdmadev);
		return -ENODEV;
	}
	gmdma_dev = mdmadev;
	return 0;
}


static int drime4_mdma_add_channel(struct dma_chan *chan)
{
	struct mdma_data	*md_data;

	md_data = kmalloc(sizeof(struct mdma_data), GFP_KERNEL);
	if (!md_data) {
		printk("MDMA: No memory for %s\n", dma_chan_name(chan));
		return -ENOMEM;
	}

	md_data->chan = chan;
	md_data->ch_used = MDMA_FREE;
	init_completion(&md_data->done);
	INIT_LIST_HEAD(&md_data->device_entry);
	list_add_tail(&md_data->device_entry, &device_list);
	n_channels++;
	/*printk("MDMA: Add Channel Success %s\n", dma_chan_name(chan));*/
	return 0;
}

static int drime4_mdma_release_channel(void)
{
	struct mdma_data	*md_data;
	struct mdma_data	*_md_data;
	list_for_each_entry_safe(md_data, _md_data, &device_list, device_entry) {
		list_del(&md_data->device_entry);
		/*printk("MDMA: delete Channel Success %d\n", md_data->chan->chan_id);*/
		dma_release_channel(md_data->chan);
	}
	return 0;
}


static int __init drime4_mdma_init(void)
{
	dma_cap_mask_t mask;
	struct dma_chan *chan;
	int err = 0;

	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);
	n_channels = 0;
	while (1) {
		chan = dma_request_channel(mask, mdma_filter, NULL);
		if (chan) {
			err = drime4_mdma_add_channel(chan);
			if (err) {
				printk("MDMA: Add Channel Fail %s\n", dma_chan_name(chan));
				break;
			}
		} else {
			printk("MDMA: Request Channel Fail\n");
			break;
		}
		if (n_channels >= 8)
			break;
	}
	err = drime4_mdma_misc_set();
	n_channels = 0;
	return err;
}


module_init(drime4_mdma_init);

static void __exit drime4_mdma_exit(void)
{
	drime4_mdma_release_channel();
	misc_deregister(gmdma_dev);
	return;
}
module_exit(drime4_mdma_exit);

MODULE_AUTHOR("kyuchun han, <kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("drime4 MDMA Controller Driver");
MODULE_LICENSE("GPL");

