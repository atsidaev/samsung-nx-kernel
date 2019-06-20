/**
 * @file d4_pp_3a_common_dd.h
 * @brief DRIMe4 PP 3A Common Device Driver Internal Header
 * @author Kyounghwan Moon <kh.moon@samsung.com>,
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _PP_3A_COMMON_DD_H_
#define _PP_3A_COMMON_DD_H_

#include "d4_pp_3a_regs.h"
#include "d4_pp_3a_if.h"

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/

struct pp_3a_reg_ctrl_base_info {
	struct device *dev_info;
	int irq_num;
};

#ifdef __cplusplus
extern "C" {
#endif

/**< 내부 함수 */
void pp_3a_request_irq(void);
void pp_3a_free_irq(void);
void pp_3a_set_basic_info(unsigned int reg_base, unsigned int mem_base, int irq_num);
void pp_3a_set_dma_interrupt_state_clean(void);
int pp_3a_get_result_address_type(void);
void pp_3a_set_result_mem_cache_flush(int area);
inline unsigned int read_pp_3a_register(unsigned int offset);
inline void write_pp_3a_register(unsigned int offset, unsigned int val);

#ifdef __cplusplus
}
#endif

#endif /* _PP_3A_COMMON_DD_H_ */

