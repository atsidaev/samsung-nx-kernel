/*
 * A030CCN01.3 LCD panel driver.
 *
 * Author: yonghoon <yh0302.jo@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/hs_spi.h>
#include <mach/dp/d4_dp.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/pinconf-drime4.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/consumer.h>


#define CALC_PARAM_CNT(x)	((sizeof(x) / 2) - 1)
/*
#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

#define MIN_BRIGHTNESS		0
#define MAX_BRIGHTNESS		10
*/
static struct d4_lcd smd_wvga_amoled_data = { .h_size = 480, .v_size = 800, .freq = 60,
		.lcd_data_width = DATA_WIDTH_24, .type = TYPE_STRIPE,
		.even_seq = SEQ_BGR, .odd_seq = SEQ_BGR, .timing = {
		.total_h_size = 524,
		.total_v_size = 857,
		.h_sync_rise = 0,
		.h_sync_fall = 10,
		.v_sync_rise = 0,
		.v_sync_fall = 2,
		.buf_read_h_start = 20,
		.enable_h_start = 28,
		.enable_h_end = 508,
		.enable_v_start = 3,
		.enable_v_end = 800,
		.inv_dot_clk = 0,
		.inv_enable_clk = 1,
		.inv_h_sync = 1,
		.inv_v_sync = 1,
	}, };

struct smd_wvga_amoled {
	struct device *dev;
    struct hs_spi_data *spi;
};

struct lcd_panel_spi_write_info {
	unsigned int	data_cnt;
	void			*data_buf;
};



/**< LCD panel control value */

static const unsigned short panel_condition_set_table[] = {
		0x0F8,
		0x101, 0x127, 0x127, 0x107, 0x107, 0x154, 0x19F, 0x163, 0x186, 0x11A,
		0x133, 0x10D, 0x100, 0x100
};

static const unsigned short display_condition_1_set_table[] = {
		0x0F2,
		0x102, 0x103, 0x137, 0x11C, 0x111 /**< VBP:3, VHB:55, HBP:28, HFP:17 */
};

static const unsigned short display_condition_2_set_table[] = {
		0x0F7,
		0x100, 0x100, 0x120
};

static const unsigned short gamma_set_table[] = {
		0x0FA,
		0x102, 0x177, 0x168, 0x177, 0x180, 0x190, 0x17D, 0x18F, 0x19C, 0x18C,
		0x190, 0x19B, 0x18F, 0x1AD, 0x1B3, 0x1AC, 0x100, 0x1BC, 0x100, 0x1B5,
		0x100, 0x1DF
};

static const unsigned short gamma_set_brightness_table[5][23] = {
	{   0x0FA, //180ccd
		0x102, 0x117, 0x113, 0x115, 0x1CF, 0x1CC, 0x1C9, 0x1CB, 0x1CE, 0x1C4, 0x1C5,
		0x1C5, 0x1BD, 0x1CE, 0x1D1, 0x1CB, 0x100, 0x19B, 0x100, 0x197, 0x100, 0x1BA  },
	{   0x0FA, //210ccd
		0x102, 0x117, 0x113, 0x115, 0x1CF, 0x1CC, 0x1C9, 0x1CA, 0x1CD, 0x1C4, 0x1C4,
		0x1C4, 0x1BC, 0x1CB, 0x1D0, 0x1C8, 0x100, 0x1A2, 0x100, 0x19F, 0x100, 0x1C3  },
	{   0x0FA, //240ccd
		0x102, 0x117, 0x113, 0x115, 0x1CF, 0x1CC, 0x1C9, 0x1C9, 0x1CB, 0x1C4, 0x1C2,
		0x1C3, 0x1BB, 0x1CC, 0x1CD, 0x1C8, 0x100, 0x1AB, 0x100, 0x1A6, 0x100, 0x1CD  },
	{   0x0FA, //270ccd
		0x102, 0x117, 0x113, 0x115, 0x1CF, 0x1CC, 0x1C9, 0x1C9, 0x1CB, 0x1C4, 0x1C1,
		0x1C1, 0x1BA, 0x1CA, 0x1CC, 0x1C7, 0x100, 0x1B3, 0x100, 0x1AD, 0x100, 0x1D5  },
	{   0x0FA, //300ccd
		0x102, 0x117, 0x113, 0x115, 0x1CF, 0x1CC, 0x1C9, 0x1C9, 0x1CB, 0x1C4, 0x1BF,
		0x1BF, 0x1B9, 0x1C8, 0x1CA, 0x1C6, 0x100, 0x1BB, 0x100, 0x1B4, 0x100, 0x1DD  },
};

