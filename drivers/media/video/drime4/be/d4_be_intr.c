/**
 * @file d4_be_intr.c
 * @brief DRIMe4 BE Interrupt Control
 * @author Niladri Mukherjee <n.mukherjee@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
*/
/*
#include <mach/be/d4_be.h>
#include "d4_be_top.h"
#include "d4_be_regs.h"
*/
#include "d4_be_if.h"
#include "d4_be_top.h"


/**< Register Base Address */
extern unsigned int be_top_reg_base;
/**< IRQ Number */
extern unsigned int be_irq_num;


static void (*BE_ISR_TABLE[BE_INT_LEVEL_MAX])(void) = { 0, };
static void (*BE_DMA_ISR_Table[BE_DMA_INT_LEVEL_MAX])(void) = { 0, };


static irqreturn_t be_isr_callback(int irq, void *dev_id)
{
	int i, status = 0;
	int mask = 0;

	BE_KERN_DEBUG_MSG ("[D4/BE]----- BE ISR RECEIVED -----\n");

	/* get status */
	status = __raw_readl(be_top_reg_base + BE_TOP_INT_STATUS_IP);

	for (i = 0; i < BE_INT_LEVEL_MAX; i++) {
		if (status & (1 << i)) {
			if(i == BE_INT_LEVEL_FME /*|| BE_INT_LEVEL_3DME_EVERY_FIN*/ || BE_INT_LEVEL_3DME_ALL_FIN) {
				be_wakeup_core_intr(i); 
			}
			if (BE_ISR_TABLE[i] != NULL)
				BE_ISR_TABLE[i]();

			mask |= (1 << i);
			__raw_writel(mask, be_top_reg_base + BE_TOP_INT_CLEAR_IP);
			mask &= ~(1 << i);
			__raw_writel(mask, be_top_reg_base + BE_TOP_INT_CLEAR_IP);
		}
	}
	return IRQ_HANDLED;
}

/**
 * @brief bayer top enable irq method
 * @fn BE_RESULT be_enable_irq(void)
 * @param  struct bayer_top_params
 *
 * @return BE_RESULT : BE_SUCCESS <br>
 * 													BE_ERR_IRQ_REGISTER_FAIL<br>
 *
 * @author Niladri Mukherjee
 * @note  - Enable Interrupt of Bayer Engine IP <br>
 *		  - The Method is a part of Driver Init Sequence <br>
 *		    hence should better be avoided calling directly <br>
 */
BE_RESULT be_enable_irq(void)
{
	int i = 0;
	BE_RESULT result = D4_BE_SUCCESS;

	BE_KERN_DEBUG_MSG ("[D4/BE]----- BE ENABLE IRQ -----\n");

	__raw_writel(1, be_top_reg_base + BE_TOP_INT_ENABLE_IP);

	/* disable all interrupt */
	__raw_writel(0, be_top_reg_base + BE_TOP_INT_ENABLE_IP);
	__raw_writel(0, be_top_reg_base + BE_TOP_INT_ENABLE_DMA);

	/**< Top Interrupt Registeration */
	BE_KERN_DEBUG_MSG(" [D4/BE]--- ENABLE IRQ Bayer IRQ No = %d ---\n", be_irq_num);

	if (request_irq(be_irq_num,
			(void *) be_isr_callback,
			IRQF_DISABLED,
			BE_MODULE_NAME,
			NULL)) {
			BE_KERN_DEBUG_MSG("[D4/BE]--- Failed request_irq ---\n");
			result = D4_BE_ERR_IRQ_REGISTER_FAIL;
	}

	/* init callback function table */
	for (i = 0; i < BE_INT_LEVEL_MAX; i++)
		BE_ISR_TABLE[i] = 0;
	return result;

}

/**
 * @brief free irq resource
 * @fn   be_deinit_int_num(void)
 * @param void
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note NONE
 */
void be_deinit_irq(void)
{
	free_irq(be_irq_num, NULL);
}

/**
 * @brief Bayer Engine Top IRQ registration from each individual IP
 * @fn void be_top_isr_register(enum be_interrupt_level int_num)
 * @param  enum be_interrupt_level int_num
 *
 * @return void <br>
 *
 * @author Niladri Mukherjee
 * @note  - Each individual IP block should call this method to register <br>
 *		  - their INTERRUPT with BAYER TOP enum be_interrupt_level int_num <br>
 *		    indicates the INTERRUPT NUMBER on which the IP would be waiting <br>
 *		    There is no callback handling in this function <br>
 */
