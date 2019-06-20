/**
 * @file d4_jpeg_if.h
 * @brief DRIMe4 JPEG Interface header
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __JPEG_INTERFACE_H__
#define __JPEG_INTERFACE_H__

#include  <media/drime4/jpeg/d4_jpeg_type.h>

#ifdef __cplusplus
extern "C" {
#endif

signed int jpeg_register_interrupt(void);
signed int jpeg_deregister_interrupt(void);
signed int jpeg_enable_irq(void);
void jpeg_disable_irq(void);
void jpeg_init_completion(void);
signed int jpeg_get_operation_sema(const unsigned long timeout);
void jpeg_set_clock(const  unsigned int clock_set);
void jpeg_get_phys_reg_info(struct JPEG_GET_REG_INFO *regInfo);
signed int jpeg_checkEncodeStatus(void);
void jpeg_k_pmu_on_off(enum jpeg_k_on_off pmu_type);

#ifdef __cplusplus
}
#endif

#endif /* __JPEG_INTERFACE_H__ */
