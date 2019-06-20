/**
 * @file d4_mipi_ctrl_dd.c
 * @brief DRIMe4 MIPI register control Function File
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/completion.h>
#include <linux/io.h>

#include "d4_mipi_ctrl_dd.h"

/**< Completion for synchronization */
static struct completion MipiCsimOpCompletion;
static struct completion MipiCsisOpCompletion;

/**< MIPI Device Info */
struct device	*mipi_dev;

/**< Physical register information */
static unsigned int mipi_phys_reg_info;

/**< Register Base Address */
static unsigned int mipi_con_reg_base;
static unsigned int mipi_csim_reg_base;
static unsigned int mipi_csis_reg_base;

/**< IRQ Number */
static unsigned int mipi_csim_irq_num;
static unsigned int mipi_csis_irq_num;

#define d4_MipiReadReg(RegAddr)		(__raw_readl(RegAddr))
#define d4_MipiWriteReg(RegAddr, Data)	(__raw_writel(Data, RegAddr))

/******************************** GLOBALS **************************************/
static void (*MIPI_CSIM_ISR_Table[MIPI_CSIM_INT_ALL])(void) = {
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL
};

static void (*MIPI_CSIS_ISR_Table[MIPI_CSIS_INT_ALL])(void) = {
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL,
   MIPI_NULL
};

/**
 * @brief Write MIPI CON register
 * @fn write_Mipi_Con_Register(const unsigned int offset, const  unsigned int val)
 * @param const unsigned int offset, const  unsigned int val
 * @return No
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void write_Mipi_Con_Register(const unsigned int offset, const  unsigned int val)
{
   d4_MipiWriteReg((mipi_con_reg_base + offset), val);
}

/**
 * @brief Read MIPI CON register
 * @fn read_Mipi_Con_Register(const unsigned int offset)
 * @param const unsigned int offset
 * @return unsigned int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
unsigned int read_Mipi_Con_Register(const unsigned int offset)
{
   return d4_MipiReadReg(mipi_con_reg_base + offset);
}

/**
 * @brief Write MIPI CSIM register
 * @fn write_Mipi_Csim_Register(const unsigned int offset, const  unsigned int val)
 * @param const unsigned int offset, const  unsigned int val
 * @return No
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void write_Mipi_Csim_Register(const unsigned int offset, const unsigned int val)
{
   d4_MipiWriteReg((mipi_csim_reg_base + offset), val);
}

/**
 * @brief Read MIPI CSIM register
 * @fn read_Mipi_Csim_Register(const unsigned int offset)
 * @param const unsigned int offset
 * @return unsigned int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
unsigned int read_Mipi_Csim_Register(const unsigned int offset)
{
   return d4_MipiReadReg(mipi_csim_reg_base + offset);
}

/**
 * @brief Write MIPI CSIS register
 * @fn write_Mipi_Csis_Register(const unsigned int offset, const  unsigned int val)
 * @param const unsigned int offset, const  unsigned int val
 * @return No
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void write_Mipi_Csis_Register(const unsigned int offset, const unsigned int val)
{
   d4_MipiWriteReg((mipi_csis_reg_base + offset), val);
}

/**
 * @brief Read MIPI CSIS register
 * @fn read_Mipi_Csis_Register(const unsigned int offset)
 * @param const unsigned int offset
 * @return unsigned int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
unsigned int read_Mipi_Csis_Register(const unsigned int offset)
{
   return d4_MipiReadReg(mipi_csis_reg_base + offset);
}

/**
 * @brief get operatoin semaphore
 * @fn   d4_MIPI_CSIM_Get_OpSema(int timeout)
 * @param int timeout
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  MIPI CSIM Interrupt
 */
MIPI_RESULT d4_MIPI_CSIM_Get_OpSema(int timeout)
{
	MIPI_RESULT semStatus = MIPI_CSIM_SUCCESS;

	semStatus = wait_for_completion_timeout(&MipiCsimOpCompletion, timeout);

	return semStatus;
}

/**
 * @brief get operatoin semaphore
 * @fn   d4_MIPI_CSIS_Get_OpSema(int timeout)
 * @param int timeout
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  MIPI CSIS Interrupt
 */
MIPI_RESULT d4_MIPI_CSIS_Get_OpSema(int timeout)
{
	MIPI_RESULT semStatus = MIPI_CSIM_SUCCESS;

	semStatus = wait_for_completion_timeout(&MipiCsisOpCompletion, timeout);

	return semStatus;
}

