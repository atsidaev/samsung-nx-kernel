/**
 * @file d4_ipcm_intr.c
 * @brief DRIMe4 IPCM Driver File
 * @author TaeWook Nam <tw.@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/device.h>
#include <linux/interrupt.h>

#include <mach/ipcm/d4_ipcm.h>
#include "d4_ipcm_regs.h"
#include "d4_ipcm_ctrl.h"

static void (*IPCM_ISR_Table[IPCM_K_MAX_INTR_NUM])(void) = {0,};
static void (*IPCM_MD_ISR_Table[MD_K_MAX_INTR_NUM])(void) = {0,};
static void (*IPCM_LDCM_ISR_Table[LDCM_K_MAX_INTR])(enum ldcm_k_intr_flags) = {0,};

struct completion ipcm_wdma0_completion;
struct completion ipcm_wdma1_completion;
struct completion ipcm_wdma2_completion;

struct completion ipcm_md_gmv_completion;
struct completion ipcm_md_rmv_completion;

struct completion ipcm_ldcm_completion[LDCM_K_MAX_INTR];

extern struct device	*ipcm_dev;

extern int ipcm_k_dma_irq_num;
extern int ipcm_k_md_irq_num;
extern int ipcm_k_ldcm_irq_num;

/**
 * @brief   IPCM core WDMA의 Interrupt handler 함수
 * @fn      irqreturn_t ipcm_k_dma_int_handler(void)
 * @param   void
 * @return  void
 *
 * @author  Jangwon Yi
 * @note    각 인터럽트에 할당된 callback 함수를 호출한다.
 */
static irqreturn_t ipcm_k_dma_int_handler(void)
{
	unsigned int i;
	unsigned int status;
	unsigned int int_enable;

	status = READ_IPCM_K_TOP_REG(0x4); /* get status */
	int_enable = READ_IPCM_K_TOP_REG(0x8);

	WRITE_IPCM_K_TOP_REG(0x8, 0); /* diable all ipcm interrupt */

	for (i = 0; i < IPCM_K_MAX_INTR_NUM; i++) {
		if (status & (1 << i)) {
			WRITE_IPCM_K_TOP_REG(0xc, 1 << i); /* int clear */
			if (IPCM_ISR_Table[i])
				IPCM_ISR_Table[i]();
		}
	}

	WRITE_IPCM_K_TOP_REG(0x8, int_enable);

	return IRQ_HANDLED;
}

/**
 * @brief   IPCM System Interrupt를 등록 및 초기화하는 함수
 * @fn      int ipcm_k_dma_int_init(void)
 * @param   void
 * @return  error code
 *
 * @author  TaeWook Nam
 * @note
 */
int ipcm_k_dma_int_init(void)
{
	unsigned int i;
	int irq_err;

	WRITE_IPCM_K_TOP_REG(0x8, 0); /* disable all interrupt */
	WRITE_IPCM_K_TOP_REG(0xc, 0xffffffff); /* clear all interrupt */

	/**< Top Interrupt Registeration */
	irq_err = request_irq(ipcm_k_dma_irq_num, (void *)ipcm_k_dma_int_handler,
				IRQF_DISABLED, IPCM_MODULE_NAME, NULL);
	if (irq_err == -EBUSY) {/* EBUSY : 16 */
		/*printk(KERN_INFO "IPCM_K IRQ is already register\n");*/
	} else if (irq_err < 0) {
		printk(KERN_INFO "failed to IPCM request_irq\n");
		return -1;
	}

	/* init callback function table */
	for (i = 0; i < IPCM_K_MAX_INTR_NUM; i++)
		IPCM_ISR_Table[i] = 0;

	return 0;
}

/**
 * @brief   IPCM System Interrupt를 Disable 해주는 함수
 * @fn      void ipcm_k_dma_int_off(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note
 */
void ipcm_k_dma_int_off(void)
{
	WRITE_IPCM_K_TOP_REG(0x8, 0); /* disable all interrupt */
	WRITE_IPCM_K_TOP_REG(0xc, 0xffffffff); /* clear all interrupt */

	free_irq(ipcm_k_dma_irq_num, NULL);
}

