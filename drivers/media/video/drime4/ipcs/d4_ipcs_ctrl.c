/**
 * @file	d4_ipcs_ctrl.c
 * @brief	IPCS Core control and set driver file for Samsung DRIMeIV Camera
 * 			 Interface driver
 *
 * @author	Dongjin Jung <djin81.jung@samsung.com>
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

#include <linux/clk.h>

#ifdef CONFIG_PMU_SELECT
#include <linux/d4-pmu.h>
#endif

#ifdef CONFIG_RMU_DEV
#include <linux/d4_rmu.h>
#endif

#include "d4_ipcs_ctrl.h"

/**********************************************************************************/
/* Device infomation */
static struct device *ipcs_k_dev;

/* Register base address */
unsigned int ipcs_k_reg_base;

/* IRQ number */
int ipcs_k_irq_num;

/* Physical register infomation */
static struct ipcs_k_physical_reg_info ipcs_k_reg_info;
/**********************************************************************************/


/**
 * @brief	IPCS 의 device Physical register information을 가져오는 함수
 * @fn      void ipcs_k_get_physical_reg_info(struct ipcs_k_physical_reg_info *reg_info)
 * @param	reg_start_addr	[out] ipcs start register address
 * @param	reg_size		[out] register size
 * @return	void
 *
 * @author	Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_get_physical_reg_info(struct ipcs_k_physical_reg_info *reg_info)
{
	reg_info->reg_start_addr = ipcs_k_reg_info.reg_start_addr;
	reg_info->reg_size = ipcs_k_reg_info.reg_size;

	/*
	printk("\n=======================================\n");
	printk("Function : %s\n", __FUNCTION__);
	printk("Start addr : 0x%x\n", reg_info->reg_start_addr);
	printk("Size : 0x%x\n", reg_info->reg_size);
	printk("=======================================\n");
	*/
}

/**
 * @brief	IPCS 의 device information을 설정하는 함수
 * @fn      void ipcs_k_set_dev_info(struct ipcs_k_reg_ctrl_base_info *info)
 * @param	dev_info		[in] ipcs device information 전달을 위한 파라메터
 * @param	reg_base		[in] ipcs register base address
 * @param	irq_num			[in] ipcs IRQ number
 * @param	reg_start_addr	[in] ipcs start register address
 * @param	reg_size		[in] register size
 * @return	void
 *
 * @author	Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_set_dev_info(struct ipcs_k_reg_ctrl_base_info *info)
{
	ipcs_k_dev = info->dev_info;
	ipcs_k_reg_base = info->reg_base;
	ipcs_k_irq_num = info->irq_num;

	ipcs_k_reg_info.reg_start_addr = info->phys_reg_info.reg_start_addr;
	ipcs_k_reg_info.reg_size = info->phys_reg_info.reg_size;

	/*
	printk("\n=======================================\n");
	printk("Function : %s\n", __FUNCTION__);
	printk("Device : %x\n", ipcs_k_dev);
	printk("Reg Base : 0x%x\n", ipcs_k_reg_base);
	printk("IRQ Num : %d\n", ipcs_k_irq_num);
	printk("Start addr : 0x%x\n", ipcs_k_reg_info.reg_start_addr);
	printk("Size : 0x%x\n", ipcs_k_reg_info.reg_size);
	printk("=======================================\n");
	*/
}

/**
 * @brief	IPCS 의 Clock frequency 설정을 하는 함수
 * @fn      void ipcs_k_clk_set_rate(unsigned int clk_rate)
 * @param	clk_rate	[in] clock frequency
 * @return	void
 *
 * @author	Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_clk_set_rate(unsigned int clk_rate)
{
	struct clk *ipcs_clk_set;

	/* Clock Enable */
	ipcs_clk_set = clk_get(ipcs_k_dev, "ipcs");
	/*clk_enable(ipcs_ctx->clock);*/
	/* Clock freq. */
	if (ipcs_clk_set == -2)
			return;

    clk_set_rate(ipcs_clk_set, clk_rate);
    /*
	printk("\n=======================================\n");
	printk("Function : %s\n", __FUNCTION__);
	printk("Clock Rate : %d Hz\n", clk_rate);
	printk("=======================================\n");
	*/
}

