/**
 * @file d4_si2c_regs.h
 * @brief  DRIMe4 Slave I2C Registers Define for Device Driver
 * @author Kyuchun Han <kyuchun.han@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DRIME4_SI2C_REGS_H
#define __DRIME4_SI2C_REGS_H

#include <mach/d4_reg_macro.h>


/******************************************************************************/
/*							Register Offset Define						*/
/******************************************************************************/

/**< Slave I2C Registers */
#define SI2C_CON				(0x00)
#define SI2C_STAT				(0x04)
#define SI2C_ADD				(0x08)
#define SI2C_DS					(0x0C)
#define SI2C_STOP				(0x10)
#define SI2C_HOLDMODE	(0x20)
#define SI2C_HOLDSET		(0x24)
#define SI2C_RESET			(0x80)

/**< SLAVE I2C CONTROL REGISTER */
#define SI2C_CON_SCALE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 4)
#define SI2C_CON_IRQPEND(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define SI2C_CON_IRQEN(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define SI2C_CON_TXDIV_16(val) \
	SET_REGISTER_VALUE(val, 0, 6, 1)
#define SI2C_CON_TXDIV_256(val) \
	SET_REGISTER_VALUE(val, 1, 6, 1)
#define SI2C_CON_ACKEN(val, x) \
	SET_REGISTER_VALUE(val, x, 7, 1)
#define SI2C_CON_IRQCLEAR(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)



/**< SLAVE I2C CONTROL STATUS REGISTER */
#define SI2C_STAT_LASTBIT(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define SI2C_STAT_ADDR0(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define SI2C_STAT_ASSLAVE(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define SI2C_STAT_ARBITR(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define SI2C_STAT_TXRXEN(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define SI2C_STAT_BUSBUSY(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define SI2C_STAT_MODESEL(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 2)


#define SI2C_STAT_GET_LASTBIT(val) \
	SET_REGISTER_VALUE(val, 0, 1)
#define SI2C_STAT_GET_ADDR0(val) \
	SET_REGISTER_VALUE(val, 1, 1)
#define SI2C_STAT_GET_ASSLAVE(val) \
	SET_REGISTER_VALUE(val, 2, 1)
#define SI2C_STAT_GET_ARBITR(val) \
	SET_REGISTER_VALUE(val, 3, 1)
#define SI2C_STAT_GET_TXRXEN(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define SI2C_STAT_GET_BUSBUSY(val) \
	SET_REGISTER_VALUE(val, 5, 1)
#define SI2C_STAT_GET_MODESEL(val) \
	SET_REGISTER_VALUE(val, 6, 2)


#define SI2C_STOP_CLOCK_RELEASE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define SI2C_STOP_DATA_RELEASE(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define SI2C_STAT_ACK_GEN(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)

#define SI2C_STOP_GET_CLOCK_RELEASE(val) \
	SET_REGISTER_VALUE(val, 0, 1)
#define SI2C_STOP_GET_DATA_RELEASE(val) \
	SET_REGISTER_VALUE(val, 1, 1)
#define SI2C_STAT_GET_ACK_GEN(val) \
	SET_REGISTER_VALUE(val, 2, 1)

#endif
