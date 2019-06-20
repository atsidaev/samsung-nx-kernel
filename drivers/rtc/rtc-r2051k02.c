/* linux/drivers/rtc/rtc-r2051k02.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 RTC Driver
 * 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
//#include <linux/clk.h>
#include <linux/log2.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/r2051k02.h>
#include <linux/delay.h>

#include <mach/gpio.h>


#include <mach/hardware.h>
//#include <mach/rtc_sysfs.h>
//#include <asm/irq.h>
//#include <mach/regs-rtc.h>

/*******************************************
* Slave Address
*******************************************/
#define SEC_ADDR 		0x0
#define MIN_ADDR		0x1
#define HOUR_ADDR		0x2
#define WEEK_ADDR		0x3
#define DAY_ADDR		0x4
#define MONTH_ADDR		0x5
#define YEAR_ADDR		0x6
#define OSIL_ADDR		0x7
#define AL_WMIN_ADDR	0x8
#define AL_WHOUR_ADDR	0x9
#define AL_WWEEK_ADDR	0xA
#define AL_DMIN_ADDR	0xB
#define AL_DHOUR_ADDR	0xC
#define byte13_ADDR		0xD
#define CTRL_1_ADDR		0xE
#define CTRL_2_ADDR		0xF

#define R2051_MAX		16

/*******************************************
* Enum & Structure Define
*******************************************/
union SEC_REG{
	unsigned char	Byte;
	struct {
		unsigned char	S			:7;
		unsigned char				:1;
	} bit;
};

union MIN_REG{
	unsigned char	Byte;
	struct {
		unsigned char	M			:7;
		unsigned char				:1;
	} bit;
};

union HOUR_REG{
	unsigned char	Byte;
	struct {
		unsigned char	H			:6;
		unsigned char				:2;
	} bit;
};

union DAY_WEEK_REG{
	unsigned char	Byte;
	struct {
		unsigned char	W			:3;
		unsigned char				:5;
	} bit;
};

union DAY_MONTH_REG{
	unsigned char	Byte;
	struct {
		unsigned char	D	 		:6;
		unsigned char	 			:2;
	} bit;
};

union MON_REG{
	unsigned char	Byte;
	struct {
		unsigned char	M			:5;
		unsigned char				:2;
		unsigned char	C19_20		:1;
	} bit;
};

union YEAR_REG{
	unsigned char	Byte;
	struct {
		unsigned char	Y			:8;
	} bit;
};

union OSCILLATION{
	unsigned char	Byte;
	struct {
		unsigned char	F			:7;
		unsigned char	DEV			:1;
	} bit;
};

union ALARM_WM{
	unsigned char	Byte;
	struct {
		unsigned char	WM	 		:7;
		unsigned char				:1;
	} bit;
};

union ALARM_WH{
	unsigned char	Byte;
	struct {
		unsigned char	WH	 		:6;
		unsigned char		 		:1;
		unsigned char				:1;
	} bit;
};

union ALARM_WW{
	unsigned char	Byte;
	struct {
		unsigned char	WW	 		:7;
		unsigned char		 		:1;
	} bit;
};

union ALARM_DM{
	unsigned char	Byte;
	struct {
		unsigned char	DM			:7;
		unsigned char				:1;
	} bit;
};

union ALARM_DH{
	unsigned char	Byte;
	struct {
		unsigned char	DH			:6;
		unsigned char				:1;
		unsigned char				:1;
	} bit;
};

union CTRL_1 {
	unsigned char	Byte;
	struct {
		unsigned char	CT			:3;
		unsigned char	TEST		:1;
		unsigned char	SCRATCH2	:1;
		unsigned char	H12_24	 	:1;
		unsigned char	DALE		:1;
		unsigned char	WALE		:1;
	} bit;
};

