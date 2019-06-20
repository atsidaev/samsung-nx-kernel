/* linux/arch/arm/mach-drime4/gpio.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Based on drivers/gpio/pl061.c
 *
 * DRIME4 - GPIO support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/amba/bus.h>
#include <linux/amba/pl061.h>

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/slab.h>

#include <mach/irqs.h>
#include <mach/gpio.h>
#include <mach/map.h>
#include "../../../kernel/power/power.h"
#include "include/mach/version_information.h"

#define PL061_GPIO_PLATDATA(group) {				\
	.gpio_base	= DRIME4_GPIO##group(0),		\
	.irq_base	= DRIME4_IRQ_GPIO_GROUP(group),		\
}

static struct pl061_platform_data gpio_plat_data[] = {
	PL061_GPIO_PLATDATA(0),
	PL061_GPIO_PLATDATA(1),
	PL061_GPIO_PLATDATA(2),
	PL061_GPIO_PLATDATA(3),
	PL061_GPIO_PLATDATA(4),
	PL061_GPIO_PLATDATA(5),
	PL061_GPIO_PLATDATA(6),
	PL061_GPIO_PLATDATA(7),
	PL061_GPIO_PLATDATA(8),
	PL061_GPIO_PLATDATA(9),
	PL061_GPIO_PLATDATA(10),
	PL061_GPIO_PLATDATA(11),
	PL061_GPIO_PLATDATA(12),
	PL061_GPIO_PLATDATA(13),
	PL061_GPIO_PLATDATA(14),
	PL061_GPIO_PLATDATA(15),
	PL061_GPIO_PLATDATA(16),
	PL061_GPIO_PLATDATA(17),
	PL061_GPIO_PLATDATA(18),
	PL061_GPIO_PLATDATA(19),
	PL061_GPIO_PLATDATA(20),
	PL061_GPIO_PLATDATA(21),
	PL061_GPIO_PLATDATA(22),
	PL061_GPIO_PLATDATA(23),
	PL061_GPIO_PLATDATA(24),
	PL061_GPIO_PLATDATA(25),
	PL061_GPIO_PLATDATA(26),
	PL061_GPIO_PLATDATA(27),
};

#define PL061_GPIO_DEVICE(_name, _group, _irq) {		\
	.dev = {						\
		.coherent_dma_mask = 0,\
		.init_name = _name,				\
		.platform_data = &gpio_plat_data[_group],	\
	},							\
	.res = {						\
		.start = DRIME4_PA_GPIO_BASE(_group),		\
		.end = DRIME4_PA_GPIO_BASE(_group) + SZ_4K - 1,	\
		.name = ((((void *)0))),					\
		.flags = IORESOURCE_MEM,			\
	},							\
	.irq = {_irq},						\
	.periphid = _group,					\
}


static AMBA_APB_DEVICE(gpio0, "dev:gpio0", 0, DRIME4_PA_GPIO_BASE(0), {IRQ_GPIOA}, &gpio_plat_data[0]);
static AMBA_APB_DEVICE(gpio1, "dev:gpio1", 0, DRIME4_PA_GPIO_BASE(1), {IRQ_GPIOB}, &gpio_plat_data[1]);
static AMBA_APB_DEVICE(gpio2, "dev:gpio2", 0, DRIME4_PA_GPIO_BASE(2), {IRQ_GPIOC}, &gpio_plat_data[2]);
static AMBA_APB_DEVICE(gpio3, "dev:gpio3", 0, DRIME4_PA_GPIO_BASE(3), {IRQ_GPIOD}, &gpio_plat_data[3]);
static AMBA_APB_DEVICE(gpio4, "dev:gpio4", 0, DRIME4_PA_GPIO_BASE(4), {IRQ_GPIOE}, &gpio_plat_data[4]);
static AMBA_APB_DEVICE(gpio5, "dev:gpio5", 0, DRIME4_PA_GPIO_BASE(5), {IRQ_GPIOF}, &gpio_plat_data[5]);
static AMBA_APB_DEVICE(gpio6, "dev:gpio6", 0, DRIME4_PA_GPIO_BASE(6), {IRQ_GPIOG}, &gpio_plat_data[6]);
static AMBA_APB_DEVICE(gpio7, "dev:gpio7", 0, DRIME4_PA_GPIO_BASE(7), {IRQ_GPIOH}, &gpio_plat_data[7]);
static AMBA_APB_DEVICE(gpio8, "dev:gpio8", 0, DRIME4_PA_GPIO_BASE(8), {IRQ_GPIOI_O}, &gpio_plat_data[8]);
static AMBA_APB_DEVICE(gpio9, "dev:gpio9", 0, DRIME4_PA_GPIO_BASE(9), {IRQ_GPIOI_O}, &gpio_plat_data[9]);
static AMBA_APB_DEVICE(gpio10, "dev:gpio10", 0, DRIME4_PA_GPIO_BASE(10), {IRQ_GPIOI_O}, &gpio_plat_data[10]);
static AMBA_APB_DEVICE(gpio11, "dev:gpio11", 0, DRIME4_PA_GPIO_BASE(11), {IRQ_GPIOI_O}, &gpio_plat_data[11]);
static AMBA_APB_DEVICE(gpio12, "dev:gpio12", 0, DRIME4_PA_GPIO_BASE(12), {IRQ_GPIOI_O}, &gpio_plat_data[12]);
static AMBA_APB_DEVICE(gpio13, "dev:gpio13", 0, DRIME4_PA_GPIO_BASE(13), {IRQ_GPIOI_O}, &gpio_plat_data[13]);
static AMBA_APB_DEVICE(gpio14, "dev:gpio14", 0, DRIME4_PA_GPIO_BASE(14), {IRQ_GPIOI_O}, &gpio_plat_data[14]);
static AMBA_APB_DEVICE(gpio15, "dev:gpio15", 0, DRIME4_PA_GPIO_BASE(15), {IRQ_GPIOP_V}, &gpio_plat_data[15]);
static AMBA_APB_DEVICE(gpio16, "dev:gpio16", 0, DRIME4_PA_GPIO_BASE(16), {IRQ_GPIOP_V}, &gpio_plat_data[16]);
static AMBA_APB_DEVICE(gpio17, "dev:gpio17", 0, DRIME4_PA_GPIO_BASE(17), {IRQ_GPIOP_V}, &gpio_plat_data[17]);
static AMBA_APB_DEVICE(gpio18, "dev:gpio18", 0, DRIME4_PA_GPIO_BASE(18), {IRQ_GPIOP_V}, &gpio_plat_data[18]);
static AMBA_APB_DEVICE(gpio19, "dev:gpio19", 0, DRIME4_PA_GPIO_BASE(19), {IRQ_GPIOP_V}, &gpio_plat_data[19]);
static AMBA_APB_DEVICE(gpio20, "dev:gpio20", 0, DRIME4_PA_GPIO_BASE(20), {IRQ_GPIOP_V}, &gpio_plat_data[20]);
static AMBA_APB_DEVICE(gpio21, "dev:gpio21", 0, DRIME4_PA_GPIO_BASE(21), {IRQ_GPIOP_V}, &gpio_plat_data[21]);
static AMBA_APB_DEVICE(gpio22, "dev:gpio22", 0, DRIME4_PA_GPIO_BASE(22), {IRQ_GPIOW_AB}, &gpio_plat_data[22]);
static AMBA_APB_DEVICE(gpio23, "dev:gpio23", 0, DRIME4_PA_GPIO_BASE(23), {IRQ_GPIOW_AB}, &gpio_plat_data[23]);
static AMBA_APB_DEVICE(gpio24, "dev:gpio24", 0, DRIME4_PA_GPIO_BASE(24), {IRQ_GPIOW_AB}, &gpio_plat_data[24]);
static AMBA_APB_DEVICE(gpio25, "dev:gpio25", 0, DRIME4_PA_GPIO_BASE(25), {IRQ_GPIOW_AB}, &gpio_plat_data[25]);
static AMBA_APB_DEVICE(gpio26, "dev:gpio26", 0, DRIME4_PA_GPIO_BASE(26), {IRQ_GPIOW_AB}, &gpio_plat_data[26]);
static AMBA_APB_DEVICE(gpio27, "dev:gpio27", 0, DRIME4_PA_GPIO_BASE(27), {IRQ_GPIOW_AB}, &gpio_plat_data[27]);

static struct amba_device *d4_gpio_devices[] __initdata = {
		&gpio0_device,
		&gpio1_device,
		&gpio2_device,
		&gpio3_device,
		&gpio4_device,
		&gpio5_device,
		&gpio6_device,
		&gpio7_device,
		&gpio8_device,
		&gpio9_device,
		&gpio10_device,
		&gpio11_device,
		&gpio12_device,
		&gpio13_device,
		&gpio14_device,
		&gpio15_device,
		&gpio16_device,
		&gpio17_device,
		&gpio18_device,
		&gpio19_device,
		&gpio20_device,
		&gpio21_device,
		&gpio22_device,
		&gpio23_device,
		&gpio24_device,
		&gpio25_device,
		&gpio26_device,
		&gpio27_device,
};

static int __init drime4_gpio_init(void)
{
	int i;
	int ret;

#if defined(CONFIG_MACH_D4_NX300)
	gpio_plat_data[12].directions |= 0x10;					// TP15(GPIO12_4) : OUT
	gpio_plat_data[24].directions |= 0x14;					// CARD_LED(GPIO24_2), TP30(GPIO24_4) : OUT
	gpio_plat_data[25].directions |= 0x20;					// MICOM_SLEEP(GPIO25_5) : OUT

	gpio_plat_data[12].values &= 0xef;						// TP15(GPIO12_4) : OUT default L	
	gpio_plat_data[24].values &= 0xeb;						// CARD_LED(GPIO24_2), TP30(GPIO24_4) : OUT default L	
	gpio_plat_data[25].values |= 0x20;						// MICOM_SLEEP(GPIO25_5) : OUT default H
#elif defined(CONFIG_MACH_D4_NX2000)

	// DSP Core Voltage Change (1.01V -> 0.98V)
	gpio_plat_data[7].directions |= 0xc0;
	gpio_plat_data[7].values |= 0x40;					// AB27(GPIO7_6) : OUT default H
	gpio_plat_data[7].values &= 0x7f;					// AB25(GPIO7_7) : OUT default L	
	
	// GPIO_WIFI_LDO_ON
	gpio_plat_data[8].directions |= 0x04;				// (GPIO8_2) Set to Output

	if(GetBoardVersion() == eBdVer_PV1 || GetBoardVersion() == eBdVer_PV1B)
	{
		gpio_plat_data[12].directions &= 0xef;					// LCD_ESD_RESET(GPIO12_4) : IN
	}
	else
	{
		gpio_plat_data[12].directions &= 0xf7;					// LCD_ESD_RESET(GPIO12_3) : IN
		gpio_plat_data[12].directions |= 0x10;					// TP15(GPIO12_4) : OUT
	}
	
	gpio_plat_data[24].directions |= 0x14;					// CARD_LED(GPIO24_2), TP30(GPIO24_4) : OUT
	gpio_plat_data[25].directions |= 0x20;					// MICOM_SLEEP(GPIO25_5) : OUT

	gpio_plat_data[25].values |= 0x20;						// MICOM_SLEEP(GPIO25_5) : OUT default H
#endif

	for (i = 0; i < ARRAY_SIZE(d4_gpio_devices); i++) {
		ret = amba_device_register(d4_gpio_devices[i] , &iomem_resource);
	}
	return 0;
}
#ifndef CONFIG_SCORE_FAST_RESUME
subsys_initcall(drime4_gpio_init);
#else
fast_subsys_initcall(drime4_gpio_init);
#endif
