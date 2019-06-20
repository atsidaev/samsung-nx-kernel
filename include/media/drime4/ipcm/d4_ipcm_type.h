/**
 * @file d4_ipcm_type.h
 * @brief DRIMe4 IPCM Interface Header
 * @author TaeWook Nam <tw.@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_IPCM_TYPE_H__
#define __D4_IPCM_TYPE_H__


#define IPCM_MAX_BUFFER	0x10

/*
 *
 * IPCM Enumeration define section
 *
 */

/**
 * @enum	ipcs_on_off
 * @brief	K_IPCS Device Driver의 On/Off member
 */
enum ipcm_k_on_off {
	K_IPCM_OFF = 0,/**< Off */
	K_IPCM_ON/**< On */
};


/**
 * @brief IPCM operation error define
 */
enum ipcm_k_op_error {
	IPCM_K_OP_ERROR_NONE = 0,	/**< No error */
	IPCM_K_OP_UNKNOWN_ERROR = -1,	/**< Unknown operation error */
	IPCM_K_OP_PARAM_ERROR = -2,	/**< Invalid input parameter */
	IPCM_K_OP_MEM_ERROR = -3,	/**< Memory alloc or free error */
	IPCM_K_OP_TIMEOUT_ERROR = -4,	/**< Interrupt Time-out */
	IPCM_K_MD_PARAM_ERROR = -5
};

/**
 * @enum	ipcm_k_int_sig
 * @brief	IPCM DMA Interrupt flag 종류
 */
enum ipcm_k_int_sig {
	IPCM_K_INTR_BUSEND_WDMA0,		/**< WMDA0의 Bus End Interrupt(Main Out) */
	IPCM_K_INTR_INPUTEND_WDMA0,	/**< WDMA0의 Input End Interrupt */
	IPCM_K_INTR_ERROR_WDMA0,		/**< WDMA0의 Error interrupt */

	IPCM_K_INTR_BUSEND_WDMA1,		/**< WMDA1의 Bus End Interrupt(Main Resize Out) */
	IPCM_K_INTR_INPUTEND_WDMA1,	/**< WDMA1의 Input End Interrupt */
	IPCM_K_INTR_ERROR_WDMA1,		/**< WDMA1의 Error Interrupt */

	IPCM_K_INTR_BUSEND_WDMA2,		/**< WMDA2의 Bus End Interrupt(Sub Resize1 Out) */
	IPCM_K_INTR_INPUTEND_WDMA2,	/**< WDMA2의 Input End Interrupt */
	IPCM_K_INTR_ERROR_WDMA2,		/**< WDMA2의 Error Interrupt */

	IPCM_K_INTR_BUSEND_WDMA3,		/**< WMDA3의 Bus End Interrupt(Sub Resize2 Out) */
	IPCM_K_INTR_INPUTEND_WDMA3,	/**< WDMA3의 Input End Interrupt */
	IPCM_K_INTR_ERROR_WDMA3,		/**< WDMA3의 Error Interrupt */

	IPCM_K_INTR_VSYNCEND_RDMA0,	/**< RDMA0의 VSYNCEND Interrupt */
	IPCM_K_INTR_VSYNCEND_RDMA1,	/**< RDAM1의 VSYNCEND Interrupt */

	IPCM_K_SELF_TEST_DMA,		/**< */
	IPCM_K_DMA_TEST_SEQ,		/**< */

	IPCM_K_INTR_NLCD_DONE,		/**< NLC Decoder(Read DMA) Done Interrupt */
	IPCM_K_INTR_NLCD_ERROR,		/**< NLC Decoder(Read DMA) Error Interrup */

	IPCM_K_INTR_NLCE_DONE,		/**< NLC Encoder(Write DMA) Done Interrupt */
	IPCM_K_INTR_NLCE_ERROR,		/**< NLC Encoder(Write DMA) Error Interrupt */

	IPCM_K_MAX_INTR_NUM
} ;

/**
 * @enum	md_k_int_sig
 * @brief	FD Interrupt flag 종류
 */
enum md_k_int_sig {
	MD_K_INTR_GMV_END,		/**< MD의 GMV End Interrupt */
	MD_K_INTR_RMV_END,		/**< MD의 RMV End Interrupt */

	MD_K_MAX_INTR_NUM
} ;


/**
 * @enum	ldcm_k_intr_flags
 * @brief	LDCm Interrupt flag 종류
 */
enum ldcm_k_intr_flags {
	LDCM_K_LDC_TILE_FINISH = 0, /**< LDC Tile Finish */
	LDCM_K_LDC_OPERATION_FINISH = 1, /**< LDC Operation Finish */
	LDCM_K_LDC_ERROR = 2, /**< LDC Error */
	LDCM_K_HDR_TILE_FINISH = 3, /**< HDR Tile Finish */
	LDCM_K_HDR_OPERATION_FINISH = 4, /**< HDR Operation Finish */
	LDCM_K_HDR_ERROR = 5, /**< HDR Error */
	LDCM_K_NRm_TILE_FINISH = 6, /**< NRm Tile Finish */
	LDCM_K_NRm_OPERATION_FINISH = 7, /**< NRm Operation Finish */
	LDCM_K_NRm_ERROR = 8, /**< NRm Error */
	LDCM_K_MnF_TILE_FINISH = 9, /**< MnF Tile Finish */
	LDCM_K_MnF_OPERATION_FINISH = 10, /**< MnF Operation Finish */
	LDCM_K_MnF_ERROR = 11, /**< MnF Error */
	LDCM_K_FD_TILE_FINISH = 12, /**< FD Tile Finish */
	LDCM_K_FD_OPERATION_FINISH = 13, /**< FD Operation Finish */
	LDCM_K_FD_ERROR = 14, /**< FD Error */
	LDCM_K_BITBLT_OPERATION_FINISH = 15, /**< BITBLT Operation Finish */
	LDCM_K_BITBLT_ERROR = 16, /**< BITBLT Error */
	LDCM_K_LVR_VIDEO_Y_TILE_FINISH = 17, /**< LVR Video Y Tile Finish */
	LDCM_K_LVR_VIDEO_Y_OPERATION_FINISH = 18, /**< LVR Video Y Operation Finish */
	LDCM_K_LVR_VIDEO_ERROR = 19, /**< LVR Video Error */
	LDCM_K_LVR_GRP_OPERATION_FINISH = 20, /**< LVR Graphic Operation Finish */
	LDCM_K_LVR_GRP_ERROR = 21, /**< LVR Graphic Error */
	LDCM_K_OTF_TILE_FINISH = 22, /**< OTF Tile Finish */
	LDCM_K_OTF_OPERATION_FINISH = 23, /**< OTF Operation Finish */
	LDCM_K_TOP_ERROR = 24, /**< TOP Error */

	LDCM_K_MAX_INTR = 27	/**< EP Core Interrupt의 전체 개수 */
};


/**
 * @struct	ipcm_k_physical_reg_info
 * @brief	Physical register information
 */
struct ipcm_k_physical_reg_info {
	unsigned int reg_start_addr;
	unsigned int reg_size;
};

/**
 * @struct	ipcm_reg_ctrl_base_info
 * @brief	ipcm을 control 함에 있어 기본이 되는 device 정보
 */
struct ipcm_k_reg_ctrl_base_info {
	struct device *dev_info;
	unsigned int reg_base;
	int dma_irq_num;
	int md_irq_num;
	int ldcm_irq_num;

	struct ipcm_k_physical_reg_info phys_reg_info;
};

#endif   /* __D4_IPCM_TYPE_H__ */

