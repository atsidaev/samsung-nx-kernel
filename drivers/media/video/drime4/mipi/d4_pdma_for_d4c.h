/**
 * @file d4_pdma_for_d4c.h
 * @brief DRIMe4 PDMA Header File for D4C DPC LUT xfer
 * @author MURUGESA <murugesa.p@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/******************************************************************************/
/*                 MIPI DMA Register Offset Define                            */
/******************************************************************************/
#define MIPI_DMA_BASE						0x50043000

/**< MIPI - RDMA Control Register */
#define MIPI_RDMA_CTRL			            (MIPI_DMA_BASE + 0x100)
#define MIPI_RDMA_WIDTH						(MIPI_DMA_BASE + 0x104)
#define MIPI_RDMA_HEIGHT					(MIPI_DMA_BASE + 0x108)
#define MIPI_RDMA_EVEN_ADDR_0               (MIPI_DMA_BASE + 0x10C)
#define MIPI_RDMA_ODD_ADDR_0                (MIPI_DMA_BASE + 0x110)
#define MIPI_RDMA_DELTA_ADDR_0              (MIPI_DMA_BASE + 0x114)
#define MIPI_RDMA_EVEN_ADDR_1               (MIPI_DMA_BASE + 0x118)
#define MIPI_RDMA_ODD_ADDR_1                (MIPI_DMA_BASE + 0x11C)
#define MIPI_RDMA_DELTA_ADDR_1              (MIPI_DMA_BASE + 0x120)
#define MIPI_RDMA_CH_MODE					(MIPI_DMA_BASE + 0x138)

/**< MIPI - DMA Control Register */
#define MIPI_DMA_START_CTRL      			(MIPI_DMA_BASE + 0xA20)
#define MIPI_RW_DMA_ENABLE_CTRL  			(MIPI_DMA_BASE + 0xA24)
#define MIPI_DMA_READY_CTRL      			(MIPI_DMA_BASE + 0xA28)
#define MIPI_DMA_HBLANK_CTRL      			(MIPI_DMA_BASE + 0xA2C)
#define MIPI_DMA_WAITCNT_CTRL      			(MIPI_DMA_BASE + 0xA34)

static unsigned int INST_BASE;
static unsigned int INST_ADDR;
static unsigned char  INST_CNT;

typedef enum {
	COUNT_0 = 0,
	COUNT_1 = 1
} Count_type;

typedef enum {
	DMALPEND_DMALPFE = 0,
	DMALPEND_DMALP = 1
} DMALPEND_type;

typedef enum {
	RD_SAR = 0,
	RD_CSR = 1,
	RD_DAR = 2,
} Rd_type;

typedef enum {
	CACHE_VALID = 0,
	CACHE_INVALID = 1,
} Cache_valid;

#if 0
#define DBG_PRINT printk
#else
#define DBG_PRINT(S...)
#endif

/* end signals to the DMAC */

void DMAEND(void)
{
	//`CPU.SingleByteWr(INST_ADDR, 8'h00);
	MEM_WRITE8(INST_ADDR, 0x00);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
}

/* flush peripheral clears the state */

void DMAFLUSHP(unsigned char peri_num)
{
	unsigned char halfword;

	//`CPU.SingleByteWr(INST_ADDR, 8'b00110101);
	MEM_WRITE8(INST_ADDR, 0x35);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	halfword = peri_num<<3;
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMAFLUSHP done. [0x%08X]\n", INST_ADDR-2);
}

/* load instructs the dmac */

void DMALD(void)
{
	unsigned char halfword;

	halfword = 0x01<<2;
	//`CPU.SingleByteWr(INST_ADDR, hardword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMALD done. [0x%08X]\n", INST_ADDR-1);
}

/* conditional load instructs the dmac */

void DMALDS(void)
{
	unsigned char halfword;

	halfword = (0x01<<2) | 0x01;
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMALD done. [0x%08X]\n", INST_ADDR-1);
}

/* load and notify peripheral instructs the dmac */

void DMALDP(unsigned char peri_no)
{
	unsigned char halfword;

	halfword = (0x09<<2) | 0x01;
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	halfword = peri_no<<3;
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMALDP done. [0x%08X]\n", INST_ADDR-2);
}

/* loop instructs the dmac */

void DMALP(Count_type counter_sel, unsigned char iter_no)
{
	unsigned char halfword;

	halfword = (0x08<<2) | (counter_sel<<1);
	//`CPU.SingleByteWr(INST_ADDR, hardword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	//`CPU.SingleByteWr(INST_ADDR, iter_no);
	MEM_WRITE8(INST_ADDR, iter_no);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMALP done. [0x%08X]\n", INST_ADDR-2);
}

/* loop end indicates the last instruction */

