/*
 *  linux/drivers/usb/host/dwc_otg/hsic_core.h
 *
 *  Copyright 2012 Samsung Electronics Co., Ltd.
 *  http://www.samsung.com/
 *  DRIMeIV HSIC/USB-OTG header
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __HSIC_CORE_H__
#define __HSIC_CORE_H__

#include <mach/irqs.h>
#include <linux/slab.h>
#include <mach/gpio_map_d4jig2nd.h>
#include <linux/gpio.h>
/* #include "lm.h" */

/* Platform, Clock, USB, HSIC Base Regs */
#define __USB_BASE		(0x20000000)
#define __HSIC_BASE		(0x20100000)
#define __PLATFORM_CTRL_BASE	(0x30100000)	/* Plf Ctrl */
#define __CLOCK_CTRL_BASE	(0x30120000)	/* CMU */
#define __USB_CFG_BASE		(0x30160000)
#define __GCLKCON1		(__CLOCK_CTRL_BASE + 0x20)


#define HSIC_CLK		(__PLATFORM_CTRL_BASE + 0x304)
#define HSIC_CLK_SWRST		(__PLATFORM_CTRL_BASE + 0x504)

/* USB CFG Regs */
#define USB3_CLK_EN		(__PLATFORM_CTRL_BASE + 0x300)
#define USB3_CLK_SWRST		(__PLATFORM_CTRL_BASE + 0x500)

#define HSIC_MODE_EXCH		(__USB_CFG_BASE + 0x60)
#define USB3_PWR_DN		(__USB_CFG_BASE + 0x04)

/* Internal Regs */
#define CLK_PATH		(__USB_CFG_BASE + 0x10)
#define SS_CLK_EN		(__USB_CFG_BASE + 0xC)

/* HSIC Link and PHY Regs */
#define	__HSIC_LINK_BASE	(__HSIC_BASE)
#define	__HSIC_PHY_BASE		(__HSIC_BASE + 0xF0000)

/* PHY Regs Read/Write Macro */
/*
#define WRITE_HSIC_PHY_REG(reg_offset, value)    \
		*((volatile unsigned int *)(__HSIC_PHY_BASE + \
		(4 * reg_offset))) = (value)
#define READ_HSIC_PHY_REG(reg_offset)          \
		*((volatile unsigned int *)(__HSIC_PHY_BASE + \
		(4 * reg_offset)))
*/

typedef unsigned int uint32;

static void hsic_bus_init(void);

/*################# GPIO INTERFACE #################*/

#define __GPIO_BASE		(0x30050000)

#define GPIO_ENABLE		1
#define WRITE_GPIO_REG(group_number, reg_offset, value) \
	DWC_WRITE_REG32(ioremap((__GPIO_BASE + 0x3FC) + \
		(group_number * 0x1000) + (4 * reg_offset), 4), value)
#define READ_GPIO_REG(group_number, reg_offset) \
	DWC_READ_REG32(ioremap((__GPIO_BASE + 0x3FC) + \
		(group_number * 0x1000) + (4 * reg_offset), 4))

typedef enum {
	GPIO_IN,
	GPIO_OUT
} egpio_direct;

typedef union {
	uint32    value;
	struct {
		uint32 iodir:8;
		uint32 reserved8_31:24;
	} bit;
} reg_iodir;

typedef union {
	uint32    value;
	struct {
		uint32 iodata:8;
		uint32 reserved8_31:24;
	} bit;
} reg_iodata;

typedef enum {
	GPIODATA,	/* Data Register */
	GPIODIR,	/* Data Direction */
	GPIOIS,		/* Interrupt Type */
	GPIOIBE,	/* Both Interrupt Enable( Edge ) */
	GPIOIEV,	/* Both Interrupt Enable( Edge or Level ) */
	GPIOIE,		/* Interrupt Enable to Interupt Controller */
	GPIORIS,	/* Raw Interrupt Status */
	GPIOMIS,	/* Interrupt Pending */
	GPIOIC		/* Interrupt Clear in GPIO IP */
} egpio_reg_offset;

