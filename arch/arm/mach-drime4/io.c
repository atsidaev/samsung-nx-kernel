/* linux/arch/arm/mach-drime4/io.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *      http://www.samsung.com/
 *
 * DRIME4 - IO support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/io.h>

#include <mach/map.h>
#include <asm/page.h>
#include <asm/mach/map.h>

#include <mach/common.h>

static struct map_desc drime4_io_desc[] __initdata = {
	{
		.virtual    = (unsigned long)DRIME4_VA_UART,
		.pfn        = __phys_to_pfn(DRIME4_PA_UART),
		.length     = SZ_4K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_TIMER,
		.pfn        = __phys_to_pfn(DRIME4_PA_TIMER),
		.length     = SZ_4K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_PLATFORM_CTRL,
		.pfn        = __phys_to_pfn(DRIME4_PA_PLATFORM_CTRL),
		.length     = SZ_4K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_RESET_CTRL,
		.pfn        = __phys_to_pfn(DRIME4_PA_RESET_CTRL),
		.length     = SZ_512,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_CLOCK_CTRL,
		.pfn        = __phys_to_pfn(DRIME4_PA_CLOCK_CTRL),
		.length     = SZ_256,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_SD_CFG,
		.pfn        = __phys_to_pfn(DRIME4_PA_SD_CFG),
		.length     = SZ_4K + SZ_16,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_GLOBAL_CTRL,
		.pfn        = __phys_to_pfn(DRIME4_PA_GLOBAL_CTRL),
		.length     = SZ_4K + SZ_32,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_L2C_CTRL,
		.pfn        = __phys_to_pfn(DRIME4_PA_L2C_CTRL),
		.length     = SZ_4K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_LS_DDR_CTRL,
		.pfn        = __phys_to_pfn(DRIME4_PA_LS_DDR_CTRL),
		.length     = SZ_64K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_HS_DDR_CTRL,
		.pfn        = __phys_to_pfn(DRIME4_PA_HS_DDR_CTRL),
		.length     = SZ_64K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_SONICS_VIC_CTRL,
		.pfn        = __phys_to_pfn(DRIME4_PA_SONICS_VIC_CTRL),
		.length     = SZ_4K + SZ_8K,
		.type       = MT_DEVICE
	},
	{ /* added by kh.moon for PP 3A Interrupt SW workaround at 2011.10.01 */
		.virtual    = (unsigned long)DRIME4_VA_PP,
		.pfn        = __phys_to_pfn(DRIME4_PA_PP),
		.length     = SZ_4K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_GPIO,
		.pfn        = __phys_to_pfn(DRIME4_PA_GPIO),
		.length     = SZ_128K,
		.type       = MT_DEVICE
	},
	{
		.virtual = (unsigned long)DRIME4_VA_WDT,
		.pfn = __phys_to_pfn(DRIME4_PA_WDT), 
		.length = SZ_16,
		.type = MT_DEVICE
	},
	{
		.virtual = (unsigned long)DRIME4_VA_CPU_SYS,
		.pfn = __phys_to_pfn(DRIME4_PA_CPU_SYS), 
		.length = SZ_32,
		.type = MT_DEVICE
	},

		
#ifdef	CONFIG_SCORE_SUSPEND
	{
		.virtual    = (unsigned long)DRIME4_VA_SONICS_MSRAM,
		.pfn        = __phys_to_pfn(DRIME4_PA_SONICS_MSRAM),
		.length     = SZ_128K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_DP,
		.pfn        = __phys_to_pfn(DRIME4_PA_DP),
		.length     = SZ_64K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_A9MP,
		.pfn        = __phys_to_pfn(DRIME4_PA_A9MP),
		.length     = SZ_64K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_PMU,
		.pfn        = __phys_to_pfn(DRIME4_PA_PMU),
		.length     = SZ_64K,
		.type       = MT_DEVICE
	},
	{
		.virtual    = (unsigned long)DRIME4_VA_SONICS_CONF,
		.pfn        = __phys_to_pfn(DRIME4_PA_SONICS_CONF),
		.length     = SZ_64K,
		.type       = MT_DEVICE
	},
#endif	/* CONFIG_SCORE_SUSPEND */
};

void __init drime4_map_io(void)
{
	iotable_init(drime4_io_desc, ARRAY_SIZE(drime4_io_desc));
}
