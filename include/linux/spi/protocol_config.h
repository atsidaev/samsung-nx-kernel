/**
 * @file protocol_config.h
 * @brief DRIMe4 Slave Protocol Config Define
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _PROTOCOL_CONFIG_H
#define _PROTOCOL_CONFIG_H



struct protocol_platform_data {
	int int_pin_num; /**< Trigger Pin Number */
	int protocol_ch;		 /**< Protocol Slave Channel */
};


void protocol_data_register(struct protocol_platform_data *info);

#endif /* _PROTOCOL_CONFIG_H */
