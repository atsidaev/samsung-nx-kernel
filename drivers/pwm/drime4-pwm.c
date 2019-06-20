/* linux/drivers/pwm/drime4-pwm.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Driver for PWM(Pulse Width Modulator) controller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/pwm.h>

/* Register map */
#define DRIME4_PWM_CONTROL		0x0
#define DRIME4_PWM_CONFIG		0x04
#define DRIME4_PWM_PRESCALE		0x08
#define DRIME4_PWM_CYCLE1		0x0c
#define DRIME4_PWM_CYCLE2		0x10
#define DRIME4_PWM_EXTIN_SELCH1		0x900
#define DRIME4_PWM_EXTIN_SELCH2		0x904

#define DRIME4_PWM_CTRL_START		(1 << 0)
#define DRIME4_PWM_CTRL_CONTINUE	(1 << 3)

#define DRIME4_PWM_CONF_HW_TRIGGER	(1 << 2)

enum drime4_pwm_devtype {
	DRIME4_PWM_CORE,
	DRIME4_PWM,
};

struct drime4_pwm_device {
	struct device		*dev;
	struct clk		*clk;

	unsigned int		period_ns;
	unsigned int		duty_ns;

	void __iomem		*mmio_base;
	void __iomem		*enable_base;

	unsigned char		use_count;
	unsigned char		pwm_id;
	struct pwm_chip		chip;
};

#define to_drime4_pwm_device(chip)	\
	container_of(chip, struct drime4_pwm_device, chip)

static int drime4_pwm_enable(struct pwm_chip *chip)
{
	struct drime4_pwm_device *pwm = to_drime4_pwm_device(chip);
	unsigned long flags;
	unsigned long htcon, htconf;

	dev_dbg(pwm->dev, "enable\n");
	clk_enable(pwm->clk);

	local_irq_save(flags);
	htcon = __raw_readl(pwm->mmio_base + DRIME4_PWM_CONTROL);
	htcon |= DRIME4_PWM_CTRL_START;
	htcon |= DRIME4_PWM_CTRL_CONTINUE;
	__raw_writel(htcon, pwm->mmio_base + DRIME4_PWM_CONTROL);

	htconf = __raw_readl(pwm->mmio_base + DRIME4_PWM_CONFIG);
	htconf &= ~DRIME4_PWM_CONF_HW_TRIGGER;
	__raw_writel(htconf, pwm->mmio_base + DRIME4_PWM_CONFIG);

	local_irq_restore(flags);

	return 0;
}

static void drime4_pwm_disable(struct pwm_chip *chip)
{
	struct drime4_pwm_device *pwm = to_drime4_pwm_device(chip);
	unsigned long flags;
	unsigned long htcon;

	dev_dbg(pwm->dev, "disable\n");

	local_irq_save(flags);
	htcon = __raw_readl(pwm->mmio_base + DRIME4_PWM_CONTROL);
	htcon &= ~DRIME4_PWM_CTRL_START;
	__raw_writel(htcon, pwm->mmio_base + DRIME4_PWM_CONTROL);
	local_irq_restore(flags);

	clk_disable(pwm->clk);
}

#define NS_IN_HZ (1000000000UL)

/* period_ns = 10^9 * (prescale + 1) * period / PWM_CLK_RATE */
/* duty_ns = 10^9 * (prescale + 1) * period / PWM_CLK_RATE */
static int drime4_pwm_config(struct pwm_chip *chip, int duty_ns, int period_ns)
{
	struct drime4_pwm_device *pwm = to_drime4_pwm_device(chip);
	unsigned long long c;
	unsigned long period_cycles;
	unsigned long period, duty;
	unsigned long prescale;
	unsigned long flags;

	if (pwm == NULL || period_ns == 0 || duty_ns > period_ns)
		return -EINVAL;

	if (period_ns == pwm->period_ns && duty_ns == pwm->duty_ns)
		return 0;

	c = clk_get_rate(pwm->clk);

	dev_dbg(pwm->dev, "config: duty_ns: %d, period_ns: %d (clkrate %lld)\n",
		duty_ns, period_ns, c);

	c = c * period_ns;
	do_div(c, NS_IN_HZ);
	period_cycles = c;

	if (period_cycles < 1)
		period_cycles = 1;
	prescale = (period_cycles - 1) / 65536;
	period = period_cycles / (prescale + 1);

	if (prescale > 65536)
		return -EINVAL;
	c = (unsigned long long)period * duty_ns;
	do_div(c, period_ns);
	duty = c;

	pwm->period_ns = period_ns;
	pwm->duty_ns = duty_ns;

	dev_dbg(pwm->dev, "config: prescale: %lu duty: %lu, period: %lu\n",
		prescale, duty, period);

	local_irq_save(flags);
	__raw_writel(prescale << 16, pwm->mmio_base + DRIME4_PWM_PRESCALE);
	__raw_writel(duty << 16 | period, pwm->mmio_base + DRIME4_PWM_CYCLE1);
	local_irq_restore(flags);

	return 0;
}

