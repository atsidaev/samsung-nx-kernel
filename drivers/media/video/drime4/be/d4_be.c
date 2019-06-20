/**
 * @file d4_be.c
 * @brief DRIMe4 Bayer Platform Driver
 * @author Niladri Mukherjee <n.mukherjee@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*====================================
 * SYSTEM INCLUDES
 ====================================*/
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/slab.h>
#include <linux/clk.h>

#ifdef CONFIG_PMU_SELECT
#include <media/drime4/opener/d4_opener_ioctl.h>
#endif


#ifdef CONFIG_PMU_SELECT
#include <linux/d4-pmu.h>
#endif

#ifdef CONFIG_RMU_DEV
#include <linux/d4_rmu.h>
#endif
/*====================================
 * USER INCLUDES
 ====================================*/
#include "d4_be_top.h"
/*====================================
 * GLOBALS
 ====================================*/



extern const struct file_operations drime4_be_ops;
struct drime4_be *g_be;
static struct be_reg_ctrl_base_info be_base_info;

static struct miscdevice drime4_be_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = BE_MODULE_NAME,
		.fops = &drime4_be_ops, /*defined in d4_be_ioctl.c*/
};

/**
 * @brief set resigter address
 * @fn   be_set_reg_ctrl_base_info(struct be_reg_ctrl_base_info *info)
 * @param struct be_reg_ctrl_base_info *info
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note  -
 */
static int drime4_bayer_register_iomap(struct platform_device *pdev,
		struct drime4_be *be, int res_num)
{
	int ret = 0;
	struct resource *res = NULL;
	void __iomem *regs;
	/*get resource for io memory*/
	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (!res) {
		dev_dbg(&pdev->dev,
				"[D4/BAYER]FAILED TO GET RESOURCE MEMORY\n");
		ret = -ENODEV;
		goto out;
	}

	/*request mem region*/
	res = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!res) {
		dev_vdbg(&pdev->dev,
				"[D4/BAYER]FAILED TO GET IO MEMORY\n");
		ret = -ENOMEM;
		goto out;
	}

	/*ioremap for register block*/
	regs = ioremap(res->start, resource_size(res));
	if (!regs) {
		dev_vdbg(&pdev->dev, "[D4/BAYER]FAILED TO GET IO-REMAP RESOURCE\n");
		printk("[D4/BAYER]FAILED TO GET IO-REMAP RESOURCE\n");
		ret = -ENOMEM;
		goto err_no_ioremap;
	}

	if (res_num == 0) {
		/*5008-0000*/
		be->reg_be_top = regs;
		be_base_info.top_reg_base = (unsigned int) regs;
		be_base_info.phys_top_reg_info.reg_start_addr = (unsigned int) res->start;
		be_base_info.phys_top_reg_info.reg_size = (unsigned int) resource_size(res);
	} else if (res_num == 1) {
		/*5008-1000*/
		be->reg_be_ghost = regs;
		be_base_info.ghost_reg_base = (unsigned int) regs;
		be_base_info.phys_ghost_reg_info.reg_start_addr	= (unsigned int) res->start;
		be_base_info.phys_ghost_reg_info.reg_size = (unsigned int) resource_size(res);
	} else if (res_num == 2) {
		/*5008-2000*/
		be->reg_be_snr = regs;
		be_base_info.snr_reg_base = (unsigned int) regs;
		be_base_info.phys_snr_reg_info.reg_start_addr= (unsigned int) res->start;
		be_base_info.phys_snr_reg_info.reg_size = (unsigned int) resource_size(res);
	} else if (res_num == 3) {
		/*5008-3000*/
		be->reg_be_sg = regs;
		be_base_info.sg_reg_base = (unsigned int) regs;
		be_base_info.phys_sg_reg_info.reg_start_addr= (unsigned int) res->start;
		be_base_info.phys_sg_reg_info.reg_size = (unsigned int) resource_size(res);
	} else if (res_num == 4) {
		/*5008-4000*/
		be->reg_be_blend = regs;
		be_base_info.blend_reg_base = (unsigned int) regs;
		be_base_info.phys_blend_reg_info.reg_start_addr	= (unsigned int) res->start;
		be_base_info.phys_blend_reg_info.reg_size = (unsigned int) resource_size(res);
	} else if (res_num == 5) {
		/*5008-5000*/
		be->reg_be_fme = regs;
		be_base_info.fme_reg_base = (unsigned int) regs;
		be_base_info.phys_fme_reg_info.reg_start_addr= (unsigned int) res->start;
		be_base_info.phys_fme_reg_info.reg_size = (unsigned int) resource_size(res);
	} else if (res_num == 6) {
		/*5008-6000*/
		be->reg_be_3dme = regs;
		be_base_info.cme_reg_base = (unsigned int) regs;
		be_base_info.phys_3dme_reg_info.reg_start_addr	= (unsigned int) res->start;
		be_base_info.phys_3dme_reg_info.reg_size = (unsigned int) resource_size(res);
	} else if (res_num == 7) {
		/*5008-7000*/
		be->reg_be_dma = regs;
		be_base_info.dma_reg_base = (unsigned int) regs;
		be_base_info.phys_dma_reg_info.reg_start_addr = (unsigned int) res->start;
		be_base_info.phys_dma_reg_info.reg_size = (unsigned int) resource_size(res);
	}
	err_no_ioremap:
		release_mem_region(res->start, resource_size(res));
	out:
		return ret;
}

