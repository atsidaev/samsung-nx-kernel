/**
 * @file d4_opener.c
 * @brief DRIMe4 OPENER
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/clk.h>

#include <mach/opener/d4_opener.h>
#include "d4_opener_if.h"

extern const struct file_operations opener_ops;

static struct miscdevice opener_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = OPENER_MODULE_NAME,
		.fops = &opener_ops,
};

static int udd_device_init;


/**
 * @brief OPENER device probe
 * @fn opener_probe(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
static int __devinit opener_probe(struct platform_device *pdev)
{
	int ret = 0;
	/* opener device registration */
	ret = misc_register(&opener_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
	}

	d4_opener_set_dev_info(&pdev->dev);
	d4_opener_init_mutex();

	return 0;
}


/**
 * @brief OPENER device remove
 * @fn opener_remove(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
static int opener_remove(struct platform_device *pdev)
{
	misc_deregister(&opener_miscdev);

	return 0;
}

/**
 * @brief OPENER device suspend
 * @fn opener_suspend(struct platform_device *pdev, pm_message_t state)
 * @param struct platform_device *pdev, pm_message_t state
 * @return int
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
static int opener_suspend(struct platform_device *pdev, pm_message_t state)
{
	misc_deregister(&opener_miscdev);
	return 0;
}

/**
 * @brief OPENER device resume
 * @fn opener_resume(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
static int opener_resume(struct platform_device *pdev)
{
	int ret = 0;

	/* opener device registration */
	ret = misc_register(&opener_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
	}
	d4_opener_set_dev_info(&pdev->dev);

	return ret;
}

static struct platform_driver opener_driver = {
	.probe  = opener_probe,
	.remove = opener_remove,
	.suspend = opener_suspend,
	.resume = opener_resume,
	.driver = {
		.name = OPENER_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

/**
 * @brief OPENER device register
 * @fn opener_register(void)
 * @param void
 * @return int
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
static int opener_register(void)
{
	platform_driver_register(&opener_driver);

	return 0;
}

/**
 * @brief OPENER device unregister
 * @fn opener_unregister(void)
 * @param void
 * @return void
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
static void opener_unregister(void)
{
	platform_driver_unregister(&opener_driver);
}

module_init(opener_register);
module_exit(opener_unregister);

MODULE_AUTHOR("Wooram Son <wooram.son@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 OPENER driver File");
MODULE_LICENSE("GPL");
