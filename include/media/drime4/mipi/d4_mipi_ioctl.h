/**
 * @file d4_mipi_ioctl.h
 * @brief DRIMe4 MIPI Interface Ioctl Define
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __DRIME4_MIPI_IOCTL_CMD_H__
#define __DRIME4_MIPI_IOCTL_CMD_H__

#include "d4_mipi_type.h"

#define MIPI_MAGIC	 'm'

#define MIPI_IOCTL_GET_PHYS_REG_INFO _IOR(MIPI_MAGIC, 0, unsigned int)
#define MIPI_IOCTL_CSIM_OPEN_IRQ _IO(MIPI_MAGIC, 1)
#define MIPI_IOCTL_CSIS_OPEN_IRQ _IO(MIPI_MAGIC, 2)
#define MIPI_IOCTL_CSIM_CLOSE_IRQ _IO(MIPI_MAGIC, 3)
#define MIPI_IOCTL_CSIS_CLOSE_IRQ _IO(MIPI_MAGIC, 4)
#define MIPI_IOCTL_CSIM_INIT_COMPLETION _IO(MIPI_MAGIC, 5)
#define MIPI_IOCTL_CSIS_INIT_COMPLETION _IO(MIPI_MAGIC, 6)
#define MIPI_IOCTL_CSIM_REGISTER_INT _IOR(MIPI_MAGIC, 7, unsigned int)
#define MIPI_IOCTL_CSIS_REGISTER_INT _IOR(MIPI_MAGIC, 8, unsigned int)
#define MIPI_IOCTL_CSIM_DEREGISTER_INT _IOR(MIPI_MAGIC, 9, unsigned int)
#define MIPI_IOCTL_CSIS_DEREGISTER_INT _IOR(MIPI_MAGIC, 10, unsigned int)
#define MIPI_IOCTL_CSIM_SEMAPHORE _IOR(MIPI_MAGIC, 11, unsigned int)
#define MIPI_IOCTL_CSIS_SEMAPHORE _IOR(MIPI_MAGIC, 12, unsigned int)

/* MURU - ADDED FOR PDMA-DPC */
#define MIPI_IOCTL_SET_DPC_PDMA   _IOR(MIPI_MAGIC, 13, unsigned int)
#define MIPI_IOCTL_START_DPC_PDMA   _IOR(MIPI_MAGIC, 14, unsigned int)
#define MIPI_IOCTL_STOP_DPC_PDMA   _IOR(MIPI_MAGIC, 15, unsigned int)

#endif /* __DRIME4_MIPI_IOCTL_CMD_H__ */
