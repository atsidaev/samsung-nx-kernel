/**
 * @file d4_pp_core_dd.c
 * @brief DRIMe4 PP Core Common Device Driver Function File
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>

#include <linux/mutex.h>
#include <asm/cacheflush.h>

#ifdef CONFIG_PMU_SELECT
#include <linux/d4-pmu.h>
#endif

#ifdef CONFIG_RMU_DEV
#include <linux/d4_rmu.h>
#endif

#include <mach/map.h>

#include "d4_pp_core_dd.h"

#include "../../csm/d4_csm_if.h"

/**< PP Core Register Offset */
#define PP_CORE_DD_COMMON_RESET					0x000C
#define PP_CORE_DD_COMMON_INT_STATUS           	0x0020
#define PP_CORE_DD_COMMON_INT_ENABLE           	0x0024
#define PP_CORE_DD_COMMON_INT_CLEAR            	0x0028
#define PP_DD_WDMA_CTRL							0x0314
#define MIPI_DD_WDMA_CTRL						0x0414
#define MIPI_DD_RW_DMA_ENABLE_CTRL  			0x0A24

/**< Register Base Address */
unsigned int pp_core_common_reg_base;
unsigned int pp_core_ctrl_reg_base;
unsigned int pp_core_dma_reg_base;

/**< IRQ Number */
static int pp_core_irq_num;

/**< PP Core Device Info */
static struct device *pp_core_dev;

/**< physical register information */
static unsigned int pp_core_phys_reg_info;

/**< PP Clock */
static struct clk *pp_clk_set;

/**< Completion for synchronization */
struct completion pp_core_wdma_completion;
struct completion pp_core_rdma_completion;
struct completion mipi_rx_wdma_completion;
struct completion mipi_tx_rdma_completion;
static struct completion pp_core_lut_gen_completion;
static struct completion pp_core_lut_load_completion;
static struct completion pp_core_vfpn_write_completion;

static void (*pp_wdma_callback_func)(void);
static void (*pp_rdma_callback_func)(void);

static void (*mipi_wdma_callback_func)(void);
static void (*mipi_rdma_callback_func)(void);

extern void pp_ssif_k_dd_sw_reset(void);
extern struct csm_wdma_state csm_wdma_error_state;

#ifdef PP_CORE_POLL_ENABLE
extern void pp_core_wdma_callback_func(void);
extern void pp_core_rdma_callback_func(void);
#endif

/* private function */
static irqreturn_t d4_pp_core_irq(int irq, void *dev_id);

/* private variable */
static DEFINE_MUTEX(pp_pmu_mutex);


/******************************************************************************/
/*                        Public Function Implementation                      */
/******************************************************************************/

/**
 * @brief PP Core interrupt 등록을 위한 함수
 * @fn void pp_core_com_request_irq(void)
 * @param  없음.
 *
 * @return 없음.
 *
 * @note
 */
void pp_core_com_request_irq(void)
{
	/**< Top Interrupt Registration */
	if (request_irq(pp_core_irq_num, d4_pp_core_irq, IRQF_DISABLED, "d4_pp_core", NULL)) {
		pr_err("PP Core request_irq failed\n");
		return;
	}

	/**< completion initialization for synchronization */
	init_completion(&pp_core_wdma_completion);
	init_completion(&pp_core_rdma_completion);
	init_completion(&mipi_tx_rdma_completion);
	init_completion(&mipi_rx_wdma_completion);
	init_completion(&pp_core_lut_load_completion);
	init_completion(&pp_core_lut_gen_completion);
	init_completion(&pp_core_vfpn_write_completion);

	/**< DMA callback initialization */
#ifdef PP_CORE_POLL_ENABLE
	pp_wdma_set_callback_func(pp_core_wdma_callback_func);
	pp_rdma_set_callback_func(pp_core_rdma_callback_func);
#else
	pp_wdma_set_callback_func(NULL);
	pp_rdma_set_callback_func(NULL);
#endif

	mipi_wdma_set_callback_func(NULL);
	mipi_rdma_set_callback_func(NULL);
}

/**
 * @brief PP Core interrupt 해제를 위한 함수
 * @fn void pp_core_com_free_irq(void)
 * @param  없음
 *
 * @return 없음
 *
 * @note
 */
void pp_core_com_free_irq(void)
{
	/**< Top Interrupt Free */
	free_irq(pp_core_irq_num, NULL);
}

void pp_wdma_set_callback_func(void (*callback_func)(void))
{
	pp_wdma_callback_func = callback_func;
}

