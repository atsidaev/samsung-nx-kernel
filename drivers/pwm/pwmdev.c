/* linux/drivers/pwm/pwmdev.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	Kyuchun Han <kyuchun.han@samsung.com>
 *
 * PWM IOCTL Device Driver
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
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/pwmdev_conf.h>
#include <linux/pwmdev.h>

#include <linux/d4_ht_pwm.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <mach/map.h>

#include <linux/gpio.h>
#include <mach/gpio.h>
#include <mach/pwm_map_d4_nx300.h>
#include <linux/pinctrl/pinmux.h>
#define PWMDEV_CORE_MODULE_NAME		"pwmdev"

#if 1
#define PWM_DEBUG_MSG(fmt, arg...)		printk("pwmdev: " fmt, ##arg)
#else
#define PWM_DEBUG_MSG(fmt, arg...)
#endif

struct pwm_data {
	dev_t devt;
	struct ht_pwm_device *pwm_1;
	struct ht_pwm_device *pwm_2;
	struct device *dev;
	unsigned int period;
	int pwmch;
	int pwmsch;
	struct list_head device_entry;
	wait_queue_head_t alarm_wq;
	unsigned int trg_value;
	unsigned int alarms;
	struct work_struct int_work;
};

static struct pwm_data *alarmlist[10];
static LIST_HEAD(device_list);


static void int_work(struct work_struct *work)
{
	struct pwm_data *pwmdev;

	pwmdev = container_of(work, struct pwm_data, int_work);

	wake_up_interruptible(&pwmdev->alarm_wq);

}

static void pwm_usr_handler(int id)
{
	struct pwm_data *pwmdata;
	struct platform_pwm_dev_data *pwmdev;

	pwmdata = alarmlist[id];

	pwmdev = pwmdata->dev->platform_data;

	gpio_set_value(pwmdev->pin_num, 0);

	wake_up_interruptible(&pwmdata->alarm_wq);
}

static void pwm_usr_shandler(int id)
{
	struct pwm_data *pwmdata;
	pwmdata = alarmlist[id];
	wake_up_interruptible(&pwmdata->alarm_wq);
}

static ssize_t pwmdev_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	return 0;
}

/* Write-only message with current device setup */
static ssize_t pwmdev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	return 0;
}



static void pwmdev_check_sch(struct pwm_data *data, struct ht_pwm_phys_reg_info *info)
{
	struct platform_pwm_dev_data *pwmdata = data->dev->platform_data;

	info->schannel = pwmdata->pwm_sch;

	if ((pwmdata->pwm_sch != PWMDEV_NULL) &&
			(pwmdata->pin_num != DRIME4_GPIO_END)) {
		alarmlist[pwmdata->pwm_sch] = data;
	}
}

static void pwmdev_swap_check(struct pwm_data *data,  struct ht_pwm_phys_reg_info *info)
{
	struct platform_pwm_dev_data *pwmdata = data->dev->platform_data;

	if (pwmdata->pin_num != DRIME4_GPIO_END) {
		info->swap_check = 0;
		gpio_request(pwmdata->pin_num, "");
	} else {
		info->swap_check = -1;
	}

	info->swap_check = -1;
}


