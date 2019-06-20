/**
 * @file d4_jxr_type.h
 * @brief DRIMe4 JPEG XR data Structure&Enumeration Define
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __D4_JXR_TYPE_H__
#define __D4_JXR_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define JXR_SUCCESS	 0
#define	JXR_FAIL	      (-1)
#define	JXR_IRQ_FAIL	(-2)
#define	JXR_NULL_CALLBACK	(-3)
#define	JXR_ENC_SEMA_TIMEOUT	(-4)
#define JXR_CMA_ALLOC_FAIL (-5)
#define JXR_CMA_FREE_FAIL (-6)
#define JXR_NULL_POINTER (-7)
#define JXR_WRONG_INT_NUM (-8)
#define JXR_SEMA_TIMEOUT (-9)
#define JXR_DEVICE_OPEN_FAIL (-10)
#define JXR_INVALID_WIDTH_HEIGHT (-11)

#define JXR_NULL NULL
#define JXR_ZERO     0
#define JXR_SEL 1

#define JXR_1MS 1000

/**
 * @enum JXR_INT
 * @brief Jxr interrupt mode
 *
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note 	NONE
 */
enum JXR_INT {
   JXR_INT_IMG_COMPLETE = 1,			/* Image Complete Interrupt */
   JXR_INT_FRAME_ERROR = 4,			/* Frame Error Interrupt */
   JXR_INT_TILE_DONE = 5,				/* Tile Done Interrupt	 */
   JXR_INT_MAX = 6					/* Maximum Interrupts */
};


/**
 * @enum JXR_REG_TYPE
 * @brief Register type
 *
 *
 * @author
 */
enum JXR_REG_TYPE {
	JXR_TOP_REG = 0,
	JXR_CTRL_REG
};


/**
 * @struct JXR_PHYS_REG_INFO
 * @brief Physical register information 사용을 위한 structure
 *
 * @author
 */
struct JXR_PHYS_REG_INFO {
	enum JXR_REG_TYPE type;
	unsigned int startAddr; /**< Register physical start address */
	unsigned int size;		 /**< Register size */
};


#ifdef __cplusplus
}
#endif

#endif   /* __D4_JXR_TYPE_H__ */

