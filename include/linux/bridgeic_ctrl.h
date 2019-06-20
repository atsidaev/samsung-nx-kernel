/**
 * @file bridgeic_ctrl.h
 * @brief Bridge Interface
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _BRIDGE_IC_H
#define _BRIDGE_IC_H

#include <linux/types.h>

#define BIC_IOC_MAGIC 'B'

#define BIC_IOCTL_POWERON					_IO(BIC_IOC_MAGIC, 1)
#define BIC_IOCTL_POWEROFF				_IO(BIC_IOC_MAGIC, 2)

#endif /* _BRIDGE_IC_H */