static const unsigned short gamma_update_table[] = {
		0x0FA,
		0x103
};

static const unsigned short etc_condition_1_table[] = {
		0x0F6,
		0x100, 0x18E, 0x10F
};

static const unsigned short etc_condition_2_table[] = {
		0x0B3,
		0x10C
};

static const unsigned short etc_condition_3_table[] = {
		0x0B5,
		0x108, 0x108, 0x108, 0x108, 0x110, 0x110, 0x120, 0x120, 0x12E, 0x119,
		0x12E, 0x127, 0x121, 0x11E, 0x11A, 0x119, 0x12C, 0x127, 0x124, 0x121,
		0x13B, 0x134, 0x130, 0x12C, 0x128, 0x126, 0x124, 0x122, 0x121, 0x11F,
		0x11E, 0x11C
};

static const unsigned short etc_condition_4_table[] = {
		0x0B6,
		0x100, 0x100, 0x111, 0x122, 0x133, 0x144, 0x144, 0x144, 0x155, 0x155,
		0x166, 0x166, 0x166, 0x166, 0x166, 0x166
};

static const unsigned short etc_condition_5_table[] = {
		0x0B7,
		0x108, 0x108, 0x108, 0x108, 0x110, 0x110, 0x120, 0x120, 0x12E, 0x119,
		0x12E, 0x127, 0x121, 0x11E, 0x11A, 0x119, 0x12C, 0x127, 0x124, 0x121,
		0x13B, 0x134, 0x130, 0x12C, 0x128, 0x126, 0x124, 0x122, 0x121, 0x11F,
		0x11E, 0x11C
};

static const unsigned short etc_condition_6_table[] = {
		0x0B8,
		0x100, 0x100, 0x111, 0x122, 0x133, 0x144, 0x144, 0x144, 0x155, 0x155,
		0x166, 0x166, 0x166, 0x166, 0x166, 0x166
};

static const unsigned short etc_condition_7_table[] = {
		0x0B9,
		0x108, 0x108, 0x108, 0x108, 0x110, 0x110, 0x120, 0x120, 0x12E, 0x119,
		0x12E, 0x127, 0x121, 0x11E, 0x11A, 0x119, 0x12C, 0x127, 0x124, 0x121,
		0x13B, 0x134, 0x130, 0x12C, 0x128, 0x126, 0x124, 0x122, 0x121, 0x11F,
		0x11E, 0x11C
};

static const unsigned short etc_condition_8_table[] = {
		0x0BA,
		0x100, 0x100, 0x111, 0x122, 0x133, 0x144, 0x144, 0x144, 0x155, 0x155,
		0x166, 0x166, 0x166, 0x166, 0x166, 0x166
};

static const unsigned short dynamic_elvss_on[] = {
		0x0B1,
		0x10D
};

static const unsigned short dynamic_elvss_off[] = {
		0x0B1,
		0x10A
};


extern int d4_set_lcd_pannel_info_set(struct d4_lcd *pannel);

static void dp_lcd_pannel_reset(void)
{

	/*
	 * GPIO_DISP_3_3V_ON
	 */
	int gpio = GPIO_DISP_3_3V_ON;
	gpio_direction_output(gpio, 0);
	gpio_set_value(gpio, 1);
	mdelay(1);
		
	/*
	 * GPIO_LCD_nRESET
	 */

	gpio = GPIO_LCD_nRESET;
	gpio_direction_output(gpio, 0);
	gpio_set_value(gpio, 1);
	mdelay(1);
	gpio_set_value(gpio, 0);
	mdelay(10);
	gpio_set_value(gpio, 1);
	mdelay(10);
	/*
	 * LCD_BL_ON
	 */
	 
	gpio = LCD_BL_ON;
	gpio_direction_output(gpio, 0);
	gpio_set_value(gpio, 1);


}

