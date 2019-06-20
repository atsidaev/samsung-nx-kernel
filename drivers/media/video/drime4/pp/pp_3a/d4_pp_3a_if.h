/**
 * @file d4_pp_3a_if.h
 * @brief DRIMe4 PP 3A Device Driver Interface Header
 * @author Kyounghwan Moon <kh.moon@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _PP_3A_INTERFACE_H_
#define _PP_3A_INTERFACE_H_

#include <media/drime4/pp/pp_3a/d4_pp_3a_type.h>

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

extern struct completion pp_3a_dma_completion;

/* PP 3A Common Control Function */

void pp_3a_request_irq(void);
void pp_3a_free_irq(void);

void pp_3a_wait_dma_done(void);
void pp_3a_set_result_mem_addr_info(void);
void pp_3a_disable_after_capture(void);


#ifdef __cplusplus
}
#endif

#endif /* _PP_3A_INTERFACE_H_ */

