/**
 * @file d4_jxr.h
 * @brief DRIMe4 JPEG XR Interface
 * @author JinHyoung An<jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __JXR_IF_H__
#define __JXR_IF_H__

#include  <media/drime4/jxr/d4_jxr_type.h>

#ifdef __cplusplus
extern "C" {
#endif
signed int jxr_initIRQ(void);
void jxr_deInitIRQ(void);
void jxr_init_completion(void);
unsigned long jxr_getSema(unsigned int timeOut);
signed int jxr_registerInterrupt(void);
signed int jxr_deRegisterInterrupt(void);

signed int jxr_enableInt(enum JXR_INT IntNum, void (*pfnIsr)(void));
signed int jxr_disableInt(enum JXR_INT IntNum);
void jxr_callBack(void);
void jxr_get_phys_reg_info(struct JXR_PHYS_REG_INFO *regInfo);


#ifdef __cplusplus
}
#endif

#endif /* __JXR_IF_H__ */

