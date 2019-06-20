/* drivers/input/touchscreen/melfas_ts.c
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

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/melfas_ts.h>

#include <linux/firmware.h>
#include <linux/regulator/consumer.h>
#include <mach/map.h>
#include <linux/gpio.h>
#include <linux/pinctrl/pinconf-drime4.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/consumer.h>

#include <mach/version_information.h>
#include <mach/d4keys.h>

#define TOUCH_FW_UPDATE

#if defined(TOUCH_FW_UPDATE)
#include "MMS100S_FIRMWARE.c"
#endif

//-----------------------------------------
// Model dependent defines
//-----------------------------------------

#define MELFAS_HW_REVISON       0x04
#define MELFAS_FW_VERSION       0x44
#define MELFAS_CONFIG_VERSION   0x04
#define TS_MAX_TOUCH            10
#define MELFAS_CRC_CHECK1_DONE  0xAA
#define MELFAS_CRC_VALUE        0X68


//-----------------------------------------
// Fixed value defines
//-----------------------------------------

#define SLOT_TYPE           1 /* 1: protocol B, 0: protocol A */
#define PRESS_KEY           1
#define RELEASE_KEY         0

#define I2C_RETRY_CNT       2
#define DOWNLOAD_RETRY_CNT  2

#define TS_READ_REGS_LEN    66
#define TS_WRITE_REGS_LEN   16

#define TS_READ_LEN_ADDR                0x0F
#define TS_READ_START_ADDR              0x10
#define TS_READ_CHIP_VENDOR_ID1_ADDR    0xC0
#define TS_READ_CHIP_VENDOR_ID2_ADDR    0xC1
#define TS_READ_HW_VER_ADDR             0xC2
#define TS_READ_FW_VER_ADDR             0xE2
#define TS_READ_CONFIG_VER_ADDR			0xE3
#define TS_OSCILLATOR_STOP_ADDR         0xB0
#define TS_READ_CRC_CHECK_ADDR1         0xB1
#define TS_READ_CRC_CHECK_ADDR2         0xB2


#define MFS_HEADER_		5
#define MFS_DATA_		20480

#define DATA_SIZE 1024
#define CLENGTH 4
#define PACKET_			(MFS_HEADER_ + MFS_DATA_)


/*
 * Config Update Commands
 */
#define ISC_CMD_ENTER_ISC						0x5F
#define ISC_CMD_ENTER_ISC_PARA1					0x01
#define ISC_CMD_ISC_ADDR						0xD5
#define ISC_CMD_ISC_STATUS_ADDR					0xD9

/*
 * ISC Status Value
 */
#define ISC_STATUS_RET_MASS_ERASE_DONE			0X0C
#define ISC_STATUS_RET_MASS_ERASE_MODE			0X08



#if 0
#define TSP_DEBUG_MSG(fmt, arg...)		printk(KERN_EMERG "tsp: " fmt, ##arg)
#else
#define TSP_DEBUG_MSG(fmt, arg...)
#endif


#if SLOT_TYPE
#define REPORT_MT(touch_number, x, y, area, pressure) \
do {     \
	input_mt_slot(ts->input_dev, touch_number);	\
	input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, true);	\
	input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);             \
	input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);             \
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, area);         \
	input_report_abs(ts->input_dev, ABS_MT_PRESSURE, pressure); \
} while (0)
#else
#define REPORT_MT(touch_number, x, y, area, pressure) \
do {     \
	input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, touch_number);\
	input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);             \
	input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);             \
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, area);         \
	input_report_abs(ts->input_dev, ABS_MT_PRESSURE, pressure); \
	input_mt_sync(ts->input_dev);                                      \
} while (0)
#endif

struct melfas_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct melfas_tsi_platform_data *pdata;
	uint32_t flags;
	void (*power)(int onoff, int pin_num);

};


static int tc_mask = 0;

static struct muti_touch_info g_Mtouch_info[TS_MAX_TOUCH];
static struct hdmi_connect_info g_hdmi_info;
static struct flip_info g_flip_info;
static struct melfas_ts_data *g_ts_data;
#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif


