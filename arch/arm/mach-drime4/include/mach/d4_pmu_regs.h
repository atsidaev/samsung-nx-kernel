/**
 * @file d4_pmu_regs.h
 * @brief  DRIMe4 Power Mamagement Unit Registers Define for Device Driver
 * @author Kyuchun Han <kyuchun.han@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DRIME4_PMU_REGS_H
#define __DRIME4_PMU_REGS_H

#include <mach/d4_reg_macro.h>


#define PMU_MODULE_NAME		"drime4_pmu"


/******************************************************************************/
/*							Register Offset Define						*/
/******************************************************************************/

/**< PMU Registers */
#define PMU_CLK_CHG_MODE				(0x00)
#define PMU_STATUS								(0x04)

#define PMU_REQ_STOP							(0x10)
#define PMU_ACK_STOP							(0x14)
#define PMU_ACK_MASK							(0x18)
#define PMU_REQ_CNT							(0x1C)
#define PMU_CLK_CHG_EN						(0x20)
#define PMU_PRE_POST_CNT				(0x24)

#define PMU_ISOEN									(0x30)
#define PMU_SCPRE									(0x34)
#define PMU_SCALL									(0x38)
#define PMU_SCACK									(0x3C)
#define PMU_USER_DATA0				(0xA0)
#define PMU_USER_DATA1				(0xA4)
#define PMU_USER_DATA2				(0xA8)
#define PMU_USER_DATA3		  	(0xAC)
#define PMU_USER_DATA4				(0xB0)
#define PMU_USER_DATA5			  (0xB4)
#define PMU_USER_DATA6				(0xB8)
#define PMU_USER_DATA7				(0xBC)

#define BUS_RESET_PP					(0x2420)
#define BUS_RESET_IPCM				(0x2820)
#define BUS_RESET_IPCS				(0x2C20)
#define BUS_RESET_EP				(0x3020)
#define BUS_RESET_BE				(0x3420)
#define BUS_RESET_DP				(0x4420)
#define BUS_RESET_JPEG			(0x4020)

/**< PMU CLK CHANGE MODE REGISTER */
#define PMU_CHG_BY_CPU(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PMU_CHG_BY_WFE(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PMU_CHG_BY_CNT(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PMU_CHG_ACK(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define PMU_CHG_CA9_WAKEUP(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)

#define PMU_CHG_BY_GET_CPU(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define PMU_CHG_BY_GET_WFE(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define PMU_CHG_BY_GET_CNT(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define PMU_CHG_GET_ACK(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define PMU_CHG_CA9_GET_WAKEUP(val) \
	GET_REGISTER_VALUE(val, 8, 1)



/**< PMU STATUS REGISTER */
#define PMU_GPU_IDLE_STATE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PMU_DP_BLANK(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PMU_EVENTO(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PMU_L2C_STOPPED(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define PMU_STANDBYWFE(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define PMU_STANDBYWFI(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define PMU_EVENTI(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)


#define PMU_GPU_IDLE_STATE_GET(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define PMU_DP_BLANK_GET(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define PMU_EVENTO_GET(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define PMU_L2C_STOPPED_GET(val) \
	GET_REGISTER_VALUE(val, 3, 1)
#define PMU_STANDBYWFE_GET(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define PMU_STANDBYWFI_GET(val) \
	GET_REGISTER_VALUE(val, 5, 1)
#define PMU_EVENTI_GET(val) \
	GET_REGISTER_VALUE(val, 6, 1)


/**< PMU STOP REQUEST REGISTER */
#define PMU_REQ_STOP_GPU(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PMU_REQ_STOP_PP(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PMU_REQ_STOP_IPCM(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PMU_REQ_STOP_IPCS(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define PMU_REQ_STOP_EP(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define PMU_REQ_STOP_BAYER(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define PMU_REQ_STOP_DP(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)

#define PMU_REQ_STOP_GPU_GET(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define PMU_REQ_STOP_PP_GET(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define PMU_REQ_STOP_IPCM_GET(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define PMU_REQ_STOP_IPCS_GET(val) \
	GET_REGISTER_VALUE(val, 3, 1)
#define PMU_REQ_STOP_EP_GET(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define PMU_REQ_STOP_BAYER_GET(val) \
	GET_REGISTER_VALUE(val, 5, 1)
#define PMU_REQ_STOP_DP_GET(val) \
	GET_REGISTER_VALUE(val, 6, 1)


/**< PMU STOP ACK REGISTER */
#define PMU_ACK_STOP_GPU(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PMU_ACK_STOP_PP(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PMU_ACK_STOP_IPCM(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PMU_ACK_STOP_IPCS(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define PMU_ACK_STOP_EP(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define PMU_ACK_STOP_BAYER(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define PMU_ACK_STOP_DP(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)

