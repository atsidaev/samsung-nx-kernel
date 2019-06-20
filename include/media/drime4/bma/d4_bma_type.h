/**
 * @file d4_bma_type.h
 * @brief DRIMe4 BMA Structure Define
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __D4_BMA_TYPE_H__
#define __D4_BMA_TYPE_H__

/**
 * @struct BMA_Buffer_Info
 * @brief Buffer 할당/해제를 위해 사용되는 구조체
 *
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note NONE
 */
struct BMA_Buffer_Info {
   unsigned int addr;	/**<  buffer start address */
   unsigned int size;	/**<  buffer size */
};

#endif   /* __D4_BMA_TYPE_H__ */
