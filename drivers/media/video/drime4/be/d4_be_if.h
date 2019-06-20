/**
 * @file d4_be_if.h
 * @brief DRIMe4 BE Interface / Control Header File
 * @author Niladri Mukherjee <n.mukherjee@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_BE_IF_H_
#define D4_BE_IF_H_


#include <media/drime4/be/d4_be_type.h>


#ifdef __cplusplus
extern "C" {
#endif

/*=================================================
 * Bayer TOP Registration Functions
 ==================================================*/
void be_get_phys_reg_info(enum be_reg_selection selection,
		   struct be_phys_reg_info *reg_info);
void be_set_clock(unsigned int clock_set);
BE_RESULT be_enable_irq(void);
void be_deinit_irq(void);
void be_init_intr_compeletion(void);
BE_RESULT be_wait_core_intr_timeout(enum be_interrupt_level intr, unsigned long timeout);
BE_RESULT be_wait_dma_intr_timeout(enum be_dma_interrupt_level intr, unsigned long timeout);
void be_init_ip_intr_compeletion(void);
void be_init_dma_intr_compeletion(void);
void be_init_an_ip_intr_compeletion(enum be_interrupt_level intr);
void be_init_a_dma_intr_compeletion(enum be_dma_interrupt_level intr);
void be_mutex_lock(void);
void be_mutex_unlock(void);
void be_top_isr_register_clbk(enum be_interrupt_level int_num, void(*callback)(void));
void be_dma_register_isr_clbk_obj(enum be_dma_interrupt_level int_num,	void(*pfunc)(void));
void be_dma_deregister_isr_clbk(enum be_dma_interrupt_level int_num);
void be_top_isr_deregister_clbk(enum be_interrupt_level int_num);
void be_set_clock_freq(unsigned int clock_set);
void be_pmu_on_off(enum be_pmu_type pmu_type);

/*=================================================
 * ISR Registration with Callback Function
 ==================================================*/
void be_kblend_register_isr_clbk(unsigned int cmd);
void be_kghost_fchk_register_isr_clbk(unsigned int cmd);
void be_ksg_register_isr_clbk(unsigned int cmd);
void be_snr_register_kisr_clbk(void);
void be_nr_register_fwt(void);
void be_nr_register_iwt(void);
void be_nr_register_strip(void);
void be_nr_block_fiwlet(void);

/*==============================================================
 * Wait for Completion Event Function and IWT Operation Function
 ===============================================================*/
BE_RESULT be_kblend_wait_for_completion(unsigned int cmd, int timeout);
BE_RESULT be_kghost_kfchk_wait_for_result(unsigned int cmd, int timeout);
BE_RESULT be_sg_wait_for_kcompletion(unsigned int cmd, int timeout);
BE_RESULT be_nr_wait_for_kcompletion(unsigned int cmd, int timeout);
BE_RESULT be_kwlet_iwt(struct be_nr_iwt_info *info);

void be_3dme_ktest(void);


int be_pmu_requeset(void);
void be_pmu_clear(void);
#ifdef __cplusplus
}
#endif


#endif /* D4_BE_IF_H_ */

