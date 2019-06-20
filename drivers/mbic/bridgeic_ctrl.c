/**
 * @file bridgeic_ctrl.c
 * @brief Bridge IC for MIPI
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include <linux/firmware.h>
#include <linux/regulator/consumer.h>
#include <mach/map.h>
#include <linux/gpio.h>
#include <linux/pinctrl/pinconf-drime4.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/consumer.h>

#include <mach/version_information.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/i2c.h>
#include <mach/gpio_map_d4_galaxynx.h>
#include <linux/d4_ht_pwm.h>
#include <linux/bridgeic_ctrl.h>
#include "../drivers/video/drime4/lcd_panel_manager/d4_lcd_panel_manager_if.h"
#include "../drivers/video/drime4/lcd/d4_dp_lcd_dd.h"

#define PREDV_2ND_MIPI

#define BRIDGE_CH 12
#define BRIDGE_ADDR_LENGTH 2
struct bic_data {
	struct i2c_client		*client;
	struct clk *clk;
};



struct bic_data *bridgeic;
struct ht_pwm_device *pwm_12;
struct miscdevice *bic;

char data0[4] = {0x00, 0x02, 0x00, 0x01};
char data1[4] = {0x00, 0x02, 0x00, 0x00};
char data2[4] = {0x00, 0x16, 0x10, 0x5F};
char data3[4] = {0x00, 0x18, 0x02, 0x03};
char data4[4] = {0x00, 0x18, 0x02, 0x13};
char data5[4] = {0x00, 0x06, 0x00, 0xC8};
char data6[4] = {0x00, 0x08, 0x00, 0x60};
char data7[4] = {0x00, 0x22, 0x0A, 0x20};  // 1296
char data8[4] = {0x00, 0x50, 0x00, 0x00};
char data9[4] = {0x01, 0x40, 0x00, 0x00};
char data10[4] = {0x01, 0x42, 0x00, 0x00};
char data11[4] = {0x01, 0x44, 0x00, 0x00};
char data12[4] = {0x01, 0x46, 0x00, 0x00};
char data13[4] = {0x01, 0x48, 0x00, 0x00};
char data14[4] = {0x01, 0x4A, 0x00, 0x00};
char data15[4] = {0x01, 0x4C, 0x00, 0x00};
char data16[4] = {0x01, 0x4E, 0x00, 0x00};
char data17[4] = {0x01, 0x50, 0x00, 0x00};
char data18[4] = {0x01, 0x52, 0x00, 0x00};
char data19[4] = {0x02, 0x10, 0x32, 0x00};
char data20[4] = {0x02, 0x12, 0x00, 0x00};
char data21[4] = {0x02, 0x14, 0x00, 0x08};
char data22[4] = {0x02, 0x16, 0x00, 0x00};
char data23[4] = {0x02, 0x18, 0x40, 0x06};
char data24[4] = {0x02, 0x1A, 0x00, 0x00};
char data25[4] = {0x02, 0x1C, 0x00, 0x03};
char data26[4] = {0x02, 0x1E, 0x00, 0x00};
char data27[4] = {0x02, 0x20, 0x08, 0x06};
char data28[4] = {0x02, 0x22, 0x00, 0x00};
char data29[4] = {0x02, 0x24, 0x64, 0x00};
char data30[4] = {0x02, 0x26, 0x00, 0x00};
char data31[4] = {0x02, 0x28, 0x00, 0x10};
char data32[4] = {0x02, 0x2A, 0x00, 0x00};
char data33[4] = {0x02, 0x2C, 0x00, 0x04};
char data34[4] = {0x02, 0x2E, 0x00, 0x00};
char data35[4] = {0x02, 0x30, 0x00, 0x05};
char data36[4] = {0x02, 0x32, 0x00, 0x00};
char data37[4] = {0x02, 0x34, 0x00, 0x07};
char data38[4] = {0x02, 0x36, 0x00, 0x00};
char data39[4] = {0x02, 0x38, 0x00, 0x01};
char data40[4] = {0x02, 0x3A, 0x00, 0x00};
char data41[4] = {0x02, 0x04, 0x00, 0x01};
char data42[4] = {0x02, 0x06, 0x00, 0x00};
char data43[4] = {0x05, 0x18, 0x00, 0x01};
char data44[4] = {0x05, 0x1A, 0x00, 0x00};
char data45[4] = {0x05, 0x00, 0x00, 0x82};
char data46[4] = {0x05, 0x02, 0xA3, 0x00};
char data47[4] = {0x00, 0x04, 0x81, 0x61};


static void bridge_ic_pwr(void)
{
	int error;

	struct ht_pwm_conf_info pwm_info;
	error = gpio_request(GPIO_BIC_MSEL, "");
	error = gpio_request(GPIO_BIC_CS, "");
	error = gpio_request(GPIO_BIC_RESX, "");

	error = gpio_direction_output(GPIO_BIC_RESX, 0);

	error = gpio_direction_output(GPIO_BIC_MSEL, 1);
	error = gpio_direction_output(GPIO_BIC_CS, 0);
	error = gpio_direction_output(GPIO_BIC_RESX, 1);




	pwm_info.opmode = CONTINUE_PULSE;
	pwm_info.trigtype = SW_TRIGGER;

	pwm_info.freq1.duty = 30;
	pwm_info.freq1.period = 60;
	pwm_info.freq2.duty = 30;
	pwm_info.freq2.period = 60;

	d4_ht_pwm_config(pwm_12, &pwm_info);
	d4_ht_pwm_enable(pwm_12);
}


static int bridge_i2c_write(char *buf, int length)
{
	int i;
	char data[4];


	for (i = 0; i < length; i++)
		data[i] = *buf++;


	i = i2c_master_send(bridgeic->client, (char *) data, length);

	if (i == length)
		return length;
	else {
		printk("[TSP] :write error : [%d]\n", i);
		return -EIO;
	}

}



static int bridge_i2c_read(u16 addr, u16 length, u8 *value)
{
	struct i2c_adapter *adapter = bridgeic->client->adapter;
	struct i2c_msg msg;
	unsigned char id_addr[BRIDGE_ADDR_LENGTH];
	int ret = -1;
	int i;

	for (i = 0; i < BRIDGE_ADDR_LENGTH; i++)
		id_addr[i] = (addr>>((1-i)*8)) & 0xFF;

	msg.addr = bridgeic->client->addr;
	msg.flags = 0x00;
	msg.len = BRIDGE_ADDR_LENGTH;
	msg.buf = (u8 *) &id_addr;

	ret = i2c_transfer(adapter, &msg, 1);

	if (ret >= 0) {
		msg.addr = bridgeic->client->addr;
		msg.flags = I2C_M_RD;
		msg.len = length;
		msg.buf = (u8 *) value;

		ret = i2c_transfer(adapter, &msg, 1);
	}

	return ret;
}

unsigned char buf[5]={0,};

static int bridge_ic_on(void)
{
	int err = 0;
	int i, tmp_val;
	unsigned int clock, cbar;


	/* TC358746XBG Power On & REF_CLK(PWM) Set */
	bridge_ic_pwr();
	/*temp data*/
	__raw_writel(0x30243, DRIME4_VA_GLOBAL_CTRL + 0x118);

	/* PCLK (SLCD_CLK) On */
	//
	//  We need PCLK (SLCD_CLK) without VSync & HSync
	//

	mdelay(1);

	/*org data : Pad Setting */
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0AC);
	__raw_writel(0x30143, DRIME4_VA_GLOBAL_CTRL + 0x0B0);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0B4);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0B8);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0BC);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0C0);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0C4);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0C8);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0CC);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0D0);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0D4);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0D8);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0DC);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0E0);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0E4);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0E8);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0EC);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0F0);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0F4);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0F8);
	__raw_writel(0x20140, DRIME4_VA_GLOBAL_CTRL + 0x0FC);

	mdelay(10);
	mdelay(10);

	/*TC358746XBG Software Reset*/
	err = bridge_i2c_write(data0, 4);
	if (err == -EIO) {
		printk("DATA 0 Write Error\n");
	}
	udelay(10);
	bridge_i2c_write(data1, 4);

	/*TC358746XBG PLL,Clock Setting*/
	bridge_i2c_write(data2, 4);
	bridge_i2c_write(data3, 4);
	mdelay(10);
	bridge_i2c_write(data4, 4);

	/*TC358746XBG DPI Input Control*/
	bridge_i2c_write(data5, 4);
	bridge_i2c_write(data6, 4);
	bridge_i2c_write(data7, 4);
	bridge_i2c_write(data8, 4);

	/*TC358746XBG D-PHY Setting*/
	bridge_i2c_write(data9, 4);
	bridge_i2c_write(data10, 4);
	bridge_i2c_write(data11, 4);
	bridge_i2c_write(data12, 4);
	bridge_i2c_write(data13, 4);
	bridge_i2c_write(data14, 4);
	bridge_i2c_write(data15, 4);
	bridge_i2c_write(data16, 4);
	bridge_i2c_write(data17, 4);
	bridge_i2c_write(data18, 4);

	/*TC358746XBG CSI2-TX PPI Control*/
	bridge_i2c_write(data19, 4);
	bridge_i2c_write(data20, 4);
	bridge_i2c_write(data21, 4);
	bridge_i2c_write(data22, 4);
	bridge_i2c_write(data23, 4);
	bridge_i2c_write(data24, 4);
	bridge_i2c_write(data25, 4);
	bridge_i2c_write(data26, 4);
	bridge_i2c_write(data27, 4);
	bridge_i2c_write(data28, 4);
	bridge_i2c_write(data29, 4);
	bridge_i2c_write(data30, 4);
	bridge_i2c_write(data31, 4);
	bridge_i2c_write(data32, 4);
	bridge_i2c_write(data33, 4);
	bridge_i2c_write(data34, 4);
	bridge_i2c_write(data35, 4);
	bridge_i2c_write(data36, 4);
	bridge_i2c_write(data37, 4);
	bridge_i2c_write(data38, 4);
	bridge_i2c_write(data39, 4);
	bridge_i2c_write(data40, 4);
	bridge_i2c_write(data41, 4);
	bridge_i2c_write(data42, 4);

	slcd_panel_init();

	/* CSI Start */
	bridge_i2c_write(data43, 4);
	bridge_i2c_write(data44, 4);

	/*Set to HS mode*/
	bridge_i2c_write(data45, 4);
	bridge_i2c_write(data46, 4);
	bridge_i2c_write(data47, 4);


	__raw_writel(0xC6A14000, DRIME4_VA_DP + 0x0D00);
	__raw_writel(0xC6AF5000, DRIME4_VA_DP + 0x0D04);
	__raw_writel(0xC6A14500, DRIME4_VA_DP + 0x0D08);
	__raw_writel(0xC6AF5500, DRIME4_VA_DP + 0x0D0C);
	__raw_writel(0x05000000, DRIME4_VA_DP + 0x0D10);
	__raw_writel(0x02D00000, DRIME4_VA_DP + 0x0D14);

	__raw_writel(0x000000A0, DRIME4_VA_DP + 0x0C08);
	__raw_writel(0x00000001, DRIME4_VA_DP + 0x0C14);

	return err;

}

