/* linux/arch/arm/mach-drime4/dma.c
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

#include <linux/dma-mapping.h>
#include <linux/amba/pl330.h>
#include <linux/amba/bus.h>
#include <linux/irq.h>

#include <mach/map.h>
#include <mach/irqs.h>
#include <mach/dma.h>


static u8 pdma_peri[] = {
	DMACH_SPI0_TX,
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

	DMACH_UART0_TX,
	DMACH_UART0_RX,
	DMACH_UART1_TX,
	DMACH_UART1_RX,

	DMACH_I2S0_TX,
	DMACH_I2S0_RX,
	DMACH_I2S1_TX,
	DMACH_I2S1_RX,
	DMACH_I2S2_TX,
	DMACH_I2S2_RX,

	DMACH_PMU,
	DMACH_MIPI_TX,
};


static struct dma_pl330_platdata drime4_pdma_pdata = {
	.nr_valid_peri = ARRAY_SIZE(pdma_peri),
	.peri_id = pdma_peri,
};

static u8 mdma_peri[] = {
		MDMACH_MEM0,
		MDMACH_MEM1,
		MDMACH_MEM2,
		MDMACH_MEM3,
		MDMACH_MEM4,
		MDMACH_MEM5,
		MDMACH_MEM6,
		MDMACH_MEM7,
};

static struct dma_pl330_platdata drime4_mdma_pdata = {
	.nr_valid_peri = ARRAY_SIZE(mdma_peri),
	.peri_id = mdma_peri,
};



static AMBA_AHB_DEVICE(drime4_pdma, "dma-pl330.0", 0x0041330, DRIME4_PA_PDMA, {IRQ_PDMA}, &drime4_pdma_pdata);

static AMBA_AHB_DEVICE(drime4_mdma, "mdma-pl330.1", 0x0041330, DRIME4_PA_MDMA, {IRQ_MDMA}, &drime4_mdma_pdata);



static int __init drime4_dma_init(void)
{
	dma_cap_set(DMA_SLAVE, drime4_pdma_pdata.cap_mask);
	dma_cap_set(DMA_CYCLIC, drime4_pdma_pdata.cap_mask);
	amba_device_register(&drime4_pdma_device, &iomem_resource);

	dma_cap_set(DMA_MEMCPY, drime4_mdma_pdata.cap_mask);
	amba_device_register(&drime4_mdma_device, &iomem_resource);



	return 0;
}
arch_initcall(drime4_dma_init);