/**
 * @brief set MIPI TX isr function
 * @fn d4_Mipi_Csim_IRQ(int irq, void *dev_id)
 * @param int irq, void *dev_id
 * @return irqreturn_t
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
irqreturn_t d4_Mipi_Csim_IRQ(int irq, void *dev_id)
{
   int cnt;

    unsigned int status = read_Mipi_Csim_Register(MIPI_CSIM_INT_SRC);
   for (cnt = 0; cnt < MIPI_CSIM_INT_ALL; cnt++) {
	if (status & (1 << cnt)) {
		/* Call the respective ISR */
		if (MIPI_CSIM_ISR_Table[cnt])
			cnt[MIPI_CSIM_ISR_Table](); /* Just Trick */

		/* Clear the Interrupt Status */
		write_Mipi_Csim_Register(MIPI_CSIM_INT_SRC, (1 << cnt));
	}
   }

    /* Clear all MIPI CSIM interrupts in INT controller */
   return IRQ_HANDLED;
}

/**
 * @brief set MIPI RX isr function
 * @fn d4_Mipi_Csis_IRQ(int irq, void *dev_id)
 * @param int irq, void *dev_id
 * @return irqreturn_t
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
irqreturn_t d4_Mipi_Csis_IRQ(int irq, void *dev_id)
{
   int cnt;

    unsigned int status = read_Mipi_Csis_Register(MIPI_CSIS_INT_SRC);

   for (cnt = 0; cnt < MIPI_CSIS_INT_ALL; cnt++) {
	if (status & (1 << cnt)) {
		/* Call the respective ISR */
		if (MIPI_CSIS_ISR_Table[cnt])
			cnt[MIPI_CSIS_ISR_Table](); /* Just Trick */

		/* Clear the Interrupt Status */
		write_Mipi_Csis_Register(MIPI_CSIS_INT_SRC, (1 << cnt));
	}
   }

    /* Clear all MIPI CSIS interrupts in INT controller */
   return IRQ_HANDLED;
}

/**
 * @brief Enable MIPI TX IRQ
 * @fn d4_Mipi_Csim_Enable_IRQ(void)
 * @param void
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
MIPI_RESULT d4_Mipi_Csim_Enable_IRQ(void)
{
	MIPI_RESULT result = MIPI_CSIM_SUCCESS;
	/**< Top Interrupt Registeration */
	if (request_irq(mipi_csim_irq_num, d4_Mipi_Csim_IRQ, IRQF_DISABLED, "d4_mipi_tx", mipi_dev)) {
		result = MIPI_CSIM_IRQ_FAIL;
		printk(KERN_DEBUG "====== d4_Mipi Csim IRQ failed ====== \n");
		return result;
	}
	d4_Mipi_Csim_Init_Completion();
	return result;
}

/**
 * @brief Enable MIPI RX IRQ
 * @fn d4_Mipi_Csis_Enable_IRQ(void)
 * @param void
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
MIPI_RESULT d4_Mipi_Csis_Enable_IRQ(void)
{
	MIPI_RESULT result = MIPI_CSIS_SUCCESS;

	/**< Top Interrupt Registeration */
	if (request_irq(mipi_csis_irq_num, d4_Mipi_Csis_IRQ, IRQF_DISABLED, "d4_mipi_rx", mipi_dev)) {
		result = MIPI_CSIS_IRQ_FAIL;
		printk(KERN_DEBUG "====== d4_Mipi Csis IRQ failed ====== \n");
		return result;
	}
	d4_Mipi_Csis_Init_Completion();
	return result;
}

