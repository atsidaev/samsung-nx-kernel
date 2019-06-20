/**
 * @file int_str_ctrl.c
 * @brief flash driver
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <mach/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinmux.h>
#include <mach/gpio.h>
#include <linux/fb.h>
#include <linux/int_str_ctrl.h>

struct miscdevice *int_str_dev;

#define PIN_LEVEL_HIGH 	1
#define PIN_LEVEL_LOW 		0

#define PIN_DIR_IN 		0
#define PIN_DIR_OUT 		1


struct intstr_data {
	dev_t			devt;
	unsigned int condition;
	int irq;
	struct completion done;
};

static struct intstr_data *gstrdev;


static int int_str_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int int_str_release(struct inode *inode, struct file *filp)
{
	return 0;
}


static int int_str_change_init(void)
{
	int err = 0;
	err = gpio_request(GPIO_STR_CHANGE, "");
	err = gpio_direction_output(GPIO_STR_CHANGE, PIN_DIR_OUT);
	err = gpio_export(GPIO_STR_CHANGE, true);
	gpio_set_value(GPIO_STR_CHANGE, PIN_LEVEL_LOW);
	return err;
}


static int int_str_ready_init(void)
{
	int err = 0;
	gpio_request(GPIO_STR_READY, "");
	gpio_direction_input(GPIO_STR_READY);
	gpio_export(GPIO_STR_READY, true);
	return err;
}

static int int_str_trg_init(void)
{
	int err = 0;
	err = gpio_request(GPIO_INT_STR_TRG, "");
	err = gpio_direction_output(GPIO_INT_STR_TRG, PIN_DIR_OUT);
	err = gpio_export(GPIO_INT_STR_TRG, true);
	gpio_set_value(GPIO_INT_STR_TRG, PIN_LEVEL_LOW);
	return err;
}

static int int_str_up_init(void)
{
	int err = 0;
	err = gpio_request(GPIO_PU_CNT, "");
	err = gpio_direction_output(GPIO_PU_CNT, PIN_DIR_OUT);
	err = gpio_export(GPIO_PU_CNT, true);
	gpio_set_value(GPIO_PU_CNT, PIN_LEVEL_LOW);
	return err;
}

static int int_str_det_init(void)
{
	int err = 0;
	err = gpio_request(GPIO_INT_STR_DET, "");
	err = gpio_direction_input(GPIO_INT_STR_DET);
	err = gpio_export(GPIO_INT_STR_DET, true);
	return err;
}

static int int_str_gpio_init(unsigned int set_val)
{
	int err = 0;
	if (set_val) {

		err = int_str_change_init();
		err = int_str_ready_init();
		err = int_str_trg_init();
		err = int_str_up_init();
		err = int_str_det_init();
	} else {
		gpio_free(GPIO_STR_CHANGE);
		gpio_free(GPIO_STR_READY);
		gpio_free(GPIO_INT_STR_TRG);
		gpio_free(GPIO_PU_CNT);
		gpio_free(GPIO_INT_STR_DET);
	}
	return err;
}

static int int_str_start(void)
{
	gpio_set_value(GPIO_STR_CHANGE, PIN_LEVEL_HIGH);
	wait_for_completion_interruptible(&gstrdev->done);

/*
	if (!wait_for_completion_timeout(&gstrdev->done,
			msecs_to_jiffies(2000))) {
		gpio_set_value(GPIO_STR_CHANGE, PIN_LEVEL_LOW);
		return -1;
	}
*/

	gpio_set_value(GPIO_STR_CHANGE, PIN_LEVEL_LOW);
	return 0;
}


static long int_str_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned int size;
	int ret = 0;
	unsigned int set_val;
	unsigned int ret_val;

	size = _IOC_SIZE(cmd);
	switch (cmd) {
	case INT_STR_SET_PIN:
		ret = copy_from_user((void *)&set_val, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		ret = int_str_gpio_init(set_val);
		break;

	case INT_STR_SET_INT:
		enable_irq(gstrdev->irq);
		break;

	case INT_STR_SET_CHARGE:
		ret_val = int_str_start();
		ret = copy_to_user((void *)arg, (const void *)&ret_val, size);
		break;

	case INT_STR_TRG:
		gpio_set_value(GPIO_INT_STR_TRG, PIN_LEVEL_HIGH);
		break;

	case INT_STR_TRG_READY:
		gpio_set_value(GPIO_INT_STR_TRG, PIN_LEVEL_LOW);
		break;

	case INT_STR_UP:
		gpio_direction_output(GPIO_PU_CNT, PIN_DIR_OUT);
		gpio_set_value(GPIO_PU_CNT, PIN_LEVEL_HIGH);
		mdelay(1);
		gpio_set_value(GPIO_PU_CNT, PIN_LEVEL_LOW);
		break;

	case INT_STR_UP_CHECK:
		ret_val = gpio_get_value(GPIO_INT_STR_DET);
		ret = copy_to_user((void *)arg, (const void *)&ret_val, size);

		break;
	default:
		break;
	}
	return ret;
}


