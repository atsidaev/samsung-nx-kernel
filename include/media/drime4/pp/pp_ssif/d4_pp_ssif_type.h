/**
 * @file d4_pp_ssif_type.h
 * @brief DRIMe4 Sensor Interface Structure&Enumeration Define
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __DRIME4_PP_SSIF_DATA_TYPE_H__
#define __DRIME4_PP_SSIF_DATA_TYPE_H__

/**
 * @enum ssif_onoff
 * @brief SSIF On/ Off 선택을 위한 Enumeration
 *
 * @author Kim Sung Hoon, DeokEun Cho
 */
enum k_ssif_onoff {
	K_SSIF_OFF,	/**< 동작 및 선택 OFF 를 위한 인자 */
	K_SSIF_ON		/**< 동작 및 선택 ON 을 위한 인자 */
};

/**
 * @enum ssif_interrupt_selection
 * @brief SSIF Interrupt 선택을 위한 Enumeration
 *
 * @author Kim Sung Hoon, DeokEun Cho
 */
enum ssif_dd_interrupt_selection {
	SSIF_DD_INT_VSYNC,			/**< SSIF VSYNC */
	SSIF_DD_INT_USER_DEFINE_0, /**< SSIF User Defined Interrupt 0 */
	SSIF_DD_INT_USER_DEFINE_1, /**< SSIF User Defined Interrupt 1 */
	SSIF_DD_INT_USER_DEFINE_2, /**< SSIF User Defined Interrupt 2 */
	SSIF_DD_INT_USER_DEFINE_3, /**< SSIF User Defined Interrupt 3 */
	SSIF_DD_INT_USER_DEFINE_4, /**< SSIF User Defined Interrupt 4 */
	SSIF_DD_INT_USER_DEFINE_5, /**< SSIF User Defined Interrupt 5 */
	SSIF_DD_INT_USER_DEFINE_6, /**< SSIF User Defined Interrupt 6 */
	SSIF_DD_INT_BT656_RX_DONE, /**< BT656 RX Done */
	SSIF_DD_INT_BT656_RX_ERROR,/**< BT656 RX Error */
	SSIF_DD_INT_SUB_LVDS_DONE, /**< Sub-LVDS Done */
	SSIF_DD_INT_MIPI_RX_DONE,  /**< MIPI RX Done */
	SSIF_DD_INT_MIPI_TX_DONE,  /**< MIPI TX Done */
	NUM_OF_SSIF_DD_INT			/**< Interrupt Total Number */
};

#endif   /* __DRIME4_PP_SSIF_DATA_TYPE_H__ */