typedef enum {
	GPIO_BIT_0,
	GPIO_BIT_1
} edata_type;

void gpio_data_get(uint32 g_num, uint32 p_num, uint32 *value)
{
	reg_iodata iodata;
	iodata.value = READ_GPIO_REG(g_num, GPIODATA);
	*value = (iodata.value>>p_num) & GPIO_ENABLE;
}

void gpio_port_set(uint32 g_num, uint32 p_num, egpio_direct type)
{
	reg_iodir iodir;

	iodir.value = READ_GPIO_REG(g_num, GPIODIR);

	if (iodir.value & type<<p_num) {
		return;
	}

	if (type == GPIO_OUT) {
		iodir.value |= (GPIO_ENABLE<<p_num);
	} else if  (type == GPIO_IN) {
		iodir.value &= ~(GPIO_ENABLE<<p_num);
	}

	WRITE_GPIO_REG(g_num, GPIODIR, iodir.value);
}

void gpio_data_set(uint32 g_num, uint32 p_num, edata_type value)
{
	reg_iodata iodata;

	iodata.value = READ_GPIO_REG(g_num, GPIODATA);

	if (iodata.value & value<<p_num) {
		return;
	}

	if (value == GPIO_BIT_0) {
		iodata.value &= ~(GPIO_ENABLE<<p_num);
	} else if (value == GPIO_BIT_1) {
		iodata.value |= (GPIO_ENABLE<<p_num);
	}
	WRITE_GPIO_REG(g_num, GPIODATA, iodata.value);
}

/*################# GPIO INTERFACE #################*/

typedef enum {
	HSIC_PHY_RESET_CONTROL,
	HSIC_PHY_SUSPEND_CONTROL,
	HSIC_PHY_TEST_CONTROL,
	HSIC_PHY_DRIVING_CONTROL,
	HSIC_PHY_BIT_STUFFING_CONTROL,
	HSIC_LINK_SCALE_DOWN_MODE_CONTROL,
	HSIC_PHY_VOLTAGE_VALID_CONTROL,
	HSIC_PHY_VOLTAGE_REGULATOR_CONTROL
} ehsic_reg_offset;

typedef union {
	uint32    value;
	struct {
	/* OTG Link Block HCLK (AHB Bus Clock) Domain Reset ?
		Control { 0 : Assert Reset, 1 : Release Reset } */
		uint32 cfg_hresetn:1;
	/* HSIC PHY Block POR (Power On Reset) ?
		Control(HSIC PHY Power On 10us Reset ) { 0 :
		Release Reset, 1 : Assert Reset } */
		uint32 cfg_por:1;
	/* OTG Link Block PHY Clock (UTMI Clock) Domain Reset ?
		Control { 0 : Assert Reset, 1 : Release Reset } */
		uint32 cfg_linkrstn:1;
	/* HSIC PHY Port Reset ? Control .
		(UTMI OPMODE ? Reset ) { 0 :
		Release Reset, 1 : Assert Reset } */
		uint32 cfg_port_reset:1;
	/* FIFO Clock FIFO CS Gating |  { 0: Gating Enable, 1: Bypass
		(Gating Disable) } */
		uint32 cfg_fifo_cs_bypass:1;
	/* Reserved */
		uint32 reserved5_31:27;
	} bit;
} reg_hsic_phy_reset_control;

typedef union {
	uint32    value;
	struct {
	/* PHY SUSPEND Mode ? { 0 : (UTMI_SUSPENDM & UTMI_L1_SUSPENDM) -> ?
		: UTMI_SUSPENDM } */
		uint32 suspend_sel:1;
	/* Reserved */
		uint32 reserved1_3:3;
	/* PHY Sleep Mode  ? { 0 : UTMI_LPM_SLEEPM (from OTG Link) 1 :
		REG_SLEEPM  } */
		uint32 sleepm_sel:1;
	/* SLEEPM_SEL (REG_SLEEPM Sleep Mode ) */
		uint32 reg_sleepm:1;
	/* Reserved */
		uint32 reserved6_31:26;
	} bit;
} reg_hsic_phy_suspend_control;

