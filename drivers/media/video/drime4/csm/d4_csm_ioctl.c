/**
 * @file d4_csm_ioctl.c
 * @brief DRIMe4 CSM(Capture Sequence Manager) Ioctl Control Function File
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#include <mach/csm/d4_csm.h>
#include "media/drime4/csm/d4_csm_ioctl.h"
#include "d4_csm_if.h"

/**
 * @brief CSM device open
 * @fn int d4_csm_open(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
int d4_csm_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief CSM device release
 * @fn int d4_csm_release(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
static int d4_csm_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief CSM device read
 * @fn ssize_t d4_csm_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
ssize_t d4_csm_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief CSM device write
 * @fn ssize_t d4_csm_write(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
ssize_t d4_csm_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief CSM DD interface function
 * @fn long d4_csm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param struct file *filp, unsigned int cmd, unsigned long arg
 * @return long
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
long d4_csm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long err = -1;
	int size = 0, ret = -1;

	struct csm_chg_wait_vd_mode wait_vd_data;
	struct csm_chg_read_out_mode read_out_data;

	if (_IOC_TYPE(cmd) != CSM_MAGIC)
		return err;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = access_ok(VERIFY_WRITE, (void *)arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret = access_ok(VERIFY_READ, (void *)arg, size);

	if (!ret)
		return err;

	err = 0;

	switch (cmd) {
	case CSM_IOCTL_CHANGE_STILL_WAIT_VD_MODE:
		memset(&wait_vd_data, 0, sizeof(struct csm_chg_wait_vd_mode));
		ret = copy_from_user((void *) &wait_vd_data, (const void *) arg, size);
		if (ret < 0) {
			printk("[OPENER IOCTL, %s, %d] copy_from_user fail\n", __FILE__, __LINE__);
			return ret;
		}
		csm_register_vsync_isr(CSM_CMD_CHG_WAIT_VD_MODE, (void *)&wait_vd_data);
		break;
	case CSM_IOCTL_CHANGE_STILL_READ_OUT_MODE:
		memset(&read_out_data, 0, sizeof(struct csm_chg_read_out_mode));
		ret = copy_from_user((void *) &read_out_data, (const void *) arg, size);
		if (ret < 0) {
			printk("[OPENER IOCTL, %s, %d] copy_from_user fail\n", __FILE__, __LINE__);
			return ret;
		}

		if(csm_register_vsync_isr(CSM_CMD_CHG_READ_OUT_MODE, (void *)&read_out_data)) {
			err = -1;
		}
		break;
	case CSM_IOCTL_GET_CSM_WDMA_STATE:
		if (copy_to_user((void *)arg, (const void *)csm_get_wdma_error_state(), sizeof(struct csm_wdma_state)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return err;
}

const struct file_operations d4_csm_ops = {
		.open 			= d4_csm_open,
		.release 			= d4_csm_release,
		.read 			= d4_csm_read,
		.write 			= d4_csm_write,
		.unlocked_ioctl 	= d4_csm_ioctl,
};

MODULE_AUTHOR("Gwon Haesu <haesu.gwon@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 CSM driver File");
MODULE_LICENSE("GPL");
