/*
 * arch/arm/mach-drime4/pm.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/

#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/serial_core.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/pinctrl/consumer.h>
#include <linux/d4_rmu.h>

#include <asm/cacheflush.h>
#include <asm/hardware/vic.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/suspend.h>
#include <mach/map.h>
#include <mach/cpu.h>
#include <mach/memory.h>
#include <mach/gpio.h>
#include <mach/d4_cma.h>
#include <mach/d4_rmu_regs.h>
#include <mach/d4_ddr_regs.h>
#include <mach/d4_cmu_regs.h>
#include <mach/d4_serial_regs.h>
#include <mach/d4_plat_ctrl_regs.h>
#include <mach/regs-timer.h>
#include <mach/pm.h>
#include <mach/gpio.h>

extern int oneshot;

/********************************************************************
 *                                                                  *
 *                    Debugging support for STR                     *
 *                                                                  *
 ********************************************************************/
#ifdef  CONFIG_PM_DEBUG
extern void printascii(const char *);

/**
 * @fn      DRIME4_PMDG
 * @brief   Print debugging message for STR test
 * @param   print-out format
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void DRIME4_PMDG(const char *fmt, ...)
{
#ifndef D4_PMDG_MAX
#define D4_PMDG_MAX 256
#endif
    va_list va;
    char buff[D4_PMDG_MAX];

    va_start(va, fmt);
    vsprintf(buff, fmt, va);
    va_end(va);

    //printascii(buff);
}
#else   // CONFIG_PM_DEBUG
#define DRIME4_PMDG(...)
#endif  // CONFIG_PM_DEBUG



/********************************************************************
 *                                                                  *
 *            Registers to be saved in drime4_pm_enter()            *
 *                                                                  *
 ********************************************************************/
/**
 * @var     drime4_cmu_save
 * @brief   Static variable to save register values of CMU
 *
 * @Commiter    Daeho Kim
 */
static struct pm_core_save drime4_cmu_save[] = { 
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + GCLKSEL1),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + GCLKSEL2),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + GCLKSEL3),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + GCLKSEL4),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + GCLKSEL5),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + GCLKCON1),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + SYSPLL1_CON1),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + SYSPLL1_CON2),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + SYSPLL2_CON1),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + SYSPLL2_CON2),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + LCDPLL_CON1),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + LCDPLL_CON2),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + ARMPLL_CON1),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + ARMPLL_CON2),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + AUDPLL_CON1),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + AUDPLL_CON2),
    SAVE_ITEM(DRIME4_VA_CLOCK_CTRL + XTLSRC_SEL),
};

/**
 * @var     drime4_plat_ctrl_save
 * @brief   Static variable to save values of platform ctrl registers
 *
 * @Commiter    Daeho Kim
 */
static struct pm_core_save drime4_plat_ctrl_save[] = { 
    SAVE_ITEM(DRIME4_VA_PLATFORM_CTRL + PLAT_NOR_CLK_EN),
    SAVE_ITEM(DRIME4_VA_PLATFORM_CTRL + PLAT_SPI_CLK_EN),
    SAVE_ITEM(DRIME4_VA_PLATFORM_CTRL + PLAT_I2C_CLK_EN),
    SAVE_ITEM(DRIME4_VA_PLATFORM_CTRL + PLAT_I2S_CLK_EN),
    SAVE_ITEM(DRIME4_VA_PLATFORM_CTRL + PLAT_ADC_CLK_EN),
};


/********************************************************************
 *                                                                  *
 *              Helper functions for drime4_pm_enter()              *
 *                                                                  *
 ********************************************************************/
/**
 * @fn      drime4_pm_sleep_flag
 * @brief   Set/Unset sleep flag in PCU control register.
 * @param   x
 *              1:set, 0:unset
 * @return  0 if success, -EINVAL otherwise.
 *
 * @Commiter    Daeho Kim
 */
static inline int drime4_pm_sleep_flag(unsigned int x)
{
    unsigned int reg_val, chk_val = 0;

    // Read & update PCU sleep flag
    reg_val = readl(DRIME4_VA_RESET_CTRL + PCU_CTRL);
    PCU_DSLPW(reg_val, x);
    writel(reg_val, DRIME4_VA_RESET_CTRL + PCU_CTRL);

    // Sync. L2 cache
    outer_sync();

    // Check the flag after update
    chk_val = readl(DRIME4_VA_RESET_CTRL + PCU_CTRL);
    if (PCU_DSLPW_CHK(chk_val) != x) {
        DRIME4_PMDG("%s(%d) value could not be set, chk_val = %x\n",
                        __func__, x, chk_val);
        return -EINVAL;
    }
    DRIME4_PMDG("%s(%d) PCU Ctrl Register value is %x\n",
                    __func__, x, chk_val);

    return 0;
}

/**
 * @fn      drime4_pm_do_save
 * @brief   Save a set of registers for restoration on resume.
 * @param   ptr: Pointer to an array of registers.
 *          count: Size of the ptr array.
 * @ret     none
 *
 * @Comment Run through the list of registers given, saving their contents 
 *              in the array for later restoration when we wakeup.
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_do_save(struct pm_core_save *ptr, int count)
{
    for (; count > 0; count--, ptr++)
        ptr->val = readl(ptr->reg);
}

/**
 * @fn      drime4_pm_do_restore
 * @brief   Restore register values from the save list.
 * @param   ptr: Pointer to an array of registers.
 *          count: Size of the ptr array.
 *
 * @Comment Restore the register values saved from drime4_pm_do_save().
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_do_restore(struct pm_core_save *ptr, int count)
{
    for (; count > 0; count--, ptr++)
        writel(ptr->val, ptr->reg);
}

/**
 * @fn      drime4_pm_save_core
 * @brief   Save register values for platform devices.
 * @param   none
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_save_core(void)
{
    drime4_pm_do_save(drime4_plat_ctrl_save, ARRAY_SIZE(drime4_plat_ctrl_save));
    drime4_pm_do_save(drime4_cmu_save, ARRAY_SIZE(drime4_cmu_save));
}

/**
 * @fn      drime4_pm_restore_core
 * @brief   Restore register values for platform devices.
 * @param   none
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_restore_core(void)
{
    drime4_pm_do_restore(drime4_cmu_save, ARRAY_SIZE(drime4_cmu_save));
    drime4_pm_do_restore(drime4_plat_ctrl_save, ARRAY_SIZE(drime4_plat_ctrl_save));
}


/********************************************************************
 *                                                                  *
 *         Variables & functions for saving/restoring VICs          *
 *                                                                  *
 ********************************************************************/
/**
 * @var     drime4_vic_save
 * @brief   Static variable to save register values of VICs
 *
 * @Commiter    Daeho Kim
 */
struct pm_vic_save drime4_vic_save[CONFIG_ARM_VIC_NR];

