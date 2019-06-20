/* linux/arch/arm/mach-drime4/include/mach/map.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *      http://www.samsung.com/
 *
 * DRIME4 - Address Map definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_MAP_H
#define __MACH_DRIME4_MAP_H

/* DRIMeIV Address Map */
#define DRIME4_ADDR_BASE		(0xF8000000)

#ifndef __ASSEMBLY__
#define DRIME4_ADDR(x)			((void __iomem __force *) \
					DRIME4_ADDR_BASE  + (x))
#else
#define DRIME4_ADDR(x)			(DRIME4_ADDR_BASE + (x))
#endif

#define DRIME4_TIMER_CH_OFF		(0x10)
#define DRIME4_TIMER(timer)		(DRIME4_VA_TIMER +  \
					((timer) * DRIME4_TIMER_CH_OFF))

#define DRIME4_VIC_CH_OFF		(0x1000)
#define DRIME4_VA_VIC(vic)		(DRIME4_VA_SONICS_VIC_CTRL +  \
					((vic) * DRIME4_VIC_CH_OFF))

#define DRIME4_GPIO_OFFSET		(0x1000)
#define DRIME4_PA_GPIO_BASE(gpio)	(DRIME4_PA_GPIO + \
					((gpio) * DRIME4_GPIO_OFFSET))
#define DRIME4_VA_GPIO_BASE(gpio)	(DRIME4_VA_GPIO + \
					((gpio) * DRIME4_GPIO_OFFSET))

/* I2C */
#define DRIME4_I2C_OFFSET		(0x1000)
#define DRIME4_PA_I2C_BASE(x)		(DRIME4_PA_I2C + \
					((x)*DRIME4_I2C_OFFSET))

/* SPI */
#define DRIME4_SPI_CH_OFF		(0x1000)
#define DRIME4_PA_SPI(spi)		(DRIME4_PA_SPI0 + \
					(spi)*DRIME4_SPI_CH_OFF)

/* I2S */
#define DRIME4_I2S_OFFSET		(0x1000)
#define DRIME4_PA_I2S(x)		(DRIME4_PA_I2S0 + \
					((x)*DRIME4_I2S_OFFSET))

/* PP Core */
#define DRIME4_PP_CORE_COMMON_SIZE	(0x100)
#define DRIME4_PP_DMA_OFFSET		(0x3000)
#define DRIME4_PP_DMA_SIZE		(0xC00)
#define DRIME4_PP_CORE_CTRL_OFFSET	(0x4000)
#define DRIME4_PP_CORE_CTRL_SIZE	(0x800)

/* MIPI */
#define	DRIME4_MIPI_CON_SIZE   (0x20)
#define	DRIME4_MIPI_CSIM_SIZE   (0x100)
#define	DRIME4_MIPI_CSIS_SIZE   (0x100)

/* PP Sensor Interface */
#define DRIME4_PP_SSIF_SIZE		(0x800)

/* SubLVDS */
#define DRIME4_SLVDS_SIZE		(0x20)

#define	DRIME4_PA_GPIO2		0x30060000
#define	DRIME4_VA_GPIO2		DRIME4_ADDR(0x190000)


/* PP 3A */
#define DRIME4_PP_3A_OFF		(0xB00)

/* IPCM */
#define DRIME4_IPCM_OFF			(0x2000)

/* IPCS */
#define DRIME4_IPCS_OFF			(0xffff)
/* JPEG */
#define DRIME4_JPEG_CLOCK_SIZE		(0x8)
#define DRIME4_JPEG_ACC_SIZE		(0x10)

#define DRIME4_JPEG_IP_OFFSET		(0x400)
#define DRIME4_JPEG_IP_SIZE		(0x2C8)

#define DRIME4_JPEG_IF_OFFSET		(0x800)
#define DRIME4_JPEG_IF_SIZE		(0x58)

#define DRIME4_JPEG_XR_SIZE		(0x80)

/* DP */
#define DRIME4_DP_TV_OFF		(0x100)
#define DRIME4_DP_LCD_OFF		(0x600)
#define DRIME4_DP_OFF			(0x10000)

/* HDMI */
#define DRIME4_HDMI_OFF			(0x1000)

/**< Bandwidth Control Register */
#define	DRIME4_SONICS_BUS_CTRL_SIZE   (0x4000)
#define	DRIME4_DREX_CTRL_SIZE   			(0x1000)

