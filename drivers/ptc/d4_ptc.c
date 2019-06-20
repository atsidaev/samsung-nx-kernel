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
#include <linux/dma-mapping.h>

#include <linux/d4_ptc.h>
#include <mach/map.h>
#include <mach/d4_ptc_regs.h>
#include <asm/div64.h>

#define PTC_ENABLE		0x1
#define PTC_DISABLE		0x0

struct ptc_device {
	struct list_head		list;
	struct platform_device	*pdev;
	struct clk *clk;
	void __iomem			*reg;
	int						irq;
	unsigned char			ptc_id;
	void					(*int_handler)(void *dev);
	void				(*int_handler_rev)(void *dev);
	enum ptc_input_mode		input_type;
	struct completion done;
	unsigned int time;
};

static DEFINE_MUTEX(ptc_lock);
static LIST_HEAD(ptc_list);

#define msecs_to_loops(t) (loops_per_jiffy / 1000 * HZ * t)

struct ptc_device *ptc_request(int ptc_id)
{
	struct ptc_device *ptc;
	int found = 0;

	mutex_lock(&ptc_lock);

	list_for_each_entry(ptc, &ptc_list, list) {
		if (ptc->ptc_id == ptc_id) {
			found = 1;
			break;
		}
	}

	if (found == 0)
		ptc = ERR_PTR(-ENOENT);

	mutex_unlock(&ptc_lock);
	return ptc;
}
EXPORT_SYMBOL(ptc_request);

int ptc_oneinput_ctrl(struct ptc_device *ptc,
				struct ptc_oneinput_info *ctl_info)
{
	void __iomem *base = ptc->reg;
	unsigned int val, int_val;

	val = readl(base + PTC_REG_CTRL_PQ);
	int_val = readl(base + PTC_REG_INT_PQ);

	if (ctl_info->pCallback != NULL) {
		ptc->int_handler = ctl_info->pCallback;
		SET_REG_CLEAR_EN_P(val, PTC_ENABLE);
		SET_INT_ENABLE_P(int_val, PTC_ENABLE);
	} else {
		SET_REG_COUNT_EN_P(val, PTC_ENABLE);
		SET_REG_CLEAR_EN_P(val, PTC_DISABLE);
		SET_INT_ENABLE_P(int_val, PTC_DISABLE);
	}

	SET_REG_SEL_EDGE_P(val, ctl_info->edge_type);
	SET_REG_COUNT_EN_Q(val, PTC_DISABLE);


	writel(val, base + PTC_REG_CTRL_PQ);
	writel(int_val, base + PTC_REG_INT_PQ);
	ptc->input_type = PTC_ONE_INPUT;
	ptc->time = msecs_to_loops(ctl_info->time);
	return 0;
}
EXPORT_SYMBOL(ptc_oneinput_ctrl);

int ptc_twoinput_ctrl(struct ptc_device *ptc,
			struct ptc_twoinput_info *ctl_info)
{
	void __iomem *base = ptc->reg;
	unsigned int val;
	unsigned int ctrl_val, int_val;
	val = readl(base + PTC_REG_CTRL_DF);
	ctrl_val = readl(base + PTC_REG_CTRL_PQ);
	int_val = readl(base + PTC_REG_INT_PQ);

	if (ctl_info->pCallback != NULL) {
		ptc->int_handler = ctl_info->pCallback;
		if (ctl_info->run_type == PTC_ZPIB) {
			SET_REG_CLEAR_EN_P(ctrl_val, PTC_ENABLE);
			SET_REG_CLEAR_EN_Q(ctrl_val, PTC_ENABLE);
			SET_INT_ENABLE_P(int_val, PTC_ENABLE);
			SET_INT_ENABLE_Q(int_val, PTC_DISABLE);
		} else {
			SET_REG_CLEAR_EN_P(val, PTC_ENABLE);
			SET_REG_CLEAR_EN_Q(val, PTC_ENABLE);
			SET_INT_ENABLE_P(int_val, PTC_DISABLE);
			SET_INT_ENABLE_Q(int_val, PTC_ENABLE);
		}
	} else {
		SET_REG_CLEAR_EN_Q(ctrl_val, PTC_DISABLE);
		SET_REG_CLEAR_EN_P(ctrl_val, PTC_DISABLE);
		SET_REG_COUNT_EN_DF(val, PTC_ENABLE);
		SET_INT_ENABLE_P(int_val, PTC_DISABLE);
		SET_INT_ENABLE_Q(int_val, PTC_DISABLE);
	}

