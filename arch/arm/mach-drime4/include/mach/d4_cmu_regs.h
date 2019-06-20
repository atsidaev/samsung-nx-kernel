/**
 * arch/arm/mach-drime4/include/mach/d4_cmu_regs.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __MACH_DRIME4_CMU_REGS_H
#define __MACH_DRIME4_CMU_REGS_H


/*
 * Register offsets
 */
#define GCLKSEL1        0x0
#define GCLKSEL2        0x4
#define GCLKSEL3        0x8
#define GCLKSEL4        0xC
#define GCLKSEL5        0x10
#define GCLKCON1        0x20
#define GCLKCMD1        0x28
#define SYSPLL1_CON1    0x30
#define SYSPLL1_CON2    0x34
#define SYSPLL2_CON1    0x38
#define SYSPLL2_CON2    0x3c
#define LCDPLL_CON1     0x40
#define LCDPLL_CON2     0x44
#define ARMPLL_CON1     0x48
#define ARMPLL_CON2     0x4C
#define AUDPLL_CON1     0x58
#define AUDPLL_CON2     0x5C
#define PLL_LOCK_STS    0x80
#define XTLSRC_SEL      0x84


#endif  /* __MACH_DRIME4_CMU_REGS_H */
