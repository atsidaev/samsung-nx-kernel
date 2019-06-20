/**
 * @file	d4_ipcm_ctrl.c
 * @brief	IPCM Core control and set driver file for Samsung DRIMeIV Camera
 *		Interface driver
 *
 * @author	TaeWook <tw.nam@samsung.com>
 *
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/io.h>
#include <linux/device.h>
#include <linux/delay.h>

#include <linux/d4-pmu.h>
#include <linux/d4_rmu.h>

#include "d4_ipcm_ctrl.h"

/**********************************************************************************/
/* Device infomation */
static struct device *ipcm_k_dev;

/* Register base address */
unsigned int ipcm_k_reg_base;
unsigned int ipcm_k_md_reg_base;
unsigned int ipcm_k_ldcm_top_reg_base;

/* IRQ number */
int ipcm_k_dma_irq_num;
int ipcm_k_md_irq_num;
int ipcm_k_ldcm_irq_num;

/* Physical register infomation */
static struct ipcm_k_physical_reg_info ipcm_k_reg_info;
/**********************************************************************************/


/**
 * @brief	IPCM 의 device Physical register information을 가져오는 함수
 * @fn		void ipcm_k_get_physical_reg_info(struct ipcm_k_physical_reg_info *reg_info)
 * @param	reg_start_addr	[out] ipcm start register address
 * @param	reg_size	[out] register size
 * @return	void
 *
 * @author	TaeWook Nam
 * @note	kernel layer
 */
void ipcm_k_get_physical_reg_info(struct ipcm_k_physical_reg_info *reg_info)
{
	reg_info->reg_start_addr = ipcm_k_reg_info.reg_start_addr;
	reg_info->reg_size = ipcm_k_reg_info.reg_size;

	/*
	printk("\n=======================================\n");
	printk("Function : %s\n", __FUNCTION__);
	printk("Start addr : 0x%x\n", reg_info->reg_start_addr);
	printk("Size : 0x%x\n", reg_info->reg_size);
	printk("=======================================\n");
	*/
}

/**
 * @brief	IPCM 의 device information을 설정하는 함수
 * @fn		void ipcm_k_set_dev_info(struct ipcm_k_reg_ctrl_base_info *info)
 * @param	dev_info		[in] ipcm device information 전달을 위한 파라메터
 * @param	reg_base		[in] ipcm register base address
 * @param	irq_num			[in] ipcm IRQ number
 * @param	reg_start_addr		[in] ipcm start register address
 * @param	reg_size		[in] register size
 * @return	void
 *
 * @author	TaeWook Nam
 * @note	kernel layer
 */
void ipcm_k_set_dev_info(struct ipcm_k_reg_ctrl_base_info *info)
{
	ipcm_k_dev = info->dev_info;
	ipcm_k_reg_base = info->reg_base;
	ipcm_k_md_reg_base = info->reg_base + IPCM_K_MD_TOP_REG_OFFSET;
	ipcm_k_ldcm_top_reg_base = info->reg_base + IPCM_K_LDCM_TOP_REG_OFFSET;

	ipcm_k_dma_irq_num = info->dma_irq_num;
	ipcm_k_md_irq_num = info->md_irq_num;
	ipcm_k_ldcm_irq_num = info->ldcm_irq_num;

	ipcm_k_reg_info.reg_start_addr = info->phys_reg_info.reg_start_addr;
	ipcm_k_reg_info.reg_size = info->phys_reg_info.reg_size;

	/*
	printk("\n=======================================\n");
	printk("Function : %s\n", __FUNCTION__);
	printk("Device : %x\n", ipcm_k_dev);
	printk("Reg Base : 0x%x\n", ipcm_k_reg_base);
	printk("IRQ Num : %d\n", ipcm_k_irq_num);
	printk("Start addr : 0x%x\n", ipcm_k_reg_info.reg_start_addr);
	printk("Size : 0x%x\n", ipcm_k_reg_info.reg_size);
	printk("=======================================\n");
	*/
}

/**
 * @brief	IPCM 의 Clock frequency 설정을 하는 함수
 * @fn		void ipcm_k_clk_set_rate(unsigned int clk_rate)
 * @param	clk_rate	[in] clock frequency
 * @return	void
 *
 * @author	TaeWook Nam
 * @note	kernel layer
 */
void ipcm_k_clk_set_rate(unsigned int clk_rate)
{
	struct clk *ipcm_clk_set;

	/* Clock Enable */
	ipcm_clk_set = clk_get(ipcm_k_dev, "ipcm");
	/*clk_enable(ipcm_ctx->clock);*/
	/* Clock freq. */

	if (ipcm_clk_set == -2)
		return;

	clk_set_rate(ipcm_clk_set, clk_rate);
	/*
	printk("\n=======================================\n");
	printk("Function : %s\n", __FUNCTION__);
	printk("Clock Rate : %d Hz\n", clk_rate);
	printk("=======================================\n");
	*/
}

/****************************************************************************/

/*static int ipcm_pmu_state;*/
/*static DEFINE_MUTEX(ipcm_pmu_mutex);*/

