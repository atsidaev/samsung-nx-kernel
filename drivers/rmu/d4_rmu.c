/* arch/arm/mach-drime4/d4_ptc.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	Kyuchun Han <kyuchun.han@samsung.com>
 *
 * DRIME4 Pulse Trigger Counter platform device driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <mach/map.h>
#include <linux/d4_rmu.h>
#include <mach/d4_rmu_regs.h>
#include <linux/delay.h>
struct d4_rmu_device {
	struct rmu_device *rmu_dev;
};

struct rmu_device {
	struct platform_device *pdev;
	void __iomem *reg;
	struct resource *ioarea;
};

static struct rmu_device *rmu_dev;

struct d4_rmu_device *d4_rmu_request(void)
{
	struct d4_rmu_device *rmu;

	rmu = kzalloc(sizeof(struct d4_rmu_device), GFP_KERNEL);
	if (!rmu) {
		return NULL;
	}
	rmu->rmu_dev = rmu_dev;
	return rmu;
}
EXPORT_SYMBOL(d4_rmu_request);

void d4_rmu_release(struct d4_rmu_device *rmu)
{
	if (rmu != NULL)
		kfree(rmu);
}
EXPORT_SYMBOL(d4_rmu_release);

int d4_pcu_resource_get(struct d4_rmu_device *rmu, enum pcu_resource power_type)
{
	struct rmu_device *rmu_dev = rmu->rmu_dev;
	void __iomem *base = rmu_dev->reg;
	unsigned int val;

	val = readl(base + PCU_LATCH);

	if ((val >> power_type) & 0x1)
		return 0;
	else
		return -1;
}
EXPORT_SYMBOL(d4_pcu_resource_get);

void d4_pcu_hold_set(struct d4_rmu_device *rmu)
{
	struct rmu_device *rmu_dev;
	void __iomem *base;
	unsigned int val;

	if (rmu == NULL)
		return;

	rmu_dev = rmu->rmu_dev;
	base = rmu_dev->reg;

	val = readl(base + PCU_CTRL);
	PCU_DPOH(val, 0x1);
	writel(val, base + PCU_CTRL);

	val = readl(base + PCU_CTRL);
	PCU_DRSTA(val, 0x0);
	writel(val, base + PCU_CTRL);
}
EXPORT_SYMBOL(d4_pcu_hold_set);

void d4_pcu_off_set(struct d4_rmu_device *rmu)
{
	struct rmu_device *rmu_dev;
	void __iomem *base;
	unsigned int val;

	if (rmu == NULL)
		return;

	rmu_dev = rmu->rmu_dev;
	base = rmu_dev->reg;
	val = readl(base + PCU_CTRL);
	PCU_DPOH(val, 0x0);
	writel(val, base + PCU_CTRL);
}
EXPORT_SYMBOL(d4_pcu_off_set);

void d4_pcu_ddroff_set(struct d4_rmu_device *rmu)
{
	struct rmu_device *rmu_dev;
	void __iomem *base;
	unsigned int val;

	if (rmu == NULL)
		return;

	rmu_dev = rmu->rmu_dev;
	base = rmu_dev->reg;

	val = readl(base + PCU_CTRL);
	PCU_DDRDPOH(val, 0x0);
	writel(val, base + PCU_CTRL);
}
EXPORT_SYMBOL(d4_pcu_ddroff_set);


void d4_pcu_intr_mask_set(struct d4_rmu_device *rmu)
{
//	struct rmu_device *rmu_dev = rmu->rmu_dev;
	void __iomem *base = rmu_dev->reg;
	unsigned int val;

	val = readl(base + PCU_CTRL);
	PCU_DCON(val, 0x0);
	writel(val, base + PCU_CTRL);

	val = readl(base + PCU_CTRL);
	PCU_DCON(val, 0x1);
	writel(val, base + PCU_CTRL);

	/* delay set*/
	mdelay(10);

	val = readl(base + PCU_CTRL);
	PCU_DCON(val, 0x0);
	writel(val, base + PCU_CTRL);

}
EXPORT_SYMBOL(d4_pcu_intr_mask_set);

void d4_sw_pll_reset(struct d4_rmu_device *rmu, enum sw_reset_pll pll_type)
{
	struct rmu_device *rmu_dev = rmu->rmu_dev;
	void __iomem *base = rmu_dev->reg;
	unsigned int val;

	val = readl(base + SWREST_PLL);
	val |= 1 << pll_type;
	writel(val, base + SWREST_PLL);

	val = readl(base + SWREST_PLL);
	val &= ~(1 << pll_type);
	writel(val, base + SWREST_PLL);

}
EXPORT_SYMBOL(d4_sw_pll_reset);

unsigned int d4_hw_status_read(struct d4_rmu_device *rmu,
		enum hw_status_reg hw_status)
{
	struct rmu_device *rmu_dev = rmu->rmu_dev;
	void __iomem *base = rmu_dev->reg;
	unsigned int val;

	val = readl(base + HW_STS);

	if (hw_status == DFTMON_CA9)
		val = (val >> hw_status) & 0x3;
	else if (hw_status == SYS_REMAP)
		val = (val >> hw_status) & 0x3;
	else if (hw_status == SYS_CFG)
		val = (val >> hw_status) & 0x7F;
	else
		val = (val >> hw_status) & 0x1;

