/**
 * @file d4_jpeg_ctrl_dd.c
 * @brief DRIMe4 jpeg register control Function File
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/clk.h>

#include <mach/d4_reg_macro.h>
#include <mach/d4_cma.h>
#include "d4_jpeg_ctrl_dd.h"

#ifdef CONFIG_PMU_SELECT
#include <linux/d4-pmu.h>
#endif

#ifdef CONFIG_RMU_DEV
#include <linux/d4_rmu.h>
#endif

/**< Register Base Address */
static unsigned int jpegTopRegBase;
static unsigned int jpegCtrlRegBase;

/**< IRQ Number */
static unsigned int jpegIrqNum;

/**< JPEG Device Info */
static struct device *jpegDev;

/**< physical register information */
static struct JPEG_PHY_REG_INFO physRegInfo[2];

/**< JPEG Clock */
static struct clk *jpeg_clk_set;

/**< Completion for synchronization */
static struct completion JpegOpCompletion;
static signed int streamOverFlow;

/**< Static Variable & Functions */
static void jpeg_callback(void);
static void jpeg_encode_error_callback(void);
static signed int jpeg_enableInt(const enum JPEG_INT intNum, void(*isrFuncPtr)(void));
static signed int jpeg_disableInt(const enum JPEG_INT intNum);

/******************************** GLOBALS **************************************/
static void (*jpegIsr[JPEG_ACCINT_MAX])(void) = {
	NULL, NULL, NULL, NULL, NULL, NULL};

/**
 * @brief set isr function
 * @fn d4_Jpeg_IRQ(int irq, void *dev_id)
 * @param int irq, void *dev_id
 * @return irqreturn_t
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
irqreturn_t d4_Jpeg_IRQ(int irq, void *dev_id)
{
	int cnt;
	unsigned int status = __raw_readl(jpegCtrlRegBase + JPEG_ACC_INT_STATUS);

	if (status & 0x2) {
		if (jpegIsr[1])
			1[jpegIsr]();

		__raw_writel(0x2, jpegCtrlRegBase + JPEG_ACC_INT_CLR);
	} else {
		for (cnt = 0; cnt < JPEG_ACCINT_MAX; cnt++) {
			if (cnt != 1) {
				if (status & (0x1 << cnt)) {
					/* Call the respective ISR */
					if (jpegIsr[cnt])
						cnt[jpegIsr]();

					__raw_writel((0x1 << cnt), jpegCtrlRegBase + JPEG_ACC_INT_CLR);
				}
			}
		}
	}

	return IRQ_HANDLED;
}

/**
 * @brief Enable jpeg IRQ
 * @fn jpeg_enable_irq(void)
 * @param void
 * @return signed int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
signed int jpeg_enable_irq(void)
{
	signed int result = JPEG_SUCCESS;

	/**< Top Interrupt Registeration */
	if (request_irq(jpegIrqNum, d4_Jpeg_IRQ, IRQF_DISABLED, "d4_jpeg", jpegDev)) {
		printk(KERN_DEBUG "====== d4_Jpeg_IRQ failed ====== \n");
		result = JPEG_IRQ_FAIL;
	}

	return result;
}

