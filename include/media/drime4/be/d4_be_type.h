/**
 * @file d4_be_type.h
 * @brief DRIMe4 BE TYPES
 * @author Niladri Mukherjee <n.mukherjee@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_BE_TYPE_H_
#define D4_BE_TYPE_H_


#define BE_RESULT int
#define D4_BE_SUCCESS (0)
#define D4_BE_ERR_IRQ_REGISTER_FAIL (-300)
#define D4_BE_ERR_INTR_TIME_OUT (-32)
#define D4_BE_AMHDR_FAILCHECK_DETECT (-33)

#define BE_WAVELET_STAT_FWT (1 << 0)
#define BE_WAVELET_STAT_IWT (1 << 1)
#define BE_WAVELET_STAT_IWTC (1 << 2)

#define BE_BLEND_CMD_LUT (1 << 0)
#define BE_BLEND_CMD_MAX (1 << 1)
#define BE_BLEND_CMD_BLD (1 << 2)

#define BE_SGTOP_CMD_PRF (1 << 0)
#define BE_SGTOP_CMD_R2S (1 << 1)
#define BE_SGTOP_CMD_GME (1 << 2)
#define BE_SGTOP_CMD_LME (1 << 3)
#define BE_SGTOP_CMD_LMC (1 << 4)

#define BE_KERN_TRUE  1
#define BE_KERN_FALSE 0


/**< Define Debug Print */
#define __D4_BE_KERN_PRINT BE_KERN_FALSE
#define __D4_BE_KERN_REGISTER_PRINT BE_KERN_FALSE

#if __D4_BE_KERN_PRINT
#define BE_KERN_DEBUG_MSG(format, args...) printk(format, ##args)
#else
#define BE_KERN_DEBUG_MSG(format, args...)
#endif

#if __D4_BE_KERN_REGISTER_PRINT
#define BE_KERN_REG_DEBUG_MSG(format, args...) printk(format, ##args)
#else
#define BE_KERN_REG_DEBUG_MSG(format, args...)
#endif


/******************************************************************************/
/*                Enumeration                   */
/******************************************************************************/
/**
 * @enum be_wavelet_cmd
 * @brief  Bayer wavelet operation stage Command type
 * 			for LLS and Amhdr
 * @author Niladri Mukherjee
 */
enum be_wavelet_cmd {
	CMD_FWT = 1, CMD_IWT = 2, CMD_IWTC = 4
};


/**
 * @enum be_reg_selection
 * @brief Register type ? íƒ???„í•œ Enumeration
 *        Register physical information ?•ë³´ë¥??»ì–´ ?¤ê¸° ?„í•œ register ? íƒ ???¬ìš©
 *
 * @author Yunmi Lee
 */
enum be_reg_selection {
	BE_TOP_REG = 0,
	BE_GHOST_REG = 1,
	BE_SNR_REG = 2,
	BE_SG_REG = 3,
	BE_BLEND_REG = 4,
	BE_FME_REG = 5,
	BE_3DME_REG = 6,
	BE_DMA_REG = 7,
	BE_REG_MAX = 8
};

/**
 * @enum be_interrupt_level
 * @brief  Bayer Interrupt Numbers
 * 			for interrupt handling
 * 			in Bayer Engine
 * @author Niladri Mukherjee
 */
enum be_interrupt_level {
	BE_INT_LEVEL_GD_GD_DIFF_GF = 0,
	/*!< BE_INT_LEVEL_GD_GD_DIFF_GF*/
	BE_INT_LEVEL_GD_FW_PRE_LVR = 1,
	/*!< BE_INT_LEVEL_GD_FW_PRE_LVR*/
	BE_INT_LEVEL_GD_FW_WEIGHT = 2,
	/*!< BE_INT_LEVEL_GD_FW_WEIGHT*/
	BE_INT_LEVEL_GD_DB_BLEND = 3,
	/*!< BE_INT_LEVEL_GD_DB_BLEND*/
	BE_INT_LEVEL_FC_DT_PRE_GF3 = 4,
	/*!< BE_INT_LEVEL_FC_DT_PRE_GF3*/
	BE_INT_LEVEL_FC_DT_GF3 = 5,
	/*!< BE_INT_LEVEL_FC_DT_GF3*/
	BE_INT_LEVEL_FC_OS_LOW_GF = 6,
	/*!< BE_INT_LEVEL_FC_OS_LOW_GF*/
	BE_INT_LEVEL_FC_OS_COUNT = 7,
	/*!< BE_INT_LEVEL_FC_OS_COUNT*/
	BE_INT_LEVEL_FC_CC_FCHK2 = 8,
	/*!< BE_INT_LEVEL_FC_CC_FCHK2*/
	BE_INT_LEVEL_SNR_SNR = 9,
	/*!< BE_INT_LEVEL_SNR_SNR*/
/*	BE_INT_LEVEL_SNR_TILE = 10,*/
	BE_INT_LEVEL_FCHK_CORNER_CASE = 10,
	/*!< BE_INT_LEVEL_SNR_TILE*/
	BE_INT_LEVEL_SNR_FWT = 11,
	/*!< BE_INT_LEVEL_SNR_FWT*/
	BE_INT_LEVEL_SNR_IWT = 12,
	/*!< BE_INT_LEVEL_SNR_IWT*/
	BE_INT_LEVEL_SNR_STRIP = 13,
	/*!< BE_INT_LEVEL_SNR_STRIP*/
	BE_INT_LEVEL_SG_PRF = 14,
	/*!< BE_INT_LEVEL_SG_PRF*/
	BE_INT_LEVEL_SG_R2S = 15,
	/*!< BE_INT_LEVEL_SG_R2S*/
	BE_INT_LEVEL_SG_GME = 16,
	/*!< BE_INT_LEVEL_SG_GME*/
	BE_INT_LEVEL_SG_LME = 17,
	/*!< BE_INT_LEVEL_SG_LME*/
	BE_INT_LEVEL_SG_LMC = 18,
	/*!< BE_INT_LEVEL_SG_LMC*/
	BE_INT_LEVEL_BLD_LUT = 19,
	/*!< BE_INT_LEVEL_BLD_LUT*/
	BE_INT_LEVEL_BLD_MAX = 20,
	/*!< BE_INT_LEVEL_BLD_MAX*/
	BE_INT_LEVEL_BLD_BLD = 21,
	/*!< BE_INT_LEVEL_BLD_BLD*/
	BE_INT_LEVEL_FME = 22,
	/*!< BE_INT_LEVEL_FME*/
//	BE_INT_LEVEL_3DME_EVERY_FIN = 23,
	BE_INT_LEVEL_FCHK_OVERSPEC = 23,
	/*!< BE_INT_LEVEL_3DME_EVERY_FIN*/
	BE_INT_LEVEL_3DME_ALL_FIN = 24,
	/*!< BE_INT_LEVEL_3DME_ALL_FIN*/
	BE_INT_LEVEL_DMA = 25,
	/*!< BE_INT_LEVEL_DMA*/
	BE_INT_LEVEL_MAX = 26,
/*!< BE_INT_LEVEL_MAX*/
	BE_LLSHDR_NR_WT_ALL = 31
};

