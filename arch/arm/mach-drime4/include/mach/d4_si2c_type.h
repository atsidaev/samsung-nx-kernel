/**
 * @file d4_si2c_type.h
 * @brief DRIMe4 Slave I2C Type
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_SI2C_TYPE_H
#define _D4_SI2C_TYPE_H

#define SI2C_ENABLE 1
#define SI2C_DISABLE 0

#define SI2C_RSTOPCON 0x04
#define SI2C_TSTOPCON 0x06

enum si2c_tran_mode {
	SI2C_SRECEIVE = 0x00,
	SI2C_STRANSMIT = 0x01
};



#endif /* _D4_SI2C_TYPE_H */
