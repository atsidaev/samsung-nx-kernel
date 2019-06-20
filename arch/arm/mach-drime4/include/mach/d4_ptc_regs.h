/**
 * @file d4_ptc_regs.h
 * @brief  DRIMe4 PWM Registers Define for Device Driver
 * @author Kyuchun Han <kyuchun.han@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DRIME4_PTC_REGS_H
#define __DRIME4_PTC_REGS_H

#include <mach/d4_reg_macro.h>


#define PTC_MODULE_NAME		"drime4_ptc"


/******************************************************************************/
/*							Register Offset Define						*/
/******************************************************************************/

/**< PTC Registers */
#define PTC_REG_COMPARE_P	(0x00)
#define PTC_REG_COUNT_P		(0x04)
#define PTC_REG_COMPARE_Q	(0x08)
#define PTC_REG_COUNT_Q		(0x0C)
#define PTC_REG_INT_PQ		(0x10)

#define PTC_REG_CTRL_PQ		(0x14)
#define PTC_REG_NR			(0x18)

#define PTC_REG_COMPARE_DF	(0x20)
#define PTC_REG_COUNT_DF	(0x24)
#define PTC_REG_INT_DF		(0x28)
#define PTC_REG_CTRL_DF		(0x2C)

/**< PTC P Register Compare Vale */
#define SET_REG_COMPARE_P(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 15)

#define GET_REG_COMPARE_P(val) \
	GET_REGISTER_VALUE(val, 0, 15)

/**< PTC P Register Count */
#define SET_REG_COUNT_P(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 15)

#define GET_REG_COUNT_P(val) \
	GET_REGISTER_VALUE(val, 0, 15)


/**< PTC Q Register Compare Value */
#define SET_REG_COMPARE_Q(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 15)

#define GET_REG_COMPARE_Q(val) \
	GET_REGISTER_VALUE(val, 0, 15)

/**< PTC Q Register Count */
#define SET_REG_COUNT_Q(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 15)

#define GET_REG_COUNT_Q(val) \
	GET_REGISTER_VALUE(val, 0, 15)


/**< PTC Register Interrupt PQ */
#define SET_EQUAL_CLEAR_P(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)

#define GET_EQUAL_FLAG_P(val) \
	GET_REGISTER_VALUE(val, 0, 1)

#define SET_INT_ENABLE_P(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define GET_INT_ENABLE_P(val) \
	GET_REGISTER_VALUE(val, 2, 1)

#define GET_EQUAL_FLAG_Q(val) \
	GET_REGISTER_VALUE(val, 0, 8)
#define SET_EQUAL_CLEAR_Q(val, x) \
	SET_REGISTER_VALUE(val, x, 9, 1)

#define SET_INT_ENABLE_Q(val, x) \
	SET_REGISTER_VALUE(val, x, 10, 1)
#define GET_INT_ENABLE_Q(val) \
	GET_REGISTER_VALUE(val, 10, 1)

/**< PTC Register Control PQ */
#define SET_REG_SEL_EDGE_P(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 2)

#define GET_REG_SEL_EDGE_P(val) \
	GET_REGISTER_VALUE(val, 0, 2)

#define SET_REG_CLEAR_EN_P(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)

#define GET_REG_CLEAR_EN_P(val) \
	GET_REGISTER_VALUE(val, 2, 1)

#define SET_REG_FCLEAR_P(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)

#define SET_REG_COUNT_EN_P(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)

#define GET_REG_COUNT_EN_P(val) \
	GET_REGISTER_VALUE(val, 4, 1)

#define SET_REG_CLEAR_EN_Q(val, x) \
	SET_REGISTER_VALUE(val, x, 10, 1)

#define GET_REG_CLEAR_EN_Q(val) \
	GET_REGISTER_VALUE(val, 10, 1)

#define SET_REG_FCLEAR_Q(val, x) \
	SET_REGISTER_VALUE(val, x, 11, 1)

#define SET_REG_COUNT_EN_Q(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 1)

#define GET_REG_COUNT_EN_Q(val) \
	GET_REGISTER_VALUE(val, 12, 1)


/**< PTC Register NR */
#define SET_REG_NR(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 16)

#define GET_REG_NR(val) \
	GET_REGISTER_VALUE(val, 0, 16)


/**< PTC Register Compare df */
#define SET_REG_COMPARE_DF(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 17)

#define GET_REG_COMPARE_DF(val) \
	GET_REGISTER_VALUE(val, 0, 17)

/**< PTC Register count df */
#define SET_REG_COUNT_DF(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 16)

#define GET_REG_COUNT_DF(val) \
	GET_REGISTER_VALUE(val, 0, 16)

#define GET_REG_COUNT_DF_CHECK(val) \
	GET_REGISTER_VALUE(val, 16, 11)


/**< PTC Register int etc */
#define GET_REG_REV_FLAG(val) \
	GET_REGISTER_VALUE(val, 0, 1)

#define SET_REG_REV_FLAG_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)

#define SET_REG_INT_EN_REV(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)

#define GET_REG_INT_EN_REV(val) \
	GET_REGISTER_VALUE(val, 2, 1)

#define GET_REG_EQUAL_FLAG_DF(val) \
	GET_REGISTER_VALUE(val, 3, 1)

#define SET_REG_EQUAL_FLAG_DF(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)

#define SET_REG_INT_EN_DF(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)

#define GET_REG_INT_EN_DF(val) \
	GET_REGISTER_VALUE(val, 5, 1)


/**< PTC Register Control etc */
#define SET_REG_REV_CHECK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)

#define GET_REG_REV_CHECK_EN(val) \
	GET_REGISTER_VALUE(val, 0, 1)

#define SET_REG_CLEAR_EN_DF(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)

#define GET_REG_CLEAR_EN_DF(val) \
	GET_REGISTER_VALUE(val, 2, 1)

#define SET_REG_FCLEAR_DF(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)

#define SET_REG_COUNT_EN_DF(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)

#define GET_REG_COUNT_EN_DF(val) \
	GET_REGISTER_VALUE(val, 4, 1)

#endif /* __DRIME4_PTC_REGS_H */