static int _drime4_bayer_resume(struct platform_device *pdev, int is_probe)
{
	int ret = 0;
	struct drime4_be *d4_be = NULL;
	int i, core_irq;
	d4_be = kzalloc(sizeof(struct drime4_be), GFP_KERNEL);
	if (!d4_be) {
		dev_dbg(&pdev->dev, "[D4/BE]FAILED TO ALLOCATE BAYER.\n");
		return -EINVAL;
	}

	d4_be->id = pdev->id;
	d4_be->dev = &pdev->dev;
	d4_be->name = pdev->name;

	if (is_probe == 1) {
		/**< Probe */
#ifndef CONFIG_PMU_SELECT
		/**< BE Clock Enable */
		d4_be->clock = clk_get(&pdev->dev, "be");

		if (d4_be->clock == -2) {
			ret = -EINVAL;
			kfree(d4_be);
			goto out;
		}

		clk_enable(d4_be->clock);
		clk_set_rate(d4_be->clock, 200000000);
#endif
	} else {
		/**< Resume */
#ifdef CONFIG_PMU_SELECT
		if (d4_get_kdd_open_count(KDD_BE) > 0) {
			d4_be->clock = clk_get(&pdev->dev, "be");
			if (d4_be->clock == -2) {
				ret = -EINVAL;
				kfree(d4_be);
				goto out;
			}
			clk_enable(d4_be->clock);
			clk_set_rate(d4_be->clock, 200000000);
		}
#endif
	}
	be_base_info.dev_info = &pdev->dev;

	/* register iomap resources */
	for (i = 0; i < 8; i++) {
		ret = drime4_bayer_register_iomap(pdev, d4_be, i);
		if (ret < 0) {
			kfree(d4_be);
			goto out;
		}
	}

	/* register irq resource */
	core_irq = platform_get_irq(pdev, 0);
	if(core_irq<0) {
		ret = core_irq;
		kfree(d4_be);
		printk("no provided irq!\n");
		goto out;
	}
	d4_be->be_irq = core_irq;
	be_base_info.irq_num = core_irq;

	/**< BE Core Base Register Address & IRQ Number Setting - 4.4 yunmi */
	be_set_reg_ctrl_base_info(&be_base_info);

	/*Bayer device registration*/
	ret = misc_register(&drime4_be_miscdev);
	if (ret < 0) {
		dev_vdbg(&pdev->dev, "[D4/BAYER]Misc Driver Regiter FAILED!!\n");
		printk("[D4/BAYER]Misc Driver Regiter FAILED!!\n");
		kfree(d4_be);
		goto out;
	}
	dev_dbg(&pdev->dev, "[D4/BAYER]..DEVICE REGISTERED AS /dev/%s\n", BE_MODULE_NAME);

	platform_set_drvdata(pdev, d4_be);
	d4_be->dev = &pdev->dev;
	g_be = d4_be;

	dev_dbg(&pdev->dev, "[D4/BAYER]..PROBE SUCCESS....\n");
	printk("[D4/BAYER]..PROBE SUCCESS..Registered as /dev/%s\n", BE_MODULE_NAME);
	out:
  	return ret;
}

