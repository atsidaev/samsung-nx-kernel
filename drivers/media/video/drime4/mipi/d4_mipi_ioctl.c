
/**
 * @file d4_mipi_ioctl.c
 * @brief DRIMe4 MIPI Ioctl Control Function File
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
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

#include "d4_mipi_if.h"
#include "media/drime4/mipi/d4_mipi_ioctl.h"
#include "media/drime4/mipi/d4_mipi_dpc_pdma.h"


extern struct drime4_mipi *g_mipi;

/**
 * @brief MIPI device open
 * @fn d4_mipi_open(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
int d4_mipi_open(struct inode *inode, struct file *filp)
{
   /* int num = MINOR(inode->i_rdev);*/
   struct drime4_mipi *mipi_ctx = g_mipi;
   filp->private_data = mipi_ctx;
   return 0;
}

/**
 * @brief MIPI device release
 * @fn d4_mipi_release(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
static int d4_mipi_release(struct inode *inode, struct file *filp)
{
   return 0;
}

/**
 * @brief MIPI device read
 * @fn d4_mipi_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
ssize_t d4_mipi_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
   return 0;
}

/**
 * @brief MIPI device write
 * @fn d4_mipi_write(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
ssize_t d4_mipi_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
   return 0;
}

/**
 * @brief MIPI DD interface function
 * @fn mipi_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param struct file *filp, unsigned int cmd, unsigned long arg
 * @return long
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
long mipi_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size = MIPI_INITIAL;
	int err = MIPI_FAIL;

	unsigned int mipi_phys_start_addr;
	unsigned int time_out = 1000;

	if (_IOC_TYPE(cmd) != MIPI_MAGIC)
		return MIPI_FAIL;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *)arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *)arg, size);

	if (!err)
		return MIPI_FAIL;

	switch (cmd) {
	case MIPI_IOCTL_GET_PHYS_REG_INFO:
		mipi_get_phys_reg_info(&mipi_phys_start_addr);
		if (copy_to_user((void *)arg, (const void *)&mipi_phys_start_addr, size)) {
			printk(KERN_DEBUG "MIPI_IOCTL_GET_PHYS_REG_INFO fail");
			return -EFAULT;
		}
		break;

	case MIPI_IOCTL_CSIM_OPEN_IRQ:
		d4_Mipi_Csim_Enable_IRQ();
		break;

	case MIPI_IOCTL_CSIS_OPEN_IRQ:
		d4_Mipi_Csis_Enable_IRQ();
		break;

	case MIPI_IOCTL_CSIM_CLOSE_IRQ:
		d4_Mipi_Csim_Deinit_IntNum();
		break;

	case MIPI_IOCTL_CSIS_CLOSE_IRQ:
		d4_Mipi_Csis_Deinit_IntNum();
		break;

	case MIPI_IOCTL_CSIM_INIT_COMPLETION:
		d4_Mipi_Csim_Init_Completion();
		break;

	case MIPI_IOCTL_CSIS_INIT_COMPLETION:
		d4_Mipi_Csis_Init_Completion();
		break;

	case MIPI_IOCTL_CSIM_REGISTER_INT:
		err = d4_Mipi_Csim_register_interrupt();
		if (copy_to_user((void *) arg, (const void *) &err,
						size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case MIPI_IOCTL_CSIS_REGISTER_INT:
		err = d4_Mipi_Csis_register_interrupt();
		if (copy_to_user((void *) arg, (const void *) &err,
						size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case MIPI_IOCTL_CSIM_DEREGISTER_INT:
		err = d4_Mipi_Csim_deregister_interrupt();
		if (copy_to_user((void *) arg, (const void *) &err,
						size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case MIPI_IOCTL_CSIS_DEREGISTER_INT:
		err = d4_Mipi_Csis_deregister_interrupt();
		if (copy_to_user((void *) arg, (const void *) &err,
						size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case MIPI_IOCTL_CSIM_SEMAPHORE:
		if (copy_from_user((void *) &time_out, (const void *) arg,
						size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		err = d4_MIPI_CSIM_Get_OpSema(time_out);

		if (copy_to_user((void *) arg, (const void *) &time_out,
						size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case MIPI_IOCTL_CSIS_SEMAPHORE:
		if (copy_from_user((void *) &time_out, (const void *) arg,
						size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		err = d4_MIPI_CSIS_Get_OpSema(time_out);

		if (copy_to_user((void *) arg, (const void *) &time_out,
						size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case MIPI_IOCTL_SET_DPC_PDMA:
	{
		D4_Mipi_Dpc_st stDpc;
		
		if (copy_from_user((void *) &stDpc, (const void *) arg, sizeof(D4_Mipi_Dpc_st))) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		d4_mipi_dpc_pdma_init(&stDpc);
		break;
	}

	case MIPI_IOCTL_START_DPC_PDMA:
	{
		D4_Mipi_Dpc_st stDpc;
		if (copy_from_user((void *) &stDpc, (const void *) arg, sizeof(D4_Mipi_Dpc_st))) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		d4_mipi_dpc_pdma_start(&stDpc);
		break;
	}

	case MIPI_IOCTL_STOP_DPC_PDMA:
		{
			D4_Mipi_Dpc_st stDpc;
			if (copy_from_user((void *) &stDpc, (const void *) arg, sizeof(D4_Mipi_Dpc_st))) {
				printk("ioctl fail: [%d]", cmd);
				return -EFAULT;
			}
			d4_mipi_dpc_pdma_stop(&stDpc);
		
			break;
		}
	
	default:
		break;
	}

   return err;
}

const struct file_operations mipi_ops = {
		.open = d4_mipi_open,
		.release = d4_mipi_release,
		.read = d4_mipi_read,
		.write = d4_mipi_write,
		.unlocked_ioctl = mipi_ioctl,
};

MODULE_AUTHOR("Gunwoo Nam <gunwoo.nam@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 MIPI driver using ioctl");
MODULE_LICENSE("GPL");