union CTRL_2 {
	unsigned char	Byte;
	struct {
		unsigned char	DAFG		:1;
		unsigned char	WAFG		:1;
		unsigned char	CTFG		:1;
		unsigned char	SCRATCH1 	:1;
		unsigned char	PON			:1;
		unsigned char	XST			:1;
		unsigned char	VDET		:1;
		unsigned char	VDSL		:1;
	} bit;
};

union R2051K02_REG{
	unsigned char		bytes[ R2051_MAX ];
	struct {
		union SEC_REG			SEC;
		union MIN_REG			MIN;
		union HOUR_REG			HOUR;
		union DAY_WEEK_REG		WEEK;
		union DAY_MONTH_REG	DAY;
		union MON_REG			MON;
		union YEAR_REG			YEAR;

		union OSCILLATION		OSCIL;
		
		union ALARM_WM			AL_WMIN;
		union ALARM_WH			AL_WHOUR;
		union ALARM_WW			AL_WWEEK;
		union ALARM_DM			AL_DMIN;
		union ALARM_DH			AL_DHOUR;

		unsigned char				byte13;

		union CTRL_1			CTRL_1;
		union CTRL_2			CTRL_2;
	} info;
};

enum {
    CENTURY_19,
    CENTURY_20
};


struct r2051k02_rtc {
	struct device		*dev;
	struct i2c_client	*i2c_client;
	struct rtc_device	*rtc;
	union R2051K02_REG r2051k02_reg;
	int			rtc_reset;
};




unsigned char dec2hex(unsigned int s_dec)
{
	unsigned int quo, i;
	unsigned int hex[10];
	unsigned char result = 0;


	hex[0] = s_dec%10;
	quo = s_dec/10;
	i = 0;

	while(1)
	{
		if(quo == 0)
		{
			break;
		}
		else
		{
			hex[i+1] = quo%10;
			quo = quo/10;
		}
		i++;
	}

	while(i>0)
	{
		unsigned int j = i;
		while(j>0)
		{
			hex[i] = hex[i]*16;
			j--;
		}
		result += hex[i];
		i--;
	}
	result += hex[i];
	return result;
}

unsigned char hex2dec(unsigned int s_hex)
{
	unsigned int quo, i;
	unsigned int dec[10];
	unsigned char result = 0;


	dec[0] = s_hex%16;
	quo = s_hex/16;
	i = 0;

	while(1)
	{
		if(quo == 0)
		{
			break;
		}
		else
		{
			dec[i+1] = quo%16;
			quo = quo/16;
		}
		i++;
	}

	while(i>0)
	{
		unsigned int j = i;
		while(j>0)
		{
			dec[i] = dec[i]*10;
			j--;
		}
		result += dec[i];
		i--;
	}
	result += dec[i];
	return result;
}


int r2051k02_master_send(const struct i2c_client *client, const char *buf, int count)
{
	int ret;
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = count;
	msg.buf = (char *)buf;

	ret = i2c_transfer(adap, &msg, 1);

	return (ret == 1) ? count : ret;
}

int r2051k02_master_recv(const struct i2c_client *client, char *buf, int count)
{
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;
	int ret;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.flags |= I2C_M_RD;
	msg.len = count;
	msg.buf = buf;

	ret = i2c_transfer(adap, &msg, 1);

	/*
	 * If everything went ok (i.e. 1 msg received), return #bytes received,
	 * else error code.
	 */
	return (ret == 1) ? count : ret;
}

static int r2051k02_write_register(struct i2c_client *i2c, unsigned char addr, 
	unsigned char *data, unsigned char count)
{
	int ret;
	unsigned char buf[17]={0,};
	int i;

	buf[0] = (addr<<4) & 0xF0;

	for(i = 0; i < count; i++)
		buf[i+1] = data[i];

	ret = r2051k02_master_send(i2c, buf, count+1);
	if (ret < 0) {
		dev_err(&i2c->dev, "I2C send failed - %d\n", ret);
		return -EIO;
	}

	return 0;
}
static int r2051k02_read_register(struct i2c_client *i2c, unsigned char *data,
	unsigned char count)
{
	int ret;
	unsigned char temp[16] ={0,};
	int i;
	
	ret = r2051k02_master_recv(i2c, temp, count);
	if (ret < 0) {
		dev_err(&i2c->dev, "I2C recv data failed - %d\n", ret);
		return -EIO;
	}

	for(i = 0; i < count-1; i++)
		data[i] = temp[i+1];

	data[15] = temp[0];
	
	return 0;
}