static struct pwm_ops drime4_pwm_ops = {
	.enable = drime4_pwm_enable,
	.disable = drime4_pwm_disable,
	.config = drime4_pwm_config,
	.owner = THIS_MODULE,
};

static int drime4_pwm_probe(struct platform_device *pdev)
{
	struct drime4_pwm_device *pwm = NULL;
	struct resource *r;
	int ret = 0;

	pwm = devm_kzalloc(&pdev->dev, sizeof(struct drime4_pwm_device),
			GFP_KERNEL);
	if (pwm == NULL) {
		dev_err(&pdev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	pwm->dev = &pdev->dev;

	platform_set_drvdata(pdev, pwm);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r)
		return -ENODEV;
	r = devm_request_mem_region(&pdev->dev, r->start,
		resource_size(r), pdev->name);
	if (!r)
		return -EBUSY;
	pwm->mmio_base = devm_ioremap(&pdev->dev, r->start, resource_size(r));
	if (!pwm->mmio_base) {
		dev_err(&pdev->dev, "failed to ioremap() registers\n");
		return -ENOMEM;
	}

	if (pdev->id_entry->driver_data == DRIME4_PWM_CORE) {
		dev_dbg(&pdev->dev, "pwm core probe\n");
		/* TODO: Something */
	} else {
		struct drime4_pwm_device *parent_pwm;
		parent_pwm = dev_get_drvdata(pdev->dev.parent);
		if (!parent_pwm) {
			dev_err(&pdev->dev, "no parent driver data\n");
			return -EINVAL;
		}
		pwm->enable_base = parent_pwm->mmio_base;
		pwm->chip.ops = &drime4_pwm_ops;
		pwm->chip.pwm_id = pdev->id;

		pwm->clk = clk_get(&pdev->dev, "pwm");
		if (IS_ERR(pwm->clk)) {
			dev_err(&pdev->dev, "failed to get clock\n");
			return PTR_ERR(pwm->clk);
		}

		ret = pwmchip_add(&pwm->chip);
		if (ret) {
			dev_err(&pdev->dev, "failed to add pwm chip\n");
			clk_put(pwm->clk);
			return ret;
		}
		dev_dbg(&pdev->dev, "pwm probe pwm_id:%d\n",
			pwm->chip.pwm_id);
	}
	return 0;
}

static int __devexit drime4_pwm_remove(struct platform_device *pdev)
{
	struct drime4_pwm_device *pwm = platform_get_drvdata(pdev);
	int ret;

	if (pdev->id_entry->driver_data == DRIME4_PWM_CORE) {
		/* TODO: Remove pwm core */
		return 0;
	}
	ret = pwmchip_remove(&pwm->chip);
	if (ret)
		return ret;
	clk_put(pwm->clk);
	return 0;
}

static struct platform_device_id drime4_pwm_driver_ids[] = {
	{
		.name = "drime4-pwm-core",
		.driver_data = DRIME4_PWM_CORE,
	}, {
		.name = "drime4-pwm",
		.driver_data = DRIME4_PWM,
	}, { },
};
MODULE_DEVICE_TABLE(platform, drime4_pwm_driver_ids);

static struct platform_driver drime4_pwm_driver = {
	.driver		= {
		.name	= "drime4-pwm",
	},
	.probe		= drime4_pwm_probe,
	.remove		= __devexit_p(drime4_pwm_remove),
	.id_table	= drime4_pwm_driver_ids,
};

static int __init drime4_pwm_init(void)
{
	return platform_driver_register(&drime4_pwm_driver);
}
arch_initcall(drime4_pwm_init);

static void __exit drime4_pwm_exit(void)
{
	platform_driver_unregister(&drime4_pwm_driver);
}
module_exit(drime4_pwm_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Chanho Park <chanho61.park <at> samsung.com>");
