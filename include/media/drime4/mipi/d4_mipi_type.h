/**
 * @file d4_mipi_type.h
 * @brief DRIMe4 MIPI Structure & Enumeration Define
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __D4_MIPI_TYPE_H__
#define __D4_MIPI_TYPE_H__

#include <mach/d4_reg_macro.h>

#define	MIPI_SUCCESS                    0
#define	MIPI_FAIL                       (-1)

#define	MIPI_CSIM_SUCCESS               0
#define	MIPI_CSIM_IRQ_FAIL              (-1)
#define MIPI_CSIM_NULL_CALLBACK         (-2)
#define MIPI_CSIM_INVALID_INT_NUM       (-3)
#define	MIPI_CSIM_P_FULL_ERROR          (-4)
#define	MIPI_CSIM_H_FULL_ERROR          (-5)

#define	MIPI_CSIS_SUCCESS               0
#define	MIPI_CSIS_IRQ_FAIL              (-1)
#define MIPI_CSIS_NULL_CALLBACK         (-2)
#define MIPI_CSIS_INVALID_INT_NUM       (-3)
#define	MIPI_CSIS_ID_ERROR              (-4)
#define	MIPI_CSIS_CRC_ERROR             (-5)
#define	MIPI_CSIS_ECC_ERROR             (-6)
#define	MIPI_CSIS_FIFO_OVERFLOW         (-7)
#define	MIPI_CSIS_LOST_FE_ERROR         (-8)
#define	MIPI_CSIS_LOST_FS_ERROR         (-9)
#define	MIPI_CSIS_SOT_HS_ERROR          (-10)
#define	MIPI_CSIS_ODD_AFTER_ERROR       (-11)
#define	MIPI_CSIS_ODD_BEFORE_ERROR      (-12)
#define	MIPI_CSIS_EVEN_AFTER_ERROR      (-13)
#define	MIPI_CSIS_EVEN_BEFORE_ERROR     (-14)

#define MIPI_INITIAL     0
#define MIPI_NULL   NULL

#define MIPI_RESULT int

/******************************************************************************/
/*                          Register Offset Define                            */
/******************************************************************************/

/**< MIPI CSIM Register */
#define MIPI_CSIM_INT_SRC                   0x002C
#define MIPI_CSIM_INT_MSK                   0x0030

/**< MIPI CSIS Register */
#define MIPI_CSIS_INT_MSK                   0x0010
#define MIPI_CSIS_INT_SRC                   0x0014


/******************************************************************************/
/*                        Register Structure Define                           */
/******************************************************************************/

/**< MIPI CSIM Register */

#define D4_MIPI_CSIM_INT_SRC_P_FULL(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_MIPI_CSIM_INT_SRC_H_FULL(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_MIPI_CSIM_INT_SRC_FRAME_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)
#define D4_MIPI_CSIM_INT_SRC_SW_RST_REL(val, x) \
    SET_REGISTER_VALUE(val, x, 30, 1)
#define D4_MIPI_CSIM_INT_SRC_PLL_STABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 31, 1)

#define D4_MIPI_CSIM_INT_MSK_P_FULL(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_MIPI_CSIM_INT_MSK_H_FULL(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_MIPI_CSIM_INT_MSK_FRAME_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)
#define D4_MIPI_CSIM_INT_MSK_SW_RST_REL(val, x) \
    SET_REGISTER_VALUE(val, x, 30, 1)
#define D4_MIPI_CSIM_INT_MSK_PLL_STABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 31, 1)


/**< MIPI CSIS Register */

