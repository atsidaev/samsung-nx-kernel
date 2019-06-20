/* arch/arm/mach-drime4/include/mach/regs-timer.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Drime4 TIMER configutation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_REGS_TIMER_H
#define __ASM_ARCH_REGS_TIMER_H __FILE__

#include <mach/map.h>

#define DRIME4_TIMER_BASE(x)		DRIME4_TIMER(x)

#define TIMER_CTR_OFFSET		(0x00)
#define TIMER_PSR_OFFSET		(0x04)
#define TIMER_LDR_OFFSET		(0x08)
#define TIMER_ISR_OFFSET		(0x0C)

/* Timer control register */
#define TIMER_CTR_ENABLE_TIMER		(1 << 0)	/* Enable Timer */
#define TIMER_CTR_ENABLE_TIMER_IRQ	(1 << 1)	/* Enable interrupt */
/* Enable timer outputs	*/
#define TIMER_CTR_ENABLE_TIMER_OUT	(1 << 2)
#define TIMER_CTR_UP_COUNTING		(1 << 4)	/* Up counting	*/

/* Up/down is controlled by
EXTUD[4:0] input registeri	*/
#define TIMER_CTR_EXTUD			(1 << 5)
#define TIMER_CTR_PULSE_MODE		(1 << 6)	/* Pulse mode */

/* Free running timer mode	*/
#define TIMER_CTR_FREE_RUNNING_MODE	(0 << 8)
/* Periodic timer mode */
#define TIMER_CTR_PERIODIC_MODE		(1 << 8)

#endif /* __ASM_ARCH_REGS_TIMER_H */
