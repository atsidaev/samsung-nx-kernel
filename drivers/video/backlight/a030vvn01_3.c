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

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

#define MIN_BRIGHTNESS		0
#define MAX_BRIGHTNESS		10

static struct d4_lcd a030vvn01_3_data = { .h_size = 640, .v_size = 480, .freq = 60,
		.lcd_data_width = DATA_WIDTH_24, .type = TYPE_STRIPE,
		.even_seq = SEQ_BGR, .odd_seq = SEQ_BGR, .timing = {
		.total_h_size = 762,
		.total_v_size = 525,
		.h_sync_rise = 0,
		.h_sync_fall = 40,
		.v_sync_rise = 0,
		.v_sync_fall = 9,
		.buf_read_h_start = 73,
		.enable_h_start = 81,
		.enable_h_end = 721,
		.enable_v_start = 27,
		.enable_v_end = 507,
		.inv_dot_clk = 1,
		.inv_enable_clk = 0,
		.inv_h_sync = 1,
		.inv_v_sync = 1,
	}, };

struct a030vvn01_3 {
	struct device *dev;
    struct hs_spi_data *spi;
};

static const unsigned short SEQ_PANEL_DISPLAY_CONTROL[] = { 
	0x0A, 0x03, 0x01, 0xF8, 0x02, 0x51, 0x03, 0x1B, 0x04, 0x40, 
	0x05, 0x40,	0x06, 0x40, 0x07, 0x40, 0x08, 0x40, 0x09, 0x40,
	0x0B, 0x77, 0x00, 0x8A,
	ENDDEF, 0x00
	};

static const unsigned short SEQ_PANEL_GAMMA_CONTROL[] = { 	
	0x0C, 0xCC, 0x0D, 0xCC, 0x0E, 0xAC, 0x0F, 0x8A, 0x10, 0x0A, 
	0x11, 0xCC, 0x12, 0xCC, 0x13, 0xAC, 0x14, 0x8A, 0x15, 0x0A, 
	0x16, 0xCC, 0x17, 0xCC, 0x18, 0xAC, 0x19, 0x8A, 0x1A, 0x0A, 
	ENDDEF, 0x00
	};


static const unsigned short SEQ_PANEL_POWER_OFF[] = { 0x0A, 0x02, ENDDEF, 0x00};


static const unsigned short SEQ_PANEL_POWER_ON[] = { 0x0A, 0x03, ENDDEF, 0x00 };


static const unsigned short SEQ_PANEL_STANDBY_OFF[] = { 0x0A, 0x03 ,ENDDEF, 0x00};

static const unsigned short SEQ_PANEL_STANDBY_ON[] = { 0x0A, 0x02 ,ENDDEF, 0x00};


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

static int a030vvn01_3_spi_write(struct a030vvn01_3 *lcd, unsigned char address,
		unsigned char command)
{
	u16 buf[1];

	struct spi_data_info xfer = { .data_len = 1, .wbuffer = buf, .rbuffer = NULL };

	buf[0] = (address << 8) | command;


	return hs_spi_polling_write(lcd->spi, &xfer);


}

static int a030vvn01_3_panel_send_sequence(struct a030vvn01_3 *lcd,
		const unsigned short *wbuf)
{
	int ret = 0, i = 0;

	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		ret = a030vvn01_3_spi_write(lcd, wbuf[i], wbuf[i + 1]);
		//printk("=====a030vvn01_3_panel_send_sequence   %d %d %d\n", wbuf[i], wbuf[i + 1], ret);
		if (ret < 0)
			return ret;
		i += 2;
	}

	return ret;
}

static int a030vvn01_3_ldi_init(struct a030vvn01_3 *lcd)
{
	int ret = 0;
	
	a030vvn01_3_panel_send_sequence(lcd, SEQ_PANEL_DISPLAY_CONTROL);
	mdelay(1);
	a030vvn01_3_panel_send_sequence(lcd, SEQ_PANEL_GAMMA_CONTROL);

	mdelay(1);
	//mdelay(10);

	//printk("%s\n", __func__);

	return ret;
}

static int a030vvn01_3_ldi_enable(struct a030vvn01_3 *lcd)
{
	int ret = 0;

	a030vvn01_3_panel_send_sequence(lcd, SEQ_PANEL_STANDBY_OFF);
	return ret;
}

static int a030vvn01_3_ldi_disable(struct a030vvn01_3 *lcd)
{
	int ret = 0;

	a030vvn01_3_panel_send_sequence(lcd, SEQ_PANEL_STANDBY_ON);
	return ret;
}


int a030vvn01_3_lcd_init(void)
{
	int ret = 0;
	struct d4_hs_spi_config spi_data;
	struct a030vvn01_3 *lcd = NULL;

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

	lcd = kzalloc(sizeof(struct a030vvn01_3), GFP_KERNEL);

	if (lcd == NULL)
		return -ENOMEM;

	if (d4_set_lcd_pannel_info_set(&a030vvn01_3_data) < 0) {
		printk("lcd pannel set fail\n");
		return -EINVAL;
	}
	dp_lcd_pannel_reset();

	spi_data.speed_hz = 781300;
	spi_data.bpw = 16;
	spi_data.mode = SH_SPI_MODE_3;
	spi_data.waittime = 100;
	spi_data.ss_inven = D4_SPI_TRAN_INVERT_OFF;
	spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF;
	spi_data.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST;

	lcd->spi = hs_spi_request(0);

	hs_spi_config(lcd->spi, &spi_data);

	ret = a030vvn01_3_ldi_init(lcd);
	if (ret) {
		printk("failed to initialize ldi.\n");
		return ret;
	}

	ret = a030vvn01_3_ldi_enable(lcd);
	if (ret) {
		printk("failed to enable ldi.\n");
		return ret;
	}
	kfree(lcd);
	return ret;
}

void a030vvn01_3_pannel_onff(void)
{
	struct a030vvn01_3 *lcd = NULL;
	struct d4_hs_spi_config spi_data;
	lcd = kzalloc(sizeof(struct a030vvn01_3), GFP_KERNEL);
	spi_data.speed_hz = 781300;
	spi_data.bpw = 16;
	spi_data.mode = SH_SPI_MODE_3;
	spi_data.waittime = 100;
	spi_data.ss_inven = D4_SPI_TRAN_INVERT_OFF;
	spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF;
	spi_data.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST;
	lcd->spi = hs_spi_request(0);
	hs_spi_config(lcd->spi, &spi_data);
	a030vvn01_3_ldi_disable(lcd);
}

int a030vvn01_3_init(void)
{
	return a030vvn01_3_lcd_init();
}

void a030vvn01_3_exit(void)
{
	struct a030vvn01_3 *lcd = NULL;
	struct d4_hs_spi_config spi_data;
	lcd = kzalloc(sizeof(struct a030vvn01_3), GFP_KERNEL);

	spi_data.speed_hz = 781300;
	spi_data.bpw = 16;
	spi_data.mode = SH_SPI_MODE_3;
	spi_data.waittime = 100;
	spi_data.ss_inven = D4_SPI_TRAN_INVERT_OFF;
	spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF;
	spi_data.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST;

	lcd->spi = hs_spi_request(0);
	hs_spi_config(lcd->spi, &spi_data);

	a030vvn01_3_ldi_disable(lcd);
}