#define PMU_ACK_STOP_GPU_GET(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define PMU_ACK_STOP_PP_GET(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define PMU_ACK_STOP_IPCM_GET(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define PMU_ACK_STOP_IPCS_GET(val) \
	GET_REGISTER_VALUE(val, 3, 1)
#define PMU_ACK_STOP_EP_GET(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define PMU_ACK_STOP_BAYER_GET(val) \
	GET_REGISTER_VALUE(val, 5, 1)
#define PMU_ACK_STOP_DP_GET(val) \
	GET_REGISTER_VALUE(val, 6, 1)


/**< PMU MASK ACK REGISTER */
#define PMU_ACK_MASK_GPU(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PMU_ACK_MASK_PP(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PMU_ACK_MASK_IPCM(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PMU_ACK_MASK_IPCS(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define PMU_ACK_MASK_EP(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define PMU_ACK_MASK_BAYER(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define PMU_ACK_MASK_DP(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)

#define PMU_ACK_MASK_GPU_GET(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define PMU_ACK_MASK_PP_GET(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define PMU_ACK_MASK_IPCM_GET(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define PMU_ACK_MASK_IPCS_GET(val) \
	GET_REGISTER_VALUE(val, 3, 1)
#define PMU_ACK_MASK_EP_GET(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define PMU_ACK_MASK_BAYER_GET(val) \
	GET_REGISTER_VALUE(val, 5, 1)
#define PMU_ACK_MASK_DP_GET(val) \
	GET_REGISTER_VALUE(val, 6, 1)


/**< PMU REQUEST COUNT REGISTER */
#define PMU_REQ_CNT_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 20)

#define PMU_REQ_CNT_GET(val) \
	GET_REGISTER_VALUE(val, 0, 20)

/**< PMU CLOCK CHANGE REQUEST ENABLE REGISTER */
#define PMU_REQ_EN_PDMA(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 0)
#define PMU_REQ_EN_CMU(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)

#define PMU_REQ_EN_PDMA_GET(val) \
	GET_REGISTER_VALUE(val, 0, 0)
#define PMU_REQ_EN_CMU_GET(val) \
	GET_REGISTER_VALUE(val, 0, 1)


/**< PMU PRE/POST COUNT REQUEST ENABLE REGISTER */
#define PMU_PRE_CNT(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 16)
#define PMU_POST_CNT(val, x) \
	SET_REGISTER_VALUE(val, x, 18, 14)

#define PMU_PRE_CNT_GET(val) \
	GET_REGISTER_VALUE(val, 0, 16)
#define PMU_POST_CNT_GET(val) \
	GET_REGISTER_VALUE(val, 18, 14)


/**< PMU ISOEN ON/OFF REGISTER */
#define PMU_ISOEN_GPU(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PMU_ISOEN_PP(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PMU_ISOEN_IPCM(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PMU_ISOEN_IPCS(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define PMU_ISOEN_EP(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define PMU_ISOEN_BAYER(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define PMU_ISOEN_DP(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)
#define PMU_ISOEN_JPEG(val, x) \
	SET_REGISTER_VALUE(val, x, 7, 1)
#define PMU_ISOEN_CODEC(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)


#define PMU_ISOEN_GPU_GET(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define PMU_ISOEN_PP_GET(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define PMU_ISOEN_IPCM_GET(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define PMU_ISOEN_IPCS_GET(val) \
	GET_REGISTER_VALUE(val, 3, 1)
#define PMU_ISOEN_EP_GET(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define PMU_ISOEN_BAYER_GET(val) \
	GET_REGISTER_VALUE(val, 5, 1)
#define PMU_ISOEN_DP_GET(val) \
	GET_REGISTER_VALUE(val, 6, 1)
#define PMU_ISOEN_JPEG_GET(val) \
	GET_REGISTER_VALUE(val, 7, 1)
#define PMU_ISOEN_CODEC_GET(val) \
	GET_REGISTER_VALUE(val, 8, 1)


/**< PMU SCPRE ON/OFF REGISTER */
#define PMU_SCPRE_GPU(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PMU_SCPRE_PP(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PMU_SCPRE_IPCM(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PMU_SCPRE_IPCS(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define PMU_SCPRE_EP(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define PMU_SCPRE_BAYER(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define PMU_SCPRE_DP(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)
#define PMU_SCPRE_JPEG(val, x) \
	SET_REGISTER_VALUE(val, x, 7, 1)
#define PMU_SCPRE_CODEC(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)