/**
 * @brief   IPCM Sub Interrupt를 Clear하는 함수
 * @fn      void ipcm_k_dma_int_clear(enum ipcm_int_sig int_sig)
 * @param   int_sig [in] interrupt select(WDMA)
 * @return  void
 *
 * @author  TaeWook Nam
 * @note
 */
void ipcm_k_dma_int_clear(enum ipcm_k_int_sig int_sig)
{
	WRITE_IPCM_K_TOP_REG(0xc, 1 << int_sig); /* int clear */
}

/**
 * @brief   IPCM Sub Interrupt를 Enable하는 함수
 * @fn      int ipcm_k_dma_int_enable(enum ipcm_k_int_sig int_sig, void(*callback)(void))
 * @param   int_sig [in] interrupt select(WDMA)
 * @param   void [in] Call back function 등록
 * @return  error return
 *
 * @author  TaeWook Nam
 * @note
 */
int ipcm_k_dma_int_enable(enum ipcm_k_int_sig int_sig, void(*callback)(void))
{
	unsigned int intr_enable;

	if (!callback) {
		dev_dbg(ipcm_dev, "\n%s:Callback function pointer is NULL.\n", __func__);
		return IPCM_K_OP_PARAM_ERROR;
	}

	dev_dbg(ipcm_dev, "\n%s:enable %d interrupt signal.\n", __func__, int_sig);

	intr_enable = READ_IPCM_K_TOP_REG(0x8);
	WRITE_IPCM_K_TOP_REG(0x8, (intr_enable |= (1 << int_sig)));

	if (int_sig == IPCM_K_INTR_BUSEND_WDMA0) {
		init_completion(&ipcm_wdma0_completion);
	} else if (int_sig == IPCM_K_INTR_BUSEND_WDMA1) {
		init_completion(&ipcm_wdma1_completion);
	} else if (int_sig == IPCM_K_INTR_NLCE_DONE) {
		init_completion(&ipcm_wdma1_completion);
	} else if (int_sig == IPCM_K_INTR_BUSEND_WDMA2) {
		init_completion(&ipcm_wdma2_completion);
	}

	IPCM_ISR_Table[int_sig] = callback;

	ipcm_k_dma_int_clear(int_sig);

	return IPCM_K_OP_ERROR_NONE;
}

/**
 * @brief   IPCM Sub Interrupt를 Disable하는 함수
 * @fn      void ipcm_k_dma_int_disable(enum ipcm_k_int_sig int_sig)
 * @param   int_sig [in] interrupt select(WDMA)
 * @return  void
 *
 * @author  TaeWook Nam
 * @note
 */
void ipcm_k_dma_int_disable(enum ipcm_k_int_sig int_sig)
{
	unsigned int intr_enable;

	intr_enable = READ_IPCM_K_TOP_REG(0x8);
	WRITE_IPCM_K_TOP_REG(0x8, (intr_enable &= ~(1 << int_sig)));

	IPCM_ISR_Table[int_sig] = 0;
}

/**
 * @brief   IPCM Motion detector Interrupt handler 함수
 * @fn      irqreturn_t ipcm_k_md_int_handler(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note    각 인터럽트에 할당된 callback 함수를 호출한다.
 */
static irqreturn_t ipcm_k_md_int_handler(void)
{
	unsigned int gmv_end, rmv_end;
	unsigned int status;
	unsigned int int_enable;

	status = READ_IPCM_K_MD_REG(0x34); /* get status */
	int_enable = READ_IPCM_K_MD_REG(0x38);

	WRITE_IPCM_K_MD_REG(0x38, 0); /* diable all ipcm interrupt */

	gmv_end = status & 0x1;
	rmv_end = (status & 0x1000) >> 12;

	if (gmv_end) {
		D4_IPCM_MD_GMV_END_X(status, 0);
		D4_IPCM_MD_GMV_END_Y(status, 0);
		D4_IPCM_MD_MV_END_ALL(status, 0);

		WRITE_IPCM_K_MD_REG(0x34, status); /* int clear */

		if (IPCM_MD_ISR_Table[MD_K_INTR_GMV_END])
			IPCM_MD_ISR_Table[MD_K_INTR_GMV_END]();
	}

	if (rmv_end) {
		D4_IPCM_MD_MV_END_ALL(status, 0);
		D4_IPCM_MD_RMV_END(status, 0);

		WRITE_IPCM_K_MD_REG(0x34, status); /* int clear */

		if (IPCM_MD_ISR_Table[MD_K_INTR_RMV_END])
			IPCM_MD_ISR_Table[MD_K_INTR_RMV_END]();
	}

	WRITE_IPCM_K_MD_REG(0x38, int_enable);

	return IRQ_HANDLED;
}

