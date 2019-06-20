/* linux/drivers/pinctrl/pinmux-drime4.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Driver for DRIME4 Pin Controller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinconf-drime4.h>

#define DRIME4_PMX_CON_FUNC_WRT			(1 << 17)
#define DRIME4_PMX_CON_PCON_WRT			(1 << 16)
#define DRIME4_PMX_CON_FUNC(func)		((func) << 8)
#define DRIME4_PMX_CON_INPUT_MASK		(1 << 6)
#define DRIME4_PMX_CON_INPUT_SHIFT		(6)
#define DRIME4_PMX_CON_INPUT_MODE_MASK		(1 << 5)
#define DRIME4_PMX_CON_INPUT_MODE_SHIFT		(5)
#define DRIME4_PMX_CON_SLEW_RATE_MASK		(1 << 4)
#define DRIME4_PMX_CON_SLEW_RATE_SHIFT		(4)
#define DRIME4_PMX_CON_PULLUP_DOWN_MASK		(1 << 3)
#define DRIME4_PMX_CON_PULLUP_DOWN_SHIFT	(3)
#define DRIME4_PMX_CON_PULLUP_DOWN_SEL_MASK	(1 << 2)
#define DRIME4_PMX_CON_PULLUP_DOWN_SEL_SHIFT	(1 << 2)
#define DRIME4_PMX_CON_DRIVE_STRENGTH_MASK	(3 << 0)
#define DRIME4_PMX_CON_DRIVE_STRENGTH_SHIFT	(0)

#define DRIME4_PMX_MODE_CTRL			(0x1000)
#define DRIME4_PMX_MODE_CTRL_I2S2_GPIO_MASK	(1 << 9)
#define DRIME4_PMX_MODE_CTRL_I2S2_GPIO_SHIFT	(9)
#define DRIME4_PMX_MODE_CTRL_I2S2_HDMI_MASK	(1 << 8)
#define DRIME4_PMX_MODE_CTRL_I2S2_HDMI_SHIFT	(8)
#define DRIME4_PMX_MODE_CTRL_SLCD_RESET_MASK	(1 << 3)
#define DRIME4_PMX_MODE_CTRL_SLCD_RESET_SHIFT	(3)
#define DRIME4_PMX_MODE_CTRL_MLCD_RESET_MASK	(1 << 2)
#define DRIME4_PMX_MODE_CTRL_MLCD_RESET_SHIFT	(2)
#define DRIME4_PMX_MODE_CTRL_CODEC_SRP_SEL_MASK	(1 << 0)
#define DRIME4_PMX_MODE_CTRL_CODEC_SRP_SEL_SHIFT	(0)

#define DRIME4_PMX_VSEL18_INIT_VAL_MASK		(0xFFFF << 0)
#define DRIME4_PMX_VSEL18_INIT_VAL_SHIFT	(0)

#define DRIME4_PMX_VSEL18_COUNT_MASK		(0xFFFF << 0)
#define DRIME4_PMX_VSEL18_COUNT_SHIFT		(0)

#define DRIME4_PMX_VSEL18_CTRL_SD1_CLK_INV_MASK	(1 << 15)
#define DRIME4_PMX_VSEL18_CTRL_SD1_CLK_INV_SHIFT	(15)
#define DRIME4_PMX_VSEL18_CTRL_SD0_SEL_MASK	(1 << 9)
#define DRIME4_PMX_VSEL18_CTRL_SD0_SEL_SHIFT	(9)
#define DRIME4_PMX_VSEL18_CTRL_SD0_CPU_MASK	(1 << 8)
#define DRIME4_PMX_VSEL18_CTRL_SD0_CPU_SHIFT	(8)
#define DRIME4_PMX_VSEL18_CTRL_SD1_MASK		(1 << 4)
#define DRIME4_PMX_VSEL18_CTRL_SD1_SHIFT	(4)
#define DRIME4_PMX_VSEL18_CTRL_SD0_MASK		(1 << 3)
#define DRIME4_PMX_VSEL18_CTRL_SD0_SHIFT	(3)
#define DRIME4_PMX_VSEL18_CTRL_ATA_MASK		(1 << 2)
#define DRIME4_PMX_VSEL18_CTRL_ATA_SHIFT	(2)
#define DRIME4_PMX_VSEL18_CTRL_SLCD_MASK	(1 << 1)
#define DRIME4_PMX_VSEL18_CTRL_SLCD_SHIFT	(1)
#define DRIME4_PMX_VSEL18_CTRL_MLCD_MASK	(1 << 0)
#define DRIME4_PMX_VSEL18_CTRL_MLCD_SHIFT	(0)

#define DRIME4_PMX_SYS_DET_USB_MASK		(1 << 2)
#define DRIME4_PMX_SYS_DET_USB_SHIFT		(2)
#define DRIME4_PMX_SYS_DET_SD1_MASK		(1 << 1)
#define DRIME4_PMX_SYS_DET_SD1_SHIFT		(1)
#define DRIME4_PMX_SYS_DET_SD0_MASK		(1 << 0)
#define DRIME4_PMX_SYS_DET_SD0_SHIFT		(0)

#define FUNC_MASK				(0xf << 8)
#define FUNC1					(1 << 8)
#define FUNC2					(1 << 9)
#define FUNC3					(1 << 10)
#define FUNC4					(1 << 11)

#define DRIME4_NUM_PADS 284

/* Pad names for the pinmux subsystem */
const struct pinctrl_pin_desc __refdata drime4_pads[] = {
	PINCTRL_PIN(1, "CEC_CON"),
	PINCTRL_PIN(2, "HOTPLUG_CON"),
	PINCTRL_PIN(3, "AV_DET_CON"),
	PINCTRL_PIN(4, "SPI0_CLK_CON"),
	PINCTRL_PIN(5, "SPI0_DI_CON"),
	PINCTRL_PIN(6, "SPI0_DO_CON"),
	PINCTRL_PIN(7, "SPI0_CS_CON"),
	PINCTRL_PIN(8, "MLCD_RESET_CON"),
	PINCTRL_PIN(9, "MLCD_EXT_EVFCLK_CON"),
	PINCTRL_PIN(10, "MLCD_EXT_CLK_CON"),
	PINCTRL_PIN(11, "MLCD_CLK_CON"),
	PINCTRL_PIN(12, "MLCD_EN_CON"),
	PINCTRL_PIN(13, "MLCD_VSYNC_CON"),
	PINCTRL_PIN(14, "MLCD_HSYNC_CON"),
	PINCTRL_PIN(15, "MLCD_DATA0_CON"),
	PINCTRL_PIN(16, "MLCD_DATA1_CON"),
	PINCTRL_PIN(17, "MLCD_DATA2_CON"),
	PINCTRL_PIN(18, "MLCD_DATA3_CON"),
	PINCTRL_PIN(19, "MLCD_DATA4_CON"),
	PINCTRL_PIN(20, "MLCD_DATA5_CON"),
	PINCTRL_PIN(21, "MLCD_DATA6_CON"),
	PINCTRL_PIN(22, "MLCD_DATA7_CON"),
	PINCTRL_PIN(23, "MLCD_DATA8_CON"),
	PINCTRL_PIN(24, "MLCD_DATA9_CON"),
	PINCTRL_PIN(25, "MLCD_DATA10_CON"),
	PINCTRL_PIN(26, "MLCD_DATA11_CON"),
	PINCTRL_PIN(27, "MLCD_DATA12_CON"),
	PINCTRL_PIN(28, "MLCD_DATA13_CON"),
	PINCTRL_PIN(29, "MLCD_DATA14_CON"),
	PINCTRL_PIN(30, "MLCD_DATA15_CON"),
	PINCTRL_PIN(31, "MLCD_DATA16_CON"),
	PINCTRL_PIN(32, "MLCD_DATA17_CON"),
	PINCTRL_PIN(33, "MLCD_DATA18_CON"),
	PINCTRL_PIN(34, "MLCD_DATA19_CON"),
	PINCTRL_PIN(35, "MLCD_DATA20_CON"),
	PINCTRL_PIN(36, "MLCD_DATA21_CON"),
	PINCTRL_PIN(37, "MLCD_DATA22_CON"),
	PINCTRL_PIN(38, "MLCD_DATA23_CON"),
	PINCTRL_PIN(39, "SPI1_CLK_CON"),
	PINCTRL_PIN(40, "SPI1_DI_CON"),
	PINCTRL_PIN(41, "SPI1_DO_CON"),
	PINCTRL_PIN(42, "SPI1_CS_CON"),
	PINCTRL_PIN(43, "SLCD_RESET_CON"),
	PINCTRL_PIN(44, "SLCD_EXT_CLK_CON"),
	PINCTRL_PIN(45, "SLCD_CLK_CON"),
	PINCTRL_PIN(46, "SLCD_EN_CON"),
	PINCTRL_PIN(47, "SLCD_VSYNC_CON"),
	PINCTRL_PIN(48, "SLCD_HSYNC_CON"),
	PINCTRL_PIN(49, "SLCD_DATA0_CON"),
	PINCTRL_PIN(50, "SLCD_DATA1_CON"),
	PINCTRL_PIN(51, "SLCD_DATA2_CON"),
	PINCTRL_PIN(52, "SLCD_DATA3_CON"),
	PINCTRL_PIN(53, "SLCD_DATA4_CON"),
	PINCTRL_PIN(54, "SLCD_DATA5_CON"),
	PINCTRL_PIN(55, "SLCD_DATA6_CON"),
	PINCTRL_PIN(56, "SLCD_DATA7_CON"),
	PINCTRL_PIN(57, "SLCD_DATA8_CON"),
	PINCTRL_PIN(58, "SLCD_DATA9_CON"),
	PINCTRL_PIN(59, "SLCD_DATA10_CON"),
	PINCTRL_PIN(60, "SLCD_DATA11_CON"),
	PINCTRL_PIN(61, "SLCD_DATA12_CON"),
	PINCTRL_PIN(62, "SLCD_DATA13_CON"),
	PINCTRL_PIN(63, "SLCD_DATA14_CON"),
	PINCTRL_PIN(64, "SLCD_DATA15_CON"),
	PINCTRL_PIN(65, "SLCD_DATA16_CON"),
	PINCTRL_PIN(66, "SLCD_DATA17_CON"),
	PINCTRL_PIN(67, "SLCD_DATA18_CON"),
	PINCTRL_PIN(68, "SLCD_DATA19_CON"),
	PINCTRL_PIN(69, "SLCD_DATA20_CON"),
	PINCTRL_PIN(70, "SLCD_DATA21_CON"),
	PINCTRL_PIN(71, "SLCD_DATA22_CON"),
	PINCTRL_PIN(72, "SLCD_DATA23_CON"),
	PINCTRL_PIN(73, "ZPIB0_CON"),
	PINCTRL_PIN(74, "ZPIC0_CON"),
	PINCTRL_PIN(75, "ZPIB1_CON"),
	PINCTRL_PIN(76, "ZPIC1_CON"),
	PINCTRL_PIN(77, "SD0_D0_CON"),
	PINCTRL_PIN(78, "SD0_D1_CON"),
	PINCTRL_PIN(79, "SD0_D2_CON"),
	PINCTRL_PIN(80, "SD0_D3_CON"),
	PINCTRL_PIN(81, "SD0_CMD_CON"),
	PINCTRL_PIN(82, "SD0_CLK_CON"),
	PINCTRL_PIN(83, "SD0_WP_CON"),
	PINCTRL_PIN(84, "SD0_CDX_CON"),
	PINCTRL_PIN(85, "SD0_PWEN_CON"),
	PINCTRL_PIN(86, "SD0_VOLT_CON"),
	PINCTRL_PIN(87, "I2C2_SCL_CON"),
	PINCTRL_PIN(88, "I2C2_SDA_CON"),
	PINCTRL_PIN(89, "I2C3_SCL_CON"),
	PINCTRL_PIN(90, "I2C3_SDA_CON"),
	PINCTRL_PIN(91, "SPI2_CLK_CON"),
	PINCTRL_PIN(92, "SPI2_DI_CON"),
	PINCTRL_PIN(93, "SPI2_DO_CON"),
	PINCTRL_PIN(94, "SPI2_CS_CON"),
	PINCTRL_PIN(95, "I2S0_CCK_CON"),
	PINCTRL_PIN(96, "I2S0_LCK_CON"),
	PINCTRL_PIN(97, "I2S0_BCK_CON"),
	PINCTRL_PIN(98, "I2S0_DI_CON"),
	PINCTRL_PIN(99, "I2S0_DO_CON"),
	PINCTRL_PIN(100, "A9_NTRST_CON"),
	PINCTRL_PIN(101, "A9_TCK_CON"),
	PINCTRL_PIN(102, "A9_TMS_CON"),
	PINCTRL_PIN(103, "A9_TDI_CON"),
	PINCTRL_PIN(104, "A9_TDO_CON"),
	PINCTRL_PIN(105, "A9_SRSTN_CON"),
	PINCTRL_PIN(106, "SYS_GPIO0_CON"),
	PINCTRL_PIN(107, "UART0_RX_CON"),
	PINCTRL_PIN(108, "UART0_TX_CON"),
	PINCTRL_PIN(109, "UART1_RX_CON"),
	PINCTRL_PIN(110, "UART1_TX_CON"),
	PINCTRL_PIN(111, "UART1_CTS_CON"),
	PINCTRL_PIN(112, "UART1_RTS_CON"),
	PINCTRL_PIN(113, "EXTIN0_CON"),
	PINCTRL_PIN(114, "EXTIN1_CON"),
	PINCTRL_PIN(115, "EXTIN2_CON"),
	PINCTRL_PIN(116, "EXTIN3_CON"),
	PINCTRL_PIN(117, "EXTIN4_CON"),
	PINCTRL_PIN(118, "EXTIN5_CON"),
	PINCTRL_PIN(119, "EXTIN6_CON"),
	PINCTRL_PIN(120, "EXTIN7_CON"),
	PINCTRL_PIN(121, "EXTIN8_CON"),
	PINCTRL_PIN(122, "PWM0_CON"),
	PINCTRL_PIN(123, "PWM1_CON"),
	PINCTRL_PIN(124, "PWM2_CON"),
	PINCTRL_PIN(125, "PWM3_CON"),
	PINCTRL_PIN(126, "PWM4_CON"),
	PINCTRL_PIN(127, "PWM5_CON"),
	PINCTRL_PIN(128, "PWM6_CON"),
	PINCTRL_PIN(129, "PWM7_CON"),
	PINCTRL_PIN(130, "PWM8_CON"),
	PINCTRL_PIN(131, "PWM9_CON"),
	PINCTRL_PIN(132, "PWM10_CON"),
	PINCTRL_PIN(133, "PWM11_CON"),
	PINCTRL_PIN(134, "SPI3_CLK_CON"),
	PINCTRL_PIN(135, "SPI3_DI_CON"),
	PINCTRL_PIN(136, "SPI3_DO_CON"),
	PINCTRL_PIN(137, "SPI3_CS_CON"),
	PINCTRL_PIN(138, "SPI4_CLK_CON"),
	PINCTRL_PIN(139, "SPI4_DI_CON"),
	PINCTRL_PIN(140, "SPI4_DO_CON"),
	PINCTRL_PIN(141, "SPI4_CS_CON"),
	PINCTRL_PIN(142, "SPI5_CLK_CON"),
	PINCTRL_PIN(143, "SPI5_DI_CON"),
	PINCTRL_PIN(144, "SPI5_DO_CON"),
	PINCTRL_PIN(145, "SPI5_CS_CON"),
	PINCTRL_PIN(146, "I2C0_SCL_CON"),
	PINCTRL_PIN(147, "I2C0_SDA_CON"),
	PINCTRL_PIN(148, "I2C1_SCL_CON"),
	PINCTRL_PIN(149, "I2C1_SDA_CON"),
	PINCTRL_PIN(150, "USB_DET_CON"),
	PINCTRL_PIN(151, "FADQ0_CON"),
	PINCTRL_PIN(152, "FADQ1_CON"),
	PINCTRL_PIN(153, "FADQ2_CON"),
	PINCTRL_PIN(154, "FADQ3_CON"),
	PINCTRL_PIN(155, "FADQ4_CON"),
	PINCTRL_PIN(156, "FADQ5_CON"),
	PINCTRL_PIN(157, "FADQ6_CON"),
	PINCTRL_PIN(158, "FADQ7_CON"),
	PINCTRL_PIN(159, "FADQ8_CON"),
	PINCTRL_PIN(160, "FADQ9_CON"),
	PINCTRL_PIN(161, "FADQ10_CON"),
	PINCTRL_PIN(162, "FADQ11_CON"),
	PINCTRL_PIN(163, "FADQ12_CON"),
	PINCTRL_PIN(164, "FADQ13_CON"),
	PINCTRL_PIN(165, "FADQ14_CON"),
	PINCTRL_PIN(166, "FADQ15_CON"),
	PINCTRL_PIN(167, "FA16_CON"),
	PINCTRL_PIN(168, "FA17_CON"),
	PINCTRL_PIN(169, "FA18_CON"),
	PINCTRL_PIN(170, "FA19_CON"),
	PINCTRL_PIN(171, "FA20_CON"),
	PINCTRL_PIN(172, "FA21_CON"),
	PINCTRL_PIN(173, "FA22_CON"),
	PINCTRL_PIN(174, "FA23_CON"),
	PINCTRL_PIN(175, "FA24_CON"),
	PINCTRL_PIN(176, "FA25_CON"),
	PINCTRL_PIN(177, "FCEN0_CON"),
	PINCTRL_PIN(178, "FCEN1_CON"),
	PINCTRL_PIN(179, "FRESETN_CON"),
	PINCTRL_PIN(180, "FWEN_CON"),
	PINCTRL_PIN(181, "FWPN_CON"),
	PINCTRL_PIN(182, "FADVN_CON"),
	PINCTRL_PIN(183, "FOEN_CON"),
	PINCTRL_PIN(184, "FCLK_CON"),
	PINCTRL_PIN(185, "FBCLK_CON"),
	PINCTRL_PIN(186, "SL_TXD0_P_CON"),
	PINCTRL_PIN(187, "SL_TXD0_N_CON"),
	PINCTRL_PIN(188, "SL_TXCLK0_P_CON"),
	PINCTRL_PIN(189, "SL_TXCLK0_N_CON"),
	PINCTRL_PIN(190, "SL_D0_P_CON"),
	PINCTRL_PIN(191, "SL_D0_N_CON"),
	PINCTRL_PIN(192, "SL_D1_P_CON"),
	PINCTRL_PIN(193, "SL_D1_N_CON"),
	PINCTRL_PIN(194, "SL_D2_P_CON"),
	PINCTRL_PIN(195, "SL_D2_N_CON"),
	PINCTRL_PIN(196, "SL_D3_P_CON"),
	PINCTRL_PIN(197, "SL_D3_N_CON"),
	PINCTRL_PIN(198, "SL_CLK0_P_CON"),
	PINCTRL_PIN(199, "SL_CLK0_N_CON"),
	PINCTRL_PIN(200, "SL_D4_P_CON"),
	PINCTRL_PIN(201, "SL_D4_N_CON"),
	PINCTRL_PIN(202, "SL_D5_P_CON"),
	PINCTRL_PIN(203, "SL_D5_N_CON"),
	PINCTRL_PIN(204, "SL_D6_P_CON"),
	PINCTRL_PIN(205, "SL_D6_N_CON"),
	PINCTRL_PIN(206, "SL_D7_P_CON"),
	PINCTRL_PIN(207, "SL_D7_N_CON"),
	PINCTRL_PIN(208, "SL_CLK1_P_CON"),
	PINCTRL_PIN(209, "SL_CLK1_N_CON"),
	PINCTRL_PIN(210, "SL_D8_P_CON"),
	PINCTRL_PIN(211, "SL_D8_N_CON"),
	PINCTRL_PIN(212, "SL_D9_P_CON"),
	PINCTRL_PIN(213, "SL_D9_N_CON"),
	PINCTRL_PIN(214, "SL_D10_P_CON"),
	PINCTRL_PIN(215, "SL_D10_N_CON"),
	PINCTRL_PIN(216, "SL_D11_P_CON"),
	PINCTRL_PIN(217, "SL_D11_N_CON"),
	PINCTRL_PIN(218, "SL_CLK2_P_CON"),
	PINCTRL_PIN(219, "SL_CLK2_N_CON"),
	PINCTRL_PIN(220, "SL_D12_P_CON"),
	PINCTRL_PIN(221, "SL_D12_N_CON"),
	PINCTRL_PIN(222, "SL_D13_P_CON"),
	PINCTRL_PIN(223, "SL_D13_N_CON"),
	PINCTRL_PIN(224, "SL_D14_P_CON"),
	PINCTRL_PIN(225, "SL_D14_N_CON"),
	PINCTRL_PIN(226, "SL_D15_P_CON"),
	PINCTRL_PIN(227, "SL_D15_N_CON"),
	PINCTRL_PIN(228, "SL_CLK3_P_CON"),
	PINCTRL_PIN(229, "SL_CLK3_N_CON"),
	PINCTRL_PIN(230, "SL_TXD1_P_CON"),
	PINCTRL_PIN(231, "SL_TXD1_N_CON"),
	PINCTRL_PIN(232, "SL_TXCLK1_P_CON"),
	PINCTRL_PIN(233, "SL_TXCLK1_N_CON"),
	PINCTRL_PIN(234, "SRP_NTRST_CON"),
	PINCTRL_PIN(235, "SRP_TCK_CON"),
	PINCTRL_PIN(236, "SRP_TMS_CON"),
	PINCTRL_PIN(237, "SRP_TDI_CON"),
	PINCTRL_PIN(238, "SRP_TDO_CON"),
	PINCTRL_PIN(239, "SRP_SRSTN_CON"),
	PINCTRL_PIN(240, "SD1_WP_CON"),
	PINCTRL_PIN(241, "SD1_CDX_CON"),
	PINCTRL_PIN(242, "SD1_PWEN_CON"),
	PINCTRL_PIN(243, "ATA_DD0_CON"),
	PINCTRL_PIN(244, "ATA_DD1_CON"),
	PINCTRL_PIN(245, "ATA_DD2_CON"),
	PINCTRL_PIN(246, "ATA_DD3_CON"),
	PINCTRL_PIN(247, "ATA_DD4_CON"),
	PINCTRL_PIN(248, "ATA_DD5_CON"),
	PINCTRL_PIN(249, "ATA_DD6_CON"),
	PINCTRL_PIN(250, "ATA_DD7_CON"),
	PINCTRL_PIN(251, "ATA_DD8_CON"),
	PINCTRL_PIN(252, "ATA_DD9_CON"),
	PINCTRL_PIN(253, "ATA_DD10_CON"),
	PINCTRL_PIN(254, "ATA_DD11_CON"),
	PINCTRL_PIN(255, "ATA_DD12_CON"),
	PINCTRL_PIN(256, "ATA_DD13_CON"),
	PINCTRL_PIN(257, "ATA_DD14_CON"),
	PINCTRL_PIN(258, "ATA_DD15_CON"),
	PINCTRL_PIN(259, "ATA_RESETB_CON"),
	PINCTRL_PIN(260, "ATA_CS0B_CON"),
	PINCTRL_PIN(261, "ATA_CS1B_CON"),
	PINCTRL_PIN(262, "ATA_DIORB_CON"),
	PINCTRL_PIN(263, "ATA_DIOWB_CON"),
	PINCTRL_PIN(264, "ATA_DA0_CON"),
	PINCTRL_PIN(265, "ATA_DA1_CON"),
	PINCTRL_PIN(266, "ATA_DA2_CON"),
	PINCTRL_PIN(267, "SYS_GPIO1_CON"),
	PINCTRL_PIN(268, "SYS_GPIO2_CON"),
	PINCTRL_PIN(269, "SD1_D0_CON"),
	PINCTRL_PIN(270, "SD1_D1_CON"),
	PINCTRL_PIN(271, "SD1_D2_CON"),
	PINCTRL_PIN(272, "SD1_D3_CON"),
	PINCTRL_PIN(273, "SD1_D4_CON"),
	PINCTRL_PIN(274, "SD1_D5_CON"),
	PINCTRL_PIN(275, "SD1_D6_CON"),
	PINCTRL_PIN(276, "SD1_D7_CON"),
	PINCTRL_PIN(277, "SD1_CMD_CON"),
	PINCTRL_PIN(278, "SD1_CLK_CON"),
	PINCTRL_PIN(279, "SD1_RSTN_CON"),
	PINCTRL_PIN(1025, "MODE_CTRL"),
	PINCTRL_PIN(1026, "CNT_INIT"),
	PINCTRL_PIN(1027, "VSELCNT"),
	PINCTRL_PIN(1028, "VSEL_CTRL"),
	PINCTRL_PIN(1029, "SYS_DET"),
};

