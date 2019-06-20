/**
 * @file	d4_ipcs.c
 * @brief	IPCS driver file for Samsung DRIMeIV Camera Interface
 * 			driver
 *
 * @author	Dongjin Jung <djin81.jung@samsung.com>
 * 			Jangwon Lee <jang_won.lee@samsung.com>
 *
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
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
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/slab.h>

#ifdef CONFIG_PMU_SELECT
#include <media/drime4/opener/d4_opener_ioctl.h>
#endif


#ifdef CONFIG_PMU_SELECT
#include <linux/d4-pmu.h>
#endif

#ifdef CONFIG_RMU_DEV
#include <linux/d4_rmu.h>
#endif

#include <mach/ipcs/d4_ipcs.h>
#include "d4_ipcs_ctrl.h"

extern int ipcs_pmu_requeset(void);
extern void ipcs_pmu_clear(void);
extern const struct file_operations drime4_ipcs_fops;

struct drime4_ipcs *g_ipcs;

static struct miscdevice ipcs_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = IPCS_MODULE_NAME,
		.fops = &drime4_ipcs_fops,
};

static struct ipcs_k_reg_ctrl_base_info ipcs_base_info;

static int ipcs_k_register_iomap(struct platform_device *pdev,
		struct drime4_ipcs *ipcs_ctx, int res_num)
{
	struct resource *res = NULL;
	void __iomem *regs;
	int ret = 0;

	/* get resource for io memory  */
	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region\n");
		ret = -ENOMEM;
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

	ipcs_base_info.reg_base = (unsigned int) regs;
	ipcs_base_info.phys_reg_info.reg_start_addr = (unsigned int) res->start;
	ipcs_base_info.phys_reg_info.reg_size = (unsigned int) resource_size(res);

	return 0;

err_no_ioremap:
	release_mem_region(res->start, resource_size(res));

out:
	return ret;
}

static int _ipcs_resume(struct platform_device *pdev, int is_probe)
{
	struct drime4_ipcs *ipcs_ctx = NULL;
	int irq;
	int ret = 0;

	ipcs_ctx = kzalloc(sizeof(struct drime4_ipcs), GFP_KERNEL);
	if (!ipcs_ctx) {
		dev_err(&pdev->dev, "failed to allocate drime4_ipcs.\n");
		/*kfree(ipcs_ctx);*/
		return -EINVAL;
	}

	ipcs_ctx->id = pdev->id;
	ipcs_ctx->dev = &pdev->dev;
	ipcs_ctx->name = pdev->name;

	ipcs_base_info.dev_info = &pdev->dev;

	if (is_probe == 1) {
		/* Probe */
#ifndef CONFIG_PMU_SELECT
		ipcs_ctx->clock = clk_get(&pdev->dev, "ipcs");
		if (ipcs_ctx->clock == -2) {
			ret = -EINVAL;
			goto out;
		}
		clk_enable(ipcs_ctx->clock);
		/* IPCS Clock freq. */
		clk_set_rate(ipcs_ctx->clock, 260000000);
#endif
	} else {
		/* Resume */
#ifdef CONFIG_PMU_SELECT
		if (d4_get_kdd_open_count(KDD_IPCS) > 0) {
			ipcs_ctx->clock = clk_get(&pdev->dev, "ipcs");
			if (ipcs_ctx->clock == -2) {
				ret = -EINVAL;
				goto out;
			}			clk_enable(ipcs_ctx->clock);
			/* IPCS Clock freq. */
			clk_set_rate(ipcs_ctx->clock, 260000000);
		}
#endif
	}

	/* register iomap resources */
	ret = ipcs_k_register_iomap(pdev, ipcs_ctx, 0); /* ipcs register base 를 얻는다. */
	if (ret < 0)
	goto out;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "No IRQ resource!\n");
		ret = -EINVAL;
		goto out;
	}
	ipcs_base_info.irq_num = irq;

	/*ipcs base register address set & IRQ number set */
	ipcs_k_set_dev_info(&ipcs_base_info);

	if (is_probe) {
		/* ipcs device registration */
		ret = misc_register(&ipcs_miscdev);
		if (ret < 0) {
			dev_err(&pdev->dev, "Failed to register misc driver.\n");
			goto out;
		}
	}
	dev_dbg(&pdev->dev, "[DRIMe4 IPCS] device registered as /dev/%s\n",
			IPCS_MODULE_NAME);

	platform_set_drvdata(pdev, ipcs_ctx);

	ipcs_ctx->dev = &pdev->dev;

	/* Request IRQ */
