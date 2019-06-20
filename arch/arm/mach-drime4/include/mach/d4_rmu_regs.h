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
#ifndef __MACH_DRIME4_RMU_REGS_H
#define __MACH_DRIME4_RMU_REGS_H

#include <mach/d4_reg_macro.h>


/*
 * Register offsets
 */
#define SWREST_IP           (0x00)
#define SWREST_PLL          (0x04)
#define HW_STS              (0x20)
#define PLL_MON             (0x24)
#define PCU_CTRL            (0x100)
#define PCU_STATE           (0x104)
#define PCU_LATCH           (0x108)
#define PCU_INT             (0x10C)


/*
 * Macros to reset IPs
 */
#define RST_MM(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define RST_DDR(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define RST_GPU(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define RST_PP(val, x) \
    SET_REGISTER_VALUE(val, x, 5, 1)
#define RST_IPCM(val, x) \
    SET_REGISTER_VALUE(val, x, 6, 1)
#define RST_IPCS(val, x) \
    SET_REGISTER_VALUE(val, x, 7, 1)
#define RST_EP(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)
#define RST_BE(val, x) \
    SET_REGISTER_VALUE(val, x, 9, 1)
#define RST_DP(val, x) \
    SET_REGISTER_VALUE(val, x, 10, 1)
#define RST_JPEG(val, x) \
    SET_REGISTER_VALUE(val, x, 11, 1)
#define RST_CODEC(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)
#define RST_MIPI(val, x) \
    SET_REGISTER_VALUE(val, x, 14, 1)
#define RST_OTF(val, x) \
    SET_REGISTER_VALUE(val, x, 15, 1)
#define RST_HDMI(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 1)
#define RST_SLVDS(val, x) \
    SET_REGISTER_VALUE(val, x, 23, 1)
#define RST_A9CPU(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)


/*
 * Macros to reset PLLs
 */
#define RST_PLL_AUD(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define RST_PLL_ARM(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define RST_PLL_LCD(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define RST_PLL_SYS1(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define RST_PLL_SYS2(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)


/*
 * Macros to check HW status
 */
#define NAND_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define RSTN_PLL2(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define RSTN_PLL1(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define RSTN_APLL(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define RSTN_DDR(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define RSTN_CA9(val, x) \
    SET_REGISTER_VALUE(val, x, 5, 1)
#define RSTN_MSYS(val, x) \
    SET_REGISTER_VALUE(val, x, 6, 1)
#define RSTN_PERI(val, x) \
    SET_REGISTER_VALUE(val, x, 7, 1)
#define RSTN_PWON(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)
#define PLL_READY(val, x) \
    SET_REGISTER_VALUE(val, x, 9, 1)
#define DFTMON_SPLL2(val, x) \
    SET_REGISTER_VALUE(val, x, 10, 1)
#define DFTMON_SPLL1(val, x) \
    SET_REGISTER_VALUE(val, x, 11, 1)
#define DFTMON_LPLL(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)
#define DFTMON_CA9(val, x) \
    SET_REGISTER_VALUE(val, x, 13, 2)
#define SYS_PCURM(val, x) \
    SET_REGISTER_VALUE(val, x, 15, 1)
#define SYS_EXMST(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 1)
#define SYS_DDRSEL(val, x) \
    SET_REGISTER_VALUE(val, x, 17, 1)
#define SYS_A9XTAL(val, x) \
    SET_REGISTER_VALUE(val, x, 18, 1)
#define SYS_REMAP(val, x) \
    SET_REGISTER_VALUE(val, x, 19, 2)
#define SYS_MODE(val, x) \
    SET_REGISTER_VALUE(val, x, 21, 1)
#define PCURSTN(val, x) \
    SET_REGISTER_VALUE(val, x, 22, 1)
#define SW_RSTN(val, x) \
    SET_REGISTER_VALUE(val, x, 23, 1)
#define SYS_CFG(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 7)
#define WDT_RSTN(val, x) \
    SET_REGISTER_VALUE(val, x, 31, 1)


/*
 * Macros to monitor PLLs
 */
#define DMON_ARMPLL(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define DMON_SYSPLL2(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define DMON_SYSPLL1(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define DMON_LCDPLL(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define DMON_AUDPLL(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)


/*
 * Macros to control PCU registers
 */
#define PCU_DCON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define PCU_DRSTA(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define PCU_DRST0(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define PCU_DRST1(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define PCU_DRST2(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define PCU_DRST3(val, x) \
    SET_REGISTER_VALUE(val, x, 5, 1)
#define PCU_DRST4(val, x) \
    SET_REGISTER_VALUE(val, x, 6, 1)
#define PCU_DPOH(val, x) \
    SET_REGISTER_VALUE(val, x, 7, 1)
#define PCU_CLK_REG(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)
#define PCU_CLK_MODE(val, x) \
    SET_REGISTER_VALUE(val, x, 9, 1)
#define PCU_DSLPW(val, x) \
    SET_REGISTER_VALUE(val, x, 10, 1)
#define PCU_DSLPW_CHK(val) \
    GET_REGISTER_VALUE(val, 10, 1)
#define PCU_DDRDPOH(val, x) \
    SET_REGISTER_VALUE(val, x, 11, 1)


/*
 * Macros to check PCU state
 */
#define PCU_STATER(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 5)
#define PCU_LATCHR(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 5)
#define PCU_INTR(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define PCU_DSLPRR(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)


#endif  /* __MACH_DRIME4_HT_PWM_REGS_H */