/**
 * @brief free MIPI TX irq resource
 * @fn   d4_Mipi_Csim_Deinit_IntNum(void)
 * @param void
 * @return void
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void d4_Mipi_Csim_Deinit_IntNum(void)
{
	free_irq(mipi_csim_irq_num, mipi_dev);
}

/**
 * @brief free MIPI RX irq resource
 * @fn   d4_Mipi_Csis_Deinit_IntNum(void)
 * @param void
 * @return void
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void d4_Mipi_Csis_Deinit_IntNum(void)
{
	free_irq(mipi_csis_irq_num, mipi_dev);
}

/**
 * @brief Set MIPI CSIM block interrupt enable
 * @fn d4_Mipi_Csim_Set_Int(enum MIPI_CSIM_INT_SRC_SEL enIntNum, void (*pfnIsr)(void))
 * @param enum MIPI_CSIM_INT_SRC_SEL enIntNum, void (*pfnIsr)(void)
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
MIPI_RESULT d4_Mipi_Csim_Set_Int(enum MIPI_CSIM_INT_SRC_SEL enIntNum, void (*pfnIsr)(void))
{
	unsigned int reg = MIPI_INITIAL;
	MIPI_RESULT result = MIPI_CSIM_SUCCESS;

	if (pfnIsr == MIPI_NULL) {
		printk(KERN_DEBUG "===== INTERRUPT CallBackFunc Error =======\n");
		result = MIPI_CSIM_NULL_CALLBACK;
	}

	if (result == MIPI_CSIM_SUCCESS) {
		enIntNum[MIPI_CSIM_ISR_Table] = pfnIsr;

		reg = read_Mipi_Csim_Register(MIPI_CSIM_INT_MSK);

		if (enIntNum == MIPI_CSIM_INT_P_FULL)
			D4_MIPI_CSIM_INT_MSK_P_FULL(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIM_INT_H_FULL)
			D4_MIPI_CSIM_INT_MSK_H_FULL(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIM_INT_FRAME_DONE)
			D4_MIPI_CSIM_INT_MSK_FRAME_DONE(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIM_INT_SW_RST_RELEASE)
			D4_MIPI_CSIM_INT_MSK_SW_RST_REL(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIM_INT_PLL_STABLE)
			D4_MIPI_CSIM_INT_MSK_PLL_STABLE(reg, MIPI_INT_MSK_DISABLE);

		write_Mipi_Csim_Register(MIPI_CSIM_INT_MSK, reg);

	}

	return result;
}

/**
 * @brief Set MIPI CSIS block interrupt enable
 * @fn d4_Mipi_Csis_Set_Int(enum MIPI_CSIS_INT_SRC_SEL enIntNum, void (*pfnIsr)(void))
 * @param enum MIPI_CSIS_INT_SRC_SEL enIntNum, void (*pfnIsr)(void)
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
MIPI_RESULT d4_Mipi_Csis_Set_Int(enum MIPI_CSIS_INT_SRC_SEL enIntNum, void (*pfnIsr)(void))
{
	unsigned int reg = MIPI_INITIAL;
	MIPI_RESULT result = MIPI_CSIS_SUCCESS;

	if (pfnIsr == MIPI_NULL) {
		printk(KERN_DEBUG "===== INTERRUPT CallBackFunc Error =======\n");
		result = MIPI_CSIS_NULL_CALLBACK;
	}

	if (result == MIPI_CSIS_SUCCESS) {
		enIntNum[MIPI_CSIS_ISR_Table] = pfnIsr;

		reg = read_Mipi_Csis_Register(MIPI_CSIS_INT_MSK);

		if (enIntNum == MIPI_CSIS_INT_ERR_ID)
			D4_MIPI_CSIS_INT_MSK_ERR_ID(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIS_INT_ERR_CRC)
			D4_MIPI_CSIS_INT_MSK_ERR_CRC(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIS_INT_ERR_ECC)
			D4_MIPI_CSIS_INT_MSK_ERR_ECC(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIS_INT_ERR_OVER)
			D4_MIPI_CSIS_INT_MSK_ERR_OVER(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIS_INT_ERR_LOST_FE)
			D4_MIPI_CSIS_INT_MSK_ERR_LOST_FE(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIS_INT_ERR_LOST_FS)
			D4_MIPI_CSIS_INT_MSK_ERR_LOST_FS(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIS_INT_ERR_SOT_HS)
			D4_MIPI_CSIS_INT_MSK_ERR_SOT_HS(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIS_INT_ERR_ODD_AFTER)
			D4_MIPI_CSIS_INT_MSK_ODD_AFTER(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIS_INT_ERR_ODD_BEFORE)
			D4_MIPI_CSIS_INT_MSK_ODD_BEFORE(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIS_INT_ERR_EVEN_AFTER)
			D4_MIPI_CSIS_INT_MSK_EVEN_AFTER(reg, MIPI_INT_MSK_DISABLE);
		else if (enIntNum == MIPI_CSIS_INT_ERR_EVEN_BEFORE)
			D4_MIPI_CSIS_INT_MSK_EVEN_BEFORE(reg, MIPI_INT_MSK_DISABLE);

		write_Mipi_Csis_Register(MIPI_CSIS_INT_MSK, reg);

	}

	return result;
}

/**
 * @brief Set MIPI CSIS block interrupt disable
 * @fn d4_Mipi_Csis_Mask_Int(enum MIPI_CSIS_INT_SRC_SEL IntNum)
 * @param enum MIPI_CSIS_INT_SRC_SEL IntNum
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
MIPI_RESULT d4_Mipi_Csis_Mask_Int(enum MIPI_CSIS_INT_SRC_SEL IntNum)
{
	unsigned int value = MIPI_INITIAL;
	MIPI_RESULT result = MIPI_CSIS_SUCCESS;

	if (IntNum >= MIPI_CSIS_INT_ALL)
		result = MIPI_CSIS_INVALID_INT_NUM;

	if (result == MIPI_CSIS_SUCCESS) {
		MIPI_CSIS_ISR_Table[(int)IntNum] = MIPI_NULL;

		value = read_Mipi_Csis_Register(MIPI_CSIS_INT_MSK);

		if (IntNum == MIPI_CSIS_INT_ERR_ID)
			D4_MIPI_CSIS_INT_MSK_ERR_ID(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIS_INT_ERR_CRC)
			D4_MIPI_CSIS_INT_MSK_ERR_CRC(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIS_INT_ERR_ECC)
			D4_MIPI_CSIS_INT_MSK_ERR_ECC(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIS_INT_ERR_OVER)
			D4_MIPI_CSIS_INT_MSK_ERR_OVER(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIS_INT_ERR_LOST_FE)
			D4_MIPI_CSIS_INT_MSK_ERR_LOST_FE(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIS_INT_ERR_LOST_FS)
			D4_MIPI_CSIS_INT_MSK_ERR_LOST_FS(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIS_INT_ERR_SOT_HS)
			D4_MIPI_CSIS_INT_MSK_ERR_SOT_HS(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIS_INT_ERR_ODD_AFTER)
			D4_MIPI_CSIS_INT_MSK_ODD_AFTER(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIS_INT_ERR_ODD_BEFORE)
			D4_MIPI_CSIS_INT_MSK_ODD_BEFORE(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIS_INT_ERR_EVEN_AFTER)
			D4_MIPI_CSIS_INT_MSK_EVEN_AFTER(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIS_INT_ERR_EVEN_BEFORE)
			D4_MIPI_CSIS_INT_MSK_EVEN_BEFORE(value, MIPI_INT_MSK_ENABLE);

		write_Mipi_Csis_Register(MIPI_CSIS_INT_MSK, value);

	}

	return result;
}

/**
 * @brief Set MIPI CSIM block interrupt disable
 * @fn d4_Mipi_Csim_Mask_Int(enum MIPI_CSIM_INT_SRC_SEL IntNum)
 * @param enum MIPI_CSIM_INT_SRC_SEL IntNum
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
MIPI_RESULT d4_Mipi_Csim_Mask_Int(enum MIPI_CSIM_INT_SRC_SEL IntNum)
{
	unsigned int value = MIPI_INITIAL;
	MIPI_RESULT result = MIPI_CSIM_SUCCESS;

	if (IntNum >= MIPI_CSIM_INT_ALL)
		result = MIPI_CSIM_INVALID_INT_NUM;

	if (result == MIPI_CSIM_SUCCESS) {
		MIPI_CSIM_ISR_Table[(int)IntNum] = MIPI_NULL;

		value = read_Mipi_Csim_Register(MIPI_CSIM_INT_MSK);

		if (IntNum == MIPI_CSIM_INT_P_FULL)
			D4_MIPI_CSIM_INT_MSK_P_FULL(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIM_INT_H_FULL)
			D4_MIPI_CSIM_INT_MSK_H_FULL(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIM_INT_FRAME_DONE)
			D4_MIPI_CSIM_INT_MSK_FRAME_DONE(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIM_INT_SW_RST_RELEASE)
			D4_MIPI_CSIM_INT_MSK_SW_RST_REL(value, MIPI_INT_MSK_ENABLE);
		else if (IntNum == MIPI_CSIM_INT_PLL_STABLE)
			D4_MIPI_CSIM_INT_MSK_PLL_STABLE(value, MIPI_INT_MSK_ENABLE);

		write_Mipi_Csim_Register(MIPI_CSIM_INT_MSK, value);

	}

	return result;
}

/**
 * @brief Register MIPI CSIM block interrupt
 * @fn d4_Mipi_Csim_register_interrupt(void)
 * @param void
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
MIPI_RESULT d4_Mipi_Csim_register_interrupt(void)
{
	MIPI_RESULT result = MIPI_SUCCESS;
	unsigned int intNum;

	for (intNum = 0; intNum < MIPI_CSIM_INT_ALL; intNum++) {
		result = d4_Mipi_Csim_Set_Int((enum MIPI_CSIM_INT_SRC_SEL)intNum, d4_Mipi_Csim_Callback);

	if (result != MIPI_SUCCESS)
		break;
	}

	return result;
}

/**
 * @brief Register MIPI CSIS block interrupt
 * @fn d4_Mipi_Csis_register_interrupt(void)
 * @param void
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
MIPI_RESULT d4_Mipi_Csis_register_interrupt(void)
{
	MIPI_RESULT result = MIPI_SUCCESS;
	unsigned int intNum;

	for (intNum = 0; intNum < MIPI_CSIS_INT_ALL; intNum++) {
		result = d4_Mipi_Csis_Set_Int((enum MIPI_CSIS_INT_SRC_SEL)intNum, d4_Mipi_Csis_Callback);

	if (result != MIPI_SUCCESS)
		break;
	}

	return result;
}

/**
 * @brief Deregister MIPI CSIM block interrupt
 * @fn d4_Mipi_Csim_deregister_interrupt(void)
 * @param void
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
MIPI_RESULT d4_Mipi_Csim_deregister_interrupt(void)
{
	MIPI_RESULT result = MIPI_SUCCESS;
	unsigned int intNum;

	for (intNum = 0; intNum < MIPI_CSIM_INT_ALL; intNum++) {
		result = d4_Mipi_Csim_Mask_Int((enum MIPI_CSIM_INT_SRC_SEL)intNum);

	if (result != MIPI_SUCCESS)
		break;
	}

	return result;
}

/**
 * @brief Deregister MIPI CSIS block interrupt
 * @fn d4_Mipi_Csis_deregister_interrupt(void)
 * @param void
 * @return MIPI_RESULT
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
MIPI_RESULT d4_Mipi_Csis_deregister_interrupt(void)
{
	MIPI_RESULT result = MIPI_SUCCESS;
	unsigned int intNum;

	for (intNum = 0; intNum < MIPI_CSIS_INT_ALL; intNum++) {
		result = d4_Mipi_Csis_Mask_Int((enum MIPI_CSIS_INT_SRC_SEL)intNum);

	if (result != MIPI_SUCCESS)
		break;
	}

	return result;
}

/**
 * @brief Initialize Completion
 * @fn   d4_Mipi_Csim_Init_Completion(void)
 * @param void
 * @return void
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void d4_Mipi_Csim_Init_Completion(void)
{
	init_completion(&MipiCsimOpCompletion);
}

/**
 * @brief Initialize Completion
 * @fn   d4_Mipi_Csis_Init_Completion(void)
 * @param void
 * @return void
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void d4_Mipi_Csis_Init_Completion(void)
{
	init_completion(&MipiCsisOpCompletion);
}

/**
 * @brief release MIPI CSIM semaphore
 * @fn   d4_Mipi_Csim_Callback(void)
 * @param void
 * @return void
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void d4_Mipi_Csim_Callback(void)
{
   complete(&MipiCsimOpCompletion);
}

/**
 * @brief release MIPI CSIS semaphore
 * @fn   d4_Mipi_Csis_Callback(void)
 * @param void
 * @return void
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void d4_Mipi_Csis_Callback(void)
{
   complete(&MipiCsisOpCompletion);
}

/**
 * @brief receive MIPI Physical base address
 * @fn   mipi_get_phys_reg_info(unsigned int *reg_info)
 * @param reg_info
 * @return void
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void mipi_get_phys_reg_info(unsigned int *reg_info)
{
	*reg_info = mipi_phys_reg_info;
}


/**
 * @brief set resigter address
 * @fn   mipi_set_reg_ctrl_base_info(struct mipi_reg_ctrl_base_info *info)
 * @param struct mipi_reg_ctrl_base_info *info
 * @return void
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
void mipi_set_reg_ctrl_base_info(struct mipi_reg_ctrl_base_info *info)
{
	mipi_dev = info->dev_info;

	mipi_con_reg_base = info->con_reg_base;
	mipi_csim_reg_base = info->csim_reg_base;
	mipi_csis_reg_base = info->csis_reg_base;
	mipi_csim_irq_num = info->csim_irq_num;
	mipi_csis_irq_num = info->csis_irq_num;

	/**< Physical register information */
	mipi_phys_reg_info = info->mipi_phys_base_addr;
}
