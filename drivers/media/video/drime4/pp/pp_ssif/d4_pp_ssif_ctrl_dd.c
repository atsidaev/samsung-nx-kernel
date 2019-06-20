/**
 * @file d4_pp_ssif_ctrl_dd.c
 * @brief DRIMe4 PP Sensor Interface Control Device Driver Function File
 * @author Main : DeokEun Cho <de.cho@samsung.com>
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
#include <asm/cacheflush.h>

#include "d4_pp_ssif_ctrl_dd.h"

/**< Register Base Address */
unsigned int pp_ssif_ctrl_reg_base;
static unsigned int sub_lvds_ctrl_reg_base;

/**< IRQ Number */
static int pp_ssif_irq_num;

/**< PP SSIF Device Info */
static struct device *pp_ssif_dev;

/**< Sensor Clock */
static struct clk *clk_set;

/**< Completion for synchronization */
struct completion pp_ssif_int_completion[NUM_OF_SSIF_DD_INT];

/* private function */
static irqreturn_t d4_pp_ssif_irq(int irq, void *dev_id);

#ifdef PP_SSIF_POLL_ENABLE
extern void pp_ssif_send_interrupt(int intr_type);
#endif

static void (*pp_ssif_int_callback_func[NUM_OF_SSIF_DD_INT])(int) = {
		NULL, /**< SSIF VSYNC */
		NULL, /**< SSIF User Define Interrupt 0 */
		NULL, /**< SSIF User Define Interrupt 1 */
		NULL, /**< SSIF User Define Interrupt 2 */
		NULL, /**< SSIF User Define Interrupt 3 */
		NULL, /**< SSIF User Define Interrupt 4 */
		NULL, /**< SSIF User Define Interrupt 5 */
		NULL, /**< SSIF User Define Interrupt 6 */
		NULL, /**< SSIF VSYNC */
		NULL, /**< BT656 RX Done */
		NULL, /**< BT656 RX Error */
		NULL, /**< MIPI RX Done */
		NULL, /**< MIPI TX Done */
};

void pp_ssif_set_reg_ctrl_base_info(struct pp_ssif_reg_ctrl_base_info *info)
{
	pp_ssif_dev				= info->dev_info;
	pp_ssif_ctrl_reg_base	= info->ctrl_reg_base ;
	sub_lvds_ctrl_reg_base  = info->slvds_reg_base ;

	pp_ssif_irq_num			= info->irq_num;
}

/**
* @brief Sensor Clock 설정을 위한 함수
* @fn void pp_ssif_k_dd_set_sensor_clock(unsigned int clock_set)
* @param  clock_set[in] 설정 하려고 하는 clock 설정 (Hz 단위)
* @return 없음
*
* @author Kim Sung Hoon, DeokEun Cho
* @note
*
*/
void pp_ssif_k_dd_set_sensor_clock(unsigned int clock_set)
{
    clk_set = clk_get(pp_ssif_dev, "ext_tg");
	if (clk_set == ERR_PTR(-ENOENT))
			return;
    clk_set_rate(clk_set, clock_set);
}

void pp_ssif_k_dd_sw_reset(void)
{
	unsigned int ssif_reset = 0;
	ssif_reset = __raw_readl(pp_ssif_ctrl_reg_base);

	ssif_reset &= ~(0x1);
	__raw_writel(ssif_reset, pp_ssif_ctrl_reg_base);

	ssif_reset |= 0x1;
	__raw_writel(ssif_reset, pp_ssif_ctrl_reg_base);

	ssif_reset &= ~(0x1);
	__raw_writel(ssif_reset, pp_ssif_ctrl_reg_base);
}

/**
 * @brief PP SSIF interrupt 등록을 위한 함수
 * @fn void pp_ssif_com_request_irq(void)
 * @param  없음.
 *
 * @return 없음.
 *
 * @note
 */
