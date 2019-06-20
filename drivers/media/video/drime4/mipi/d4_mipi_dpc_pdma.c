/**
 * @file d4_mipi_pdma.c
 * @brief DRIMe4 MIPI PDMA control Function File for D4C DPC LUT xfer
 * @author MURUGESA <murugesa.p@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


// -------------------------------------------------------------------
// 				Includes
// -------------------------------------------------------------------
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <mach/d4_spi_regs.h>
#include <mach/dma.h>
#include <mach/map.h>
#include <linux/delay.h>

#include <linux/dmaengine.h>
#include <linux/amba/pl330.h>

#include <mach/mipi/d4_mipi.h>
#include "d4_mipi_ctrl_dd.h"

#include "media/drime4/mipi/d4_mipi_ioctl.h"
#include "media/drime4/mipi/d4_mipi_dpc_pdma.h"
#include "d4_pdma_for_d4c.h"

/* -------------------------------------------------------------------*/
/* 				Defines				*/
/* -------------------------------------------------------------------*/
#if 0
#define DBG_PRINT1 printk
#else
#define DBG_PRINT1(S...)
#endif

#define XFER_DMAADDR_INVALID DMA_BIT_MASK(32)

#define PDMA_CH0 0
#define PDMA_PERI_ID 31

#define GPIO_DATA_REG1(GroupNumber) ((0x30050000 + 0x3FC) + (GroupNumber * 0x1000))

#define DBGCMD		0xd04
#define DBGINST0	0xd08
#define DBGINST1	0xd0c

#define DEFAULT_DPC_REQ_SIZE_KB	12

/* -------------------------------------------------------------------*/
/* 				Globals					*/
/* -------------------------------------------------------------------*/
struct dma_chan *tx_chan;
dma_addr_t tx_dma;
enum dma_ch tx_dmach;

unsigned int viraddr_gdma_ch0_prog_base = 0;
unsigned int phyaddr_gdma_ch0_prog_base = 0;
unsigned int viraddr_gdma_ch0_data_base;

D4_Mipi_Pdma_st stPdma;
unsigned int ui32DataSz = 0; 
unsigned int ui32PdmaProgBase;
unsigned int * ui32pwbuffer = NULL;
unsigned int ui32_mipi_gdma_data_size = 0;

/* -------------------------------------------------------------------*/
/* 				Externals				*/
/* -------------------------------------------------------------------*/
extern struct drime4_mipi *g_mipi;

/* -------------------------------------------------------------------*/
/* 				PDMA - Initialization Functions				*/
/* -------------------------------------------------------------------*/
static int d4_mipi_pdma_map(void* pvWbuffer, unsigned int ui32DataSz)
{
	tx_dma = dma_map_single(g_mipi->dev, (void *)pvWbuffer, ui32DataSz, DMA_TO_DEVICE);

	if (dma_mapping_error(g_mipi->dev, tx_dma)) {
		dev_err(g_mipi->dev, "dma_map_single Tx failed\n");
		tx_dma = XFER_DMAADDR_INVALID;
		return -ENOMEM;
	}

	return 0;
}

static void d4_mipi_pdma_unmap (void* pvWbuffer, unsigned int ui32DataSz)
{
	if (pvWbuffer != NULL && tx_dma != XFER_DMAADDR_INVALID)
		dma_unmap_single(g_mipi->dev, tx_dma, ui32DataSz, DMA_TO_DEVICE);
}

static int d4_mipi_pdma_acquire(void)
{
	dma_cap_mask_t mask;

	/* Tru to acquire a generic DMA engine slave channel */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);

	tx_chan = dma_request_channel(mask, pl330_filter, (void *)tx_dmach);

	if (!tx_chan) {
		dev_err(g_mipi->dev, "no TX DMA channel!\n");
		return -ENODEV;
	}

/*	printk ("setup for DMA on TX %s\n",	dma_chan_name(tx_chan));*/

	return 0;
}

/* -------------------------------------------------------------------*/
/* 				PDMA - Data Trasfer Functions				*/
/* -------------------------------------------------------------------*/
extern int pl330_external_intruction_start(unsigned int codebase, struct dma_chan *tx_chan);

int d4_mipi_pdma_transfer(unsigned int *pui32Data, unsigned int data_len)
{
	int status = 0;
	int reval;

	ui32_mipi_gdma_data_size = data_len;
	ui32pwbuffer = pui32Data;
	tx_dmach = DMACH_MIPI_TX;

	if (d4_mipi_pdma_map(ui32pwbuffer, ui32_mipi_gdma_data_size)) {
		printk("\nMIPI_DPC_ERR > Unable to map message buffers!\n");
		status = -ENOMEM;
		return status;
	}

	if (d4_mipi_pdma_acquire()) {
		printk("\nMIPI_DPC_ERR > Unable to get a dma channel\n");
		status = -ENOMEM;
		return status;
	}

/*	printk ("\nPDMA Xfering data_size [%d] wbuffer [%x]\n", ui32_mipi_gdma_data_size, (unsigned int)ui32pwbuffer);*/

	pl330_external_intruction_start(ui32PdmaProgBase, tx_chan);

	return status;
}


void d4_mipi_pdma_transfer_end(void)
{
	d4_mipi_pdma_unmap(ui32pwbuffer, ui32_mipi_gdma_data_size);

	if (tx_chan)
		dma_release_channel(tx_chan);
}