/**
 * @fn      drime4_pm_save_vic
 * @brief   Restore register values for VIC
 * @param   vic: Index of VIC
 *          save: Pointer to the saving storage
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_save_vic(unsigned int vic, struct pm_vic_save *save)
{
    void __iomem *regs = DRIME4_VA_VIC(vic);

    save->vic_irq_status    = readl(regs + VIC_IRQ_STATUS);
    save->vic_fiq_status    = readl(regs + VIC_FIQ_STATUS);
    save->vic_raw_status    = readl(regs + VIC_RAW_STATUS);
    save->vic_int_select    = readl(regs + VIC_INT_SELECT);
    save->vic_int_enable    = readl(regs + VIC_INT_ENABLE);
    save->vic_int_soft      = readl(regs + VIC_INT_SOFT);
    save->vic_protect       = readl(regs + VIC_PROTECT);
}

/**
 * @fn      drime4_pm_save_vics
 * @brief   Save register values for VICs
 * @param   none
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_save_vics(void)
{
    struct pm_vic_save *save = drime4_vic_save;
    unsigned int vic;

    for (vic = 0; vic < CONFIG_ARM_VIC_NR; vic++, save++)
        drime4_pm_save_vic(vic, save);
}

/**
 * @fn      drime4_pm_restore_vic
 * @brief   Restore register values for VIC
 * @param   vic: Index of VIC
 *          save: Pointer to the saving storage
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_restore_vic(unsigned int vic, struct pm_vic_save *save)
{
    void __iomem *regs = DRIME4_VA_VIC(vic);

    writel(save->vic_irq_status, regs + VIC_IRQ_STATUS);
    writel(save->vic_fiq_status, regs + VIC_FIQ_STATUS);
    writel(save->vic_raw_status, regs + VIC_RAW_STATUS);
    writel(save->vic_int_select, regs + VIC_INT_SELECT);
    writel(save->vic_int_enable, regs + VIC_INT_ENABLE);
    writel(save->vic_int_soft, regs + VIC_INT_SOFT);
    writel(save->vic_protect, regs + VIC_PROTECT);
}

/**
 * @fn      drime4_pm_restore_vics
 * @brief   Restore register values for VICs
 * @param   none
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_restore_vics(void)
{
    struct pm_vic_save *save = drime4_vic_save;
    unsigned int vic;

    for (vic = 0; vic < CONFIG_ARM_VIC_NR; vic++, save++)
        drime4_pm_restore_vic(vic, save);
    
}

/**
 * @fn      drime4_pm_disable_vics
 * @brief   Disable VICs
 * @param   none
 * @return  none
 *
 * @Comment It should be called after L2 cache is turned off.
 *              So, writel_relaxed() is used instead of writel().
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_disable_vics(void)
{
    unsigned int vic;
    void __iomem *base;

    for (vic = 0; vic < CONFIG_ARM_VIC_NR; vic++) {
        base = DRIME4_VA_VIC(vic);

        writel_relaxed(0, base + VIC_INT_SELECT);
        writel_relaxed(0, base + VIC_INT_ENABLE);
        writel_relaxed(~0, base + VIC_INT_ENABLE_CLEAR);
        writel_relaxed(0, base + VIC_INT_SOFT);
        writel_relaxed(~0, base + VIC_INT_SOFT_CLEAR);
        writel_relaxed(0, base + VIC_PROTECT);
        writel_relaxed(0, base + VIC_ITCR);

        writel_relaxed(0, base + VIC_PL192_VECT_ADDR);
    }
}


/********************************************************************
 *                                                                  *
 *         Variables & functions for saving/restoring UARTs         *
 *                                                                  *
 ********************************************************************/
/**
 * TODO
 * It should be defined at kernel configuration file
 */
#ifndef CONFIG_SERIAL_AMBA_PL011_DRIME4_UARTS
#define CONFIG_SERIAL_AMBA_PL011_DRIME4_UARTS   1
#endif

/**
 * @var     drime4_uart_save
 * @brief   Static variable to save register values of UARTs
 *
 * @Commiter    Daeho Kim
 */
struct pm_uart_save drime4_uart_save[CONFIG_SERIAL_AMBA_PL011_DRIME4_UARTS];

/**
 * @fn      drime4_pm_save_uart
 * @brief   Save register values for UART
 * @param   uart: Index of UART
 *          save: Pointer to the saving storage
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_save_uart(unsigned int uart, struct pm_uart_save *save)
{
    void __iomem *regs = DRIME4_VA_UARTx(uart);

    save->uartdr    = readl(regs + PL011_UARTDR);
    save->uartrsr   = readl(regs + PL011_UARTRSR);
    save->uartilpr  = readl(regs + PL011_UARTILPR);
    save->uartibrd  = readl(regs + PL011_UARTIBRD);
    save->uartlcr_h = readl(regs + PL011_UARTLCR_H);
    save->uarttcr   = readl(regs + PL011_UARTTCR);
    save->uartifls  = readl(regs + PL011_UARTIFLS);
    save->uartimsc  = readl(regs + PL011_UARTIMSC);
    save->uartdmacr = readl(regs + PL011_UARTDMACR);
}

/**
 * @fn      drime4_pm_save_uarts
 * @brief   Save register values for UARTs
 * @param   none
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_save_uarts(void)
{
    struct pm_uart_save *save = drime4_uart_save;
    unsigned int uart;

    for (uart = 0; uart < CONFIG_SERIAL_AMBA_PL011_DRIME4_UARTS; uart++, save++)
        drime4_pm_save_uart(uart, save);
}

/**
 * @fn      drime4_pm_restore_uart
 * @brief   Restore register values for UART
 * @param   uart: Index of UART
 *          save: Pointer to the saving storage
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_restore_uart(unsigned int uart, struct pm_uart_save *save)
{
    void __iomem *regs = DRIME4_VA_UARTx(uart);

    writel(save->uartdr, regs + PL011_UARTDR);
    writel(save->uartrsr, regs + PL011_UARTRSR);
    writel(save->uartilpr, regs + PL011_UARTILPR);
    writel(save->uartibrd, regs + PL011_UARTIBRD);
    writel(save->uartlcr_h, regs + PL011_UARTLCR_H);
    writel(save->uarttcr, regs + PL011_UARTTCR);
    writel(save->uartifls, regs + PL011_UARTIFLS);
    writel(save->uartimsc, regs + PL011_UARTIMSC);
    writel(save->uartdmacr, regs + PL011_UARTDMACR);
}

/**
 * @fn      drime4_pm_restore_uarts
 * @brief   Restore register values for UARTs
 * @param   none
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_restore_uarts(void)
{
    struct pm_uart_save *save = drime4_uart_save;
    unsigned int uart;

    for (uart = 0; uart < CONFIG_SERIAL_AMBA_PL011_DRIME4_UARTS; uart++, save++)
        drime4_pm_restore_uart(uart, save);

}


/********************************************************************
 *                                                                  *
 *         Variables & functions for saving/restoring GPIOs         *
 *                                                                  *
 ********************************************************************/
/**
 * @var     drime4_gpio_save
 * @brief   Static variable to save register values of GPIOs
 *
 * @Commiter    Daeho Kim
 */
struct pm_gpio_save drime4_gpio_save[DRIME4_GPIO_GROUP_NR];

