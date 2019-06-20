/* linux/arch/arm/mach-drime4/irq.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - IRQ support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/io.h>
#include <linux/irq.h>

#include <asm/hardware/vic.h>
#include <mach/hardware.h>
#include <mach/map.h>
#include <mach/irqs.h>
#include <mach/common.h>

void __init drime4_init_irq(void)
{
	drime4_clk_init();

	vic_init((void __iomem *)DRIME4_VA_VIC0, IRQ_INTC0_START,
		IRQ_INTC0_VALID_MASK, 0);
	vic_init((void __iomem *)DRIME4_VA_VIC1, IRQ_INTC1_START,
		IRQ_INTC1_VALID_MASK, 0);
	vic_init((void __iomem *)DRIME4_VA_VIC2, IRQ_INTC2_START,
		IRQ_INTC2_VALID_MASK, 0);
}