/**
 * @dev: a pointer back to containing device
 * @virtbase: the offset to the controller in virtual memory
 */
struct drime4_pmx {
	struct device *dev;
	struct pinctrl_dev *pctl;
	u32 phybase;
	u32 physize;
	void __iomem *virtbase;
};

/**
 * struct drime4_pin_group - describes a DRIMe4 pin group
 * @name: the name of this specific pin group
 * @pins: an array of discrete physical pins used in this group, taken
 *	from the driver-local pin enumeration space
 * @num_pins: the number of pins in this group array, i.e. the number of
 *	elements in .pins so we can iterate over that array
 * @func: selected function value
 */
struct drime4_pin_group {
	const char *name;
	const unsigned int *pins;
	const unsigned num_pins;
	const unsigned func;
};

static const unsigned mlcd_pins[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	31, 32, 33, 34, 35, 36, 37, 38};

static const unsigned sd0_pins[] = {77, 78, 79, 80, 81, 82, 83, 84};
static const unsigned sd0_pwr_pins[] = {85, 86};
/*
 * org
static const unsigned slcd_pins[] = {43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
	53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
	70, 71, 72};
*/

static const unsigned slcd_pins[] = {43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
	53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64};
static const unsigned slcd_pins_clk[] = {45};


static const unsigned i2c0_pins[] = {146, 147};
static const unsigned i2c1_pins[] = {148, 149};
static const unsigned i2c2_pins[] = {87, 88};
static const unsigned i2c3_pins[] = {89, 90};
static const unsigned i2c4_pins[] = {44, 45};
static const unsigned i2c5_pins[] = {57, 58};
static const unsigned i2c6_pins[] = {273, 274};
static const unsigned i2c7_pins[] = {275, 276};
static const unsigned bt656i0_pins[] = {46, 47, 48, 49, 50, 51, 52, 53,
	54, 55, 56};
