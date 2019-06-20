/**
 * @file d4_ep_ioctl.h
 * @brief DRIMe4 EP IOCTL Interface Header File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_EP_IOCTL_H_
#define D4_EP_IOCTL_H_

#include "d4_ep_type.h"

/*
 * EP IOCTL Debug Message Macro
 */

/**< EP IOCTL 코드에서 디버그 메세지 출력 (TRUE: 출력, FALSE: 출력안함) */
/* #define EP_IOCTL_DEBUG_MSG_ON TRUE */
#define EP_IOCTL_ERROR_MSG_ON TRUE

#ifdef EP_IOCTL_DEBUG_MSG_ON
	#define EP_IOCTL_DEBUG_MSG(format, args...) pr_debug(format, ##args)
#else
	#define EP_IOCTL_DEBUG_MSG(format, args...)
#endif

#ifdef EP_IOCTL_ERROR_MSG_ON
	#define EP_IOCTL_ERROR_MSG(format, args...) pr_err(format, ##args)
#else
	#define EP_IOCTL_ERROR_MSG(format, args...)
#endif

#define EP_IOCTL_CMD_MAGIC		'h'

/* UDD *******************************************************/
#define EP_IOCTL_GET_PHYS_REG_INFO	_IOR(EP_IOCTL_CMD_MAGIC, 100, struct ep_reg_info)	/**< EP Sub-block IP들의 레지스터 주소를 가져오는 인터페이스 */

#define EP_IOCTL_INTR_WAIT					_IOW(EP_IOCTL_CMD_MAGIC, 6, struct ep_k_intr_wait_info)	/**< EP의 특정 Core Interrupt 발생시 까지 대기, 주의: Timeout 발생시 음수값(-ETIMEDOUT) 반환 */
#define EP_IOCTL_INTR_WAIT_QUEUE_INIT		_IOW(EP_IOCTL_CMD_MAGIC, 7, struct ep_k_intr_wait_info)	/**< EP_IOCTL_INTR_WAIT 인터페이스에서 사용하는 Wait Queue를 초기화 */
#define EP_IOCTL_DMA_INTR_WAIT				_IOW(EP_IOCTL_CMD_MAGIC, 8, struct ep_k_dma_intr_wait_info)	/**< EP의 특정 DMAInterrupt 발생시 까지 대기, 주의: Timeout 발생시 음수값(-ETIMEDOUT) 반환 */
#define EP_IOCTL_DMA_INTR_WAIT_QUEUE_INIT	_IOW(EP_IOCTL_CMD_MAGIC, 9, struct ep_k_dma_intr_wait_info)	/**< EP_IOCTL_DMA_INTR_WAIT 인터페이스에서 사용하는 Wait Queue를 초기화 */

#define EP_IOCTL_PATH_INIT			_IO(EP_IOCTL_CMD_MAGIC, 20)
#define EP_IOCTL_PATH_TRY_INIT		_IO(EP_IOCTL_CMD_MAGIC, 21)
#define EP_IOCTL_PATH_OPEN			_IOW(EP_IOCTL_CMD_MAGIC, 22, struct ep_k_path_info)
#define EP_IOCTL_PATH_CLOSE			_IOW(EP_IOCTL_CMD_MAGIC, 23, int)
#define EP_IOCTL_PATH_GET_DESCRIPTOR		_IOWR(EP_IOCTL_CMD_MAGIC, 24, struct ep_path_desc)
#define EP_IOCTL_PATH_BLOCK_FRAME_START		_IOW(EP_IOCTL_CMD_MAGIC, 25, struct ep_k_path_block_start_info)
#define EP_IOCTL_PATH_BLOCK_TILE_START		_IOW(EP_IOCTL_CMD_MAGIC, 26, struct ep_k_path_block_start_info)

#define EP_IOCTL_UDD_LOCK		_IO(EP_IOCTL_CMD_MAGIC, 40)
#define EP_IOCTL_UDD_UNLOCK		_IO(EP_IOCTL_CMD_MAGIC, 41)

#define EP_IOCTL_SET_CLK_RATE	_IOW(EP_IOCTL_CMD_MAGIC, 50, int)

#endif /* D4_EP_IOCTL_H_ */