#if defined(TOUCH_FW_UPDATE)
static int melfas_ts_check_crc(struct melfas_ts_data *ts);
static int melfas_ts_get_version(struct melfas_ts_data *ts, u8 *val);
static int melfas_ts_firmware_update(struct melfas_ts_data *ts, int reboot);
static eMFSRet_t melfas_ts_firmware_update_isc(struct melfas_ts_data *ts);
static void melfas_ts_reboot(struct melfas_ts_data *ts);
static int melfas_ts_firmware_write(struct melfas_ts_data *ts, const unsigned char *_pBinary_Data);
static int melfas_ts_firmware_verify(struct melfas_ts_data *ts, const unsigned char *_pBinary_Data);
static int melfas_ts_mass_erase(struct melfas_ts_data *ts);
#endif


void d4_melfas_update_touchmask_state(int touchmask_state)
{
	if(touchmask_state & MASK_TOUCH)
		tc_mask = MASK_TOUCH;
	else
		tc_mask = -1;
}

static int melfas_i2c_read(struct i2c_client *client, u16 addr, u16 length, u8 *value)
{
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg;
	int ret = -1;

	msg.addr = client->addr;
	msg.flags = 0x00;
	msg.len = 1;
	msg.buf = (u8 *) &addr;

	ret = i2c_transfer(adapter, &msg, 1);

	if (ret >= 0) {
		msg.addr = client->addr;
		msg.flags = I2C_M_RD;
		msg.len = length;
		msg.buf = (u8 *) value;

		ret = i2c_transfer(adapter, &msg, 1);
	}
/*
	if (ret < 0)
		pr_err("[TSP] : read error : [%d]", ret);
*/
	return ret;
}

static int melfas_i2c_read_only(struct i2c_client *client, char *buf, u16 length, u8 *value)
{
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg;
	int ret = -1;

	msg.addr = client->addr;
	msg.flags = 0x00;
	msg.len = 5;
	msg.buf = (u8 *) buf;

	ret = i2c_transfer(adapter, &msg, 1);
	
	if (ret >= 0) {
		msg.addr = client->addr;
		msg.flags = I2C_M_RD;
		msg.len = length;
		msg.buf = (u8 *) value;

		ret = i2c_transfer(adapter, &msg, 1);			
	}
	return ret;
}

static int melfas_i2c_write(struct i2c_client *client, char *buf, int length)
{
	int i;
	char data[length];

	for (i = 0; i < length; i++)
		data[i] = *buf++;

	i = i2c_master_send(client, (char *) data, length);
	if (i == length)
		return length;
	else {
		pr_err("tsp: write error : [%d]", i);
		return -EIO;
	}
}

#if SLOT_TYPE
static void melfas_ts_release_all_finger(struct melfas_ts_data *ts)
{
	int i;
	TSP_DEBUG_MSG("melfas_ts_release_all_finger()\n");
	if(ts == NULL) {
		return;
	}

	for (i = 0; i < TS_MAX_TOUCH; i++) {
		input_mt_slot(ts->input_dev, i);
		input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);
	}
	input_sync(ts->input_dev);
}
#else
static void melfas_ts_release_all_finger(struct melfas_ts_data *ts)
{
	int i;
	
	TSP_DEBUG_MSG("melfas_ts_release_all_finger()\n");
	if(ts == NULL) {
		return;
	}

	for (i = 0; i < TS_MAX_TOUCH; i++) {
		if (-1 == g_Mtouch_info[i].pressure)
		continue;

		if (g_Mtouch_info[i].pressure == 0)
		input_mt_sync(ts->input_dev);

		if (0 == g_Mtouch_info[i].pressure)
		g_Mtouch_info[i].pressure = -1;
	}
	input_sync(ts->input_dev);
}
#endif


#if defined(TOUCH_FW_UPDATE)
static int melfas_ts_check_crc(struct melfas_ts_data *ts)
{
	int ret = -1;
	uint8_t crc_check[2] = {0, };
	uint8_t i = 0;

	for (i = 0; i < I2C_RETRY_CNT; i++)	{
		ret = melfas_i2c_read(ts->client, TS_READ_CRC_CHECK_ADDR1, 1, &crc_check[0]);		
		if (ret >= 0) {
			if (crc_check[0] != MELFAS_CRC_CHECK1_DONE)
				ret = -1;
			break;
		}
		mdelay(10);		
	}

	TSP_DEBUG_MSG("crc check=0x%02x (fw=0x%02x) \n", crc_check[0], MELFAS_CRC_CHECK1_DONE);
	if (ret < 0) goto done;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = melfas_i2c_read(ts->client, TS_READ_CRC_CHECK_ADDR2, 1, &crc_check[1]);
		if (ret >= 0) {
			if (crc_check[1] != MELFAS_CRC_VALUE)
				ret = -1;
			break;
		}
		mdelay(10);
	}

	TSP_DEBUG_MSG("crc value=0x%02x (fw=0x%02x) \n", crc_check[1], MELFAS_CRC_VALUE);
		