#include <linux/d4_rmu.h>

 static void drime4_poweroff(void)
{
	struct d4_rmu_device * rmu;

	printk(KERN_EMERG"- shutdown -\n");

	rmu = d4_rmu_request();
	if (rmu == -12)
		return;

 	d4_pcu_intr_mask_set(NULL);
	d4_pcu_hold_set(rmu);
	d4_pcu_ddroff_set(rmu);
	d4_pcu_off_set(rmu);

	d4_rmu_release(rmu);
}


static int r2051k02_rtc_alarm_irq_clear(struct device *dev, int shutdown)
{
	struct r2051k02_rtc *r2051k02 = (struct r2051k02_rtc *)dev_get_drvdata(dev);
	union R2051K02_REG *reg = &r2051k02->r2051k02_reg;

	if(!r2051k02)
		return -1;

	if(reg->info.CTRL_2.bit.DAFG == 1){
		reg->info.CTRL_2.bit.DAFG = 0;
		reg->info.CTRL_2.bit.WAFG = 0;
		if(r2051k02_write_register(r2051k02->i2c_client, CTRL_2_ADDR, &(reg->bytes[CTRL_2_ADDR]), 1) != 0) return -1;
		if(shutdown){
			if(gpio_get_value(GPIO_POWER_ON)==1) //power key off
				drime4_poweroff();
		}
	}
	return 0;
}

static int r2051k02_rtc_alarm_enable(struct device *dev, unsigned int enabled)
{
	struct r2051k02_rtc *r2051k02 = (struct r2051k02_rtc *)dev_get_drvdata(dev);
	union R2051K02_REG *reg = &r2051k02->r2051k02_reg;

	if(!r2051k02)
		return -1;
	if(!reg){
		return -1;
	}
	
	if (enabled)
		reg->info.CTRL_1.bit.DALE = 1;
	else
		reg->info.CTRL_1.bit.DALE = 0;

	reg->info.CTRL_1.bit.WALE = 0;
	reg->info.CTRL_1.bit.H12_24 = 1;
	if(r2051k02_write_register(r2051k02->i2c_client, CTRL_1_ADDR, &(reg->bytes[CTRL_1_ADDR]), 1) != 0) return -1;	


	return 0;
}

static int r2051k02_rtc_open(struct device *dev)
{
	return 0;
}

static void r2051k02_rtc_release(struct device *dev)
{
	return 0;
}

static int r2051k02_rtc_gettime(struct device *dev, struct rtc_time *rtc_tm)
{
	struct r2051k02_rtc *r2051k02 = (struct r2051k02_rtc *)dev_get_drvdata(dev);
	union R2051K02_REG *reg = &r2051k02->r2051k02_reg;

	if(!r2051k02){
		return -1;
	}
	if(!reg){
		return -1;
	}

	if(r2051k02_read_register(r2051k02->i2c_client, &(reg->bytes[SEC_ADDR]),16) != 0) return -1;

	rtc_tm->tm_sec = hex2dec(reg->info.SEC.bit.S);
	rtc_tm->tm_min = hex2dec(reg->info.MIN.bit.M);
	rtc_tm->tm_hour = hex2dec(reg->info.HOUR.bit.H);
	rtc_tm->tm_mday = hex2dec(reg->info.DAY.bit.D);
	rtc_tm->tm_mon = hex2dec(reg->info.MON.bit.M) -1;

	if((reg->info.MON.bit.C19_20) == CENTURY_19)
		rtc_tm->tm_year = hex2dec(reg->info.YEAR.bit.Y)+1900;
	else 
		rtc_tm->tm_year = hex2dec(reg->info.YEAR.bit.Y)+2000;	

	rtc_tm->tm_year -= 1900;

//	printk(KERN_EMERG"r2051k02 read time %04d.%02d.%02d %02d:%02d:%02d\n",
//		 1900 + rtc_tm->tm_year,1+rtc_tm->tm_mon, rtc_tm->tm_mday,
//		 rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec);

	return rtc_valid_tm(rtc_tm);
}

