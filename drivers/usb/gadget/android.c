/*
 * Gadget Driver for Android
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * Copyright (C) 2010 Samsung Electronics,
 * Author : SoonYong Cho <soonyong.cho@samsung.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/* #define DEBUG */
/* #define VERBOSE_DEBUG */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mutex.h>


#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>

#include <linux/usb/android_composite.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>

#include "gadget_chips.h"

/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */

#if 1
/** changed by wonjung1.kim **/
#include "usbstring.c"
#include "config.c"
#include "epautoconf.c"
#include "composite.c"
#else
extern void usb_function_set_enabled(struct usb_function *f, int enabled);
extern void usb_composite_force_reset(struct usb_composite_dev *cdev);
extern int usb_composite_register(struct usb_composite_driver *driver);
#endif
/** changed by wonjung1.kim **/

//#define _SLP_USB_

#ifdef _SLP_USB_
static struct class *android_class;
#endif

void usb_drime4_ss_udc_run_stop(unsigned int is_on);


/* soonyong.cho : refer product id and config string of usb from 'arch/arm/plat-samsung/include/plat/devs.h' */
//#include <plat/devs.h>

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
/* soonyong.cho : Added functions and modifed composite framework for samsung composite.
 *                Developers can make custom composite easily using this custom samsung framework.
 */
