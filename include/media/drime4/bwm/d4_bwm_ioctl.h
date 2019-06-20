/**
 * @file d4_bwm_ioctl.h
 * @brief DRIMe4 BWM(Bandwidth Manager) IOCTL Interface Header File
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_BWM_IOCTL_H__
#define __D4_BWM_IOCTL_H__

#include <media/drime4/bwm/d4_bwm_type.h>

#define BWM_MAGIC 'm'

/**
 * @struct	ipcs_k_physical_reg_info
 * @brief	Physical register information
 */
struct bwm_physical_reg_info {
	unsigned int sonics_bus_ctrl_reg_start_addr;
	unsigned int sonics_bus_ctrl_reg_size;
	unsigned int drex_ctrl_reg_start_addr;
	unsigned int drex_ctrl_reg_size;
};

/******************************************************************************/
/** 													BWM Control Function 												 ***/
/******************************************************************************/

#define BWM_IOCTL_GET_PHYS_REG_INFO	_IOR(BWM_MAGIC, 1, struct bwm_physical_reg_info)

#endif /* __D4_BWM_IOCTL_H__ */
