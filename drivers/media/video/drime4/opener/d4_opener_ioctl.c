/**
 * @file d4_opener_ioctl.c
 * @brief DRIMe4 OPENER Ioctl Control Function File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
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

#include <linux/uaccess.h>
#include <asm/cacheflush.h>

#include <mach/d4_mem.h>
#include "d4_opener_if.h"
#include "media/drime4/opener/d4_opener_ioctl.h"

/**
 * @brief OPENER device open
 * @fn int opener_open(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
int opener_open(struct inode *inode, struct file *filp)
{
   return 0;
}

/**
 * @brief OPENER device release
 * @fn int opener_release(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
static int opener_release(struct inode *inode, struct file *filp)
{
   return 0;
}

/**
 * @brief OPENER device read
 * @fn ssize_t opener_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
ssize_t opener_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
   return 0;
}

/**
 * @brief OPENER device write
 * @fn ssize_t opener_write(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
ssize_t opener_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
   return 0;
}

/**
 * @brief OPENER DD interface function
 * @fn long opener_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param struct file *filp, unsigned int cmd, unsigned long arg
 * @return long
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
long opener_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long err = -1;
	int size, ret = 0;
	enum kdd_devices dev = 0;
	struct kdd_open_pid_list open_pid_list;

	memset(&open_pid_list, 0x0, sizeof(struct kdd_open_pid_list));

	if (_IOC_TYPE(cmd) != OPENER_MAGIC)
		return err;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = access_ok(VERIFY_WRITE, (void *)arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret = access_ok(VERIFY_READ, (void *)arg, size);

	if (!ret)
		return err;

	switch (cmd) {
	case OPENER_IOCTL_GET_OPEN_CNT:
		ret = copy_from_user((void *) &dev, (const void *) arg, size);
		if (ret < 0) {
			printk("[OPENER IOCTL, %s, %d] copy_from_user fail\n", __FILE__, __LINE__);
			return ret;
		}
		return d4_get_kdd_open_count(dev);
		break;

	case OPENER_IOCTL_RESET_OPEN_CNT:
		ret = copy_from_user((void *) &dev, (const void *) arg, size);
		if (ret < 0) {
			printk("[OPENER IOCTL, %s, %d] copy_from_user fail\n", __FILE__, __LINE__);
			return ret;
		}
		d4_reset_kdd_open_count(dev);
		break;

	case OPENER_IOCTL_PRINT_PID_LIST:
		ret = copy_from_user((void *) &dev, (const void *) arg, size);
		if (ret < 0) {
			printk("[OPENER IOCTL, %s, %d] copy_from_user fail\n", __FILE__, __LINE__);
			return ret;
		}
		d4_print_kdd_open_pid_list(dev);
		break;

	case OPENER_IOCTL_GET_PID_LIST:
		ret = copy_from_user((void *) &open_pid_list, (const void *) arg, size);
		if (ret < 0) {
			printk("[OPENER IOCTL, %s, %d] copy_from_user fail\n", __FILE__, __LINE__);
			return ret;
		}

		ret = d4_get_kdd_open_pid(&open_pid_list);
		if (ret < 0)
			return ret;

		ret = copy_to_user((void *) arg, (const void *) &open_pid_list, size);
		if (ret < 0) {
			printk("[OPENER IOCTL, %s, %d] copy_to_user fail\n", __FILE__, __LINE__);
			return ret;
		}
		break;

	default:
		break;
	}
   return ret;
}

const struct file_operations opener_ops = {
		.open = opener_open,
		.release = opener_release,
		.read = opener_read,
		.write = opener_write,
		.unlocked_ioctl = opener_ioctl,
};

MODULE_AUTHOR("Wooram Son <wooram.son@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 OPENER driver using ioctl");
MODULE_LICENSE("GPL");

