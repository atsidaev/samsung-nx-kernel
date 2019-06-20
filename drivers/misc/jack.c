/*
 *  Jack Monitoring Interface
 *
 *  Copyright (C) 2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/jack.h>
#include <linux/slab.h>

#include <linux/pinctrl/consumer.h>
//#include <linux/pinctrl/machine.h>
#include <mach/gpio.h>
#include <linux/delay.h>

struct jack_data {
	struct jack_platform_data	*pdata;
};

static struct platform_device *jack_dev;

static void jack_set_data(struct jack_platform_data *pdata,
		const char *name, int value)
{
	if (!strcmp(name, "usb"))
		pdata->usb_online = value;
	else if (!strcmp(name, "charger"))
		pdata->charger_online = value;
	else if (!strcmp(name, "charger1"))
		pdata->charger1_online = value;
	else if (!strcmp(name, "hdmi"))
		pdata->hdmi_online = value;
	else if (!strcmp(name, "earjack"))
		pdata->earjack_online = value;
	else if (!strcmp(name, "earkey"))
		pdata->earkey_online = value;
	else if (!strcmp(name, "ums"))
		pdata->ums_online = value;
	else if (!strcmp(name, "cdrom"))
		pdata->cdrom_online = value;
	else if (!strcmp(name, "jig"))
		pdata->jig_online = value;
	else if (!strcmp(name, "host"))
		pdata->host_online = value;
	else if (!strcmp(name, "release"))
		pdata->release_online = value;
	else if (!strcmp(name, "mmc"))
		pdata->mmc_online = value;
    else if (!strcmp(name, "usb_discont"))
		pdata->usb_sp_online = value;
}

int jack_get_data(const char *name)
{
	struct jack_data *jack = platform_get_drvdata(jack_dev);

	/* static analysis bug fix 2012-09-10 Han Seungwon */
	if(jack == NULL)	return -EINVAL;
	/* static analysis bug fix 2012-09-10 Han Seungwon */

	if (!strcmp(name, "usb"))
		return jack->pdata->usb_online;
	else if (!strcmp(name, "charger"))
		return jack->pdata->charger_online;
	else if (!strcmp(name, "charger1"))
		return jack->pdata->charger1_online;
	else if (!strcmp(name, "hdmi"))
		return jack->pdata->hdmi_online;
	else if (!strcmp(name, "earjack"))
		return jack->pdata->earjack_online;
	else if (!strcmp(name, "earkey"))
		return jack->pdata->earkey_online;
	else if (!strcmp(name, "ums"))
		return jack->pdata->ums_online;
	else if (!strcmp(name, "cdrom"))
		return jack->pdata->cdrom_online;
	else if (!strcmp(name, "jig"))
		return jack->pdata->jig_online;
	else if (!strcmp(name, "host"))
		return jack->pdata->host_online;
	else if (!strcmp(name, "release"))
		return jack->pdata->release_online;
	else if (!strcmp(name, "mmc"))
		return jack->pdata->mmc_online;
    else if (!strcmp(name, "usb_discont"))
		return jack->pdata->usb_sp_online;

	return -EINVAL;
}
EXPORT_SYMBOL_GPL(jack_get_data);

void jack_event_handler(const char *name, int value)
{
	struct jack_data *jack = platform_get_drvdata(jack_dev);
	char env_str[32];

	char *envp[] = { env_str, NULL };

	/* static analysis bug fix 2012-09-10 Han Seungwon */
	if(jack == NULL)	return;
	/* static analysis bug fix 2012-09-10 Han Seungwon */

	jack_set_data(jack->pdata, name, value);

	sprintf(env_str, "CHGDET=%s", name);
	dev_info(&jack_dev->dev, "jack event %s\n", env_str);

	if (value == 1)  {
		 kobject_uevent_env(&jack_dev->dev.kobj, KOBJ_ADD, envp);
    }else { 
         kobject_uevent_env(&jack_dev->dev.kobj, KOBJ_REMOVE, envp);
    }
}
EXPORT_SYMBOL_GPL(jack_event_handler);