/*
	ret = ipcs_k_interrupt_init();
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to request irq.\n");
		goto out;
	}
*/
	g_ipcs = ipcs_ctx;

	dev_dbg(&pdev->dev, "[DRIMe4 IPCS] probe/resume success.\n");
	return 0;

out:
	kfree(ipcs_ctx);
	return ret;
}

static int _ipcs_suspend(struct platform_device *pdev, int clk_di)
{
	int ret = 0;
	struct resource *res = NULL;

	/* Free IRQ */
	/*ipcs_k_interrupt_free();*/

#ifdef CONFIG_PMU_SELECT
	int reval;
	struct d4_rmu_device *rmu;
	reval = d4_pmu_check(PMU_IPCS);
	if (reval != 0) {
		reval = ipcs_pmu_requeset();
		if (reval)
			return -1;

		rmu = d4_rmu_request();
		if (rmu == NULL)
			goto out;

		d4_pmu_isoen_set(PMU_IPCS, PMU_CTRL_ON);
		d4_sw_isp_reset(rmu, RST_IPCS);
		clk_disable(g_ipcs->clock);
		clk_put(g_ipcs->clock);

		d4_pmu_scpre_set(PMU_IPCS, PMU_CTRL_ON);
		d4_pmu_scall_set(PMU_IPCS, PMU_CTRL_ON);
		d4_rmu_release(rmu);
	}
#endif

	if (clk_di) {
		misc_deregister(&ipcs_miscdev);
	}
	
	iounmap((void __iomem *)ipcs_base_info.reg_base);

#ifndef CONFIG_PMU_SELECT
	/* IPCS Clock Disable */
	if (clk_di != 0) {
		clk_disable(g_ipcs->clock);
		clk_put(g_ipcs->clock);
	}
#endif

	/* get resource for io memory  */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region\n");
		ret = -ENODEV;
		goto out;
	}

	/* release mem region  */
	release_mem_region(res->start, resource_size(res));
	/* dev_dbg(&pdev->dev, "release io memory region\n"); */

	kfree(g_ipcs);
	dev_dbg(&pdev->dev, "[DRIMe4 IPCS] remove/suspend success.\n");
	return 0;

out:
	kfree(g_ipcs);
	dev_err(&pdev->dev, "[DRIMe4 IPCS] remove/suspend fail.\n");
	return ret;
}

static int __devinit ipcs_probe(struct platform_device *pdev)
{
	return _ipcs_resume(pdev, 1);
}

static int ipcs_remove(struct platform_device *pdev)
{
	return _ipcs_suspend(pdev, 1);
}

static int ipcs_suspend(struct platform_device *pdev, pm_message_t state)
{
	return _ipcs_suspend(pdev, 0);
}

static int ipcs_resume(struct platform_device *pdev)
{
	return _ipcs_resume(pdev, 0);
}

static struct platform_driver ipcs_driver = {
		.probe = ipcs_probe,
		.remove = ipcs_remove,
		.suspend = ipcs_suspend,
		.resume = ipcs_resume,
		.driver = {
			.name = IPCS_MODULE_NAME,
			.owner = THIS_MODULE,
		},
};

static int ipcs_register(void)
{
	platform_driver_register(&ipcs_driver);
	return 0;
}

static void ipcs_unregister(void)
{
	platform_driver_unregister(&ipcs_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(ipcs_register);
#else
fast_dev_initcall(ipcs_register);
#endif
module_exit(ipcs_unregister);

MODULE_AUTHOR("Dongjin Jung <djin81.jung@samsung.com>, Jangwon Lee <jang_won.lee@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 IPCS driver using ioctl");
MODULE_LICENSE("GPL");
