/*
 * include/linux/melfas_ts.h - platform data structure for MCS Series sensor
 *
 * Copyright (C) 2010 Melfas, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_MELFAS_TS_H
#define _LINUX_MELFAS_TS_H

#define MELFAS_TS_NAME "melfas-ts"

enum {
	None = 0,
	TOUCH_SCREEN,
	TOUCH_KEY
};

struct hdmi_connect_info {
	int is_hdmi_connected;
	int hdmi_res_x;
	int hdmi_res_y;
};

struct flip_info {
	int h_flip;
	int v_flip;
};

struct muti_touch_info {
	int pressure;
	int area;
	int posX;
	int posY;
};

struct melfas_tsi_platform_data {
	uint32_t version;
	int max_x;
	int max_y;
	int max_pressure;
	int max_area;
	int gpio_scl;
	int gpio_sda;
	void (*power)(int on);	/* Only valid in first array entry */
	void (*power_enable)(int en, int pin_num);
	int (*int_enable)(int pin_num);
	int pin_num;
	int pwr_num;
};

typedef enum
{
	MRET_NONE = -1,
	MRET_SUCCESS = 0,
	MRET_FILE_OPEN_ERROR,
	MRET_FILE_CLOSE_ERROR,
	MRET_FILE_FORMAT_ERROR,
	MRET_WRITE_BUFFER_ERROR,
	MRET_I2C_ERROR,
	MRET_MASS_ERASE_ERROR,
	MRET_FIRMWARE_WRITE_ERROR,
	MRET_FIRMWARE_VERIFY_ERROR,
	MRET_UPDATE_MODE_ENTER_ERROR,
	MRET_LIMIT
} eMFSRet_t;

#endif /* _LINUX_MELFAS_TS_H */