static long pwmdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pwm_data *data = (struct pwm_data *) filp->private_data;

	u32 size;
	u32 ret;

	struct ht_pwm_conf_info pwm_setup;
	struct ht_pwm_mconf_info pwm_msetup;
	struct ht_pwm_phys_reg_info pwm_phyinfo;
	unsigned int value;
	int swap;

	value = 0;
	size = _IOC_SIZE(cmd);

	switch (cmd) {
	case PWM_SIOCTL_SET:
		ret = copy_from_user((void *) &pwm_setup, (const void *) arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		d4_ht_pwm_config(data->pwm_1, &pwm_setup);
		break;

	case PWM_SIOCTL_ENABLE:
		d4_ht_pwm_enable(data->pwm_1);
		break;

	case PWM_SIOCTL_DISABLE:
		d4_ht_pwm_disable(data->pwm_1);
		break;

	case PWM_SIOCTL_INTEN:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		d4_ht_pwm_int_func_set(data->pwm_1, pwm_usr_handler);
		d4_ht_pwm_int_enalbe(data->pwm_1, value);
		break;

	case PWM_SIOCTL_EXTEN:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		d4_ht_pwm_extinput_set(data->pwm_1, value);
		break;

	case PWM_MIOCTL_ENABLE:
		d4_ht_pwm_enable(data->pwm_1);
		d4_ht_pwm_enable(data->pwm_2);
		break;

	case PWM_MIOCTL_DISABLE:
		d4_ht_pwm_disable(data->pwm_1);
		d4_ht_pwm_disable(data->pwm_2);
		break;

	case PWM_MIOCTL_INTEN:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		if (value != NOT_USED) {
			d4_ht_pwm_int_func_set(data->pwm_1, pwm_usr_handler);
			d4_ht_pwm_int_func_set(data->pwm_2, pwm_usr_handler);
			d4_ht_pwm_int_enalbe(data->pwm_1, value);
			d4_ht_pwm_int_enalbe(data->pwm_2, value);
		} else {
			d4_ht_pwm_int_enalbe(data->pwm_1, value);
			d4_ht_pwm_int_enalbe(data->pwm_2, value);
		}
		break;
	case PWM_MIOCTL_EXTEN:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		d4_ht_pwm_extinput_set(data->pwm_1, value);
		d4_ht_pwm_extinput_set(data->pwm_2, value);
		break;
	case PWM_MIOCTL_SET:
		ret = copy_from_user((void *) &pwm_msetup, (const void *) arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		d4_ht_pwm_config(data->pwm_1, &pwm_msetup.mch_config);
		d4_ht_pwm_config(data->pwm_2, &pwm_msetup.sch_config);
		break;

	case PWM_SIOCTL_CLEAR:
		d4_ht_pwm_clear(data->pwm_1);
		break;

	case PWM_MIOCTL_CLEAR:
		d4_ht_pwm_clear(data->pwm_1);
		d4_ht_pwm_clear(data->pwm_2);
		break;

	case PWM_EXT_TRG:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		gpio_request(data->trg_value, "");
		gpio_direction_output(data->trg_value, !value);
		gpio_set_value(data->trg_value, value);
		break;

	case PWM_PHY_INFO:
		d4_pwm_get_phy_info(data->pwm_1, &pwm_phyinfo);
		pwmdev_check_sch(data, &pwm_phyinfo);
		pwmdev_swap_check(data, &pwm_phyinfo);
		copy_to_user((void *) arg, (const void *) &pwm_phyinfo, size);
		break;

	case PWM_INT_MFUNC_SET:
		d4_ht_pwm_int_func_set(data->pwm_1, pwm_usr_handler);
		d4_ht_pwm_int_func_set(data->pwm_2, pwm_usr_handler);
		break;
	case PWM_INT_SFUNC_SET:
		d4_ht_pwm_int_func_set(data->pwm_1, pwm_usr_shandler);
		break;

	case PWM_PAD_CTRL:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		d4_ht_pwm_pad_set(data->pwm_1, value);
		break;


	default:
		break;
	}
	return 0;
}

static int pwmdev_open(struct inode *inode, struct file *filp)
{
	struct pwm_data *data;
	int status = -1;
	list_for_each_entry(data, &device_list, device_entry) {
		if (data->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}

	if (status == 0) {
		nonseekable_open(inode, filp);

		filp->private_data = data;

		alarmlist[data->pwmch] = data;
	}
	return status;

}

static int pwmdev_release(struct inode *inode, struct file *filp)
{
	return 0;
}


static unsigned int pwmdev_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	struct pwm_data *data = (struct pwm_data *) file->private_data;
	interruptible_sleep_on(&data->alarm_wq);
	mask |= POLLIN | POLLRDNORM;

	return mask;
}

int pwmdev_io_mmap(struct file *file, struct vm_area_struct *vma)
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

bool pwmdev_auto_configue(struct platform_device *pdev)
{
	int i = 0;
	bool bl_retval = false;
	struct platform_pwm_dev_data *pwmdata = pdev->dev.platform_data;

	for(i = 0; i < PWMDEV_MAX; i++)
	{
		if(pwm_pin_map[i].pwm_sch == pdev->id)
		{
			pdev->id = PWMDEV_NULL;
			break;
		}
	}
	if(pdev->id != PWMDEV_NULL)
	{
		if (pwmdata->pwm_sch == PWMDEV_AUTO) 
		{
			pwmdata->pwm_sch = pwm_pin_map[pdev->id].pwm_sch;
			pwmdata->pin_num = pwm_pin_map[pdev->id].pin_num;
		}
	}
	return bl_retval;
}

const struct file_operations pwmdev_ops = { .open = pwmdev_open,
		.release = pwmdev_release, .read = pwmdev_read, .write = pwmdev_write,
		.unlocked_ioctl = pwmdev_ioctl, .poll = pwmdev_poll, .mmap = pwmdev_io_mmap };

static void pwmdev_set(struct miscdevice *pwmdev, const char *fmt, ...)
{
	va_list vargs;
	va_start(vargs, fmt);
	pwmdev->name = kvasprintf(GFP_KERNEL, fmt, vargs);
	pwmdev->minor = MISC_DYNAMIC_MINOR;
	pwmdev->fops = &pwmdev_ops;
}

static int pwm_dev_probe(struct platform_device *pdev)
{
	struct pwm_data *data;
	struct miscdevice *pwmdev;
	struct platform_pwm_dev_data *pwmdata = pdev->dev.platform_data;
	int ret;

	/*allocate Auto configue*/
	PWM_DEBUG_MSG("pwm_dev_probe() : id=%d\n", pdev->id);

	pwmdev_auto_configue(pdev);

	if (pdev->id == PWMDEV_NULL) {
		ret = 0;
	} else {
		pwmdev = kzalloc(sizeof(*pwmdev), GFP_KERNEL);

		if (!pwmdev) {
			dev_err(&pdev->dev, "no memory for state\n");
			ret = -ENOMEM;
			goto err_allocdev;
		}
		pwmdev_set(pwmdev, "pwmdev.%d", pdev->id);

		data = kzalloc(sizeof(*data), GFP_KERNEL);
		if (!data) {
			dev_err(&pdev->dev, "no memory for state\n");
			ret = -ENOMEM;
			goto err_allocdata;
		}

		data->dev = &pdev->dev;

		INIT_LIST_HEAD(&data->device_entry);

		init_waitqueue_head(&data->alarm_wq);

	/*	INIT_WORK(&data->int_work, int_work);*/

		data->pwm_1 = d4_ht_pwm_request(pdev->id, "pwmdev");
		data->pwmch = pdev->id;
		data->trg_value = pwmdata->input_trg_port;
		if (pwmdata->pwm_sch != PWMDEV_NULL) {
			data->pwm_2 = d4_ht_pwm_request(pwmdata->pwm_sch, "pwmdev");
			data->pwmsch = pwmdata->pwm_sch;
		}
		ret = misc_register(pwmdev);
		if (ret < 0) {
			dev_err(&pdev->dev, "misc_register fail\n");
			goto err_pwm;
		}

		data->devt = MKDEV(10, pwmdev->minor);
		list_add(&data->device_entry, &device_list);
		platform_set_drvdata(pdev, data);
	}
	return ret;

err_pwm: kfree(data);
err_allocdata: kfree(pwmdev);
err_allocdev:

	return ret;
}

static int pwm_dev_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_PM
static int pwm_dev_suspend(struct platform_device *pdev,
		pm_message_t state)
{
	return 0;
}

static int pwm_dev_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define pwm_dev_suspend	NULL
#define pwm_dev_resume	NULL
#endif

static struct platform_driver pwm_dev_driver = { .driver = {
	.name = "pwmdev",
	.owner = THIS_MODULE,
}, .probe = pwm_dev_probe, .remove = pwm_dev_remove,
		.suspend = pwm_dev_suspend, .resume = pwm_dev_resume, };

static int __init pwm_dev_init(void)
{
	return platform_driver_register(&pwm_dev_driver);
}
module_init(pwm_dev_init);

static void __exit pwm_dev_exit(void)
{
	platform_driver_unregister(&pwm_dev_driver);
}
module_exit(pwm_dev_exit);

MODULE_DESCRIPTION("PWM based Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwmdev");