static int bridge_ic_off(void)
{
	int error = -1;;
	/*
	 *  SLCD Output Off (after all sync transfer done)
	 */

	/*
	 *  PWM, SLCD Clock Off
	 */

	/**< SLCD Power Off */
	d4_dp_sublcd_off();

	/**< Bridge IC Reset */
	error = gpio_request(GPIO_BIC_RESX, "");
	if (error < 0)
		return -1;

	error = gpio_direction_output(GPIO_BIC_RESX, 0);
	if (error < 0)
		return -1;

	return 0;
}



static int d4_bridgeic_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int d4_bridgeic_release(struct inode *inode, struct file *filp)
{
	return 0;
}


long d4_bridgeic_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size;
	int err = -1;
	int ret = 0;
	unsigned char *v_name;
	EBdVersion bv_ver;

	switch (cmd) {
	case BIC_IOCTL_POWERON:
		err = bridge_ic_on();
		break;


	case BIC_IOCTL_POWEROFF:
		err = bridge_ic_off();
		break;


	default:
		break;
	}

	return err;
}

const struct file_operations d4_bridgeic_fops = { .open = d4_bridgeic_open,
		.release = d4_bridgeic_release, .unlocked_ioctl = d4_bridgeic_ioctl };

static void d4_bridgeic_ioctl_set(struct miscdevice *mdmadev)
{
	mdmadev->name = "bic";
	mdmadev->minor = MISC_DYNAMIC_MINOR;
	mdmadev->fops = &d4_bridgeic_fops;
}