done:

	return ret;
}

static int melfas_ts_get_version(struct melfas_ts_data *ts, u8 *val)
{
	int ret = -1;
	ret = melfas_i2c_read(ts->client, TS_READ_HW_VER_ADDR, 1, &val[0]);
	if (ret >= 0) {
		pr_info("tsp: HW Revision:     0x%02x \n", val[0]);
	}

	if (ret < 0) goto done;
	
	ret = melfas_i2c_read(ts->client, TS_READ_FW_VER_ADDR, 1, &val[1]);
	if (ret >= 0) {
			pr_info("tsp: CORE Revision:   0x%02x \n", val[1]);
	}

	if (ret < 0) goto done;

	ret = melfas_i2c_read(ts->client, TS_READ_CONFIG_VER_ADDR, 1, &val[2]);
	if (ret >= 0) {
			pr_info("tsp: CONFIG Revision: 0x%02x \n", val[2]);
	}

done:
	return ret;
}

static int melfas_ts_firmware_update(struct melfas_ts_data *ts, int reboot)
{
	int ret = 0, update = 0, crc = 0;
	uint8_t fw_ver[3] = {0, };

	if (reboot)
	{
		melfas_ts_reboot(ts);
	}

	// get f/w version.
	ret = melfas_ts_get_version(ts, fw_ver);
	if (ret < 0) 
	{
		pr_err("tsp: melfas_ts_get_version fail! [%d]", ret);
		update = 1;
	}
	else
	{
		if (fw_ver[2] < MELFAS_CONFIG_VERSION)
		{
			// update f/w if version is lower.
			update = 1;
		}
		else
		{
			// update f/w if crc is invalid.
			crc = melfas_ts_check_crc(ts);
			if (crc < 0)
				update = 1;
		}
	}

	if (update)
	{
		eMFSRet_t res = MRET_SUCCESS;

		pr_info("tsp: FW upgrading start... (0x%02x -> 0x%02x, crc=%s) \n", 
			fw_ver[2], MELFAS_CONFIG_VERSION, (crc<0)?"NG":"OK");

		res = melfas_ts_firmware_update_isc(ts);
		if (res != MRET_SUCCESS)
		{
			ret = -1;
			pr_info("tsp: FW upgrading failed (res=%d, config ver=0x%02x) \n", res, MELFAS_CONFIG_VERSION);
		}
		else
		{
			ret = 0;
			pr_info("tsp: FW upgrading done! \n");			
		}
	}
	
	return ret;
}