static const unsigned bt656i1_pins[] = {219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229};
static const unsigned i2s1_pins[] = {57, 58, 59, 60, 61};
static const unsigned i2s2_pins[] = {62, 63, 64, 65, 66};
static const unsigned i2s3_pins[] = {114, 115, 116, 117, 118};
static const unsigned i2s4_pins[] = {119, 120, 121, 130, 131};
static const unsigned ccd0_pins[] = {234, 235, 236, 237, 238, 239, 243,
	244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
	256, 257, 258, 259};
static const unsigned ccd1_pins[] = {59, 60, 61, 62, 63, 64, 65, 66, 67,
	68, 69, 70, 71, 72};
static const unsigned ata_gr1_pins[] = {91, 92, 93, 94};
static const unsigned ata_gr2_pins[] = {243, 244, 245, 246, 247, 248, 249,
	250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263,
	264, 265, 266};
#ifdef CONFIG_DRIME4_EMMC
static const unsigned sd1_pins[] = {240, 241, 269, 270, 271, 272, 273, 274,
	275, 276, 277, 278, 279};
#else
static const unsigned sd1_pins[] = {269, 270, 271, 272,	277, 278 };
#endif
static const unsigned efs0_pins[] = {75, 76};
static const unsigned efs1_pins[] = {277, 278};
static const unsigned ptc0_gr1_pins[] = {73, 74};
static const unsigned ptc0_gr2_pins[] = {134, 135};
static const unsigned ptc1_gr1_pins[] = {75, 76};
static const unsigned ptc1_gr2_pins[] = {136, 137};
static const unsigned spi0_pins[] = {4, 5, 6, 7};
static const unsigned spi1_pins[] = {39, 40, 41, 42};
static const unsigned spi2_pins[] = {91, 92, 93, 94};
static const unsigned spi3_pins[] = {134, 135, 136, 137};
static const unsigned spi4_pins[] = {138, 139, 140, 141};
static const unsigned spi5_pins[] = {142, 143, 144, 145};
static const unsigned spi6_pins[] = {67, 68, 69, 70};
static const unsigned spi7_pins[] = {269, 270, 271, 272};
static const unsigned spi8_pins[] = {114, 115, 116, 117};
static const unsigned spi9_pins[] = {118, 119, 120, 121};
static const unsigned pwm0_pins[] = {122};
static const unsigned pwm1_pins[] = {123};
static const unsigned pwm2_pins[] = {124};
static const unsigned pwm3_pins[] = {125};
static const unsigned pwm4_pins[] = {126};
static const unsigned pwm5_pins[] = {127};
static const unsigned pwm6_pins[] = {128};
static const unsigned pwm7_pins[] = {129};
static const unsigned pwm8_pins[] = {130};
static const unsigned pwm9_pins[] = {131};
static const unsigned pwm10_pins[] = {132};
static const unsigned pwm11_pins[] = {133};
static const unsigned pwm12_pins[] = {71};
static const unsigned pwm13_pins[] = {72};
static const unsigned pwm14_pins[] = {267};
static const unsigned pwm15_pins[] = {268};
static const unsigned pwm16_pins[] = {148};
static const unsigned pwm17_pins[] = {149};
static const unsigned uart1_pins[] = {109, 110, 111, 112};
static const unsigned slvds_pins[] = {186, 187, 188, 189, 190, 191, 192,
	193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205,
	206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218,
	219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231,
	232, 233};
