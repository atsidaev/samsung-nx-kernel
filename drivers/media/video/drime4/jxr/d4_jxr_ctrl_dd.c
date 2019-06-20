/**
 * @file d4_jxr_ctrl_dd.c
 * @brief DRIMe4 JPEG_XR Control Device Driver Function File
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <asm/cacheflush.h>

#include <mach/d4_reg_macro.h>
#include <mach/d4_cma.h>
#include "d4_jxr_ctrl_dd.h"

/* #define RELEASE_MODE
#ifdef RELEASE_MODE
asmlinkage int NULL_PRINTK(const char *fmt, ...) { return 0; }
#define printk NULL_PRINTK
#else
#define printk(fmt, args...)							\
	{															\
		printk("\r[%s() %d]	", __FUNCTION__, __LINE__);		\
		printk(fmt, ##args); 								\
	}
#endif
*/

irqreturn_t  d4_Jxr_IRQ(int irq, void *dev_id);

static struct device	*jxr_dev;
static struct completion JxrOpCompletion;
static struct JXR_PHYS_REG_INFO physRegInfo;

static unsigned int jxr_reg_base;
static unsigned int jxr_irq_num;
#ifdef __TBD__
static unsigned int *paTileOffset;
static unsigned char gJxrHorTileNum;
static unsigned char gJxrVerTileNum;
static unsigned int gImgBaseAddr;
#endif

static void (*jxrIsrTbl[JXR_INT_MAX])(void) = {
   JXR_NULL, /* 0 - Reserved   */
   JXR_NULL, /* 1 - Image Complete  */
   JXR_NULL, /* 2 - Reserved */
   JXR_NULL, /* 3 - Reserved */
   JXR_NULL, /* 4 - Frame Error */
   JXR_NULL  /* 5 - Tile Done */
};

/**
 * @brief JXR semaphore optain
 * @fn jxr_getSema(unsigned int timeOut)
 * @param Field[in] unsigned int timeOut
 * @return unsigned long
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
unsigned long jxr_getSema(unsigned int timeOut)
{
	return wait_for_completion_timeout(&JxrOpCompletion, timeOut);
}

/**
 * @brief JXR Call the respective ISR
 * @fn  jxr_IRQ(int irq, void *dev_id)
 * @param int, void *
 * @return irqreturn_t
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
irqreturn_t jxr_IRQ(int irq, void *dev_id)
{
	unsigned int cnt;
	unsigned int status = __raw_readl(jxr_reg_base + JXR_INT_STATUS_REG);

	for (cnt = 0; cnt < JXR_INT_MAX; cnt++) {
		if (status & (1 << cnt)) {
			/* Call the respective ISR */
			if (jxrIsrTbl[cnt])
				jxrIsrTbl[cnt]();

			__raw_writel((1 << cnt), jxr_reg_base + JXR_INT_STATUS_REG);
		}
	}

	return IRQ_HANDLED;
}

