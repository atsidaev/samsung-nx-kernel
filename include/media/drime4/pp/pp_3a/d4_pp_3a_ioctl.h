 /**
 * @file d4_pp_3a_ioctl.h
 * @brief DRIMe4 PP 3a Interface Ioctl Define
 * @author Kyounghwan Moon <kh.moon@samsung.com>,
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef __DRIME4_PP_3A_IOCTL_CMD_H__
#define __DRIME4_PP_3A_IOCTL_CMD_H__

#include "d4_pp_3a_type.h"

#define PP_3A_MAGIC	 'a'

#define PP_3A_IOCTL_OPEN_IRQ 						_IO(PP_3A_MAGIC, 1)
#define PP_3A_IOCTL_CLOSE_IRQ 						_IO(PP_3A_MAGIC, 2)

#define PP_3A_IOCTL_WAIT_DMA_DONE 					_IO(PP_3A_MAGIC, 10)
#define PP_3A_IOCTL_RESULT_MEM_SET 					_IO(PP_3A_MAGIC, 11)

#define PP_3A_IOCTL_3A_DISABLE_AFTER_CAPTURE 					_IO(PP_3A_MAGIC, 20)

#endif   /* __DRIME4_PP_3A_IOCTL_CMD_H__ */

