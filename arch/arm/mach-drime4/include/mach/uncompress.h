/* linux/arch/arm/mach-drime4/include/mach/uncompress.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Cloned from linux/include/asm-arm/arch-integrator/uncompress.h
 *
 * DRIME4 - uncompress definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_UNCOMPRESS_H
#define __MACH_DRIME4_UNCOMPRESS_H

#define AMBA_UART_DR	(*(volatile unsigned char *)0x30040000)
#define AMBA_UART_LCRH	(*(volatile unsigned char *)0x3004002c)
#define AMBA_UART_CR	(*(volatile unsigned char *)0x30040030)
#define AMBA_UART_FR	(*(volatile unsigned char *)0x30040018)

/*
 * This does not append a newline
 */
static void putc(int c)
{
	while (AMBA_UART_FR & (1 << 5))
		barrier();

	AMBA_UART_DR = c;
}

static inline void flush(void)
{
	while (AMBA_UART_FR & (1 << 3))
		barrier();
}

/*
 * nothing to do
 */
#define arch_decomp_setup()

#define arch_decomp_wdog()

#endif