void pp_rdma_set_callback_func(void (*callback_func)(void))
{
	pp_rdma_callback_func = callback_func;
}

void pp_core_com_wait_lut_load_done(void)
{
	wait_for_completion(&pp_core_lut_load_completion);
}

int pp_core_com_wait_lut_load_done_with_timeout(unsigned int timemout)
{
	int wait_time = 0;

	wait_time = wait_for_completion_timeout(&pp_core_lut_load_completion, msecs_to_jiffies(timemout));

	if (wait_time == 0)
		return -1;

	return 0;
}

void pp_core_com_wait_lut_generation_done(void)
{
	wait_for_completion(&pp_core_lut_gen_completion);
}

void pp_core_com_wait_vfpn_done(void)
{
	wait_for_completion(&pp_core_vfpn_write_completion);
}

void pp_core_com_wait_init_wdma(void)
{
	init_completion(&pp_core_wdma_completion);
}

void pp_core_com_wait_init_rdma(void)
{
	init_completion(&pp_core_rdma_completion);
}

void pp_core_com_wait_init_lut_load(void)
{
	init_completion(&pp_core_lut_load_completion);
}

void pp_core_com_wait_wdma_done(void)
{
	wait_for_completion(&pp_core_wdma_completion);
}

void pp_core_com_wait_rdma_done(void)
{
	wait_for_completion(&pp_core_rdma_completion);
}

void pp_core_com_set_pp_clock(unsigned int clock_set)
{
	pp_clk_set = clk_get(pp_core_dev, "pp");
	if (pp_clk_set == ERR_PTR(-ENOENT))
			return;
	clk_set_rate(pp_clk_set, clock_set);
}

void pp_core_com_core_reset(void)
{
	unsigned int mipi_int_enable = 0;
	unsigned int mipi_int_status = 0;

	mipi_int_status = __raw_readl(pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_STATUS);
	mipi_int_status &= 0x07800000;

	if (mipi_int_status != 0) {
		mdelay(10);
		mipi_int_status = __raw_readl(pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_STATUS);
		mipi_int_status &= 0x07800000;

		if (mipi_int_status != 0) {
			pr_err("Error: MIPI interrupt exist\n");
			return;
		}
	}

	mipi_int_enable = __raw_readl(pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_ENABLE);
	mipi_int_enable &= 0x07800000;

	if (mipi_int_enable != 0) {
		preempt_disable();
		__raw_writel(0x00000001, (pp_core_common_reg_base + PP_CORE_DD_COMMON_RESET));
		__raw_writel(mipi_int_enable, (pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_ENABLE));
		preempt_enable();
	} else {
		__raw_writel(0x00000001, (pp_core_common_reg_base + PP_CORE_DD_COMMON_RESET));
	}
}

void pp_core_get_phys_reg_info(unsigned int *reg_info)
{
	*reg_info = pp_core_phys_reg_info;
}

void mipi_wdma_set_callback_func(void (*callback_func)(void))
{
	mipi_wdma_callback_func = callback_func;
}

void mipi_rdma_set_callback_func(void (*callback_func)(void))
{
	mipi_rdma_callback_func = callback_func;
}

void mipi_com_wait_wdma_done(void)
{
	wait_for_completion(&mipi_rx_wdma_completion);
}

void mipi_com_wait_rdma_done(void)
{
	wait_for_completion(&mipi_tx_rdma_completion);
}

void pp_rmu_bus_dma(void)
{
	int i = 0;

	/**< DMA start */
	__raw_writel(0x00000080, (pp_core_dma_reg_base + 0x544));
	__raw_writel(0x80000000, (pp_core_dma_reg_base + 0x53C));
	__raw_writel(0x00000000, (pp_core_dma_reg_base + 0x554));
	__raw_writel(0x00010000, (pp_core_dma_reg_base + 0x548));

	/**< Wait for Error Interrupt */
	for (i = 0; i < 100; i++) {
		udelay(50);
	}
}


static void pp_rmu_dma_sw_reset(void)
{
	/**< DMA Reset & Interrupt Clear */
	__raw_writel(0xFFFFFFFF, (pp_core_common_reg_base + 0x0C));
	__raw_writel(0xFFFFFFFF, (pp_core_common_reg_base + 0x28));
}


int pp_pmu_requeset(void)
{
	int ret = 0;
#ifdef CONFIG_PMU_SELECT
	/* DMA Start */
	__raw_writel(0x1, (pp_core_dma_reg_base + 0x55C));
	ret = pmu_wait_for_ack(PMU_PP);

	if(ret != 0) {
		return ret;
	}
#endif
	return ret;
}


