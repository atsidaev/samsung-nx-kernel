/* linux/drivers/pmu/d4-pwm.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	Kyuchun Han <kyuchun.han@samsung.com>
 *
 * DRIME4 Hardware Trigger PWM platform device driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <mach/d4_pmu_regs.h>
#include <linux/d4-pmu.h>
#include <mach/dma.h>
#include <linux/dma-mapping.h>




static DEFINE_MUTEX(pmu_lock);


struct d4_pmu_device {
	struct platform_device *pdev;
	void __iomem *regs;
	void __iomem *regs2;
	struct resource *pmu_res;
};

static LIST_HEAD(pmu_list);

static struct d4_pmu_device *pmudev;

static inline void wait_for_ack(enum pmu_ip_type ip)
{
	void __iomem *base = pmudev->regs;

	int timeout = 4000;
	while (0 == (0x1&(__raw_readl(base + PMU_ACK_STOP) >> ip))) {

		if (--timeout < 0) {
			printk(KERN_ERR "PMU not responding (!ack)\n");
			return;
		}
		udelay(10);
	}
}

int pmu_wait_for_ack(enum pmu_ip_type ip)
{
	int ret;

	int timeout = 40000;
	void __iomem *base = pmudev->regs2;
	unsigned int val;
	unsigned int reg;
	mutex_lock(&pmu_lock);

	ret = 0;

	if (ip == PMU_GPU)
		goto out;

	if (ip == PMU_PP)
		reg = BUS_RESET_PP;
	else if (ip == PMU_IPCM)
		reg = BUS_RESET_IPCM;
	else if (ip == PMU_IPCS)
		reg = BUS_RESET_IPCS;
	else if (ip == PMU_BAYER)
		reg = BUS_RESET_BE;
	else if (ip == PMU_EP)
		reg = BUS_RESET_EP;
	else
		reg = BUS_RESET_DP;

	val = __raw_readl(base + reg + 0x8);
	ret = 0;
	while (0 != (0x3&(__raw_readl(base + reg + 0x8) >> 4))) {
		if (--timeout < 0) {
			printk(KERN_ERR "PMU not responding (!ack)\n");
			ret = -1;
			goto out;
		}
		udelay(10);
	}

out:
		mutex_unlock(&pmu_lock);
		return ret;
}


int wait_for_stable(enum pmu_ip_type ip)
{
	int ret;
	void __iomem *base = pmudev->regs;

	int timeout = 50000;
	ret = 0;
	mutex_lock(&pmu_lock);
	while (1 == (0x1&(__raw_readl(base + PMU_SCACK) >> ip))) {

		if (--timeout < 0) {
			printk(KERN_ERR "PMU not scack\n");
			ret = -1;
			goto out;
		}
		udelay(2);
	}

	udelay(1);
	out:
			mutex_unlock(&pmu_lock);
			return ret;
}


static inline void d4_pmu_request_stop(enum pmu_ip_type ip)
{
	void __iomem *base = pmudev->regs;
	unsigned int val;
	val = __raw_readl(base + PMU_REQ_STOP);
	val &= ~(1 << ip);
	__raw_writel(val, base + PMU_REQ_STOP);

}

int d4_pmu_check(enum pmu_ip_type ip)
{
	unsigned int val;
	void __iomem *base = pmudev->regs;
	int rtval = 0;
	mutex_lock(&pmu_lock);
	val = __raw_readl(base + PMU_SCACK);
	val = 0x1 & (val >> ip);

	if (val)
		rtval = 0;
	else
		rtval = -1;
	mutex_unlock(&pmu_lock);
	return rtval;
}

int d4_pmu_request(enum pmu_ip_type ip)
{
	unsigned int val;
	void __iomem *base = pmudev->regs;
	mutex_lock(&pmu_lock);

	val = __raw_readl(base + PMU_REQ_STOP);
	val |= (1 << ip);
	__raw_writel(val, base + PMU_REQ_STOP);
	wait_for_ack(ip);
	mutex_unlock(&pmu_lock);
	return 0;
}


void d4_pmu_request_clear(enum pmu_ip_type ip)
{
	unsigned int val;
	void __iomem *base = pmudev->regs;
	mutex_lock(&pmu_lock);
	val = __raw_readl(base + PMU_REQ_STOP);
	val &= ~(1 << ip);
	__raw_writel(val, base + PMU_REQ_STOP);
	mutex_unlock(&pmu_lock);
}


void d4_pmu_isoen_set(enum pmu_ip_type ip, enum pmu_ctrl_type value)
{
	void __iomem *base = pmudev->regs;
	unsigned int val;

	mutex_lock(&pmu_lock);
	val = __raw_readl(base + PMU_ISOEN);

	if (value == PMU_CTRL_ON) {
		val |= (1 << ip);
	} else {
		val &= ~(1 << ip);
	}
	__raw_writel(val, base + PMU_ISOEN);
	mutex_unlock(&pmu_lock);
}
EXPORT_SYMBOL(d4_pmu_isoen_set);

void d4_pmu_scpre_set(enum pmu_ip_type ip, enum pmu_ctrl_type value)
{
	void __iomem *base = pmudev->regs;
	unsigned int val;
	mutex_lock(&pmu_lock);
	val = __raw_readl(base + PMU_SCPRE);

	if (value == PMU_CTRL_ON) {
		d4_pmu_request_stop(ip);
		val |= (1 << ip);
	} else {
		val &= ~(1 << ip);
	}
	__raw_writel(val, base + PMU_SCPRE);
	mutex_unlock(&pmu_lock);
}
EXPORT_SYMBOL(d4_pmu_scpre_set);


void d4_pmu_scall_set(enum pmu_ip_type ip, enum pmu_ctrl_type value)
{
	void __iomem *base = pmudev->regs;
	unsigned int val;
	mutex_lock(&pmu_lock);
	val = __raw_readl(base + PMU_SCALL);

	if (value == PMU_CTRL_ON) {
		val |= (1 << ip);
	} else {
		val &= ~(1 << ip);
	}
	__raw_writel(val, base + PMU_SCALL);
	mutex_unlock(&pmu_lock);
}
EXPORT_SYMBOL(d4_pmu_scall_set);


void d4_pmu_bus_reset(enum pmu_ip_type ip)
{
	void __iomem *base = pmudev->regs2;
	unsigned int val;
	unsigned int reg;
		mutex_lock(&pmu_lock);

		if ((ip == PMU_GPU) || (ip == PMU_CODEC))
			goto out;

		if (ip == PMU_PP)
			reg = BUS_RESET_PP;
		else if (ip == PMU_IPCM)
			reg = BUS_RESET_IPCM;
		else if (ip == PMU_IPCS)
			reg = BUS_RESET_IPCS;
		else if (ip == PMU_BAYER)
			reg = BUS_RESET_BE;
		else if (ip == PMU_EP)
			reg = BUS_RESET_EP;
		else if (ip == PMU_JPEG)
			reg = BUS_RESET_JPEG;
		else
			reg = BUS_RESET_DP;

		val = __raw_readl(base + reg);
		val = val|0x1;
		__raw_writel(val, (base + reg));
		val = val&0xFFFFFFFE;
		__raw_writel(val, (base + reg));

out:
		mutex_unlock(&pmu_lock);
}


struct resource *d4_pmu_res_request(void)
{
	return pmudev->pmu_res;
}
EXPORT_SYMBOL(d4_pmu_res_request);



static int d4_pmu_probe(struct platform_device *pdev)
{
	struct resource *dmatx_res;
	struct device *dev = &pdev->dev;
	struct resource *res;
	int ret;

	ret = 0;

	if (!pdev) {
		dev_err(dev, "No Platform Data\n");
		return -ENXIO;
	}

	pmudev = kzalloc(sizeof(struct d4_pmu_device), GFP_KERNEL);
	if (pmudev == NULL) {
		dev_err(dev, "failed to allocate d4_pmu_device\n");
		return -ENOMEM;
	}

	pmudev->pdev = pdev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		ret = -ENODEV;
		goto err_free;
	}

	res = request_mem_region(res->start, resource_size(res), pdev->name);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to request memory resource\n");
		ret = -EBUSY;
		goto err_free;
	}

	pmudev->regs = ioremap(res->start, resource_size(res));

	if (pmudev->regs == NULL) {
		dev_err(&pdev->dev, "failed to ioremap() registers\n");
		ret = -ENODEV;
		goto err_free;
	}

	/*bus register memory*/
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (res == NULL) {
		dev_err(&pdev->dev, "no bus memory resource defined\n");
		ret = -ENODEV;
		goto err_free;
	}

	res = request_mem_region(res->start, resource_size(res), pdev->name);

	if (res == NULL) {
		dev_err(&pdev->dev, "failed to request bus memory resource\n");
		ret = -EBUSY;
		goto err_free;
	}
	pmudev->pmu_res = res;
	pmudev->regs2 = ioremap(res->start, resource_size(res));

	if (pmudev->regs2 == NULL) {
		dev_err(&pdev->dev, "failed to ioremap() bus registers\n");
		ret = -ENODEV;
		goto err_free;
	}
	platform_set_drvdata(pdev, pmudev);
	return ret;