void be_top_isr_register(enum be_interrupt_level int_num)
{
	unsigned int intr_mask;
	/*Enable the interrupt*/
	intr_mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_ENABLE_IP);
	WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_ENABLE_IP,
			(intr_mask |= (1<<int_num)));

	/*Clear the interrupt*/
	intr_mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_IP);
	WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_IP,
			(intr_mask |= (1<<int_num)));

	/*Clear the interrupt*/
	intr_mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_IP);
	WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_IP,
			(intr_mask &= ~(1<<int_num)));
}

/**
 * @brief Bayer Engine Top IRQ registration from each individual IP
 * @fn void be_top_isr_register_clbk(enum be_interrupt_level int_num, void(*callback)(void))
 * @param  enum be_interrupt_level int_num, void(*callback)(void)
 *
 * @return void <br>
 * @author Niladri Mukherjee
 * @note  - Each individual IP block should call this method to register <br>
 *		  - their INTERRUPT with BAYER TOP enum be_interrupt_level int_num <br>
 *		    indicates the INTERRUPT NUMBER on which the IP would be waiting <br>
 *		    void(*callback)(void) indicates the callback function for the IP <br>
 */
void be_top_isr_register_clbk(enum be_interrupt_level int_num, void(*callback)(void))
{
	unsigned int intr_mask = 0;
	unsigned int clear_mask = 0;
	/*Enable the interrupt*/
	BE_KERN_DEBUG_MSG("[%s %d] INTERRUPT NUM = %d\n", __func__, __LINE__, int_num);
	intr_mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_ENABLE_IP);
	BE_KERN_DEBUG_MSG("[%s %d] INTERRUPT MASK = %x\n", __func__, __LINE__, intr_mask);
	WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_ENABLE_IP,
			(intr_mask |= (1<<int_num)));

	/*Clear the interrupt*/
	clear_mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_IP);
	WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_IP,
			(clear_mask |= (1<<int_num)));

	/*Clear the interrupt*/
	clear_mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_IP);
	WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_IP,
			(clear_mask &= ~(1<<int_num)));

	BE_ISR_TABLE[int_num] = callback;
}

void be_dma_ISR(void)
{
	int i, status, clear;
	status = READ_BE_REG(be_top_reg_base, D4_BE_TOP_INT_STATUS_DMA);

	for (i = 0; i < BE_DMA_INT_LEVEL_MAX; i++) {
		if (status & (0x1 << i)) {
			/*enable*/
			clear = status | (0x1 << i);
			writel(clear, be_top_reg_base + BE_TOP_INT_CLEAR_DMA);
			/*disable*/
			clear = status & ~(0x1 << i);
			writel(clear, be_top_reg_base + BE_TOP_INT_CLEAR_DMA);
		}
	}
}

/**
 * @brief Bayer Engine Top IRQ de-registration from each individual IP
 * @fn void be_top_isr_deregister(enum be_interrupt_level interrupt_number)
 * @param  enum be_interrupt_level interrupt_number
 *
 * @return enum d4_be_error_code <br>
 *
 * @author Niladri Mukherjee
 * @note  - Each individual IP block should call this method to de-register <br>
 *		  - their INTERRUPT with BAYER TOP, enum be_interrupt_level interrupt_number <br>
 *		    indicates the INTERRUPT NUMBER the IP would use to de-register <br>
 *		    - THIS METHOD CAN GET OBSOLETE LATER <br>
 */
void be_top_isr_deregister(enum be_interrupt_level interrupt_number)
{

	unsigned int mask = 0;
	mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_ENABLE_IP);
	mask = mask & ~(1 << interrupt_number);
	WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_ENABLE_IP, mask);

}

/**
 * @brief Bayer Engine Top IRQ de-registration from each individual IP
 * @fn void be_top_isr_deregister_clbk(enum be_interrupt_level int_num)
 * @param  enum be_interrupt_level int_num
 *
 * @return void <br>
 *
 * @author Niladri Mukherjee
 * @note  - Each individual IP block should call this method to de-register <br>
 *		  - their INTERRUPT with BAYER TOP, enum be_interrupt_level int_num <br>
 *		    indicates the INTERRUPT NUMBER the IP would use to de-register <br>
 *		    -THIS METHOD ALSO NULLIFIES ISR TABLE , SHOULD BE USED WITH CALLBACK <br>
 *		    -INTERRUPT REGISTRATION FUNCTION, <br>
 */
