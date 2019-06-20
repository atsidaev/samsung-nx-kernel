/**
 * arch/arm/mach-drime4/include/mach/pm.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * Based on arch/arm/plat-s3c/include/plat/pm.h,
 * Copyright 2008-2009 Simtec Electronics Ben Dooks <ben@simtec.co.uk>
 * S5P series device definition for power management
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MACH_DRIME4_PM_H
#define __MACH_DRIME4_PM_H


/**
 * Power configuration size
 *  Byte size to save/restore power configurations.
 * TODO
 *  Need to be updated according to power policy.
 */
#define POWER_CONF_SZ   (sizeof(unsigned int) * 8)


/**
 * struct pm_core_save - Save block for platform device's registers.
 * @reg: Pointer to the register to save.
 * @val: Holder for the value saved in reg.
 *
 * This describes a list of registers which is used by the pm core and
 * other subsystems to save and restore register values over suspend.
 */
struct pm_core_save {
    void __iomem    *reg;
    unsigned long   val;
};

#define SAVE_ITEM(x)    { .reg = (x) }


/**
 * struct pm_vic_save - Save block for VIC registers.
 *
 * Save block for VIC registers to be held over sleep and restored if they
 * are needed. (DDI0273A_VIC_PL192.pdf)
 */
struct pm_vic_save {
    u32 vic_irq_status;
    u32 vic_fiq_status;
    u32 vic_raw_status;
    u32 vic_int_select;
    u32 vic_int_enable;
    u32 vic_int_soft;
    u32 vic_protect;
};


/**
 * struct pm_uart_save - Save block for UART registers.
 *
 * Save block for UART registers to be held over sleep and restored if they
 * are needed. (DDI0183G_uart_pl011_r1p5_trm.pdf)
 */
struct pm_uart_save {
    u32 uartdr;
    u32 uartrsr;
    u32 uartilpr;
    u32 uartibrd;
    u32 uartfbrd;
    u32 uartlcr_h;
    u32 uarttcr;
    u32 uartifls;
    u32 uartimsc;
    u32 uartdmacr;
};


/**
 * struct pm_gpio_save - Save block for GPIO registers.
 *
 * Save block for GPIO registers to be held over sleep and restored if they
 * are needed. (DDI0190B_gpio_pl061_trm.pdf)
 */
struct pm_gpio_save {
    u8 gpiodata;
    u8 gpiodir;
    u8 gpiois;
    u8 gpioibe;
    u8 gpioiev;
    u8 gpioie;
};


/**
 * Register offset of GPIO(PL061)
 */
#define PL061_GPIODIR 0x400
#define PL061_GPIOIS  0x404
#define PL061_GPIOIBE 0x408
#define PL061_GPIOIEV 0x40c
#define PL061_GPIOIE  0x410
#define PL061_GPIORIS 0x414
#define PL061_GPIOMIS 0x418
#define PL061_GPIOIC  0x41c


/**
 * Register offsets of DP
 */
#define DP_LCD_CLK_EN       0x004
#define DP_INTR_EN          0x010
#define DP_BACKGROUND_MONO  0x714
#define DP_BACKGROUND_COLOR 0x71c


/**
 * Register offsets of PMU
 */
#define PMU_STOP_REQ        0x10
#define PMU_ISO_EN          0x30
#define PMU_SCACK	    0x3c


/**
 * Extern functions
 */
/* From arch/arm/mach-drime4/pm.c */
#ifdef  CONFIG_PM
extern int __init drime4_pm_init(void);
#else   // CONFIG_PM
static inline int drime4_pm_init(void)
{
    return 0;
}
#endif  // CONFIG_PM

/* From arch/arm/mach-drime4/sleep.S */
extern int drime4_cpu_save(unsigned long saveblk);

#endif  // __MACH_DRIME4_PM_H