#define JACK_OUTPUT(name)							\
static ssize_t jack_show_##name(struct device *dev,		\
		struct device_attribute *attr, char *buf)			\
{													\
	struct jack_data *chip = dev_get_drvdata(dev);	\
	/* static analysis bug fix 2012-09-10 Han Seungwon */	\
	if(chip == NULL)	return -1;						\
	/* static analysis bug fix 2012-09-10 Han Seungwon */	\
	return sprintf(buf, "%d\n", chip->pdata->name);		\
}													\
static DEVICE_ATTR(name, S_IRUGO, jack_show_##name, NULL);

JACK_OUTPUT(usb_online);
JACK_OUTPUT(charger_online);
JACK_OUTPUT(charger1_online);
JACK_OUTPUT(hdmi_online);
JACK_OUTPUT(earjack_online);
JACK_OUTPUT(earkey_online);
JACK_OUTPUT(jig_online);
JACK_OUTPUT(host_online);
JACK_OUTPUT(release_online);
JACK_OUTPUT(mmc_online);
JACK_OUTPUT(usb_sp_online);


#ifdef CONFIG_USBSTARTSTOP_SYSFS
static int startstop_usb = 0;	// 0: usb stop, 1: usb_start
/* swaggu.han : sysfs for to show status of usb start/stop status
 *                Path (/sys/devices/platform/jack/UsbStartStop)
 */
static ssize_t UsbStartStop_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", startstop_usb);
}

/* swaggu.han : sysfs for to change status of usb start/stop
 *                Path (/sys/devices/platform/jack/UsbStartStop)
 */
static ssize_t UsbStartStop_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value = 0;
	int ret = 0;
	
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	ret = sscanf(buf, "%d", &value);

	if(ret == -1)
	{
		return -1;
	}
	/* static analysis bug fix 2012-09-10 Han Seungwon */

	if(value == startstop_usb)
	{
		return size;
	}

	switch(value) 
	{
		case 0:	/* usb stop */
			startstop_usb = 0;
			usb_drime4_ss_udc_run_stop(0);
			msleep(500);
			pinctrl_request_gpio(GPIO_USB_DET);
			ret = gpio_request(GPIO_USB_DET, "USB Detect");
			if (ret != 0)
			{
				return -1;
			}
			gpio_direction_output(GPIO_USB_DET, 0);

			gpio_free(GPIO_USB_DET);
			pinctrl_free_gpio(GPIO_USB_DET);
			
			break;
		case 1:	/* usb start */
			startstop_usb = 1;
			usb_drime4_ss_udc_run_stop(1);
			//msleep(500);
			
			pinctrl_request_gpio(GPIO_USB_DET);
			ret = gpio_request(GPIO_USB_DET, "USB Detect");
			if (ret != 0)
			{
				return -1;
			}

			gpio_direction_output(GPIO_USB_DET, 1);

			gpio_free(GPIO_USB_DET);
			pinctrl_free_gpio(GPIO_USB_DET);
			
			break;
		default:
			break;
	}

	return size;
}

static DEVICE_ATTR(UsbStartStop, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, UsbStartStop_show, UsbStartStop_store);
#endif

extern void change_hdmi_setting(unsigned long value);
extern unsigned long g_change_hdmi_value;
#define HDMI_HPD_GPIO	DRIME4_GPIO0(1)

static ssize_t show_change_hdmi(struct device *dev, struct device_attribute *attr, char *buf)
{
	int value;
	value = gpio_get_value(HDMI_HPD_GPIO);

	return sprintf(buf, "%d\n", value);
}

static ssize_t change_hdmi_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int ret;
	unsigned long value = 0;
	ret = sscanf(buf, "%d", &value);

	if (value > 2) {
		printk(KERN_ALERT"modify hdmi value error %d\n", value);
		return -1;
	}

	change_hdmi_setting(value);
	g_change_hdmi_value = value;
	return size;
}