/****************************************************************************/
/**
 * @brief	IPCS 의 RMU release process function
 * @fn      void ipcs_k_dma_rmu_bus(void)
 * @param	void
 * @return	void
 *
 * @author	Kyuchun Han, Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_dma_rmu_bus(void)
{
	int i;

	/* DMA Start */
	WRITE_IPCS_K_REG(977, 0x80);
	WRITE_IPCS_K_REG(975, 0x80000000);
	WRITE_IPCS_K_REG(981, 0x0);
	WRITE_IPCS_K_REG(978, 0x10000);

	/* Wait for Err INT */
	for (i = 0; i < 100; i++)
		udelay(50);
}

static void ipcs_k_dma_sw_reset(void)
{
	/* DMA Reset & INT Clear */
	WRITE_IPCS_K_REG(10, 0xEFFFFFEF);
	WRITE_IPCS_K_REG(3, 0xFFFFFFFF);
}


int ipcs_pmu_requeset(void)
{
	int ret = 0;
#ifdef CONFIG_PMU_SELECT
	/* DMA Start */
	WRITE_IPCS_K_REG(983, 0x1);
	ret = pmu_wait_for_ack(PMU_IPCS);

	if(ret != 0) {
		return ret;
	}
#endif
	return ret;
}

void ipcs_pmu_clear(void)
{
	WRITE_IPCS_K_REG(983, 0x0);
}

/**
 * @brief	IPCS 의 PMU(Power Management Unit) 설정을 하는 함수
 * @fn      void ipcs_k_pmu_on_off(enum ipcs_k_on_off pmu_type)
 * @param	pmu_type	[in] On/Off
 * @return	void
 *
 * @author	Kyuchun Han, Dongjin Jung
 * @note	kernel layer
 */
void ipcs_k_pmu_on_off(enum ipcs_k_on_off pmu_type)
{
#ifdef CONFIG_PMU_SELECT

	int reval;
	struct clk *clock;
	struct d4_rmu_device *rmu;

	if (pmu_type == IPCS_K_ON) {
		/*printk("************* IPCS_PMU_ON ************* \n");*/
		reval = d4_pmu_check(PMU_IPCS);
		if (reval != 0) {
			reval = ipcs_pmu_requeset();
			if (reval)
				return;

			rmu = d4_rmu_request();

			if (rmu == NULL)
				return;

			d4_pmu_isoen_set(PMU_IPCS, PMU_CTRL_ON);
			d4_sw_isp_reset(rmu, RST_IPCS);
			clock = clk_get(ipcs_k_dev, "ipcs");

			if (clock == -2) {
				if (rmu != NULL)
					kfree(rmu);
				return;
			}

			clk_disable(clock);
			clk_put(clock);
			d4_pmu_scpre_set(PMU_IPCS, PMU_CTRL_ON);
			d4_pmu_scall_set(PMU_IPCS, PMU_CTRL_ON);
			d4_rmu_release(rmu);
		}
	} else {
		/*printk("************* IPCS_PMU_OFF ************* \n");*/
		rmu = d4_rmu_request();
		if (rmu == NULL)
			return;
		d4_pmu_scpre_set(PMU_IPCS, PMU_CTRL_OFF);
		d4_pmu_scall_set(PMU_IPCS, PMU_CTRL_OFF);
		wait_for_stable(PMU_IPCS);

		clock = clk_get(ipcs_k_dev, "ipcs");

		if (clock == -2) {
			if (rmu != NULL)
				kfree(rmu);
			return;
		}

		d4_sw_isp_reset(rmu, RST_IPCS);
		udelay(1);

		clk_enable(clock);
		clk_set_rate(clock, 260000000);
		d4_pmu_isoen_set(PMU_IPCS, PMU_CTRL_OFF);

		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_IPCS);
		/* Added part *************************/
			/*2~4*/
			d4_pmu_bus_reset(PMU_IPCS);
			ipcs_k_dma_rmu_bus();

			/*rmu reset*/
			d4_sw_isp_reset(rmu, RST_IPCS);
			udelay(1);
			d4_sw_isp_reset_release(rmu, RST_IPCS);

			/*2~4*/
			d4_pmu_bus_reset(PMU_IPCS);
			ipcs_k_dma_rmu_bus();

			ipcs_k_dma_sw_reset();
			d4_pmu_bus_reset(PMU_IPCS);
			/**************************************/
			d4_rmu_release(rmu);
	}

#endif
}

/****************************************************************************/



