/* linux/arch/arm/mach-drime4/include/mach/debug-macro.S
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - Debug macro
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <mach/map.h>

	.macro	addruart,rp,rv, tmp
		ldr	\rp, =DRIME4_PA_UART	@ physical address
		ldr	\rv, =DRIME4_VA_UART	@ virtual address
	.endm

#include <asm/hardware/debug-pl01x.S>