err_free:
	kfree(pmudev);
err_occur:
	return ret;
}


static int d4_pmu_remove(struct platform_device *pdev)
{
	struct d4_pmu_device *pmu = platform_get_drvdata(pdev);
	struct resource *mem_res;

	if (pmu == NULL)
		return -1;

	iounmap((void *) pmu->regs);

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem_res != NULL)
		release_mem_region(mem_res->start, resource_size(mem_res));

	platform_set_drvdata(pdev, NULL);
	kfree(pmu);
	return 0;
}


#ifdef CONFIG_PM
static int d4_pmu_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int d4_pmu_resume(struct platform_device *pdev)
{
	return 0;
}

#else
#define d4_pmu_suspend NULL
#define d4_pmu_resume NULL
#endif

static struct platform_driver d4_pmu_driver = { .driver = {
	.name = "drime4_pmu",
	.owner = THIS_MODULE,
}, .probe = d4_pmu_probe, .remove = d4_pmu_remove,
		.suspend = d4_pmu_suspend, .resume = d4_pmu_resume, };



static int __init d4_pmu_init(void)
{
	int ret;
	ret = platform_driver_register(&d4_pmu_driver);
	return 0;
}

arch_initcall(d4_pmu_init);

static void __exit d4_pmu_exit(void)
{
	platform_driver_unregister(&d4_pmu_driver);
}


module_exit(d4_pmu_exit);

MODULE_AUTHOR("Kyuchun han, <kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("drime4 PMU Driver");
MODULE_LICENSE("GPL");

