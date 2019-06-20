/**
 * @file	d4_ipcs_intr_ctrl.c
 * @brief	IPCS_K interrupt control and set driver file for Samsung DRIMe4 Camera
 * 			 Interface driver
 *
 * @author	Dongjin Jung <djin81.jung@samsung.com>
 *
 * Copyright (c) 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/interrupt.h>

#include "d4_ipcs_intr_ctrl.h"

static void (*ipcs_intr_table[IPCS_K_INTR_MAX])(void) = { 0, };

extern int ipcs_k_irq_num;

struct completion ipcs_wdma0_completion;
struct completion ipcs_wdma1_completion;
struct completion ipcs_wdma2_completion;
struct completion ipcs_wdma3_completion;

/************************************************************************************/

static irqreturn_t ipcs_k_interrupt_handler(void);

/**
 * @brief   IPCS의 ISR handler
 * @fn      static irqreturn_t ipcs_k_interrupt_handler(void)
 * @param   void
 * @return  static irqreturn_t
 *
 * @author  Dongjin Jung
 * @note
 */
static irqreturn_t ipcs_k_interrupt_handler(void)
{
	volatile unsigned int intr_num;
	volatile unsigned int status;
	volatile unsigned int enable;

	status = READ_IPCS_K_REG(1);
	enable = READ_IPCS_K_REG(2);

	WRITE_IPCS_K_REG(2, 0);/* disable all ipcs interrupt */

	for (intr_num = 0; intr_num < IPCS_K_INTR_MAX; intr_num++) {
		if (status & (1 << intr_num)) {
			/* interrupt clear */
			WRITE_IPCS_K_REG(3, (0x0000 | (1 << intr_num)));

			if (ipcs_intr_table[intr_num])
				ipcs_intr_table[intr_num]();
		}
	}
	WRITE_IPCS_K_REG(2, enable);

	return IRQ_HANDLED;
}

/**
 * @brief   IPCS interrupt register initialization function
 * @fn      enum ipcs_op_error_id d4_ipcs_interrupt_init(void)
 * @param   void
 * @return  ipcs_error_id
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
int ipcs_k_interrupt_init(void)
{
	volatile unsigned int intr_num;
	volatile int irq_err;

	/*printk("Function : %s\n", __FUNCTION__);*/

	/*printk(KERN_INFO "\n d4_ipcs_interrtupt_init::ipcs_irq_num : %d", ipcs_irq_num);*/
	irq_err = request_irq(ipcs_k_irq_num, (void *) ipcs_k_interrupt_handler,
			IRQF_DISABLED, "d4_ipcs", NULL);
	/*printk(KERN_INFO "IPCS_K IRQ Return value : %d \n", irq_flags);*/

	if (irq_err == -EBUSY) {/* EBUSY : 16 */
		/*printk(KERN_INFO "IPCS_K IRQ is already register\n");*/
	} else if (irq_err < 0) {
		printk(KERN_INFO "IPCS IRQ failed\n");
		return -1;
	}

	for (intr_num = 0; intr_num < IPCS_K_INTR_MAX; intr_num++) {
		ipcs_intr_table[intr_num] = 0;
	}
	return 0;
}

/**
 * @brief   IPCS interrupt free function
 * @fn      void ipcs_k_interrupt_free(void)
 * @param   void
 * @return  ipcs_error_id
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_interrupt_free(void)
{
	free_irq(ipcs_k_irq_num, NULL);
}

/**
 * @brief   IPCS interrupt clear function
 * @fn      void ipcs_k_interrupt_clear(enum ipcs_k_interrupt intr)
 * @param   intr	[in] Interrupt
 * @return  void
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_interrupt_clear(enum ipcs_k_interrupt intr)
{
	WRITE_IPCS_K_REG(3, 1 << intr);
}

/**
 * @brief   IPCS interrupt enable function
 * @fn      void ipcs_k_interrupt_enable(enum ipcs_k_interrupt intr,
 *  void(*callback_func)(void))
 * @param   intr	[in] Interrupt
 * @param   callback_func	[in] Register call back function
 * @return  void
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_interrupt_enable(enum ipcs_k_interrupt intr, void(*callback_func)(
		void))
{
	volatile unsigned int enable;

	/*printk("Function: %s, Intr: %d\n", __FUNCTION__, intr);*/

	enable = READ_IPCS_K_REG(2);

	enable |= (1 << intr);

	WRITE_IPCS_K_REG(2, enable);

	/* call back function */
	ipcs_intr_table[intr] = callback_func;

	if (intr == IPCS_K_INTR_BUSEND_WDMA0) {
		init_completion(&ipcs_wdma0_completion);
	} else if (intr == IPCS_K_INTR_BUSEND_WDMA1) {
		init_completion(&ipcs_wdma1_completion);
	} else if (intr == IPCS_K_INTR_BUSEND_WDMA2) {
		init_completion(&ipcs_wdma2_completion);
	} else if (intr == IPCS_K_INTR_BUSEND_WDMA3) {
		init_completion(&ipcs_wdma3_completion);
	}

	/* interrupt clear */
	ipcs_k_interrupt_clear(intr);
}