static int r2051k02_rtc_settime(struct device *dev, struct rtc_time *tm)
{
	struct r2051k02_rtc *r2051k02 = (struct r2051k02_rtc *)dev_get_drvdata(dev);
	union R2051K02_REG *reg = &r2051k02->r2051k02_reg;

	if(!r2051k02)
		return -1;
	if(!reg){
		return -1;
	}

//	printk(KERN_EMERG"r2051k02 set time %04d.%02d.%02d %02d:%02d:%02d\n",
//		 1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
//		 tm->tm_hour, tm->tm_min, tm->tm_sec);

	(reg->info.SEC.bit.S) =dec2hex(tm->tm_sec);
	(reg->info.MIN.bit.M) = dec2hex(tm->tm_min);
	(reg->info.HOUR.bit.H) = dec2hex(tm->tm_hour);
	(reg->info.DAY.bit.D) = dec2hex(tm->tm_mday);
	(reg->info.MON.bit.M) = dec2hex(tm->tm_mon+1);

	if((tm->tm_year +1900) < 2000){
		(reg->info.MON.bit.C19_20) = CENTURY_19;
		(reg->info.YEAR.bit.Y) = dec2hex(tm->tm_year);
	}else{
		(reg->info.MON.bit.C19_20) = CENTURY_20;
		(reg->info.YEAR.bit.Y) = dec2hex(tm->tm_year - 100);
	}

	if(r2051k02_write_register(r2051k02->i2c_client, SEC_ADDR, &(reg->bytes[SEC_ADDR]), 7) != 0) return -1;	

	return 0;
}

static int r2051k02_rtc_getalarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct r2051k02_rtc *r2051k02 = (struct r2051k02_rtc *)dev_get_drvdata(dev);
	union R2051K02_REG *reg = &r2051k02->r2051k02_reg;
	struct rtc_time *alarm_tm = &alarm->time;
	int i =0;

	if(!r2051k02)
		return -1;
	if(!reg){
		return -1;
	}
	
	if(r2051k02_read_register(r2051k02->i2c_client, &(reg->bytes[SEC_ADDR]),16) !=0) return -1;

	alarm_tm->tm_min = hex2dec(reg->info.AL_DMIN.Byte);
	alarm_tm->tm_hour = hex2dec(reg->info.AL_DHOUR.Byte);

//	printk(KERN_EMERG"r2051k02 read alarm %02d:%02d\n",
//		 alarm_tm->tm_hour, alarm_tm->tm_min);

	return 0;
}

static int r2051k02_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct r2051k02_rtc *r2051k02 = (struct r2051k02_rtc *)dev_get_drvdata(dev);
	union R2051K02_REG *reg = &r2051k02->r2051k02_reg;
	struct rtc_time *alrm_tm = &alarm->time;

	int i =0;

	
	if(!r2051k02)
		return -1;
	if(!reg){
		return -1;
	}
	