/**
 * @fn      drime4_pm_save_gpio
 * @brief   Save register values for GPIO
 * @param   gpio: Index of GPIO
 *          save: Pointer to the saving storage
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_save_gpio(unsigned int gpio, struct pm_gpio_save *save)
{
    void __iomem *regs = DRIME4_VA_GPIO_BASE(gpio);
    unsigned int offset;

    save->gpiodir   = readl(regs + PL061_GPIODIR);
    save->gpiois    = readl(regs + PL061_GPIOIS);
    save->gpioibe   = readl(regs + PL061_GPIOIBE);
    save->gpioiev   = readl(regs + PL061_GPIOIEV);
    save->gpioie    = readl(regs + PL061_GPIOIE);

    for (offset=0; offset < DRIME4_GPIO_NR; offset++) {
        if (save->gpiodir & (1 << offset))
            save->gpiodata |= gpio_get_value(gpio) << offset;
    }
}

#define DRIME4_GPIO_CTRL_SIZE	0x458

void drime4_pm_input_gpios(void)
{
    unsigned long offset, i, value;
	void __iomem *regs;

	/* all port gpio input */
	for (offset = 0; offset <= DRIME4_GPIO_CTRL_SIZE; offset += 4) {
		if (offset != 0x418)	/* GPIO25_7 oneshot */
			writel(0x20800, DRIME4_VA_GLOBAL_CTRL + offset);
	}

    for (i = 0; i < DRIME4_GPIO_GROUP_NR; i++) {
	    regs = DRIME4_VA_GPIO_BASE(i);
		if (i == 25) {
			writel(0x80, regs + PL061_GPIODIR);
		} else {
			writel(0, regs + PL061_GPIODIR);
		}
    }
}


/**
 * @fn      drime4_pm_save_gpios
 * @brief   Save register values for GPIOs
 * @param   none
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_save_gpios(void)
{
    struct pm_gpio_save *save = drime4_gpio_save;
    unsigned int gpio;

    for (gpio = 0; gpio < DRIME4_GPIO_GROUP_NR; gpio++, save++)
        drime4_pm_save_gpio(gpio, save);
}

/**
 * @fn      drime4_pm_restore_gpio
 * @brief   Restore register values for GPIO
 * @param   gpio: Index of GPIO
 *          save: Pointer to the saving storage
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_restore_gpio(unsigned int gpio, struct pm_gpio_save *save)
{
    void __iomem *regs = DRIME4_VA_GPIO_BASE(gpio);
    unsigned int offset;
    u8 value = 0;

    /**
     * TODO
     */
    for (offset=0; offset < DRIME4_GPIO_NR; offset++) {
        if (save->gpiodir & (1 << offset))
            value |= save->gpiodata & (1 << offset);
    }
    gpio_set_value(gpio, value);

    writel(save->gpiodir, regs + PL061_GPIODIR);
    writel(save->gpiois, regs + PL061_GPIOIS);
    writel(save->gpioibe, regs + PL061_GPIOIBE);
    writel(save->gpioiev, regs + PL061_GPIOIEV);
    writel(save->gpioie, regs + PL061_GPIOIE);
}

/**
 * @fn      drime4_pm_restore_gpios
 * @brief   Restore register values for GPIOs
 * @param   none
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_restore_gpios(void)
{
    struct pm_gpio_save *save = drime4_gpio_save;
    unsigned int gpio;

    for (gpio = 0; gpio < DRIME4_GPIO_GROUP_NR; gpio++, save++)
        drime4_pm_restore_gpio(gpio, save);

}


/********************************************************************
 *                                                                  *
 *        DDR self-refresh enter & Power off request to PCU.        *
 *             Called from sleep.S/drime4_cpu_save().               *  
 *                                                                  *
 ********************************************************************/
#ifdef  CONFIG_SCORE_SUSPEND
extern void l1_cache_off(void);
extern void drime4_pm_ppoff_end(void);

/**
 * @fn      drime4_pm_ppoff_sram
 * @brief   Make DDR enter to self-refresh mode and Request power off to PCU
 * @param   none
 * @return  none
 *
 * @Comment It is executed on SRAM
 *
 * @Commiter    Daeho Kim
 */