void pp_pmu_clear(void)
{
	__raw_writel(0x0, (pp_core_dma_reg_base + 0x55C));
}

/**
 * @brief	PP 의 PMU(Power Management Unit) 설정을 하는 함수
 * @fn      void pp_com_pmu_on_off(enum pp_dd_onoff onoff)
 * @param	onoff	[in] On/Off
 * @return	void
 * @note
 */

void pp_com_pmu_on_off(enum pp_dd_onoff onoff)
{
#ifdef CONFIG_PMU_SELECT
	int reval = 0;
	int reg;
	struct clk *clock;
	struct d4_rmu_device *rmu;

	if (onoff == PP_DD_ON) {
		reval = d4_pmu_check(PMU_PP);
		if (reval != 0) {

			reval = pp_pmu_requeset();
			if (reval)
				return;

			rmu = d4_rmu_request();
			if (rmu == NULL)
				return;

			d4_pmu_isoen_set(PMU_PP, PMU_CTRL_ON);
			d4_sw_isp_reset(rmu, RST_PP);
			clock = clk_get(pp_core_dev, "pp");
			if (clock == ERR_PTR(-ENOENT)) {
				d4_rmu_release(rmu);
				return;
			}
			clk_disable(clock);
			clk_put(clock);

			d4_pmu_scpre_set(PMU_PP, PMU_CTRL_ON);
			d4_pmu_scall_set(PMU_PP, PMU_CTRL_ON);
			d4_rmu_release(rmu);
		}
	} else {
		rmu = d4_rmu_request();
		d4_pmu_scpre_set(PMU_PP, PMU_CTRL_OFF);
		udelay(10);
		d4_pmu_scall_set(PMU_PP, PMU_CTRL_OFF);
		udelay(15);
		wait_for_stable(PMU_PP);
		udelay(10);
		
		clock = clk_get(pp_core_dev, "pp");
		if (clock == ERR_PTR(-ENOENT)) {
			d4_rmu_release(rmu);
			return;
		}

		/**< method 1 */
		d4_sw_isp_reset(rmu, RST_PP);
		udelay(1);

		// 16 MBUS CLOCK
		clk_disable(clock);
		reg = readl(DRIME4_VA_CLOCK_CTRL+0x04);
		reg &= ~(0xF << 8);
		reg |= (0x10 << 8);
		writel(reg, DRIME4_VA_CLOCK_CTRL+0x04);
		clk_enable(clock);
		udelay(1);

		d4_pmu_isoen_set(PMU_PP, PMU_CTRL_OFF);
		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_PP);
		udelay(1);

		/*2~4*/
		d4_pmu_bus_reset(PMU_PP);
		pp_rmu_bus_dma();

		/* RMU Reset*/
		d4_sw_isp_reset(rmu, RST_PP);
		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_PP);
		udelay(1);

		/*2~4*/
		d4_pmu_bus_reset(PMU_PP);
		pp_rmu_bus_dma();

		pp_rmu_dma_sw_reset();
		d4_pmu_bus_reset(PMU_PP);

		clk_disable(clock);
		clk_set_rate(clock, 200000000);
		clk_enable(clock);
		udelay(1);
		d4_rmu_release(rmu);
	}
#endif
}

/******************************************************************************/
/*                        Private Function Implementation                     */
/******************************************************************************/

void pp_core_set_reg_ctrl_base_info(struct pp_core_reg_ctrl_base_info *info)
{
	pp_core_dev				= info->dev_info;
	pp_core_common_reg_base	= info->common_reg_base;
	pp_core_ctrl_reg_base	= info->ctrl_reg_base ;
	pp_core_dma_reg_base	= info->dma_reg_base;

	pp_core_irq_num			= info->irq_num;

	/**< physical register information */
	pp_core_phys_reg_info = info->pp_phys_base_addr;
}