	writel(ctrl_val, base + PTC_REG_CTRL_PQ);
	writel(val, base + PTC_REG_CTRL_DF);
	writel(int_val, base + PTC_REG_INT_PQ);
	ptc->input_type = PTC_TWO_INPUT;
	return 0;
}
EXPORT_SYMBOL(ptc_twoinput_ctrl);


void ptc_revers_ctrl(struct ptc_device *ptc, ptc_callback pCallback_rev)
{
	void __iomem *base = ptc->reg;
	unsigned int val;
	unsigned int ctrl_val;
	val = readl(base + PTC_REG_CTRL_DF);
	ctrl_val = readl(base + PTC_REG_INT_DF);

	if (pCallback_rev != NULL) {
		ptc->int_handler_rev = pCallback_rev;
		SET_REG_INT_EN_REV(ctrl_val, PTC_ENABLE);
	} else {
		ptc->int_handler_rev = NULL;
		SET_REG_INT_EN_REV(ctrl_val, PTC_DISABLE);
	}

	SET_REG_REV_CHECK_EN(val, PTC_ENABLE);
	SET_REG_COUNT_EN_DF(val, PTC_ENABLE);
	writel(val, base + PTC_REG_CTRL_DF);
	writel(ctrl_val, base + PTC_REG_INT_DF);
}
EXPORT_SYMBOL(ptc_revers_ctrl);


unsigned int ptc_start(struct ptc_device *ptc, enum ptc_input_mode mode)
{
	void __iomem *base = ptc->reg;
	unsigned int ctrl;

	ctrl = 0;

	ctrl = readl(base + PTC_REG_CTRL_PQ);
	if (mode == PTC_ONE_INPUT)
		SET_REG_COUNT_EN_P(ctrl, PTC_ENABLE);

	else {
		SET_REG_COUNT_EN_P(ctrl, PTC_ENABLE);
		SET_REG_COUNT_EN_Q(ctrl, PTC_ENABLE);
	}
	writel(ctrl, base + PTC_REG_CTRL_PQ);
	if (!wait_for_completion_timeout(&ptc->done,
				msecs_to_jiffies(ptc->time))) {
		return -EIO;
	}
	return 0;
}
EXPORT_SYMBOL(ptc_start);


void ptc_stop(struct ptc_device *ptc)
{
	void __iomem *base = ptc->reg;
	unsigned int ctrl;

	ctrl = readl(base + PTC_REG_CTRL_PQ);
	SET_REG_COUNT_EN_P(ctrl, PTC_DISABLE);
	SET_REG_COUNT_EN_Q(ctrl, PTC_DISABLE);

	writel(ctrl, base + PTC_REG_CTRL_PQ);
}
EXPORT_SYMBOL(ptc_stop);

void ptc_count_set(struct ptc_device *ptc, enum ptc_run_type cnt_type,
				unsigned short cnt_value)
{
	void __iomem *base = ptc->reg;

	switch (cnt_type) {
	case PTC_ZPIB:
		writel(cnt_value, base + PTC_REG_COMPARE_P);
		writel(0x0, base + PTC_REG_COUNT_P);
		break;
	case PTC_ZPIC:
		writel(cnt_value, base + PTC_REG_COMPARE_Q);
		writel(0x0, base + PTC_REG_COUNT_Q);
		break;
	default:
		break;
	}
}
EXPORT_SYMBOL(ptc_count_set);

int ptc_count_get(struct ptc_device *ptc, enum ptc_run_type cnt_type)
{
	void __iomem *base = ptc->reg;
	int val;
	unsigned int check;
	val = -1;

	switch (cnt_type) {
	case PTC_ZPIB:
		val = readl(base + PTC_REG_COUNT_P);
		break;

	case PTC_ZPIC:
		val = readl(base + PTC_REG_COUNT_Q);
		break;

	case PTC_DF:
		val = readl(base + PTC_REG_COUNT_DF);
		check = GET_REG_COUNT_DF_CHECK(val);
		if (check == 0)
			val = GET_REG_COUNT_DF(val);
		 else
			val = ~(GET_REG_COUNT_DF(val)) + 1;

		break;

	default:
		break;
	}
	return val;
}
EXPORT_SYMBOL(ptc_count_get);