//	printk(KERN_EMERG"r2051k02_rtc_set alarm: %02d:%02d\n",
//		 alrm_tm->tm_hour, alrm_tm->tm_min);

	r2051k02_rtc_alarm_enable(dev, 0);

	(reg->info.AL_WMIN.bit.WM) = 0;
	(reg->info.AL_WHOUR.bit.WH) = 0;
	(reg->info.AL_WWEEK.bit.WW) = 0;

	(reg->info.AL_DMIN.bit.DM) = dec2hex(alrm_tm->tm_min);
	(reg->info.AL_DHOUR.bit.DH) = dec2hex(alrm_tm->tm_hour);

	if(r2051k02_write_register(r2051k02->i2c_client, AL_WMIN_ADDR, &(reg->bytes[AL_WMIN_ADDR]), 5) != 0) return -1;

	r2051k02_rtc_alarm_enable(dev, 1);


//	if(r2051k02_read_register(r2051k02->i2c_client, &(reg->bytes[SEC_ADDR]),16) !=0) return -1;

	return 0;
}

static const struct rtc_class_ops r2051k02_rtcops = {
	.open		= r2051k02_rtc_open,
	.release		= r2051k02_rtc_release,
	.read_time	= r2051k02_rtc_gettime,
	.set_time		= r2051k02_rtc_settime,
//	.read_alarm	= r2051k02_rtc_getalarm,
//	.set_alarm	= r2051k02_rtc_setalarm,
//	.alarm_irq_enable = r2051k02_rtc_alarm_enable,
};


static int r2051k02_rtc_set_init(struct device *dev)
{
	struct r2051k02_rtc *r2051k02 = (struct r2051k02_rtc *)dev_get_drvdata(dev);
	union R2051K02_REG *reg = &r2051k02->r2051k02_reg;
	int i =0;

	if(!r2051k02)
		return -1;
	if(!reg){
		return -1;
	}

	if(r2051k02_read_register(r2051k02->i2c_client, &(reg->bytes[SEC_ADDR]),16) !=0) return -1;
	printk(KERN_EMERG"r2051k02 add [%d] data [0x%x]\n",15, (reg->bytes[15]));

	if((reg->info.CTRL_2.bit.PON == 1) || (reg->info.CTRL_2.bit.XST == 0) ){
		r2051k02->rtc_reset = 0x7; /* set rtc reset data for app */
		i = 4;

		while(i--){
			if(r2051k02_read_register(r2051k02->i2c_client, &(reg->bytes[SEC_ADDR]),16) !=0) return -1;
			printk(KERN_EMERG"r2051k02 add [%d] data [0x%x]\n",15, (reg->bytes[15]));

			if((reg->info.CTRL_2.bit.PON == 1) || (reg->info.CTRL_2.bit.XST == 0) ){
				/* check rtc reset */
				reg->info.CTRL_2.bit.DAFG = 0;
				reg->info.CTRL_2.bit.WAFG = 0;
				reg->info.CTRL_2.bit.CTFG = 0;
				reg->info.CTRL_2.bit.SCRATCH1 = 0;
				reg->info.CTRL_2.bit.PON = 0;
				reg->info.CTRL_2.bit.XST = 1;
				reg->info.CTRL_2.bit.VDET = 0;
				reg->info.CTRL_2.bit.VDSL = 1;

				if(r2051k02_write_register(r2051k02->i2c_client, CTRL_2_ADDR, &(reg->bytes[CTRL_2_ADDR]), 1) != 0) return -1;

				mdelay(300);
			} else {
				break;
			}
		}
	} else {
		r2051k02->rtc_reset = 0x0; /* clear rtc reset data for app */
	}
	
	r2051k02_rtc_alarm_enable(dev, 0);

	/* Oscillation Adjustment */
	reg->info.OSCIL.bit.DEV = 0;
	reg->info.OSCIL.bit.F     =  0x8;		// (32768.77 - 32768.05)*10+1 = 8.2 almost 8 (0x8)
//	reg->info.OSCIL.bit.F     =  0x45;		//(32766.33 - 327628.05)*10 = -27.2 almost = -27 (-0x5)
	if(r2051k02_write_register(r2051k02->i2c_client, OSIL_ADDR, &(reg->bytes[OSIL_ADDR]), 1) != 0) return -1;

	return 0;
}