#define D4_MIPI_CSIS_INT_MSK_ERR_ID(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_MIPI_CSIS_INT_MSK_ERR_CRC(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_MIPI_CSIS_INT_MSK_ERR_ECC(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_MIPI_CSIS_INT_MSK_ERR_OVER(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define D4_MIPI_CSIS_INT_MSK_ERR_LOST_FE(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_MIPI_CSIS_INT_MSK_ERR_LOST_FS(val, x) \
    SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_MIPI_CSIS_INT_MSK_ERR_SOT_HS(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_MIPI_CSIS_INT_MSK_ODD_AFTER(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 1)
#define D4_MIPI_CSIS_INT_MSK_ODD_BEFORE(val, x) \
    SET_REGISTER_VALUE(val, x, 29, 1)
#define D4_MIPI_CSIS_INT_MSK_EVEN_AFTER(val, x) \
    SET_REGISTER_VALUE(val, x, 30, 1)
#define D4_MIPI_CSIS_INT_MSK_EVEN_BEFORE(val, x) \
    SET_REGISTER_VALUE(val, x, 31, 1)


#define D4_MIPI_CSIS_INT_SRC_ERR_ID(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_MIPI_CSIS_INT_SRC_ERR_CRC(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_MIPI_CSIS_INT_SRC_ERR_ECC(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_MIPI_CSIS_INT_SRC_ERR_OVER(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define D4_MIPI_CSIS_INT_SRC_ERR_LOST_FE(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_MIPI_CSIS_INT_SRC_ERR_LOST_FS(val, x) \
    SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_MIPI_CSIS_INT_SRC_ERR_SOT_HS(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 4)
#define D4_MIPI_CSIS_INT_SRC_ODD_AFTER(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 1)
#define D4_MIPI_CSIS_INT_SRC_ODD_BEFORE(val, x) \
    SET_REGISTER_VALUE(val, x, 29, 1)
#define D4_MIPI_CSIS_INT_SRC_EVEN_AFTER(val, x) \
    SET_REGISTER_VALUE(val, x, 30, 1)
#define D4_MIPI_CSIS_INT_SRC_EVEN_BEFORE(val, x) \
    SET_REGISTER_VALUE(val, x, 31, 1)




/**
 * @enum MIPI_CSIM_INT_SRC_SEL
 * @brief MIPI CSIM Interrupt Source/Clear 결정
 *
 * @author Gunwoo nam <gunwoo.nam@samsung.com>
 * @note 	INT발생시 해당 bit가 1로 Set되며 Clear시에 1로 Set된 bit에 다시 1을 Write함
 */
enum MIPI_CSIM_INT_SRC_SEL {
   MIPI_CSIM_INT_P_FULL			= 0,	/**< MIPI CSIM Payload full INT  */
   MIPI_CSIM_INT_H_FULL,				/**< MIPI CSIM Header full INT  */
   MIPI_CSIM_INT_FRAME_DONE		= 24,	/**< MIPI CSIM Frame Done INT  */
   MIPI_CSIM_INT_SW_RST_RELEASE	= 30,	/**< MIPI CSIM Software Reset is released INT  */
   MIPI_CSIM_INT_PLL_STABLE		= 31,	/**< MIPI CSIM Software Reset is released INT  */
   MIPI_CSIM_INT_ALL,
};

/**
 * @enum MIPI_INT_MSK_ON_OFF
 * @brief MIPI Interrupt MASK 결정
 *
 * @author Gunwoo nam <gunwoo.nam@samsung.com>
 * @note 	MSK Enable시 해당 INT 미발생
 */
enum MIPI_INT_MSK_ON_OFF {
	MIPI_INT_MSK_DISABLE = 0,	 /**< MIPI CSIM INT MSK Disable  */
	MIPI_INT_MSK_ENABLE,		 /**< MIPI CSIM INT MSK Enable  */
};

/**
 * @enum MIPI_CSIS_INT_SRC_SEL
 * @brief MIPI CSIS Interrupt Source/Clear 결정
 *
 * @author Gunwoo nam <gunwoo.nam@samsung.com>
 * @note 	INT발생시 해당 bit가 1로 Set되며 Clear시에 1로 Set된 bit에 다시 1을 Write함
 */
enum MIPI_CSIS_INT_SRC_SEL {
   MIPI_CSIS_INT_ERR_ID             = 0,    /**< Unknown ID Err  */
   MIPI_CSIS_INT_ERR_CRC,                   /**< CRC Err  */
   MIPI_CSIS_INT_ERR_ECC,                   /**< ECC Err  */
   MIPI_CSIS_INT_ERR_OVER,                  /**< Image FIFO overflow  */
   MIPI_CSIS_INT_ERR_LOST_FE,               /**< Lost of Frame End packet  */
   MIPI_CSIS_INT_ERR_LOST_FS,               /**< Lost of Frame Start packet  */
   MIPI_CSIS_INT_ERR_SOT_HS         = 12,   /**< Start of transmission error  */
   MIPI_CSIS_INT_ERR_ODD_AFTER      = 28,   /**< Non image data are received at odd frame and Arter image  */
   MIPI_CSIS_INT_ERR_ODD_BEFORE,            /**< Non image data are received at odd frame and Before image  */
   MIPI_CSIS_INT_ERR_EVEN_AFTER,            /**< Non image data are received at even frame and Arter image  */
   MIPI_CSIS_INT_ERR_EVEN_BEFORE,           /**< Non image data are received at even frame and Before image  */
   MIPI_CSIS_INT_ALL,
};

#endif   /* __D4_MIPI_TYPE_H__ */