/**
 * @brief   IPCS interrupt disable function
 * @fn      void ipcs_k_interrupt_disable(enum ipcs_k_interrupt intr)
 * @param   intr	[in] Interrupt
 * @return  void
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_interrupt_disable(enum ipcs_k_interrupt intr)
{
	volatile unsigned int disable;

	/*printk("Function: %s, Intr: %d\n", __FUNCTION__, intr);*/

	disable = READ_IPCS_K_REG(2);

	disable &= ~(disable & (1 << intr));

	WRITE_IPCS_K_REG(2, disable);

	ipcs_intr_table[intr] = 0;

	/* interrupt clear */
	ipcs_k_interrupt_clear(intr);
}

#ifndef IPCS_TIMEOUT_CHECK
/**
 * @brief   IPCS WDMA0 wait for completion function
 * @fn      void ipcs_k_wdma0_wait_done(void)
 * @param   void
 * @return  void
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_wdma0_wait_done(void)
{
	/*printk("Function: %s\n", __FUNCTION__);*/
	wait_for_completion(&ipcs_wdma0_completion);
}

/**
 * @brief   IPCS WDMA1 wait for completion function
 * @fn      void ipcs_k_wdma0_wait_done(void)
 * @param   void
 * @return  void
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_wdma1_wait_done(void)
{
	/*printk("Function: %s\n", __FUNCTION__);*/
	wait_for_completion(&ipcs_wdma1_completion);
}

/**
 * @brief   IPCS WDMA2 wait for completion function
 * @fn      void ipcs_k_wdma0_wait_done(void)
 * @param   void
 * @return  void
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_wdma2_wait_done(void)
{
	/*printk("Function: %s\n", __FUNCTION__);*/
	wait_for_completion(&ipcs_wdma2_completion);
}

/**
 * @brief   IPCS WDMA3 wait for completion function
 * @fn      void ipcs_k_wdma0_wait_done(void)
 * @param   void
 * @return  void
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_wdma3_wait_done(void)
{
	/*printk("Function: %s\n", __FUNCTION__);*/
	wait_for_completion(&ipcs_wdma3_completion);
}

#else

/**
 * @brief   IPCS WDMA0 wait for completion function
 * @fn      int ipcs_k_wdma0_wait_for_completion(unsigned long timeout)
 * @param   timeout [in] mssec 단위가 아니다.
 * @return  error is -1
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
int ipcs_k_wdma0_wait_for_completion(unsigned long timeout)
{
	if (!wait_for_completion_timeout(&ipcs_wdma0_completion, timeout)) {
		printk("%s timeout \n", __FUNCTION__);
		return -1;
	}
	return 0;
}

/**
 * @brief   IPCS WDMA1 wait for completion function
 * @fn      int ipcs_k_wdma0_wait_for_completion(unsigned long timeout)
 * @param   timeout [in] mssec 단위가 아니다.
 * @return  error is -1
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
int ipcs_k_wdma1_wait_for_completion(unsigned long timeout)
{
	if (!wait_for_completion_timeout(&ipcs_wdma1_completion, timeout)) {
		printk("%s timeout \n", __FUNCTION__);
		return -1;
	}
	return 0;
}

/**
 * @brief   IPCS WDMA2 wait for completion function
 * @fn      int ipcs_k_wdma0_wait_for_completion(unsigned long timeout)
 * @param   timeout [in] mssec 단위가 아니다.
 * @return  error is -1
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
int ipcs_k_wdma2_wait_for_completion(unsigned long timeout)
{
	if (!wait_for_completion_timeout(&ipcs_wdma2_completion, timeout)) {
		printk("%s timeout \n", __FUNCTION__);
		return -1;
	}
	return 0;
}

/**
 * @brief   IPCS WDMA3 wait for completion function
 * @fn      int ipcs_k_wdma0_wait_for_completion(unsigned long timeout)
 * @param   timeout [in] mssec 단위가 아니다.
 * @return  error is -1
 *
 * @author  Dongjin Jung
 * @note	kernel layer
 */
int ipcs_k_wdma3_wait_for_completion(unsigned long timeout)
{
	if (!wait_for_completion_timeout(&ipcs_wdma3_completion, timeout)) {
		printk("%s timeout \n", __FUNCTION__);
		return -1;
	}
	return 0;
}

#endif

/*********************************************************************/
