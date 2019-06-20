/*
 * Copyright (c) 2011 Samsung Electronics
 *	Kyuchun Han <kyuchun.han@samsung.com>
 *
 * GPIO_ioctl Device Driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <mach/map.h>
#include <linux/irq.h>
#include <linux/pinctrl/consumer.h>
#include <mach/irqs.h>
#include <mach/gpio.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/poll.h>

#include <linux/mm.h>
#include <linux/gpio_ioctl.h>

#define GPIO_MODULE_NAME		"gpiodev"

struct gpio_io_dev_data {
	dev_t devt;
	struct device *dev;
	unsigned int phy_addr;
	unsigned int phy_size;
	struct list_head device_entry;
	wait_queue_head_t gpio_wq;
	struct work_struct int_work;
	unsigned int shandler_num;
	struct pinctrl *pmx;
	struct pinctrl_state	*pins_default;
};

struct gpio_info {
	unsigned int phy_start;
	unsigned int phy_end;
};

static LIST_HEAD(device_list);


static void gpio_io_get_phy(struct gpio_io_dev_data *data,
		struct gpio_get_reg_info *gpio_info)
{
	gpio_info->phys_addr = data->phy_addr;
	gpio_info->phys_size = data->phy_size;

}

static int gpio_io_open(struct inode *inode, struct file *filp)
{
	struct gpio_io_dev_data *data;
	int status = -1;
	list_for_each_entry(data, &device_list, device_entry) {
		if (data->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}
	if (status == 0) {
		filp->private_data = data;
		nonseekable_open(inode, filp);
	}

	return status;
}

static void int_work(struct work_struct *work)
{
	struct gpio_io_dev_data *gpiodev;

	gpiodev = container_of(work, struct gpio_io_dev_data, int_work);
	wake_up_interruptible(&gpiodev->gpio_wq);
}




static irqreturn_t gpio_int_isr(int irq, void *data)
{
	struct gpio_io_dev_data *gpiodata = data;

	wake_up_interruptible(&gpiodata->gpio_wq);
/*	schedule_work(&gpiodata->int_work);*/
	return IRQ_HANDLED;
}


static int request_gpio_int(struct gpio_int_info *int_info,
		void *data)
{
	int irq, error;
	unsigned int gpio_pin = 0;

	if (int_info == NULL)
		return;

	gpio_pin = (int_info->group * 8) + int_info->pinnum;

	irq = gpio_to_irq(gpio_pin);


	if (irq < 0) {
		error = irq;
		printk(KERN_ERR "Unable to get irq number\n");
		goto fail;
	}
	error = request_irq(irq, gpio_int_isr, IRQF_DISABLED, "", data);
	if (error) {
		printk(KERN_ERR "Unable to request irq\n");
		goto fail;
	}
	disable_irq(irq);
	return 0;

fail:
	return error;
}

static int gpio_int_type(struct gpio_int_info *int_info,
		void *data)
{
	int irq, error;
	unsigned int gpio_pin = 0;

	if (int_info == NULL)
		return;

	gpio_pin = (int_info->group * 8) + int_info->pinnum;

	irq = gpio_to_irq(gpio_pin);

	error = irq_set_irq_type(irq, int_info->enable);
	return error;
}

static void gpio_int_sel(struct gpio_int_info *int_info,
		void *data)
{
	int irq;
	unsigned int gpio_pin;

	gpio_pin = 0;

	if (int_info == NULL)
		return;

	gpio_pin = (int_info->group * 8) + int_info->pinnum;

	irq = gpio_to_irq(gpio_pin);

	if (int_info->enable)
		disable_irq(irq);
	else
		enable_irq(irq);
}


static void gpio_shandler_set(void *data, unsigned int pin_num)
{
	struct gpio_io_dev_data *gpiodata = data;
	gpiodata->shandler_num &= ~(1<<pin_num);
}

static int gpio_io_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t gpio_io_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	return 0;
}

static ssize_t gpio_io_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	return 0;
}

