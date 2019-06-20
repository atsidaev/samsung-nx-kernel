/**
 * @file dev-opener.c
 * @brief DRIMe4 Driver Opener Device File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/opener/d4_opener.h>

struct platform_device drime4_device_opener = {
   .name	    = OPENER_MODULE_NAME,
   .id		    = -1,
   .num_resources   = 0,
   .dev		    = { },
};

