/**
 * @file d4_bma_ctrl_dd.c
 * @brief DRIMe4 BMA Function File
 * @author Junkwon Choi <Junkwon.choi@samsung.com>
 * 2012 Samsung Electronics
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

#include <mach/d4_cma.h>
#include "d4_bma_if.h"

static struct device *bma_dev;

/**
 * @brief set device info
 * @fn   void d4_bma_set_dev_info(struct device *info)
 * @param struct device *info
 * @return void
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
void d4_bma_set_dev_info(struct device *info)
{
	bma_dev = info;
}

/**
 * @brief BMA buffer alloc 함수.
 * @fn int d4_bma_alloc_buf(struct BMA_Buffer_Info *info)
 * @param *info[in/out] struct BMA_Buffer_Info
 * @return 실패시 -1, 성공시 0
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
int d4_bma_alloc_buf(struct BMA_Buffer_Info *info)
{
	unsigned int result;

	info->addr = d4_cma_alloc(bma_dev, info->size);

	if (info->addr < 0)
		result = -1;
	else
		result = 0;

	return result;
}

/**
 * @brief BMA buffer free 함수
 * @fn int d4_bma_free_buf(unsigned int addr)
 * @param addr[in] 해제하려는 buffer의 시작주소
 * @return 실패시 -1, 성공시 0
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
int d4_bma_free_buf(unsigned int addr)
{
	int result;
	if (addr)
		result = d4_cma_free(bma_dev, addr);
	else
		result = -1;

	return result;
}