MODULE_AUTHOR("SoonYong Cho");
#else
MODULE_AUTHOR("Mike Lockwood");
#endif
MODULE_DESCRIPTION("Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static const char longname[] = "Gadget Android";

/* Default vendor and product IDs, overridden by platform data */
#define VENDOR_ID		0x04e8	/* SAMSUNG */
/* soonyong.cho : default product id refered as <plat/devs.h> */
#define PRODUCT_ID		SAMSUNG_DEBUG_PRODUCT_ID


struct android_dev
{
	struct usb_composite_dev *cdev;
	int num_products;
	struct android_usb_product *products;
	int num_functions;
	char **functions;

	int product_id;
	int version;
	int current_usb_mode;   /* soonyong.cho : save usb mode except tethering and askon mode. */
	int requested_usb_mode; /*                requested usb mode from app included tethering and askon */
	int debugging_usb_mode; /*		  debugging usb mode */
	struct mutex	usb_mode_lock;	/* to guarantee stable switching */
};

static struct android_dev *_android_dev;

/* string IDs are assigned dynamically */

#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

/* String Table */

static struct usb_string strings_dev[] = 
{
	/* These dummy values should be overridden by platform data */
	/* soonyong.cho : Samsung string default product id refered as <plat/devs.h> */
	[STRING_MANUFACTURER_IDX].s = "SAMSUNG",
	[STRING_PRODUCT_IDX].s = "SLP",
	[STRING_SERIAL_IDX].s = "C110_SLP",
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_dev = 
{
	.language	= 0x0409,	/* en-us */
	.strings		= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = 
{
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = 
{
	.bLength				= sizeof(device_desc),
	.bDescriptorType		= USB_DT_DEVICE,
	.bcdUSB				= __constant_cpu_to_le16(0x0200),
	.bDeviceClass			= USB_CLASS_PER_INTERFACE,
	.idVendor				= __constant_cpu_to_le16(VENDOR_ID),
	.idProduct			= __constant_cpu_to_le16(PRODUCT_ID),
	.bcdDevice			= __constant_cpu_to_le16(0xffff),
	.bNumConfigurations	= 1,
};

static struct list_head _functions = LIST_HEAD_INIT(_functions);
static int _registered_function_count = 0;
static int _added_config_count = 0;

static void samsung_enable_function(int mode);

static int android_bind_config(struct usb_configuration *c)
{
	_added_config_count++;

	return 0;
}

/* soonyong.cho : It is default config string. It'll be changed to real config string when last function driver is registered. */
#define ANDROID_DEFAULT_CONFIG_STRING "Samsung Android Shared Config"	/* android default config string */

static int android_setup_config(struct usb_configuration *c, const struct usb_ctrlrequest *ctrl)
{
	int i;
	int ret = -EOPNOTSUPP;
/* soonyong.cho : Do not call same function config when function has many interface.
 *                If another function driver has different config function, It needs calling.
 */
	char temp_name[128]={0,};
	for (i = 0; i < c->next_interface_id; i++) 
	{
		if (!c->interface[i]->disabled && c->interface[i]->setup)
		{
			if (!strcmp(temp_name, c->interface[i]->name)) 
			{
				continue;
			}
			else
			{
				strcpy(temp_name,c->interface[i]->name);
			}

			ret = c->interface[i]->setup(c->interface[i], ctrl);

			if (ret >= 0)
			{
				return ret;
			}
		}
	}
	return ret;
}

static struct usb_configuration android_first_config = 
{
	.label		= "Samsung First Config",
	.bind		= android_bind_config,
	.setup		= android_setup_config,
	.bConfigurationValue = 1,
	.bmAttributes	= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower	= 0x30, /* 96ma */
};

static struct usb_configuration android_second_config = 
{
	.label		= "Samsung Second Config",
	.bind		= android_bind_config,
	.setup		= android_setup_config,
	.bConfigurationValue = 2,
	.bmAttributes	= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower	= 0x30, /* 96ma */
};

static struct android_usb_function *get_function(const char *name)
{
	struct android_usb_function	*f;
	
	list_for_each_entry(f, &_functions, list)
	{
		if (!strcmp(name, f->name))
		{
			return f;
		}
	}
	return NULL;
}

static void upper_config_functions(struct android_dev *adev)
{
	struct android_usb_function	*f;
	char **functions = adev->functions;
	int i;

	for(i = 0; i < adev->num_functions; i++)
	{
		char *name = *functions++;
		INFO(adev->cdev, "func->name=%s\n",name);
		f = get_function(name);
		if(f) 
		{
			//INFO(adev->cdev, "get_function->name=%s\n", f->name);
			if(f->bind_upper_config)
			{
				f->bind_upper_config(adev->cdev);
			}
		}
		else
		{
			printk(KERN_ERR "function %s not found in bind_upper/bind_functions\n", name);
		}
	}
}

static int product_has_function_fconfig(struct android_usb_product *p, struct usb_function *f)
{
	char **functions = p->functions;
	int count = p->num_functions;
	const char *name = f->name;
	int i;

	for(i = 0; i < count; i++) 
	{
		if(!strcmp(name, *functions++))
		{
			return 1;
		}
	}
	return 0;
}

static int product_has_function_sconfig(struct android_usb_product *p, struct usb_function *f)
{
	char **functions = p->se_functions;
	int count = p->se_num_functions;
	const char *name = f->name;
	int i;

	for(i = 0; i < count; i++) 
	{
		if(!strcmp(name, *functions++))
		{
			return 1;
		}
	}
	return 0;
}

static int product_matches_functions(struct android_usb_product *p)
{
	struct usb_function *f;
	list_for_each_entry(f, &android_first_config.functions, list)
	{
		if(product_has_function_fconfig(p, f) == !!f->disabled)
		{
			return 0;
		}
	}

	if(p->se_num_functions != 0) 
	{
		list_for_each_entry(f, &android_second_config.functions, list)
		{
			if(product_has_function_sconfig(p, f) == !!f->disabled)
			{
				return 0;
			}
		}
	}
	return 1;
}

static int get_product_id(struct android_dev *dev)
{
	struct android_usb_product *p = dev->products;
	int count	= dev->num_products;
	int mode	= dev->current_usb_mode;
	int i;

	if(mode != -1 && p)
	{
		for(i = 0; i < count; i++, p++)
		{
			if(p->mode == mode)
			{
				return p->product_id;
			}
		}
	}
	p = dev->products;

	if(list_empty(&android_first_config.functions) && list_empty(&android_second_config.functions))
	{
		return dev->product_id;
	}

	if(p)
	{
		for(i = 0; i < count; i++, p++)
		{
			if (product_matches_functions(p))
			{
				return p->product_id;
			}
		}
	}
	DBG(dev->cdev, "num_products=%d, pid=0x%x\n",count, dev->product_id);
	/* use default product ID */
	return dev->product_id;
}

static int android_bind(struct usb_composite_dev *cdev)
{
	struct android_dev *adev = _android_dev;
	struct usb_gadget	*gadget = cdev->gadget;
	int			gcnum, id, product_id, ret;

	printk(KERN_INFO "android_bind\n");

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	id = usb_string_id(cdev);
	if(id < 0)
	{
		return id;
	}
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if(id < 0)
	{
		return id;
	}
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	id = usb_string_id(cdev);
	if(id < 0)
	{
		return id;
	}
	strings_dev[STRING_SERIAL_IDX].id = id;
	device_desc.iSerialNumber = id;

	//if (gadget->ops->wakeup)
		//android_config_driver.bmAttributes |= USB_CONFIG_ATT_WAKEUP;

	adev->current_usb_mode = -1;
	adev->cdev = cdev;

	/* bind our functions if they have all registered */

	/* register our configuration */
//	ret = usb_add_config(cdev, &android_first_config); // 36 Version
   	ret = usb_add_config(cdev, &android_first_config, (android_first_config.bind));

	if(ret) 
	{
		printk(KERN_ERR "usb_add_config failed\n");
		return ret;
	}

	//ret = usb_add_config(cdev, &android_second_config);// 36 Version
	ret = usb_add_config(cdev, &android_second_config, (android_second_config.bind));


	if(ret)
	{
		printk(KERN_ERR "usb_add_config failed\n");
		return ret;
	}

	if(_registered_function_count == adev->num_functions)
	{
		upper_config_functions(adev);
	}

	gcnum = usb_gadget_controller_number(gadget);
	if(gcnum >= 0)
	{
		/* Samsung KIES needs fixed bcdDevice number */
		device_desc.bcdDevice = cpu_to_le16(0x0400);
	}
	else
	{
		/* gadget zero is so simple (for now, no altsettings) that
		 * it SHOULD NOT have problems with bulk-capable hardware.
		 * so just warn about unrcognized controllers -- don't panic.
		 *
		 * things like configuration and altsetting numbering
		 * can need hardware-specific attention though.
		 */
		pr_warning("%s: controller '%s' not recognized\n", 	longname, gadget->name);
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	usb_gadget_set_selfpowered(gadget);

	product_id = get_product_id(adev);
	device_desc.idProduct = __constant_cpu_to_le16(product_id);
	cdev->desc.idProduct = device_desc.idProduct;

	INFO(cdev, "bind pid=0x%x,vid=0x%x,bcdDevice=0x%x,serial=%s\n",
	cdev->desc.idProduct, device_desc.idVendor, device_desc.bcdDevice, strings_dev[STRING_SERIAL_IDX].s);
	return 0;
}

static struct usb_composite_driver android_usb_driver = 
{
	.name	= "android_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind	= android_bind,
	.enable_function	= android_enable_function,
};


void android_register_function(struct android_usb_function *f)
{
	struct android_dev *adev = _android_dev;

	printk(KERN_ERR "android_register_function %s\n", f->name);
	list_add_tail(&f->list, &_functions);
	_registered_function_count++;
	//printk(KERN_INFO "android_register_function 2 %d\n",adev->num_functions);

	/* bind our functions if they have all registered
	 * and the main driver has bound.
	 */
	//INFO(adev->cdev, "name=%s, registered_function_count=%d, adev->num_functions=%d\n",f->name, _registered_function_count, adev->num_functions);
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	//printk(KERN_EMERG "name=%s, registered_function_count=%d, adev->num_functions=%d\n",f->name, _registered_function_count, adev->num_functions);
/* static analysis bug fix 2012-09-10 Han Seungwon */
	if(adev && (_added_config_count == 2) && (_registered_function_count == adev->num_functions)) 
	{
       	//INFO(adev->cdev, "SSSSSSSS");
		printk(KERN_ERR "SSSSSSSS \n");

		upper_config_functions(adev);
		/* Change usb mode when device register last function driver */
		samsung_enable_function(USBSTATUS_UMS);
		//samsung_enable_function(USBSTATUS_MTPONLY);
		/* soonyong.cho : If usb switch can call usb cable handler safely, you don't need below code.
		 *		  Below codes are used for to turn on always.
		 *		  Do not enable udc. USB switch must call usb cable handler when cable status is changed.
		 */
		//INFO(adev->cdev, "Don't enable udc.\n");
	}

}

/*
 * Description  : Set enable functions
 * Parameters   : char** functions (product function list), int num_f (number of product functions)
 * Return value : Count of enable functions
 *
 * Written by SoonYong,Cho  (Fri 5, Nov 2010)
 */
static int set_enable_functions(struct android_usb_product *p, struct usb_composite_dev *cdev)
{
	struct usb_function		*func;
	int i;
	int total_count = 0, bind_func_num = 0;
	struct android_usb_function	*ff;
	char **functions_name;

	/* First step, unbind each function of config & del config list*/
	while(!list_empty(&cdev->configs)) 
	{
		struct usb_configuration	*c;

		c = list_first_entry(&cdev->configs, struct usb_configuration, list);

		for(i=0; i<MAX_CONFIG_INTERFACES; i++)
		{
			c->interface[i] = NULL;
		}
		c->next_interface_id = 0;

		while (!list_empty(&c->functions)) 
		{
			struct usb_function		*f;

			f = list_first_entry(&c->functions,
					struct usb_function, list);
			list_del(&f->list);
			if (f->unbind) 
			{
				DBG(cdev, "unbind function '%s'/%p\n", f->name, f);
				f->unbind(c, f);
				/* may free memory for "f" */
			}
		}

		list_del(&c->list);
	}

	usb_ep_autoconfig_reset(cdev->gadget);								//reset end-points

	printk(KERN_INFO "Now re-config for PID : 0x%x\n",p->product_id);
	/*real bind for first config*/
	functions_name = p->functions;
	for(i = 0; i < p->num_functions; i++) 
	{
		/*real bind */
		printk(KERN_INFO "1st config:get_function->name=%s num_f(%d)=%d\n", 	*functions_name, p->num_functions, i+1);
		ff = get_function(*functions_name++);
		if (ff) 
		{
			if(ff->bind_config)
			{
				ff->bind_config(&android_first_config);						//real function bind
			}
			++total_count;
			bind_func_num++;
		}
		else
		{
			printk(KERN_ERR "1st conf function %s not found in bind_functions\n", *functions_name);
		}
	}
	if (bind_func_num) 
	{
		list_add_tail(&android_first_config.list, &cdev->configs);
		bind_func_num = 0;
	}

	/*real bind for second config*/
	functions_name = p->se_functions;
	for(i = 0; i < p->se_num_functions; i++) 
	{
		/*real bind */
		printk(KERN_INFO "2nd config:get_function->name=%s num_f(%d)=%d\n", *functions_name, p->se_num_functions, i+1);
		ff = get_function(*functions_name++);
		if (ff) 
		{
			if(ff->bind_config)
			{
				ff->bind_config(&android_second_config);						//real function bind
			}
			++total_count;
			bind_func_num++;
		}
		else
		{
			printk(KERN_ERR "2nd conf function %s not found in bind_functions\n", *functions_name);
		}
	}
	
	if (bind_func_num)
	{
		list_add_tail(&android_second_config.list, &cdev->configs);
	}

	//after real binding, func->disabled : enable
	list_for_each_entry(func, &android_first_config.functions, list)
	{
		usb_function_set_enabled(func, 1);
	}

	list_for_each_entry(func, &android_second_config.functions, list)
	{
		usb_function_set_enabled(func, 1);
	}

	return total_count;
}

/*
 * Description  : Set product using function as set_enable_function
 * Parameters   : struct android_dev *adev (Refer adev->products), __u16 mode (usb mode)
 * Return Value : -1 (fail to find product), positive value (number of functions)
 *
 * Written by SoonYong,Cho  (Fri 5, Nov 2010)
 */
static int set_product(struct android_dev *adev, __u16 mode)
{
	struct android_usb_product *p = adev->products;
	int count = adev->num_products;
	int i, ret;

	/* static analysis bug fix 2012-09-27 Han Seungwon */
	if(adev->cdev == NULL)
	{
		return -1;
	}

 	/* Save usb mode always even though it will be failed */
	adev->requested_usb_mode = mode;

	if (p) 
	{
		for (i = 0; i < count; i++, p++) 
		{
			if(p->mode == mode) 
			{
				/* It is for setting dynamic interface in composite.c */
				adev->cdev->products		= p;
				adev->cdev->desc.bDeviceClass	 = p->bDeviceClass;
				adev->cdev->desc.bDeviceSubClass	 = p->bDeviceSubClass;
				adev->cdev->desc.bDeviceProtocol	 = p->bDeviceProtocol;

				ret = set_enable_functions(p, adev->cdev);
				//INFO(adev->cdev, "Change Device Descriptor : DeviceClass(0x%x),SubClass(0x%x),Protocol(0x%x)\n",
				//	p->bDeviceClass, p->bDeviceSubClass, p->bDeviceProtocol);
				if(ret == 0)
				{
					DBG(adev->cdev, "Can't find functions(mode=0x%x)\n", mode);
				}
				else
				{
					DBG(adev->cdev, "set function num=%d\n", ret);
				}
				return ret;
			}
		}
	}

#if 0
	else
		INFO(adev->cdev, "adev->products is not available\n");
#endif	/* static analysis bug fix 2012-09-18 Han Seungwon */


	//INFO(adev->cdev, "mode=0x%x is not available\n",mode);	/* static analysis bug fix 2012-09-18 Han Seungwon */
	return -1;
}

/*
 * Description  : Enable functions for samsung composite driver
 * Parameters   : struct usb_function *f (It depends on function's sysfs), int enable (1:enable, 0:disable)
 * Return value : void
 *
 * Written by SoonYong,Cho  (Fri 5, Nov 2010)
 */
void android_enable_function(struct usb_function *f, int enable)
{
	struct android_dev *dev = _android_dev;
	int product_id = 0;
	int ret = -1;

	/* static analysis bug fix 2012-09-25 Han Seungwon */
	if(dev == NULL)
	{
		return;
	}
	
	INFO(dev->cdev, "++ f->name=%s enable=%d\n", f->name, enable);

	if(enable) {
		if (!strcmp(f->name, "acm")) {
			ret = set_product(dev, USBSTATUS_SAMSUNG_KIES);
			if (ret != -1)
				dev->current_usb_mode = USBSTATUS_SAMSUNG_KIES;
		}
		if (!strcmp(f->name, "adb")) {
			ret = set_product(dev, USBSTATUS_ADB);
			if (ret != -1)
				dev->debugging_usb_mode = 1; /* save debugging status */
		}
		if (!strcmp(f->name, "mtp")) {
			ret = set_product(dev, USBSTATUS_MTPONLY);
			if (ret != -1)
				dev->current_usb_mode = USBSTATUS_MTPONLY;
		}
		if (!strcmp(f->name, "rndis")) {
			ret = set_product(dev, USBSTATUS_VTP);
		}
		if (!strcmp(f->name, "usb_mass_storage")) {
			ret = set_product(dev, USBSTATUS_UMS);
			if (ret != -1)
				dev->current_usb_mode = USBSTATUS_UMS;
		}
#ifdef CONFIG_USB_ANDROID_ECM
		if (!strcmp(f->name, "ecm")) {
			ret = set_product(dev, USBSTATUS_ECM);
			if (ret != -1)
				dev->current_usb_mode = USBSTATUS_ECM;
		}
#endif

	}
	else { /* for disable : Return old mode. If Non-GED model changes policy, below code has to be modified. */
		if (!strcmp(f->name, "rndis") && dev->debugging_usb_mode)
			ret = set_product(dev, USBSTATUS_ADB);
		else
			ret = set_product(dev, dev->current_usb_mode);

		if(!strcmp(f->name, "adb"))
			dev->debugging_usb_mode = 0;
	} /* if(enable) */

	if(ret == -1) {
		INFO(dev->cdev, "Can't find product. It is not changed !\n");
		return ;
	}

	product_id = get_product_id(dev);
	device_desc.idProduct = __constant_cpu_to_le16(product_id);

	if (dev->cdev)
		dev->cdev->desc.idProduct = device_desc.idProduct;

	/* force reenumeration */
	INFO(dev->cdev, "dev->cdev=0x%p, dev->cdev->gadget=0x%p, dev->cdev->gadget->speed=0x%x, mode=%d\n",
		dev->cdev, dev->cdev->gadget, dev->cdev->gadget->speed, dev->current_usb_mode);
	usb_composite_force_reset(dev->cdev);

	INFO(dev->cdev, "finished setting pid=0x%x\n",product_id);
}

/*
 * Description  : Enable functions for samsung composite driver using mode
 * Parameters   : int mode (Static mode number such as KIES, UMS, MTP, etc...)
 * Return value : void
 *
 * Written by SoonYong,Cho  (Fri 5, Nov 2010)
 */
static void samsung_enable_function(int mode)
{
	struct android_dev *adev = _android_dev;
	int product_id = 0;
	int ret = -1;
	//INFO(adev->cdev, "enable mode=0x%x\n", mode);

	/* static analysis bug fix 2012-09-25 Han Seungwon */
	if(adev == NULL)
	{
		return -1;
	}
	/* static analysis bug fix 2012-09-25 Han Seungwon */


	if(adev->current_usb_mode == mode) {
			INFO(adev->cdev, "Same usb mode. Usb mode is not changed\n");
			return;
	}

	mutex_lock(&adev->usb_mode_lock);

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE_ADVANCED
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	//INFO(adev->cdev, "disconnect usb first\n");
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	if(adev->cdev && adev->cdev->gadget) {
        //usb_gadget_vbus_disconnect(adev->cdev->gadget);
		usb_gadget_disconnect(adev->cdev->gadget);
		msleep(1);
	}
#endif

	switch(mode) {
		case USBSTATUS_UMS:
			//INFO(adev->cdev, "mode = USBSTATUS_UMS (0x%x)\n", mode);	/* static analysis bug fix 2012-09-18 Han Seungwon */
			ret = set_product(adev, USBSTATUS_UMS);
			break;
		case USBSTATUS_SAMSUNG_KIES:
			//INFO(adev->cdev, "mode = USBSTATUS_SAMSUNG_KIES (0x%x)\n", mode);	/* static analysis bug fix 2012-09-18 Han Seungwon */
			ret = set_product(adev, USBSTATUS_SAMSUNG_KIES);
			break;
		case USBSTATUS_MTPONLY:
			//INFO(adev->cdev, "mode = USBSTATUS_MTPONLY (0x%x)\n", mode);	/* static analysis bug fix 2012-09-18 Han Seungwon */
			ret = set_product(adev, USBSTATUS_MTPONLY);
			break;
		case USBSTATUS_ADB:
			//INFO(adev->cdev, "mode = USBSTATUS_ADB (0x%x)\n", mode);	/* static analysis bug fix 2012-09-18 Han Seungwon */
			ret = set_product(adev, USBSTATUS_ADB);
			break;
		case USBSTATUS_VTP: /* do not save usb mode */
			//INFO(adev->cdev, "mode = USBSTATUS_VTP (0x%x)\n", mode);	/* static analysis bug fix 2012-09-18 Han Seungwon */
			ret = set_product(adev, USBSTATUS_VTP);
			break;
#ifdef CONFIG_USB_ANDROID_ECM
		case USBSTATUS_ECM:
			//INFO(adev->cdev, "mode = USBSTATUS_ECM (0x%x)\n", mode);	/* static analysis bug fix 2012-09-18 Han Seungwon */
			ret = set_product(adev, USBSTATUS_ECM);
			break;
#endif
	}

	if(ret == -1) {
		/* static analysis bug fix 2012-09-10 Han Seungwon */
		//INFO(adev->cdev, "Can't find product. It is not changed !\n");
		/* static analysis bug fix 2012-09-10 Han Seungwon */
#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE_ADVANCED
		/* static analysis bug fix 2012-09-10 Han Seungwon */
		//INFO(adev->cdev, "connect usb again\n");
		/* static analysis bug fix 2012-09-10 Han Seungwon */
		if(adev->cdev && adev->cdev->gadget)
			usb_gadget_connect(adev->cdev->gadget);
#endif
		mutex_unlock(&adev->usb_mode_lock);
		return ;
	}
	else {
//		INFO(adev->cdev, "Save new usb mode (mode=%d)\n", mode);
		adev->current_usb_mode = mode;
	}

	product_id = get_product_id(adev);
	device_desc.idProduct = __constant_cpu_to_le16(product_id);

	if (adev->cdev)
		adev->cdev->desc.idProduct = device_desc.idProduct;

	/* force reenumeration */
//	INFO(adev->cdev, "adev->cdev=0x%p, adev->cdev->gadget=0x%p, adev->cdev->gadget->speed=0x%x, mode=%d\n",
//		adev->cdev, adev->cdev->gadget, adev->cdev->gadget->speed, adev->current_usb_mode);
#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE_ADVANCED
	//INFO(adev->cdev, "connect usb again\n");
	if(adev->cdev && adev->cdev->gadget){
        //usb_gadget_vbus_connect(adev->cdev->gadget);
        usb_gadget_connect(adev->cdev->gadget);
	}
#else
	usb_composite_force_reset(adev->cdev);
#endif
	//dev_info(adev->cdev, "BBB \n");

	mutex_unlock(&adev->usb_mode_lock);

	//INFO(adev->cdev, "finished setting pid=0x%x\n",product_id);
}


/* soonyong.cho : sysfs for to show status of usb config
 *                Path (/sys/devices/platform/android_usb/UsbMenuSel)
 */
static ssize_t UsbMenuSel_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct android_dev *a_dev = _android_dev;
	int value = -1;

	if(a_dev->cdev) {
		switch(a_dev->requested_usb_mode) {
			case USBSTATUS_UMS:
				return sprintf(buf, "[UsbMenuSel] UMS\n");
			case USBSTATUS_SAMSUNG_KIES:
				return sprintf(buf, "[UsbMenuSel] %s\n", ANDROID_KIES_CONFIG_STRING);
			case USBSTATUS_MTPONLY:
				return sprintf(buf, "[UsbMenuSel] MTP\n");
			case USBSTATUS_ASKON:
				return sprintf(buf, "[UsbMenuSel] ASK\n");
			case USBSTATUS_VTP:
				return sprintf(buf, "[UsbMenuSel] TETHERING\n");
			case USBSTATUS_ADB:
				return sprintf(buf, "[UsbMenuSel] ACM_ADB_UMS\n");
#ifdef CONFIG_USB_ANDROID_ECM
			case USBSTATUS_ECM:
				return sprintf(buf, "[UsbMenuSel] ECM\n");
#endif
		}
	}
	else {
		DBG(a_dev->cdev, "Fail to show usb menu switch. dev->cdev is not valid\n");
	}

	return sprintf(buf, "%d\n", value);
}

/* soonyong.cho : sysfs for to change status of usb config
 *                Path (/sys/devices/platform/android_usb/UsbMenuSel)
 */
static ssize_t UsbMenuSel_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_dev *a_dev = _android_dev;
	int value;
	int ret = 0;

	usb_drime4_ss_udc_run_stop(1);
	
	/* static analysis bug fix 2012-09-10 Han Seungwon */
	ret = sscanf(buf, "%d", &value);

	if(ret == -1)
	{
		return -1;
	}
	/* static analysis bug fix 2012-09-10 Han Seungwon */

	dev_dbg(dev, "%s\n", __func__);

	switch(value) {
		case 0:
			INFO(a_dev->cdev, "Enable KIES(%d)\n", value);
			dev_dbg(dev, "Enable KIES(%d)\n", value);
			//samsung_enable_function(USBSTATUS_SAMSUNG_KIES);
			msleep(200);
			samsung_enable_function(USBSTATUS_MTPONLY);
			break;
		case 1:
			INFO(a_dev->cdev, "Enable MTP(%d)\n", value);
           	dev_dbg(dev, "Enable MTP(%d)\n", value);
			samsung_enable_function(USBSTATUS_MTPONLY);
			break;
		case 2:
			INFO(a_dev->cdev, "Enable UMS(%d)\n", value);
           		dev_dbg(dev, "Enable UMS(%d)\n", value);
			msleep(200);
			samsung_enable_function(USBSTATUS_UMS);
			break;
		case 3:
			INFO(a_dev->cdev, "Enable ASKON(%d)\n", value);
           		dev_dbg(dev, "Enable ASKON(%d)\n", value);
			samsung_enable_function(USBSTATUS_ASKON);
			break;
#ifdef CONFIG_USB_ANDROID_ECM
		case 4:
			INFO(a_dev->cdev, "Enable ECM(%d)\n", value);
			samsung_enable_function(USBSTATUS_ECM);
			break;
#endif
		default:
			DBG(a_dev->cdev, "Fail : value(%d) is not invaild.\n", value);
           		dev_dbg(dev, "Fail : value(%d) is not invaild.\n", value);
	}

	//usb_drime4_ss_udc_run_stop(0);

	return size;
}

/* soonyong.cho : attribute of sysfs for usb menu switch */
static DEVICE_ATTR(UsbMenuSel, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, UsbMenuSel_switch_show, UsbMenuSel_switch_store);

#ifdef _SLP_USB_
#define USB_MODE_VERSION	"1.0"

static CLASS_ATTR_STRING(version, S_IRUSR | S_IRGRP | S_IROTH,
			 USB_MODE_VERSION);

static DEVICE_ATTR(enable, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, "", "");
static DEVICE_ATTR(functions, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, "", "");
static DEVICE_ATTR(idVendor, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, "", "");
static DEVICE_ATTR(idProduct, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, "", "");
static DEVICE_ATTR(bDeviceClass, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, "", "");

#endif

static int android_probe(struct platform_device *pdev)
{
	struct android_usb_platform_data *pdata = pdev->dev.platform_data;
	struct android_dev *adev = _android_dev;

	//printk(KERN_INFO "android_probe pdata: %p\n", pdata);
	//dev_dbg(pdev->dev, "android_probe pdata\n");

#ifdef _SLP_USB_

	int err=0;

	printk(KERN_DEBUG "%s\n", __func__);
	android_class = class_create(THIS_MODULE, "usb_mode");
	if (IS_ERR(android_class))
		return PTR_ERR(android_class);

    err = class_create_file(android_class, &class_attr_version.attr);
	if (err) {
		printk(KERN_ERR "usb_mode: can't create sysfs version file\n");
		goto err_class;
	}

	adev = kzalloc(sizeof(*adev), GFP_KERNEL);
	if (!adev) {
		printk(KERN_ERR "usb_mode: can't alloc for adev\n");
		err = -ENOMEM;
		goto err_attr;
	}

   	&pdev->dev = device_create(android_class, NULL,
				MKDEV(0, 0), NULL, "usb0");


	dev_set_drvdata(pdev->dev, adev);

	err = device_create_file(adev->cdev, &dev_attr_enable);
	err = device_create_file(adev->cdev, &dev_attr_functions);
	err = device_create_file(adev->cdev, &dev_attr_idVendor);
	err = device_create_file(adev->cdev, &dev_attr_idProduct);
	err = device_create_file(adev->cdev, &dev_attr_bDeviceClass);

	if(err) 
	{
        	printk(KERN_ERR "android_probe:  can't device_create_file\n");
	}


#endif

	printk(KERN_ERR "[android_probe] pdata:%p \n", pdata);
	
	if(pdata) 
	{
		printk(KERN_ERR "[android_probe] 1 \n");	
		adev->products = pdata->products;
		adev->num_products = pdata->num_products;
		adev->functions = pdata->functions;
		adev->num_functions = pdata->num_functions;
		if (pdata->vendor_id)
			device_desc.idVendor =
				__constant_cpu_to_le16(pdata->vendor_id);
		if (pdata->product_id) {
			adev->product_id = pdata->product_id;
			device_desc.idProduct =
				__constant_cpu_to_le16(pdata->product_id);
		}
		if (pdata->version)
			adev->version = pdata->version;

		if (pdata->product_name)
			strings_dev[STRING_PRODUCT_IDX].s = pdata->product_name;
		if (pdata->manufacturer_name)
			strings_dev[STRING_MANUFACTURER_IDX].s =
					pdata->manufacturer_name;
		if (pdata->serial_number)
			strings_dev[STRING_SERIAL_IDX].s = pdata->serial_number;
		printk(KERN_INFO "vid=0x%x,pid=0x%x,ver=0x%x,product_name=%s,manufacturer_name=%s,serial=%s\n",
			pdata->vendor_id, pdata->product_id, pdata->version, pdata->product_name, pdata->manufacturer_name,
			pdata->serial_number);
	}

	/* soonyong.cho : Create attribute of sysfs as '/sys/devices/platform/usb_mode/UsbMenuSel'
	 *                It is for USB menu selection.
	 * 		  Application for USB Setting made by SAMSUNG uses property that uses below sysfs.
	 */
	if (device_create_file(&pdev->dev, &dev_attr_UsbMenuSel) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_UsbMenuSel.attr.name);

	/* soonyong.cho : If you use usb switch and enable usb switch before to initilize final function driver,
	 *		  it can be called as vbus_session function without to initialize product number
	 *		  and present product.
	 *		  But, Best guide is that usb switch doesn't initialize before usb driver.
	 *		  If you want initialize, please implement it.
	 */
#if 0
	return usb_composite_register(&android_usb_driver); //by wonjung1.kim
#else
	return usb_composite_probe(&android_usb_driver, android_bind); //39version
#endif


#ifdef _SLP_USB_


 err_class:
	class_destroy(android_class);

#endif

}


static int android_suspend(struct platform_device *pdev, pm_message_t state)
{
	printk("android_suspend\r\n");

	return 0;
}


static int android_resume(struct platform_device *pdev)
{
	printk("android_resume\r\n");
	samsung_enable_function(USBSTATUS_UMS);

	return 0;
}

static struct platform_driver android_platform_driver = {
	.driver = { .name = "usb_mode", },
	.probe = android_probe,
	.suspend = android_suspend,
	.resume = android_resume,
};

static int __init init(void)
{
	struct android_dev *adev;

	printk(KERN_ERR "android init\n");

	adev = kzalloc(sizeof(*adev), GFP_KERNEL);
	if (!adev)
		return -ENOMEM;

	mutex_init(&adev->usb_mode_lock);

	/* set default values, which should be overridden by platform data */
	adev->product_id = PRODUCT_ID;
	_android_dev = adev;

	//printk(KERN_INFO "android init pid=0x%x\n",adev->product_id);
	return platform_driver_register(&android_platform_driver);
}
#ifndef CONFIG_SCORE_FAST_RESUME
module_init(init);
#else
fast_dev_initcall(init);
#endif


static void __exit cleanup(void)
{
	printk("android clean\n");
	usb_composite_unregister(&android_usb_driver);
	platform_driver_unregister(&android_platform_driver);
	kfree(_android_dev);
	_android_dev = NULL;
}
module_exit(cleanup);