static const unsigned srp1_pins[] = {234, 235, 236, 237, 238, 239};
static const unsigned srp2_pins[] = {109, 110, 111, 112, 132, 133};

#define DEF_PIN_GROUP(_name, _pins, _func)	\
	{			\
		.name = _name,	\
		.pins = _pins,	\
		.num_pins = ARRAY_SIZE(_pins),	\
		.func = _func,	\
	}

static const struct drime4_pin_group drime4_pin_groups[] = {
	DEF_PIN_GROUP("sd0grp", sd0_pins, FUNC1),
	DEF_PIN_GROUP("sd0_pwr_grp", sd0_pwr_pins, FUNC1),
	DEF_PIN_GROUP("slcdgrp", slcd_pins, FUNC1),
	DEF_PIN_GROUP("slcdgrp_clk", slcd_pins_clk, FUNC1),
	DEF_PIN_GROUP("i2c0grp", i2c0_pins, FUNC1),
	DEF_PIN_GROUP("i2c1grp", i2c1_pins, FUNC1),
	DEF_PIN_GROUP("i2c2grp", i2c2_pins, FUNC1),
	DEF_PIN_GROUP("i2c3grp", i2c3_pins, FUNC1),
	DEF_PIN_GROUP("i2c4grp", i2c4_pins, FUNC2),
	DEF_PIN_GROUP("i2c5grp", i2c5_pins, FUNC3),
	DEF_PIN_GROUP("i2c6grp", i2c6_pins, FUNC2),
	DEF_PIN_GROUP("i2c7grp", i2c7_pins, FUNC2),
	DEF_PIN_GROUP("bt656i0grp", bt656i0_pins, FUNC2),
	DEF_PIN_GROUP("bt656I1grp", bt656i1_pins, FUNC2),
	DEF_PIN_GROUP("i2s1grp", i2s1_pins, FUNC2),
	DEF_PIN_GROUP("i2s2grp", i2s2_pins, FUNC2),
	DEF_PIN_GROUP("i2s3grp", i2s3_pins, FUNC2),
	DEF_PIN_GROUP("i2s4grp", i2s4_pins, FUNC2),
	DEF_PIN_GROUP("ccd0grp", ccd0_pins, FUNC2),
	DEF_PIN_GROUP("ccd1grp", ccd1_pins, FUNC3),
	DEF_PIN_GROUP("atagrp1", ata_gr1_pins, FUNC2),
	DEF_PIN_GROUP("atagrp2", ata_gr2_pins, FUNC1),
	DEF_PIN_GROUP("sd1grp", sd1_pins, FUNC1),
	DEF_PIN_GROUP("efs0grp", efs0_pins, FUNC2),
	DEF_PIN_GROUP("efs1grp", efs1_pins, FUNC2),
	DEF_PIN_GROUP("ptc0grp1", ptc0_gr1_pins, FUNC1),
	DEF_PIN_GROUP("ptc0grp2", ptc0_gr2_pins, FUNC2),
	DEF_PIN_GROUP("ptc1grp1", ptc1_gr1_pins, FUNC1),
	DEF_PIN_GROUP("ptc1grp2", ptc1_gr2_pins, FUNC2),
	DEF_PIN_GROUP("spi0grp", spi0_pins, FUNC1),
	DEF_PIN_GROUP("spi1grp", spi1_pins, FUNC1),
	DEF_PIN_GROUP("spi2grp", spi2_pins, FUNC1),
	DEF_PIN_GROUP("spi3grp", spi3_pins, FUNC1),
	DEF_PIN_GROUP("spi4grp", spi4_pins, FUNC1),
	DEF_PIN_GROUP("spi5grp", spi5_pins, FUNC1),
	DEF_PIN_GROUP("spi6grp", spi6_pins, FUNC2),
	DEF_PIN_GROUP("spi7grp", spi7_pins, FUNC2),
	DEF_PIN_GROUP("spi8grp", spi8_pins, FUNC3),
	DEF_PIN_GROUP("spi9grp", spi9_pins, FUNC3),
	DEF_PIN_GROUP("pwm0grp", pwm0_pins, FUNC1),
	DEF_PIN_GROUP("pwm1grp", pwm1_pins, FUNC1),
	DEF_PIN_GROUP("pwm2grp", pwm2_pins, FUNC1),
	DEF_PIN_GROUP("pwm3grp", pwm3_pins, FUNC1),
	DEF_PIN_GROUP("pwm4grp", pwm4_pins, FUNC1),
	DEF_PIN_GROUP("pwm5grp", pwm5_pins, FUNC1),
	DEF_PIN_GROUP("pwm6grp", pwm6_pins, FUNC1),
	DEF_PIN_GROUP("pwm7grp", pwm7_pins, FUNC1),
	DEF_PIN_GROUP("pwm8grp", pwm8_pins, FUNC1),
	DEF_PIN_GROUP("pwm9grp", pwm9_pins, FUNC1),
	DEF_PIN_GROUP("pwm10grp", pwm10_pins, FUNC1),
	DEF_PIN_GROUP("pwm11grp", pwm11_pins, FUNC1),
	DEF_PIN_GROUP("pwm12grp", pwm12_pins, FUNC2),
	DEF_PIN_GROUP("pwm13grp", pwm13_pins, FUNC2),
	DEF_PIN_GROUP("pwm14grp", pwm14_pins, FUNC2),
	DEF_PIN_GROUP("pwm15grp", pwm15_pins, FUNC2),
	DEF_PIN_GROUP("pwm16grp", pwm16_pins, FUNC2),
	DEF_PIN_GROUP("pwm17grp", pwm17_pins, FUNC2),
	DEF_PIN_GROUP("uart1grp", uart1_pins, FUNC1),
	DEF_PIN_GROUP("slvdsgrp", slvds_pins, FUNC1),
	DEF_PIN_GROUP("srp1grp", srp1_pins, FUNC1),
	DEF_PIN_GROUP("srp2grp", srp2_pins, FUNC2),
	DEF_PIN_GROUP("mlcdcon", mlcd_pins, FUNC1),
};