#if defined(CONFIG_ALT_REBOOT)
extern unsigned long is_poweroff_status;
extern void kernel_restart(char *cmd);

static void watchdog_restart(unsigned long data)
{
	printk(KERN_ALERT"\n\n\nwatchdog restart \n\n\n");
	kernel_restart(NULL);
}

static unsigned int is_watchdog_working;
static struct timer_list watch_dog_timer;
static unsigned int watchdog_time;

static ssize_t show_watchdog_timer(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", watchdog_time);
}

static ssize_t set_watchdog_timer(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{

	int ret = 0;

	ret = sscanf(buf, "%d", &watchdog_time);

	if (is_watchdog_working) del_timer(&watch_dog_timer);

	if (watchdog_time == 0) {
		is_watchdog_working = 0;
		printk(KERN_ALERT"watchdog stopped\n");
	} else {
		setup_timer(&watch_dog_timer, watchdog_restart, 0);
		mod_timer(&watch_dog_timer, jiffies + msecs_to_jiffies(1000*watchdog_time));
		is_watchdog_working = 1;
		printk(KERN_ALERT"\n\nwatchdog setted %d\n\n", watchdog_time);
	}
	return size;
}

static ssize_t show_poweroff_enter(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", is_poweroff_status);
}

extern void reboot_exec(int sec);

static ssize_t set_poweroff_enter(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int ret = 0;
	
	// enable poweroff watchdog. (time: 5 sec)
	reboot_exec(5);

	ret = sscanf(buf, "%d", &is_poweroff_status);
	return size;
}

// for alt watchdog
static DEVICE_ATTR(set_watchdog, S_IRUGO | S_IWUGO | S_IRUSR | S_IWUSR, show_watchdog_timer, set_watchdog_timer);

// for power off logo
static DEVICE_ATTR(poweroff_enter, S_IRUGO | S_IWUGO | S_IRUSR | S_IWUSR, show_poweroff_enter, set_poweroff_enter);
#endif

static DEVICE_ATTR(change_hdmi_detection, S_IRUGO | S_IWUGO | S_IRUSR | S_IWUSR, show_change_hdmi, change_hdmi_store);

static int jack_device_init(struct jack_data *jack)
{
	struct jack_platform_data *pdata = jack->pdata;
	int ret;

	if (pdata->usb_online != -1)
		ret = device_create_file(&jack_dev->dev,
				&dev_attr_usb_online);
	if (pdata->charger_online != -1)
		ret = device_create_file(&jack_dev->dev,
				&dev_attr_charger_online);
	if (pdata->charger1_online != -1)
		ret = device_create_file(&jack_dev->dev,
				&dev_attr_charger1_online);
	if (pdata->hdmi_online != -1)
		ret = device_create_file(&jack_dev->dev,
				&dev_attr_hdmi_online);
	if (pdata->earjack_online != -1)
		ret = device_create_file(&jack_dev->dev,
				&dev_attr_earjack_online);
	if (pdata->earkey_online != -1)
		ret = device_create_file(&jack_dev->dev,
				&dev_attr_earkey_online);
	if (pdata->jig_online != -1)
		ret = device_create_file(&jack_dev->dev,
				&dev_attr_jig_online);
	if (pdata->host_online != -1)
		ret = device_create_file(&jack_dev->dev,
				&dev_attr_host_online);
	if (pdata->release_online != -1)
		ret = device_create_file(&jack_dev->dev,
				&dev_attr_release_online);
	if (pdata->mmc_online != -1)
		ret = device_create_file(&jack_dev->dev,
				&dev_attr_mmc_online);
    if (pdata->usb_sp_online != -1)
         ret = device_create_file(&jack_dev->dev,
                &dev_attr_usb_sp_online);

#ifdef CONFIG_USBSTARTSTOP_SYSFS
	ret = device_create_file(&jack_dev->dev, &dev_attr_UsbStartStop);
	startstop_usb = 0;
#endif

	ret = device_create_file(&jack_dev->dev, &dev_attr_change_hdmi_detection);

#if defined(CONFIG_ALT_REBOOT)
	ret = device_create_file(&jack_dev->dev, &dev_attr_set_watchdog);
	ret = device_create_file(&jack_dev->dev, &dev_attr_poweroff_enter);
#endif
	return 0;
}

static int __devinit jack_probe(struct platform_device *pdev)
{
	struct jack_platform_data *pdata = pdev->dev.platform_data;
	struct jack_data *jack;

	jack = kzalloc(sizeof(struct jack_data), GFP_KERNEL);
	if (!jack) {
		dev_err(&pdev->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, jack);
	jack_dev = pdev;
	jack->pdata = pdata;

	jack_device_init(jack);

	return 0;
}

static int __devexit jack_remove(struct platform_device *pdev)
{
	struct jack_platform_data *pdata = pdev->dev.platform_data;

	if (pdata->usb_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_usb_online);
	if (pdata->charger_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_charger_online);
	if (pdata->charger1_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_charger1_online);
	if (pdata->hdmi_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_hdmi_online);
	if (pdata->earjack_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_earjack_online);
	if (pdata->earkey_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_earkey_online);
	if (pdata->jig_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_jig_online);
	if (pdata->host_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_host_online);
	if (pdata->release_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_release_online);
	if (pdata->mmc_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_mmc_online);
	if (pdata->usb_sp_online != -1)
		device_remove_file(&jack_dev->dev, &dev_attr_usb_sp_online);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static int jack_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct jack_platform_data *pdata = pdev->dev.platform_data;
	struct jack_data *jack = platform_get_drvdata(jack_dev);
	int value = 0;

	if(jack != NULL){
		if (pdata->usb_online != -1)
			jack_set_data(jack->pdata, "usb", value);
		if (pdata->charger_online != -1)
			jack_set_data(jack->pdata, "charger", value);
		if (pdata->charger1_online != -1)
			jack_set_data(jack->pdata, "charger1", value);
		if (pdata->hdmi_online != -1)
			jack_set_data(jack->pdata, "hdmi", value);
		if (pdata->earjack_online != -1)
			jack_set_data(jack->pdata, "earjack", value);
		if (pdata->earkey_online != -1)
			jack_set_data(jack->pdata, "earkey", value);
		if (pdata->jig_online != -1)
			jack_set_data(jack->pdata, "jig", value);
		if (pdata->host_online != -1)
			jack_set_data(jack->pdata, "host", value);
		if (pdata->release_online != -1)
			jack_set_data(jack->pdata, "release", value);
		if (pdata->mmc_online != -1)
			jack_set_data(jack->pdata, "mmc", value);
		if (pdata->usb_sp_online != -1)
			jack_set_data(jack->pdata, "usb_discont", value);
	}

#if defined(CONFIG_ALT_REBOOT)
	is_poweroff_status = 0;
#endif
	return 0;
}

static struct platform_driver jack_driver = {
	.probe		= jack_probe,
	.remove		= __devexit_p(jack_remove),
	.suspend		= jack_suspend,
	.driver		= {
		.name	= "jack",
		.owner	= THIS_MODULE,
	},
};

static int __init jack_init(void)
{
	return platform_driver_register(&jack_driver);
}
#ifndef CONFIG_SCORE_FAST_RESUME
subsys_initcall(jack_init);
#else
fast_subsys_initcall(jack_init);
#endif
static void __exit jack_exit(void)
{
	platform_driver_unregister(&jack_driver);
}
module_exit(jack_exit);

MODULE_AUTHOR("Minkyu Kang <mk7.kang@samsung.com>");
MODULE_DESCRIPTION("Jack Monitoring Interface");
MODULE_LICENSE("GPL");