void drime4_pm_ppoff_sram(void)
{
    /* Set registers */
    __asm__ __volatile__(
        "mov r0, #0             @ r0=0x0 from now on\n\t" 
        "movw r1, #65535\n\t" 
        "movt r1, #65535        @ r1=0xffff_ffff from now on\n\t" 
        "movw r4, #0\n\t" 
        "movt r4, #12305        @ r4=0x3011_0000 (DRIME4_PA_RESET_CTRL)\n\t" 
        "movw r10, #0\n\t" 
        "movt r10, #24064       @ r10=0x5e00_0000 (DRIME4_PA_SONICS_CONF)\n\t" 
        "movw r11, #0\n\t" 
        "movt r11, #20480       @ r11=0x5000_0000 (DRIME4_PA_LS_DDR_CTRL)\n\t" 
        "movw r12, #40960\n\t" 
        "movt r12, #12293       @ r12=0x3005_a000 (DRIME4_PA_GPIO_BASE(10))\n\t"
	: : );

#if 0
    /* Caculate DDR check sum */
    __asm__ __volatile__(
	"movw r8,#0x9000	@ Check 1SHOT bit\n\t"
	"movt r8,#0x3006\n\t"
	"ldr r2, [r8, #1024]\n\t"
	"bic r2, r2, #128\n\t"
	"str r2, [r8, #1024]    @ GPIO_DIR=0x400\n\t"
	"ldr r3, [r8, #1020]\n\t"
	"and r3, r3, #128\n\t"

        "movw r8, #0\n\t"
        "movt r8, #0xE000\n\t"
        "movw r7, #0\n\t"
        "movt r7, #0xC000\n\t"
        "movw r6, #0\n\t"
        "movt r6, #0\n\t"
        "check_sum_loop:\n\t"
        "ldr r5, [r7]\n\t"
        "add r6, r6, r5\n\t"
        "add r7, r7, #4\n\t"
        "cmp r7, r8\n\t"
        "bne check_sum_loop\n\t"
	"movw r8, #0xFFFC\n\t"
	"movt r8, #0xDFFF\n\t"
	"str r6, [r8]\n\t"

	/*  Park Jungjae: Without setting 1SHOT bit here, 
	    after calculating check sum, 1SHOT bit has no effect.
	    I don't know the reason. */
	"cmp r3, #128		@ Set 1SHOT bit, again.\n\t"
	"bne no_reset\n\t"
	"movw r8,#0x9000\n\t"
	"movt r8,#0x3006\n\t"
	"ldr r2, [r8, #1024]\n\t"
	"orr r2, r2, #128\n\t"
	"str r2, [r8, #1024]    @ GPIO_DIR=0x400\n\t"
	"ldr r3, [r8, #1020]\n\t"
	"orr r3, r3, #128\n\t"
	"str r3, [r8, #1020]    @ GPIO_DATA=0x3fc\n\t"
	"no_reset:\n\t"

        : : );
#endif

    /* Block accesses to DDR */
    // Block DDR transactions from IPs
    __asm__ __volatile__(
        "mov r2, #16            @ 0x10\n\t"
        "movw r3, #34848        @ 0x8820 : ddr\n\t"
        "movt r3, #0\n\t"
        "str r2, [r10, r3]\n\t"
        //*(volatile unsigned int*)(DRIME4_VA_SONICS_CONF+0x1020) = 0x10; //arm0
        //*(volatile unsigned int*)(DRIME4_VA_SONICS_CONF+0x1420) = 0x10; //arm1
        "movw r3, #6176         @ 0x1820 : gpu\n\t"
        "str r2, [r10, r3]\n\t"
        "movw r3, #7200         @ 0x1c20 : mdma\n\t"
        "str r2, [r10, r3]\n\t"
        //*(volatile unsigned int*)(DRIME4_VA_SONICS_CONF+0x2020) = 0x10; //peri
        "movw r3, #9248         @ 0x2420 : pp\n\t"
        "str r2, [r10, r3]\n\t"
        "movw r3, #10272        @ 0x2820 : ipcm\n\t"
        "str r2, [r10, r3]\n\t"
        "movw r3, #11296        @ 0x2c20 : ipcs\n\t"
        "str r2, [r10, r3]\n\t"
        "movw r3, #12320        @ 0x3020 : cp\n\t"
        "str r2, [r10, r3]\n\t"
        "movw r3, #13344        @ 0x3420 : bayer\n\t"
        "str r2, [r10, r3]\n\t"
        "movw r3, #14368        @ 0x3820 : codec0\n\t"
        "str r2, [r10, r3]\n\t"
        "movw r3, #15392        @ 0x3c20 : codec1\n\t"
        "str r2, [r10, r3]\n\t"
        "movw r3, #16416        @ 0x4020 : jpeg\n\t"
        "str r2, [r10, r3]\n\t"
        "movw r3, #17440        @ 0x4420 : dp\n\t"
        "str r2, [r10, r3]\n\t"
        : : );

    // Check if DDR command-queue is empty or if there is any BUS access
    __asm__ __volatile__(
        "check_access:\n\t"
        "ldr r2, [r11]\n\t"
        "and r2, r2, #768       @ 0x300\n\t"
        "cmp r2, #768           @ Check DDR access\n\t"
        "bne check_access\n\t"
        "movw r3, #34856        @ 0x8828\n\t"
        "movt r3, #0\n\t"
        "ldr r2, [r10, r3]\n\t"
        "cmp r2, #0             @ Check BUS access\n\t"
        "bne check_access\n\t"
        "add r0, r0, #1\n\t"
        "cmp r0, #100\n\t"
        "bne check_access       @ Repeat 100 times\n\t"
        "mov r0, #0\n\t"
        : : );

    // Block DDR transactions from SONICS bus 
    __asm__ __volatile__(
        "ldr r2, [r10, r3]\n\t"
        "orr r2, r2, #16        @ 0x10\n\t"
        "str r2, [r10, r3]\n\t"
        : : );


    /**
     * DDR pad tension setting
     * GPIO channel 10 - port 5 : direction OUT, data HIGH
     */
    __asm__ __volatile__(
        "ldr r2, [r12, #1024]\n\t"
        "orr r2, r2, #32\n\t"
        "str r2, [r12, #1024]    @ GPIO_DIR=0x400\n\t"
        "ldr r3, [r12, #1020]\n\t"
        "orr r3, r3, #32\n\t"
        "str r3, [r12, #1020]    @ GPIO_DATA=0x3fc\n\t"
        : : );


    /* DDR self-refresh enter */
    /**
     * TODO
     * For chip0 only now, How about chip1??
     */
    // Wait command-queue empty
    __asm__ __volatile__(
        "cq_empty:\n\t"
        "ldr r2, [r11]\n\t"
        "ubfx r2, r2, #8, #2\n\t"
        "cmp r2, #3\n\t"
        "bne cq_empty\n\t"
        : : );

    // DDR Pre-charge
    __asm__ __volatile__(
        "pre_charge:\n\t"
        "movw r2, #0\n\t"
        "movt r2, #256          @ 0x01000000 : pre-charge cmd\n\t"
        "str r2, [r11, #16]     @ 0x10 : DDR cmd register\n\t"
        "ldr r3, [r11, #72]     @ 0x48 : DDR status register\n\t"
        "cmp r3, #0\n\t"
        "bne pre_charge\n\t"
        : : );

    // DDR self-refresh
    __asm__ __volatile__(
        "movw r2, #0\n\t"
        "movt r2, #1024         @ 0x04000000 : self-refresh cmd\n\t"
        "str r2, [r11, #16]     @ 0x10 : DDR cmd register\n\t"
        : : );

#if 0   // Deprecated code
    // Delay (1ms in ARM 400MHz)
    /*
     * TODO
     * Delay problem occured when executing drime4_pm_ppoff_sram() on SRAM.
     * Need to check it.
     */
    __asm__ __volatile__(
        "movw r2, #6784\n\t"
        "movt r2, #6            @ 0x00061a80\n\t"
        "delay_loop1:\n\t"
        "mov r0, r0\n\t"
        "dmb sy\n\t"
        "sub r2, r2, #1\n\t"
        "cmp r2, #0\n\t"
        "bne delay_loop1\n\t"
        : : );
#endif  // Deprecated code

#if 0   // Deprecated code
    // Drive strength selection for low power consumption in DDR self-refresh mode
    ddr_phyzq_ctrl = readl_relaxed(DRIME4_VA_LS_DDR_CTRL + DDR_PHYZQ_CTRL) & 0xfffff8ff;
    writel_relaxed(ddr_phyzq_ctrl, DRIME4_VA_LS_DDR_CTRL + DDR_PHYZQ_CTRL);
#endif  // Deprecated code

    /**
     * DDR pad re-tension setting
     * GPIO channel 10 - port 5 : direction OUT, data LOW
     */
    __asm__ __volatile__(
        "ldr r2, [r12, #1020]\n\t"
        "bic r2, r2, #32\n\t"
        "str r2, [r12, #1020]   @ GPIO_DATA=0x3fc\n\t"
        : : );

#if 0   // Deprecated code
    // Delay (1ms in ARM 400MHz)
    /*
     * TODO
     * Delay problem occured when executing drime4_pm_ppoff_sram() on SRAM.
     * Need to check it.
     */
    __asm__ __volatile__(
        "movw r2, #6784\n\t"
        "movt r2, #6            @ 0x00061a80\n\t"
        "delay_loop2:\n\t"
        "mov r0, r0\n\t"
        "dmb sy\n\t"
        "sub r2, r2, #1\n\t"
        "cmp r2, #0\n\t"
        "bne delay_loop2\n\t"
        : : );
#endif  // Deprecated code

#if 0
/**
 * If Power Lever is 'ON' --- 0x300513FC 0bit is '0'
 * make the 1SHOT_HS port to High --- 0x300693FC 7bit '0' -> '1'
*/
	__asm__ __volatile__(
		"movw r12,#0x13fc\n\t"
		"movt r12,#0x3005\n\t"
		"ldr r2, [r12]\n\t"
		"and r2, r2, #1\n\t"
		"cmp r2, #1\n\t"
		"beq lever_off\n\t"
		"movw r12,#0x9000\n\t"
		"movt r12,#0x3006\n\t"
		"ldr r2, [r12, #1024]\n\t"
		"orr r2, r2, #128\n\t"
		"str r2, [r12, #1024]    @ GPIO_DIR=0x400\n\t"
		"ldr r3, [r12, #1020]\n\t"
		"orr r3, r3, #128\n\t"
		"str r3, [r12, #1020]    @ GPIO_DATA=0x3fc\n\t"
		"lever_off:\n\t"
		: : );
#endif

    /* Power-off request to PCU */
    // Set SYSTEM power-on-hold
    __asm__ __volatile__(
        "ldr r2, [r4, #256]     @ 0x100 : PCU control register\n\t" 
        "orr r2, r2, #128\n\t"
        "str r2, [r4, #256]\n\t"
        : : );
    // Set DDR power-on-hold
    __asm__ __volatile__(
        "ldr r2, [r4, #256]\n\t" 
        "orr r2, r2, #2048\n\t"
        "str r2, [r4, #256]\n\t"
        : : );
    // Set DSLPW sleep flag
    __asm__ __volatile__(
        "set_dslpw:\n\t"
        "ldr r2, [r4, #256]\n\t" 
        "orr r2, r2, #1024\n\t"
        "str r2, [r4, #256]\n\t"
        "ldr r2, [r4, #256]\n\t"
        "and r2, r2, #1024\n\t"
        "cmp r2, #1024\n\t"
        "bne pre_charge\n\t"
        : : );
    // Reset key-detection all
    __asm__ __volatile__(
        "ldr r2, [r4, #256]\n\t" 
        "bic r2, #2\n\t"
        "str r2, [r4, #256]\n\t"
        : : );
    // Set SYSTEM power-off
    __asm__ __volatile__(
        "ldr r2, [r4, #256]\n\t" 
        "bic r2, #128\n\t"
        "str r2, [r4, #256]\n\t"
        : : );

    // Label
    __asm__ __volatile__(
        "drime4_pm_ppoff_end:\n\t"
        "nop\n\t"
        : : );
}