unsigned int r2051k02_rtc_get_boot_info(struct device *dev)
{
	struct r2051k02_rtc *r2051k02 = (struct r2051k02_rtc *)dev_get_drvdata(dev);

	if(!r2051k02)
		return -1;

	printk(KERN_EMERG"rtc reset %d\n", r2051k02->rtc_reset);
	return r2051k02->rtc_reset;
}

void r2051k02_rtc_set_boot_info(struct device *dev)
{
	struct r2051k02_rtc *r2051k02 = (struct r2051k02_rtc *)dev_get_drvdata(dev);

	if(!r2051k02)
		return;
	
	r2051k02->rtc_reset = 0;
	printk(KERN_EMERG"rtc reset %d\n", r2051k02->rtc_reset);

}

static ssize_t
show_exrtc_boot_info(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	unsigned int data;
	ssize_t size = 0;
	data = r2051k02_rtc_get_boot_info(dev);
	size = sprintf(buf, "%d\n", data);
	return size;
}

static ssize_t
store_exrtc_boot_info(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	r2051k02_rtc_set_boot_info(dev);
	return 0;
}

static DEVICE_ATTR(boot_info, S_IRUGO | S_IWUSR,
			show_exrtc_boot_info, store_exrtc_boot_info);



int r2051k02_create_node(struct device *dev)
{
	int rc;
	if (!dev)
		return -1;

	rc = device_create_file(dev, &dev_attr_boot_info);
	if (rc) {
		dev_info(dev, "failed to create sysfs d4keys node\n");
		return -1;
	}
	return 0;
}

void r2051k02_remove_node(struct device *dev)
{
	device_remove_file(dev, &dev_attr_boot_info);
}


static int __devinit r2051k02_rtc_probe(struct i2c_client *client,
				      const struct i2c_device_id *id)
{
	struct r2051k02_rtc *r2051k02= NULL;
	struct rtc_time rtc_tm;
	int ret = 0;

	r2051k02 = kzalloc(sizeof(struct r2051k02_rtc), GFP_KERNEL);
	if (r2051k02 == NULL)
		return -ENOMEM;

	r2051k02->dev = &client->dev;
	r2051k02->i2c_client = client;
	
	i2c_set_clientdata(client, r2051k02);

	if (IS_ERR(r2051k02->rtc)) {
		dev_err(&client->dev, "r2051k02 cannot attach rtc\n");
		ret = PTR_ERR(r2051k02->rtc);
		goto fail_nortc;
	}

	if(r2051k02_rtc_set_init(&client->dev) != 0)
		goto fail_nortc;

	if(r2051k02_rtc_alarm_irq_clear(&client->dev, 0) != 0)
		goto fail_nortc;		


	r2051k02_create_node(&client->dev);
	
	r2051k02->rtc = rtc_device_register("r2051k02", &client->dev, &r2051k02_rtcops,
				  THIS_MODULE);

	/* Check RTC Time */
	if(r2051k02_rtc_gettime(&client->dev, &rtc_tm) == -1)
		goto fail_nortc;

	if(rtc_valid_tm(&rtc_tm)) {
		rtc_tm.tm_year	= 111;
		rtc_tm.tm_mon	= 6;
		rtc_tm.tm_mday	= 7;
		rtc_tm.tm_hour	= 12;
		rtc_tm.tm_min	= 59;
		rtc_tm.tm_sec		= 55;
		r2051k02_rtc_settime(&client->dev, &rtc_tm);
		dev_warn(&client->dev, "warning: invalid RTC value so initializing it\n");
	}

	return 0;

fail_nortc:
	i2c_set_clientdata(client, NULL);
	kfree (r2051k02);
	return ret;
}