void be_top_isr_deregister_clbk(enum be_interrupt_level int_num)
{
	unsigned int intr_mask = 0;
	BE_ISR_TABLE[int_num] = NULL;
	intr_mask &= ~(1 << int_num);
	WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_ENABLE_IP, intr_mask);
}

/**
 * @brief be_dma_isr_clbk method
 * @fn void be_dma_isr_clbk(void)
 * @param  void
 *
 * @return enum d4_be_error_code error code <br>
 *
 * @author Niladri Mukherjee
 * @note  - Enable DMA ISR of Bayer Engine IP <br>
 */
void __be_dma_isr_clbk(void)
{
	int i, status, mask, new_mask;

	status = READ_BE_REG(be_top_reg_base , BE_TOP_INT_STATUS_DMA);

	for (i = 0; i < BE_DMA_INT_LEVEL_MAX; i++) {
		if ((status >> i) & 0x1) {
			if (BE_DMA_ISR_Table[i] != NULL)
				BE_DMA_ISR_Table[i]();
			mask = READ_BE_REG(be_top_reg_base , BE_TOP_INT_CLEAR_DMA);
			WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_DMA,	(mask |= (1<<i)));
			new_mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_STATUS_DMA);
			WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_DMA,	(new_mask &= ~(1<<i)));
		}
	}
}

/**
 * @brief Bayer Engine DMA IRQ registration from each individual IP
 * @fn void be_dma_register_isr_clbk_obj(enum be_dma_interrupt_level int_num, void (*pfunc)(void))
 * @param  enum be_dma_interrupt_level int_num, void (*pfunc)(void)
 *
 * @return void <br>
 *
 * @author Niladri Mukherjee
 * @note  - Individual IP block can call this method to register <br>
 *		  - their INTERRUPT with BAYER DMA enum be_dma_interrupt_level int_num <br>
 *		    indicates the INTERRUPT NUMBER on which the IP would be waiting for DMA <br>
 *		    void (*pfunc)(void) indicates the callback function takes no argument <br>
 */
void be_dma_register_isr_clbk_obj(enum be_dma_interrupt_level int_num,
		void(*pfunc)(void))
{
	unsigned int mask;
	mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_ENABLE_DMA);
	/*Enable the interrupt*/
	WRITE_BE_REG (be_top_reg_base, BE_TOP_INT_ENABLE_DMA,
			(mask |= (1<<int_num)));
	mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_DMA);
	/*Clear the interrupt*/
	WRITE_BE_REG (be_top_reg_base, BE_TOP_INT_CLEAR_DMA,
			(mask |= (1<<int_num)));
	/*Clear the interrupt*/
	mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_DMA);
	WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_CLEAR_DMA,
			(mask &= ~(1<<int_num)));

	BE_DMA_ISR_Table[int_num] = (void *) pfunc;
}


/**
 * @brief Bayer DMA IRQ de-registration from each individual IP
 * @fn void be_dma_deregister_isr_clbk(enum be_dma_interrupt_level int_num)
 * @param  enum be_dma_interrupt_level int_num
 *
 * @return void <br>
 *
 * @author Niladri Mukherjee
 * @note  - Each individual IP block should call this method to de-register <br>
 *		  - their INTERRUPT with BAYER DMA enum be_dma_interrupt_level int_num <br>
 *		    indicates the INTERRUPT NUMBER on which the IP would be waiting <br>
 *		    The method also De-Initialises the DMA ISR TABLE <br>
 */
void be_dma_deregister_isr_clbk(enum be_dma_interrupt_level int_num)
{
	unsigned int mask;
	mask = READ_BE_REG(be_top_reg_base, BE_TOP_INT_ENABLE_DMA);

	WRITE_BE_REG(be_top_reg_base, BE_TOP_INT_ENABLE_DMA,
			(mask &= ~(1<<int_num)));

	BE_DMA_ISR_Table[int_num] = NULL;
}


MODULE_AUTHOR("Niladri Mukherjee <n.mukherjee@samsung.com>");
MODULE_DESCRIPTION("Samsung Drime IV BE Driver");
MODULE_LICENSE("GPL");