	return val;
}
EXPORT_SYMBOL(d4_hw_status_read);

unsigned int d4_pll_monitor_read(struct d4_rmu_device *rmu,
		enum pll_monitor_reg pll_mon)
{
	struct rmu_device *rmu_dev = rmu->rmu_dev;
	void __iomem *base = rmu_dev->reg;
	unsigned int val;

	val = readl(base + PLL_MON);
	val = (val >> pll_mon) & 0x1;

	return val;
}
EXPORT_SYMBOL(d4_pll_monitor_read);

void d4_sw_isp_reset(struct d4_rmu_device *rmu, enum sw_reset_ips ip_type)
{
	struct rmu_device *rmu_dev;
	void __iomem *base;
	unsigned int val;

	if (rmu == NULL)
		return;

	if (rmu->rmu_dev == NULL)
		return;

	rmu_dev = rmu->rmu_dev;
	base = rmu_dev->reg;

	val = readl(base + SWREST_IP);
	val |= (1 << ip_type);
	writel(val, base + SWREST_IP);
}
EXPORT_SYMBOL(d4_sw_isp_reset);


void d4_sw_isp_reset_release(struct d4_rmu_device *rmu, enum sw_reset_ips ip_type)
{
	struct rmu_device *rmu_dev;
	void __iomem *base;
	unsigned int val;

	if (rmu == NULL)
		return;

	rmu_dev = rmu->rmu_dev;
	base = rmu_dev->reg;
	
	val = readl(base + SWREST_IP);
	val &= ~(1 << ip_type);
	writel(val, base + SWREST_IP);
}
EXPORT_SYMBOL(d4_sw_isp_reset_release);

static void pcu_hold_set(void)
{
	void __iomem *base = rmu_dev->reg;
	unsigned int val;

	val = readl(base + PCU_CTRL);
	PCU_DPOH(val, 0x1);
	writel(val, base + PCU_CTRL);

	val = readl(base + PCU_CTRL);
	PCU_DRSTA(val, 0x0);
	writel(val, base + PCU_CTRL);
}


static void pcu_off_set(void)
{
	void __iomem *base = rmu_dev->reg;
	unsigned int val;

	val = readl(base + PCU_CTRL);
	PCU_DPOH(val, 0x0);
	writel(val, base + PCU_CTRL);
}



static int d4_rmu_probe(struct platform_device *pdev)
{
	struct rmu_device *rmu;
	struct resource *res;
	int ret = 0;

	if (!pdev) {
		dev_err(&pdev->dev, "No Platform Data\n");
		return -ENXIO;
	}

	rmu = kzalloc(sizeof(struct rmu_device), GFP_KERNEL);
	if (rmu == NULL) {
		dev_err(&pdev->dev, "failed to allocate ht_ptc_device\n");
		return -ENOMEM;
	}

	rmu_dev = rmu;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		ret = -ENODEV;
		goto err_free;
	}

	rmu->ioarea
			= request_mem_region(res->start, resource_size(res), pdev->name);
	if (rmu->ioarea == NULL) {
		dev_err(&pdev->dev, "failed to request memory resource\n");
		ret = -EBUSY;
		goto err_free;
	}

	rmu->reg = ioremap(res->start, resource_size(res));
	if (rmu->reg == NULL) {
		dev_err(&pdev->dev, "failed to ioremap() registers\n");
		ret = -ENODEV;
		goto err_free_mem;
	}
	platform_set_drvdata(pdev, rmu);
	return ret;

err_free_mem:
    release_mem_region(res->start, resource_size(res));

err_free:
    kfree(rmu);

	return ret;
}


static void __devexit d4_rmu_shutdown(struct platform_device *pdev)
{
//    pcu_hold_set();
//    pcu_off_set();
}

static int __devexit d4_rmu_remove(struct platform_device *pdev)
{
	struct rmu_device *rmu = platform_get_drvdata(pdev);
	if (rmu == NULL)
		return -1;

	release_resource(rmu->ioarea);
	kfree(rmu->ioarea);

	return 0;
}

#ifdef CONFIG_PM
static int d4_rmu_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int d4_rmu_resume(struct platform_device *pdev)
{
	return 0;
}

#else
#define d4_rmu_suspend NULL
#define d4_rmu_resume NULL
#endif

static struct platform_driver d4_rmu_driver = { .driver = { .name =
		"drime4-rmu", .owner = THIS_MODULE, }, .probe = d4_rmu_probe, .remove =
		d4_rmu_remove, .suspend = d4_rmu_suspend, .resume = d4_rmu_resume,
		.shutdown = d4_rmu_shutdown, };

static int __init d4_rmu_init(void)
{
	int ret;
	ret = platform_driver_register(&d4_rmu_driver);
	if (ret)
	printk(KERN_ERR "%s: failed to add ptc driver\n", __func__);

	return ret;
}

arch_initcall(d4_rmu_init);

static void __exit d4_rmu_exit(void)
{
	platform_driver_unregister(&d4_rmu_driver);
}

module_exit(d4_rmu_exit);

MODULE_AUTHOR("Kyuchun han, <kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("drime4 Reset Management Unit Controller Driver");
MODULE_LICENSE("GPL");