static int drime4_get_groups_count(struct pinctrl_dev *pctldev)
{
	return ARRAY_SIZE(drime4_pin_groups);
}

static const char *drime4_get_group_name(struct pinctrl_dev *pctldev,
				       unsigned selector)
{
	if (selector >= ARRAY_SIZE(drime4_pin_groups))
		return NULL;
	return drime4_pin_groups[selector].name;
}

static int drime4_get_group_pins(struct pinctrl_dev *pctldev, unsigned selector,
			       const unsigned **pins,
			       unsigned *num_pins)
{
	if (selector >= ARRAY_SIZE(drime4_pin_groups))
		return -EINVAL;
	*pins = (unsigned *) drime4_pin_groups[selector].pins;
	*num_pins = drime4_pin_groups[selector].num_pins;
	return 0;
}

static void drime4_pin_dbg_show(struct pinctrl_dev *pctldev, struct seq_file *s,
		   unsigned offset)
{
	seq_printf(s, "drime4-pinmux");
}

static struct pinctrl_ops drime4_pctrl_ops = {
	.get_groups_count = drime4_get_groups_count,
	.get_group_name = drime4_get_group_name,
	.get_group_pins = drime4_get_group_pins,
	.pin_dbg_show = drime4_pin_dbg_show,
};

struct drime4_pmx_func {
	const char *name;
	const char * const *groups;
	const unsigned num_groups;
};