/**
 * @brief   IPCM Motion detector Interrupt를 등록 및 초기화하는 함수
 * @fn      void ipcm_k_md_int_init(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note
 */
void ipcm_k_md_int_init(void)
{
	unsigned int i;

	WRITE_IPCM_K_MD_REG(0x38, 0); /* disable all interrupt */
	WRITE_IPCM_K_MD_REG(0x34, 0); /* clear all interrupt */

	/**< Top Interrupt Registeration */
	if (request_irq(ipcm_k_md_irq_num,
			(void *)ipcm_k_md_int_handler,
			IRQF_DISABLED,
			IPCM_MODULE_NAME,
			NULL)) {
		printk(KERN_INFO "failed to request_irq\n");
		return;
	}

	/* init callback function table */
	for (i = 0; i < MD_K_MAX_INTR_NUM; i++)
		IPCM_MD_ISR_Table[i] = 0;

}

/**
 * @brief   IPCM Motion detector Interrupt를 Disable하는 함수
 * @fn      void ipcm_k_md_int_off(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note
 */
void ipcm_k_md_int_off(void)
{
	WRITE_IPCM_K_MD_REG(0x38, 0); /* disable all interrupt */
	WRITE_IPCM_K_MD_REG(0x34, 0); /* clear all interrupt */

	free_irq(ipcm_k_md_irq_num, NULL);
}

/**
 * @brief   IPCM MD Interrupt를 Enable하는 함수
 * @fn      int ipcm_k_md_int_enable(enum md_k_int_sig int_sig, void(*callback)(void))
 * @param   int_sig [in] interrupt select
 * @param   void [in] Call back function 등록
 * @return  error return
 *
 * @author  TaeWook Nam
 * @note
 */
int ipcm_k_md_int_enable(enum md_k_int_sig int_sig, void(*callback)(void))
{
	unsigned int intr_enable;

	if (!callback) {
		dev_dbg(ipcm_dev, "\n%s:callback function pointer is NULL.\n", __func__);
		return IPCM_K_OP_PARAM_ERROR;
	}

	intr_enable = READ_IPCM_K_MD_REG(0x38);

	if (int_sig == MD_K_INTR_GMV_END) {
		init_completion(&ipcm_md_gmv_completion);
		D4_IPCM_MD_INT_SRC_GMV_END(intr_enable, 1);
	} else if (int_sig == MD_K_INTR_RMV_END) {
		init_completion(&ipcm_md_rmv_completion);
		D4_IPCM_MD_INT_SRC_RMV_END(intr_enable, 1);
	}

	WRITE_IPCM_K_MD_REG(0x34, 0); /* clear int */
	WRITE_IPCM_K_MD_REG(0x38, intr_enable);

	IPCM_MD_ISR_Table[int_sig] = callback;

	return IPCM_K_OP_ERROR_NONE;
}

/**
 * @brief   IPCM MD Interrupt를 Disable하는 함수
 * @fn      void ipcm_k_md_int_disable(enum md_k_int_sig int_sig)
 * @param   int_sig		[in] interrupt select
 * @return  void
 *
 * @author  TaeWook Nam
 * @note
 */
void ipcm_k_md_int_disable(enum md_k_int_sig int_sig)
{
	unsigned int intr_enable;

	intr_enable = READ_IPCM_K_MD_REG(0x38);

	if (int_sig == MD_K_INTR_GMV_END) {
		D4_IPCM_MD_INT_SRC_GMV_END(intr_enable, 0);
	} else if (int_sig == MD_K_INTR_RMV_END) {
		D4_IPCM_MD_INT_SRC_RMV_END(intr_enable, 0);
	}

	WRITE_IPCM_K_MD_REG(0x34, 0); /* clear int */
	WRITE_IPCM_K_MD_REG(0x38, intr_enable);

	IPCM_MD_ISR_Table[int_sig] = 0;
}

