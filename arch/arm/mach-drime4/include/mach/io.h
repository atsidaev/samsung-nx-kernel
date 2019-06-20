/* linux/arch/arm/mach-drime4/include/mach/io.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Cloned from linux/include/asm-arm/arch-integrator/io.h
 *
 * DRIME4 - IO definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_IO_H
#define __MACH_DRIME4_IO_H

#define __io(a)		__typesafe_io(a)
#define __mem_pci(a)		(a)

#define IO_SPACE_LIMIT	0xFFFFFFFF

#endif
