/**
 * @file d4_pp_core.c
 * @brief DRIMe4 PP Core Driver File
 * @author Sunghoon Kim <bluesay.kim@samsung.com>
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
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/memory.h>
#include <linux/slab.h>
#include <linux/completion.h>

#include <mach/pp/pp_core/d4_pp_core.h>
#include "d4_pp_core_dd.h"
#include <linux/io.h>
#include <linux/i2c.h>


#ifdef CONFIG_PMU_SELECT
#include <linux/d4-pmu.h>
#endif

#ifdef CONFIG_RMU_DEV
#include <linux/d4_rmu.h>
#endif

extern const struct file_operations pp_core_ops;
static struct miscdevice pp_core_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = PP_CORE_MODULE_NAME,
		.fops = &pp_core_ops,
};

struct drime4_pp_core *g_pp_core;
extern int pp_pmu_requeset(void);
extern void pp_pmu_clear(void);
/**< PP Core Base Register Address & IRQ Number Setting */
static struct pp_core_reg_ctrl_base_info pp_core_base_info;
static struct resource *d4_pp_core_mem[3];

static int d4_pp_core_register_iomap(struct platform_device *pdev,
		struct drime4_pp_core *pp_core_ctx, int res_num)
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
	d4_pp_core_mem[res_num] = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!d4_pp_core_mem[res_num]) {
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
		pp_core_base_info.common_reg_base 	= (unsigned int) regs;
		pp_core_base_info.pp_phys_base_addr = (unsigned int) res->start;
	} else if (res_num == 1) {
		pp_core_base_info.dma_reg_base = (unsigned int) regs;
	} else if (res_num == 2) {
		pp_core_base_info.ctrl_reg_base = (unsigned int) regs;
	}

	return 0;

	err_no_ioremap:
		release_resource(d4_pp_core_mem[res_num]);
		kfree(d4_pp_core_mem[res_num]);
	out:
		return ret;
}

static int __devinit d4_pp_core_probe(struct platform_device *pdev)
{
	struct drime4_pp_core *pp_core = NULL;
	int irq;
	int ret = 0;

	pp_core = kzalloc(sizeof(struct drime4_pp_core), GFP_KERNEL);
	if (!pp_core) {
		dev_err(&pdev->dev, "failed to allocate drime4_pp_core.\n");
		ret = -EINVAL;
		goto out;
	}

	pp_core->pd = (struct drime4_pp_core_dev_data *)pdev->dev.platform_data;
	if (pp_core->pd == NULL) {
		dev_err(&pdev->dev, "pp_core platform_data is empty.\n");
		ret = -ENOSPC;
		goto err_no_platform_data;
	}

	pp_core->id = pdev->id;
	pp_core->dev = &pdev->dev;
	pp_core->name = pdev->name;

	pp_core_base_info.dev_info = &pdev->dev;

	/**< PP Clock Enable */
#ifndef CONFIG_PMU_SELECT
	pp_core->clock = clk_get(&pdev->dev, "pp");
	if (pp_core->clock == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_no_platform_data;
	}
	clk_enable(pp_core->clock);

	clk_set_rate(pp_core->clock, 200000000);
#endif
	/* register iomap resources */
	ret = d4_pp_core_register_iomap(pdev, pp_core, 0); /* PP Common Register Base */
	if (ret < 0) {
		goto err_no_iomap_0;
	}

	ret = d4_pp_core_register_iomap(pdev, pp_core, 1); /* PP DMA Register Base */
	if (ret < 0) {
		goto err_no_iomap_1;
	}

	ret = d4_pp_core_register_iomap(pdev, pp_core, 2); /* PP Control Register Base */
	if (ret < 0) {
		goto err_no_iomap_2;
	}

	/* register irq resource */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "No IRQ resource!\n");
		goto err_no_iomap_2;
	}

	pp_core_base_info.irq_num = irq;

	/**< PP Core Base Register Address & IRQ Number Setting, Device Info */
	pp_core_set_reg_ctrl_base_info(&pp_core_base_info);

	/* pp core device registration */
	ret = misc_register(&pp_core_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		goto err_no_register;
	}
	dev_dbg(&pdev->dev, "device registered as /dev/%s \n", PP_CORE_MODULE_NAME);

	platform_set_drvdata(pdev, pp_core);

	pp_core->dev = &pdev->dev;
	g_pp_core = pp_core;

	dev_dbg(&pdev->dev, "probe success.\n");

	return 0;

	err_no_register:
		iounmap((void __iomem *)pp_core_base_info.ctrl_reg_base);
		release_resource(d4_pp_core_mem[2]);
		kfree(d4_pp_core_mem[2]);
	err_no_iomap_2:
		iounmap((void __iomem *)pp_core_base_info.dma_reg_base);
		release_resource(d4_pp_core_mem[1]);
		kfree(d4_pp_core_mem[1]);
	err_no_iomap_1:
		iounmap((void __iomem *)pp_core_base_info.common_reg_base);
		release_resource(d4_pp_core_mem[0]);
		kfree(d4_pp_core_mem[0]);
	err_no_iomap_0:

	err_no_platform_data:
		kfree(pp_core);

	out:
		return ret;
}