static const char * const sd0grps[] = { "sd0grp", "sd0_pwr_grp" };
static const char * const slcdgrps[] = { "slcdgrp" };
static const char * const slcdgrps_clk[] = { "slcdgrp_clk" };
static const char * const i2c0grps[] = { "i2c0grp" };
static const char * const i2c1grps[] = { "i2c1grp" };
static const char * const i2c2grps[] = { "i2c2grp" };
static const char * const i2c3grps[] = { "i2c3grp" };
static const char * const i2c4grps[] = { "i2c4grp" };
static const char * const i2c5grps[] = { "i2c5grp" };
static const char * const i2c6grps[] = { "i2c6grp" };
static const char * const i2c7grps[] = { "i2c7grp" };
static const char * const i2s1grps[] = { "i2s1grp" };
static const char * const i2s2grps[] = { "i2s2grp" };
static const char * const i2s3grps[] = { "i2s3grp" };
static const char * const i2s4grps[] = { "i2s4grp" };
static const char * const bt656i0grps[] = { "bt656i0grp" };
static const char * const bt656i1grps[] = { "bt656i1grp" };
static const char * const ccd0grps[] = { "ccd0grp" };
static const char * const ccd1grps[] = { "ccd1grp" };
static const char * const atagrps[] = { "atagrp1", "atagrp2" };
static const char * const sd1grps[] = { "sd1grp" };
static const char * const efs0grps[] = { "efs0grp" };
static const char * const efs1grps[] = { "efs1grp" };
static const char * const ptc0grps[] = { "ptc0grp1", "ptc0grp2" };
static const char * const ptc1grps[] = { "ptc1grp1", "ptc1grp2" };
static const char * const spi0grps[] = { "spi0grp" };
static const char * const spi1grps[] = { "spi1grp" };
static const char * const spi2grps[] = { "spi2grp" };
static const char * const spi3grps[] = { "spi3grp" };
static const char * const spi4grps[] = { "spi4grp" };
static const char * const spi5grps[] = { "spi5grp" };
static const char * const spi6grps[] = { "spi6grp" };
static const char * const spi7grps[] = { "spi7grp" };
static const char * const spi8grps[] = { "spi8grp" };
static const char * const spi9grps[] = { "spi9grp" };
static const char * const pwm0grps[] = { "pwm0grp" };
static const char * const pwm1grps[] = { "pwm1grp" };
static const char * const pwm2grps[] = { "pwm2grp" };
static const char * const pwm3grps[] = { "pwm3grp" };
static const char * const pwm4grps[] = { "pwm4grp" };
static const char * const pwm5grps[] = { "pwm5grp" };
static const char * const pwm6grps[] = { "pwm6grp" };
static const char * const pwm7grps[] = { "pwm7grp" };
static const char * const pwm8grps[] = { "pwm8grp" };
static const char * const pwm9grps[] = { "pwm9grp" };
static const char * const pwm10grps[] = { "pwm10grp" };
static const char * const pwm11grps[] = { "pwm11grp" };
static const char * const pwm12grps[] = { "pwm12grp" };
static const char * const pwm13grps[] = { "pwm13grp" };
static const char * const pwm14grps[] = { "pwm14grp" };
static const char * const pwm15grps[] = { "pwm15grp" };
static const char * const pwm16grps[] = { "pwm16grp" };
static const char * const pwm17grps[] = { "pwm17grp" };
static const char * const uart1grps[] = { "uart1grp" };
static const char * const slvdsgrps[] = { "slvdsgrp" };
static const char * const srp1grps[] = { "srp1grp" };
static const char * const srp2grps[] = { "srp2grp" };
static const char * const mlcdgrps[] = { "mlcdcon" };

#define DEF_PMX_FUNC(_name, _group)			\
	{	.name = _name,				\
		.groups = _group,			\
		.num_groups = ARRAY_SIZE(_group),	\
	}						\

static const struct drime4_pmx_func drime4_pmx_functions[] = {
	DEF_PMX_FUNC("sd0", sd0grps),
	DEF_PMX_FUNC("slcd", slcdgrps),
	DEF_PMX_FUNC("i2c0", i2c0grps),
	DEF_PMX_FUNC("i2c1", i2c1grps),
	DEF_PMX_FUNC("i2c2", i2c2grps),
	DEF_PMX_FUNC("i2c3", i2c3grps),
	DEF_PMX_FUNC("i2c4", i2c4grps),
	DEF_PMX_FUNC("i2c5", i2c5grps),
	DEF_PMX_FUNC("i2c6", i2c6grps),
	DEF_PMX_FUNC("i2c7", i2c7grps),
	DEF_PMX_FUNC("i2s1", i2s1grps),
	DEF_PMX_FUNC("i2s2", i2s2grps),
	DEF_PMX_FUNC("i2s3", i2s3grps),
	DEF_PMX_FUNC("i2s4", i2s4grps),
	DEF_PMX_FUNC("bt656i0", bt656i0grps),
	DEF_PMX_FUNC("bt656i1", bt656i1grps),
	DEF_PMX_FUNC("ccd0", ccd0grps),
	DEF_PMX_FUNC("ccd1", ccd1grps),
	DEF_PMX_FUNC("ata", atagrps),
	DEF_PMX_FUNC("sd1", sd1grps),
	DEF_PMX_FUNC("ptc0", ptc0grps),
	DEF_PMX_FUNC("ptc1", ptc1grps),
	DEF_PMX_FUNC("spi0", spi0grps),
	DEF_PMX_FUNC("spi1", spi1grps),
	DEF_PMX_FUNC("spi2", spi2grps),
	DEF_PMX_FUNC("spi3", spi3grps),
	DEF_PMX_FUNC("spi4", spi4grps),
	DEF_PMX_FUNC("spi5", spi5grps),
	DEF_PMX_FUNC("spi6", spi6grps),
	DEF_PMX_FUNC("spi7", spi7grps),
	DEF_PMX_FUNC("spi8", spi8grps),
	DEF_PMX_FUNC("spi9", spi9grps),
	DEF_PMX_FUNC("pwm0", pwm0grps),
	DEF_PMX_FUNC("pwm1", pwm1grps),
	DEF_PMX_FUNC("pwm2", pwm2grps),
	DEF_PMX_FUNC("pwm3", pwm3grps),
	DEF_PMX_FUNC("pwm4", pwm4grps),
	DEF_PMX_FUNC("pwm5", pwm5grps),
	DEF_PMX_FUNC("pwm6", pwm6grps),
	DEF_PMX_FUNC("pwm7", pwm7grps),
	DEF_PMX_FUNC("pwm8", pwm8grps),
	DEF_PMX_FUNC("pwm9", pwm9grps),
	DEF_PMX_FUNC("pwm10", pwm10grps),
	DEF_PMX_FUNC("pwm11", pwm11grps),
	DEF_PMX_FUNC("pwm12", pwm12grps),
	DEF_PMX_FUNC("pwm13", pwm13grps),
	DEF_PMX_FUNC("pwm14", pwm14grps),
	DEF_PMX_FUNC("pwm15", pwm15grps),
	DEF_PMX_FUNC("pwm16", pwm16grps),
	DEF_PMX_FUNC("pwm17", pwm17grps),
	DEF_PMX_FUNC("uart1", uart1grps),
	DEF_PMX_FUNC("slvds", slvdsgrps),
	DEF_PMX_FUNC("srp1", srp1grps),
	DEF_PMX_FUNC("srp2", srp2grps),
};

static inline void pmx_writel(u32 value, struct drime4_pmx *pmx, unsigned pin)
{
	__raw_writel(value, pmx->virtbase + (pin - 1) * 4);
}

static inline u32 pmx_readl(struct drime4_pmx *pmx, unsigned pin)
{
	return __raw_readl(pmx->virtbase + (pin - 1) * 4);
}

static void drime4_pmx_endisable(struct drime4_pmx *pmx, unsigned selector,
	unsigned group, bool enable)
{
	u32 regval;
	const struct drime4_pin_group *pgrp =
		&drime4_pin_groups[group];
	const unsigned *pins = NULL;
	unsigned num_pins = 0;
	int i;

	drime4_get_group_pins(pmx->pctl, group, &pins, &num_pins);

	for (i = 0; i < num_pins; i++) {
		regval = pmx_readl(pmx, pins[i]);
		regval &= ~FUNC_MASK;
		regval |= DRIME4_PMX_CON_FUNC_WRT | pgrp->func;
		pmx_writel(regval, pmx, pins[i]);
	}
}

static int drime4_pmx_enable(struct pinctrl_dev *pctldev, unsigned selector,
			   unsigned group)
{
	int i;
	struct drime4_pmx *pmx = pinctrl_dev_get_drvdata(pctldev);
	unsigned num_groups = drime4_pmx_functions[selector].num_groups;

	for (i = 0; i < num_groups; i++)
		drime4_pmx_endisable(pmx, selector, group + i, true);

	return 0;
}

static void drime4_pmx_disable(struct pinctrl_dev *pctldev, unsigned selector,
			   unsigned group)
{
	return;
}

static int drime4_pmx_get_functions_count(struct pinctrl_dev *pctldev)
{
	return ARRAY_SIZE(drime4_pmx_functions);
}

static const char *drime4_pmx_get_func_name(struct pinctrl_dev *pctldev,
					  unsigned selector)
{
	return drime4_pmx_functions[selector].name;
}

static int drime4_pmx_get_groups(struct pinctrl_dev *pctldev, unsigned selector,
			       const char * const **groups,
			       unsigned * const num_groups)
{
	*groups = drime4_pmx_functions[selector].groups;
	*num_groups = drime4_pmx_functions[selector].num_groups;
	return 0;
}

