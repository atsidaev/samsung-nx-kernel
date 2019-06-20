/**
 * @file d4_csm_ioctl.h
 * @brief DRIMe4 CSM(Capture Sequence Manager) IOCTL Interface Header File
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_CSM_IOCTL_H__
#define __D4_CSM_IOCTL_H__

#include <media/drime4/csm/d4_csm_type.h>

#define CSM_MAGIC 'c'


/******************************************************************************/
/** 			CSM Control Function 												 ***/
/******************************************************************************/

#define CSM_IOCTL_CHANGE_STILL_WAIT_VD_MODE	_IOWR(CSM_MAGIC, 1, struct csm_chg_wait_vd_mode)
#define CSM_IOCTL_CHANGE_STILL_READ_OUT_MODE	_IOWR(CSM_MAGIC, 2, struct csm_chg_read_out_mode)
#define CSM_IOCTL_GET_CSM_WDMA_STATE			_IOWR(CSM_MAGIC, 3, struct csm_wdma_state)

#endif /* __D4_CSM_IOCTL_H__ */