void DMALPEND(DMALPEND_type nf, Count_type counter_sel, unsigned char backward_jump)
{
	unsigned char halfword;

	halfword = (0x1<<5) | (nf<<4) | (0x01<<3) | (counter_sel<<2);
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	//`CPU.SingleByteWr(INST_ADDR, backward_jump);
	MEM_WRITE8(INST_ADDR, backward_jump);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMALPEND done. [0x%08X]\n", INST_ADDR-2);
}

/* move instructs the dmac */

void DMAMOV(Rd_type rd, unsigned int imm)
{
	unsigned char halfword;

	//`CPU.SingleByteWr(INST_ADDR, 8'b10111100);
	MEM_WRITE8(INST_ADDR, 0xbc);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	//`CPU.SingleByteWr(INST_ADDR, rd);
	MEM_WRITE8(INST_ADDR, rd);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	halfword = (unsigned char) imm & 0x000000FF;
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	halfword = (unsigned char) ((imm & 0x0000FF00)>>8);
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	halfword = (unsigned char) ((imm & 0x00FF0000)>>16);
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	halfword = (unsigned char) ((imm & 0xFF000000)>>24);
	//`CPU.SingleByteWr(INST_ADDR, hardword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMAMOV done. [0x%08X]\n", INST_ADDR-6);
}

/* no operation */

void DMANOP(void)
{
	//`CPU.SingleByteWr(INST_ADDR, 8'b00011000);
	MEM_WRITE8(INST_ADDR, 0x18);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMANOP done. [0x%08X]\n", INST_ADDR-1);
}

/* send event instructs the dmac */

void DMASEV(unsigned char event_inst)
{
	unsigned char halfword;

	//`CPU.SingleByteWr(INST_ADDR, 8'b00110100);
	MEM_WRITE8(INST_ADDR, 0x34);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	halfword = event_inst<<3;
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMASEV done. [0x%08X]\n", INST_ADDR-2);
}

/* store instructs the dmac */

void DMAST(void)
{
	unsigned char halfword;

	halfword = 0x02<<2;
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMAST done. [0x%08X]\n", INST_ADDR-1);
}

/* conditional store instructs the dmac */

void DMASTS(void)
{
	unsigned char halfword;

	halfword = (0x02<<2) | 0x01;
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMAST done. [0x%08X]\n", INST_ADDR-1);
}

/* store zero instructs the dmac */

void DMASTZ(void)
{
	//`CPU.SingleByteWr(INST_ADDR, 8'b00001100);
	MEM_WRITE8(INST_ADDR, 0x0c);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMASTZ done. [0x%08X]\n", INST_ADDR-1);
}

// wait for event instructs the dmac

void DMAWFE(Cache_valid i, unsigned char event_inst)
{
	unsigned char halfword;

	//`CPU.SingleByteWr(INST_ADDR, 8'b00110110);
	MEM_WRITE8(INST_ADDR, 0x36);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;

	halfword = (event_inst<<3) | (i<<1);
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMAWFE done. [0x%08X]\n", INST_ADDR-2);
}

/* wait for peripheral instructs the dmac */

void DMAWFP(unsigned char peri_no)
{
	unsigned char halfword;

	halfword = (0x0c<<2) | 0x01;
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	halfword = peri_no<<3;
	//`CPU.SingleByteWr(INST_ADDR, halfword);
	MEM_WRITE8(INST_ADDR, halfword);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMAWFP done. [0x%08X]\n", INST_ADDR-2);
}

/* write memory barrier forces the dma channel */

void DMAWMB(void)
{
	//`CPU.SingleByteWr(INST_ADDR, 8'b00010011);
	MEM_WRITE8(INST_ADDR, 0x13);
	INST_ADDR = INST_ADDR + 1;
	INST_CNT  = INST_CNT + 1;
	DBG_PRINT("DMAWMB done. [0x%08X]\n", INST_ADDR-1);
}

//---------------------------------------------------------------------
// DMA_S_SET : Single MemSet
//---------------------------------------------------------------------
void DMA_S_SET(unsigned int sar, unsigned int dar)
{
	DMAMOV(RD_SAR, sar);     // SAR
	DMAMOV(RD_DAR, dar);     // DAR
	DMALD();
	DMAST();
}

//---------------------------------------------------------------------
// DMA_POLL : Polling
//---------------------------------------------------------------------
void DMA_POLL(unsigned int poll_en_addr, unsigned int poll_data_addr, unsigned int poll_start_addr, unsigned int target_addr)
{
	DMA_S_SET(poll_en_addr, 0x30011014);
	DMA_S_SET(poll_data_addr, 0x30011018);
	DMA_S_SET(poll_start_addr, 0x30011010);
	DMA_S_SET(target_addr, 0x3001101c);

	DMALP(0,100);
	INST_CNT = 0;
	DMAWFP(31);
	DMAFLUSHP(31);
	DMALDS();
	DMASTS();
	DMAWMB();
	DMALPEND(0,1,INST_CNT);         // DMALPFE
}

