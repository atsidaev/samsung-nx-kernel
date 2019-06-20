/* linux/arch/arm/mach-drime4/include/mach/gpio.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - GPIO support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_GPIO_H
#define __MACH_DRIME4_GPIO_H

#define gpio_get_value		__gpio_get_value
#define gpio_set_value		__gpio_set_value
#define gpio_cansleep		__gpio_cansleep
#define gpio_to_irq		__gpio_to_irq

#define DRIME4_GPIO_NR		8
#define DRIME4_GPIO_GROUP_NR	28

#define DRIME4_GPIO0(nr)	(nr)
#define DRIME4_GPIO1(nr)	(DRIME4_GPIO_NR + (nr))
#define DRIME4_GPIO2(nr)	(DRIME4_GPIO_NR * 2 + (nr))
#define DRIME4_GPIO3(nr)	(DRIME4_GPIO_NR * 3 + (nr))
#define DRIME4_GPIO4(nr)	(DRIME4_GPIO_NR * 4 + (nr))
#define DRIME4_GPIO5(nr)	(DRIME4_GPIO_NR * 5 + (nr))
#define DRIME4_GPIO6(nr)	(DRIME4_GPIO_NR * 6 + (nr))
#define DRIME4_GPIO7(nr)	(DRIME4_GPIO_NR * 7 + (nr))
#define DRIME4_GPIO8(nr)	(DRIME4_GPIO_NR * 8 + (nr))
#define DRIME4_GPIO9(nr)	(DRIME4_GPIO_NR * 9 + (nr))
#define DRIME4_GPIO10(nr)	(DRIME4_GPIO_NR * 10 + (nr))
#define DRIME4_GPIO11(nr)	(DRIME4_GPIO_NR * 11 + (nr))
#define DRIME4_GPIO12(nr)	(DRIME4_GPIO_NR * 12 + (nr))
#define DRIME4_GPIO13(nr)	(DRIME4_GPIO_NR * 13 + (nr))
#define DRIME4_GPIO14(nr)	(DRIME4_GPIO_NR * 14 + (nr))
#define DRIME4_GPIO15(nr)	(DRIME4_GPIO_NR * 15 + (nr))
#define DRIME4_GPIO16(nr)	(DRIME4_GPIO_NR * 16 + (nr))
#define DRIME4_GPIO17(nr)	(DRIME4_GPIO_NR * 17 + (nr))
#define DRIME4_GPIO18(nr)	(DRIME4_GPIO_NR * 18 + (nr))
#define DRIME4_GPIO19(nr)	(DRIME4_GPIO_NR * 19 + (nr))
#define DRIME4_GPIO20(nr)	(DRIME4_GPIO_NR * 20 + (nr))
#define DRIME4_GPIO21(nr)	(DRIME4_GPIO_NR * 21 + (nr))
#define DRIME4_GPIO22(nr)	(DRIME4_GPIO_NR * 22 + (nr))
#define DRIME4_GPIO23(nr)	(DRIME4_GPIO_NR * 23 + (nr))
#define DRIME4_GPIO24(nr)	(DRIME4_GPIO_NR * 24 + (nr))
#define DRIME4_GPIO25(nr)	(DRIME4_GPIO_NR * 25 + (nr))
#define DRIME4_GPIO26(nr)	(DRIME4_GPIO_NR * 26 + (nr))
#define DRIME4_GPIO27(nr)	(DRIME4_GPIO_NR * 27 + (nr))

#define DRIME4_GPIO_END	(DRIME4_GPIO27(DRIME4_GPIO_NR) + 1)

#define ARCH_NR_GPIOS		(DRIME4_GPIO_END)

#include <asm-generic/gpio.h>

#if defined(CONFIG_MACH_D4ES)
#include "gpio_map_d4es.h"
#elif defined(CONFIG_MACH_D4JIG2ND)
#include "gpio_map_d4jig2nd.h"
#elif defined(CONFIG_MACH_D4_NX300)
#include "gpio_map_d4_nx300.h"
#elif defined(CONFIG_MACH_D4_NX2000)
#include "gpio_map_d4_nx2000.h"
#elif defined(CONFIG_MACH_D4_GALAXYNX)
#include "gpio_map_d4_galaxynx.h"
#elif defined(CONFIG_MACH_D4_GALAXYNX_SIMULATOR)
#include "gpio_map_d4_galaxynx_simulator.h"
#endif

#endif
