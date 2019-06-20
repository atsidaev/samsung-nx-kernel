/* linux/arch/arm/mach-drime4/include/mach/dma.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - DMA support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_DMA_H
#define __MACH_DRIME4_DMA_H

enum dma_ch {
	/*PDMA*/
	DMACH_SPI0_TX = 0,
	DMACH_SPI0_RX,
	DMACH_SPI1_TX,
	DMACH_SPI1_RX,
	DMACH_SPI2_TX,
	DMACH_SPI2_RX,
	DMACH_SPI3_TX,
	DMACH_SPI3_RX,
	DMACH_SPI4_TX,
	DMACH_SPI4_RX,
	DMACH_SPI5_TX,
	DMACH_SPI5_RX,
	DMACH_SPI6_TX,
	DMACH_SPI6_RX,
	DMACH_SPI7_TX,
	DMACH_SPI7_RX,

	DMACH_UART0_TX = 16,
	DMACH_UART0_RX,
	DMACH_UART1_TX,
	DMACH_UART1_RX,

	DMACH_I2S0_TX = 20,
	DMACH_I2S0_RX,
	DMACH_I2S1_TX,
	DMACH_I2S1_RX,
	DMACH_I2S2_TX,
	DMACH_I2S2_RX,

	DMACH_PMU = 26,
	DMACH_MIPI_TX = 27,
	/*MDMA*/
	MDMACH_MEM0 = 29,
	MDMACH_MEM1,
	MDMACH_MEM2,
	MDMACH_MEM3,
	MDMACH_MEM4,
	MDMACH_MEM5,
	MDMACH_MEM6,
	MDMACH_MEM7,
	DMACH_MAX
};




static inline bool drime4_dma_has_circular(void)
{
	return true;
}

struct drime4_dma_client {
	char                *name;
};
#endif
