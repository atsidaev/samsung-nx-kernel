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

	 
#ifndef __D4_MIPI_DPC_PDMA_H
#define __D4_MIPI_DPC_PDMA_H

/* -------------------------------------------------------------------*/
/* 				Includes			*/
/* -------------------------------------------------------------------*/

/* -------------------------------------------------------------------*/
/* 				Defines			*/
/* -------------------------------------------------------------------*/
#define SIZE_1MB			(1024*1024)

#define REG_WRITE(x,data)	(*((volatile unsigned int*)(x)) = (data))
#define MEM_WRITE8(x,data)	(*((volatile unsigned char*)(x)) = (data))

#define Outp32(addr, data)	(*(volatile unsigned int *)(addr) = (data))
#define Inp32(addr, data)	(data = (*(volatile unsigned int *)(addr)))

#define pdmaOutp32(offset, x) Outp32(pdma_reg_base+offset, x)
#define pdmaInp32(offset, x)  Inp32(pdma_reg_base+offset, x)

/* -------------------------------------------------------------------*/
/* 				Type Definitions			*/
/* -------------------------------------------------------------------*/
typedef struct{
	unsigned int gpio_addr;			/* gpio data access address*/
	unsigned char gpio_bit; 		/* gpio bit*/
	unsigned int prog_addr;			/* dma prog base addr*/
	unsigned int data_addr; 		/* dma data base addr*/
	unsigned int dma_event;			/* dma interrupt event selection*/
}D4_Mipi_Pdma_st;

typedef struct{
	unsigned int mipi_rdma_base;	/* mipi read dma setting*/
	unsigned int no_of_req; 		/* DPC Request for 1 frame*/
	unsigned int dpc_req_size_nKB;	/* bytes per request*/
	unsigned int x;					/* stride				*/
	unsigned int y;
	unsigned int prog_addr;			/* dma prog base addr*/
	unsigned int data_addr; 		/* dma data base addr*/
	unsigned char ui8GpioGroup;
	unsigned char ui8GpioPin;
}D4_Mipi_Dpc_st;

/* -------------------------------------------------------------------*/
/* 				Function Definitions			*/
/* -------------------------------------------------------------------*/
void d4_mipi_dpc_pdma_init(D4_Mipi_Dpc_st *pstDpc);
void d4_mipi_dpc_pdma_start(D4_Mipi_Dpc_st *pstDpc);
void d4_mipi_dpc_pdma_stop(D4_Mipi_Dpc_st *pstDpc);

void d4_mipi_dpc_pdma_start_new(unsigned int phyaddr_gdma_ch0_prog_base);
void d4_mipi_dpc_set_lutsize(unsigned int ui32DataSize);

#endif /* __D4_MIPI_DPC_PDMA_H */
/* -------------------------------------------------------------------*/
/* 				File End			*/
/* -------------------------------------------------------------------*/