/**
 * @enum be_dma_interrupt_level
 * @brief  Bayer DMA Interrupt Command type
 * 			for LLS and Amhdr
 * @author Niladri Mukherjee
 */
enum be_dma_interrupt_level {
	BE_DMA_INT_LEVEL_WDMA0_INTINPUTEND = 0,
	BE_DMA_INT_LEVEL_WDMA0_INTBUSEND = 1,
	BE_DMA_INT_LEVEL_WDMA0_INTERROR	= 2,

	BE_DMA_INT_LEVEL_WDMA1_INTINPUTEND = 4,
	BE_DMA_INT_LEVEL_WDMA1_INTBUSEND = 5,
	BE_DMA_INT_LEVEL_WDMA1_INTERROR = 6,

	BE_DMA_INT_LEVEL_WDMA2_INTINPUTEND = 8,
	BE_DMA_INT_LEVEL_WDMA2_INTBUSEND = 9,
	BE_DMA_INT_LEVEL_WDMA2_INTERROR = 10,

	BE_DMA_INT_LEVEL_MAX
};


enum be_dma_intr_block
{
	BLD_DMA_INTR,
	R2S_DMA_INTR,
	LMC_DMA_INTR
};



/**
 * @enum be_blend_cmd
 * @brief  Bayer YCC Operation mode
 * 			for Amhdr
 * @author Niladri Mukherjee
 */
enum be_blend_cmd {
	CMD_LUT = 1 << 0, CMD_MAX = 1 << 1, CMD_BLD = 1 << 2
};

/**
 * @enum be_wavelet_int_status
 * @brief  Bayer wavelet interrupt stage
 * 			handling for LLS and Amhdr
 * @author Niladri Mukherjee
 */
enum be_wavelet_int_status {
	STAT_FWT = 1 << 0, STAT_IWT = 1 << 1, STAT_IWTC = 1 << 2
};


/**
 * @enum be_sgtop_cmd
 * @brief  Bayer Sg Operation Process Control
 * 			during SG Operation
 * @author Niladri Mukherjee
 */
enum be_sgtop_cmd {
	CMD_PRF = 1 << 0,
	CMD_R2S = 1 << 1,
	CMD_GME = 1 << 2,
	CMD_LME = 1 << 3,
	CMD_LMC = 1 << 4
};

/**
 * @enum	be_pmu_type
 * @brief	BE Device Driver??On/Off member
 */
enum be_pmu_type {
	BE_PMU_OFF = 0,/**< Off */
	BE_PMU_ON/**< On */
};
/******************************************************************************/
/*                  Structure                   */
/******************************************************************************/
/**
 * @struct be_phys_reg_info
 * @brief Physical register information ?¬ìš©???„í•œ structure
 *
 * @author Yunmi Lee
 */
struct be_phys_reg_info {
	unsigned int reg_start_addr; /**< Register physical start address */
	unsigned int reg_size;		 /**< Register size */
};

struct be_hdrlls_completion_event {
	enum be_interrupt_level hdrlls_ctrl;
	unsigned int timeout;
	int event_result;
};

struct be_iwt_info {
	unsigned int inv_addr;
	unsigned int inv_size;
	unsigned int lw;
};

struct be_nr_iwt_info {
	unsigned int inv_addr;
	unsigned int inv_size;
	unsigned int lw;
	unsigned int stride;
	int dma_bit;
};

#endif		/* D4_BE_TYPE_H_ */
