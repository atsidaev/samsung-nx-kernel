/* linux/arch/arm/mach-drime4/dev-spi3.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	Hwan Soon Sung <hs2704.sung@samsung.com>
 *
 * Base DRIME4 platform device definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/d4_spi_regs.h>
#include <mach/dma.h>

static struct resource drime4_spi3_resource[] = {
	[0] = {
		.start = DRIME4_PA_SPI3,
		.end   = DRIME4_PA_SPI3 + DRIME4_SPI_CH_OFF - 1,
		.flags = IORESOURCE_MEM,
	},

	/* SPI driver do not use ip's irq,
	   instead driver works on polling or DMACH irq */
	[1] = {
		.start = IRQ_SPI3,
		.end   = IRQ_SPI3,
		.flags = IORESOURCE_IRQ,
	},

	[2] = {
		.start = DMACH_SPI3_TX,
		.end   = DMACH_SPI3_TX,
		.flags = IORESOURCE_DMA,
	},

	[3] = {
		.start = DMACH_SPI3_RX,
		.end   = DMACH_SPI3_RX,
		.flags = IORESOURCE_DMA,
	},

};

static u64 drime4_device_spi3_dmamask = DMA_BIT_MASK(32);

static struct drime4_spi_info drime4_spi3_pdata = {
	.num_cs	       = 1,
	/* The number of slaves
	   there must be at least one chipselect */
	.fifo_lvl_mask = 0x1ff,
	.rx_lvl_offset = 20,
	.tx_lvl_offset = 8,
	.high_speed    = 1,
	.clk_from_cmu  = false,
	.valid_bits    = 16,
};

struct platform_device drime4_device_spi3 = {
	.name		  = SPI_MODULE_NAME,
	.id		  = 3,
	.num_resources	  = ARRAY_SIZE(drime4_spi3_resource),
	.resource	  = drime4_spi3_resource,
	.dev = {
		.dma_mask = &drime4_device_spi3_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &drime4_spi3_pdata,
	},
};
