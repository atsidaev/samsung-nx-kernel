/**
 * @file d4_bma.c
 * @brief DRIMe4 BMA
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * 2012 Samsung Electronics
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
#include <linux/module.h>

#include <mach/bma/d4_bma.h>
#include "d4_bma_if.h"

extern const struct file_operations bma_ops;

static struct miscdevice bma_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = BMA_MODULE_NAME,
		.fops = &bma_ops,
};

/**
 * @brief BMA device probe
 * @fn bma_probe(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int __devinit bma_probe(struct platform_device *pdev)
{
	int ret = 0;
	/* bma device registration */
	ret = misc_register(&bma_miscdev);
	if (ret < 0)
		dev_err(&pdev->dev, "Failed to register misc driver.\n");

	d4_bma_set_dev_info(&pdev->dev);

	return 0;
}


/**
 * @brief BMA device remove
 * @fn bma_remove(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int bma_remove(struct platform_device *pdev)
{
	misc_deregister(&bma_miscdev);

	return 0;
}

/**
 * @brief BMA device suspend
 * @fn bma_suspend(struct platform_device *pdev, pm_message_t state)
 * @param struct platform_device *pdev, pm_message_t state
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int bma_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

/**
 * @brief BMA device resume
 * @fn bma_resume(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int bma_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver bma_driver = {
	.probe  = bma_probe,
	.remove = bma_remove,
	.suspend = bma_suspend,
	.resume = bma_resume,
	.driver = {
		.name = BMA_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

/**
 * @brief BMA device register
 * @fn bma_register(void)
 * @param void
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int bma_register(void)
{
	platform_driver_register(&bma_driver);

	return 0;
}

/**
 * @brief BMA device unregister
 * @fn bma_unregister(void)
 * @param void
 * @return void
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static void bma_unregister(void)
{
	platform_driver_unregister(&bma_driver);
}

module_init(bma_register);
module_exit(bma_unregister);

MODULE_AUTHOR("Junkwon Choi <junkwon.choi@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 BMA driver File");
MODULE_LICENSE("GPL");