static eMFSRet_t melfas_ts_firmware_update_isc(struct melfas_ts_data *ts)
{
	eMFSRet_t ret = MRET_SUCCESS;

	melfas_ts_reboot(ts);
	mdelay(30);

	if ((ret = melfas_ts_mass_erase(ts)) && ret != MRET_SUCCESS)
	{
		/* Slave Address가 Default Address(0x48)와 다른  경우에만 필요합니다.*/
		/* Slave Address가 0x48일 경우에는 goto MCSDL_DOWNLOAD_FINISH;만 있으면 됩니다. */

		/* Slave Address != Default Address(0x48) 일 경우 Code start!!! */
		goto MCSDL_DOWNLOAD_FINISH;
	}
	
	melfas_ts_reboot(ts);
	mdelay(30);
	
	if ((ret = melfas_ts_firmware_write(ts, MELFAS_binary)) && ret != MRET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;

	if ((ret = melfas_ts_firmware_verify(ts, MELFAS_binary)) && ret != MRET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;
	
	TSP_DEBUG_MSG("<MELFAS> melfas_ts_firmware_update_FINISHED!!\n");

MCSDL_DOWNLOAD_FINISH:
	melfas_ts_reboot(ts);
	
	return ret;
}

static void melfas_ts_reboot(struct melfas_ts_data *ts)
{
	ts->power(0, ts->pdata->pwr_num);
	mdelay(100);
	ts->power(1, ts->pdata->pwr_num);
	mdelay(100);
}

static int melfas_ts_firmware_write(struct melfas_ts_data *ts, const unsigned char *_pBinary_Data)
{
	int i, k = 0;
	unsigned char write_buffer[MFS_HEADER_ + DATA_SIZE];
	unsigned short int start_addr = 0;
	TSP_DEBUG_MSG("<MELFAS> FIRMARE WRITING...\n");
	while (start_addr * CLENGTH < MFS_DATA_)
	{
		write_buffer[0] = ISC_CMD_ISC_ADDR;
		write_buffer[1] = (unsigned char) ((start_addr) & 0X00FF);
		write_buffer[2] = (unsigned char) ((start_addr >> 8) & 0X00FF);
		write_buffer[3] = 0X00;
		write_buffer[4] = 0X00;

		for (i = 0; i < DATA_SIZE; i++)
			write_buffer[MFS_HEADER_ + i] = _pBinary_Data[i	+ start_addr * CLENGTH];

		mdelay(5);
		if (melfas_i2c_write(ts->client, write_buffer, MFS_HEADER_ + DATA_SIZE) < 0)
			{
				TSP_DEBUG_MSG("<MELFAS> FIRMARE Failed    %d\n", i);
				return MRET_I2C_ERROR;
			}
		mdelay(5);
		k++;
		start_addr = DATA_SIZE * k / CLENGTH;
	}

	return MRET_SUCCESS;
}

static int melfas_ts_firmware_verify(struct melfas_ts_data *ts, const unsigned char *_pBinary_Data)
{

	int i, k = 0;
	unsigned char write_buffer[MFS_HEADER_], read_buffer[DATA_SIZE] = {0, };
	unsigned short int start_addr = 0;

	TSP_DEBUG_MSG("<MELFAS> FIRMARE VERIFY...\n");
	while (start_addr * CLENGTH < MFS_DATA_)
	{
		write_buffer[0] = ISC_CMD_ISC_ADDR;
		write_buffer[1] = (unsigned char) ((start_addr) & 0X00FF);
		write_buffer[2] = 0x40 + (unsigned char) ((start_addr >> 8) & 0X00FF);
		write_buffer[3] = 0X00;
		write_buffer[4] = 0X00;
/*
		if (!melfas_i2c_write(ts->client, write_buffer, MFS_HEADER_))
			{
			TSP_DEBUG_MSG(KERN_EMERG "<MELFAS> FIRMARE varify write Failed    %d\n", i);
			return MRET_I2C_ERROR;
			}
		mdelay(5);
		*/
		if (!melfas_i2c_read_only(ts->client, write_buffer, DATA_SIZE, read_buffer))
			{
				TSP_DEBUG_MSG("<MELFAS> VERIFY Failed\n");			
			return MRET_I2C_ERROR;
			}
		for (i = 0; i < DATA_SIZE; i++)
			if (read_buffer[i] != _pBinary_Data[i + start_addr * CLENGTH])
			{
				return MRET_FIRMWARE_VERIFY_ERROR;
			}
		k++;
		start_addr = DATA_SIZE * k / CLENGTH;

	}
	return MRET_SUCCESS;
}

static int melfas_ts_mass_erase(struct melfas_ts_data *ts)
{
	int i = 0;
	const unsigned char mass_erase_cmd[MFS_HEADER_] =
						{ ISC_CMD_ISC_ADDR, 0, 0xC1, 0, 0 };
	unsigned char read_buffer[4] = { 0, };

	mdelay(5);
	
	if (!melfas_i2c_write(ts->client, (char *)mass_erase_cmd, MFS_HEADER_))
		return MRET_I2C_ERROR;

	mdelay(5);

	while (read_buffer[2] != ISC_STATUS_RET_MASS_ERASE_DONE)
	{
		if (!melfas_i2c_read(ts->client, ISC_CMD_ISC_STATUS_ADDR, 4, read_buffer))
			return MRET_I2C_ERROR;

		if (read_buffer[2] == ISC_STATUS_RET_MASS_ERASE_DONE)
		{
			TSP_DEBUG_MSG("ISC_STATUS_RET_MASS_ERASE_DONE\n");
			return MRET_SUCCESS;
		}
		else if (read_buffer[2] == ISC_STATUS_RET_MASS_ERASE_MODE)
		mdelay(1);
		if (i > 20)
			{
				TSP_DEBUG_MSG("MRET_MASS_ERASE_ERROR\n");
			return MRET_MASS_ERASE_ERROR;
			}
		i++;
	}
	return MRET_SUCCESS;
} 

#endif

void set_ts_hdmi_info(int is_hdmi_connected, int res_x, int res_y)
{
	if (g_ts_data) {
		input_set_abs_params(g_ts_data->input_dev, ABS_MT_POSITION_X, 0, g_ts_data->pdata->max_x + res_x, 0, 0);
		input_set_abs_params(g_ts_data->input_dev, ABS_MT_POSITION_Y, 0, g_ts_data->pdata->max_y + res_y, 0, 0);

		input_set_abs_params(g_ts_data->input_dev, ABS_X, 0, g_ts_data->pdata->max_x + res_x, 0, 0);
		input_set_abs_params(g_ts_data->input_dev, ABS_Y, 0, g_ts_data->pdata->max_y + res_y, 0, 0);

		g_hdmi_info.is_hdmi_connected = is_hdmi_connected;
		g_hdmi_info.hdmi_res_x = res_x;
		g_hdmi_info.hdmi_res_y = res_y;
	}
}

void set_ts_h_flip_info(int h)
{
	g_flip_info.h_flip = h;
}

void set_ts_v_flip_info(int v)
{
	g_flip_info.v_flip = v;
}

static void melfas_ts_get_data(struct melfas_ts_data *ts)
{
	int ret = 0, i;
	uint8_t buf[TS_READ_REGS_LEN] = { 0, };
	int read_num, fingerID, Touch_Type = 0, touchState = 0;
	int temp;
	int temp_x, temp_y;

 	TSP_DEBUG_MSG("melfas_ts_get_data()\n");
 	if (ts == NULL) {
		pr_info("tsp: %s: ts data is NULL \n", __func__);
		return;
 	}

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = melfas_i2c_read(ts->client, TS_READ_LEN_ADDR, 1, buf);
		if (ret >= 0) {
 			TSP_DEBUG_MSG("TS_READ_LEN_ADDR [%d] \n", ret);
 			break; /* i2c success */
		}
		mdelay(1);
	}

	if (ret < 0) {
		pr_info("tsp: %s,%d: i2c read fail[%d] \n", __func__, __LINE__, ret);
		return;
	} else {
		read_num = buf[0];
 		TSP_DEBUG_MSG("read_num[%d] \n",  read_num);
 	}

	if ((0 < read_num) && (read_num < TS_READ_REGS_LEN)) {
		for (i = 0; i < I2C_RETRY_CNT; i++) {
			ret = melfas_i2c_read(ts->client, TS_READ_START_ADDR, read_num, buf);

			if (ret >= 0) {
 				TSP_DEBUG_MSG("TS_READ_START_ADDR [%d] \n", ret);
 				break; /* i2c success */
			}
			mdelay(1);
		}

		if (ret < 0) {
			pr_info("tsp: %s,%d: i2c read fail[%u] \n", __func__, 	__LINE__, ret);
			return;
		} else {
			if (tc_mask == MASK_TOUCH)
				return ;

			for (i = 0; i < read_num; i = i + 6) {
				Touch_Type = (buf[i] >> 5) & 0x03;

				/* touch type is panel */
				if (Touch_Type == TOUCH_SCREEN) {
					fingerID = (buf[i] & 0x0F) - 1;
					touchState = (buf[i] & 0x80);

					g_Mtouch_info[fingerID].posX = ((uint16_t)(buf[i + 1] & 0x0F) << 8 | buf[i + 2]);
					g_Mtouch_info[fingerID].posY = (uint16_t)(buf[i + 1] & 0xF0) << 4 | buf[i + 3];
					g_Mtouch_info[fingerID].area = buf[i + 4];

					if (touchState)
						g_Mtouch_info[fingerID].pressure = buf[i + 5];
					else
						g_Mtouch_info[fingerID].pressure = 0;
				}
			}

			for (i = 0; i < TS_MAX_TOUCH; i++) {
				if (g_Mtouch_info[i].pressure == -1)
					continue;

#if SLOT_TYPE
		#if defined(CONFIG_MACH_D4_NX300)
				temp_x = min(g_Mtouch_info[i].posX, ts->pdata->max_y);
				temp_y = min(g_Mtouch_info[i].posY, ts->pdata->max_x);

				if (g_hdmi_info.is_hdmi_connected) {
					temp_x = ts->pdata->max_x + (temp_x * g_hdmi_info.hdmi_res_x / ts->pdata->max_y);
					temp_y = (temp_y * g_hdmi_info.hdmi_res_y / ts->pdata->max_x);
				}
				else {
					temp = temp_x;
					temp_x = ts->pdata->max_x - temp_y;
					temp_y = temp;						
				}

				if(g_flip_info.h_flip) {
					temp_x = ts->pdata->max_x - temp_x;
				}
				if(g_flip_info.v_flip) {
					temp_y = ts->pdata->max_y - temp_y;
				}
		#elif defined(CONFIG_MACH_D4_NX2000)
				temp_x = min(g_Mtouch_info[i].posX, ts->pdata->max_x);
				temp_y = min(g_Mtouch_info[i].posY, ts->pdata->max_y);

				if (g_hdmi_info.is_hdmi_connected) {
					int temp_math_result_x, temp_math_result_y;
					temp = temp_x;
					
					if (g_hdmi_info.hdmi_res_y > ts->pdata->max_x)
						temp_math_result_x = (temp_x * (g_hdmi_info.hdmi_res_y*100000 / ts->pdata->max_x))/100000;
					else
						temp_math_result_x = (temp_x * (g_hdmi_info.hdmi_res_y / ts->pdata->max_x*100000))/100000;

					if (g_hdmi_info.hdmi_res_x > ts->pdata->max_y)
					 	temp_math_result_y = (temp_y * (g_hdmi_info.hdmi_res_x*100000 / ts->pdata->max_y))/100000;
					else
						temp_math_result_y = (temp_y * (g_hdmi_info.hdmi_res_x / ts->pdata->max_y*100000))/100000;	

					temp_x = g_hdmi_info.hdmi_res_y - temp_math_result_x;
					temp_y = g_hdmi_info.hdmi_res_x - temp_math_result_y;
				}
				else {
					temp_x = ts->pdata->max_x - temp_x;
					temp_y = ts->pdata->max_y - temp_y;						
				}			
		#endif
				if (g_Mtouch_info[i].pressure == 0) {
					/* release event */
					input_mt_slot(ts->input_dev, i);
					input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);
				} else {
					REPORT_MT(i, temp_x, temp_y, g_Mtouch_info[i].area, g_Mtouch_info[i].pressure);
				}
#else
				if (g_Mtouch_info[i].pressure == 0) {
					/* release event */
					input_mt_sync(ts->input_dev);
				} else {
					REPORT_MT(i, g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].area, g_Mtouch_info[i].pressure);
				}
#endif
 				TSP_DEBUG_MSG("Touch ID: %d, State : %d, x: %d, y: %d, z: %d w: %d\n", i, (g_Mtouch_info[i].pressure > 0), temp_x, temp_y, g_Mtouch_info[i].pressure, g_Mtouch_info[i].area);
 
				if (g_Mtouch_info[i].pressure == 0)
					g_Mtouch_info[i].pressure = -1;
			}
			input_sync(ts->input_dev);
		}
	}
}

