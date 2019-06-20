/**
 * @file	d4_utility_ioctl.h
 * @brief	Utility IOCTL file for Samsung DRIMe VI camera interface driver
 *
 * @author
 * Copyright (c) 2013 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_UTILITY_IOCTL_CMD_H__
#define __D4_UTILITY_IOCTL_CMD_H__

#define D4_UTILITY_MAGIC	 'u'

#define D4_UTILITY_GET_BV 			_IOR(D4_UTILITY_MAGIC, 1, unsigned int)
/*#define D4_UTILITY_GET_MIPI_IC _IO(D4_UTILITY_MAGIC, 2)*/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif   /* __D4_UTILITY_IOCTL_CMD_H__ */