static irqreturn_t d4_pp_core_irq(int irq, void *dev_id)
{
	unsigned int int_status  = 0;
	unsigned int int_disable = 0;
	unsigned int lut_load_int_check  = 0;
	unsigned int wdma_ctrl = 0;

	int_status = __raw_readl(pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_STATUS);

	/**< WDMA Write Done Interrupt Checking */
	if ((int_status & PP_WDMA_FRAME_WRITE_DONE_INT) == PP_WDMA_FRAME_WRITE_DONE_INT) {
		/**< WDMA Disable */
		wdma_ctrl = __raw_readl(pp_core_dma_reg_base + PP_DD_WDMA_CTRL);
		wdma_ctrl &= ~(0x1 << 28);
		__raw_writel(wdma_ctrl, (pp_core_dma_reg_base + PP_DD_WDMA_CTRL));

		/**< Interrupt Disable */
		int_disable = __raw_readl(pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_ENABLE);
		int_disable &= ~PP_WDMA_FRAME_WRITE_DONE_INT;
		__raw_writel(int_disable, (pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_ENABLE));

		/**< Interrupt Clear */
		__raw_writel(PP_WDMA_FRAME_WRITE_DONE_INT, (pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_CLEAR));

		complete(&pp_core_wdma_completion);

#ifdef PP_CORE_POLL_ENABLE
		if (pp_wdma_callback_func != NULL) {
			pp_wdma_callback_func();
		}
#endif
		csm_wdma_error_state.wdma_done_intr = 1;
	}

	/**< Interrupt Disable, WDMA 는 최대한 빠르게 처리 하기 위해 따로 처리 */
		int_disable = __raw_readl(pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_ENABLE);
		int_disable &= ((~int_status) | PP_MIPI_RDMA_FRAME_READ_DONE_INT | PP_MIPI_WDMA_FRAME_WRITE_DONE_INT);
		__raw_writel(int_disable, (pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_ENABLE));

	/**< RDMA read Done Interrupt Checking */
	if ((int_status & PP_RDMA_FRAME_READ_DONE_INT) == PP_RDMA_FRAME_READ_DONE_INT) {

	    /**< Interrupt Clear */
		__raw_writel(PP_RDMA_FRAME_READ_DONE_INT, (pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_CLEAR));

		complete(&pp_core_rdma_completion);

#ifdef PP_CORE_POLL_ENABLE
		if (pp_rdma_callback_func != NULL) {
			pp_rdma_callback_func();
		}
#endif
	}

	/**< DPC LUT Generation Interrupt Checking */
	if ((int_status & PP_DPC_LUT_GENERATION_DONE_INT) == PP_DPC_LUT_GENERATION_DONE_INT) {

		complete(&pp_core_lut_gen_completion);
	}

	/**< VFPN Write Done Interrupt Checking */
	if ((int_status & PP_VFPN_DATA_WRITE_DONE_INT) == PP_VFPN_DATA_WRITE_DONE_INT) {

		complete(&pp_core_vfpn_write_completion);
	}

	lut_load_int_check = (PP_DPC_LUT_LOAD_DONE_INT | PP_BG_LUT_LOAD_DONE_INT |
						  PP_XYS_LUT_LOAD_DONE_INT | PP_CS_LUT_LOAD_DONE_INT |
						  PP_HBR_LUT_LOAD_DONE_INT | PP_VBR_LUT_LOAD_DONE_INT);

	/**< LUT Loading Done Interrupt Checking, Load Done Interrupt 는 동시에 발생 할수 없다고 가정  */
	if ((int_status & lut_load_int_check) != 0) {

		complete(&pp_core_lut_load_completion);
	} else {
		/**< PP WDMA Error checking */
		if ((int_status & (0x1 << 21)) == (0x1 << 21)) {
			csm_wdma_error_state.wdma_error = 1;
			pr_err("PP WDMA Error\n");
		}
	}

	/**< MIPI RDMA read Done Interrupt Checking */
	if ((int_status & PP_MIPI_RDMA_FRAME_READ_DONE_INT) == PP_MIPI_RDMA_FRAME_READ_DONE_INT) {

	    /**< Interrupt Clear */
		__raw_writel(PP_MIPI_RDMA_FRAME_READ_DONE_INT, (pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_CLEAR));

		if (mipi_rdma_callback_func != NULL) {
			mipi_rdma_callback_func();
		}
	}

	/**< MIPI WDMA write Done Interrupt Checking */
	if ((int_status & PP_MIPI_WDMA_FRAME_WRITE_DONE_INT) == PP_MIPI_WDMA_FRAME_WRITE_DONE_INT) {

		/**< Interrupt Clear */
		__raw_writel(PP_MIPI_WDMA_FRAME_WRITE_DONE_INT, (pp_core_common_reg_base + PP_CORE_DD_COMMON_INT_CLEAR));

		if (mipi_wdma_callback_func != NULL) {
			mipi_wdma_callback_func();
		}
	}
	/**< For Debugging */
	/**< printk("Int: %#010x \n", int_status); */
	return IRQ_HANDLED;
}