static int _drime4_bayer_suspend(struct platform_device *pdev, int clk_di)
{
#ifdef CONFIG_PMU_SELECT
	int reval;
	struct d4_rmu_device *rmu;

	reval = d4_pmu_check(PMU_BAYER);
	if (reval != 0) {
		reval = be_pmu_requeset();
		if (reval)
			return -1;

		rmu = d4_rmu_request();
		if (rmu == NULL)
			return -1;

		d4_pmu_isoen_set(PMU_BAYER, PMU_CTRL_ON);
		d4_sw_isp_reset(rmu, RST_BE);

		clk_disable(g_be->clock);
		clk_put(g_be->clock);

		d4_pmu_scpre_set(PMU_BAYER, PMU_CTRL_ON);
		d4_pmu_scall_set(PMU_BAYER, PMU_CTRL_ON);
		d4_rmu_release(rmu);
	}
#endif
	misc_deregister(&drime4_be_miscdev);
	iounmap(g_be->reg_be_dma);
	iounmap(g_be->reg_be_3dme);
	iounmap(g_be->reg_be_fme);
	iounmap(g_be->reg_be_blend);
	iounmap(g_be->reg_be_sg);
	iounmap(g_be->reg_be_snr);
	iounmap(g_be->reg_be_ghost);
	iounmap(g_be->reg_be_top);

	free_irq(g_be->be_irq, NULL);

#ifndef CONFIG_PMU_SELECT
	/* BE Clock Disable */
	if (clk_di != 0) {
		clk_disable(g_be->clock);
		clk_put(g_be->clock);
	}
#endif
	kfree(g_be);

	return 0;
}

static int drime4_bayer_release_iomap(struct platform_device *pdev, int res_num)
{
	int ret = 0;
	struct resource *res = NULL;
	/* get resource for io memory  */
	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (!res) {
		dev_dbg(&pdev->dev,
				"[D4/BAYER]FAILED TO GET IO MEMORY REGION\n");
		ret = -ENODEV;
		goto out;
	}

	/* release mem region  */
	release_mem_region(res->start, resource_size(res));
	dev_dbg(&pdev->dev, "[D4/BAYER]RELEASE IO MEMORY REGION\n");

	out:
	return ret;
}

static int drime4_bayer_suspend(struct platform_device *pdev,
		pm_message_t state)
{
	int ret = 0;

	int i;
	printk("#####[D4/BE] drime4_bayer_suspend\n");
	ret = _drime4_bayer_suspend(pdev, 0);
	if (ret < 0)
		return ret;

	for (i = 0; i < 8; i++)
		ret = drime4_bayer_release_iomap(pdev, i);

	return ret;
}

static int __devinit drime4_bayer_probe(struct platform_device *pdev)
{
	return _drime4_bayer_resume(pdev, 1);
}

static int drime4_bayer_remove(struct platform_device *pdev)
{
	return _drime4_bayer_suspend(pdev, 1);
}

static int drime4_bayer_resume(struct platform_device *pdev)
{
	return _drime4_bayer_resume(pdev, 0);
}

static struct platform_driver drime4_be_driver = {
		.probe = drime4_bayer_probe,
		.remove = drime4_bayer_remove,
		.suspend = drime4_bayer_suspend,
		.resume = drime4_bayer_resume,
		.driver = {
			.name = BE_MODULE_NAME,
			.owner = THIS_MODULE,
		}, };

static int drime4_bayer_register(void)
{
	return platform_driver_register(&drime4_be_driver);
}

static void drime4_bayer_unregister(void)
{
	platform_driver_unregister(&drime4_be_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(drime4_bayer_register);
#else
fast_dev_initcall(drime4_bayer_register);
#endif
module_exit(drime4_bayer_unregister);

MODULE_AUTHOR("Niladri Mukherjee <n.mukherjee@samsung.com>");
MODULE_DESCRIPTION("Samsung Drime IV BE Driver");
MODULE_LICENSE("GPL");
