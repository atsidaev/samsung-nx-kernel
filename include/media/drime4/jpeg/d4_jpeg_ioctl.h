/**
 * @file d4_jpeg_ioctl.h
 * @brief DRIMe4 JPEG IOCTL Interface Header File
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "d4_jpeg_type.h"

#ifndef __D4_JPEG_IOCTL_H__
#define __D4_JPEG_IOCTL_H__

#define JPEG_MAGIC 'j'

struct JPEG_IOCTL_SEMA_INFO {
	unsigned long timeOut;
	signed int result;
};

#define JPEG_IOCTL_OPEN_IRQ 				_IO(JPEG_MAGIC, 1)
#define JPEG_IOCTL_CLOSE_IRQ 				_IO(JPEG_MAGIC, 2)
#define JPEG_IOCTL_INIT_COMPLETION		_IO(JPEG_MAGIC, 3)
#define JPEG_IOCTL_SET_CLOCK				_IOW(JPEG_MAGIC, 4, unsigned int)
#define JPEG_IOCTL_REGISTER_INT			_IOR(JPEG_MAGIC, 5, signed int)
#define JPEG_IOCTL_DEREGISTER_INT			_IOR(JPEG_MAGIC, 6, signed int)
#define JPEG_IOCTL_SEMAPHORE 				_IOWR(JPEG_MAGIC, 7, struct JPEG_IOCTL_SEMA_INFO)
#define JPEG_IOCTL_GET_PHYS_REG_INFO	_IOWR(JPEG_MAGIC, 8, struct JPEG_GET_REG_INFO)
#define JPEG_IOCTL_CHECK_ENC_STATUS		_IOR(JPEG_MAGIC, 9, signed int)

#endif /* __D4_JPEG_IOCTL_H__ */
