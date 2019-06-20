/**
 * @file d4_jpeg_type.h
 * @brief DRIMe4 jpeg Structure & Enumeration Define
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __D4_JPEG_TYPE_H__
#define __D4_JPEG_TYPE_H__

/******************************************************************************/
/*                Enumeration                   */
/******************************************************************************/
#define JPEG_SUCCESS					      0
#define JPEG_FAIL							   (-1)
#define JPEG_IRQ_FAIL						   (-2)
#define JPEG_NULL_CALLBACK				   (-3)
#define JPEG_STREAM_SIZE_OVERFLOW 	   (-9)
#define JPEG_INVALID_INT_NUM		  	 (-12)

#define JPEG_ACC_INT_EN					 (0x04)
#define JPEG_ACC_INT_CLR				 (0x08)
#define JPEG_ACC_INT_STATUS			 (0x0C)

#define JPEG_JP_CMD						(0x400)
#define JPEG_JF_CMD						(0x800)


/**
 * @enum	jpeg_on_off
 * @brief	K_JPEG Device Driver의 On/Off member
 */
enum jpeg_k_on_off {
	K_JPEG_OFF = 0,/**< Off */
	K_JPEG_ON/**< On */
};

/**
 * @enum JPEG_INT
 * @brief JPEG ACC block interrupt bit number.
 *
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note 	NONE
 */
enum JPEG_INT {
   JPEG_ACCINT_TRCMPT = 0,			 /**<  Transfer Complete Interrupt */
   JPEG_ACCINT_ERR = 1,				 /**<  Error Interrupt */
   JPEG_ACCINT_DEND = 2,			 /**<  Decode End Interrupt */
   JPEG_ACCINT_MRKDET = 3,			 /**<  Marker Detect Interrupt */
   JPEG_ACCINT_EEND = 4,			 /**<  Encode Interrupt */
   JPEG_ACCINT_MRKSET = 5,			 /**<  Marker Set Interrupt */
   JPEG_ACCINT_MAX = 6				 /**<  Max Jpeg Interrupts */
};

/**
 * @enum JPEG_REG_TYPE
 * @brief Register type 선택을 위한 Enumeration
 *        Register physical information 정보를 얻어 오기 위한 register 선택 시 사용
 *
 * @author JinHyoung An <jh0913.an@samsung.com>
 */
enum JPEG_REG_TYPE {
	JPEG_TOP_REG,
	JPEG_CTRL_REG
};


/******************************************************************************/
/*                  Structure                   */
/******************************************************************************/


/**
 * @struct JPEG_PHY_REG_INFO
 * @brief Physical register information 사용을 위한 structure
 *
 * @author JinHyoung An <jh0913.an@samsung.com>
 */
struct JPEG_PHY_REG_INFO {
	unsigned int startAddr; /**< Register physical start address */
	unsigned int size;		 /**< Register size */
};

/**
 * @struct JPEG_GET_REG_INFO
 * @brief register type and Physical register information
 *
 * @author JinHyoung An <jh0913.an@samsung.com>
 */
struct JPEG_GET_REG_INFO {
	enum JPEG_REG_TYPE type;
	struct JPEG_PHY_REG_INFO info;
};

#endif   /* __D4_JPEG_TYPE_H__ */

