/**
 * @file d4_sma.c
 * @brief DRIMe4 SMA
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

#include <mach/sma/d4_sma.h>
#include <mach/d4_cma.h>
#include "d4_sma_if.h"

extern const struct file_operations sma_ops;

static struct miscdevice sma_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = SMA_MODULE_NAME,
	.fops = &sma_ops,
};

/**
 * @brief SMA device probe
 * @fn sma_probe(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int __devinit sma_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct SMA_Buffer_Info info;

	/* sma device registration */
	ret = misc_register(&sma_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		return -EINVAL;
	}

	d4_sma_set_dev_info(&pdev->dev);

	info.size = CMA_REGION1_SMA_SIZE;

	ret = d4_sma_alloc_buf(&info);
	if (ret < 0)
	{
		misc_deregister(&sma_miscdev);
	}

	return ret;
}


/**
 * @brief SMA device remove
 * @fn sma_remove(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int sma_remove(struct platform_device *pdev)
{
	misc_deregister(&sma_miscdev);

	return 0;
}

/**
 * @brief SMA device suspend
 * @fn sma_suspend(struct platform_device *pdev, pm_message_t state)
 * @param struct platform_device *pdev, pm_message_t state
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int sma_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

/**
 * @brief SMA device resume
 * @fn sma_resume(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int sma_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver sma_driver = {
	.probe  = sma_probe,
	.remove = sma_remove,
	.suspend = sma_suspend,
	.resume = sma_resume,
	.driver = {
		.name = SMA_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

/**
 * @brief SMA device register
 * @fn sma_register(void)
 * @param void
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int sma_register(void)
{
	platform_driver_register(&sma_driver);

	return 0;
}

/**
 * @brief SMA device unregister
 * @fn sma_unregister(void)
 * @param void
 * @return void
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static void sma_unregister(void)
{
	platform_driver_unregister(&sma_driver);
}

module_init(sma_register);
module_exit(sma_unregister);

MODULE_AUTHOR("Junkwon Choi <junkwon.choi@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 SMA driver File");
MODULE_LICENSE("GPL");