/**
 * @brief   IPCM Motion detector Interrupt handler 함수
 * @fn      irqreturn_t ipcm_k_ldcm_int_handler(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note    각 인터럽트에 할당된 callback 함수를 호출한다.
 */
static irqreturn_t ipcm_k_ldcm_int_handler(void)
{
	int i;
	unsigned int status, clear;
	unsigned int int_enable;

	status = READ_IPCM_K_LDCM_TOP_REG(0x20);
	int_enable = READ_IPCM_K_LDCM_TOP_REG(0x24);

	WRITE_IPCM_K_LDCM_TOP_REG(0x24, 0); /* diable all ldcm interrupt */

	for (i = 0; i < LDCM_K_MAX_INTR; i++) {
		if (status & (0x1 << i)) {
			clear = status | (0x1 << i);
			WRITE_IPCM_K_LDCM_TOP_REG(0x28, clear);

			/* printk("[IPCM LDCm IRQ Handler] Clear bit %d\n", i); */

			if (IPCM_LDCM_ISR_Table[i])
				IPCM_LDCM_ISR_Table[i](i);
		}
	}

	WRITE_IPCM_K_LDCM_TOP_REG(0x24, int_enable); /* enable ldcm interrupt */

	return IRQ_HANDLED;
}

/**
 * @brief   IPCM Motion detector Interrupt를 등록 및 초기화하는 함수
 * @fn      void ipcm_k_ldcm_int_init(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note
 */
void ipcm_k_ldcm_int_init(void)
{
	unsigned int i;

	WRITE_IPCM_K_LDCM_TOP_REG(0x24, 0); /* disable all interrupt */
	WRITE_IPCM_K_LDCM_TOP_REG(0x28, 0); /* clear all interrupt */

	/**< Top Interrupt Registeration */
	if (request_irq(ipcm_k_ldcm_irq_num,
			(void *)ipcm_k_ldcm_int_handler,
			IRQF_DISABLED,
			IPCM_MODULE_NAME,
			NULL)) {
		printk(KERN_INFO "failed to request_irq\n");
		return;
	}

	/* init callback function table */
	for (i = 0; i < LDCM_K_MAX_INTR; i++)
		IPCM_LDCM_ISR_Table[i] = 0;
}

/**
 * @brief   IPCM Motion detector Interrupt를 Disable하는 함수
 * @fn      void ipcm_k_ldcm_int_off(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note
 */
void ipcm_k_ldcm_int_off(void)
{
	WRITE_IPCM_K_LDCM_TOP_REG(0x24, 0); /* disable all interrupt */
	WRITE_IPCM_K_LDCM_TOP_REG(0x28, 0); /* clear all interrupt */

	free_irq(ipcm_k_ldcm_irq_num, NULL);
}

/**
 * @brief   IPCM MD Interrupt를 Enable하는 함수
 * @fn      int ipcm_k_ldcm_int_enable(enum md_k_int_sig int_sig, void(*callback)(void))
 * @param   int_sig [in] interrupt select
 * @param   void [in] Call back function 등록
 * @return  error return
 *
 * @author  TaeWook Nam
 * @note
 */
int ipcm_k_ldcm_int_enable(enum ldcm_k_intr_flags int_sig, void(*callback)(enum ldcm_k_intr_flags))
{
	unsigned int intr_enable;

	if (!callback) {
		dev_dbg(ipcm_dev, "\n%s:callback function pointer is NULL.\n", __func__);
		return IPCM_K_OP_PARAM_ERROR;
	}

	intr_enable = READ_IPCM_K_LDCM_TOP_REG(0x24);

	intr_enable = intr_enable & (~(0x1 << int_sig));
	init_completion(&ipcm_ldcm_completion[int_sig]);

	WRITE_IPCM_K_LDCM_TOP_REG(0x28, 0); /* clear all interrupt */
	WRITE_IPCM_K_LDCM_TOP_REG(0x24, intr_enable); /* disable all interrupt */

	IPCM_LDCM_ISR_Table[int_sig] = callback;

	return IPCM_K_OP_ERROR_NONE;
}