/* for internal use */

// DP
#define __DRIME4_VA_DP_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_VA_CLOCK_CTRL + GCLKCON1))>>22) & 0x1)
#define __DRIME4_PA_DP_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_PA_CLOCK_CTRL + GCLKCON1))>>22) & 0x1)
#define __DRIME4_VA_DP_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_VA_PMU + PMU_SCACK))>>6)&1))
#define __DRIME4_PA_DP_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_PA_PMU + PMU_SCACK))>>6)&1))

// BE
#define __DRIME4_VA_BE_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_VA_CLOCK_CTRL + GCLKCON1))>>20) & 0x1)	
#define __DRIME4_PA_BE_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_PA_CLOCK_CTRL + GCLKCON1))>>20) & 0x1)	
#define __DRIME4_VA_BE_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_VA_PMU + PMU_SCACK))>>5)&1))	
#define __DRIME4_PA_BE_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_PA_PMU + PMU_SCACK))>>5)&1))	

// EP
#define __DRIME4_VA_EP_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_VA_CLOCK_CTRL + GCLKCON1))>>19) & 0x1)	
#define __DRIME4_PA_EP_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_PA_CLOCK_CTRL + GCLKCON1))>>19) & 0x1)	
#define __DRIME4_VA_EP_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_VA_PMU + PMU_SCACK))>>4)&1))
#define __DRIME4_PA_EP_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_PA_PMU + PMU_SCACK))>>4)&1))

// IPCS
#define __DRIME4_VA_IPCS_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_VA_CLOCK_CTRL + GCLKCON1))>>18) & 0x1)	
#define __DRIME4_PA_IPCS_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_PA_CLOCK_CTRL + GCLKCON1))>>18) & 0x1)	
#define __DRIME4_VA_IPCS_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_VA_PMU + PMU_SCACK))>>3)&1))
#define __DRIME4_PA_IPCS_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_PA_PMU + PMU_SCACK))>>3)&1))

// IPCM
#define __DRIME4_VA_IPCM_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_VA_CLOCK_CTRL + GCLKCON1))>>17) & 0x1)	
#define __DRIME4_PA_IPCM_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_PA_CLOCK_CTRL + GCLKCON1))>>17) & 0x1)	
#define __DRIME4_VA_IPCM_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_VA_PMU + PMU_SCACK))>>2)&1))	
#define __DRIME4_PA_IPCM_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_PA_PMU + PMU_SCACK))>>2)&1))

// PP
#define __DRIME4_VA_PP_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_VA_CLOCK_CTRL + GCLKCON1))>>16) & 0x1)	
#define __DRIME4_PA_PP_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_PA_CLOCK_CTRL + GCLKCON1))>>16) & 0x1)	
#define __DRIME4_VA_PP_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_VA_PMU + PMU_SCACK))>>1)&1))	
#define __DRIME4_PA_PP_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_PA_PMU + PMU_SCACK))>>1)&1))

// GPU
#define __DRIME4_VA_GPU_CLK_ON() \
	(((*(volatile unsigned int*)(DRIME4_VA_CLOCK_CTRL + GCLKCON1))>>21) & 0x1)
#define __DRIME4_PA_GPU_CLK_ON() \
	(((*(volatile unsigned int*)(DRIME4_PA_CLOCK_CTRL + GCLKCON1))>>21) & 0x1)
#define __DRIME4_VA_GPU_PWR_ON() \
	(!((*(volatile unsigned int*)(DRIME4_VA_PMU + PMU_SCACK))&1))	
#define __DRIME4_PA_GPU_PWR_ON() \
	(!((*(volatile unsigned int*)(DRIME4_PA_PMU + PMU_SCACK))&1))

// CODEC
#define __DRIME4_VA_CODEC_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_VA_CLOCK_CTRL + GCLKCON1))>>8) & 0x1)
#define __DRIME4_PA_CODEC_CLK_ON()	\
	(((*(volatile unsigned int*)(DRIME4_PA_CLOCK_CTRL + GCLKCON1))>>8) & 0x1)
#define __DRIME4_VA_CODEC_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_VA_PMU + PMU_SCACK))>>8)&1))
#define __DRIME4_PA_CODEC_PWR_ON()	\
	(!(((*(volatile unsigned int*)(DRIME4_PA_PMU + PMU_SCACK))>>8)&1))

// JPEG
#define __DRIME4_VA_JPEG_CLK_ON()	\
        (((*(volatile unsigned int*)(DRIME4_VA_CLOCK_CTRL + GCLKCON1))>>9) & 0x1)
#define __DRIME4_PA_JPEG_CLK_ON()	\
        (((*(volatile unsigned int*)(DRIME4_PA_CLOCK_CTRL + GCLKCON1))>>9) & 0x1)
#define __DRIME4_VA_JPEG_PWR_ON()	\
        (!(((*(volatile unsigned int*)(DRIME4_VA_PMU + PMU_SCACK))>>7)&1))
