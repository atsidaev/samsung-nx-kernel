 /**
 * @file d4_csm_if.h
 * @brief DRIMe4 CSM Device Driver Header
 * @author Gwon Haesu <haesu.gwon@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __CSM_CTRL_DD_H__
#define __CSM_CTRL_DD_H__

#include  <media/drime4/csm/d4_csm_type.h>

#ifdef __cplusplus
extern "C" {
#endif

struct csm_wdma_state* csm_get_wdma_error_state(void);
void csm_init(void);
void csm_deregister_vsync_isr(enum csm_cmd_list cmd);
int csm_register_vsync_isr(enum csm_cmd_list cmd, void *data);

#ifdef __cplusplus
}
#endif

#endif /* __CSM_CTRL_DD_H__ */