/* NVROM base address */
#define DRIME4_PA_FLASH			0x18000000
#define DRIME4_VA_FLASH			DRIME4_ADDR(0)
#define DRIME4_PA_ROM			0x10000000
#define DRIME4_VA_ROM			DRIME4_ADDR(0x10000)
#define DRIME4_PA_SDRAM			0xC0000000

/* Peri_AHB/AXI base address */
#define DRIME4_PA_USB			0x20000000
#define DRIME4_PA_HSIC			0x20100000
#define DRIME4_PA_SD0			0x20200000
#define DRIME4_PA_SD1			0x20300000
#define DRIME4_PA_ATA			0x20400000
#define DRIME4_PA_NFCON			0x20600000

/* Peri_APB0 base address*/
#define DRIME4_PA_PL352			0x30000000
#define DRIME4_PA_PDMA			0x30010000
#define DRIME4_PA_PWM			0x30020000
#define DRIME4_PA_PTC			0x30030000
#define DRIME4_PA_UART			0x30040000
#define DRIME4_VA_UART			DRIME4_ADDR(0x20000)
#define DRIME4_PA_GPIO			0x30050000
#define DRIME4_VA_GPIO			DRIME4_ADDR(0x30000)
#define DRIME4_PA_SPI0			0x30070000
#define DRIME4_PA_SPI1			DRIME4_PA_SPI(1)
#define DRIME4_PA_SPI2			DRIME4_PA_SPI(2)
#define DRIME4_PA_SPI3			DRIME4_PA_SPI(3)
#define DRIME4_PA_SPI4			DRIME4_PA_SPI(4)
#define DRIME4_PA_SPI5			DRIME4_PA_SPI(5)
#define DRIME4_PA_SPI6			DRIME4_PA_SPI(6)
#define DRIME4_PA_SPI7			DRIME4_PA_SPI(7)
#define DRIME4_PA_I2C			0x30080000
#define DRIME4_PA_I2S0			0x30090000
#define DRIME4_PA_I2S1			DRIME4_PA_I2S(1)
#define DRIME4_PA_I2S2			DRIME4_PA_I2S(2)
#define DRIME4_PA_ADC			0x300A0000
#define DRIME4_PA_TIMER			0x300B0000
#define DRIME4_VA_TIMER			DRIME4_ADDR(0x50000)
#define DRIME4_VA_TIMER0		DRIME4_TIMER(0)
#define DRIME4_VA_TIMER1		DRIME4_TIMER(1)
#define DRIME4_VA_TIMER2		DRIME4_TIMER(2)
#define DRIME4_VA_TIMER3		DRIME4_TIMER(3)
#define DRIME4_VA_TIMER4		DRIME4_TIMER(4)
#define DRIME4_PA_MCPU_TIMER		(DRIME4_PA_TIMER)
#define DRIME4_PA_WDT			(DRIME4_PA_TIMER+0x1000)
#define DRIME4_VA_WDT           DRIME4_ADDR(0x560000)
#define DRIME4_PA_EFS_CON		0x300C0000
#define DRIME4_PA_SD_CFG		0x300D0000
#define DRIME4_VA_SD_CFG		DRIME4_ADDR(0x55000)
#define DRIME4_PA_NFDMA			0x300E0000
#define DRIME4_PA_PLATFORM_CTRL		0x30100000
#define DRIME4_VA_PLATFORM_CTRL		DRIME4_ADDR(0x60000)
#define DRIME4_PA_RESET_CTRL		0x30110000
#define DRIME4_VA_RESET_CTRL		DRIME4_ADDR(0x70000)
#define DRIME4_PA_CLOCK_CTRL		0x30120000
#define DRIME4_VA_CLOCK_CTRL		DRIME4_ADDR(0x80000)
#define DRIME4_PA_GLOBAL_CTRL		0x30130000
#define DRIME4_VA_GLOBAL_CTRL		DRIME4_ADDR(0x90000)
#define DRIME4_PA_RTC			0x30140000
#define DRIME4_VA_RTC			DRIME4_ADDR(0x100000)
#define DRIME4_PA_PMU			0x30150000
#define DRIME4_VA_PMU			DRIME4_ADDR(0x110000)
#define DRIME4_PA_USB_CFG		0x30160000
#define DRIME4_PA_PERI_AXI		0x30170000
#define DRIME4_VA_PERI_AXI		DRIME4_ADDR(0x120000)
#define DRIME4_PA_ARM_DEBUG		0x30180000
#define DRIME4_VA_ARM_DEBUG		DRIME4_ADDR(0x130000)
#define DRIME4_PA_CPU_SYS		0x301A0000
#define DRIME4_VA_CPU_SYS		DRIME4_ADDR(0x140000)
#define DRIME4_PA_L2C_CTRL		0x301B0000
#define DRIME4_VA_L2C_CTRL		DRIME4_ADDR(0x150000)

