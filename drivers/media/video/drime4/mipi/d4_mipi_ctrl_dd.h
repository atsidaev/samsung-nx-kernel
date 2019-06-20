 /**
 * @file d4_mipi_ctrl_dd.h
 * @brief DRIMe4 MIPI register Control Device Driver Header
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __MIPI_CTRL_DD_H__
#define __MIPI_CTRL_DD_H__

#include  "d4_mipi_if.h"

/**
 * @struct	mipi_reg_ctrl_base_info
 * @brief	register resource member
 */
struct mipi_reg_ctrl_base_info{
   struct device *dev_info;
   unsigned int con_reg_base;    /**< CON Register - Virtual Base Address */
   unsigned int csim_reg_base;   /**< CSIM Register - Virtual Base Address */
   unsigned int csis_reg_base;   /**< CSIS Register - Virtual Base Address */
   int csim_irq_num;     /**< MIPI TX IRQ Number */
   int csis_irq_num;     /**< MIPI RX IRQ Number */

   unsigned int mipi_phys_base_addr;     /**< MIPI  - Physical register information */
};

#ifdef __cplusplus
extern "C" {
#endif
void write_Mipi_Con_Register(const unsigned int offset, const  unsigned int val);
unsigned int read_Mipi_Con_Register(const unsigned int offset);
void write_Mipi_Csim_Register(const unsigned int offset, const unsigned int val);
unsigned int read_Mipi_Csim_Register(const unsigned int offset);
void write_Mipi_Csis_Register(const unsigned int offset, const unsigned int val);
unsigned int read_Mipi_Csis_Register(const unsigned int offset);
MIPI_RESULT d4_MIPI_CSIM_Get_OpSema(int timeout);
MIPI_RESULT d4_MIPI_CSIS_Get_OpSema(int timeout);
irqreturn_t d4_Mipi_Csim_IRQ(int irq, void *dev_id);
irqreturn_t d4_Mipi_Csis_IRQ(int irq, void *dev_id);
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
void d4_Mipi_Csim_Callback(void);
void d4_Mipi_Csis_Callback(void);
void mipi_get_phys_reg_info(unsigned int *reg_info);
void mipi_set_reg_ctrl_base_info(struct mipi_reg_ctrl_base_info *info);

#ifdef __cplusplus
}
#endif

#endif /* __MIPI_CTRL_DD_H__ */

