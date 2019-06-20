/* linux/drivers/pwm/pwmdev.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	Kyuchun Han <kyuchun.han@samsung.com>
 *
 * PWM IOCTL Device Driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/pwmdev_conf.h>
#include <linux/pwmdev.h>

#include <linux/d4_ht_pwm.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <mach/map.h>

#include <linux/pinctrl/pinmux.h>

#include <linux/d4_ptc.h>
#include <linux/ptc_ioctl.h>

struct ptc_data {
	dev_t devt;
	struct device *dev;
	struct ptc_device *ptc;
	struct list_head device_entry;
};

static LIST_HEAD(device_list);




static ssize_t ptcdev_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	return 0;
}

/* Write-only message with current device setup */
static ssize_t ptcdev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	return 0;
}


struct ptc_oneinput_info ctlinfo;


void test_handler(void *dev)
{
	struct ptc_device *ptc = dev;
	ptc_cnt_clear(ptc, PTC_ONE_INPUT);
}

static long ptcdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct ptc_data *data = (struct ptc_data *) filp->private_data;
	unsigned int size, ret;
	unsigned int value;
	struct ptc_config ptcdata;
	value = 0;
	size = _IOC_SIZE(cmd);

	switch (cmd) {
	case PTC_CONFIG_SET:
		ret = copy_from_user((void *) &ptcdata, (const void *) arg, size);
		if (ret < 0) {
			printk("PTC_CONFIG_SET failed\n");
			return ret;
		}
		ctlinfo.edge_type = ptcdata.edge_type;
		ctlinfo.pCallback = test_handler;
		ctlinfo.time = ptcdata.waittime;

		if (ptcdata.mode)
			ptc_count_set(data->ptc, PTC_ZPIC, ptcdata.count);
		else
			ptc_count_set(data->ptc, PTC_ZPIB, ptcdata.count);

		ptc_int_set(data->ptc, PTC_INT_ENABLE, ptcdata.mode);
		ret = ptc_oneinput_ctrl(data->ptc, &ctlinfo);
		break;
	case PTC_START:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		if (ret)
			goto err;

		ret = ptc_start(data->ptc, value);
		break;

	case PTC_STOP:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		if (ret)
			goto err;

		ptc_stop(data->ptc);
		break;

	case PTC_CNT_CLEAR:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		if (ret)
			goto err;

		ptc_cnt_clear(data->ptc, PTC_ONE_INPUT);
		break;


	default:
		ret = -1;
		break;
	}
err:
	return ret;
}

static int ptcdev_open(struct inode *inode, struct file *filp)
{
	struct ptc_data *data;
	int status = -1;
	list_for_each_entry(data, &device_list, device_entry) {
		if (data->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}
	if (status == 0) {
		filp->private_data = data;
		nonseekable_open(inode, filp);
	}
	return status;
}

static int ptcdev_release(struct inode *inode, struct file *filp)
{
	return 0;
}


int ptcdev_io_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size = 0;

	size = (vma->vm_end - vma->vm_start);

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
			vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}


const struct file_operations ptcdev_ops = { .open = ptcdev_open,
		.release = ptcdev_release, .read = ptcdev_read, .write = ptcdev_write,
		.unlocked_ioctl = ptcdev_ioctl, .mmap = ptcdev_io_mmap };


static int ptcdev_set(struct miscdevice *pwmdev, const char *fmt, ...)
{
	va_list vargs;
	va_start(vargs, fmt);
	pwmdev->name = kvasprintf(GFP_KERNEL, fmt, vargs);
	if (pwmdev->name == NULL)
		return -1;
	pwmdev->minor = MISC_DYNAMIC_MINOR;
	pwmdev->fops = &ptcdev_ops;
	return 1;
}

static int ptc_dev_probe(struct platform_device *pdev)
{
	struct ptc_data *data;
	struct miscdevice *ptcdev;
	int ret = 0;

	ptcdev = kzalloc(sizeof(*ptcdev), GFP_KERNEL);

	if (!ptcdev) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_allocdev;
	}


	ret = ptcdev_set(ptcdev, "ptcdev.%d", pdev->id);

	if (ret == -1) {
		kfree(ptcdev);
		return -1;
	}

	data = kzalloc(sizeof(*data), GFP_KERNEL);

	if (!data) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	data->dev = &pdev->dev;
	INIT_LIST_HEAD(&data->device_entry);

	data->ptc = ptc_request(pdev->id);

	if (IS_ERR(data->ptc)) {
		dev_err(&pdev->dev, "unable to request ptc\n");
		ret = PTR_ERR(data->ptc);
		goto err_ptc;
	}

	ret = misc_register(ptcdev);
	if (ret < 0) {
		printk("failed to register ptc io ctrl\n");
		goto err_ptc;
	}

	data->devt = MKDEV(10, ptcdev->minor);
	list_add(&data->device_entry, &device_list);
	platform_set_drvdata(pdev, data);

	return 0;

err_ptc:
	kfree(data);
err_alloc:
	kfree(ptcdev->name);
	kfree(ptcdev);

err_allocdev:
	return ret;
}

static int ptc_dev_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_PM
static int ptc_dev_suspend(struct platform_device *pdev,
		pm_message_t state)
{
	return 0;
}

static int ptc_dev_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define ptc_dev_suspend	NULL
#define ptc_dev_resume	NULL
#endif

static struct platform_driver ptc_dev_driver = { .driver = {
	.name = "ptcdev",
	.owner = THIS_MODULE,
}, .probe = ptc_dev_probe, .remove = ptc_dev_remove,
		.suspend = ptc_dev_suspend, .resume = ptc_dev_resume, };

static int __init ptc_dev_init(void)
{
	return platform_driver_register(&ptc_dev_driver);
}
module_init(ptc_dev_init);

static void __exit ptc_dev_exit(void)
{
	platform_driver_unregister(&ptc_dev_driver);
}
module_exit(ptc_dev_exit);

MODULE_DESCRIPTION("PTC based CLOCK COUNT Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ptcdev");