static int drime4_pmx_gpio_request_enable(struct pinctrl_dev *pctldev,
	struct pinctrl_gpio_range *range,
	unsigned offset)
{
	u32 regval;
	struct drime4_pmx *pmx = pinctrl_dev_get_drvdata(pctldev);

	regval = pmx_readl(pmx, offset);
	regval &= ~FUNC_MASK;
	regval |= DRIME4_PMX_CON_FUNC_WRT | FUNC4;
	pmx_writel(regval, pmx, offset);

	return 0;
}

static struct pinmux_ops drime4_pmx_ops = {
	.get_functions_count = drime4_pmx_get_functions_count,
	.get_function_name = drime4_pmx_get_func_name,
	.get_function_groups = drime4_pmx_get_groups,
	.enable = drime4_pmx_enable,
	.disable = drime4_pmx_disable,
	.gpio_request_enable = drime4_pmx_gpio_request_enable,
};

static u16 drime4_pmx_pcon_get(struct drime4_pmx *pmx,
	u32 pin, u32 mask, u32 shift)
{
	u32 regval;
	regval = pmx_readl(pmx, pin);
	return (u16)((regval & mask) >> shift);
}

static void drime4_pmx_pcon_set(struct drime4_pmx *pmx,
	u32 pin, u32 mask, u32 val)
{
	u32 regval;

	regval = pmx_readl(pmx, pin);
	regval &= ~mask;
	regval |= DRIME4_PMX_CON_PCON_WRT | val;
	pmx_writel(regval, pmx, pin);
}

static u16 drime4_pmx_pcon_get_group(struct drime4_pmx *pmx,
	unsigned group, u32 mask, u32 shift)
{
	int ret;
	const unsigned *pins = NULL;
	unsigned num_pins = 0;

	ret = drime4_get_group_pins(pmx->pctl, group, &pins, &num_pins);
	if (ret)
		return 0;

	return drime4_pmx_pcon_get(pmx, pins[0], mask, shift);
}

static void drime4_pmx_pcon_set_group(struct drime4_pmx *pmx,
	unsigned group, u32 mask, u32 val)
{
	int ret;
	const unsigned *pins = NULL;
	unsigned num_pins = 0;
	int i;

	ret = drime4_get_group_pins(pmx->pctl, group, &pins, &num_pins);
	if (ret)
		return;

	for (i = 0; i < num_pins; i++)
		drime4_pmx_pcon_set(pmx, pins[i], mask, val);
}