static irqreturn_t melfas_ts_irq_handler(int irq, void *handle)
{
	struct melfas_ts_data *ts = (struct melfas_ts_data *) handle;
 	TSP_DEBUG_MSG("melfas_ts_irq_handler()\n" );
 
	melfas_ts_get_data(ts);
	return IRQ_HANDLED;
}

static ssize_t
show_melfas_oscillator_stop(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	return size;
}

static ssize_t
store_melfas_oscillator_stop(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{	
	struct melfas_ts_data *ts = (struct melfas_ts_data *)dev_get_drvdata(dev);
	unsigned char write_buffer[2] = {TS_OSCILLATOR_STOP_ADDR, 0x01};

	if (ts == NULL)
		return 0;

	melfas_ts_release_all_finger(ts);
	disable_irq(ts->client->irq);

	melfas_i2c_write(ts->client, write_buffer, 2);
		
	return 0;
}

static DEVICE_ATTR(melfas_oscillator_stop, S_IRUGO | S_IWUSR,
			show_melfas_oscillator_stop, store_melfas_oscillator_stop);



int melfas_ts_create_node(struct device *dev)
{
	int rc;
	if (!dev)
		return -1;

	rc = device_create_file(dev, &dev_attr_melfas_oscillator_stop);
	if (rc) {
		dev_info(dev, "failed to create sysfs d4keys node\n");
		return -1;
	}
	return 0;
}

void melfas_ts_remove_node(struct device *dev)
{
	device_remove_file(dev, &dev_attr_melfas_oscillator_stop);
}


static int melfas_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct melfas_ts_data *ts;
	unsigned long pin_config = 0;	
	int ret = 0, i;

 	TSP_DEBUG_MSG("melfas_ts_probe()\n" );
 	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_info("melfas_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	ts = kmalloc(sizeof(struct melfas_ts_data), GFP_KERNEL);
	if (ts == NULL) {
		pr_info("tsp: %s: failed to create a state of melfas-ts\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	ts->pdata = client->dev.platform_data;

	/*request power pin*/
	ts->pdata->power(ts->pdata->pwr_num);

	/*pwer enable*/
	if (ts->pdata->power_enable)
		ts->power = ts->pdata->power_enable;

	ts->power(0, ts->pdata->pwr_num);
	msleep(10);
	ts->power(1, ts->pdata->pwr_num);

	ts->client = client;

	pin_config = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH, PIN_CONFIG_DRIVE_STRENGTH_X6);

	ret = pin_config_group_set("drime4-pinmux", "i2c4grp", pin_config);
	if (ret < 0) {
		TSP_DEBUG_MSG("Data strength control: Fail \n");
		goto err_input_dev_alloc_failed;	
	}

	/*
	 pinmux_request_gpio(DRIME4_GPIO12(4));
	 gpio_request(DRIME4_GPIO12(4), "");
	 gpio_direction_input(DRIME4_GPIO12(4));
	 ts->client->irq = gpio_to_irq(DRIME4_GPIO12(4));
	 irq_set_irq_type(ts->client->irq, IRQ_TYPE_EDGE_FALLING);
	 */
	ts->client->irq = ts->pdata->int_enable(ts->pdata->pin_num);

	i2c_set_clientdata(client, ts);

	melfas_ts_create_node(&client->dev);
	
#if defined(TOUCH_FW_UPDATE)
	for (i = 0; i < DOWNLOAD_RETRY_CNT; i++) {
		ret = melfas_ts_firmware_update(ts, 1);
		if (ret >= 0)
			break;
		mdelay(10);
		pr_info("tsp: melfas_ts_firmware_update retries=%d \n", (i+1));
	}

	//if (ret < 0) goto err_detect_failed;
#endif

	ts->input_dev = input_allocate_device();
	if (!ts->input_dev) {
		pr_info("tsp: %s: Not enough memory\n", __func__);
		ret = -ENOMEM;
		goto err_input_dev_alloc_failed;
	}
	g_ts_data = ts;

#ifdef SLOT_TYPE
	input_mt_init_slots(ts->input_dev, TS_MAX_TOUCH);
#endif /*SLOT_TYPE*/

	ts->input_dev->name = "melfas-ts";

	__set_bit(EV_ABS, ts->input_dev->evbit);
	__set_bit(EV_KEY, ts->input_dev->evbit);
	__set_bit(BTN_TOUCH, ts->input_dev->keybit);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, ts->pdata->max_x, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, ts->pdata->max_y, 0, 0);

	input_set_abs_params(ts->input_dev, ABS_X, 0, ts->pdata->max_x, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, 0, ts->pdata->max_y, 0, 0);

	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, ts->pdata->max_area, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0,	ts->pdata->max_pressure, 0, 0);

#ifndef SLOT_TYPE
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, TS_MAX_TOUCH-1, 0, 0);
#endif /* ! SLOT_TYPE*/

	ret = input_register_device(ts->input_dev);
	if (ret) {
		pr_info("tsp: %s: Failed to register device\n", __func__);
		ret = -ENOMEM;
		goto err_input_register_device_failed;
	}

	if (ts->client->irq) {
		ret = request_threaded_irq(client->irq, NULL, melfas_ts_irq_handler, IRQF_TRIGGER_FALLING|IRQF_ONESHOT, ts->client->name, ts);
		/*(IRQF_TRIGGER_LOW | IRQF_ONESHOT), ts->client->name, ts);*/

		if (ret != 0) {
			pr_info("tsp: %s: Can't allocate irq %d, ret %d\n", __func__, ts->client->irq, ret);
			ret = -EBUSY;
			goto err_request_irq;
		}
	}

	for (i = 0; i < TS_MAX_TOUCH; i++) /* _SUPPORT_MULTITOUCH_ */
		g_Mtouch_info[i].pressure = -1;

 	TSP_DEBUG_MSG("Start touchscreen. name: %s, irq: %d\n", ts->client->name, ts->client->irq);
 	return 0;