static int smd_wvga_amoled_spi_write(struct smd_wvga_amoled *lcd, struct lcd_panel_spi_write_info *info)
{
	int ret = -1;
	struct spi_data_info spi_w_data;
	spi_w_data.data_len = info->data_cnt;
	spi_w_data.wbuffer = info->data_buf;
	spi_w_data.rbuffer = NULL;

	ret = hs_spi_polling_write(lcd->spi, &spi_w_data);

	if (ret < 0)
		return ret;

	return 0;
}

static int smd_wvga_amoled_panel_send_sequence(struct smd_wvga_amoled *lcd, const unsigned short *wbuf, unsigned int param_cnt)
{
	int ret = 0;
	struct lcd_panel_spi_write_info spi_set_info;

	spi_set_info.data_cnt = (param_cnt + 1);
	spi_set_info.data_buf = (void *)wbuf;

	ret = smd_wvga_amoled_spi_write(lcd, &spi_set_info);
	udelay(10);

	return ret;
}

static int smd_wvga_amoled_ldi_init(struct smd_wvga_amoled *lcd)
{
	int ret = 0;

	smd_wvga_amoled_panel_send_sequence(lcd, panel_condition_set_table, CALC_PARAM_CNT(panel_condition_set_table));

	smd_wvga_amoled_panel_send_sequence(lcd, display_condition_1_set_table, CALC_PARAM_CNT(display_condition_1_set_table));
	smd_wvga_amoled_panel_send_sequence(lcd, display_condition_2_set_table, CALC_PARAM_CNT(display_condition_2_set_table));

	/**< Gamma Setting */
	smd_wvga_amoled_panel_send_sequence(lcd, gamma_set_table, CALC_PARAM_CNT(gamma_set_table));
	smd_wvga_amoled_panel_send_sequence(lcd, gamma_update_table, CALC_PARAM_CNT(gamma_update_table));

	smd_wvga_amoled_panel_send_sequence(lcd, etc_condition_1_table, CALC_PARAM_CNT(etc_condition_1_table));
	smd_wvga_amoled_panel_send_sequence(lcd, etc_condition_2_table, CALC_PARAM_CNT(etc_condition_2_table));
	smd_wvga_amoled_panel_send_sequence(lcd, etc_condition_3_table, CALC_PARAM_CNT(etc_condition_3_table));
	smd_wvga_amoled_panel_send_sequence(lcd, etc_condition_4_table, CALC_PARAM_CNT(etc_condition_4_table));
	smd_wvga_amoled_panel_send_sequence(lcd, etc_condition_5_table, CALC_PARAM_CNT(etc_condition_5_table));
	smd_wvga_amoled_panel_send_sequence(lcd, etc_condition_6_table, CALC_PARAM_CNT(etc_condition_6_table));
	smd_wvga_amoled_panel_send_sequence(lcd, etc_condition_7_table, CALC_PARAM_CNT(etc_condition_7_table));
	smd_wvga_amoled_panel_send_sequence(lcd, etc_condition_8_table, CALC_PARAM_CNT(etc_condition_8_table));

	/**< ELVSS Setting */
		/**< ELVSS OFF */
	smd_wvga_amoled_panel_send_sequence(lcd, dynamic_elvss_off, CALC_PARAM_CNT(dynamic_elvss_off));


	mdelay(10);

	return ret;


}

static void mlcd_1_ctrl_standby_on(struct smd_wvga_amoled *lcd)
{
	unsigned short set_value = 0;

	set_value = 0x010;
	smd_wvga_amoled_panel_send_sequence(lcd, &set_value, 0);
	mdelay(120);
}

static void mlcd_1_ctrl_standby_off(struct smd_wvga_amoled *lcd)
{
}

static void mlcd_1_ctrl_display_on(struct smd_wvga_amoled *lcd)
{
	unsigned short set_value = 0;
	set_value = 0x011;
	smd_wvga_amoled_panel_send_sequence(lcd, &set_value, 0);
	mdelay(120);
	set_value = 0x029;
	smd_wvga_amoled_panel_send_sequence(lcd, &set_value, 0);
}