typedef union {
	uint32    value;
	struct {
	/* Common Block Power-Down Control {
		0 : Common Block is powered up,
		1 : Common Block is powered down. } */
		uint32 cfg_common:1;
	/* IDDQ Test Enable (This test signal enables you to perform
		IDDQ testing by powering down all analog blocks) { 0 :
		The analog blocks are powered up, 1 :
		The analog blocks are powered down } */
		uint32 cfg_siddq:1;
	/* Loopback Test Enable (Loopback mode is for test purposes only;
		it cannot be used for normal operation) { 0:
		During data transmission, the receive logic is disabled, 1:
		During data transmission, the receive logic is enabled } */
		uint32 cfg_loopback:1;
	/* Reserved */
		uint32 reserved3_31:29;
	} bit;
} reg_hsic_phy_test_control;

typedef union {
	uint32    value;
	struct {
	/* DPPD_SEL { 0: DP_PULLDOWN = 0, 1: DP_PULLDOWN = 1 } */
		uint32 dppdown_reg:1;
	/* Bus Keepers Resistor (DP_PULLDOWN) Signal Selection. { 0 :
		UTMI_DPPDOWN  (OTG Link ), 1: DPPDOWN_REG } */
		uint32 dppd_sel:1;
	/* Reserved */
		uint32 reserved2_3:2;
	/* DMPD_SEL { 0: DM_PULLDOWN = 0, 1: DM_PULLDOWN = 1 } */
		uint32 dmpdown_reg:1;
	/* Bus Keepers Resistor (DM_PULLDOWN) Signal Selection. { 0 :
		UTMI_DMPDOWN  (OTG Link ), 1: DMPDOWN_REG } */
		uint32 dmpd_sel:1;
	/* Reserved */
		uint32 reserved6_7:2;
	/* Driver Pull-Up Impedance Adjustment { 00 : +5%, 01 :
		Design Default, 10 : +5%, 11 : +11% } */
		uint32 txrputune:2;
	/* Reserved */
		uint32 reserved10_11:2;
	/* Driver Pull-Down Impedance Adjustment { 00 : +5%, 01 :
		Design Default, 10 : +5%, 11 : +11% } */
		uint32 txrpdtune:2;
	/* Reserved */
		uint32 reserved14_15:2;
	/* Driver Slew Rate Adjustment { 1111: +20%, 0111: +10%, 0011:
		Design default, 0001: -10%, 0000: -20% } */
		uint32 txsrtune:4;
	/* Reserved */
		uint32 reserved20_23:4;
	/* Reference Clock Selection { 0 : 12MHz Clock from Clock Gen, 1 :
		12MHz Clock from USB3 PHY } */
		uint32 refclk_sel:1;
	/* Reserved */
		uint32 reserved25_31:7;
	} bit;
} reg_hsic_phy_driving_control;

typedef union {
	uint32    value;
	struct {
	/* This signal controls bit stuffing on DATAIN[7:0]
		when OPMODE[1:0] = 2¯b11 {
		0: Bit stuffing is disabled,
		1: Bit stuffing is enabled. } */
		uint32 txbitstuffen:1;
	/* This signal controls bit stuffing on DATAINH[7:0]
		when OPMODE[1:0] = 2¯b11 {
		0: Bit stuffing is disabled,
		1: Bit stuffing is enabled. } */
		uint32 txbitstuffenh:1;
	/* Reserved */
		uint32 reserved2_31:30;
	} bit;
} reg_hsic_phy_bit_stuffing_control;

