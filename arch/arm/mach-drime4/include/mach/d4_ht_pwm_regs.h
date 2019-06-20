/**
 * @file d4_ht_pwm_regs.h
 * @brief  DRIMe4 PWM Registers Define for Device Driver
 * @author Kyuchun Han <kyuchun.han@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DRIME4_HT_PWM_REGS_H
#define __DRIME4_HT_PWM_REGS_H

#include <mach/d4_reg_macro.h>


#define PWM_MODULE_NAME		"drime4_pwm"


/******************************************************************************/
/*							Register Offset Define						*/
/******************************************************************************/

/**< HT PWM Registers */
#define PWM_CH_CTRL			(0x00)
#define PWM_CH_CONFIG		(0x04)
#define PWM_CH_PRE			(0x08)
#define PWM_CH_CYCLE1		(0x0C)
#define PWM_CH_CYCLE2		(0x10)


/**< PWM CHANNEL CONTROL REGISTER */
#define PWM_CH_CTRL_START(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PWM_CH_CTRL_INTSTATUS(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PWM_CH_CTRL_INTCLEAR(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PWM_CH_CTRL_CONTINUE(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define PWM_CH_CTRL_INVERT(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)

#define PWM_CH_CTRL_GET_START(val) \
	SET_REGISTER_VALUE(val, 0, 1)
#define PWM_CH_CTRL_GET_INTSTATUS(val) \
	SET_REGISTER_VALUE(val, 1, 1)
#define PWM_CH_CTRL_GET_INTCLEAR(val) \
	SET_REGISTER_VALUE(val, 2, 1)
#define PWM_CH_CTRL_GET_CONTINUE(val) \
	SET_REGISTER_VALUE(val, 3, 1)
#define PWM_CH_CTRL_GET_INVERT(val) \
	GET_REGISTER_VALUE(val, 4, 1)



/**< PWM CHANNEL CONFIGURE REGISTER */
#define PWM_CH_CONFIG_MODE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 4)
#define PWM_CH_CONFIG_MODE0(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define PWM_CH_CONFIG_MODE1(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define PWM_CH_CONFIG_MODE2(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define PWM_CH_CONFIG_MODE3(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define PWM_CH_CONFIG_INTEN(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define PWM_CH_CONFIG_INTEN1(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define PWM_CH_CONFIG_INTEN2(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)
#define PWM_CH_CONFIG_NUM_CYCLE2(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)
#define PWM_CH_CONFIG_NUM_CYCLE1(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 8)


/**< PWM CHANNEL PRE REGISTER */
#define PWM_CH_PRE_SET1(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 16)
#define PWM_CH_PRE_SET2(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 16)


/**< PWM CHANNEL CYCLE1 REGISTER */
#define PWM_CH_CYCLE1_PERIOD(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 16)
#define PWM_CH_CYCLE1_DUTY(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 16)

/**< PWM CHANNEL CYCLE2 REGISTER */
#define PWM_CH_CYCLE2_PERIOD(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 16)
#define PWM_CH_CYCLE2_DUTY(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 16)


#define PWM_CH_EXTIN_SET(val, x, num) \
	SET_REGISTER_VALUE(val, x, num, 4)
#endif /* __DRIME4_HT_PWM_REGS_H */

