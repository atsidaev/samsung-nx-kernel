/**
 * @file d4_jpeg.c
 * @brief DRIMe4 JPEG Driver File
 * @author JinHyoung An <jh0913.an@samsung.com>
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

#ifdef CONFIG_PMU_SELECT
#include <media/drime4/opener/d4_opener_ioctl.h>
#include <linux/d4-pmu.h>
#endif

#include <mach/jpeg/d4_jpeg.h>
#include "d4_jpeg_ctrl_dd.h"

extern const struct file_operations jpeg_ops;

static struct miscdevice jpeg_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = JPEG_MODULE_NAME,
	.fops = &jpeg_ops,
};

static struct JPEG_REG_CTRL_BASE_INFO jpeg_base_info;
static struct resource *d4_jpeg_mem[2];
struct drime4_jpeg *jpeg_main_ctx;

/**
 * @brief JPEG device resource register
 * @fn d4_jpeg_register_iomap(struct platform_device *pdev, struct drime4_jpeg *jpeg_ctx, int res_num)
 * @param struct platform_device *pdev, struct drime4_jpeg *jpeg_ctx, int res_num
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jpeg_register_iomap(struct platform_device *pdev,
				struct drime4_jpeg *jpeg_ctx, int res_num)
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
	d4_jpeg_mem[res_num] = request_mem_region(res->start,
					resource_size(res), pdev->name);
	if (!d4_jpeg_mem[res_num]) {
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

	if (0 == res_num) {
		jpeg_base_info.topRegBase = (unsigned int)regs;
		jpeg_base_info.physTopReg.startAddr = (unsigned int)res->start;
		jpeg_base_info.physTopReg.size = (unsigned int)resource_size(res);
	} else if (1 == res_num) {
		jpeg_base_info.ctrlRegBase = (unsigned int)regs;
		jpeg_base_info.physCtrlReg.startAddr  = (unsigned int)res->start;
		jpeg_base_info.physCtrlReg.size = (unsigned int)resource_size(res);
	}

	return 0;

	ERROR_IOREMAP:
	release_resource(d4_jpeg_mem[res_num]);
	kfree(d4_jpeg_mem[res_num]);

	out: return ret;
}

/**
 * @brief JPEG device resource release
 * @fn d4_jpeg_release_iomap(struct platform_device *pdev, struct drime4_jpeg *jpeg_ctx, int res_num)
 * @param struct platform_device *pdev, struct drime4_jpeg *jpeg_ctx, int res_num
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jpeg_release_iomap(struct platform_device *pdev,
				struct drime4_jpeg *jpeg_ctx, int res_num)
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

	out: return ret;
}

/**
 * @brief JPEG device probe
 * @fn d4_jpeg_probe(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int __devinit d4_jpeg_probe(struct platform_device *pdev)
{
	struct drime4_jpeg *jpeg_ctx = NULL;
	int ret = 0;

	jpeg_ctx = kzalloc(sizeof(struct drime4_jpeg), GFP_KERNEL);
	if (!jpeg_ctx) {
		dev_err(&pdev->dev, "failed to allocate drime4_jpeg.\n");
		ret = -EINVAL;
		goto out;
	}

	jpeg_ctx->pd = (struct drime4_jpeg_dev_data *)pdev->dev.platform_data;
	if (jpeg_ctx->pd == NULL) {
		dev_err(&pdev->dev, "jpeg platform_data is empty.\n");
		ret = -ENOSPC;
		goto ERROR_PLATFORM_DATA;
	}

	jpeg_ctx->id 	  = pdev->id;
	jpeg_ctx->dev    = &pdev->dev;
	jpeg_ctx->name = pdev->name;

	/**< JPEG Clock Enable */
#ifndef CONFIG_PMU_SELECT
	jpeg_ctx->clock = clk_get(&pdev->dev, "jpeg");

	if (jpeg_ctx->clock == -2) {
		kfree(jpeg_ctx);
		ret = -EINVAL;
		goto out;
	}
	clk_enable(jpeg_ctx->clock);
	clk_set_rate(jpeg_ctx->clock, 216000000);