static int drime4_pin_config_getmask(enum pin_config_param param,
				unsigned long *mask,
				unsigned long *shift)
{
	switch (param) {
	case PIN_CONFIG_INPUT:
		*mask = DRIME4_PMX_CON_INPUT_MASK;
		*shift = DRIME4_PMX_CON_INPUT_SHIFT;
		break;
	case PIN_CONFIG_INPUT_MODE:
		*mask = DRIME4_PMX_CON_INPUT_MODE_MASK;
		*shift = DRIME4_PMX_CON_INPUT_MODE_SHIFT;
		break;
	case PIN_CONFIG_SLEW_RATE:
		*mask = DRIME4_PMX_CON_SLEW_RATE_MASK;
		*shift = DRIME4_PMX_CON_SLEW_RATE_SHIFT;
		break;
	case PIN_CONFIG_PULLUP_DOWN:
		*mask = DRIME4_PMX_CON_PULLUP_DOWN_MASK;
		*shift = DRIME4_PMX_CON_PULLUP_DOWN_SHIFT;
		break;
	case PIN_CONFIG_PULLUP_DOWN_SEL:
		*mask = DRIME4_PMX_CON_PULLUP_DOWN_SEL_MASK;
		*shift = DRIME4_PMX_CON_PULLUP_DOWN_SEL_SHIFT;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		*mask = DRIME4_PMX_CON_DRIVE_STRENGTH_MASK;
		*shift = DRIME4_PMX_CON_DRIVE_STRENGTH_SHIFT;
		break;
	case PIN_CONFIG_MODE_CTRL_I2S_GPIO:
		*mask = DRIME4_PMX_MODE_CTRL_I2S2_GPIO_MASK;
		*shift = DRIME4_PMX_MODE_CTRL_I2S2_GPIO_SHIFT;
		break;
	case PIN_CONFIG_MODE_CTRL_I2S_HDMI:
		*mask = DRIME4_PMX_MODE_CTRL_I2S2_HDMI_MASK;
		*shift = DRIME4_PMX_MODE_CTRL_I2S2_HDMI_SHIFT;
		break;
	case PIN_CONFIG_MODE_CTRL_SLCD_RESET:
		*mask = DRIME4_PMX_MODE_CTRL_SLCD_RESET_MASK;
		*shift = DRIME4_PMX_MODE_CTRL_SLCD_RESET_SHIFT;
		break;
	case PIN_CONFIG_MODE_CTRL_MLCD_RESET:
		*mask = DRIME4_PMX_MODE_CTRL_MLCD_RESET_MASK;
		*shift = DRIME4_PMX_MODE_CTRL_MLCD_RESET_SHIFT;
		break;
	case PIN_CONFIG_MODE_CTRL_CODEC_SRP_SEL:
		*mask = DRIME4_PMX_MODE_CTRL_CODEC_SRP_SEL_MASK;
		*shift = DRIME4_PMX_MODE_CTRL_CODEC_SRP_SEL_SHIFT;
		break;
	case PIN_CONFIG_VSEL18_INIT_VAL:
		*mask = DRIME4_PMX_VSEL18_INIT_VAL_MASK;
		*shift = DRIME4_PMX_VSEL18_INIT_VAL_SHIFT;
		break;
	case PIN_CONFIG_VSEL18_COUNT:
		*mask = DRIME4_PMX_VSEL18_COUNT_MASK;
		*shift = DRIME4_PMX_VSEL18_COUNT_SHIFT;
		break;
	case PIN_CONFIG_VSEL18_CTRL_SD1_CLK_INV:
		*mask = DRIME4_PMX_VSEL18_CTRL_SD1_CLK_INV_MASK;
		*shift = DRIME4_PMX_VSEL18_CTRL_SD1_CLK_INV_SHIFT;
		break;
	case PIN_CONFIG_VSEL18_CTRL_SD0_SEL:
		*mask = DRIME4_PMX_VSEL18_CTRL_SD0_SEL_MASK;
		*shift = DRIME4_PMX_VSEL18_CTRL_SD0_SEL_SHIFT;
		break;
	case PIN_CONFIG_VSEL18_CTRL_SD0_CPU:
		*mask = DRIME4_PMX_VSEL18_CTRL_SD0_CPU_MASK;
		*shift = DRIME4_PMX_VSEL18_CTRL_SD0_CPU_SHIFT;
		break;
	case PIN_CONFIG_VSEL18_CTRL_SD1:
		*mask = DRIME4_PMX_VSEL18_CTRL_SD1_MASK;
		*shift = DRIME4_PMX_VSEL18_CTRL_SD1_SHIFT;
		break;
	case PIN_CONFIG_VSEL18_CTRL_SD0:
		*mask = DRIME4_PMX_VSEL18_CTRL_SD0_MASK;
		*shift = DRIME4_PMX_VSEL18_CTRL_SD0_SHIFT;
		break;
	case PIN_CONFIG_VSEL18_CTRL_ATA:
		*mask = DRIME4_PMX_VSEL18_CTRL_ATA_MASK;
		*shift = DRIME4_PMX_VSEL18_CTRL_ATA_SHIFT;
		break;
	case PIN_CONFIG_VSEL18_CTRL_SLCD:
		*mask = DRIME4_PMX_VSEL18_CTRL_SLCD_MASK;
		*shift = DRIME4_PMX_VSEL18_CTRL_SLCD_SHIFT;
		break;
	case PIN_CONFIG_VSEL18_CTRL_MLCD:
		*mask = DRIME4_PMX_VSEL18_CTRL_MLCD_MASK;
		*shift = DRIME4_PMX_VSEL18_CTRL_MLCD_SHIFT;
		break;
	case PIN_CONFIG_SYS_DET_USB:
		*mask = DRIME4_PMX_SYS_DET_USB_MASK;
		*shift = DRIME4_PMX_SYS_DET_USB_SHIFT;
		break;
	case PIN_CONFIG_SYS_DET_SD1:
		*mask = DRIME4_PMX_SYS_DET_SD1_MASK;
		*shift = DRIME4_PMX_SYS_DET_SD1_SHIFT;
		break;
	case PIN_CONFIG_SYS_DET_SD0:
		*mask = DRIME4_PMX_SYS_DET_SD0_MASK;
		*shift = DRIME4_PMX_SYS_DET_SD0_SHIFT;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int drime4_pin_config_get(struct pinctrl_dev *pctldev,
			       unsigned pin,
			       unsigned long *config)
{
	int ret;
	unsigned long mask, shift;
	u16 args;
	struct drime4_pmx *pmx = pinctrl_dev_get_drvdata(pctldev);
	enum pin_config_param param = to_config_param(*config);

	ret = drime4_pin_config_getmask(param, &mask, &shift);
	if (ret)
		return ret;

	args = drime4_pmx_pcon_get(pmx, pin, mask, shift);
	*config = to_config_packed(param, args);

	return 0;
}

static int drime4_pin_config_set(struct pinctrl_dev *pctldev,
			       unsigned pin,
			       unsigned long config)
{
	int ret;
	unsigned long mask, shift;
	struct drime4_pmx *pmx = pinctrl_dev_get_drvdata(pctldev);
	enum pin_config_param param = to_config_param(config);
	u16 args = to_config_argument(config);

	ret = drime4_pin_config_getmask(param, &mask, &shift);
	if (ret)
		return ret;

	drime4_pmx_pcon_set(pmx, pin, mask, args << shift);

	return 0;
}

static int drime4_pin_config_group_get(struct pinctrl_dev *pctldev,
				     unsigned selector,
				     unsigned long *config)
{
	int ret;
	unsigned long mask, shift;
	u16 args;
	struct drime4_pmx *pmx = pinctrl_dev_get_drvdata(pctldev);
	enum pin_config_param param = to_config_param(*config);

	ret = drime4_pin_config_getmask(param, &mask, &shift);
	if (ret)
		return ret;

	args = drime4_pmx_pcon_get_group(pmx, selector, mask, shift);
	*config = to_config_packed(param, args);

	return 0;
}

static int drime4_pin_config_group_set(struct pinctrl_dev *pctldev,
				     unsigned selector,
				     unsigned long config)
{
	int ret;
	unsigned long mask, shift;
	struct drime4_pmx *pmx = pinctrl_dev_get_drvdata(pctldev);
	enum pin_config_param param = to_config_param(config);
	u16 args = to_config_argument(config);

	ret = drime4_pin_config_getmask(param, &mask, &shift);
	if (ret)
		return ret;

	drime4_pmx_pcon_set_group(pmx, selector, mask, args << shift);

	return 0;
}

static void drime4_pin_config_dbg_show(struct pinctrl_dev *pctldev,
				     struct seq_file *s,
				     unsigned offset)
{
}

static void drime4_pin_config_group_dbg_show(struct pinctrl_dev *pctldev,
					   struct seq_file *s,
					   unsigned selector)
{
}

static struct pinconf_ops drime4_pcfg_ops = {
	.pin_config_get		= &drime4_pin_config_get,
	.pin_config_set		= &drime4_pin_config_set,
	.pin_config_group_get	= &drime4_pin_config_group_get,
	.pin_config_group_set	= &drime4_pin_config_group_set,
	.pin_config_dbg_show	= &drime4_pin_config_dbg_show,
	.pin_config_group_dbg_show	= &drime4_pin_config_group_dbg_show,
};

static struct pinctrl_gpio_range drime4_gpio_range0 = {
	.name = "DRIME4*",
	.id = 0,
	.base = 0,
	.pin_base = 1,
	.npins = 76,
};

static struct pinctrl_gpio_range drime4_gpio_range1 = {
	.name = "DRIME4*",
	.id = 1,
	.base = 76,
	.pin_base = 87,
	.npins = 8,
};

static struct pinctrl_gpio_range drime4_gpio_range2 = {
	.name = "DRIME4*",
	.id = 2,
	.base = 84,
	.pin_base = 99,
	.npins = 1,
};

static struct pinctrl_gpio_range drime4_gpio_range3 = {
	.name = "DRIME4*",
	.id = 3,
	.base = 85,
	.pin_base = 106,
	.npins = 45,
};

static struct pinctrl_gpio_range drime4_gpio_range4 = {
	.name = "DRIME4*",
	.id = 4,
	.base = 130,
	.pin_base = 186,
	.npins = 94,
};

static struct pinctrl_desc drime4_pmx_desc = {
	.name = "pinmux-drime4",
	.pins = drime4_pads,
	.npins = ARRAY_SIZE(drime4_pads),
	.pctlops = &drime4_pctrl_ops,
	.pmxops = &drime4_pmx_ops,
	.confops = &drime4_pcfg_ops,
	.owner = THIS_MODULE,
};

static int __init drime4_pmx_probe(struct platform_device *pdev)
{
	struct drime4_pmx *pmx;
	struct resource *res;
	int ret;
	/* Create state holders etc for this driver */
	pmx = devm_kzalloc(&pdev->dev, sizeof(struct drime4_pmx), GFP_KERNEL);
	if (!pmx)
		return -ENOMEM;

	pmx->dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENOENT;
		goto err_free;
	}

	if (devm_request_mem_region(&pdev->dev, res->start,
			resource_size(res), pdev->name) == NULL) {
		ret = -EBUSY;
		goto err_free;
	}

	pmx->virtbase = devm_ioremap(&pdev->dev, res->start,
				resource_size(res));
	if (!pmx->virtbase) {
		ret = -ENOMEM;
		goto err_free;
	}

	pmx->pctl = pinctrl_register(&drime4_pmx_desc, &pdev->dev, pmx);
	if (IS_ERR(pmx->pctl)) {
		dev_err(&pdev->dev, "could not register U300 pinmux driver\n");
		ret = PTR_ERR(pmx->pctl);
		goto err_free;
	}

	pinctrl_add_gpio_range(pmx->pctl, &drime4_gpio_range0);
	pinctrl_add_gpio_range(pmx->pctl, &drime4_gpio_range1);
	pinctrl_add_gpio_range(pmx->pctl, &drime4_gpio_range2);
	pinctrl_add_gpio_range(pmx->pctl, &drime4_gpio_range3);
	pinctrl_add_gpio_range(pmx->pctl, &drime4_gpio_range4);

	platform_set_drvdata(pdev, pmx);

	dev_info(&pdev->dev, "initialized DRIME4 pinmux driver\n");

	return 0;

err_free:
	devm_kfree(&pdev->dev, pmx);
	return ret;
}

static int __exit drime4_pmx_remove(struct platform_device *pdev)
{
	struct drime4_pmx *pmx = platform_get_drvdata(pdev);

	if (pmx) {
		pinctrl_unregister(pmx->pctl);
		platform_set_drvdata(pdev, NULL);
	}

	return 0;
}
static struct platform_driver drime4_pmx_driver = {
	.driver = {
		.name = "drime4-pinmux",
		.owner = THIS_MODULE,
	},
	.probe = drime4_pmx_probe,
	.remove = __exit_p(drime4_pmx_remove),
};

static int __init drime4_pmx_init(void)
{
	return platform_driver_register(&drime4_pmx_driver);
}
arch_initcall(drime4_pmx_init);

static void __exit drime4_pmx_exit(void)
{
	platform_driver_unregister(&drime4_pmx_driver);
}
module_exit(drime4_pmx_exit);

MODULE_AUTHOR("Chanho Park <chanho61.park@samsung.com>");
MODULE_LICENSE("GPL v2");
