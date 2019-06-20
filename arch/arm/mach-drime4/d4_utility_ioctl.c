/**
 * @file	d4_utility_ioctl.c
 * @brief	Utility driver file for Samsung DRIMe4 using ioctl
 *
 * @author
 *
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/interrupt.h>

#include <asm/page.h>
#include <asm/cacheflush.h>
#include <mach/d4_mem.h>
#include <media/drime4/usih_type.h>
#include <mach/d4_utility_ioctl.h>
#include <mach/version_information.h>



struct miscdevice *bvdev;

extern int bridge_ic_on(void);

static int d4_utility_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int d4_utility_release(struct inode *inode, struct file *filp)
{
	return 0;
}


long d4_utility_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size;
	int err = -1;
	int ret = 0;
	unsigned char *v_name;
	EBdVersion bv_ver;

	if (_IOC_TYPE(cmd) != D4_UTILITY_MAGIC)
		return -1;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *) arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *) arg, size);

	if (!err)
		return -1;

	switch (cmd) {

	case D4_UTILITY_GET_BV:
		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		bv_ver = GetBoardVersion();
		v_name = GetBoardVersionName();
		printk("kernel board version :%d, %s\n", bv_ver, v_name);
		ret = copy_to_user((void *)arg,	&bv_ver, 1);
		break;

/*	case D4_UTILITY_GET_MIPI_IC:
		bridge_ic_on();*/
	default:
		break;
	}

	return 0;
}


const struct file_operations d4_utility_fops = { .open = d4_utility_open,
		.release = d4_utility_open, .unlocked_ioctl = d4_utility_ioctl };

static void d4_utility_ioctl_set(struct miscdevice *mdmadev)
{
	mdmadev->name = "bvcheck";
	mdmadev->minor = MISC_DYNAMIC_MINOR;
	mdmadev->fops = &d4_utility_fops;
}


static int d4_utility_misc_set(void)
{
	int ret;
	bvdev = kzalloc(sizeof(*bvdev), GFP_KERNEL);
	if (!bvdev) {
		printk("no memory for state\n");
		return -ENOMEM;;
	}
	d4_utility_ioctl_set(bvdev);
	ret = misc_register(bvdev);
	if (ret < 0) {
		printk("failed to register misc driver\n");
		kfree(bvdev);
		return -ENODEV;
	}
	return 0;
}


static int __init d4_utility_init(void)
{
	int err = -1;

	err = d4_utility_misc_set();
	return err;
}


static void __exit d4_utility_exit(void)
{
	misc_deregister(bvdev);
	return;
}


module_init(d4_utility_init);
module_exit(d4_utility_exit);


MODULE_AUTHOR("Kyuchun Han <kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 Utility for Version Check");
MODULE_LICENSE("GPL");

