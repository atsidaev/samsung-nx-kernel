/**
 * @file dev-bwm.c
 * @brief DRIMe4 BWM(Bandwidth Manager) Platform Device File
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <mach/map.h>
#include <mach/bwm/d4_bwm.h>

static struct resource drime4_bwm_resource[] = {
	[0] = {
		.start = DRIME4_PA_LS_DDR_CTRL,
		.end   = DRIME4_PA_LS_DDR_CTRL + DRIME4_DREX_CTRL_SIZE - 1,
		.flags = IORESOURCE_MEM,
	}
};

struct platform_device drime4_device_bwm = {
   .name	    		= BWM_MODULE_NAME,
   .id		    		= -1,
   .num_resources   	= ARRAY_SIZE(drime4_bwm_resource),
   .resource	    	= drime4_bwm_resource,
   .dev		    		= {

   },
};