typedef union {
	uint32    value;
	struct {
	/* Scale-Down Mode : When this signal is enabled during simulation,
	the core uses scaled-down timing values,
	resulting in faster simulations.
	When it is disabled, actual timing values are used.
	 {	b00: Disables all scale-downs. Actual timing values are used.
		Required for synthesis.
		b01: Enables scale-down of all timing values except Device
			mode suspend and resume.
			These include: Speed enumeration. HNP/SRP. Host mode
			suspend and resume.
		b10: Enables scale-down of Device mode suspend and resume
			timing values only.
		b11: Enables bit 0 and bit 1 scale-down timing values. }
	*/
		uint32 scaledown:2;
	/* Reserved */
		uint32 reserved2_31:30;
    } bit;
} reg_hsic_link_scale_down_mode_control;

typedef union {
	uint32    value;
	struct {
	/* A-Device Session Valid ?{ 0: Session End, 1: Session Valid } */
		uint32 cfg_avalid:1;
	/* B-Device Session Valid ? { 0: Session End, 1: Session Valid } */
		uint32 cfg_bvalid:1;
	/* PHY Host Device ?(IDDIG PHY HSIC PHY ? ) */
		uint32 cfg_iddig:1;
	/* VBUS_VALID ?{ 0: Vbus is not valid, 1: Vbus is valid } */
		uint32 cfg_vbusvalid:1;
	/* B-Device Session End ?{ 0: Session Valid, 1: Session End } */
		uint32 cfg_sessend:1;
	/* Reserved */
		uint32 reserved5_31:27;
	} bit;
} reg_hsic_phy_voltage_valid_control;

typedef union {
	uint32    value;
	struct {
	/* VREG12 Boost Enable { 0: Boost Enable,1: Boost Disable } */
		uint32 vregboostenb:1;
	/* Enables or disables VREG12 { 0: VREG12 Block is disabled
		(VDD18 should be tied to VDD12),1: VREG12 Block is enabled } */
		uint32 vregpu:1;
	/* This signal should be 1 b1 */
		uint32 vregxtcap:1;
	/* Reserved */
		uint32 reserved3_31:29;
	} bit;
} reg_hsic_phy_voltage_regulator_control;

#define ON 1
#define OFF 0

typedef union {
	uint32    value;
	struct {
		unsigned int sd1_clk_con:1;		/* 0:OFF, 1:ON */
		unsigned int i2s_clk_con:1;		/* 0:OFF, 1:ON */
		unsigned int extserslk_con:1;	/* 0:OFF, 1:ON */
		unsigned int hsic_clk_con:1;	/* 0:OFF, 1:ON */
		unsigned int uart0_clk_con:1;	/* 0:OFF, 1:ON */
		unsigned int usb3_clk_con:1;	/* 0:OFF, 1:ON */
		unsigned int reserved6_31:26;
	} bit;
} CLOCK_CTRL_GCLKCON1;

/*
 * Function:  hsic_bus_init
 * Input: Void
 * Return: Void
 * Description: This function initializes all the HSIC related
 * platform settings.
 */
static void hsic_bus_init()
{
	reg_hsic_phy_reset_control		hsic_phy_reset_control;
	reg_hsic_phy_voltage_valid_control	hsic_phy_voltage_valid_control;
	reg_hsic_phy_test_control		hsic_phy_test_control;

	/* Enable the Global Clock for HSIC */
	DWC_WRITE_REG32(ioremap(__GCLKCON1, 4), \
			DWC_READ_REG32(ioremap(__GCLKCON1, 4)) | 0x10);
	DWC_WRITE_REG32(ioremap(HSIC_CLK, 4), ON);
	DWC_WRITE_REG32(ioremap(HSIC_CLK_SWRST, 4), ON);

	/* Enable the Global Clock for USB3.0 For PHY */
	/* USB Clock Enable */
	DWC_WRITE_REG32(ioremap(__GCLKCON1, 4), \
			DWC_READ_REG32(ioremap(__GCLKCON1, 4)) | 0x40);
	DWC_WRITE_REG32(ioremap(USB3_CLK_EN, 4), ON);
	DWC_WRITE_REG32(ioremap(USB3_CLK_SWRST, 4), ON);

	/* USB Bus Configuration for 480Mb/s */
	/* USB3 Bus clk enable */
	DWC_WRITE_REG32(ioremap(USB3_CLK_EN, 4), 0xFFFFFFFF);
	/* HSIC Mode exchange mode */
	DWC_WRITE_REG32(ioremap(HSIC_MODE_EXCH, 4), 0x0);
	/* USB3 Power down off */
	DWC_WRITE_REG32(ioremap(USB3_PWR_DN, 4), 0x5400);

	/* Internal PLL */
	/* CLK Path */
	DWC_WRITE_REG32(ioremap(CLK_PATH, 4), 0x1080);
	/* Super speed Clk Enable */
	DWC_WRITE_REG32(ioremap(SS_CLK_EN, 4), 0x273C);
	DWC_MDELAY(100);
	/* CLK Path */
	DWC_WRITE_REG32(ioremap(SS_CLK_EN, 4), 0x2738);

	/* HSIC PHY Settings */
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + 0x0C, 4), 0x0032200);
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + 0x10, 4), 0x00);
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + 0x14, 4), 0x00);
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + 0x18, 4), 0x0B);
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + 0x1C, 4), 0x03);

