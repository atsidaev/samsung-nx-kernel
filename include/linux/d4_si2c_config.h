/**
 * @file d4_si2c_config.h
 * @brief DRIMe4 Slave I2C Structure&Enumeration Define
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_SI2C_CONFIG_H
#define _D4_SI2C_CONFIG_H


/**
 * @wait_time	: setup value for Waiting Time
 * @ret_val 	: return value
 */
struct d4_si2c_config {
	int wait_time;
	int ret_val;

};

#endif /* _D4_SI2C_CONFIG_H */