#define __DRIME4_PA_JPEG_PWR_ON()	\
        (!(((*(volatile unsigned int*)(DRIME4_PA_PMU + PMU_SCACK))>>7)&1))

// Stop request
#define __DP_STOP_REQ (*(volatile unsigned int*)(0x500b145C))
#define __BE_STOP_REQ (*(volatile unsigned int*)(0x5008755C))
#define __EP_STOP_REQ (*(volatile unsigned int*)(0x5007825C))
#define __IPCS_STOP_REQ (*(volatile unsigned int*)(0x50050F5C))
#define __IPCM_STOP_REQ (*(volatile unsigned int*)(0x50060F5C))
#define __PP_STOP_REQ (*(volatile unsigned int*)(0x5004355C))
#define __GPU_STOP_REQ (*(volatile unsigned int*)(0x30150010))

// Stop request Ack.
#define __DP_STOP_ACK 	( !(((*(volatile unsigned int*)(0x5e004428)) >> 4) & 0x03) )
#define __BE_STOP_ACK 	( !(((*(volatile unsigned int*)(0x5e003428)) >> 4) & 0x03) )
#define __EP_STOP_ACK 	( !(((*(volatile unsigned int*)(0x5e003028)) >> 4) & 0x03) )
#define __IPCS_STOP_ACK ( !(((*(volatile unsigned int*)(0x5e002c28)) >> 4) & 0x03) )
#define __IPCM_STOP_ACK	( !(((*(volatile unsigned int*)(0x5e002828)) >> 4) & 0x03) )
#define __PP_STOP_ACK 	( !(((*(volatile unsigned int*)(0x5e002428)) >> 4) & 0x03) )
#define __GPU_STOP_ACK	( (*(volatile unsigned int*)(0x30150014)) & 0x01 )

// ETC.
#define __IPCS_CLK_SET (*(volatile unsigned int*)(0x5005002c))
#define __IPCM_CLK_SET (*(volatile unsigned int*)(0x5006002c))
#define __PP_DMA_Enable_rdma (*(volatile unsigned int*)(0x50043a10))

void
drime4_pm_ip_check(void)
{
        // Clock check
	if (__DRIME4_VA_CODEC_CLK_ON())
                printk("CODEC clock is alive!\n");
	if (__DRIME4_VA_JPEG_CLK_ON())
                printk("JPEG clock is alive!\n");
	if (__DRIME4_VA_PP_CLK_ON())
                printk("PP clock is alive!\n");
	if (__DRIME4_VA_IPCM_CLK_ON())
                printk("IPCM clock is alive!\n");
	if (__DRIME4_VA_IPCS_CLK_ON())
                printk("IPCS clock is alive!\n");
	if (__DRIME4_VA_EP_CLK_ON())
                printk("EP clock is alive!\n");
	if (__DRIME4_VA_BE_CLK_ON())
                printk("BE clock is alive!\n");
	if (__DRIME4_VA_GPU_CLK_ON())
                printk("GPU clock is alive!\n");
        if (__DRIME4_VA_DP_CLK_ON())
                printk("DP clock is alive!\n");

        // Power check
	if (__DRIME4_VA_GPU_PWR_ON())
                printk("GPU power on!\n");
	if (__DRIME4_VA_PP_PWR_ON())
                printk("PP power on!\n");
	if (__DRIME4_VA_IPCM_PWR_ON())
                printk("IPCM power on!\n");
	if (__DRIME4_VA_IPCS_PWR_ON())
                printk("IPCS power on!\n");
	if (__DRIME4_VA_EP_PWR_ON())
                printk("EP power on!\n");
	if (__DRIME4_VA_BE_PWR_ON())
                printk("BE power on!\n");
	if (__DRIME4_VA_DP_PWR_ON())
                printk("DP power on!\n");
	if (__DRIME4_VA_JPEG_PWR_ON())
                printk("JPEG power on!\n");
	if (__DRIME4_VA_CODEC_PWR_ON())
                printk("CODEC power on!\n");
}

/**
 * @fn      drime4_pm_ppoff_enter
 * @brief   Do some minor jobs before drime4_pm_ppoff_sram()
 * @param   none
 * @return  none
 *
 * @Comment Copy drime4_pm_ppoff_sram() to SRAM, block some IPs to DDR, flush/disable L2 cache
 *          and move PC to SRAM
 *
 * @Commiter    Daeho Kim
 */
extern int drime4_peri_clk_check(struct clk *clk);
extern struct clk dp_clk;

#define DRIME4_WDOGREG(x) ((x) + DRIME4_VA_WDT)
#define DRIME4_WDTCR		 DRIME4_WDOGREG(0x0)
#define	DRIME4_WDTPSR		DRIME4_WDOGREG(0x4)
#define DRIME4_WDTLDR		DRIME4_WDOGREG(0x8)
#define DRIME4_WDTVLR		DRIME4_WDOGREG(0xC)
#define DRIME4_WDTISR		DRIME4_WDOGREG(0x10)