/**
 * @brief   IPCM MD Interrupt를 Disable하는 함수
 * @fn      void ipcm_k_ldcm_int_disable(enum md_k_int_sig int_sig)
 * @param   int_sig		[in] interrupt select
 * @return  void
 *
 * @author  TaeWook Nam
 * @note
 */
void ipcm_k_ldcm_int_disable(enum md_k_int_sig int_sig)
{
	unsigned int intr_enable;

	intr_enable = READ_IPCM_K_LDCM_TOP_REG(0x24);

	intr_enable = intr_enable | (0x1 << int_sig);

	WRITE_IPCM_K_LDCM_TOP_REG(0x28, 0); /* clear all interrupt */
	WRITE_IPCM_K_LDCM_TOP_REG(0x24, intr_enable); /* disable all interrupt */

	IPCM_LDCM_ISR_Table[int_sig] = 0;
}



#ifndef IPCM_TIMEOUT_CHECK
/**
 * @brief   IPCM WDMA0 wait for completion function
 * @fn      void ipcm_k_wdma0_wait_done(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
void ipcm_k_wdma0_wait_done(void)
{
	/*printk("Function: %s\n", __FUNCTION__);*/
	wait_for_completion(&ipcm_wdma0_completion);
}

/**
 * @brief   IPCM WDMA1 wait for completion function
 * @fn      void ipcm_k_wdma0_wait_done(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
void ipcm_k_wdma1_wait_done(void)
{
	/*printk("Function: %s\n", __FUNCTION__);*/
	wait_for_completion(&ipcm_wdma1_completion);
}

/**
 * @brief   IPCM WDMA2 wait for completion function
 * @fn      void ipcm_k_wdma0_wait_done(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
void ipcm_k_wdma2_wait_done(void)
{
	/*printk("Function: %s\n", __FUNCTION__);*/
	wait_for_completion(&ipcm_wdma2_completion);
}


#else

/**
 * @brief   IPCM WDMA0 wait for completion function
 * @fn      int ipcm_k_wdma0_wait_for_completion(unsigned long timeout)
 * @param   timeout [in] mssec 단위가 아니다.
 * @return  error is -1
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
int ipcm_k_wdma0_wait_for_completion(unsigned long timeout)
{
	if (!wait_for_completion_timeout(&ipcm_wdma0_completion, timeout)) {
		printk("%s timeout\n", __func__);
		return -1;
	}
	return 0;
}

/**
 * @brief   IPCM WDMA1 wait for completion function
 * @fn      int ipcm_k_wdma0_wait_for_completion(unsigned long timeout)
 * @param   timeout [in] mssec 단위가 아니다.
 * @return  error is -1
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
int ipcm_k_wdma1_wait_for_completion(unsigned long timeout)
{
	if (!wait_for_completion_timeout(&ipcm_wdma1_completion, timeout)) {
		printk("%s timeout\n", __func__);
		return -1;
	}
	return 0;
}

/**
 * @brief   IPCM WDMA2 wait for completion function
 * @fn      int ipcm_k_wdma0_wait_for_completion(unsigned long timeout)
 * @param   timeout [in] mssec 단위가 아니다.
 * @return  error is -1
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
int ipcm_k_wdma2_wait_for_completion(unsigned long timeout)
{
	if (!wait_for_completion_timeout(&ipcm_wdma2_completion, timeout)) {
		printk("%s timeout\n", __func__);
		return -1;
	}
	return 0;
}

#endif

#ifndef IPCM_TIMEOUT_CHECK
/**
 * @brief   IPCM MD wait for gmv's completion function
 * @fn      void ipcm_k_md_gmv_wait_done(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
void ipcm_k_md_gmv_wait_done(void)
{
	/*printk("Function: %s\n", __FUNCTION__);*/
	wait_for_completion(&ipcm_md_gmv_completion);
}
/**
 * @brief   IPCM MD wait for rmv's completion function
 * @fn      void ipcm_k_md_rmv_wait_done(void)
 * @param   void
 * @return  void
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
void ipcm_k_md_rmv_wait_done(void)
{
	/*printk("Function: %s\n", __FUNCTION__);*/
	wait_for_completion(&ipcm_md_rmv_completion);
}
#else
/**
 * @brief   IPCM MD wait for gmv's completion function
 * @fn      int ipcm_k_md_gmv_wait_for_completion(unsigned long timeout)
 * @param   timeout [in] mssec 단위가 아니다.
 * @return  error is -1
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
int ipcm_k_md_gmv_wait_for_completion(unsigned long timeout)
{
	if (!wait_for_completion_timeout(&ipcm_md_gmv_completion, timeout)) {
		printk("%s timeout\n", __func__);
		return -1;
	}
	return 0;
}

/**
 * @brief   IPCM MD wait for rmv's completion function
 * @fn      int ipcm_k_md_rmv_wait_for_completion(unsigned long timeout)
 * @param   timeout [in] mssec 단위가 아니다.
 * @return  error is -1
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
int ipcm_k_md_rmv_wait_for_completion(unsigned long timeout)
{
	if (!wait_for_completion_timeout(&ipcm_md_rmv_completion, timeout)) {
		printk("%s timeout\n", __func__);
		return -1;
	}
	return 0;
}
#endif


/**
 * @brief   IPCM MD wait for rmv's completion function
 * @fn      int ipcm_k_md_rmv_wait_for_completion(unsigned long timeout)
 * @param   timeout [in] mssec 단위가 아니다.
 * @return  error is -1
 *
 * @author  TaeWook Nam
 * @note    kernel layer
 */
