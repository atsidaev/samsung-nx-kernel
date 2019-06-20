/*
 * CH2330  AMOLED LCD panel driver.
 *
 * Author: sejong <sejong55.oh@samsung.com>
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

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

#define MIN_BRIGHTNESS		0
#define MAX_BRIGHTNESS		10

#if (defined (CONFIG_DRIME4_REAL_BOARD) | defined (CONFIG_MACH_NX))
static struct d4_lcd ch2330_data = {.h_size = 640, .v_size = 480, .freq = 60,
	.lcd_data_width = DATA_WIDTH_24, .type = TYPE_STRIPE,
	.even_seq = SEQ_BGR, .odd_seq = SEQ_BGR, .timing = {
		.total_h_size = 857,
		.total_v_size = 524,
		.h_sync_rise = 0,
		.h_sync_fall = 60,
		.v_sync_rise = 0,
		.v_sync_fall = 10,
		.buf_read_h_start = 142,
		.enable_h_start = 150,
		.enable_h_end = 790,
		.enable_v_start = 38,
		.enable_v_end = 517,
		.inv_dot_clk = 1,
		.inv_enable_clk = 0,
		.inv_h_sync = 1,
		.inv_v_sync = 1,
	},};
#else
static struct d4_lcd ch2330_data = { .h_size = 640, .v_size = 480, .freq = 60,
		.lcd_data_width = DATA_WIDTH_24, .type = TYPE_STRIPE,
		.even_seq = SEQ_RGB, .odd_seq = SEQ_RGB, .timing = {
		.total_h_size = 857,
		.total_v_size = 524,
		.h_sync_rise = 0,
		.h_sync_fall = 60,
		.v_sync_rise = 0,
		.v_sync_fall = 10,
		.buf_read_h_start = 142,
		.enable_h_start = 150,
		.enable_h_end = 790,
		.enable_v_start = 38,
		.enable_v_end = 517,
		.inv_dot_clk = 1,
		.inv_enable_clk = 0,
		.inv_h_sync = 1,
		.inv_v_sync = 1,
	}, };
#endif

struct ch2330 {
	struct device *dev;
    struct hs_spi_data *spi;
};

static const unsigned short SEQ_PANEL_DISPLAY_CONTROL[] = { 0x00, 0xA4, 0x01,
		0xA0, 0x02, 0xA0, 0x03, 0xA0, 0x04, 0xA1, 0x05, /*0x7A*/0x96, 0x06,
		0x26, 0x07, 0x07, 0x08, 0xA0, 0x09, 0xA0, /* CM0,CM1 :0,0 24bit- RGB Interface */
		0xA5, 0x19, 0xA6, 0x14, ENDDEF, 0x00 };

static const unsigned short SEQ_PANEL_POWER_CONTROL[] = { 0x0A, 0xA1, 0x0B,
		0x22, 0x0C, 0x33, 0x0D, 0xA3, 0x0E, 0xA1, 0x0F, 0x6A, 0x10, 0x72, 0x11,
		0xA3, 0x12, 0xA3, 0x13, 0x77, ENDDEF, 0x00 };

static const unsigned short SEQ_PANEL_TIMING_CONTROL[] = { 0x14, 0xA0, 0x15,
		0xA1, 0x16, 0xE2, 0x17, 0xB8, 0x18, 0xEF, 0x19, 0x9D, 0x1A, 0x36, 0x1B,
		0x51, 0x1C, 0x00, 0x1D, 0x00, 0x1E, 0x0E, 0x1F, 0x07, 0x20, 0x0B, 0x21,
		0x2F, 0x22, 0x2F, 0x23, 0x2F, 0x24, 0x2F, ENDDEF, 0x00 };

static const unsigned short SEQ_PANEL_GAMMA_CONTROL[] = { 0x25, 0x44, 0x26,
		0x35, 0x27, 0x6D, 0x28, 0x3C, 0x29, 0x35, 0x2A, 0x47, 0x2B, 0x61, 0x2C,
		0x35, 0x2D, 0x5D, 0x2E, 0x39, 0x2F, 0x35, 0x30, 0x46, 0x31, 0x5D, 0x32,
		0x35, 0x33, 0x69, 0x34, 0x3A, 0x35, 0x33, 0x36, 0x44, 0x37, 0x7D,
		ENDDEF, 0x00 };

static const unsigned short SEQ_PANEL_STANDBY_OFF[] = { 0x9A, 0x80, 0x00, 0xA0,
		SLEEPMSEC, 160, 0x01, 0xA1, SLEEPMSEC, 20, 0x00, 0xA3, ENDDEF, 0x00 };

static const unsigned short SEQ_PANEL_STANDBY_ON[] = { 0x00, 0xA0, SLEEPMSEC,
		20, 0x01, 0xA0, SLEEPMSEC, 20, 0x00, 0xA4, ENDDEF, 0x00 };

static const unsigned short SEQ_PANEL_DISPLAY_ON[] = { 0x01, 0xA1, SLEEPMSEC,
		20, 0x00, 0xA3, ENDDEF, 0x00 };

