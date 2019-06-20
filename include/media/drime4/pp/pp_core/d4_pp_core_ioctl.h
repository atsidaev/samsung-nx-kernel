/**
 * @file d4_pp_core_ioctl.h
 * @brief DRIMe4 PP Core Interface Ioctl Define
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __DRIME4_PP_CORE_IOCTL_CMD_H__
#define __DRIME4_PP_CORE_IOCTL_CMD_H__

#include "d4_pp_core_type.h"

#define PP_CORE_MAGIC	 't'


#define PP_CORE_IOCTL_OPEN_IRQ 						_IO(PP_CORE_MAGIC, 1)
#define PP_CORE_IOCTL_CLOSE_IRQ 					_IO(PP_CORE_MAGIC, 2)

#define PP_CORE_IOCTL_WAIT_LUT_LOAD_DONE 			_IO(PP_CORE_MAGIC, 10)
#define PP_CORE_IOCTL_WAIT_LUT_GENERATION_DONE 		_IO(PP_CORE_MAGIC, 11)
#define PP_CORE_IOCTL_WAIT_VFPN_DONE 				_IO(PP_CORE_MAGIC, 12)
#define PP_CORE_IOCTL_WAIT_WDMA_DONE 				_IO(PP_CORE_MAGIC, 13)
#define PP_CORE_IOCTL_WAIT_RDMA_DONE 				_IO(PP_CORE_MAGIC, 14)
#define PP_CORE_IOCTL_WAIT_INIT_WDMA 				_IO(PP_CORE_MAGIC, 15)
#define PP_CORE_IOCTL_WAIT_INIT_RDMA 				_IO(PP_CORE_MAGIC, 16)
#define PP_CORE_IOCTL_CORE_RESET	 				_IO(PP_CORE_MAGIC, 17)

#define PP_CORE_IOCTL_WAIT_LUT_LOAD_DONE_WITH_TIMEOUT	_IOW(PP_CORE_MAGIC, 18, unsigned int)
#define PP_CORE_IOCTL_WAIT_INIT_LUT_LOAD 			_IO(PP_CORE_MAGIC, 19)

#define MIPI_IOCTL_WAIT_WDMA_DONE 					_IO(PP_CORE_MAGIC, 37)
#define MIPI_IOCTL_WAIT_RDMA_DONE 					_IO(PP_CORE_MAGIC, 38)
#define MIPI_IOCTL_WAIT_WDMA_CALLBACK 				_IO(PP_CORE_MAGIC, 39)
#define MIPI_IOCTL_WAIT_WDMA_CALLBACK_CLEAR 				_IO(PP_CORE_MAGIC, 40)
#define MIPI_IOCTL_WAIT_RDMA_CALLBACK 				_IO(PP_CORE_MAGIC, 30)

#define PP_CORE_IOCTL_SET_PP_CLOCK					_IOW(PP_CORE_MAGIC, 51, unsigned int)
#define PP_CORE_IOCTL_PP_PMU_ONOFF					_IOW(PP_CORE_MAGIC, 52, enum pp_dd_onoff)

#define PP_CORE_IOCTL_GET_PHYS_REG_INFO				_IOR(PP_CORE_MAGIC, 63, unsigned int)

#endif   /* __DRIME4_PP_CORE_IOCTL_CMD_H__ */

