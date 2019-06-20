/**
 * @file int_str_ctrl.h
 * @brief flash Interface
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _INT_STR_H
#define _INT_STR_H

#include <linux/types.h>

#define INT_STR_MAGIC 'S'

#define INT_STR_SET_PIN				_IOW(INT_STR_MAGIC, 1, unsigned int) /*direction set*/
#define INT_STR_SET_INT				_IO(INT_STR_MAGIC, 2) /*int set*/
#define INT_STR_SET_CHARGE		_IOR(INT_STR_MAGIC, 3, unsigned int) /*charge*/
#define INT_STR_TRG						_IO(INT_STR_MAGIC, 4) /*start*/
#define INT_STR_TRG_READY			_IO(INT_STR_MAGIC, 5) /*start*/
#define INT_STR_UP							_IO(INT_STR_MAGIC, 6) /*start*/
#define INT_STR_UP_CHECK			_IOR(INT_STR_MAGIC, 7, unsigned int) /*start*/
#endif /* _INT_STR_H */