/**
 * @brief JXR request ISQ resource
 * @fn  jxr_initIRQ(void)
 * @param void
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
signed int jxr_initIRQ(void)
{
	signed int result = JXR_SUCCESS;

	if (request_irq(jxr_irq_num, jxr_IRQ, IRQF_DISABLED, "d4_jpeg_xr", jxr_dev)) {
		printk(KERN_DEBUG "====== JXR IRQ init  Failed ====== \n");
		result = JXR_IRQ_FAIL;
	}

	return result;
}

/**
 * @brief free ipq resource
 * @fn   jxr_deInitIRQ(void)
 * @param void
 * @return void
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
void jxr_deInitIRQ(void)
{
	free_irq(jxr_irq_num, jxr_dev);
}

/**
 * @brief init completion.
 * @fn   jxr_init_completion(void)
 * @param void
 * @return void
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
void jxr_init_completion(void)
{
	init_completion(&JxrOpCompletion);
}

/**
 * @brief JXR enable interrupt
 * @fn  d4_Jxr_EnableInt(enum JXR_INT IntNum, void (*isr)(void))
 * @param enum JXR_INT IntNum, void (*isr)(void)
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
signed int jxr_enableInt(enum JXR_INT intNum, void (*isr)(void))
{
	unsigned int value = JXR_ZERO;
	signed int result = JXR_SUCCESS;

	if (JXR_NULL == isr) {
		result = JXR_NULL_CALLBACK;
	} else {
		jxrIsrTbl[(int)intNum] = isr;

		value =  __raw_readl(jxr_reg_base + JXR_INT_ENABLE_REG);

		if (JXR_INT_IMG_COMPLETE == intNum) {
			D4_JXR_INT_ENABLE_REG_IMAGE_COMPLETION(value, 1);
		} else if (JXR_INT_FRAME_ERROR == intNum) {
			D4_JXR_INT_ENABLE_REG_FRAME_ERR(value, 1);
		} else if (JXR_INT_TILE_DONE == intNum) {
			D4_JXR_INT_ENABLE_REG_TILE_DONE(value, 1);
		}

		__raw_writel(value, jxr_reg_base + JXR_INT_ENABLE_REG);
	}

	return result;
}

/**
 * @brief JXR disable interrupt
 * @fn  d4_Jxr_DisableInt(enum JXR_INT IntNum)
 * @param enum JXR_INT
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
signed int jxr_disableInt(enum JXR_INT intNum)
{
	unsigned int value = JXR_ZERO;
	signed int result = JXR_SUCCESS;

	if (intNum >= JXR_INT_MAX) {
		result = JXR_WRONG_INT_NUM;
	} else {
		jxrIsrTbl[(int)intNum] = JXR_NULL;

		value = __raw_readl(jxr_reg_base + JXR_INT_ENABLE_REG);

		if (JXR_INT_IMG_COMPLETE == intNum) {
			D4_JXR_INT_ENABLE_REG_IMAGE_COMPLETION(value, 0);
		} else if (JXR_INT_FRAME_ERROR == intNum) {
			D4_JXR_INT_ENABLE_REG_FRAME_ERR(value, 0);
		} else if (JXR_INT_TILE_DONE == intNum) {
			D4_JXR_INT_ENABLE_REG_TILE_DONE(value, 0);
		}

		__raw_writel(value, jxr_reg_base + JXR_INT_ENABLE_REG);
	}

	return result;
}

/**
 * @brief JXR enable interrupt and call back function
 * @fn  jxr_registerInterrupt(void)
 * @param enum void
 * @return signed int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
signed int jxr_registerInterrupt(void)
{
	signed int result = JXR_SUCCESS;

	result = jxr_disableInt(JXR_INT_TILE_DONE);

	if (!result) {
		result = jxr_enableInt(JXR_INT_IMG_COMPLETE, jxr_callBack);
	}

	return result;
}

/**
 * @brief JXR disable interrupt and call back function
 * @fn  jxr_deRegisterInterrupt(void)
 * @param enum void
 * @return signed int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
signed int jxr_deRegisterInterrupt(void)
{
	signed int result = JXR_SUCCESS;

	result = jxr_disableInt(JXR_INT_TILE_DONE);

	if (!result) {
		result = jxr_disableInt(JXR_INT_IMG_COMPLETE);
	}

	return result;
}

/**
 * @brief JXR Image done ISR Callback function
 * @fn  jxr_callBack(void)
 * @param void
 * @return void
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
void jxr_callBack(void)
{
	complete(&JxrOpCompletion);
}

/**
 * @brief JXR get physical register information.
 * @fn  jxr_get_phys_reg_info(struct JXR_PHYS_REG_INFO *regInfo)
 * @param struct JXR_PHYS_REG_INFO *
 * @return void
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
void jxr_get_phys_reg_info(struct JXR_PHYS_REG_INFO *regInfo)
{
	regInfo->startAddr = regInfo->type ? physRegInfo.startAddr : JXR_TOP_REG_BASE;
	regInfo->size	     = regInfo->type ? physRegInfo.size 	    : JXR_TOP_REG_SIZE;
}


/**
 * @brief JXR set base resigter information
 * @fn  jxr_set_reg_ctrl_base_info(struct jxr_reg_ctrl_base_info *info)
 * @param struct jxr_reg_ctrl_base_info *
 * @return void
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
void jxr_set_reg_ctrl_base_info(struct jxr_reg_ctrl_base_info *info)
{
	jxr_dev 				= info->dev_info;
	jxr_reg_base			= info->op_reg_base;
	jxr_irq_num			= info->irq_num;

	physRegInfo.startAddr = info->physCtrlReg.startAddr;
	physRegInfo.size 		 = info->physCtrlReg.size;
}

