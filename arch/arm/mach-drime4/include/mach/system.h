/* linux/arch/arm/mach-drime4/include/mach/system.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Based on linux/include/asm-arm/arch-integrator/system.h
 *
 * DRIME4 - system support header
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_SYSTEM_H
#define __MACH_DRIME4_SYSTEM_H

#include <mach/watchdog-reset.h>

static inline void arch_idle(void)
{
	/*
	 * This should do all the clock switching
	 * and wait for interrupt tricks
	 */
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	/*
	 * To reset, we hit the on-board reset register
	 * in the system FPGA
	 * Currently nothing here
	 */
 	arch_wdt_reset();
}

#endif