/**
 * @brief free IRQ resource
 * @fn   jpeg_disable_irq(void)
 * @param void
 * @return void
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
void jpeg_disable_irq(void)
{
	free_irq(jpegIrqNum, jpegDev);
}

void jpeg_init_completion(void)
{
	init_completion(&JpegOpCompletion);
}

void jpeg_set_clock(const unsigned int clock_set)
{
	jpeg_clk_set = clk_get(jpegDev, "jpeg");
	if (jpeg_clk_set == -2)
			return;
	clk_set_rate(jpeg_clk_set, clock_set);
}

signed int jpeg_register_interrupt(void)
{
	signed int result = JPEG_SUCCESS;
	unsigned int intNum;

	for (intNum = 0; intNum < JPEG_ACCINT_MAX; intNum++) {
		if (JPEG_ACCINT_ERR == intNum) {
			result = jpeg_enableInt((enum JPEG_INT)intNum, jpeg_encode_error_callback);
		} else {
			result = jpeg_enableInt((enum JPEG_INT)intNum, jpeg_callback);
		}

		if (result != JPEG_SUCCESS)
			break;
	}

	return result;
}

signed int jpeg_deregister_interrupt(void)
{
	signed int result = JPEG_SUCCESS;
	unsigned int intNum;

	for (intNum = 0; intNum < JPEG_ACCINT_MAX; intNum++) {
		result = jpeg_disableInt((enum JPEG_INT)intNum);

		if (result != JPEG_SUCCESS)
			break;
	}

	return result;
}

/**
 * @brief get operatoin semaphore
 * @fn   jpeg_get_operation_sema(const unsigned long timeOut)
 * @param unsigned long timeOut
 * @return signed int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
signed int jpeg_get_operation_sema(const unsigned long timeOut)
{
	return (signed int)wait_for_completion_timeout(&JpegOpCompletion, (timeOut*20)/HZ);
}

/**
 * @brief JPEG get physical resigter information.
 * @fn   jpeg_get_phys_reg_info(struct JPEG_GET_REG_INFO *regInfo)
 * @param struct JPEG_GET_REG_INFO *
 * @return none
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
void jpeg_get_phys_reg_info(struct JPEG_GET_REG_INFO *regInfo)
{
	switch (regInfo->type) {
	case JPEG_TOP_REG:
		regInfo->info.startAddr = physRegInfo[JPEG_TOP_REG].startAddr;
		regInfo->info.size = physRegInfo[JPEG_TOP_REG].size;
		break;
	case JPEG_CTRL_REG:
		regInfo->info.startAddr = physRegInfo[JPEG_CTRL_REG].startAddr;
		regInfo->info.size = physRegInfo[JPEG_CTRL_REG].size;
		break;
	}
}

/**
 * @brief set resigter address
 * @fn   jpeg_set_reg_ctrl_base_info(struct jpeg_reg_ctrl_base_info *info)
 * @param struct jpeg_reg_ctrl_base_info *
 * @return void
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
void jpeg_set_reg_ctrl_base_info(struct JPEG_REG_CTRL_BASE_INFO *info)
{
	jpegDev = info->dev_info;
	jpegTopRegBase = info->topRegBase;
	jpegCtrlRegBase = info->ctrlRegBase;
	jpegIrqNum = info->irq_num;

	/**< physical register information */
	physRegInfo[JPEG_TOP_REG].startAddr = info->physTopReg.startAddr;
	physRegInfo[JPEG_TOP_REG].size = info->physTopReg.size;
	physRegInfo[JPEG_CTRL_REG].startAddr = info->physCtrlReg.startAddr;
	physRegInfo[JPEG_CTRL_REG].size = info->physCtrlReg.size;
}

/**
 * @brief release semaphore
 * @fn   jpeg_callback(void)
 * @param void
 * @return void
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static void jpeg_callback(void)
{
	complete(&JpegOpCompletion);
}

/**
 * @brief release semaphore and STOP jpeg encode
 * @fn   jpeg_encode_error_callback(void)
 * @param void
 * @return void
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static void jpeg_encode_error_callback(void)
{
	unsigned int value = 0;

	/* stop JPEG encode */
	SET_REGISTER_VALUE(value, 0, 0, 1);
	SET_REGISTER_VALUE(value, 0, 1, 1);
	SET_REGISTER_VALUE(value, 1, 2, 1);
	__raw_writel(value, jpegCtrlRegBase + JPEG_JP_CMD);

	value = 0;
	SET_REGISTER_VALUE(value, 1, 2, 1);
	SET_REGISTER_VALUE(value, 1, 3, 1);
	__raw_writel(value, jpegCtrlRegBase + JPEG_JF_CMD);

	streamOverFlow = JPEG_STREAM_SIZE_OVERFLOW;
	complete(&JpegOpCompletion);
}