/**
 * @brief	IPCM 의 RMU release process function
 * @fn      void ipcm_k_dma_rmu_bus(void)
 * @param	void
 * @return	void
 *
 * @author	Kyuchun Han, Dongjin Jung
 * @note	kernel layer
 */
void ipcm_k_dma_rmu_bus(void)
{
	int i;

	/* DMA Start */
	WRITE_IPCM_K_TOP_REG(0xF44, 0x80);
	WRITE_IPCM_K_TOP_REG(0xF3C, 0x80000000);
	WRITE_IPCM_K_TOP_REG(0xF54, 0x0);
	WRITE_IPCM_K_TOP_REG(0xF48, 0x10000);

	/* Wait for Err INT */
	for (i = 0; i < 100; i++)
		udelay(50);
}

static void ipcm_k_dma_sw_reset(void)
{
	/* DMA Reset & INT Clear */
	WRITE_IPCM_K_TOP_REG(0x28, 0xEFFFFFEF);
	WRITE_IPCM_K_TOP_REG(0xC, 0xFFFFFFFF);
}

int ipcm_pmu_requeset(void)
{
	int ret = 0;
#ifdef CONFIG_PMU_SELECT
	/* DMA Start */
	WRITE_IPCM_K_TOP_REG(0xF5C, 0x1)
	ret = pmu_wait_for_ack(PMU_IPCM);

	if(ret != 0) {
		return ret;
	}
#endif
	return ret;
}

void ipcm_pmu_clear(void)
{
	WRITE_IPCM_K_TOP_REG(0xF5C, 0x0);
}

/**
 * @brief	IPCM 의 PMU(Power Management Unit) 설정을 하는 함수
 * @fn      void ipcm_k_pmu_on_off(enum ipcm_k_on_off pmu_type)
 * @param	pmu_type	[in] On/Off
 * @return	void
 *
 * @author	Kyuchun Han, Dongjin Jung
 * @note	kernel layer
 */
void ipcm_k_pmu_on_off(enum ipcm_k_on_off pmu_type)
{
#ifdef CONFIG_PMU_SELECT
	/*mutex_lock(&ipcm_pmu_mutex);*/

	int reval;
	struct clk *clock;
	struct d4_rmu_device *rmu;

	if (pmu_type == K_IPCM_ON) {

		/*if (ipcm_pmu_state == 0) {
			mutex_unlock(&ipcm_pmu_mutex);
			return;
		}
		ipcm_pmu_state = 0;*/

		reval = d4_pmu_check(PMU_IPCM);
		if (reval != 0) {
			reval = ipcm_pmu_requeset();
			if (reval)
				return;

			rmu = d4_rmu_request();
			if (rmu == NULL)
				return;

			d4_pmu_isoen_set(PMU_IPCM, PMU_CTRL_ON);
			d4_sw_isp_reset(rmu, RST_IPCM);
			clock = clk_get(ipcm_k_dev, "ipcm");

			if (clock == -2) {
				if (rmu != NULL)
					kfree(rmu);
				return;
			}

			clk_disable(clock);
			clk_put(clock);

			d4_pmu_scpre_set(PMU_IPCM, PMU_CTRL_ON);
			d4_pmu_scall_set(PMU_IPCM, PMU_CTRL_ON);
			d4_rmu_release(rmu);
		}
	} else {
		/*if (ipcm_pmu_state == 1) {
			mutex_unlock(&ipcm_pmu_mutex);
			return;
		}
		ipcm_pmu_state = 1;*/


		d4_pmu_scpre_set(PMU_IPCM, PMU_CTRL_OFF);
		d4_pmu_scall_set(PMU_IPCM, PMU_CTRL_OFF);

		reval = wait_for_stable(PMU_IPCM);
		if (reval) {
			d4_pmu_scpre_set(PMU_IPCM, PMU_CTRL_ON);
			d4_pmu_scall_set(PMU_IPCM, PMU_CTRL_ON);
			return;
		}

		rmu = d4_rmu_request();
		clock = clk_get(ipcm_k_dev, "ipcm");

		if (clock == -2) {
			if (rmu != NULL)
				kfree(rmu);
			return;
		}

		d4_sw_isp_reset(rmu, RST_IPCM);
		udelay(1);

		clk_enable(clock);
		clk_set_rate(clock, 200000000);
		d4_pmu_isoen_set(PMU_IPCM, PMU_CTRL_OFF);

		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_IPCM);
		/* Added part *************************/

		/*2~4*/
		d4_pmu_bus_reset(PMU_IPCM);
		ipcm_k_dma_rmu_bus();
		/*rmu reset*/
		d4_sw_isp_reset(rmu, RST_IPCM);
		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_IPCM);
		/*2~4*/
		d4_pmu_bus_reset(PMU_IPCM);
		ipcm_k_dma_rmu_bus();

		ipcm_k_dma_sw_reset();
		d4_pmu_bus_reset(PMU_IPCM);
		/**************************************/
		d4_rmu_release(rmu);
	}

	/*mutex_unlock(&ipcm_pmu_mutex);*/
#endif
}

