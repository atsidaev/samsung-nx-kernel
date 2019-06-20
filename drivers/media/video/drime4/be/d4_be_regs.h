
#ifndef D4_BE_REGS_H_
#define D4_BE_REGS_H_

/* Register Read/Write Macros */
#define DRIME4_BE_REG(x) (x)
#define WRITE_BE_REG(reg_be_addr, offset, val) __raw_writel(val, reg_be_addr + offset)
#define READ_BE_REG(reg_be_addr, offset) __raw_readl(reg_be_addr + offset)

#define BE_K_BASE					(0x0000)/*0x50080000(BaseAddress)*/
#define BE_K_GH_BASE				(0x1000)
#define BE_K_SNR_BASE				(0x2000)
#define BE_K_SG_BASE				(0x3000)
#define BE_K_BLEND_BASE				(0x4000)
#define BE_K_FME_BASE				(0x5000)
#define BE_K_3DME_BASE				(0x6000)
#define BE_K_DMA_BASE				(0x7000)
#define BE_K_TOP_BASE				(BE_K_BASE)


#define BE_SGR2S_WDMA_CH						0
#define BE_SGLMC_WDMA_CH	  					0
#define BE_WLET_KWDMA_CH_IWT                    1


/*BE Offset*/
/* Register Read/Write Macros */
#define BE_TOP_INT_ENABLE_IP 			DRIME4_BE_REG(0x0010)
#define BE_TOP_INT_CLEAR_IP	 			DRIME4_BE_REG(0x0014)
#define BE_TOP_INT_STATUS_IP			DRIME4_BE_REG(0x0018)
#define BE_TOP_INT_ENABLE_DMA			DRIME4_BE_REG(0x0020)
#define BE_TOP_INT_CLEAR_DMA   			DRIME4_BE_REG(0x0024)
#define BE_TOP_INT_STATUS_DMA  			DRIME4_BE_REG(0x0028)

#define D4_BE_BLD_STATUS	  			DRIME4_BE_REG(0x0004)
#define D4_BE_SGTOP_STATUS				DRIME4_BE_REG(0x0004)
#define D4_BE_SNR_WLET_STATUS  			DRIME4_BE_REG(0x080C)
//Combine this into one
#define D4_BE_SNR_STATUS				DRIME4_BE_REG(0x0008)
#define D4_BE_SG_INT_MASK				DRIME4_BE_REG(0x0008)
#define D4_BE_BLD_INT_MASK	  			DRIME4_BE_REG (0x0008)
#define D4_BE_TOP_INT_STATUS_DMA  		DRIME4_BE_REG(0x0028)



#define D4_BE_KSNR_WLET_START_IWT  		(0x0800)
#define D4_BE_KSNR_WLET_IWT_CONTINUE	(0x0830)
#define D4_BE_KVDMA_W1_IFMT10     		(0x0380)
#define D4_BE_KVDMA_W1_EVN0       		(0x038C)
#define D4_BE_KVDMA_W1_ODD0       		(0x0390)
#define D4_BE_KVDMA_W1_FOFF0      		(0x0394)

#define D4_BE_KVDMA_W1_XOFF10     		(0x03A4)
#define D4_BE_KVDMA_W1_BGNX10     		(0x03B0)
#define D4_BE_KVDMA_W1_TIMETHR    		(0x03B4)
#define D4_BE_KVDMA_W1_BURSTLEN   		(0x03B8)
#define D4_BE_KVDMA_W1_XOFF32     		(0x03E4)
#define D4_BE_KVDMA_W1_BGNX32     		(0x03F0)


#endif