/* A9MP */
#define DRIME4_PA_A9MP			0x40000000
#define DRIME4_VA_A9MP			DRIME4_ADDR(0x300000)

/* M/M_APB */
#define DRIME4_PA_LS_DDR_CTRL		0x50000000
#define DRIME4_VA_LS_DDR_CTRL		DRIME4_ADDR(0x160000)
#define DRIME4_PA_HS_DDR_CTRL		0x50010000
#define DRIME4_VA_HS_DDR_CTRL		DRIME4_ADDR(0x170000)
#define DRIME4_PA_PP_CORE		0x50040000
#define DRIME4_PA_PP_3A			0x50040100

/**< Bandwidth Control Register */
#define	DRIME4_PA_SONICS_BUS_CTRL	0x5E000000

#define	DRIME4_PA_LS_DDR_CTRL			0x50000000
#define	DRIME4_VA_LS_DDR_CTRL			DRIME4_ADDR(0x160000)

/* added by kh.moon for PP 3A Interrupt SW workaround at 2011.10.01 */
#define DRIME4_VA_PP			DRIME4_ADDR(0x180000)
#define DRIME4_3A_INTR_OFF		(0x104)
#define DRIME4_VA_3A_INTR		(DRIME4_VA_PP + DRIME4_3A_INTR_OFF)
#define DRIME4_VA_PP_3A_INTR		DRIME4_VA_3A_INTR

#define DRIME4_PA_PP_SSIF		0x50042400
#define DRIME4_PA_PP			0x50040000
#define DRIME4_PA_SLVDS			0x50048020
#define DRIME4_VA_PP			DRIME4_ADDR(0x180000)
#define	DRIME4_PA_MIPI_CON	    0x50048000
#define	DRIME4_PA_MIPI_CSIM	    0x50049000
#define	DRIME4_PA_MIPI_CSIS	    0x5004A000
#define DRIME4_PA_IPCS			0x50050000
#define DRIME4_PA_IPCM			0x50060000
#define DRIME4_PA_EP			0x50070000
#define DRIME4_PA_BAYER			0x50080000
#define DRIME4_PA_CODEC			0x50090000
#define DRIME4_PA_JPEG			0x500A0000
#define DRIME4_PA_DP			0x500B0000
#define DRIME4_VA_DP			DRIME4_ADDR(0x190000)
#define DRIME4_PA_MDMA			0x500D0000
#define DRIME4_PA_HDMI			0x50100000
#define DRIME4_PA_SONICS_CONF		0x5E000000
#define DRIME4_VA_SONICS_CONF		DRIME4_ADDR(0x200000)

/* M/M_Sonics */
#define DRIME4_PA_SONICS_GPU		0x70040000
#define DRIME4_PA_SONICS_VIC_CTRL	0x70090000
#define DRIME4_VA_SONICS_VIC_CTRL	DRIME4_ADDR(0x250000)
#define DRIME4_VA_VIC0			DRIME4_VA_VIC(0)
#define DRIME4_VA_VIC1			DRIME4_VA_VIC(1)
#define DRIME4_VA_VIC2			DRIME4_VA_VIC(2)

#define DRIME4_PA_SONICS_MCPU_CTRL	(DRIME4_PA_SONICS_INTR_CTRL)
#define DRIME4_PA_SONICS_MSRAM		0x70200000
#define DRIME4_VA_SONICS_MSRAM		DRIME4_ADDR(0x280000)
#define DRIME4_PA_SONICS_SRP		0x70100000
#define DRIME4_PA_SRP_DAP		0x50090000
#define DRIME4_PA_SRP_COMMBOX		0x50098000

#define TICKS_PER_uSEC      27

#endif
