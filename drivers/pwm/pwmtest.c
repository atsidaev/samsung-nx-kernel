/*
 * PWM test module
 *
 * Copyright (C) 2011 SAMSUNG Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/pwm.h>
#include <linux/delay.h>

static unsigned int test_id;
module_param(test_id, uint, S_IRUGO);

static unsigned int period_ns = 1000000;
module_param(period_ns, uint, S_IRUGO);

static unsigned int duty_ns = 500000;
module_param(duty_ns, uint, S_IRUGO);

struct pwm_device *pwm;

static int pwmtest_start(void)
{
	int ret = 0;
	ret = pwm_config(pwm, duty_ns, period_ns);
	if (ret)
		printk(KERN_INFO "failed to configure pwm\n");
	pwm_enable(pwm);
	return ret;
}

static void pwmtest_stop(void)
{
	pwm_disable(pwm);
}

static int __init pwmtest_init(void)
{
	pwm = pwm_request(test_id, "pwm_test");
	if (!pwm) {
		printk(KERN_ERR "failed to retrive pwm device\n");
		return -ENODEV;
	}
	if (pwmtest_start()) {
		printk(KERN_INFO "pwm test failed\n");
		return -EIO;
	}
	return 0;
}
late_initcall(pwmtest_init);

static void __exit pwmtest_exit(void)
{
	pwmtest_stop();
	if (pwm)
		pwm_free(pwm);
}
module_exit(pwmtest_exit);

MODULE_AUTHOR("Chanho Park <chanho61.park@samsung.com>");
MODULE_LICENSE("GPL v2");