err_request_irq:
	pr_info("tsp: %s: err_request_irq failed\n", __func__);
	free_irq(client->irq, ts);
err_input_register_device_failed:
	pr_info("tsp: %s: err_input_register_device failed\n", __func__);
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
	pr_info("tsp: %s: err_input_dev_alloc failed\n", __func__);

#if 0
#if defined(TOUCH_FW_UPDATE)
err_detect_failed:
	pr_info("tsp: %s: err_after_get_regulator failed_\n", __func__);
#endif
#endif

	kfree(ts);

err_alloc_data_failed:
	pr_info("tsp: %s: err_after_get_regulator failed_\n", __func__);

err_check_functionality_failed:
	pr_info("tsp: %s: err_check_functionality failed_\n", __func__);

	return ret;
}

static int melfas_ts_remove(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

	if(ts == NULL)
	{
		return -1;
	}

	free_irq(client->irq, ts);
	ts->power(0, ts->pdata->pwr_num);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	g_ts_data = NULL;	
	return 0;
}

#ifdef CONFIG_PM
static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

 	TSP_DEBUG_MSG("melfas_ts_suspend()\n" );

	if (ts == NULL) {
		return -1;
	}	

//	ts->power(0, ts->pdata->pwr_num);
	return 0;
}

#define BOOT_BOOTMODE_ADDRESS	(DRIME4_VA_SONICS_MSRAM + 0x1FFE0)
#define BOOTMODE_SNAPSHOT_MAGIC  (0x534e4150) /* snapshot boot */

