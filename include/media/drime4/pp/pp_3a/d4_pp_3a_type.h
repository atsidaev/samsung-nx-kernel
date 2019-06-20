/**
 * @file d4_pp_3a_type.h
 * @brief DRIMe4 PP 3A Structure&Enumeration Define
 * @author Kyounghwan Moon <kh.moon@samsung.com>,
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __DRIME4_PP_3A_DATA_TYPE_H__
#define __DRIME4_PP_3A_DATA_TYPE_H__

#define	PP_3A_SUCCESS	 0	/**< 동작 결과 Success 인 경우 */
#define	PP_3A_FAIL	-1	/**< 동작 결과 Fail 인 경우 */

/******************************************************************************/
/*                                Enumeration                                 */
/******************************************************************************/

/**
 * @enum pp_3a_onoff
 * @brief ON/OFF및 동작 설정을 위한 Enumeration
 *
 */
enum pp_3a_onoff {
	PP_3A_OFF,		/**< 동작  중지 및 설정 OFF */
	PP_3A_ON		/**< 동작  중지 및 설정 ON*/
};


#endif   /* __DRIME4_PP_3A_DATA_TYPE_H__ */

