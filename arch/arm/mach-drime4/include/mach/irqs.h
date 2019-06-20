/* linux/arch/arm/mach-drime4/include/mach/irqs.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - IRQ definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_IRQS_H
#define __MACH_DRIME4_IRQS_H

/* INTC0 - interrupt controller 0 */
#define IRQ_INTC0_START		0
#define IRQ_WDT			0
#define IRQ_TIMER0		1
#define IRQ_TIMER1		2
#define IRQ_TIMER2		3
#define IRQ_PP_CORE		4
#define IRQ_PP_SSIF		5
#define IRQ_PP_3A		6
#define IRQ_IPC_IPCM		7
#define IRQ_IPC_IPCS		8
#define IRQ_IPC_MD		9
#define IRQ_IPC_LDC		10
#define IRQ_EP_CORE		11
#define IRQ_EP_DMA		12
#define IRQ_BAYERE		13
#define IRQ_GPIOG		14
#define IRQ_GPIOH		15
#define IRQ_GPU_C		16
#define IRQ_DP_CORE		17
#define IRQ_DP_DMA		18
#define IRQ_CODEC_SRP		19
#define IRQ_CODEC_MFC		20
#define IRQ_JPEG		21
#define IRQ_PDMA		22
#define IRQ_PDMA_ABT		23
#define IRQ_MDMA		24
#define IRQ_MDMA_ABT		25
#define IRQ_GPIOA		26
#define IRQ_GPIOB		27
#define IRQ_GPIOC		28
#define IRQ_GPIOD		29
#define IRQ_GPIOE		30
#define IRQ_GPIOF		31
#define IRQ_INTC0_END		31

#define IRQ_INTC1_START		32
#define IRQ_TIMER3		32
#define IRQ_TIMER4		33
#define IRQ_TIMER5		34
#define IRQ_MIPI_RX		35
#define IRQ_MIPI_TX		36
#define IRQ_ADC			37
#define IRQ_USB			38
#define IRQ_SD0			39
#define IRQ_SD1			40
#define IRQ_HSIC		41
#define IRQ_ATA			42
#define IRQ_HDMI		43
#define IRQ_PTC0		44
#define IRQ_PTC1		45
#define IRQ_RTC_PRI		46
#define IRQ_RTC_ALM		47
#define IRQ_UART0		48
#define IRQ_UART1		49
#define IRQ_PWM0		50
#define IRQ_PWM1		51
#define IRQ_PWM2		52
#define IRQ_PWM3		53
#define IRQ_PWM4		54
#define IRQ_PWM5		55
#define IRQ_PWM6		56
#define IRQ_PWM7		57
#define IRQ_PWM8		58
#define IRQ_PWM9		59
#define IRQ_PWM10		60
#define IRQ_PWM11		61
#define	IRQ_NFC			62
#define	IRQ_NFC_DMA		63
#define IRQ_INTC2_END		63

#define IRQ_INTC2_START		64
#define IRQ_SPI0		64
#define IRQ_SPI1		65
#define IRQ_SPI2		66
#define IRQ_SPI3		67
#define IRQ_SPI4		68
#define IRQ_SPI5		69
#define IRQ_SPI6		70
#define IRQ_SPI7		71
#define IRQ_I2C0		72
#define IRQ_I2C1		73
#define IRQ_I2C2		74
#define IRQ_I2C3		75
#define IRQ_I2C4		76
#define IRQ_I2C5		77
#define IRQ_I2C6		78
#define IRQ_I2S0		79
#define IRQ_I2S1		80
#define IRQ_I2S2		81
#define	IRQ_PWM12		82
#define	IRQ_PWM13		83
#define	IRQ_PWM14		84
#define	IRQ_PWM15		85
#define	IRQ_PWM16		86
#define	IRQ_PWM17		87
#define	IRQ_PCU			88
#define	IRQ_GPIOI_O		89
#define	IRQ_GPIOP_V		90
#define	IRQ_GPIOW_AB		91
#define	IRQ_CA9_L2C		92
#define	IRQ_CA9_PMU		93
#define	IRQ_CS_CTI		94
#define	IRQ_SONICS		95
#define IRQ_INTC3_END		95

#define IRQ_MAIN_NR		(IRQ_INTC3_END - IRQ_INTC0_START + 1)

#define IRQ_GPIO_BASE		(IRQ_INTC0_START + IRQ_MAIN_NR)
#define IRQ_GPIO_NR_GROUPS	(28)
#define IRQ_GPIO_GROUP_SIZE	(8)
#define IRQ_GPIO_NR		(IRQ_GPIO_NR_GROUPS * IRQ_GPIO_GROUP_SIZE)
#define DRIME4_IRQ_GPIO(x)	(IRQ_GPIO_BASE + (x))
#define DRIME4_IRQ_GPIO_GROUP(x)	DRIME4_IRQ_GPIO(x * IRQ_GPIO_GROUP_SIZE)

#define NR_IRQS			(IRQ_MAIN_NR + IRQ_GPIO_NR)
#define IRQ_UNKNOWN		-1

#define IRQ_INTC0_VALID_MASK	0xffffffff
#define IRQ_INTC1_VALID_MASK	0xffffffff
#define IRQ_INTC2_VALID_MASK	0xffffffff

#endif
