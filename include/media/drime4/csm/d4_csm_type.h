/**
 * @file d4_csm_type.h
 * @brief DRIMe4 CSM(Capture Sequence Manager) Structure & Enumeration Define
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __DRIME4_K_CSM_TYPE_H__
#define __DRIME4_K_CSM_TYPE_H__

/******************************************************************************/
/*                                Enumeration                                 */
/******************************************************************************/
enum csm_cmd_list {
	CSM_CMD_CHG_WAIT_VD_MODE,
	CSM_CMD_CHG_READ_OUT_MODE,
	CSM_CMD_MAX
};

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/
struct csm_wdma_state {
	unsigned int wdma_error;
	unsigned int wdma_done_intr;
};

struct csm_spi_data {
	unsigned int ch;
	unsigned int addr;
	unsigned int data;
};

struct csm_chg_wait_vd_mode {
	unsigned int ref_vsync;	/* 0 : vsync falling , 1 : user defined, ... */
	struct csm_spi_data sensor_spi_data[2];
};

struct csm_chg_read_out_mode {
	unsigned int ref_vsync;	/* 0 : vsync falling , 1 : user defined, ... */
	unsigned int d4c_enable;
	struct csm_spi_data d4c_spi_data[11];
	struct csm_spi_data sensor_spi_data[2];
};

#endif   /* __DRIME4_K_CSM_TYPE_H__ */