#endif
	jpeg_base_info.dev_info = &pdev->dev;

	/* register iomap resources */
	ret = d4_jpeg_register_iomap(pdev, jpeg_ctx, 0); /* CLOCK Common Register Base */
	if (ret < 0)
		goto ERROR_IOREMAP_0;

	ret = d4_jpeg_register_iomap(pdev, jpeg_ctx, 1); /* JPEG Common Register Base */
	if (ret < 0)
		goto ERROR_IOREMAP_1;

	/* register irq resource */
	jpeg_base_info.irq_num = platform_get_irq(pdev, 0);
	if (jpeg_base_info.irq_num < 0) {
		dev_err(&pdev->dev, "No IRQ specified.\n");
		ret = -ENOENT;
		goto ERROR_MISC_REGISTER;
	}


	/**< JPEG Core Base Register Address & IRQ Number Setting */
	jpeg_set_reg_ctrl_base_info(&jpeg_base_info);

	/* jpeg device registration */
	ret = misc_register(&jpeg_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		goto ERROR_MISC_REGISTER;
	}
	dev_dbg(&pdev->dev, "device registered as /dev/%s \n", JPEG_MODULE_NAME);

	platform_set_drvdata(pdev, jpeg_ctx);

	jpeg_ctx->dev = &pdev->dev;
	jpeg_main_ctx = jpeg_ctx;

	dev_dbg(&pdev->dev, "probe success.\n");

	return 0;

	ERROR_MISC_REGISTER:
	iounmap((void __iomem *)jpeg_base_info.ctrlRegBase);
	release_resource(d4_jpeg_mem[1]);
	kfree(d4_jpeg_mem[1]);
	ERROR_IOREMAP_1:
	iounmap((void __iomem *)jpeg_base_info.topRegBase);
	release_resource(d4_jpeg_mem[0]);
	kfree(d4_jpeg_mem[0]);
	ERROR_IOREMAP_0:
	ERROR_PLATFORM_DATA:
	kfree(jpeg_ctx);

	out:
	return ret;
}

/**
 * @brief JPEG device remove
 * @fn d4_jpeg_remove(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jpeg_remove(struct platform_device *pdev)
{
	misc_deregister(&jpeg_miscdev);

	iounmap((void __iomem *)jpeg_base_info.topRegBase);
	iounmap((void __iomem *)jpeg_base_info.ctrlRegBase);

	release_resource(d4_jpeg_mem[0]);
	release_resource(d4_jpeg_mem[1]);

	kfree(d4_jpeg_mem[0]);
	kfree(d4_jpeg_mem[1]);

	/**< JPEG Clock Disable */
#ifndef CONFIG_PMU_SELECT
	clk_disable(jpeg_main_ctx->clock);
	clk_put(jpeg_main_ctx->clock);
#endif
	kfree(jpeg_main_ctx);
	return 0;
}

