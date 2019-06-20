/**
 * @file d4_csm.c
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

extern const struct file_operations d4_csm_ops;

static struct miscdevice d4_csm_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name  = CSM_MODULE_NAME,
		.fops  = &d4_csm_ops,
};

/**
 * @brief CSM device probe
 * @fn d4_csm_probe(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
static int __devinit d4_csm_probe(struct platform_device *pdev)
{
	struct resource *res = NULL;
	int ret = 0;

	/* CSM device registration */
	ret = misc_register(&d4_csm_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		return -1;
	}

	csm_init();

	return 0;
}


/**
 * @brief CSM device remove
 * @fn d4_d4_csm_remove(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
static int d4_csm_remove(struct platform_device *pdev)
{
	misc_deregister(&d4_csm_miscdev);

	return 0;
}

/**
 * @brief CSM device suspend
 * @fn d4_csm_suspend(struct platform_device *pdev, pm_message_t state)
 * @param struct platform_device *pdev, pm_message_t state
 * @return int
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
static int d4_csm_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

/**
 * @brief CSM device resume
 * @fn d4_csm_resume(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
static int d4_csm_resume(struct platform_device *pdev)
{
	csm_init();

	return 0;
}

static struct platform_driver d4_csm_driver = {
	.probe   = d4_csm_probe,
	.remove  = d4_csm_remove,
	.suspend = d4_csm_suspend,
	.resume  = d4_csm_resume,
	.driver  = {
		.name  = CSM_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

/**
 * @brief CSM device register
 * @fn d4_csm_register(void)
 * @param void
 * @return int
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
static int d4_csm_register(void)
{
	platform_driver_register(&d4_csm_driver);
	return 0;
}

/**
 * @brief CSM device unregister
 * @fn d4_csm_unregister(void)
 * @param void
 * @return void
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * @note  NONE
 */
static void d4_csm_unregister(void)
{
	platform_driver_unregister(&d4_csm_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(d4_csm_register);
#else
fast_dev_initcall(d4_csm_register);
#endif
module_exit(d4_csm_unregister);

MODULE_AUTHOR("Gwon Haesu <haesu.gwon@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 CSM driver File");
MODULE_LICENSE("GPL");