int ipcm_k_ldcm_wait_for_completion(enum ldcm_k_intr_flags intr, unsigned long timeout)
{
	if (!wait_for_completion_timeout(&ipcm_ldcm_completion[intr], timeout)) {
		printk("%s timeout\n", __func__);
		return -1;
	}
	return 0;
}

/**
* @brief	1 frame의 quickview 종료후 PP로 부의 입력 data 차단
* @fn		void ipcm_quickview_done(void)
* @param 	void
* @return	void
*
* @author	TaeWook Nam
* @note		이 함수는 다음과 같은 동작을 처리한다.
*		- pp로 부터 입력되는 data 차단
*		- pp출력에 연결된 liveview의 wdma 출력 차단
*/
void ipcm_k_quickview_done(void)
{
	unsigned int DMA_ctrl_set;
	unsigned int top_reg;

	unsigned char live_w0_flag;
	unsigned char live_w1_flag;
	unsigned char live_w2_flag;

	top_reg = READ_IPCM_K_TOP_REG(0);
	DMA_ctrl_set = READ_IPCM_K_TOP_REG(0x104);

	/* disable PP input */
	D4_IPCM_K_TOP_CONTROL_INPUT_PP(top_reg, 0);

	WRITE_IPCM_K_TOP_REG(0, top_reg);

	/* Get WDMA use info */
	live_w0_flag = D4_IPCM_K_DMACTRL_PP_CON_WDMA0_GET(DMA_ctrl_set);
	live_w1_flag = D4_IPCM_K_DMACTRL_PP_CON_WDMA1_GET(DMA_ctrl_set);
	live_w2_flag = D4_IPCM_K_DMACTRL_PP_CON_WDMA2_GET(DMA_ctrl_set);

	/* disconnect PP-WDMA connect and disable WDMA mode */
	if (live_w0_flag) {
		D4_IPCM_K_TOP_CONTROL_IPCM_WDMA_MODE_MAIN(top_reg, 0);
		D4_IPCM_K_DMACTRL_PP_CON_WDMA0(DMA_ctrl_set, 0);
	}

	if (live_w1_flag) {
		D4_IPCM_K_TOP_CONTROL_IPCM_WDMA_MODE_RSZ(top_reg, 0);
		D4_IPCM_K_DMACTRL_PP_CON_WDMA1(DMA_ctrl_set, 0);
	}

	if (live_w2_flag) {
		D4_IPCM_K_TOP_CONTROL_IPCM_WDMA_MODE_SRSZ(top_reg, 0);
		D4_IPCM_K_DMACTRL_PP_CON_WDMA2(DMA_ctrl_set, 0);
	}

	WRITE_IPCM_K_TOP_REG(0x104, DMA_ctrl_set);
}