int drime4_pm_ppoff_enter(void)
{
    unsigned long addr;
    pgd_t *pgd_p;
    pmd_t *pmd_p;
    pte_t *pte_p;
    pte_t pte;
    int is_dp_clk_alive = drime4_peri_clk_check(&dp_clk);

    printk(KERN_INFO "%s\n", __func__);

    drime4_pm_ip_check();

	d4_pcu_intr_mask_set(NULL);

#if defined (CONFIG_MACH_D4_NX300) || defined (CONFIG_MACH_D4_NX2000)
	/**
	 * Initialize 1SHOT_HS port 
	 * Set Low
	*/

#if 0
////****************************************** watch dog rest
	if(gpio_get_value(GPIO_POWER_ON) == 0){
		/* disable watchdog */
		__raw_writel(0, DRIME4_WDTCR);

		/* Scale Set */
		__raw_writel(0x80, DRIME4_WDTPSR);
		__raw_writel(0xFF, DRIME4_WDTLDR);

		/* watchdog reset */
		__raw_writel(0x0007, DRIME4_WDTCR);

		mdelay(1000);
	}
////****************************************** watch dog rest
#endif

	if(oneshot == 1 && gpio_get_value(GPIO_POWER_ON) == 0){
		pinctrl_request_gpio(GPIO_1SHOT_HS);
		gpio_request(GPIO_1SHOT_HS,"1shot_hs");
		gpio_direction_output(GPIO_1SHOT_HS,1);
		oneshot = 0;
	}
#endif

    /* Execute drime4_pm_ppoff_sram() on SRAM */
    // Copy drime4_pm_ppoff_sram() to SRAM
    printk(KERN_INFO "Copy last code to SRAM\n");
    memset((void *)(DRIME4_VA_SONICS_MSRAM), 0, PAGE_SIZE);
    /**
     * Executing code on SRAM needs some spare space in front of the text
     * It is found from experience, but do not know why.
     */
    memcpy((void *)(DRIME4_VA_SONICS_MSRAM + PAGE_SIZE/4), 
                    (void *)drime4_pm_ppoff_sram, 
                    drime4_pm_ppoff_end - drime4_pm_ppoff_sram);

    // Modify PTE XN bit(execute permassion) on SRAM
    addr = (unsigned long)DRIME4_VA_SONICS_MSRAM;
    pgd_p = pgd_offset_k(addr);
    pmd_p = pmd_offset(pgd_p, addr);
    pte_p = pte_offset_map(pmd_p, addr);

    pte = ptep_modify_prot_start(&init_mm, addr, pte_p);
    pte &= ~L_PTE_XN;
    ptep_modify_prot_commit(&init_mm, addr, pte_p, pte);

    pte_unmap(pte_p);


    /* Disable Peri. clocks */
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_NOR_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_PWM_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_PTC_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_UART_CLK_EN);
    //writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_GPIO_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_SPI_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_I2C_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_I2S_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_ADC_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_TIMER_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_EFS_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_SD_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_NAND_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_USB3_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_HSIC_CLK_EN);
    writel_relaxed(0x0, DRIME4_VA_PLATFORM_CTRL + PLAT_ATA_CLK_EN);


    /* Block DDR access from IPs */
    // DP
    if (is_dp_clk_alive) {	// Is DP CLK alive? then BLock DDR access. or do nothing...
    	writel_relaxed(0x70, DRIME4_VA_DP + DP_LCD_CLK_EN);
    	writel_relaxed(0x00, DRIME4_VA_DP + DP_INTR_EN);
    	writel_relaxed(0x00, DRIME4_VA_DP + DP_BACKGROUND_MONO);
    	writel_relaxed(0x00, DRIME4_VA_DP + DP_BACKGROUND_COLOR);
    }

    // SCU
    writel_relaxed(0x61, DRIME4_VA_A9MP);

    drime4_pm_disable_vics();

    // Stop REQ to ISPs & Check ACK
    //writel_relaxed(0x3f, DRIME4_VA_PMU + PMU_STOP_REQ);

    // Enable isolation
    //writel_relaxed(0x1ff, DRIME4_VA_PMU + PMU_ISO_EN);

    // GCLK Gating
    //writel_relaxed(0x0, DRIME4_VA_CLOCK_CTRL + GCLKCON1);


    /* Disable MMU */
    /**
     * TODO
     * Need stable clean/invalidate/disable code for L1 & L2 cache
     */
    // Flush L1 cache
    flush_cache_all();

    // Disable L1 cache
    __asm__ __volatile__ (
        "mrc p15, 0, r0, c1, c0, 0\n\t"
        "bic r0, r0, #(1 << 2)          @Disable D-cache\n\t"
        "bic r0, r0, #(1 << 12)         @Disable I-cache\n\t"
        "mcr p15, 0, r0, c1, c0, 0\n\t"
        : : : "memory");

    // Flush L1 cache (double check)
    flush_cache_all();

    // L1 Cache off
    l1_cache_off();

    if ( __DRIME4_PA_DP_CLK_ON() && __DRIME4_PA_DP_PWR_ON() ) {
	__DP_STOP_REQ =  0x1;	// DP Stop Req.
	while (!__DP_STOP_ACK);
    }

    if ( __DRIME4_PA_BE_CLK_ON() && __DRIME4_PA_BE_PWR_ON() ) {
	__BE_STOP_REQ =  0x1;	// BE Stop Req.
	while (!__BE_STOP_ACK);
    }

//    if ( __DRIME4_PA_EP_CLK_ON() && __DRIME4_PA_EP_PWR_ON() ) {
//	__EP_STOP_REQ =  0x1;	// EP Stop Req.
//	while (!__EP_STOP_ACK);
//    }

    if ( __DRIME4_PA_IPCS_CLK_ON() && __DRIME4_PA_IPCS_PWR_ON() ) {
	// IPCS_CLK_SET : bit[8](CLK_DMA)
	if ( ( __IPCS_CLK_SET >> 8) & 0x01 ) {
	    // IPCS Stop Req.
	    __IPCS_STOP_REQ =  0x1;
	    while (!__IPCS_STOP_ACK);
	}
    }
	
    if ( __DRIME4_PA_IPCM_CLK_ON() && __DRIME4_PA_IPCM_PWR_ON() ) {
	// IPCM_CLK_SET : bit[5](CLK_DMA)
	if ( ( __IPCM_CLK_SET  >> 5) & 0x01 ) {
	// IPCM Stop Req.
	    __IPCM_STOP_REQ =  0x1;
	    while (!__IPCM_STOP_ACK);
	}
    }

    if ( __DRIME4_PA_PP_CLK_ON() && __DRIME4_PA_PP_PWR_ON() ) {
	// PP_DMA_Enable_rdma : bit[0](dma_enable_sm)
	if ( __PP_DMA_Enable_rdma & 0x01 ) {
	// PP Stop Req.
	    __PP_STOP_REQ =  0x1;
	    while (!__PP_STOP_ACK);
	}
    }

    if ( __DRIME4_PA_GPU_CLK_ON() && __DRIME4_PA_GPU_PWR_ON() ) {
	__GPU_STOP_REQ =  0x1;	// GPU Stop Req.
	while (!__GPU_STOP_ACK);
    }

	__asm__ __volatile__(	"nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" );
	__asm__ __volatile__(	"nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" );
	__asm__ __volatile__(	"nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" );
	__asm__ __volatile__(	"nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" );
	__asm__ __volatile__(	"nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" );
	__asm__ __volatile__(	"nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" );
	__asm__ __volatile__(	"nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" );
	__asm__ __volatile__(	"nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" );
	__asm__ __volatile__(	"nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" );
	__asm__ __volatile__(	"nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" "nop;" );

	*(volatile unsigned int*)(DRIME4_PA_PMU + PMU_ISO_EN) = 0x1ff;	// Enable isolation.

	*(volatile unsigned int*)(0x30120020) =  0x0;	// gclkcon gating all.

    /* Move PC to SRAM */
    __asm__ __volatile__(
        "movw r2, #1024\n\t"
        "movt r2, #28704    @ 0x70200400 = DRIME4_VA_SONICS_MSRAM + PAGE_SIZE/4 \n\t"
        "mov pc, r2\n\t"
        : : );


    panic("PPOFF failed\n");
}
#endif  // CONFIG_SCORE_SUSPEND


/********************************************************************
 *                                                                  *
 *              STR main function suspend_ops->enter()              * 
 *                                                                  *
 ********************************************************************/
extern int drime4_cpu_save(unsigned long saveblk);
extern void drime4_cpu_restore(void);

unsigned long drime4_ppoff;
unsigned long reserved_for_resume_point;

/**
 * @fn      drime4_pm_enter
 * @brief   suspend_ops->enter()
 * @param   state: not-used
 * @return  0 if success, nagative value otherwise
 *
 * @Comment Save core device's context and resume point, flush/disable L2 cache
 *          and save MMU context.
 *
 * @Commiter    Daeho Kim
 */
