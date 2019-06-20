/*
 * include/linux/i2c-d4-ap2isp-master.h
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 * Authors:
 *	Jeongsup Jeong <jeongsup.jeong@samsung.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _I2C_D4_AP2ISP_MASTER_H
#define _I2C_D4_AP2ISP_MASTER_H

#include <linux/ioctl.h>


typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned long DWORD;

struct _packet {
	BYTE address;
	BYTE *data;
	DWORD length;
};


#define AP2ISP_MAJOR_NUM	90

#define IOCTL_MSG			_IO(AP2ISP_MAJOR_NUM, 1)
#define IOCTL_GET_RETURN	_IOW(AP2ISP_MAJOR_NUM, 2, int)
#define IOCTL_SET_SADDR		_IOW(AP2ISP_MAJOR_NUM, 3, unsigned int)
#define IOCTL_SET_MSG		_IOW(AP2ISP_MAJOR_NUM, 4, struct _packet)
#define IOCTL_GET_MSG		_IOR(AP2ISP_MAJOR_NUM, 5, struct _packet)

#define AP2ISP_DEVICE_NAME	"i2c-ap2isp-master"

#endif /* _I2C_D4_AP2ISP_MASTER_H */