signed int jpeg_checkEncodeStatus(void)
{
	signed int retVal = streamOverFlow;

	streamOverFlow = 0;

	return retVal;
}



/**
 * @brief Set JPEG ACC block interrupt enable
 * @fn jpeg_enableInt(enum JPEG_INT IntNum, void(*isrFuncPtr)(void))
 * @param enum JPEG_INT IntNum, void(*)(void)
 * @return signed int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static signed int jpeg_enableInt(const enum JPEG_INT intNum, void(*isrFuncPtr)(void))
{
	unsigned int value = 0;
	signed int result = JPEG_SUCCESS;

	if (NULL == isrFuncPtr) {
		printk(KERN_DEBUG "===== INTERRUPT CallBackFunc Error =======\n");
		result = JPEG_NULL_CALLBACK;
	}

	if (JPEG_SUCCESS == result) {
		intNum[jpegIsr] = isrFuncPtr;

		value = __raw_readl(jpegCtrlRegBase + JPEG_ACC_INT_EN);

		if (JPEG_ACCINT_TRCMPT == intNum) {
			SET_REGISTER_VALUE(value, 1, 0, 1);
		} else if (JPEG_ACCINT_ERR == intNum) {
			SET_REGISTER_VALUE(value, 1, 1, 1);
		} else if (JPEG_ACCINT_DEND == intNum) {
			SET_REGISTER_VALUE(value, 1, 2, 1);
		} else if (JPEG_ACCINT_MRKDET == intNum) {
			SET_REGISTER_VALUE(value, 1, 3, 1);
		} else if (JPEG_ACCINT_EEND == intNum) {
			SET_REGISTER_VALUE(value, 1, 4, 1);
		} else if (JPEG_ACCINT_MRKSET == intNum) {
			SET_REGISTER_VALUE(value, 1, 5, 1);
		}

		__raw_writel(value, jpegCtrlRegBase + JPEG_ACC_INT_EN);
	}

	return result;
}

/**
 * @brief Set JPEG interrupt disable
 * @fn jpeg_disableInt(enum JPEG_INT IntNum)
 * @param enum JPEG_INT
 * @return signed int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static signed int jpeg_disableInt(const enum JPEG_INT intNum)
{
	unsigned int value = 0;
	signed int result = JPEG_SUCCESS;

	if (intNum >= JPEG_ACCINT_MAX) {
		result = JPEG_INVALID_INT_NUM;
	}

	if (JPEG_SUCCESS == result) {
		jpegIsr[(int)intNum] = NULL;

		value = __raw_readl(jpegCtrlRegBase + JPEG_ACC_INT_EN);

		if (JPEG_ACCINT_TRCMPT == intNum) {
			SET_REGISTER_VALUE(value, 0, 0, 1);
		} else if (JPEG_ACCINT_ERR == intNum) {
			SET_REGISTER_VALUE(value, 0, 1, 1);
		} else if (JPEG_ACCINT_DEND == intNum) {
			SET_REGISTER_VALUE(value, 0, 2, 1);
		} else if (JPEG_ACCINT_MRKDET == intNum) {
			SET_REGISTER_VALUE(value, 0, 3, 1);
		} else if (JPEG_ACCINT_EEND == intNum) {
			SET_REGISTER_VALUE(value, 0, 4, 1);
		} else if (JPEG_ACCINT_MRKSET == intNum) {
			SET_REGISTER_VALUE(value, 0, 5, 1);
		}

		__raw_writel(value, jpegCtrlRegBase + JPEG_ACC_INT_EN);
	}

	return result;
}

/**
 * @brief	IPCM 의 RMU release process function
 * @fn      void jpeg_k_dma_rmu_bus(void)
 * @param	void
 * @return	void
 *
 * @author	Kyuchun Han, Dongjin Jung
 * @note	kernel layer
 */
