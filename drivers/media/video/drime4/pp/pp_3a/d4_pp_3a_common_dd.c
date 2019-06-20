/**
 * @file d4_pp_3a_common_dd.c
 * @brief DRIMe4 PP 3a Common Device Driver Function File
 * @author Kyounghwan Moon <kh.moon@samsung.com>,
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
#include <asm/page.h>
#include <asm/cacheflush.h>

#include "d4_pp_3a_common_dd.h"

/**< Register Base Address */
static unsigned int pp_3a_reg_base;

/**< 3A Irq Number */
int pp_3a_irq_num;

/**< 3A Result Mem Base Address */
static unsigned int pp_3a_mem_base;

static int pp_3a_disable_flag = 0;
extern struct drime4_pp_3a *g_pp_3a;

struct completion pp_3a_dma_completion;

static irqreturn_t d4_pp_3a_irq(int irq, void *dev_id)
{
/*	printk("3A DMA Interrupt \n"); */

	pp_3a_set_dma_interrupt_state_clean();
	complete(&pp_3a_dma_completion);

	if (pp_3a_disable_flag) {
		unsigned int sRegValue;

		sRegValue = read_pp_3a_register(PP_3A_REGISTER_000_OFFSET);
		D4_PP_3A_SET_CTRL_0_3A_BLOCK_ENABLE(sRegValue, STATIC_A3_STATUS_DISABLE);
		write_pp_3a_register(PP_3A_REGISTER_000_OFFSET, sRegValue);

		sRegValue = read_pp_3a_register(PP_3A_REGISTER_004_OFFSET);
		D4_PP_3A_SET_DMA_INT_EN(sRegValue, STATIC_A3_STATUS_DISABLE);
		write_pp_3a_register(PP_3A_REGISTER_004_OFFSET, sRegValue);

		pp_3a_disable_flag = 0;
	}

	return IRQ_HANDLED;
}

/******************************************************************************/
/*                        Public Function Implementation                      */
/******************************************************************************/

/**
 * brief 3A register write 함수
 * fn inline void write_pp_3a_register(unsigned int offset, unsigned int val)
 * param  offset[in] 3A 주소 오프셋
 * param  val[in] 레지스터에 쓸 값
 * return 없음
 * author Kyounghwan Moon
 * note - 3A Register를 write 할 때 사용
 */
inline void write_pp_3a_register(unsigned int offset, unsigned int val)
{
    writel(val, (pp_3a_reg_base + offset));
}

/**
 * brief 3A register read 함수
 * fn inline void read_pp_3a_register(unsigned int offset)
 * param  offset[in] 3A 주소 오프셋
 * return register 값 리턴
 * author Kyounghwan Moon
 * note - 3A Register를 read 할 때 사용
 */
inline unsigned int read_pp_3a_register(unsigned int offset)
{
    return readl(pp_3a_reg_base + offset);
}

/**
 * @brief PP 3A interrupt 등록을 위한 함수
 * @fn void pp_3a_com_request_irq(void)
 * @param  없음.
 *
 * @return 없음.
 *
 * @note
 */
void pp_3a_request_irq(void)
{
	/**< completion initialization for synchronization */
	init_completion(&pp_3a_dma_completion);

	/**< Top Interrupt Registration */
	if (request_irq(pp_3a_irq_num, d4_pp_3a_irq, IRQF_DISABLED, "d4_pp_3a", NULL)) {
		printk("PP 3A request_irq failed\n");
		return;
	}
}

/**
 * @brief Still Capture 후 PP 3A를 disable 하는데 사용
 * @fn void pp_3a_disable_after_capture(void)
 * @param  없음.
 *
 * @return 없음.
 *
 * @note
 */
void pp_3a_disable_after_capture(void)
{
	pp_3a_disable_flag = 1;
}

/**
 * @brief PP 3A interrupt 해제를 위한 함수
 * @fn void pp_3a_com_free_irq(void)
 * @param  없음
 *
 * @return 없음
 *
 * @note
 */
