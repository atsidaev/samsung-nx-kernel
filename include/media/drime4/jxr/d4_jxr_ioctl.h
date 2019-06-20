/**
 * @file d4_jxr_ioctl.h
 * @brief DRIMe4 JPEG-XR IOCTL Interface Header File
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "d4_jxr_type.h"

#ifndef __D4_JXR_IOCTL_H__
#define __D4_JXR_IOCTL_H__

#define JXR_MAGIC 'x'

struct JXR_SEMA_INFO {
	unsigned long timeOut;
	signed int result;
};


#define JXR_ENABLE_IRQ 				_IOW(JXR_MAGIC, 1, signed int)
#define JXR_DISABLE_IRQ 				_IO(JXR_MAGIC, 2)
#define JXR_INIT_COMPLETION		_IO(JXR_MAGIC, 3)
#define JXR_SET_CLOCK				_IOW(JXR_MAGIC, 4, unsigned int)
#define JXR_REGISTER_INT			_IOWR(JXR_MAGIC, 5, signed int)
#define JXR_DEREGISTER_INT		_IOR(JXR_MAGIC, 6, signed int)
#define JXR_SEMAPHORE 				_IOWR(JXR_MAGIC, 7, struct JXR_SEMA_INFO)
#define JXR_GET_PHYS_REG_INFO	_IOWR(JXR_MAGIC, 8, struct JXR_PHYS_REG_INFO)

#endif /* __D4_JXR_IOCTL_H_ */
