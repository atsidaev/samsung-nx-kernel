/**
 * @file d4_pp_3a.c
 * @brief DRIMe4 PP 3a Device Driver File
 * @author Kyounghwan Moon <kh.moon@samsung.com>,
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
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/slab.h>
#include <linux/completion.h>

#include <mach/pp/pp_3a/d4_pp_3a.h>
#include "d4_pp_3a_common_dd.h"
#include <mach/d4_cma.h>

extern const struct file_operations pp_3a_ops;

static struct miscdevice pp_3a_miscdev = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= PP_3A_MODULE_NAME,
	.fops		= &pp_3a_ops,
};

struct drime4_pp_3a *g_pp_3a;

/**
 * brief 3A device driver 등록 시 ioremap을 하는 함수
 * fn static int pp_3a_register_iomap(struct platform_device *pdev, struct drime4_pp_3a *pp_3a_ctx, int res_num)
 * param  struct platform_device *pdev, struct drime4_pp_3a *pp_3a_ctx, int res_num
 * return error state
 * author Kyounghwan Moon
 * note - Device Driver 등록 할 때 사용
 */
static int pp_3a_register_iomap(struct platform_device *pdev,
		struct drime4_pp_3a *pp_3a_ctx, int res_num)
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
	res = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!res) {
		dev_err(&pdev->dev, "failed to request io memory region\n");
		ret = -ENOMEM;
		goto out;
	}

	/* ioremap for register block */
	regs = ioremap(res->start, resource_size(res));
	if (!regs) {
		dev_err(&pdev->dev, "failed to remap io region\n");
		ret = -ENOMEM;
		goto err_no_ioremap;
	}

	pp_3a_ctx->regs_base = regs;

	return 0;

err_no_ioremap:
	release_resource(res);
	kfree(res);
out:
	return ret;
}

/**
 * brief 3A device driver 등록 함수
 * fn static int __devinit pp_3a_probe(struct platform_device *pdev)
 * param  struct platform_device *pdev
 * return error state
 * author Kyounghwan Moon
 * note - Device Driver 등록 할 때 사용
 */
static int __devinit pp_3a_probe(struct platform_device *pdev)
{
	struct drime4_pp_3a *pp_3a_ctx = NULL;
	int ret = 0;

	pp_3a_ctx = kzalloc(sizeof(struct drime4_pp_3a), GFP_KERNEL);
	if (!pp_3a_ctx) {
		dev_err(&pdev->dev, "failed to allocate drime4_pp_3a.\n");
		return -EINVAL;
	}

	pp_3a_ctx->pd = (struct drime4_pp_3a_dev_data *)pdev->dev.platform_data;
	if (pp_3a_ctx->pd == NULL) {
		dev_err(&pdev->dev, "pp_3a platform_data is empty.\n");
		kfree(pp_3a_ctx);
		return -ENOSPC;
	}

	pp_3a_ctx->id = pdev->id;
	pp_3a_ctx->dev = &pdev->dev;
	pp_3a_ctx->name = pdev->name;

	/* register iomap resources */
	ret = pp_3a_register_iomap(pdev, pp_3a_ctx, 0); /* pp_3a register base 를 얻는다. */
	if (ret < 0)
		goto out;

	/* register irq resource */
	/* 인터럽트 처리를 위한 irq 번호를 얻는다. */
	pp_3a_ctx->irq = platform_get_irq(pdev, 0);
	if (pp_3a_ctx->irq < 0) {
		dev_err(&pdev->dev, "failed to get drime4_pp_3a irq\n");
		goto out;
	}

	/* pp_3a device registration */
	ret = misc_register(&pp_3a_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		goto out;
	}

	pp_3a_ctx->result_mem_base = d4_cma_alloc(&pdev->dev, SZ_1M);
	memset((unsigned int *)pp_3a_ctx->result_mem_base, 0, SZ_1M);

	platform_set_drvdata(pdev, pp_3a_ctx);

	pp_3a_ctx->dev = &pdev->dev;
	g_pp_3a = pp_3a_ctx;

	pp_3a_set_basic_info((unsigned int)pp_3a_ctx->regs_base, pp_3a_ctx->result_mem_base, pp_3a_ctx->irq);

	dev_dbg(&pdev->dev, "probe success.\n");

	return 0;

out:
	kfree(pp_3a_ctx);
	return ret;
}

/**
 * brief 3A device driver 등록 해제 함수
 * fn static int pp_3a_remove(struct platform_device *pdev)
 * param struct platform_device *pdev
 * return 0
 * author Kyounghwan Moon
 * note - Device Driver 등록 해제 할 때 사용
 */
static int pp_3a_remove(struct platform_device *pdev)
{
	struct resource *res;

	misc_deregister(&pp_3a_miscdev);
	iounmap(g_pp_3a->regs_base);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res)
		release_mem_region(res->start, resource_size(res));

	d4_cma_free(&pdev->dev, g_pp_3a->result_mem_base);
	kfree(g_pp_3a);
	return 0;
}

#ifdef CONFIG_PM
static int pp_3a_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int pp_3a_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define pp_3a_suspend NULL
#define pp_3a_resume  NULL
#endif

static struct platform_driver pp_3a_driver = {
	.probe		= pp_3a_probe,
	.remove		= pp_3a_remove,
	.suspend 		= pp_3a_suspend,
	.resume 		= pp_3a_resume,
	.driver		= {
		.name	= PP_3A_MODULE_NAME,
		.owner	= THIS_MODULE,
	},
};

/**
 * brief 3A device file 등록 함수
 * fn static int pp_3a_register(void)
 * param  없음
 * return 0
 * author Kyounghwan Moon
 * note - Device file 등록 할 때 사용
 */
static int pp_3a_register(void)
{
	platform_driver_register(&pp_3a_driver);
	return 0;
}

/**
 * brief 3A device file 등록 해제 함수
 * fn static void pp_3a_unregister(void)
 * param  없음
 * return 없음
 * author Kyounghwan Moon
 * note - Device file 등록 해제 할 때 사용
 */
static void pp_3a_unregister(void)
{
	platform_driver_unregister(&pp_3a_driver);
}

module_init(pp_3a_register);
module_exit(pp_3a_unregister);

MODULE_AUTHOR("Kyounghwan Moon <kh.moon@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 PP 3A driver File");
MODULE_LICENSE("GPL");
