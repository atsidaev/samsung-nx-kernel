/* linux/arch/arm/mach-drime4/common.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - Board Common Support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <mach/common.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/version_information.h>
#include <mach/watchdog-reset.h>
#include <asm/system_misc.h>

static struct amba_device uart0_device = {
	.dev = { .init_name = "uart0" },
	.res		= {
		.start	= DRIME4_PA_UART,
		.end	= DRIME4_PA_UART + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	.irq		= { IRQ_UART0 },
	.periphid	= 0,
};

static struct amba_device *amba_devs[] __initdata = {
	&uart0_device,
};

static void __init drime4_init_cache(void)
{
#ifdef CONFIG_CACHE_L2X0
	void __iomem *l2x0_base;
	__u32 reg;

	l2x0_base = DRIME4_VA_L2C_CTRL;

	/* Prefetch Control register */
	reg = readl_relaxed(l2x0_base + 0xf60);
	reg |= (0x1 << 30);
	reg |= (0x1 << 29);
	reg |= (0x1 << 24);
	reg |= (0x1 << 23);
	reg &= ~(0x1f << 0);
	writel_relaxed(reg, l2x0_base + 0xf60);

	/* TAG RAM Control register */
	reg = readl_relaxed(l2x0_base + 0x108);
	reg &= ~(0x7 << 8);
	reg &= ~(0x7 << 4);
	reg &= ~(0x7 << 0);
	writel_relaxed(reg, l2x0_base + 0x108);
	/* DATA RAM Control register */
	reg = readl_relaxed(l2x0_base + 0x10C);
	reg &= ~(0x7 << 8);
	reg &= ~(0x7 << 4);
	reg &= ~(0x7 << 0);
	writel_relaxed(reg, l2x0_base + 0x10C);

	asm("dmb");
	asm("dsb");
	asm("isb");

	l2x0_init(l2x0_base, 0x40000001, 0x0);
#endif
}

void __init drime4_init_early(void)
{
	drime4_add_clk_table();
	drime4_init_cache();
}

void __init drime4_init_common_devices(void)
{
	int i;

	VersionInfo_Initialize();

	for (i = 0; i < ARRAY_SIZE(amba_devs); i++) {
		struct amba_device *d = amba_devs[i];
		amba_device_register(d, &iomem_resource);
	}
}


void __init drime4_init_restart(char mode, const char *cmd)
{
	arch_wdt_reset();
}

  