static irqreturn_t int_str_isr(int irq, void *data)
{
	complete(&gstrdev->done);
	return IRQ_HANDLED;
}


const struct file_operations int_str_ops = { .open = int_str_open,
		.release = int_str_release, .unlocked_ioctl = int_str_ioctl };

static void int_str_ops_set(struct miscdevice *mdmadev, const char *fmt, ...)
{
	va_list vargs;
	va_start(vargs, fmt);
	mdmadev->name = kvasprintf(GFP_KERNEL, fmt, vargs);
	mdmadev->minor = MISC_DYNAMIC_MINOR;
	mdmadev->fops = &int_str_ops;
}


static int int_str_misc_set(void)
{
	int ret;
	int_str_dev = kzalloc(sizeof(*int_str_dev), GFP_KERNEL);

	if (!int_str_dev) {
		printk("no memory for state\n");
		return -ENOMEM;;
	}
	int_str_ops_set(int_str_dev, "int_str");
	ret = misc_register(int_str_dev);
	if (ret < 0) {
		printk("failed to register MDMA ioctl\n");
		kfree(int_str_dev);
		return -ENODEV;
	}

	return 0;
}



static int int_str_init_setup(struct platform_device *pdev)
{
	int err;
	int irq_flg;
	gstrdev = kzalloc(sizeof(*gstrdev), GFP_KERNEL);
	if (gstrdev == NULL)
		return -1;

	gstrdev->irq = gpio_to_irq(GPIO_STR_READY);

	init_completion(&gstrdev->done);

	irq_flg = IRQF_TRIGGER_FALLING;



	err = request_irq(gstrdev->irq, int_str_isr, irq_flg, pdev->name, gstrdev);
	disable_irq_nosync(gpio_to_irq(GPIO_STR_READY));
	if (err < 0) {
		printk(KERN_ERR "%s: INT STR setup has been failed", __func__);
		goto err_irq;
	}

	err = int_str_misc_set();
	if (err < 0) {
			printk(KERN_ERR "%s: Misc register failed", __func__);
			goto err_irq;
		}


	pinctrl_request_gpio(GPIO_STR_CHANGE);
	pinctrl_request_gpio(GPIO_STR_READY);
	pinctrl_request_gpio(GPIO_INT_STR_TRG);
	pinctrl_request_gpio(GPIO_PU_CNT);
	pinctrl_request_gpio(GPIO_INT_STR_DET);

	int_str_gpio_init(1);


	return 0;

err_irq:
	kfree(gstrdev);
	return err;
}


static int int_str_probe(struct platform_device *pdev)
{
	return int_str_init_setup(pdev);
}

static int int_str_resume(struct platform_device *pdev)
{
	pinctrl_request_gpio(GPIO_STR_CHANGE);
	pinctrl_request_gpio(GPIO_STR_READY);
	pinctrl_request_gpio(GPIO_INT_STR_TRG);
	pinctrl_request_gpio(GPIO_PU_CNT);
	pinctrl_request_gpio(GPIO_INT_STR_DET);
	int_str_gpio_init(1);
	return 0;
}


static int int_str_remove(struct platform_device *pdev)
{
	return 0;
}

static int int_str_suspend(struct platform_device *pdev, pm_message_t state)
{
	pinctrl_free_gpio(GPIO_STR_CHANGE);
	pinctrl_free_gpio(GPIO_STR_READY);
	pinctrl_free_gpio(GPIO_INT_STR_TRG);
	pinctrl_free_gpio(GPIO_PU_CNT);
	pinctrl_free_gpio(GPIO_INT_STR_DET);
	return 0;
}


static struct platform_driver int_str_driver = {
		.driver = {
				.name = "intstr",
				.owner = THIS_MODULE,
		},
		.probe = int_str_probe, .remove = int_str_remove,
		.suspend = int_str_suspend, .resume = int_str_resume, };


static int __init int_str_init(void)
{
	return platform_driver_register(&int_str_driver);
}

static void __exit int_str_exit(void)
{
	misc_deregister(int_str_dev);
	platform_driver_unregister(&int_str_driver);
}


module_init(int_str_init);
module_exit(int_str_exit);


MODULE_DESCRIPTION("Driver for Internal STR");
MODULE_AUTHOR("kyuchun han <kyuchun.han@samsung.com>");
MODULE_VERSION("0.2");
MODULE_LICENSE("GPL");
