/* linux/arch/arm/mach-drime4/dev-usb-dwc3.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - USB3 support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <mach/map.h>
#include <mach/irqs.h>
#include <mach/devs.h>


#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
//#include <linux/host_notify.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <mach/hardware.h>
#include <mach/map.h>
#include <mach/dma.h>


/* Android USB OTG Gadget */
#include <linux/usb/android_composite.h>
#define S3C_VENDOR_ID		0x18d1
#define S3C_PRODUCT_ID		0x0001
#define S3C_ADB_PRODUCT_ID	0x0005
#define MAX_USB_SERIAL_NUM	17


static char *usb_functions_ums[] = {
	"usb_mass_storage",
};


static u64 dwc3_dmamask = DMA_BIT_MASK(32);


static char *usb_functions_mtp_acm[] = {
	"mtp",
	"acm",
};

/* mtp only mode */
static char *usb_functions_mtp[] = {
	"mtp",
};



static char *usb_functions_all[] = {
	"usb_mass_storage",
	"mtp",
};

static struct android_usb_product usb_products[] = {
	{
#if 0
		.product_id	= 0x685B, //SAMSUNG_UMS_PRODUCT_ID,
#else
		.product_id	= D4_UDC_MASS_PID,
#endif	/* #if 0 */
		.num_functions	= ARRAY_SIZE(usb_functions_ums),
		.functions	= usb_functions_ums,
		.bDeviceClass	= USB_CLASS_PER_INTERFACE,
		.bDeviceSubClass= 0,
		.bDeviceProtocol= 0,
		.mode		= 0x0, //USBSTATUS_UMS,
	},
    {
#if 0
		.product_id	= 0x685C, 	//.product_id	= 0x685C, //60 ; // SAMSUNG_KIES_PRODUCT_ID,
#else
		.product_id	= D4_UDC_PICT_PID,
#endif	/* #if 0 */
		.num_functions = ARRAY_SIZE(usb_functions_mtp),	//.num_functions	= ARRAY_SIZE(usb_functions_mtp),
		.functions	= usb_functions_mtp,	//.functions	= usb_functions_mtp,
		//.se_num_functions	= ARRAY_SIZE(usb_functions_mtp_acm),
		//.se_functions	= usb_functions_mtp_acm,
		.bDeviceClass	= 0x0,	//.bDeviceClass	= 0xEF,
		.bDeviceSubClass= 0x00,	//.bDeviceSubClass= 0x02,
		.bDeviceProtocol= 0x00,
		.mode		= 0x2, //USBSTATUS_MTPONLY			0x2

	},


};

// serial number should be changed as real device for commercial release
static char device_serial[MAX_USB_SERIAL_NUM]="0123456789ABCDEF";

static struct android_usb_platform_data android_usb_pdata = {
#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
/* soonyong.cho : refered from S1 */
	.vendor_id		= SAMSUNG_VENDOR_ID,
	.product_id		= SAMSUNG_KIES_PRODUCT_ID,
	.manufacturer_name	= "SAMSUNG",
	.product_name		= "NX300",
#else
	.vendor_id		= S3C_VENDOR_ID,
	.product_id		= S3C_PRODUCT_ID,
	.manufacturer_name	= "Android",//"Samsung",
	.product_name		= "Android",//"Samsung SMDKV210",
#endif
	.serial_number		= device_serial,
	.num_products 		= ARRAY_SIZE(usb_products),
	.products 		= usb_products,
	.num_functions 		= ARRAY_SIZE(usb_functions_all),
	.functions 		= usb_functions_all,
};

struct platform_device s3c_device_android_usb = {
	.name	= "usb_mode",
	.id	= -1,
	.dev	= {
		.platform_data	= &android_usb_pdata,
	},
};
EXPORT_SYMBOL(s3c_device_android_usb);

static struct usb_mass_storage_platform_data ums_pdata = {
	.vendor			= "Samsung",
	.product		= "UMS Composite",//"SMDKV210",
	.release		= 1,
	.nluns			= 2,			/* for UMS , MMC*/
};
struct platform_device s3c_device_usb_mass_storage= {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &ums_pdata,
	},
};
EXPORT_SYMBOL(s3c_device_usb_mass_storage);










static struct resource drime4_usb_dwc3_resource[] = {
	[0] = {
		.start	= DRIME4_PA_USB,
		.end	= DRIME4_PA_USB + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= DRIME4_PA_USB_CFG,
		.end	= DRIME4_PA_USB_CFG + SZ_256 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= IRQ_USB,
		.end	= IRQ_USB,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device drime4_device_usb_dwc3 = {
	.name		= "drime4-dwc3",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(drime4_usb_dwc3_resource),
	.resource	= drime4_usb_dwc3_resource,
	.dev		= {
		.dma_mask = &dwc3_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	}
};
EXPORT_SYMBOL(drime4_device_usb_dwc3);


