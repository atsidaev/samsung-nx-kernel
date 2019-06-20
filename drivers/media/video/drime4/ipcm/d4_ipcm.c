/**
 * @file d4_ipcm.c
 * @brief DRIMe4 IPCM Driver File
 * @author TaeWook Nam <tw.@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#define DEBUG


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
/*#include <linux/cma.h>*/
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


#include <mach/ipcm/d4_ipcm.h>
#include "d4_ipcm_ctrl.h"

/*#define IPCM_LDCM_DEF*/

extern int ipcm_pmu_requeset(void);
extern void ipcm_pmu_clear(void);
extern const struct file_operations drime4_ipcm_fops;

static struct miscdevice ipcm_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = IPCM_MODULE_NAME,
	.fops = &drime4_ipcm_fops,
};

struct drime4_ipcm *g_ipcm;

static struct ipcm_k_reg_ctrl_base_info ipcm_base_info;

static int ipcm_register_iomap(struct platform_device *pdev,
		struct drime4_ipcm *ipcm_ctx, int res_num)
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

	ipcm_base_info.reg_base = (unsigned int)regs;
	ipcm_base_info.phys_reg_info.reg_start_addr = (unsigned int) res->start;
	ipcm_base_info.phys_reg_info.reg_size = (unsigned int) resource_size(res);

	return 0;

err_no_ioremap:
	release_mem_region(res->start, resource_size(res));
out:
	return ret;

}

static int _ipcm_resume(struct platform_device *pdev, int is_probe)
{
	struct drime4_ipcm *ipcm_ctx = NULL;
	int ret = 0;
	int ipcm_irq_num;
	int ipcm_md_irq_num;
	int ipcm_ldcm_irq_num;

	ipcm_ctx = kzalloc(sizeof(struct drime4_ipcm), GFP_KERNEL);
	if (!ipcm_ctx) {
		dev_err(&pdev->dev, "failed to allocate drime4_ipcm.\n");
		return -EINVAL;
	}

	ipcm_ctx->id = pdev->id;
	ipcm_ctx->dev = &pdev->dev;
	ipcm_ctx->name = pdev->name;

	ipcm_base_info.dev_info = &pdev->dev;

	if (is_probe == 1) {
		/* Probe */
#ifndef CONFIG_PMU_SELECT
		ipcm_ctx->clock = clk_get(&pdev->dev, "ipcm");
		if (ipcm_ctx->clock == -2) {
			ret = -EINVAL;
			goto out;
		}
		clk_enable(ipcm_ctx->clock);
		clk_set_rate(ipcm_ctx->clock, 200000000);
#endif
	} else {
		/* Resume */
#ifdef CONFIG_PMU_SELECT
		if (d4_get_kdd_open_count(KDD_IPCM) > 0) {
			ipcm_ctx->clock = clk_get(&pdev->dev, "ipcm");
			if (ipcm_ctx->clock == -2) {
				ret = -EINVAL;
				goto out;
			}
			clk_enable(ipcm_ctx->clock);
			clk_set_rate(ipcm_ctx->clock, 200000000);
		}
#endif
	}
	spin_lock_init(&ipcm_ctx->irqlock);

	/* register iomap resources */
	ret = ipcm_register_iomap(pdev, ipcm_ctx, 0); /* get ipcm register base */
	if (ret < 0)
		goto out;

	ipcm_irq_num = platform_get_irq(pdev, 0);
	if (ipcm_irq_num < 0) {
		 dev_err(&pdev->dev, "No IRQ resource!\n");
		 ret = -EINVAL;
		 goto out;
	}
	ipcm_md_irq_num = platform_get_irq(pdev, 1);
	if (ipcm_md_irq_num < 0) {
		 dev_err(&pdev->dev, "No IRQ resource!\n");
		 ret = -EINVAL;
		 goto out;
	}
	ipcm_ldcm_irq_num = platform_get_irq(pdev, 2);
	if (ipcm_ldcm_irq_num < 0) {
		 dev_err(&pdev->dev, "No IRQ resource!\n");
		 ret = -EINVAL;
		 goto out;
	}

	ipcm_base_info.dma_irq_num = ipcm_irq_num;
	ipcm_base_info.md_irq_num = ipcm_md_irq_num;
	ipcm_base_info.ldcm_irq_num = ipcm_ldcm_irq_num;


	/* set ipcm device info to global structure for internal functions. */
	ipcm_k_set_dev_info(&ipcm_base_info);

	if (is_probe) {
		/* ipcm device registration */
		ret = misc_register(&ipcm_miscdev);
		if (ret < 0) {
			dev_err(&pdev->dev, "Failed to register misc driver.\n");
			goto out;
		}
	}
	
	dev_dbg(&pdev->dev, "[DRIMe4 IPCM] device registered as /dev/%s\n",
			IPCM_MODULE_NAME);

	platform_set_drvdata(pdev, ipcm_ctx);

	ipcm_ctx->dev = &pdev->dev;
	g_ipcm = ipcm_ctx;

	dev_dbg(&pdev->dev, "[DRIMe4 IPCM] probe/resume success.\n");

	return 0;

out:
	kfree(ipcm_ctx);
	return ret;
}