#if 0
	/* GPIO Port Settings for SMSC4640 hub reset */
	GPIO_port_set(22, 5, 1);
#endif

	/* HSIC Reset */
	hsic_phy_reset_control.value = 0;
	hsic_phy_reset_control.bit.cfg_hresetn = 0;
	hsic_phy_reset_control.bit.cfg_por = 1;
	hsic_phy_reset_control.bit.cfg_linkrstn = 0;
	hsic_phy_reset_control.bit.cfg_port_reset = 0;
	hsic_phy_reset_control.bit.cfg_fifo_cs_bypass = 1;
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + \
		HSIC_PHY_RESET_CONTROL * 4, 4), hsic_phy_reset_control.value);
	DWC_MDELAY(10);

	/* Common Block Power Up */
	hsic_phy_test_control.value = 0;
	hsic_phy_test_control.bit.cfg_common = 0;	/* 45n PHY */
	hsic_phy_test_control.bit.cfg_siddq = 0;
	hsic_phy_test_control.bit.cfg_loopback = 0;
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + \
		HSIC_PHY_TEST_CONTROL * 4, 4), hsic_phy_test_control.value);

	/* Voltage Valid Setting */
	hsic_phy_voltage_valid_control.value = 0;
	hsic_phy_voltage_valid_control.bit.cfg_avalid = 1;
	hsic_phy_voltage_valid_control.bit.cfg_bvalid = 1;
	hsic_phy_voltage_valid_control.bit.cfg_iddig = 0;
	hsic_phy_voltage_valid_control.bit.cfg_vbusvalid = 1;
	hsic_phy_voltage_valid_control.bit.cfg_sessend = 0;
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + \
		HSIC_PHY_VOLTAGE_VALID_CONTROL * 4, 4), \
		hsic_phy_voltage_valid_control.value);

	/* HSIC Reset Release */
	hsic_phy_reset_control.value = 0;
	hsic_phy_reset_control.bit.cfg_fifo_cs_bypass = 1; /* 0x10 */
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + \
		HSIC_PHY_RESET_CONTROL * 4, 4), hsic_phy_reset_control.value);
	DWC_MDELAY(100);

	hsic_phy_reset_control.bit.cfg_linkrstn = 1; /* 0x14 */
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + \
		HSIC_PHY_RESET_CONTROL * 4, 4), hsic_phy_reset_control.value);
	DWC_MDELAY(100);

	hsic_phy_reset_control.bit.cfg_hresetn = 1; /* 0x15 */
	DWC_WRITE_REG32(ioremap(__HSIC_PHY_BASE + \
		HSIC_PHY_RESET_CONTROL * 4, 4), hsic_phy_reset_control.value);


#if 0
	/* GPIO Data Settings for SMSC4640 hub */
	GPIO_data_set(22, 5, 0);
	DWC_MDELAY(2);
	GPIO_data_set(22, 5, 1);
#endif
}

#endif /* __HSIC_CORE_H__ */