void ptc_revers_clear(struct ptc_device *ptc)
{
	void __iomem *base = ptc->reg;
	unsigned int val;
	unsigned int ctrl_val;
	val = readl(base + PTC_REG_CTRL_DF);
	ctrl_val = readl(base + PTC_REG_INT_DF);

	SET_REG_INT_EN_REV(ctrl_val, PTC_DISABLE);
	SET_REG_REV_CHECK_EN(val, PTC_DISABLE);
	SET_REG_COUNT_EN_DF(val, PTC_DISABLE);

	writel(val, base + PTC_REG_CTRL_DF);
	writel(ctrl_val, base + PTC_REG_INT_DF);
}
EXPORT_SYMBOL(ptc_revers_clear);

void ptc_cnt_clear(struct ptc_device *ptc, enum ptc_input_mode val)
{
	void __iomem *base = ptc->reg;
	unsigned int ctrl;

	ctrl = 0;

	ctrl = readl(base + PTC_REG_CTRL_PQ);

	if (val == PTC_ONE_INPUT)
		SET_REG_FCLEAR_P(ctrl, PTC_ENABLE);
	else {
		SET_REG_FCLEAR_P(ctrl, PTC_ENABLE);
		SET_REG_FCLEAR_Q(ctrl, PTC_ENABLE);
	}
	writel(ctrl, base + PTC_REG_CTRL_PQ);
}
EXPORT_SYMBOL(ptc_cnt_clear);


void ptc_df_clear(struct ptc_device *ptc)
{
	void __iomem *base = ptc->reg;
	unsigned int ctrl;

	ctrl = readl(base + PTC_REG_CTRL_DF);
	SET_REG_FCLEAR_DF(ctrl, PTC_ENABLE);
	writel(ctrl, base + PTC_REG_CTRL_DF);
}
EXPORT_SYMBOL(ptc_df_clear);

void ptc_nr_set(struct ptc_device *ptc, unsigned short nr_clk)
{
	void __iomem *base = ptc->reg;
	writel(nr_clk, base + PTC_REG_NR);
}
EXPORT_SYMBOL(ptc_nr_set);

void ptc_int_set(struct ptc_device *ptc, enum ptc_int_type int_type,
				enum ptc_input_mode mode)
{
	void __iomem *base = ptc->reg;
	unsigned int ctrl;
	unsigned int intctrl;

	ctrl = 0;
	intctrl = 0;

	ctrl = readl(base + PTC_REG_CTRL_PQ);
	intctrl = readl(base + PTC_REG_INT_PQ);

	if (int_type == PTC_INT_ENABLE) {
		if (mode == PTC_ONE_INPUT) {
			SET_REG_CLEAR_EN_P(ctrl, PTC_ENABLE);
			SET_INT_ENABLE_P(intctrl, PTC_ENABLE);
		} else {
			SET_REG_CLEAR_EN_P(ctrl, PTC_ENABLE);
			SET_REG_CLEAR_EN_Q(ctrl, PTC_ENABLE);
			SET_INT_ENABLE_P(intctrl, PTC_ENABLE);
			SET_INT_ENABLE_Q(intctrl, PTC_ENABLE);
		}
	} else {
		if (mode == PTC_ONE_INPUT) {
			SET_REG_CLEAR_EN_P(ctrl, PTC_DISABLE);
			SET_INT_ENABLE_P(intctrl, PTC_DISABLE);
		} else {
			SET_REG_CLEAR_EN_Q(ctrl, PTC_DISABLE);
			SET_INT_ENABLE_Q(intctrl, PTC_DISABLE);
		}
	}
	writel(intctrl, base + PTC_REG_INT_PQ);
	writel(ctrl, base + PTC_REG_CTRL_PQ);
}
EXPORT_SYMBOL(ptc_int_set);

static int d4_ptc_register(struct ptc_device *ptc)
{
	mutex_lock(&ptc_lock);
	list_add_tail(&ptc->list, &ptc_list);
	mutex_unlock(&ptc_lock);
	return 0;
}


