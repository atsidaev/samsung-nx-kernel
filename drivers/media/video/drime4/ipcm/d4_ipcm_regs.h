/**
 * @file	d4_ipcm_regs.h
 * @brief	IPCM Register base define file for Samsung DRIMeVI camera
 *		interface driver
 *
 * @author	TaeWook Nam <tw.nam@samsung.com>,
 *
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_K_IPCM_REGS_H__
#define __D4_K_IPCM_REGS_H__

#include <mach/d4_reg_macro.h>

/*
 ///////////////////////// K_IPCS Top Register Offset Define ////////////////////
 */
#define IPCM_K_MD_TOP_REG_OFFSET		(0x600)
#define IPCM_K_LDCM_TOP_REG_OFFSET		(0x700)
#define IPCM_K_LDCM_CORE_REG_OFFSET		(0x800)

#define IPCM_K_BASE			(0x0)/*0x50060000(BaseAddress)*/
#define IPCM_K_TOP_BASE			(IPCM_K_BASE)
/*
 ////////////////////////////////////////////////////////////////////////////////
*/


/*
 *
 * IPCM MD Interrupt Register Define
 *
 *
 */

/* INT_STATUS */
#define D4_IPCM_MD_MV_END_ALL(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_IPCM_MD_GMV_END_X(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_IPCM_MD_GMV_END_Y(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_IPCM_MD_RMV_END(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 1)

/* INT_ENABLE */
#define D4_IPCM_MD_INT_SRC_GMV_END(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_IPCM_MD_INT_SRC_RMV_END(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 1)

/* IPC TOP CONTROL, 0 */
#define D4_IPCM_K_TOP_CONTROL_INPUT_PP(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_IPCM_K_TOP_CONTROL_INPUT_PP_PACK(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_IPCM_K_TOP_CONTROL_TOGGLE(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_IPCM_K_TOP_CONTROL_DELAY(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define D4_IPCM_K_TOP_CONTROL_INPUT_PP_CH(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 2)
#define D4_IPCM_K_TOP_CONTROL_IPCM_WDMA_MODE_MAIN(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_IPCM_K_TOP_CONTROL_IPCM_WDMA_MODE_RSZ(val, x) \
	SET_REGISTER_VALUE(val, x, 17, 1)
#define D4_IPCM_K_TOP_CONTROL_IPCM_WDMA_MODE_SRSZ(val, x) \
	SET_REGISTER_VALUE(val, x, 18, 1)
#define D4_IPCM_K_TOP_CONTROL_IPCM_BYPASS(val, x) \
	SET_REGISTER_VALUE(val, x, 31, 1)

/* VDMA Control */
#define D4_IPCM_K_DMACTRL_PP_CON_WDMA0(val, x) \
	SET_REGISTER_VALUE(val, x, 20, 1)
#define D4_IPCM_K_DMACTRL_PP_CON_WDMA1(val, x) \
	SET_REGISTER_VALUE(val, x, 21, 1)
#define D4_IPCM_K_DMACTRL_PP_CON_WDMA2(val, x) \
	SET_REGISTER_VALUE(val, x, 22, 1)

#define D4_IPCM_K_DMACTRL_PP_CON_WDMA0_GET(val) \
	GET_REGISTER_VALUE(val, 20, 1)
#define D4_IPCM_K_DMACTRL_PP_CON_WDMA1_GET(val) \
	GET_REGISTER_VALUE(val, 21, 1)
#define D4_IPCM_K_DMACTRL_PP_CON_WDMA2_GET(val) \
	GET_REGISTER_VALUE(val, 22, 1)

#endif /* __D4_K_IPCS_REGS_H__ */

