/**
 * @file d4_jxr.c
 * @brief DRIMe4 JPEG XR Driver File
 * @author JiinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/slab.h>

#include <mach/jxr/d4_jxr.h>
#include "d4_jxr_ctrl_dd.h"

extern const struct file_operations jxr_ops;

static struct miscdevice jxr_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = JXR_MODULE_NAME,
		.fops = &jxr_ops,
};

static struct jxr_reg_ctrl_base_info jxr_base_info;
static struct resource *d4_jxr_mem;
struct drime4_jxr *jxr_main_ctx;

/**
 * @brief JPEG_XR device resource register
 * @fn jxr_register_iomap(struct platform_device *pdev, struct drime4_jxr *jxr_ctx, int res_num)
 * @param struct platform_device *pdev, struct drime4_jxr *jxr_ctx, int res_num
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jxr_register_iomap(struct platform_device *pdev,
		struct drime4_jxr *jxr_core_ctx, int res_num)
{
	struct resource *res = NULL;
	void __iomem *regs;
	int ret = 0;

	/* get resource for io memory  */
	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region\n");
		ret = -ENODEV;
		goto out;
	}

	/* request mem region  */
	d4_jxr_mem = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!d4_jxr_mem) {
		dev_err(&pdev->dev, "failed to request io memory region\n");
		ret = -ENOMEM;
		goto out;
	}

	/* ioremap for register block */
	regs = ioremap(res->start, resource_size(res));
	if (!regs) {
		dev_err(&pdev->dev, "failed to remap io region\n");
		ret = -ENOMEM;
		goto ERROR_IOREMAP;
	}

	jxr_base_info.op_reg_base 			= (unsigned int)regs;
	jxr_base_info.physCtrlReg.startAddr	= (unsigned int)res->start;
	jxr_base_info.physCtrlReg.size		= (unsigned int)resource_size(res);

	return 0;

	ERROR_IOREMAP:
		release_resource(d4_jxr_mem);
		kfree(d4_jxr_mem);
	out:
	return ret;
}

/**
 * @brief JPEG-XR device resource release
 * @fn jxr_release_iomap(struct platform_device *pdev, struct drime4_jxr *jxr_ctx, int res_num)
 * @param struct platform_device *pdev, struct drime4_jxr *jxr_ctx, int res_num
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jxr_release_iomap(struct platform_device *pdev,
		struct drime4_jxr *jxr_ctx, int res_num)
{
	struct resource *res = NULL;
	int ret = 0;

	/* get resource for io memory  */
	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region\n");
		ret = -ENODEV;
		goto out;
	}

	/* release mem region  */
	release_mem_region(res->start, resource_size(res));
	/* dev_err(&pdev->dev, "release io memory region\n"); */

	out:
	return ret;
}


/**
 * @brief JPEG_XR device probe
 * @fn jxr_probe(struct platform_device *pdev)
 * @param struct platform_device *
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int __devinit d4_jxr_probe(struct platform_device *pdev)
{
	struct drime4_jxr *jxr_ctx = NULL;
	int irq = 0;
	int ret = 0;

	jxr_ctx = kzalloc(sizeof(struct drime4_jxr), GFP_KERNEL);
	if (!jxr_ctx) {
		dev_err(&pdev->dev, "failed to allocate drime4_jxr.\n");
		ret = -EINVAL;
		goto out;
	}

	jxr_ctx->pd = (struct drime4_jxr_dev_data *)pdev->dev.platform_data;
	if (jxr_ctx->pd == NULL) {
		dev_err(&pdev->dev, "jxr platform_data is empty.\n");
		ret = -ENOSPC;
		goto ERROR_PLATFORM_DATA;
	}

	jxr_ctx->id = pdev->id;
	jxr_ctx->dev = &pdev->dev;
	jxr_ctx->name = pdev->name;

	jxr_base_info.dev_info = &pdev->dev;

	/* register iomap resources */
	ret = d4_jxr_register_iomap(pdev, jxr_ctx, 0); /* jxr Op Register Base */
	if (ret < 0)
	   goto ERROR_IOMAP;

	/* register irq resource */
	irq = platform_get_irq(pdev, 0);
	jxr_base_info.irq_num = irq;

	/**< jxr Core Base Register Address & IRQ Number Setting */
	jxr_set_reg_ctrl_base_info(&jxr_base_info);

	/* JXR device registration */
	ret = misc_register(&jxr_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		goto ERROR_MISC_REGISTER;
	}
	dev_dbg(&pdev->dev, "device registered as /dev/%s \n", JXR_MODULE_NAME);

	platform_set_drvdata(pdev, jxr_ctx);

	jxr_ctx->dev = &pdev->dev;
	jxr_main_ctx = jxr_ctx;

	dev_dbg(&pdev->dev, "probe success.\n");

	return 0;

	ERROR_MISC_REGISTER:
		iounmap((void __iomem *)jxr_base_info.op_reg_base);
		release_resource(d4_jxr_mem);
		kfree(d4_jxr_mem);

	ERROR_IOMAP:
	ERROR_PLATFORM_DATA:
	kfree(jxr_ctx);
	out:
	return ret;
}