/**
 * @brief JPEG device suspend
 * @fn d4_jpeg_suspend(struct platform_device *pdev, pm_message_t state)
 * @param struct platform_device *pdev, pm_message_t state
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jpeg_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret = 0;
#ifdef CONFIG_PMU_SELECT
	int reval;
	reval = d4_pmu_check(PMU_JPEG);
	if (reval != 0) {
		d4_pmu_isoen_set(PMU_JPEG, PMU_CTRL_ON);

		clk_disable(jpeg_main_ctx->clock);
		clk_put(jpeg_main_ctx->clock);
		d4_pmu_scpre_set(PMU_JPEG, PMU_CTRL_ON);
		d4_pmu_scall_set(PMU_JPEG, PMU_CTRL_ON);
	}
#endif

#if 0
	misc_deregister(&jpeg_miscdev);
#endif

	iounmap((void __iomem *)jpeg_base_info.topRegBase);
	iounmap((void __iomem *)jpeg_base_info.ctrlRegBase);

#ifndef CONFIG_PMU_SELECT
	clk_disable(jpeg_main_ctx->clock);
	clk_put(jpeg_main_ctx->clock);
#endif

	ret = d4_jpeg_release_iomap(pdev, jpeg_main_ctx, 0);
	if (ret < 0)
		goto out;

	ret = d4_jpeg_release_iomap(pdev, jpeg_main_ctx, 1);
	if (ret < 0)
		goto out;

	kfree(jpeg_main_ctx);
	return ret;

	out:
	printk(KERN_DEBUG "jpeg_release_iomap fail : %d\n", ret);
	kfree(jpeg_main_ctx);
	return ret;
}

/**
 * @brief JPEG device resume
 * @fn d4_jpeg_resume(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jpeg_resume(struct platform_device *pdev)
{
	struct drime4_jpeg *jpeg_ctx = NULL;
	int ret = 0;

	printk("\n\n [ jpeg resume start ]\n\n");
	jpeg_ctx = kzalloc(sizeof(struct drime4_jpeg), GFP_KERNEL);
	if (!jpeg_ctx) {
		dev_err(&pdev->dev, "failed to allocate drime4_jpeg.\n");
		ret = -EINVAL;
		goto out;
	}

	jpeg_ctx->pd = (struct drime4_jpeg_dev_data *)pdev->dev.platform_data;
	if (jpeg_ctx->pd == NULL) {
		dev_err(&pdev->dev, "jpeg platform_data is empty.\n");
		ret = -ENOSPC;
		goto ERR_PLATFORM_DATA;
	}

	jpeg_ctx->id = pdev->id;
	jpeg_ctx->dev = &pdev->dev;
	jpeg_ctx->name = pdev->name;

#ifdef CONFIG_PMU_SELECT
	if (d4_get_kdd_open_count(KDD_JPEG) > 0) {
		jpeg_ctx->clock = clk_get(&pdev->dev, "jpeg");

		if (jpeg_ctx->clock == -2) {
			kfree(jpeg_ctx);
			return -EINVAL;
		}
		clk_enable(jpeg_ctx->clock);
		clk_set_rate(jpeg_ctx->clock, 216000000);
	}
#endif
	jpeg_base_info.dev_info = &pdev->dev;

	/* register iomap resources */
	ret = d4_jpeg_register_iomap(pdev, jpeg_ctx, 0); /* CLOCK Common Register Base */
	if (ret < 0)
		goto ERR_IOREMAP_0;

	ret = d4_jpeg_register_iomap(pdev, jpeg_ctx, 1); /* JPEG Common Register Base */
	if (ret < 0)
		goto ERR_IOREMAP_1;

	/* register irq resource */
	jpeg_base_info.irq_num = platform_get_irq(pdev, 0);

	/**< JPEG Core Base Register Address & IRQ Number Setting */
	jpeg_set_reg_ctrl_base_info(&jpeg_base_info);

#if 0
	/* jpeg device registration */
	ret = misc_register(&jpeg_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		goto ERR_MISC_REGISTER;
	}
	dev_err(&pdev->dev, "device registered as /dev/%s \n", JPEG_MODULE_NAME);
#endif

	platform_set_drvdata(pdev, jpeg_ctx);

	jpeg_ctx->dev = &pdev->dev;
	jpeg_main_ctx = jpeg_ctx;

	dev_err(&pdev->dev, "probe success.\n");
	printk("\n\n [ jpeg resume end ]\n\n");

	return 0;

	ERR_MISC_REGISTER:
	iounmap((void __iomem *)jpeg_base_info.ctrlRegBase);
	release_resource(d4_jpeg_mem[1]);
	kfree(d4_jpeg_mem[1]);
	ERR_IOREMAP_1:
	iounmap((void __iomem *)jpeg_base_info.topRegBase);
	release_resource(d4_jpeg_mem[0]);
	kfree(d4_jpeg_mem[0]);
	ERR_IOREMAP_0:
	ERR_PLATFORM_DATA:
	kfree(jpeg_ctx);

	out:
	return ret;
}

static struct platform_driver jpeg_driver = {
	.probe = d4_jpeg_probe,
	.remove = d4_jpeg_remove,
	.suspend = d4_jpeg_suspend,
	.resume = d4_jpeg_resume,
	.driver = {
		.name = JPEG_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

/**
 * @brief JPEG device register
 * @fn d4_jpeg_register(void)
 * @param void
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jpeg_register(void)
{
	platform_driver_register(&jpeg_driver);

	return 0;
}

/**
 * @brief JPEG device unregister
 * @fn d4_jpeg_unregister(void)
 * @param void
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static void d4_jpeg_unregister(void)
{
	platform_driver_unregister(&jpeg_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(d4_jpeg_register);
#else
fast_dev_initcall(d4_jpeg_register);
#endif
module_exit(d4_jpeg_unregister);

MODULE_AUTHOR("JinHyoung An <jh0913.an@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 JPEG driver File");
MODULE_LICENSE("GPL");