#define PMU_SCPRE_GPU_GET(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define PMU_SCPRE_PP_GET(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define PMU_SCPRE_IPCM_GET(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define PMU_SCPRE_IPCS_GET(val) \
	GET_REGISTER_VALUE(val, 3, 1)
#define PMU_SCPRE_EP_GET(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define PMU_SCPRE_BAYER_GET(val) \
	GET_REGISTER_VALUE(val, 5, 1)
#define PMU_SCPRE_DP_GET(val) \
	GET_REGISTER_VALUE(val, 6, 1)
#define PMU_SCPRE_JPEG_GET(val) \
	GET_REGISTER_VALUE(val, 7, 1)
#define PMU_SCPRE_CODEC_GET(val) \
	GET_REGISTER_VALUE(val, 8, 1)

/**< PMU SCALL ON/OFF REGISTER */
#define PMU_SCALL_GPU(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PMU_SCALL_PP(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PMU_SCALL_IPCM(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PMU_SCALL_IPCS(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define PMU_SCALL_EP(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define PMU_SCALL_BAYER(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define PMU_SCALL_DP(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)
#define PMU_SCALL_JPEG(val, x) \
	SET_REGISTER_VALUE(val, x, 7, 1)
#define PMU_SCALL_CODEC(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)


#define PMU_SCALL_GPU_GET(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define PMU_SCALL_PP_GET(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define PMU_SCALL_IPCM_GET(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define PMU_SCALL_IPCS_GET(val) \
	GET_REGISTER_VALUE(val, 3, 1)
#define PMU_SCALL_EP_GET(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define PMU_SCALL_BAYER_GET(val) \
	GET_REGISTER_VALUE(val, 5, 1)
#define PMU_SCALL_DP_GET(val) \
	GET_REGISTER_VALUE(val, 6, 1)
#define PMU_SCALL_JPEG_GET(val) \
	GET_REGISTER_VALUE(val, 7, 1)
#define PMU_SCALL_CODEC_GET(val) \
	GET_REGISTER_VALUE(val, 8, 1)

/**< PMU SCACK ON/OFF REGISTER */
#define PMU_SCACK_GPU(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PMU_SCACK_PP(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PMU_SCACK_IPCM(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PMU_SCACK_IPCS(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define PMU_SCACK_EP(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define PMU_SCACK_BAYER(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define PMU_SCACK_DP(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)
#define PMU_SCACK_JPEG(val, x) \
	SET_REGISTER_VALUE(val, x, 7, 1)
#define PMU_SCACK_CODEC(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)


#define PMU_SCACK_GPU_GET(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define PMU_SCACK_PP_GET(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define PMU_SCACK_IPCM_GET(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define PMU_SCACK_IPCS_GET(val) \
	GET_REGISTER_VALUE(val, 3, 1)
#define PMU_SCACK_EP_GET(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define PMU_SCACK_BAYER_GET(val) \
	GET_REGISTER_VALUE(val, 5, 1)
#define PMU_SCACK_DP_GET(val) \
	GET_REGISTER_VALUE(val, 6, 1)
#define PMU_SCACK_JPEG_GET(val) \
	GET_REGISTER_VALUE(val, 7, 1)
#define PMU_SCACK_CODEC_GET(val) \
	GET_REGISTER_VALUE(val, 8, 1)


/**< PMU USER DATA REGISTER */
#define PMU_USER_DATA0_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 32)
#define PMU_USER_DATA1_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 32)
#define PMU_USER_DATA2_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 32)
#define PMU_USER_DATA3_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 32)
#define PMU_USER_DATA4_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 32)
#define PMU_USER_DATA5_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 32)
#define PMU_USER_DATA6_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 32)
#define PMU_USER_DATA7_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 32)

#define PMU_USER_DATA0_GET(val) \
	GET_REGISTER_VALUE(val, 0, 32)
#define PMU_USER_DATA1_GET(val) \
	GET_REGISTER_VALUE(val, 0, 32)
#define PMU_USER_DATA2_GET(val) \
	GET_REGISTER_VALUE(val, 0, 32)
#define PMU_USER_DATA3_GET(val) \
	GET_REGISTER_VALUE(val, 0, 32)
#define PMU_USER_DATA4_GET(val) \
	GET_REGISTER_VALUE(val, 0, 32)
#define PMU_USER_DATA5_GET(val) \
	GET_REGISTER_VALUE(val, 0, 32)
#define PMU_USER_DATA6_GET(val) \
	GET_REGISTER_VALUE(val, 0, 32)
#define PMU_USER_DATA7_GET(val) \
	GET_REGISTER_VALUE(val, 0, 32)

#endif /* __DRIME4_PMU_REGS_H */