static int d4_bridgeic_misc_set(void)
{
	int ret;
	bic = kzalloc(sizeof(*bic), GFP_KERNEL);
	if (!bic) {
		printk("no memory for state\n");
		return -ENOMEM;;
	}
	d4_bridgeic_ioctl_set(bic);
	ret = misc_register(bic);
	if (ret < 0) {
		printk("failed to register misc driver\n");
		kfree(bic);
		return -ENODEV;
	}
	return 0;
}



static int bridge_ic_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	struct bic_data *bic;

	bic = kzalloc(sizeof(struct bic_data), GFP_KERNEL);
	if (!bic) {
		printk("Bridge IC Data Get Fail\n");
		return -1;
	}

	bic->client = client;
	bridgeic = bic;
	pwm_12 = d4_ht_pwm_request(BRIDGE_CH, "");

	d4_bridgeic_misc_set();
	return ret;
}



#ifdef CONFIG_PM
static int bridge_ic_suspend(struct i2c_client *client)
{
	return 0;
}

static int bridge_ic_resume(struct i2c_client *client)
{
	return 0;
}
#else
#define bridge_ic_suspend NULL
#define bridge_ic_resume NULL
#endif


static int bridge_ic_remove(struct i2c_client *client)
{
	if (!bridgeic) {
		kfree(bridgeic);
	}
	return 0;
}

static const struct i2c_device_id 	bridge_ic_id[] = {{ "bridgeic", 0 }, {}};

static struct i2c_driver bridge_ic_driver = {
	.driver = {
		.name = "bridgeic",
	},
	.id_table = bridge_ic_id,
	.probe = bridge_ic_probe,
	.remove = bridge_ic_remove,
	.suspend = bridge_ic_suspend,
	.resume = bridge_ic_resume,
};

static int __devinit bridge_ic_init(void)
{
	return i2c_add_driver(&bridge_ic_driver);
}

static void __exit bridge_ic_exit(void)
{
	i2c_del_driver(&bridge_ic_driver);
}

module_init(bridge_ic_init);
module_exit(bridge_ic_exit);

MODULE_DESCRIPTION("Driver for Bridge IC Controller");
MODULE_AUTHOR("kyuchun han <kyuchun.han@samsung.com>");
MODULE_VERSION("0.2");
MODULE_LICENSE("GPL");