/* -------------------------------------------------------------------*/
/* 				PDMA Instructions generation					*/
/* -------------------------------------------------------------------*/
void d4_mipi_dpc_pdma_set(D4_Mipi_Pdma_st *pstPdma, D4_Mipi_Dpc_st *pstDpc)
{
	unsigned int i, mipi_rdma_base_tmp;
	unsigned int data_phyaddr = (pstPdma->data_addr);
	
    /*-----------------------------------------------------------------------------*/
    /* PDMA Data*/
    /*-----------------------------------------------------------------------------*/
	REG_WRITE(pstPdma->data_addr+0x00, pstPdma->gpio_bit); /* poll enable*/
	REG_WRITE(pstPdma->data_addr+0x04, pstPdma->gpio_bit); /* poll enable*/
	REG_WRITE(pstPdma->data_addr+0x08, 0x00000001); /* poll start, bit[0]=1*/
	REG_WRITE(pstPdma->data_addr+0x0c, 0x00000000); /* all=0*/

	mipi_rdma_base_tmp = pstDpc->mipi_rdma_base;

	for(i=0; i<pstDpc->no_of_req; i++)
	{
		REG_WRITE(pstPdma->data_addr+0x010+i*4, mipi_rdma_base_tmp+i*(pstDpc->dpc_req_size_nKB*1024)); /* rdma base addr*/
		REG_WRITE(pstPdma->data_addr+0x1000+i*4, mipi_rdma_base_tmp+i*(pstDpc->dpc_req_size_nKB*1024)+pstDpc->x*2); /* rdma base addr + stride*/
	}

	/*-----------------------------------------------------------------------------*/
	/* PDMA Code					*/
	/*-----------------------------------------------------------------------------*/
	INST_BASE = viraddr_gdma_ch0_prog_base;
	INST_ADDR = viraddr_gdma_ch0_prog_base;

	// Init
	DMAFLUSHP(PDMA_PERI_ID);
	DMAMOV(1, 		/* csr*/
		0<<28 |		/* [31..28] endian swap*/
		0<<25 |		/* [27..25] dst_awcache*/
		0<<22 |		/* [24..22] dst_awprot*/
		0<<18 |     	/* [21..18] dst_burst_len*/
		2<<15 |     	/* [17..15] dst_burst_size*/
		0<<14 |     	/* [14..14] dst_addr_inc*/
		0<<11 |		/* [13..11] src_cache*/
		0<<8  |		/* [10..08] src_prot*/
		0<<4  |		/* [07..04] src_burst_len*/
		2<<1  |    	/* [03..01] src_burst_size*/
		0 ); 			/* [00..00] src_addr_inc*/

	for(i=0; i<pstDpc->no_of_req; i++)
	{
		/* Polling GPIO High */
		DMA_POLL(data_phyaddr, data_phyaddr+0x04, data_phyaddr+0x08, pstPdma->gpio_addr);

		/* RDMA start*/
		DMA_S_SET(data_phyaddr+0x010+i*4, MIPI_RDMA_EVEN_ADDR_0); 	/* base addr even*/
		DMA_S_SET(data_phyaddr+0x1000+i*4,MIPI_RDMA_ODD_ADDR_0); 	/* base addr even + stride*/
		DMA_S_SET(data_phyaddr+0x0c, MIPI_RW_DMA_ENABLE_CTRL); 	/* rdma en*/
		DMA_S_SET(data_phyaddr+0x08, MIPI_DMA_START_CTRL); 			/* rdma start_1*/
		DMA_S_SET(data_phyaddr+0x0c, MIPI_DMA_START_CTRL); 			/* rdma start_0*/

		/* Polling GPIO Low */
		DMA_POLL(data_phyaddr, data_phyaddr+0x0c, data_phyaddr+0x08, pstPdma->gpio_addr);
	}

	DMAWMB();
	DMASEV(pstPdma->dma_event);
	DMAEND();
}

/* -------------------------------------------------------------------*/
/* 				IOCTL Functions					*/
/* -------------------------------------------------------------------*/

void d4_mipi_dpc_pdma_init(D4_Mipi_Dpc_st *pstDpc)
{
	viraddr_gdma_ch0_prog_base = pstDpc->prog_addr;
	viraddr_gdma_ch0_data_base = pstDpc->data_addr;

	stPdma.gpio_addr = GPIO_DATA_REG1(pstDpc->ui8GpioGroup);/* [31:0] gpio_addr */
	stPdma.gpio_bit = 1<<pstDpc->ui8GpioPin;			/* [7:0]  gpio_bit */
	stPdma.prog_addr = viraddr_gdma_ch0_prog_base; 	 		/* [31:0] prog_addr */
	stPdma.data_addr = viraddr_gdma_ch0_data_base; 	 		/* [31:0] data_addr */
	stPdma.dma_event = PDMA_CH0;				   			/* dma_event */

	ui32DataSz = (pstDpc->no_of_req * pstDpc->dpc_req_size_nKB * 1024); /*muru - TBD*/

	d4_mipi_dpc_pdma_set(&stPdma, pstDpc);

	phyaddr_gdma_ch0_prog_base = viraddr_gdma_ch0_prog_base;
}

void d4_mipi_dpc_pdma_start(D4_Mipi_Dpc_st *pstDpc)
{
	ui32PdmaProgBase = phyaddr_gdma_ch0_prog_base;

	d4_mipi_pdma_transfer ((unsigned int *)phyaddr_gdma_ch0_prog_base, ui32DataSz);
}

void d4_mipi_dpc_pdma_stop(D4_Mipi_Dpc_st *pstDpc)
{
	d4_mipi_pdma_transfer_end();
}

/* -------------------------------------------------------------------*/
/* 				File End						*/
/* -------------------------------------------------------------------*/

