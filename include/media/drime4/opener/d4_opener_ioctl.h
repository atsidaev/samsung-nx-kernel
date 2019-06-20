/**
 * @file d4_opener_ioctl.h
 * @brief DRIMe4 OPENER IOCTL Interface Header File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "d4_opener_type.h"

#ifndef __D4_OPENER_IOCTL_H__
#define __D4_OPENER_IOCTL_H__

#define OPENER_MAGIC 'b'

#define OPENER_IOCTL_GET_OPEN_CNT			_IOW(OPENER_MAGIC, 10, enum kdd_devices)
#define OPENER_IOCTL_RESET_OPEN_CNT			_IOW(OPENER_MAGIC, 11, enum kdd_devices)
#define OPENER_IOCTL_PRINT_PID_LIST			_IOW(OPENER_MAGIC, 12, enum kdd_devices)
#define OPENER_IOCTL_GET_PID_LIST			_IOW(OPENER_MAGIC, 13, struct kdd_open_pid_list)

enum kdd_open_flag d4_kdd_open(enum kdd_devices dev);
enum kdd_close_flag d4_kdd_close(enum kdd_devices dev);
int d4_get_kdd_open_count(enum kdd_devices dev);
void d4_reset_kdd_open_count(enum kdd_devices dev);
void d4_print_kdd_open_pid_list(enum kdd_devices dev);

#endif /* __D4_OPENER_IOCTL_H__ */
