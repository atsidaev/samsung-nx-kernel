/*
 * CH2330  AMOLED LCD panel driver.
 *
 * Author: sejong <sc.jun@samsung.com>
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
#include <linux/pinctrl/consumer.h>
#include <mach/version_information.h>

extern int ch2330_init(void);
extern void ch2330_exit(void);
extern void ch2330_pannel_onff(void);

extern int a030vvn01_3_init(void);
extern void a030vvn01_3_exit(void);
extern void a030vvn01_3_pannel_onff(void);

extern int smd_wvga_amoled_init(void);
extern void smd_wvga_amoled_exit(void);
extern void smd_wvga_amoled_pannel_onff(void);

void change_hdmi_setting(unsigned long value)
{
}

unsigned long g_change_hdmi_value;

int nx300_lcd_init(void)
{
	//return ch2330_init();					// Jig2
	//return a030vvn01_3_init();			// DV3
	return smd_wvga_amoled_init();
}

void nx300_lcd_off(void)
{
	//ch2330_pannel_onff();				// Jig2
	//a030vvn01_3_pannel_onff();			// DV3
	smd_wvga_amoled_pannel_onff();
}

static int __init nx300_lcd_dev_init(void)
{
	int ret;
	int gpio = GPIO_LCD_nRESET;
	ret = pinctrl_request_gpio(gpio);

	if (!ret) {
		ret = gpio_request(gpio, "GPIO_LCD_nRESET");
		if (ret < 0)
			pr_err("cannot request gpio %d, ret=%d\n", gpio, ret);
	}
	else
		pr_err("cannot retrive gpio %d, ret=%d\n", gpio, ret);

	gpio = GPIO_DISP_3_3V_ON;
	ret = pinctrl_request_gpio(gpio);
	if (!ret) {
		ret = gpio_request(gpio, "GPIO_DISP_3_3V_ON");
		if (ret < 0)
			pr_err("cannot request gpio %d, ret=%d\n", gpio, ret);
	}
	else
		pr_err("cannot retrive gpio %d, ret=%d\n", gpio, ret);

	return nx300_lcd_init();
}

static void __exit nx300_lcd_dev_exit(void)
{
	//ch2330_exit();					// Jig2
	//a030vvn01_3_exit();			// DV3
	smd_wvga_amoled_exit();
}
	
#ifndef CONFIG_SCORE_FAST_RESUME
module_init(nx300_lcd_dev_init);
#else
fast_dev_initcall(nx300_lcd_dev_init);
#endif
module_exit(nx300_lcd_dev_exit);