void pp_3a_free_irq(void)
{
	/**< Top Interrupt Free */
	free_irq(pp_3a_irq_num, NULL);
}

/**
 * @brief 3A DMA Interrupt flag clear 함수
 * @fn void pp_3a_set_dma_interrupt_state_clean(void)
 * @param  없음
 * @return 없음
 * @author Kyounghwan Moon
 * @note - 3A Interrupt 처리 후 Interrupt flag를 clear하는데 사용
 */
void pp_3a_set_dma_interrupt_state_clean(void)
{
	unsigned int sRegValue = 0;

	D4_PP_3A_SET_DMA_INT_CLEAR(sRegValue, 1);
	write_pp_3a_register(PP_3A_REGISTER_1FC_OFFSET, sRegValue);
}

/**
 * @brief 3A DMA Interrupt flag clear 함수
 * @fn int pp_3a_get_result_address_type(void)
 * @param  없음
 * @return 3A Result Mem address indication Interrupt state를 리턴함
 * @author Kyounghwan Moon
 * @note - 3A Interrupt 처리 시 result mem address offset을 확인 하는데 사용
 */
int pp_3a_get_result_address_type(void)
{
	unsigned int sRegValue = 0;

	sRegValue = read_pp_3a_register(PP_3A_REGISTER_004_OFFSET);
	return D4_PP_3A_GET_SEL_BANK_INDICATE(sRegValue);
}

/**
 * @brief 3A result memory를 cache flush하는 함수
 * @fn void pp_3a_set_result_mem_cache_flush(int area)
 * @param  area[in] cache flush 할 영역을 설정함
 * @return 없음
 * @author Kyounghwan Moon
 * @note - 3A Interrupt 처리 후 result data mem address 영역을 cache flush하는데 사용
 */
void pp_3a_set_result_mem_cache_flush(int area)
{
	unsigned int base = 0;
	unsigned int end = 0;
	switch (area) {
	case 0:
		base = pp_3a_mem_base;
		end = base + SZ_256K - 1;
		dmac_flush_range((void *)base, (void *)end);
		outer_flush_range(base, end);
		break;
	case 1:
		base = pp_3a_mem_base + SZ_256K;
		end = base + SZ_256K - 1;
		dmac_flush_range((void *)base, (void *)end);
		outer_flush_range(base, end);
		break;
	case 2:
		base = pp_3a_mem_base + SZ_512K;
		end = base + SZ_256K - 1;
		dmac_flush_range((void *)base, (void *)end);
		outer_flush_range(base, end);
		break;
	case 3:
		base = pp_3a_mem_base + SZ_512K + SZ_256K;
		end = base + SZ_256K - 1;
		dmac_flush_range((void *)base, (void *)end);
		outer_flush_range(base, end);
		break;
	default:
		break;
	}
}


void pp_3a_wait_dma_done(void)
{
	wait_for_completion_interruptible_timeout(&pp_3a_dma_completion, msecs_to_jiffies(140));
	pp_3a_set_dma_interrupt_state_clean();
}

/**
 * brief 3A 기본 정보 설정 함수
 * fn void pp_3a_set_basic_info(int irq_num)
 * param  irq_num 3A Irq number
 * return 없음
 * author Kyounghwan Moon
 * note - Probe에서 호출하여 common_dd 코드에 사용할 기본 정보를 넘겨주는데 사용
 */
void pp_3a_set_basic_info(unsigned int reg_base, unsigned int mem_base, int irq_num)
{
	pp_3a_irq_num			= irq_num;
	pp_3a_mem_base			= mem_base;
	pp_3a_reg_base			= reg_base;
}

void pp_3a_set_result_mem_addr_info(void)
{
	write_pp_3a_register(PP_3A_REGISTER_170_OFFSET,pp_3a_mem_base);
}