static int __devexit r2051k02_rtc_remove(struct i2c_client *client)
{
	struct r2051k02_rtc *r2051k02 = i2c_get_clientdata(client);

	if(!r2051k02)
		return -1;

	kfree (r2051k02);
	return 0;
}

#ifdef CONFIG_PM
static int r2051k02_rtc_suspend(struct i2c_client *client, pm_message_t state)
{
	struct r2051k02_rtc *r2051k02 = i2c_get_clientdata(client);

	struct rtc_time time = {0,};
	struct rtc_wkalrm alarm;
//	printk(KERN_EMERG"r2051k02_rtc_suspend\n");

	if(r2051k02){
		r2051k02_rtc_gettime(&client->dev, &time);
//			return -1;
#if 0

//	if(time.tm_hour == 23){
//		alarm.time.tm_min  = time.tm_min;
//		alarm.time.tm_hour = 0 ;
//	}else{
//		alarm.time.tm_min  = time.tm_min;
//		alarm.time.tm_hour = time.tm_hour + 1; ;
//	}

	alarm.time.tm_min  = time.tm_min + 2;
	alarm.time.tm_hour = time.tm_hour;

	if(alarm.time.tm_min > 59){
		alarm.time.tm_min  = alarm.time.tm_min - 60;
		alarm.time.tm_hour += 1;
	}
#endif

		if(time.tm_min == 0){
			alarm.time.tm_min = 60 - 1;

			if(time.tm_hour == 0)
				alarm.time.tm_hour = 24 - 1;
			else
				alarm.time.tm_hour = time.tm_hour - 1;
		}else{
			alarm.time.tm_min  = time.tm_min - 1;
			alarm.time.tm_hour = time.tm_hour;
		}

		r2051k02_rtc_setalarm(&client->dev, &alarm);
//			return -1;
	}
	return 0;
}

static int r2051k02_rtc_resume(struct i2c_client *client)
{
	struct r2051k02_rtc *r2051k02 = i2c_get_clientdata(client);
	struct rtc_time rtc_tm;

	if(r2051k02){
		i2c_set_clientdata(client, r2051k02);
		if(r2051k02_rtc_set_init(&client->dev) != 0)
			return -1;

		if(r2051k02_rtc_alarm_irq_clear(&client->dev, 1) != 0)
			return -1;
#if 1
		if(r2051k02_rtc_gettime(&client->dev, &rtc_tm) !=0) {
			rtc_tm.tm_year	= 111;
			rtc_tm.tm_mon	= 6;
			rtc_tm.tm_mday	= 7;
			rtc_tm.tm_hour	= 12;
			rtc_tm.tm_min	= 59;
			rtc_tm.tm_sec		= 55;
			r2051k02_rtc_settime(&client->dev, &rtc_tm);
		}
#endif
	}
	return 0;
}

#else
#define r2051k02_rtc_suspend	NULL
#define r2051k02_rtc_resume	NULL
#endif


static const struct i2c_device_id r2051k02_i2c_id[] = {
	{ R2051K02_I2C_DEV_NAME, 0 },
	{ }
};

static struct i2c_driver r2051k02_i2c_rtc_driver = {
	.probe = r2051k02_rtc_probe,
	.remove = __devexit_p(r2051k02_rtc_remove),
	.suspend = r2051k02_rtc_suspend,
	.resume = r2051k02_rtc_resume,
	.id_table = r2051k02_i2c_id,
	.driver = {
		.name = R2051K02_I2C_DEV_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init r2051k02_rtc_init(void)
{
	return i2c_add_driver(&r2051k02_i2c_rtc_driver);
}
static void __exit r2051k02_rtc_exit(void)
{
	i2c_del_driver(&r2051k02_i2c_rtc_driver);
}

module_init(r2051k02_rtc_init);
module_exit(r2051k02_rtc_exit);

MODULE_DESCRIPTION("R2051K02 RTC Driver");
MODULE_AUTHOR("TaeKyung Kim <tksss.kim@samsung.com>");
