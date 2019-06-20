/**
 * @file dev-csm.c
 * @brief DRIMe4 CSM(Capture Sequence Manager) Platform Device File
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/csm/d4_csm.h>

struct platform_device drime4_device_csm = {
   .name	    = CSM_MODULE_NAME,
   .id		    = -1,
   .num_resources   = 0,
   .dev		    = { },
};
