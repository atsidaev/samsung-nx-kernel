 /**
 * @file d4_opener_ctrl_dd.h
 * @brief DRIMe4 OPENER Device Driver Header
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __OPENER_CTRL_DD_H__
#define __OPENER_CTRL_DD_H__

#include  <media/drime4/opener/d4_opener_type.h>

#ifdef __cplusplus
extern "C" {
#endif

void d4_opener_set_dev_info(struct device *info);
void d4_opener_init_mutex(void);
int d4_get_kdd_open_pid(struct kdd_open_pid_list *pid_list);


#ifdef __cplusplus
}
#endif


#endif /* __OPENER_CTRL_DD_H__ */