static int melfas_ts_resume(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);
	int i, ret = 0;
	u32 boot_type = 0;
	
	TSP_DEBUG_MSG("melfas_ts_resume()\n");

	if(ts == NULL) {
		return -1;
	}

//	ts->power(1, ts->pdata->pwr_num);
//	msleep(200);

#if defined(TOUCH_FW_UPDATE)
	boot_type = *(volatile u32 *)BOOT_BOOTMODE_ADDRESS;
	if (boot_type == BOOTMODE_SNAPSHOT_MAGIC) {
		for (i = 0; i < DOWNLOAD_RETRY_CNT; i++) {
			ret = melfas_ts_firmware_update(ts, 0);
			if (ret >= 0)
				break;
			mdelay(10);
			pr_info("tsp: melfas_ts_firmware_update retries=%d \n", (i+1));
		}
	}
#endif

	if (gpio_get_value(ts->pdata->pin_num) == 0) {
		melfas_ts_get_data(ts);
	}

	enable_irq(client->irq);
	return 0;
}
#else
#define melfas_ts_suspend NULL
#define melfas_ts_resume NULL
#endif

static const struct i2c_device_id
		melfas_ts_id[] = { { MELFAS_TS_NAME, 0 }, { } };

static struct i2c_driver melfas_ts_driver = {
	.driver = {
		.name = MELFAS_TS_NAME,
	},
	.id_table = melfas_ts_id, .probe = melfas_ts_probe, .remove = __devexit_p(melfas_ts_remove), .suspend = melfas_ts_suspend, .resume = melfas_ts_resume,
};

static int __devinit melfas_ts_init(void)
{
	return i2c_add_driver(&melfas_ts_driver);
}

static void __exit melfas_ts_exit(void)
{
	i2c_del_driver(&melfas_ts_driver);
}

MODULE_DESCRIPTION("Driver for Melfas MIP Touchscreen Controller");
MODULE_AUTHOR("MinSang, Kim <kimms@melfas.com>");
MODULE_VERSION("0.2");
MODULE_LICENSE("GPL");

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(melfas_ts_init);
#else
fast_dev_initcall(melfas_ts_init);
#endif
module_exit(melfas_ts_exit);


