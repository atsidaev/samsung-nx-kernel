/* arch/arm/mach-drime4/include/mach/watchdog-reset.h
 *
 * Copyright (c) 2012 Samsung Electronics
  *
  *
 * arch_reset() function
  *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <mach/map.h>

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/delay.h>

#define DRIME4_WDOGREG(x) ((x) + DRIME4_VA_WDT)
#define DRIME4_WDTCR		 DRIME4_WDOGREG(0x0)
#define	DRIME4_WDTPSR		DRIME4_WDOGREG(0x4)
#define DRIME4_WDTLDR		DRIME4_WDOGREG(0x8)
#define DRIME4_WDTVLR		DRIME4_WDOGREG(0xC)
#define DRIME4_WDTISR		DRIME4_WDOGREG(0x10)

/*reset all system after 0.4 msec */
static inline void arch_wdt_reset(void)
{
	/* disable watchdog */
	__raw_writel(0, DRIME4_WDTCR);

	/* Scale Set */
	__raw_writel(0x80, DRIME4_WDTPSR);
	__raw_writel(0xFF, DRIME4_WDTLDR);

	/* watchdog reset */
	__raw_writel(0x0007, DRIME4_WDTCR);

//		printk("%s: End \n", __func__);

	mdelay(500);
}


