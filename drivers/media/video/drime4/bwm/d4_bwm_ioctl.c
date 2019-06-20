/**
 * @file d4_bwm_ioctl.c
 * @brief DRIMe4 BWM(Bandwidth Manager) Ioctl Control Function File
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
#include <asm/cacheflush.h>

#include <linux/d4-pmu.h>
#include <mach/bwm/d4_bwm.h>
#include "media/drime4/bwm/d4_bwm_ioctl.h"

static struct bwm_physical_reg_info phys_reg_info;

/**
 * @brief LCD Panel Manager device open
 * @fn int d4_bwm_open(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
int d4_bwm_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief LCD Panel Manager device release
 * @fn int d4_bwm_release(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int d4_bwm_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief LCD Panel Manager device read
 * @fn ssize_t d4_bwm_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
ssize_t d4_bwm_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief LCD Panel Manager device write
 * @fn ssize_t d4_bwm_write(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
ssize_t d4_bwm_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief LCD Panel Manager DD interface function
 * @fn long d4_bwm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param struct file *filp, unsigned int cmd, unsigned long arg
 * @return long
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
long d4_bwm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long err = -1;
	int size = 0, ret = -1;

	if (_IOC_TYPE(cmd) != BWM_MAGIC)
		return err;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = access_ok(VERIFY_WRITE, (void *)arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret = access_ok(VERIFY_READ, (void *)arg, size);

	if (!ret)
		return err;

	switch (cmd) {
	case BWM_IOCTL_GET_PHYS_REG_INFO:

		ret = copy_to_user((void *)arg, (const void *)&phys_reg_info, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;
	default:
		break;
	}

	return 0;
}

int d4_bwm_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size = 0;

	size     = (vma->vm_end - vma->vm_start);

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
			vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

const struct file_operations d4_bwm_ops = {
		.open 			= d4_bwm_open,
		.release 		= d4_bwm_release,
		.read 			= d4_bwm_read,
		.write 			= d4_bwm_write,
		.mmap 			= d4_bwm_mmap,
		.unlocked_ioctl = d4_bwm_ioctl,
};

static struct miscdevice d4_bwm_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name  = BWM_MODULE_NAME,
		.fops  = &d4_bwm_ops,
};

/**
 * @brief BWM device probe
 * @fn d4_bwm_probe(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int __devinit d4_bwm_probe(struct platform_device *pdev)
{
	struct resource *res = NULL;
	int ret = 0;

	/* BWM device registration */
	ret = misc_register(&d4_bwm_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		return -1;
	}

	/**< Get Physical Register information  */
	res = d4_pmu_res_request();
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region\n");
		ret = -ENODEV;
		goto err_d4_bwm_probe;
	}
	phys_reg_info.sonics_bus_ctrl_reg_start_addr 	= res->start;
	/**< sonics bus size: 0x4000 */
	phys_reg_info.sonics_bus_ctrl_reg_size  		= resource_size(res);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region\n");
		ret = -ENODEV;
		goto err_d4_bwm_probe;
	}

	phys_reg_info.drex_ctrl_reg_start_addr 	= res->start;
	phys_reg_info.drex_ctrl_reg_size 		= resource_size(res);
	release_resource(res);

	return 0;
	
err_d4_bwm_probe:
	misc_deregister(&d4_bwm_miscdev);
	return ret;
}


/**
 * @brief BWM device remove
 * @fn d4_d4_bwm_remove(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int d4_bwm_remove(struct platform_device *pdev)
{
	misc_deregister(&d4_bwm_miscdev);

	return 0;
}

/**
 * @brief BWM device suspend
 * @fn d4_bwm_suspend(struct platform_device *pdev, pm_message_t state)
 * @param struct platform_device *pdev, pm_message_t state
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int d4_bwm_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

/**
 * @brief BWM device resume
 * @fn d4_bwm_resume(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int d4_bwm_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver d4_bwm_driver = {
	.probe   = d4_bwm_probe,
	.remove  = d4_bwm_remove,
	.suspend = d4_bwm_suspend,
	.resume  = d4_bwm_resume,
	.driver  = {
		.name  = BWM_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

/**
 * @brief BWM device register
 * @fn d4_bwm_register(void)
 * @param void
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int d4_bwm_register(void)
{
	platform_driver_register(&d4_bwm_driver);
	return 0;
}

/**
 * @brief BWM device unregister
 * @fn d4_bwm_unregister(void)
 * @param void
 * @return void
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static void d4_bwm_unregister(void)
{
	platform_driver_unregister(&d4_bwm_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(d4_bwm_register);
#else
fast_dev_initcall(d4_bwm_register);
#endif
module_exit(d4_bwm_unregister);

MODULE_AUTHOR("Kim Sunghoon <bluesay.kim@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 BWM driver File");
MODULE_LICENSE("GPL");
