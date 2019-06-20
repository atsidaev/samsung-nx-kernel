/**
 * @file dev-jxr.c
 * @brief DRIMe4 JPEG_XR Platform Device File
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/jxr/d4_jxr.h>

static struct resource drime4_jxr_resource[] = {
   [0] = {
      .start = DRIME4_PA_JPEG + 0x2000,
      .end   = DRIME4_PA_JPEG + 0x2000 + DRIME4_JPEG_XR_SIZE,
      .flags = IORESOURCE_MEM,
   },
   [1] = {
      .start = IRQ_JPEG,
      .end   = IRQ_JPEG,
      .flags = IORESOURCE_IRQ,
   }
};

static struct drime4_jxr_dev_data jxr_device_data = {
   .param0 = 0x12345678,
   .param1 = 0x44444444,
   .param2 = 0x88888888,
   .param3 = 0xcccccccc
};

struct platform_device drime4_device_jxr = {
   .name		    = JXR_MODULE_NAME,
   .id		        = -1,
   .num_resources	= ARRAY_SIZE(drime4_jxr_resource),
   .resource	    = drime4_jxr_resource,
   .dev		    = {
   .platform_data = &jxr_device_data,
   },
};

