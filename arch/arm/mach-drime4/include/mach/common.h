/* linux/arch/arm/mach-drime4/include/mach/common.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - Board Common definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_COMMON_H
#define __MACH_DRIME4_COMMON_H

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/amba/bus.h>
#include <linux/types.h>
#include <linux/io.h>
#include <asm/irq.h>
#include <mach/map.h>
#include <mach/irqs.h>
#include <mach/clock.h>

void __init drime4_map_io(void);
void __init drime4_init_irq(void);
void __init drime4_init_early(void);
void __init drime4_init_common_devices(void);
int __init drime4_clk_init(void);
void __init drime4_init_restart(char mode, const char *cmd);

extern struct sys_timer drime4_timer;

#endif
