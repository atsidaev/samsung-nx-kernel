/**
* @file d4_mipi_if.h
* @brief DRIMe4 MIPI Interface header
* @author Gunwoo Nam <gunwoo.nam@samsung.com>
* 2011 Samsung Electronics
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#ifndef __MIPI_INTERFACE_H__
#define __MIPI_INTERFACE_H__

#include  <media/drime4/mipi/d4_mipi_type.h>


#ifdef __cplusplus
extern "C" {
#endif

MIPI_RESULT d4_MIPI_CSIM_Get_OpSema(int timeout);
MIPI_RESULT d4_MIPI_CSIS_Get_OpSema(int timeout);
MIPI_RESULT d4_Mipi_Csim_Enable_IRQ(void);
MIPI_RESULT d4_Mipi_Csis_Enable_IRQ(void);
void d4_Mipi_Csim_Deinit_IntNum(void);
void d4_Mipi_Csis_Deinit_IntNum(void);
MIPI_RESULT d4_Mipi_Csim_Set_Int(enum MIPI_CSIM_INT_SRC_SEL enIntNum, void (*pfnIsr)(void));
MIPI_RESULT d4_Mipi_Csis_Set_Int(enum MIPI_CSIS_INT_SRC_SEL enIntNum, void (*pfnIsr)(void));
MIPI_RESULT d4_Mipi_Csim_Mask_Int(enum MIPI_CSIM_INT_SRC_SEL IntNum);
MIPI_RESULT d4_Mipi_Csis_Mask_Int(enum MIPI_CSIS_INT_SRC_SEL IntNum);
MIPI_RESULT d4_Mipi_Csim_register_interrupt(void);
MIPI_RESULT d4_Mipi_Csis_register_interrupt(void);
MIPI_RESULT d4_Mipi_Csim_deregister_interrupt(void);
MIPI_RESULT d4_Mipi_Csis_deregister_interrupt(void);
void d4_Mipi_Csim_Init_Completion(void);
void d4_Mipi_Csis_Init_Completion(void);
void mipi_get_phys_reg_info(unsigned int *reg_info);


#ifdef __cplusplus
}
#endif

#endif /* __MIPI_INTERFACE_H__ */

