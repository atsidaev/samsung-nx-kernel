/**
 * @file d4_pp_ssif.c
 * @brief DRIMe4 PP Sensor Interface Driver File
 * @author DeokEun Cho <de.cho@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/clk.h>
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
#include <linux/pinctrl/consumer.h>

#include <mach/pp/pp_ssif/d4_pp_ssif.h>
#include "d4_pp_ssif_ctrl_dd.h"

struct drime4_pp_ssif *g_pp_ssif;
static struct resource *d4_pp_ssif_mem[2];

/**< PP SSIF Base Register Address & IRQ Number Setting */
static struct pp_ssif_reg_ctrl_base_info pp_ssif_base_info;

static int pp_ssif_register_iomap(struct platform_device *pdev,
		struct drime4_pp_ssif *pp_ssif_ctx, int res_num)
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
	d4_pp_ssif_mem[res_num] = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!d4_pp_ssif_mem[res_num]) {
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

	if (res_num == 0) {
		pp_ssif_base_info.ctrl_reg_base = (unsigned int) regs;
	} else if (res_num == 1) {
		pp_ssif_base_info.slvds_reg_base = (unsigned int) regs;
	}

	return 0;

	err_no_ioremap:
		release_resource(d4_pp_ssif_mem[res_num]);
		kfree(d4_pp_ssif_mem[res_num]);

	out:
		return ret;
}

static int __devinit pp_ssif_probe(struct platform_device *pdev)
{
	struct drime4_pp_ssif *pp_ssif_ctx = NULL;
	int irq;
	int ret = 0;

	pp_ssif_ctx = kzalloc(sizeof(struct drime4_pp_ssif), GFP_KERNEL);
	if (!pp_ssif_ctx) {
		dev_err(&pdev->dev, "failed to allocate drime4_pp_ssif.\n");
		ret = -EINVAL;
		goto out;
	}

	pp_ssif_ctx->pd = (struct drime4_pp_ssif_dev_data *)pdev->dev.platform_data;
	if (pp_ssif_ctx->pd == NULL) {
		dev_err(&pdev->dev, "pp_ssif platform_data is empty.\n");
		ret = -ENOSPC;
		goto err_no_platform_data;
	}

	pp_ssif_ctx->id = pdev->id;
	pp_ssif_ctx->dev = &pdev->dev;
	pp_ssif_ctx->name = pdev->name;

	pp_ssif_base_info.dev_info = &pdev->dev;

	/**< SubLVDS pinmux setting */
	pp_ssif_ctx->pmx = devm_pinctrl_get(&pdev->dev);
	pp_ssif_ctx->pins_default = pinctrl_lookup_state(pp_ssif_ctx->pmx, PINCTRL_STATE_DEFAULT);
	pinctrl_select_state(pp_ssif_ctx->pmx, pp_ssif_ctx->pins_default);

	/* register iomap resources */
	ret = pp_ssif_register_iomap(pdev, pp_ssif_ctx, 0); /* pp_ssif register base 를 얻는다. */
	if (ret < 0) {
		goto err_no_iomap_0;
	}

	ret = pp_ssif_register_iomap(pdev, pp_ssif_ctx, 1); /* SubLVDS register base 를 얻는다. */
	if (ret < 0) {
		goto err_no_iomap_1;
	}

	/* register irq resource */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "No IRQ resource!\n");
		goto err_no_iomap_1;
	}

	pp_ssif_base_info.irq_num = irq;

	/**< PP SSIF Base Register Address & IRQ Number Setting, Device Info */
	pp_ssif_set_reg_ctrl_base_info(&pp_ssif_base_info);

	platform_set_drvdata(pdev, pp_ssif_ctx);

	pp_ssif_ctx->dev = &pdev->dev;
	g_pp_ssif = pp_ssif_ctx;

	dev_dbg(&pdev->dev, "probe success.\n");

	return 0;

	err_no_iomap_1:
		iounmap((void __iomem *)pp_ssif_base_info.ctrl_reg_base);
		release_resource(d4_pp_ssif_mem[0]);
		kfree(d4_pp_ssif_mem[0]);

	err_no_iomap_0:

	err_no_platform_data:
		kfree(pp_ssif_ctx);

	out:
		return ret;
}

static int pp_ssif_remove(struct platform_device *pdev)
{
	iounmap((void __iomem *)pp_ssif_base_info.ctrl_reg_base);

	release_resource(d4_pp_ssif_mem[0]);
	kfree(d4_pp_ssif_mem[0]);
	release_resource(d4_pp_ssif_mem[1]);
	kfree(d4_pp_ssif_mem[1]);

	kfree(g_pp_ssif);
	return 0;
}


static struct platform_driver pp_ssif_driver = {
	.probe		= pp_ssif_probe,
	.remove		= pp_ssif_remove,
	.driver		= {
		.name	= PP_SSIF_MODULE_NAME,
		.owner	= THIS_MODULE,
	},
};

static int pp_ssif_register(void)
{
	platform_driver_register(&pp_ssif_driver);
	return 0;
}

static void pp_ssif_unregister(void)
{
	platform_driver_unregister(&pp_ssif_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(pp_ssif_register);
#else
fast_dev_initcall(pp_ssif_register);
#endif
module_exit(pp_ssif_unregister);

MODULE_AUTHOR("DeokEun Cho <de.cho@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 PP Sensor Interface File");
MODULE_LICENSE("GPL");