void jpeg_k_reset(void)
{
	__raw_writel(0x7, jpegCtrlRegBase);
	__raw_writel(0x0, jpegCtrlRegBase);
	__raw_writel(0x7, jpegCtrlRegBase);

	__raw_writel(0x4, jpegCtrlRegBase + JPEG_JP_CMD);
	__raw_writel(0x0, jpegCtrlRegBase + JPEG_JP_CMD);
	__raw_writel(0x4, jpegCtrlRegBase + JPEG_JP_CMD);

	__raw_writel(0x8, jpegCtrlRegBase + 0x1000);
	__raw_writel(0x0, jpegCtrlRegBase + 0x1000);
	__raw_writel(0x8, jpegCtrlRegBase + 0x1000);
}


void jpeg_reset(void)
{
	unsigned int temp;
	/*DMA start*/
	__raw_writel(0x0, jpegTopRegBase);
	__raw_writel(0x2, jpegTopRegBase);
	__raw_writel(0x2, jpegTopRegBase);
	__raw_writel(0x0, jpegCtrlRegBase);
	__raw_writel(0x7, jpegCtrlRegBase);
	__raw_writel(0x29, jpegCtrlRegBase + 0x4);

	__raw_writel(0xD932, jpegCtrlRegBase + 0x804);
	__raw_writel(0x100008, jpegCtrlRegBase + 0x810);
	__raw_writel(0x0, jpegCtrlRegBase + 0x80C);
	__raw_writel(0x81000000, jpegCtrlRegBase + 0x814);
	__raw_writel(0x81003000, jpegCtrlRegBase + 0x818);
	__raw_writel(0x81006000, jpegCtrlRegBase + 0x820);
	__raw_writel(0x81009000, jpegCtrlRegBase + 0x824);
	__raw_writel(0x0, jpegCtrlRegBase + 0x828);
	__raw_writel(0x0, jpegCtrlRegBase + 0x82C);
	__raw_writel(0x800, jpegCtrlRegBase + 0x81C);
	__raw_writel(0x800, jpegCtrlRegBase + 0x830);
	__raw_writel(0x84000000, jpegCtrlRegBase + 0x838);


	/* INT enable */
	__raw_writel(0x2f, jpegCtrlRegBase + 0x4);
	__raw_writel(0x2, jpegCtrlRegBase + 0x800);
	__raw_writel(0x0, jpegCtrlRegBase + 0x400);
	__raw_writel(0x236614, jpegCtrlRegBase + 0x408);
	__raw_writel(0x100008, jpegCtrlRegBase + 0x40C);
	__raw_writel(0x1120111, jpegCtrlRegBase + 0x410);
	__raw_writel(0xFFFF, jpegCtrlRegBase + 0x414);
	__raw_writel(0x0, jpegCtrlRegBase + 0x418);
	__raw_writel(0x4, jpegCtrlRegBase + 0x400);
	__raw_writel(0x1, jpegCtrlRegBase + 0x400);


	/* Wait for Interrupt*/
	/*
	temp = (__raw_readl(jpegCtrlRegBase + 0xC) & (1<<5)) >> 5;
	while (temp == 0)
	temp = (__raw_readl(jpegCtrlRegBase + 0xC) & (1<<5)) >> 5;
	 */

	udelay(5);

	/* INT Clear */
	temp = __raw_readl(jpegCtrlRegBase + 0x8);
	temp |= ((0x1)<<5);
	__raw_writel(temp, jpegCtrlRegBase + 0x8);
	/*__raw_writel(0x20, jpegCtrlRegBase + 0x8);*/
	__raw_writel(0x1, jpegCtrlRegBase + 0x800);
	__raw_writel(0x2, jpegCtrlRegBase + 0x400);

	/* Wait for Interrupt*/
	/*
	temp = __raw_readl(jpegCtrlRegBase + 0xC) & 0x1;
	while (temp == 0)
	temp = __raw_readl(jpegCtrlRegBase + 0xC) & 0x1;
	 */

	udelay(5);
	/* INT Clear */
	temp = __raw_readl(jpegCtrlRegBase + 0x8);
	temp |= 0x1;
	__raw_writel(temp, jpegCtrlRegBase + 0x8);
/*	__raw_writel(0x1, jpegCtrlRegBase + 0x8);*/
	__raw_writel(0xC, jpegCtrlRegBase + 0x800);
	__raw_writel(0x4, jpegCtrlRegBase + 0x400);
	__raw_writel(0x4D, jpegCtrlRegBase + 0x844);
}