extern unsigned long g_sdmmc_dma;
extern unsigned long g_sdio_dma;
extern unsigned long g_event_buff_dma;
extern unsigned long g_ctrl_buff_dma;
extern unsigned long g_ep0_trb_dma;
static int drime4_pm_enter(suspend_state_t state)
{
    unsigned int reg=0;
	unsigned long backup_1 = 0;
	unsigned long backup_2 = 0;
	unsigned long backup_3 = 0;
	unsigned long backup_4 = 0;
	unsigned long backup_5 = 0;
	unsigned long backup_6 = 0;
	unsigned long backup_7 = 0;

    DRIME4_PMDG("%s(%d)\n", __func__, state);

#if defined(CONFIG_MACH_D4_NX300)
//	drime4_pm_input_gpios();
#endif

    /**
     * TODO
     * Platform devices' context need not to be saved until now.
     * It, however, need to be saved later.
     */
#if 0
    /* Save core devices context */
    drime4_pm_save_gpios();
    drime4_pm_save_uarts();
    drime4_pm_save_vics();
    drime4_pm_save_core();
#endif


    /**
     * TODO
     * Set POWER configuration flags ?
     */


    /* Save the address of drime4_pm_ppoff_enter() */
    drime4_ppoff = (unsigned long) drime4_pm_ppoff_enter;

	backup_1 = *(unsigned long*)reserved_for_resume_point;
	backup_2 = *(unsigned long*)(reserved_for_resume_point + 0x04);
	backup_3 = *(unsigned long*)(reserved_for_resume_point + 0x0C);
	backup_4 = *(unsigned long*)(reserved_for_resume_point + 0x10);
	backup_5 = *(unsigned long*)(reserved_for_resume_point + 0x14);
	backup_6 = *(unsigned long*)(reserved_for_resume_point + 0x18);
	backup_7 = *(unsigned long*)(reserved_for_resume_point + 0x1C);

    /* Save the resume point(=drime4_cpu_restore) for wake-up from PPOFF state */
    *(unsigned long *)reserved_for_resume_point = (unsigned long)&drime4_cpu_restore;
    DRIME4_PMDG("reserved_for_resume_point 0x%x, resume_point 0x%x\n", 
            reserved_for_resume_point, (unsigned long)&drime4_cpu_restore);
	*(unsigned long *)(reserved_for_resume_point + 0x04) = g_sdmmc_dma;
	*(unsigned long *)(reserved_for_resume_point + 0x0C) = g_sdio_dma;
	*(unsigned long *)(reserved_for_resume_point + 0x10) = g_event_buff_dma;
	*(unsigned long *)(reserved_for_resume_point + 0x14) = g_ctrl_buff_dma;
	*(unsigned long *)(reserved_for_resume_point + 0x18) = g_ep0_trb_dma;
	*(unsigned long *)(reserved_for_resume_point + 0x1C) = 0; 	/* Reserved for DDR check sum save */

    /* Flush & disable L2 cache */
    /**
     * TODO
     * Need stable code for clean/invalidate/disable L1&L2 cache
     */
    // Flush L1 cache
    flush_cache_all();

    // Clean & invalidate L2 cache
    // Clean-up L2 cache for whole address space
    outer_clean_range(0x0, 0xffffffff);
    outer_inv_all();

    // Disable L1 cache & invalidate TLB
    __asm__ __volatile__ (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        "bic %0, %0, #(1 << 2)          @ Disable L1 cache\n\t"
        "mcr p15, 0, %0, c1, c0, 0\n\t"
        :"+r"(reg) : : "memory");
    __asm__ __volatile__ (
        "mcr p15, 0, r0, c8, c3, 0      @ Invalidate TLB\n\t"
        "mcr p15, 0, r0, c8, c5, 0\n\t"
        "mcr p15, 0, r0, c8, c6, 0\n\t"
        "mcr p15, 0, r0, c8, c7, 0\n\t"
        : : : "memory");

    // L2 cache off
    outer_disable();

    // Enable L1 cache
    __asm__ __volatile__ (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        "orr %0, %0, #(1 << 2)          @ Enable L1 cache\n\t"
        "mcr p15, 0, %0, c1, c0, 0\n\t"
        :"+r"(reg) : : "memory");


    /* Save CPU context and make SYSTEM sleep */
	cpu_suspend(0, drime4_cpu_save);
    /* Return from drime4_cpu_restore() */
    DRIME4_PMDG("Wake-up from sleep mode\n");


    /* Restore the cpu state using the kernel's cpu init code. */
    cpu_init();


    /* Clear SLEEP Flag */
    if ( drime4_pm_sleep_flag(0) < 0 ) {
        DRIME4_PMDG(KERN_ERR "%s: error: clear sleep flag failed\n", __func__);
        return 0;
    }

    *(unsigned long*)reserved_for_resume_point = backup_1;
    *(unsigned long*)(reserved_for_resume_point + 0x04) = backup_2;
    *(unsigned long*)(reserved_for_resume_point + 0x0C) = backup_3;
    *(unsigned long*)(reserved_for_resume_point + 0x10) = backup_4;
    *(unsigned long*)(reserved_for_resume_point + 0x14) = backup_5;
    *(unsigned long*)(reserved_for_resume_point + 0x18) = backup_6;
    *(unsigned long*)(reserved_for_resume_point + 0x1C) = backup_7;


    /**
     * TODO
     * Platform devices' context need not to be saved until now.
     * It, however, need to be saved later.
     */
#if 0
    /* Restore COREs */
    drime4_pm_restore_core();
    drime4_pm_restore_vics();
    /**
     * All register bits of PL011 are reset to a logic 0 by a system or power-on reset
     * reffered from CH3.1, DDI0183G_uart_pl011_r1p5_trm.pdf
     */
    drime4_pm_restore_uarts();
    drime4_pm_restore_gpios();
#endif


    DRIME4_PMDG("%s returned\n", __func__);
    return 0;

}

/**
 * @fn      drime4_pm_prepare
 * @brief   suspend_ops->prepare()
 * @param   none
 * @return  0 if success, negative value otherwise
 *
 * @Commiter    Daeho Kim
 */
static int drime4_pm_prepare(void)
{
    return 0;
}

/**
 * @fn      drime4_pm_finish
 * @brief   suspend_ops->finish()
 * @param   none
 * @return  none
 *
 * @Commiter    Daeho Kim
 */
static void drime4_pm_finish(void)
{
}


/********************************************************************
 *                                                                  *
 *                   PM Driver Initialization                       * 
 *                                                                  *
 ********************************************************************/
/**
 * @var     drime4_pm_ops
 * @brief   Static variable containing DRIMe4 PM operations
 *
 * @Comment .enter and .valid fields should be implemented, others are optional
 *
 * @Commiter    Daeho Kim
 */
static const struct platform_suspend_ops drime4_pm_ops = {
    .prepare    = drime4_pm_prepare,
    .enter      = drime4_pm_enter,
    .finish     = drime4_pm_finish,
    .valid      = suspend_valid_only_mem,
};

/**
 * @fn      drime4_pm_init
 * @brief   Register DRIMe4 PM operations
 * @param   none
 * @return  0 if success, negative value otherwise
 *
 * @Commiter    Daeho Kim
 */
int __init drime4_pm_init(void)
{
    printk(KERN_INFO "DRIMeIV Power Management\n");

    suspend_set_ops(&drime4_pm_ops);
    return 0;
}