static void mlcd_1_ctrl_display_off(struct smd_wvga_amoled *lcd)
{
	unsigned short set_value = 0;

	set_value = 0x010;
	smd_wvga_amoled_panel_send_sequence(lcd, &set_value, 0);
	mdelay(120);	
}

static int smd_wvga_amoled_ldi_enable(struct smd_wvga_amoled *lcd)
{
	int ret = 0;

	mlcd_1_ctrl_standby_off(lcd);
	mlcd_1_ctrl_display_on(lcd);
	return ret;
}

static int smd_wvga_amoled_ldi_disable(struct smd_wvga_amoled *lcd)
{
	int ret = 0;

	mlcd_1_ctrl_standby_on(lcd);
	mlcd_1_ctrl_display_off(lcd);
	return ret;
}


int smd_wvga_amoled_lcd_init(void)
{
	int ret = 0;
	struct d4_hs_spi_config spi_data;
	struct smd_wvga_amoled *lcd = NULL;

#if 1
	unsigned long pin_config = 0;

	/**< Data strength를  높이기 위한 임시 코드, for NX1 proto board */
	pin_config = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
			PIN_CONFIG_DRIVE_STRENGTH_X6);
	ret = pin_config_group_set("drime4-pinmux", "mlcdcon", pin_config);
	if (ret < 0) {
		printk("Data strength control: Fail \n");
		return ret;
	}

	ret = pin_config_group_set("drime4-pinmux", "spi0grp", pin_config);
	if (ret < 0) {
		printk("Data strength control: Fail \n");
		return ret;
	}
#endif

	lcd = kzalloc(sizeof(struct smd_wvga_amoled), GFP_KERNEL);

	if (lcd == NULL)
		return -ENOMEM;

	if (d4_set_lcd_pannel_info_set(&smd_wvga_amoled_data) < 0) {
		printk("lcd pannel set fail\n");
		return -EINVAL;
	}
	dp_lcd_pannel_reset();

	spi_data.speed_hz = 5000000;
	spi_data.bpw = 9;
	spi_data.mode = SH_SPI_MODE_3;
	spi_data.waittime = 100;
	spi_data.ss_inven = D4_SPI_TRAN_INVERT_OFF;
	spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF;
	spi_data.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST;

	lcd->spi = hs_spi_request(0);

	hs_spi_config(lcd->spi, &spi_data);

	ret = smd_wvga_amoled_ldi_init(lcd);
	if (ret) {
		printk("failed to initialize ldi.\n");
		return ret;
	}

	ret = smd_wvga_amoled_ldi_enable(lcd);
	if (ret) {
		printk("failed to enable ldi.\n");
		return ret;
	}
	kfree(lcd);
	return ret;
}

void smd_wvga_amoled_pannel_onff(void)
{
	struct smd_wvga_amoled *lcd = NULL;
	struct d4_hs_spi_config spi_data;
	lcd = kzalloc(sizeof(struct smd_wvga_amoled), GFP_KERNEL);
	spi_data.speed_hz = 5000000;
	spi_data.bpw = 9;
	spi_data.mode = SH_SPI_MODE_3;
	spi_data.waittime = 100;
	spi_data.ss_inven = D4_SPI_TRAN_INVERT_OFF;
	spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF;
	spi_data.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST;
	lcd->spi = hs_spi_request(0);
	hs_spi_config(lcd->spi, &spi_data);
	smd_wvga_amoled_ldi_disable(lcd);
}

int smd_wvga_amoled_init(void)
{
	return smd_wvga_amoled_lcd_init();
}

void smd_wvga_amoled_exit(void)
{
	struct smd_wvga_amoled *lcd = NULL;
	struct d4_hs_spi_config spi_data;
	lcd = kzalloc(sizeof(struct smd_wvga_amoled), GFP_KERNEL);

	spi_data.speed_hz = 5000000;
	spi_data.bpw = 9;
	spi_data.mode = SH_SPI_MODE_3;
	spi_data.waittime = 100;
	spi_data.ss_inven = D4_SPI_TRAN_INVERT_OFF;
	spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF;
	spi_data.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST;

	lcd->spi = hs_spi_request(0);
	hs_spi_config(lcd->spi, &spi_data);

	smd_wvga_amoled_ldi_disable(lcd);
}