static int d4_pp_core_remove(struct platform_device *pdev)
{
	misc_deregister(&pp_core_miscdev);
	iounmap((void __iomem *)pp_core_base_info.common_reg_base);
	iounmap((void __iomem *)pp_core_base_info.ctrl_reg_base);
	iounmap((void __iomem *)pp_core_base_info.dma_reg_base);

	release_resource(d4_pp_core_mem[0]);
	release_resource(d4_pp_core_mem[1]);
	release_resource(d4_pp_core_mem[2]);
	kfree(d4_pp_core_mem[0]);
	kfree(d4_pp_core_mem[1]);
	kfree(d4_pp_core_mem[2]);

	/**< PP Clock Disable */
#ifndef CONFIG_PMU_SELECT
	clk_disable(g_pp_core->clock);
	clk_put(g_pp_core->clock);
#endif
	kfree(g_pp_core);

	return 0;
}

#ifdef CONFIG_PM
static int pp_core_suspend(struct platform_device *pdev, pm_message_t state)
{
#ifdef CONFIG_PMU_SELECT
	int reval;
	struct clk *clock;
	struct d4_rmu_device *rmu;
	d4_pmu_check(PMU_PP);
	reval = d4_pmu_check(PMU_PP);
	if (reval != 0) {

		reval = pp_pmu_requeset();
		if (reval)
			return -1;

		rmu = d4_rmu_request();
		if (rmu == NULL)
			return;
		d4_pmu_isoen_set(PMU_PP, PMU_CTRL_ON);
		d4_sw_isp_reset(rmu, RST_PP);
		clk_disable(g_pp_core->clock);
		clk_put(g_pp_core->clock);
		d4_pmu_scpre_set(PMU_PP, PMU_CTRL_ON);
		d4_pmu_scall_set(PMU_PP, PMU_CTRL_ON);
		d4_rmu_release(rmu);
	}
#endif
	return 0;
}

static int pp_core_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define pp_core_suspend NULL
#define pp_core_resume  NULL
#endif

static struct platform_driver pp_core_driver = {
		.probe  = d4_pp_core_probe,
		.remove = d4_pp_core_remove,
		.suspend = pp_core_suspend,
		.resume = pp_core_resume,
		.driver = {
				.name = PP_CORE_MODULE_NAME,
				.owner = THIS_MODULE,
		},
};

static int d4_pp_core_register(void)
{
	platform_driver_register(&pp_core_driver);
	return 0;
}

static void d4_pp_core_unregister(void)
{
	platform_driver_unregister(&pp_core_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(d4_pp_core_register);
#else
fast_dev_initcall(d4_pp_core_register);
#endif
module_exit(d4_pp_core_unregister);

MODULE_AUTHOR("SungHoon Kim <bluesay.kim@samsung.com>");
MODULE_AUTHOR("Haesu Gwon <haesu.gwon@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 PP Core driver File");
MODULE_LICENSE("GPL");