static long gpio_io_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	u32 size, ret;
	unsigned int value;

	struct gpio_int_info int_info;
	struct gpio_get_reg_info gpio_info;
	struct gpio_io_dev_data *data =
			(struct gpio_io_dev_data *) filp->private_data;
	size = _IOC_SIZE(cmd);

	value = 0;

	switch (cmd) {
	case GPIO_PHY_INFO:
		gpio_io_get_phy(data, &gpio_info);
		ret = copy_to_user((void *) arg, (const void *) &gpio_info, size);
		break;
	case GPIO_INT_SET:
		ret = copy_from_user((void *) &int_info, (const void *) arg, size);
		if (ret < 0)
			return ret;
		request_gpio_int(&int_info, data);
		break;

	case GPIO_INT_SELECT:
		ret = copy_from_user((void *) &int_info, (const void *) arg, size);
		if (ret < 0)
			return ret;
		gpio_int_sel(&int_info, data);
		break;
	case	GPIO_INT_TYPE:
		ret = copy_from_user((void *) &int_info, (const void *) arg, size);
		if (ret < 0)
			return ret;
		gpio_int_type(&int_info, data);
		break;

	case GPIO_PAD_SET:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		if (ret < 0)
			return ret;
		pinctrl_request_gpio(value);
		break;

	case GPIO_PAD_FREE:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		if (ret < 0)
			return ret;

		pinctrl_free_gpio(value);
/*		
		data->pins_default = pinctrl_lookup_state(data->pmx, PINCTRL_STATE_DEFAULT);
		pinctrl_select_state(data->pmx, data->pins_default);
*/
		break;

	default:
		break;
	}
	return 0;
}

int gpio_io_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size = 0;

	size = (vma->vm_end - vma->vm_start);

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
			vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}


static unsigned int gpio_comm_poll(struct file *filp,
				   struct poll_table_struct *wait)
{
	struct gpio_io_dev_data	*gpiodev;
	unsigned int mask = 0;
	gpiodev = filp->private_data;
	interruptible_sleep_on(&gpiodev->gpio_wq);
	mask |= POLLIN | POLLRDNORM;
	return mask;
}

const struct file_operations gpio_io_ops = { .open = gpio_io_open,
		.release = gpio_io_release, .read = gpio_io_read,
		.write = gpio_io_write, .unlocked_ioctl = gpio_io_ioctl,
		.mmap = gpio_io_mmap,
		.poll    = gpio_comm_poll};

static void gpiodev_set(struct miscdevice *gpiodev, const char *fmt, ...)
{
	va_list vargs;
	va_start(vargs, fmt);
	gpiodev->name = kvasprintf(GFP_KERNEL, fmt, vargs);
	gpiodev->minor = MISC_DYNAMIC_MINOR;
	gpiodev->fops = &gpio_io_ops;
}


static int gpio_io_probe(struct platform_device *pdev)
{
	int ret;
	struct gpio_io_dev_data *gpio_data;
	struct miscdevice *gpiodev;
	struct gpio_info *g_data = pdev->dev.platform_data;

	gpiodev = kzalloc(sizeof(*gpiodev), GFP_KERNEL);
	if (!gpiodev) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_allocdev;
	}
	gpiodev_set(gpiodev, "gpiodev.%d", pdev->id);

	gpio_data = kzalloc(sizeof(*gpio_data), GFP_KERNEL);

	if (!gpio_data) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	gpio_data->dev = &pdev->dev;
	gpio_data->phy_addr = g_data->phy_start;
	gpio_data->phy_size = g_data->phy_end - g_data->phy_start + 1;

	gpio_data->shandler_num = 0xFF;


	gpio_data->pmx = devm_pinctrl_get(&pdev->dev);
	gpio_data->pins_default = pinctrl_lookup_state(gpio_data->pmx, PINCTRL_STATE_DEFAULT);

	INIT_LIST_HEAD(&gpio_data->device_entry);
	init_waitqueue_head(&gpio_data->gpio_wq);

	/*INIT_WORK(&gpio_data->int_work, int_work);*/
	ret = misc_register(gpiodev);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register gpio io ctrl\n");
		goto err_bl;
	}
	gpio_data->devt = MKDEV(10, gpiodev->minor);
	list_add(&gpio_data->device_entry, &device_list);
	platform_set_drvdata(pdev, gpio_data);
	return 0;

err_bl:
	kfree(gpio_data);
err_alloc:
	kfree(gpiodev);
err_allocdev:
	return ret;
}

#define gpio_io_suspend	NULL
#define gpio_io_resume	NULL

static struct platform_driver
		gpio_io_driver = { .driver = {
			.name = GPIO_MODULE_NAME,
			.owner = THIS_MODULE,
		}, .probe = gpio_io_probe, .suspend = gpio_io_suspend,
				.resume = gpio_io_resume, };

static int __init gpio_io_init(void)
{
	return platform_driver_register(&gpio_io_driver);
}
module_init(gpio_io_init);

static void __exit gpio_io_exit(void)
{
	platform_driver_unregister(&gpio_io_driver);
}
module_exit(gpio_io_exit);

MODULE_DESCRIPTION("gpio io based Test Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:gpio_io");