/**
 * @brief JPEG_XR device remove
 * @fn jxr_remove(struct platform_device *pdev)
 * @param struct platform_device *
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jxr_remove(struct platform_device *pdev)
{
	misc_deregister(&jxr_miscdev);

	iounmap((void __iomem *)jxr_base_info.op_reg_base);
	release_resource(d4_jxr_mem);
	kfree(d4_jxr_mem);

	kfree(jxr_main_ctx);

	return 0;
}

/**
 * @brief JPEG-XR device suspend
 * @fn jxr_suspend(struct platform_device *pdev, pm_message_t state)
 * @param struct platform_device *pdev, pm_message_t state
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jxr_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret = 0;
	printk("\n\n [ jpeg suspend start ]\n\n");
	misc_deregister(&jxr_miscdev);

	iounmap((void __iomem *)jxr_base_info.op_reg_base);

	ret = d4_jxr_release_iomap(pdev, jxr_main_ctx, 0);
	if (ret < 0)
		goto out;

	printk("\n\n [ jpeg suspend end ]\n\n");
	kfree(jxr_main_ctx);
	return ret;

	out:
	printk(KERN_DEBUG "jpeg_release_iomap fail : %d\n", ret);
	kfree(jxr_main_ctx);
	return ret;
}

/**
 * @brief JPEG device resume
 * @fn jpeg_resume(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jxr_resume(struct platform_device *pdev)
{
	struct drime4_jxr *jxr_ctx = NULL;
	int ret = 0;

	printk("\n\n [ jxr resume start ]\n\n");
	jxr_ctx = kzalloc(sizeof(struct drime4_jxr), GFP_KERNEL);
	if (!jxr_ctx) {
		dev_err(&pdev->dev, "failed to allocate drime4_jxr.\n");
		ret = -EINVAL;
		goto out;
	}

	jxr_ctx->pd = (struct drime4_jxr_dev_data *)pdev->dev.platform_data;
	if (jxr_ctx->pd == NULL) {
		dev_err(&pdev->dev, "jpeg-xr platform_data is empty.\n");
		ret = -ENOSPC;
		goto ERR_PLATFORM_DATA;
	}

	jxr_ctx->id = pdev->id;
	jxr_ctx->dev = &pdev->dev;
	jxr_ctx->name = pdev->name;

	jxr_base_info.dev_info = &pdev->dev;

	/* register iomap resources */
	ret = d4_jxr_register_iomap(pdev, jxr_ctx, 0); /* jxr Op Register Base */
	if (ret < 0)
		goto ERR_IOREMAP;

	/* register irq resource */
	jxr_base_info.irq_num = platform_get_irq(pdev, 0);

	/**< JPEG-XR Base Register Address & IRQ Number Setting */
	jxr_set_reg_ctrl_base_info(&jxr_base_info);

	/* JPEG-XR device registration */
	ret = misc_register(&jxr_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		goto ERR_MISC_REGISTER;
	}
	dev_err(&pdev->dev, "device registered as /dev/%s \n", JXR_MODULE_NAME);

	platform_set_drvdata(pdev, jxr_ctx);

	jxr_ctx->dev = &pdev->dev;
	jxr_main_ctx = jxr_ctx;

	dev_err(&pdev->dev, "probe success.\n");
	printk("\n\n [ JPEG-XR resume end ]\n\n");

	return 0;

	ERR_MISC_REGISTER:
		iounmap((void __iomem *)jxr_base_info.op_reg_base);
		release_resource(d4_jxr_mem);
		kfree(d4_jxr_mem);

	ERR_IOREMAP:
	ERR_PLATFORM_DATA:
		kfree(jxr_ctx);

	out:
	return ret;
}


static struct platform_driver jxr_driver = {
		.probe  = d4_jxr_probe,
		.remove = d4_jxr_remove,
		.suspend = d4_jxr_suspend,
		.resume = d4_jxr_resume,
		.driver = {
		.name = JXR_MODULE_NAME,
		.owner = THIS_MODULE,
		},
};

/**
 * @brief JPEG_XR device register
 * @fn jxr_register(void)
 * @param void
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int jxr_register(void)
{
	platform_driver_register(&jxr_driver);

	return 0;
}

/**
 * @brief JPEG_XR device un-register
 * @fn jxr_unregister(void)
 * @param void
 * @return void
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static void jxr_unregister(void)
{
	platform_driver_unregister(&jxr_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(jxr_register);
#else
fast_dev_initcall(jxr_register);
#endif
module_exit(jxr_unregister);

MODULE_AUTHOR("JinHyoung An <jh0913.an@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 jxr driver File");
MODULE_LICENSE("GPL");