void jpeg_dma_reset(void)
{
	__raw_writel(0x0, jpegCtrlRegBase);
	__raw_writel(0x7, jpegCtrlRegBase);
}


void jpeg_pmu_request()
{

}

/**
 * @brief	JPEG 의 PMU(Power Management Unit) 설정을 하는 함수
 * @fn      void jpeg_k_pmu_on_off(enum jpeg_k_on_off pmu_type)
 * @param	pmu_type	[in] On/Off
 * @return	void
 *
 * @author	Kyuchun Han, Dongjin Jung
 * @note	kernel layer
 */
void jpeg_k_pmu_on_off(enum jpeg_k_on_off pmu_type)
{
#ifdef CONFIG_PMU_SELECT
	/*mutex_lock(&ipcs_pmu_mutex);*/

	int reval;
	struct clk *clock;
	struct d4_rmu_device *rmu;

	if (pmu_type == K_JPEG_ON) {
		clock = clk_get(jpegDev, "jpeg");
		if (clock == ERR_PTR(-ENOENT))
			return;

		clk_disable(clock);
		clk_put(clock);

	} else {
		clock = clk_get(jpegDev, "jpeg");
		if (clock == ERR_PTR(-ENOENT))
			return;
		clk_enable(clock);
		clk_set_rate(clock, 216000000);
	}




#if 0
	if (pmu_type == K_JPEG_ON) {
		reval = d4_pmu_check(PMU_JPEG);
		if (reval != 0) {

			reval = d4_pmu_request(PMU_JPEG);
			if (reval)
				return;

			rmu = d4_rmu_request();

			d4_pmu_isoen_set(PMU_JPEG, PMU_CTRL_ON);

			d4_sw_isp_reset(rmu, RST_JPEG);

			clock = clk_get(jpegDev, "jpeg");

			if (clock == ERR_PTR(-ENOENT))
					return;

			clk_disable(clock);
			clk_put(clock);

			d4_pmu_request_clear(PMU_JPEG);

			d4_pmu_scpre_set(PMU_JPEG, PMU_CTRL_ON);
			d4_pmu_scall_set(PMU_JPEG, PMU_CTRL_ON);
			d4_rmu_release(rmu);
		}
	} else {
		rmu = d4_rmu_request();
		d4_pmu_scpre_set(PMU_JPEG, PMU_CTRL_OFF);
		udelay(100);
		d4_pmu_scall_set(PMU_JPEG, PMU_CTRL_OFF);
		reval = wait_for_stable(PMU_JPEG);
		if (reval) {
			d4_pmu_scpre_set(PMU_JPEG, PMU_CTRL_ON);
			d4_pmu_scall_set(PMU_JPEG, PMU_CTRL_ON);
			return;
		}
		clock = clk_get(jpegDev, "jpeg");
		clk_enable(clock);
		clk_set_rate(clock, 216000000);
		d4_pmu_isoen_set(PMU_JPEG, PMU_CTRL_OFF);

		/**< method 1 */
		/* RMU Reset*/
		d4_sw_isp_reset(rmu, RST_JPEG);
		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_JPEG);

		/*2~4*/
		d4_pmu_bus_reset(PMU_JPEG);
		jpeg_reset();

		/* RMU Reset*/
		d4_sw_isp_reset(rmu, RST_JPEG);
		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_JPEG);

		/*2~4*/
		d4_pmu_bus_reset(PMU_JPEG);
		jpeg_reset();

		/* DMA Reset */
		jpeg_dma_reset();

		/* bus Reset */
		d4_pmu_bus_reset(PMU_JPEG);

		d4_rmu_release(rmu);
	}
#endif
	/*mutex_unlock(&ipcs_pmu_mutex);*/
#endif
}


