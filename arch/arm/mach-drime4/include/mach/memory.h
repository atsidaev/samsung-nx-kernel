/* linux/arch/arm/mach-drime4/include/mach/memory.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - Memory definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_MEMORY_H
#define __MACH_DRIME4_MEMORY_H

/*
 * Physical DRAM offset.
 */
#define PLAT_PHYS_OFFSET		UL(0xC0000000)
#define MEM_SIZE			UL(0x10000000)
#define CONSISTENT_DMA_SIZE		(SZ_32M + SZ_8M + SZ_4M + SZ_2M)

#endif
