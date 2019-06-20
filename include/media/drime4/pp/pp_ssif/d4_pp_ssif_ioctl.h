/**
 * @file d4_pp_ssif_ioctl.h
 * @brief DRIMe4 Sensor Interface Ioctl Define
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
 #ifndef __DRIME4_PP_SSIF_IOCTL_CMD_H__
#define __DRIME4_PP_SSIF_IOCTL_CMD_H__

#include "d4_pp_ssif_type.h"

#define PP_SSIF_MAGIC	 's'


#define PP_SSIF_TEST								_IO(PP_SSIF_MAGIC, 0)

#define PP_SSIF_IOCTL_OPEN_IRQ 						_IO(PP_SSIF_MAGIC, 1)
#define PP_SSIF_IOCTL_CLOSE_IRQ 					_IO(PP_SSIF_MAGIC, 2)

#define PP_SSIF_IOCTL_WAIT_INT						_IOW(PP_SSIF_MAGIC, 10, enum ssif_dd_interrupt_selection)

#define PP_SSIF_IOCTL_SET_SENSOR_CLOCK				_IOW(PP_SSIF_MAGIC, 20, unsigned int)
#define PP_SSIF_IOCTL_CHECK_FPS						_IO(PP_SSIF_MAGIC, 21)

#define PP_SSIF_IOCTL_SET_DELAY					_IOW(PP_SSIF_MAGIC, 50, unsigned int)

#endif   /* __DRIME4_PP_SSIF_IOCTL_CMD_H__ */

