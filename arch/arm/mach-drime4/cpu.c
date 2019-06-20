/* linux/arch/arm/mach-drime4/cpu.c
 *
 * Copyright (c) 2009-2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * DRIMe4 CPU Support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sysdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <mach/cpu.h>

struct sysdev_class drime4_sysclass = {
	.name	= "drime4-core",
};

static struct sys_device drime4_sysdev = {
	.cls	= &drime4_sysclass,
};

static int __init drime4_core_init(void)
{
	return sysdev_class_register(&drime4_sysclass);
}

core_initcall(drime4_core_init);