static irqreturn_t d4_ptc_irq(int irq, void *dev)
{
	struct ptc_device *ptc = dev;
	void __iomem *base = ptc->reg;
	unsigned int val, rev_val;

	val = readl(base + PTC_REG_INT_PQ);
	rev_val = readl(base + PTC_REG_INT_DF);

	if (GET_EQUAL_FLAG_P(val) == 0x1) {
		SET_EQUAL_CLEAR_P(val, PTC_ENABLE);
		writel(val, base + PTC_REG_INT_PQ);
		if (ptc->int_handler != NULL)
			ptc->int_handler(ptc);

	} else if (GET_EQUAL_FLAG_Q(val) == 0x1) {
		SET_EQUAL_CLEAR_Q(val, PTC_ENABLE);
		writel(val, base + PTC_REG_INT_PQ);
		if (ptc->int_handler != NULL)
			ptc->int_handler(ptc);

	} else if (GET_REG_REV_FLAG(rev_val) == 0x1) {
		SET_REG_REV_FLAG_CLR(rev_val, PTC_ENABLE);
		writel(rev_val, base + PTC_REG_INT_DF);
		if (ptc->int_handler_rev != NULL)
			ptc->int_handler_rev(ptc);

	} else {
		return IRQ_HANDLED;
	}
	complete(&ptc->done);
	return IRQ_HANDLED;
}



static int d4_ptc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ptc_device *ptc;
	struct resource *res;
	int ret;


	if (!pdev) {
		printk("No Platform Data\n");
		return -ENXIO;
	}

	ptc = kzalloc(sizeof(struct ptc_device), GFP_KERNEL);
	if (ptc == NULL) {
		dev_err(dev, "failed to allocate ht_ptc_device\n");
		ret = -ENOMEM;
		goto err_fail;
	}

	ptc->pdev = pdev;
	ptc->ptc_id = pdev->id;


	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		ret = -ENODEV;
		goto err_malloc;
	}

	res = request_mem_region(res->start, resource_size(res), pdev->name);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to request memory resource\n");
		ret = -EBUSY;
		goto err_malloc;
	}

	ptc->reg = ioremap(res->start, resource_size(res));
	if (ptc->reg == NULL) {
		dev_err(&pdev->dev, "failed to ioremap() registers\n");
		ret = -ENODEV;
		goto err_free_mem;
	}

	init_completion(&ptc->done);

	ptc->irq = platform_get_irq(pdev, 0);
	if (ptc->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		ret = -ENOENT;
		goto err_irq;
	}

	ret = request_irq(ptc->irq, d4_ptc_irq, 0, pdev->name, ptc);
	if (ret) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_irq;
	}

	ret = d4_ptc_register(ptc);
	if (ret) {
		dev_err(dev, "failed to register ptc\n");
		ret = -EBUSY;
		goto err_irq;
	}

	platform_set_drvdata(pdev, ptc);

	ptc->clk = clk_get(&pdev->dev, "ptc");

	if (ptc->clk == -2) {
		ret = -ENXIO;
		goto err_irq;
	}

	clk_enable(ptc->clk);
	return 0;

err_irq:
	iounmap((void *) ptc->reg);

err_free_mem:
	release_mem_region(res->start, resource_size(res));


err_malloc:
	kfree(ptc);

err_fail:
	return ret;
}



static int __devexit d4_ptc_remove(struct platform_device *pdev)
{
	struct ptc_device *ptc = platform_get_drvdata(pdev);

	if (ptc != NULL)
		kfree(ptc);

	return 0;
}

#ifdef CONFIG_PM
static int d4_ptc_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int d4_ptc_resume(struct platform_device *pdev)
{
	return 0;
}

#else
#define d4_ptc_suspend NULL
#define d4_ptc_resume NULL
#endif

static struct platform_driver d4_ptc_driver = {
	.driver		= {
		.name	= "drime4-ptc",
		.owner	= THIS_MODULE,
	},
	.probe		= d4_ptc_probe,
	.remove		= d4_ptc_remove,
	.suspend	= d4_ptc_suspend,
	.resume		= d4_ptc_resume,
};

static int __init d4_ptc_init(void)
{
	int ret;
	ret = platform_driver_register(&d4_ptc_driver);
	if (ret)
		printk(KERN_ERR "%s: failed to add ptc driver\n", __func__);

	return ret;
}

arch_initcall(d4_ptc_init);


static void __exit d4_ptc_exit(void)
{
	platform_driver_unregister(&d4_ptc_driver);
}

module_exit(d4_ptc_exit);

MODULE_AUTHOR("Kyuchun han, <kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("drime4 Pulse Trigger Counter Controller Driver");
MODULE_LICENSE("GPL");