static const unsigned short SEQ_PANEL_DISPLAY_OFF[] = { 0x00, 0xA0, SLEEPMSEC,
		20, 0x01, 0xA0, ENDDEF, 0x00 };

extern int d4_set_lcd_pannel_info_set(struct d4_lcd *pannel);

static void dp_lcd_pannel_reset(void)
{
	int ret;
	int gpio = GPIO_LCD_nRESET;

	gpio_direction_output(gpio, 0);
	gpio_set_value(gpio, 1);
	
	/*
	 * GPIO_DISP_3_3V_ON
	 */
	gpio = GPIO_DISP_3_3V_ON;
	gpio_direction_output(gpio, 0);
	gpio_set_value(gpio, 1);
	
}

static int ch2330_spi_write(struct ch2330 *lcd, unsigned char address,
		unsigned char command)
{
	u16 buf[1];

 struct spi_data_info xfer = { .data_len = 1, .wbuffer = buf, .rbuffer = NULL };

 buf[0] = (address << 8) | command;


    return hs_spi_polling_write(lcd->spi, &xfer);


}

static int ch2330_panel_send_sequence(struct ch2330 *lcd,
		const unsigned short *wbuf)
{
	int ret = 0, i = 0;

	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC) {
			ret = ch2330_spi_write(lcd, wbuf[i], wbuf[i + 1]);
			if (ret < 0)
				return ret;
		} else
			udelay(wbuf[i+1]*1000);
		i += 2;
	}

	return ret;
}

static int ch2330_ldi_init(struct ch2330 *lcd)
{
	int ret = 0;
	printk("ch2330_ldi_init()\n");

	ch2330_panel_send_sequence(lcd, SEQ_PANEL_DISPLAY_CONTROL);
	ch2330_panel_send_sequence(lcd, SEQ_PANEL_POWER_CONTROL);
	ch2330_panel_send_sequence(lcd, SEQ_PANEL_TIMING_CONTROL);
	ch2330_panel_send_sequence(lcd, SEQ_PANEL_GAMMA_CONTROL);

	mdelay(10);


	return ret;
}

static int ch2330_ldi_enable(struct ch2330 *lcd)
{
	int ret = 0;

	ch2330_panel_send_sequence(lcd, SEQ_PANEL_STANDBY_OFF);
	ch2330_panel_send_sequence(lcd, SEQ_PANEL_DISPLAY_ON);

	return ret;
}

static int ch2330_ldi_disable(struct ch2330 *lcd)
{
	int ret = 0;

	ch2330_panel_send_sequence(lcd, SEQ_PANEL_STANDBY_ON);
	ch2330_panel_send_sequence(lcd, SEQ_PANEL_DISPLAY_OFF);

	return ret;
}


int ch2330_lcd_init(void)
{
	int ret = 0;
	struct d4_hs_spi_config spi_data;
	struct ch2330 *lcd = NULL;

	lcd = kzalloc(sizeof(struct ch2330), GFP_KERNEL);

	if (lcd == NULL)
		return -ENOMEM;

	if (d4_set_lcd_pannel_info_set(&ch2330_data) < 0) {
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

	ret = ch2330_ldi_init(lcd);
	if (ret) {
		printk("failed to initialize ldi.\n");
		return ret;
	}

	ret = ch2330_ldi_enable(lcd);
	if (ret) {
		printk("failed to enable ldi.\n");
		return ret;
	}
	kfree(lcd);
	return ret;
}

void ch2330_pannel_onff(void)
{
	struct ch2330 *lcd = NULL;
	struct d4_hs_spi_config spi_data;
	lcd = kzalloc(sizeof(struct ch2330), GFP_KERNEL);
	if(lcd == NULL)	{
		return;	
	}
	
	spi_data.speed_hz = 781300;
	spi_data.bpw = 16;
	spi_data.mode = SH_SPI_MODE_3;
	spi_data.waittime = 100;
	spi_data.ss_inven = D4_SPI_TRAN_INVERT_OFF;
	spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF;
	spi_data.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST;
	lcd->spi = hs_spi_request(0);
	hs_spi_config(lcd->spi, &spi_data);
	ch2330_ldi_disable(lcd);
	kfree(lcd);
}

int ch2330_init(void)
{
    return ch2330_lcd_init();
}

void ch2330_exit(void)
{
	struct ch2330 *lcd = NULL;
	struct d4_hs_spi_config spi_data;
	lcd = kzalloc(sizeof(struct ch2330), GFP_KERNEL);
	if(lcd == NULL)	{
		return;	
	}

	spi_data.speed_hz = 781300;
	spi_data.bpw = 16;
	spi_data.mode = SH_SPI_MODE_3;
	spi_data.waittime = 100;
	spi_data.ss_inven = D4_SPI_TRAN_INVERT_OFF;
	spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF;
	spi_data.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST;

	lcd->spi = hs_spi_request(0);
	hs_spi_config(lcd->spi, &spi_data);

	ch2330_ldi_disable(lcd);
	kfree(lcd);	
}

