/* linux/arch/arm/mach-drime4/dev-pwm.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	kyuchun han <kyuchun.han@samsung.com>
 *
 * Base DRIME4 platform device definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include <mach/map.h>

struct gpio_reg_info_data {
	unsigned int phy_start;
	unsigned int phy_end;
};

#define UGPIO_RES(_no, _add)	\
	(struct gpio_reg_info_data [1]) { \
		{					\
			.phy_start = _add + (DRIME4_GPIO_OFFSET*_no),			\
			.phy_end	= _add + (DRIME4_GPIO_OFFSET*(_no + 1)) - 1,	\
		},	\
	}

#define DEFINE_GPIO_UDEVICE(_group_no, _add)			\
	.name		= "gpiodev",		\
	.id		= _group_no,			\
	.dev		= {					\
			.platform_data	= UGPIO_RES(_group_no, _add),	\
	},	\


struct platform_device gpio_udevice[] = {
		[0] = {DEFINE_GPIO_UDEVICE(0, DRIME4_PA_GPIO)},
		[1] = {DEFINE_GPIO_UDEVICE(1, DRIME4_PA_GPIO)},
		[2] = {DEFINE_GPIO_UDEVICE(2, DRIME4_PA_GPIO)},
		[3] = {DEFINE_GPIO_UDEVICE(3, DRIME4_PA_GPIO)},
		[4] = {DEFINE_GPIO_UDEVICE(4, DRIME4_PA_GPIO)},
		[5] = {DEFINE_GPIO_UDEVICE(5, DRIME4_PA_GPIO)},
		[6] = {DEFINE_GPIO_UDEVICE(6, DRIME4_PA_GPIO)},
		[7] = {DEFINE_GPIO_UDEVICE(7, DRIME4_PA_GPIO)},
		[8] = {DEFINE_GPIO_UDEVICE(8, DRIME4_PA_GPIO)},
		[9] = {DEFINE_GPIO_UDEVICE(9, DRIME4_PA_GPIO)},
		[10] = {DEFINE_GPIO_UDEVICE(10, DRIME4_PA_GPIO)},
		[11] = {DEFINE_GPIO_UDEVICE(11, DRIME4_PA_GPIO)},
		[12] = {DEFINE_GPIO_UDEVICE(12, DRIME4_PA_GPIO)},
		[13] = {DEFINE_GPIO_UDEVICE(13, DRIME4_PA_GPIO)},
		[14] = {DEFINE_GPIO_UDEVICE(14, DRIME4_PA_GPIO)},
		[15] = {DEFINE_GPIO_UDEVICE(15, DRIME4_PA_GPIO)},
		[16] = {DEFINE_GPIO_UDEVICE(16, DRIME4_PA_GPIO)},
		[17] = {DEFINE_GPIO_UDEVICE(17, DRIME4_PA_GPIO)},
		[18] = {DEFINE_GPIO_UDEVICE(18, DRIME4_PA_GPIO)},
		[19] = {DEFINE_GPIO_UDEVICE(19, DRIME4_PA_GPIO)},
		[20] = {DEFINE_GPIO_UDEVICE(20, DRIME4_PA_GPIO)},
		[21] = {DEFINE_GPIO_UDEVICE(21, DRIME4_PA_GPIO)},
		[22] = {DEFINE_GPIO_UDEVICE(22, DRIME4_PA_GPIO)},
		[23] = {DEFINE_GPIO_UDEVICE(23, DRIME4_PA_GPIO)},
		[24] = {DEFINE_GPIO_UDEVICE(24, DRIME4_PA_GPIO)},
		[25] = {DEFINE_GPIO_UDEVICE(25, DRIME4_PA_GPIO)},
		[26] = {DEFINE_GPIO_UDEVICE(26, DRIME4_PA_GPIO)},
		[27] = {DEFINE_GPIO_UDEVICE(27, DRIME4_PA_GPIO)}, };

EXPORT_SYMBOL(gpio_udevice);