static int _ipcm_suspend(struct platform_device *pdev, int clk_di)
{
	struct resource *res = NULL;
#ifdef CONFIG_PMU_SELECT
	int reval;
	struct d4_rmu_device *rmu;
	reval = d4_pmu_check(PMU_IPCM);
	if (reval != 0) {
		reval = ipcm_pmu_requeset();
		if (reval)
			return;

		rmu = d4_rmu_request();
		if (rmu == NULL)
			return;
		d4_pmu_isoen_set(PMU_IPCM, PMU_CTRL_ON);
		d4_sw_isp_reset(rmu, RST_IPCM);
		clk_disable(g_ipcm->clock);
		clk_put(g_ipcm->clock);
		d4_pmu_scpre_set(PMU_IPCM, PMU_CTRL_ON);
		d4_pmu_scall_set(PMU_IPCM, PMU_CTRL_ON);
		d4_rmu_release(rmu);
	}
#endif
	if (clk_di) {
		misc_deregister(&ipcm_miscdev);
	}
	iounmap((void __iomem *)ipcm_base_info.reg_base);

	free_irq(ipcm_base_info.dma_irq_num, NULL);
	free_irq(ipcm_base_info.md_irq_num, NULL);
	free_irq(ipcm_base_info.ldcm_irq_num, NULL);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res)
		release_mem_region(res->start, resource_size(res));

	/**< IPCM Clock Disable */
#ifndef CONFIG_PMU_SELECT
	if (clk_di != 0) {
		clk_disable(g_ipcm->clock);
		clk_put(g_ipcm->clock);
	}
#endif
	kfree(g_ipcm);
	dev_dbg(&pdev->dev, "[DRIMe4 IPCM] remove/suspend success.\n");

	return 0;
}
static int __devinit ipcm_probe(struct platform_device *pdev)
{
	return _ipcm_resume(pdev, 1);
}

static int ipcm_remove(struct platform_device *pdev)
{
	return _ipcm_suspend(pdev, 1);
}

static int ipcm_suspend(struct platform_device *pdev, pm_message_t state)
{
	return _ipcm_suspend(pdev, 0);
}

static int ipcm_resume(struct platform_device *pdev)
{
	return _ipcm_resume(pdev, 0);
}

static struct platform_driver ipcm_driver = {
		.probe = ipcm_probe,
		.remove = ipcm_remove,
		.suspend = ipcm_suspend,
		.resume = ipcm_resume,
		.driver = {
			.name = IPCM_MODULE_NAME,
			.owner = THIS_MODULE,
		},
};

static int ipcm_register(void)
{
	platform_driver_register(&ipcm_driver);
	return 0;
}

static void ipcm_unregister(void)
{
	platform_driver_unregister(&ipcm_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(ipcm_register);
#else
fast_dev_initcall(ipcm_register);
#endif
module_exit(ipcm_unregister);

MODULE_AUTHOR("TaeWook Nam <tw.nam@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 IPCM driver using ioctl");
MODULE_LICENSE("GPL");