void pp_ssif_com_request_irq(void)
{
	int i = 0;

	/**< PP SSIF Interrupt Registeration */
	if (request_irq(pp_ssif_irq_num, d4_pp_ssif_irq, IRQF_DISABLED, "d4_pp_ssif", NULL)) {
		pr_err("PP SSIF request_irq failed\n");
		return;
	}

	/**< completion initialization for synchronization */
	for (i = 0; i < NUM_OF_SSIF_DD_INT; i++) {
		init_completion(&pp_ssif_int_completion[i]);
#ifdef PP_SSIF_POLL_ENABLE
		pp_ssif_set_callback_func((enum ssif_dd_interrupt_selection) i, pp_ssif_send_interrupt);
#else
		pp_ssif_set_callback_func((enum ssif_dd_interrupt_selection) i, NULL);
#endif
	}
}

/**
 * @brief PP SSIF interrupt 해제를 위한 함수
 * @fn void pp_core_com_free_irq(void)
 * @param  없음
 *
 * @return 없음
 *
 * @note
 */
void pp_ssif_com_free_irq(void)
{
	/**< Top Interrupt Free */
	free_irq(pp_ssif_irq_num, NULL);
}

/**
* @brief SSIF Interrupt Callbakc Function을 등록 하기 위한 함수
* @fn void pp_ssif_set_callback_func(enum ssif_interrupt_selection selection, void (*callback_func)(void))
* @param  selection[in] 사용 하려고 하는 Interrupt 선택
* @param  callback_func[in] 등록 하려고 하는 callback function
* @return 없음
*
* @author Kim Sung Hoon, DeokEun Cho
* @note   - callback function 은 최대한 짧게 작성 하도록 한다. <br>
* 		  - callback function 안에서는 printk를 사용하지 않도록 한다. <br>
*
*/
void pp_ssif_set_callback_func(enum ssif_dd_interrupt_selection selection, void (*callback_func)(int))
{
	pp_ssif_int_callback_func[selection] = callback_func;
}

void k_pp_ssif_set_interrupt_enable(enum ssif_dd_interrupt_selection selection, enum k_ssif_onoff onoff)
{
	unsigned int interrupt_mask = 0;
	unsigned int interrupt_value = 0;

	interrupt_mask = __raw_readl(pp_ssif_ctrl_reg_base + K_PP_SSIF_INTR_MASK);
	interrupt_value = (0x1 << selection);

	if (onoff == K_SSIF_OFF) {
		interrupt_mask |= interrupt_value;
		__raw_writel(interrupt_mask, (pp_ssif_ctrl_reg_base + K_PP_SSIF_INTR_MASK));
	} else if (onoff == K_SSIF_ON) {
		interrupt_mask &= ~interrupt_value;
		__raw_writel(interrupt_mask, (pp_ssif_ctrl_reg_base + K_PP_SSIF_INTR_MASK));
	}

}

static irqreturn_t d4_pp_ssif_irq(int irq, void *dev_id)
{
	int i = 0;
	unsigned int interrupt_state = 0;

	interrupt_state = __raw_readl(pp_ssif_ctrl_reg_base + K_PP_SSIF_INTR_STATE);

	for (i = 0; i < NUM_OF_SSIF_DD_INT; i++) {
		if ((interrupt_state & (0x1 << i)) == (0x1 << i)) {
			/**< Interrupt Clear */
			interrupt_state |= (0x1 << i);
			__raw_writel(interrupt_state, (pp_ssif_ctrl_reg_base + K_PP_SSIF_INTR_STATE));

			interrupt_state &= ~(0x1 << i);
			__raw_writel(interrupt_state, (pp_ssif_ctrl_reg_base + K_PP_SSIF_INTR_STATE));

			/**< Call Back Function Execution */
			if (pp_ssif_int_callback_func[i] != NULL) {
				pp_ssif_int_callback_func[i](i);
			}
		}
	}
	return IRQ_HANDLED;
}
