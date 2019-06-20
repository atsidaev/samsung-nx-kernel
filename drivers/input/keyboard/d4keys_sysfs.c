/*
 * drivers/input/keyboard/d4key_sysfs.c
 *
 * This driver is to create sysfs node to notify d4key_sysfs status to user area.
 *
 * Copyright (C) 2012 for Samsung Electronics
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
 *
 */

#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/uaccess.h>
#include <mach/gpio.h>
#include <mach/map.h>
#include <linux/io.h>

/* devman enum
	enum bootmode{
		mode_smart = 0,
		mode_p,
		mode_a,
		mode_s,
		mode_m,
		mode_i,
		mode_magic,
		mode_wifi,
		mode_scene,
		mode_movie,
	};
*/

int g_boot_mode;
int g_boot_shutter;
int g_boot_power;
static int keymask_status = 0;
int oneshot;
int g_adckey_value;
int oneshot_direct=0;
enum {
	CPU_LIVE_CLK = 0x0,
	CPU_MOVIE_CLK = 0x311,
};

extern int d4_keys_polled_update_keymask_state(int keymask_state);
extern int d4_keys_update_keymask_state(int keymask_state);
extern void d4_melfas_update_touchmask_state(int touchmask_state);

static int atoi(const char *name)
{
    int val = 0;

    for (;; name++) {
        switch (*name) {
            case '0'...'9':
                val = 10 * val + (*name-'0');
                break;
            default:
                return val;
        }
    }

    return val;
}



static ssize_t
store_cpuclk(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t size)
{
	int value = 0;
	unsigned long state = 0;
	ssize_t ret = -EINVAL;

	
	ret = kstrtoul(buf, 2, &state);
	if (ret)
		return ret;

	if(state == 1) // liveview
		__raw_writel(CPU_LIVE_CLK, (DRIME4_VA_CPU_SYS+0x0));
	else // movie
		__raw_writel(CPU_MOVIE_CLK, (DRIME4_VA_CPU_SYS+0x0));

	value = __raw_readl(DRIME4_VA_CPU_SYS+0x0);
	
	printk("state %d, 0x%x,\n", state, value);
	return size;
}

static ssize_t
show_cpuclk(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;

	return size;
}
static DEVICE_ATTR(cpuclk, S_IRUGO | S_IWUSR,
			show_cpuclk,store_cpuclk);


static ssize_t
show_adckey_value(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
		
	size = sprintf(buf, "%d\n" , g_adckey_value);
	return size;
}

static DEVICE_ATTR(adckey_value, S_IRUGO | S_IWUSR,
			show_adckey_value,NULL);

#if defined (CONFIG_MACH_D4_NX300) || defined (CONFIG_MACH_D4_NX2000)
static ssize_t
store_1shot(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t size)
{

	unsigned long state = 0;
	ssize_t ret = -EINVAL;

	ret = kstrtoul(buf, 10, &state);
	if (ret)
		return ret;

	if(state == 1 || state == 2){
		oneshot = 1;
	}else
		oneshot = 0;

	if(state == 9){
		oneshot_direct = 1;
	}

	printk(KERN_EMERG"1shot[%d], state %d,\n", oneshot, state);
	return size;
}

static ssize_t
show_1shot(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;

	printk("show oneshot %d\n",oneshot);

	return size;
}
static DEVICE_ATTR(1shot, S_IRUGO | S_IWUSR,
			show_1shot,store_1shot);
#endif

static ssize_t
show_boot_power(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	
	if(gpio_get_value(GPIO_POWER_ON)==0)
		g_boot_power = 1;
	else
		g_boot_power = 0;
		
	size = sprintf(buf, "%d\n" , g_boot_power);
	return size;
}

static DEVICE_ATTR(pw, S_IRUGO | S_IWUSR,
			show_boot_power,NULL);



static ssize_t
show_boot_shutter(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	
	if(gpio_get_value(GPIO_SHUTTER_KEY2)==0)
		g_boot_shutter = 2;
	else
		if(gpio_get_value(GPIO_SHUTTER_KEY1_DSP) == 0)
			g_boot_shutter = 1;
		else
			g_boot_shutter = 0;

		
	size = sprintf(buf, "%d\n" , g_boot_shutter);
	return size;
}

static DEVICE_ATTR(shutter, S_IRUGO | S_IWUSR,
			show_boot_shutter,NULL);


static ssize_t
show_boot_mode(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	int mode;

	switch (g_boot_mode) {
	case KEY_F1:
		mode = 0;
		break;
	case KEY_F2:
		mode = 1;
		break;
	case KEY_F3:
		mode = 2;
		break;
	case KEY_F4:
		mode = 3;
		break;
	case KEY_F5:
		mode = 4;
		break;
	case KEY_F6:
		mode = 5;
		break;
	case KEY_F7:
		mode = 6;
		break;
	case KEY_F8:
		mode = 7;
		break;
	case KEY_F9:
		mode = 8;
		break;
	case KEY_F10:
		mode = 9;
		break;
	default:
		mode = -1;
	}

	size = sprintf(buf , "%d\n" , mode);
	return size;
}

static ssize_t
store_boot_mode(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	return 0;
}

 static DEVICE_ATTR(mode, S_IRUGO | S_IWUSR,
			show_boot_mode, store_boot_mode);


static ssize_t 
show_key_mask(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	size = sprintf(buf, "%d\n",keymask_status);

	return size;
}

static ssize_t 
store_key_mask(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	int value;

	value = atoi(buf);

	keymask_status = value;
	printk("keymask_status [%x] \n",value);

	d4_keys_polled_update_keymask_state(keymask_status);
	d4_keys_update_keymask_state(keymask_status);
	d4_melfas_update_touchmask_state(keymask_status);
	return 0;
}

 static DEVICE_ATTR(keymask, S_IRUGO | S_IWUSR,
			show_key_mask, store_key_mask);



int d4keys_create_node(struct device *dev)
{
	int rc;

	/* initialize error check variables */
	g_boot_mode = -1;

	if (!dev)
		return -1;
	rc = device_create_file(dev, &dev_attr_mode);
	if (rc) {
		dev_info(dev, "failed to create sysfs d4keys node\n");
		return -1;
	}
	rc = device_create_file(dev, &dev_attr_shutter);
	if (rc) {
		dev_info(dev, "failed to create sysfs d4keys node\n");
		return -1;
	}
	rc = device_create_file(dev, &dev_attr_keymask);
	if (rc) {
		dev_info(dev, "failed to create sysfs d4keys node\n");
		return -1;
	}
	rc = device_create_file(dev, &dev_attr_pw);
	if (rc) {
		dev_info(dev, "failed to create sysfs d4keys node\n");
		return -1;
	}
#if defined (CONFIG_MACH_D4_NX300) || defined (CONFIG_MACH_D4_NX2000)
	rc = device_create_file(dev, &dev_attr_1shot);
	if (rc) {
		dev_info(dev, "failed to create sysfs d4keys node\n");
		return -1;
	}
#endif
	rc = device_create_file(dev, &dev_attr_adckey_value);
	if (rc) {
		dev_info(dev, "failed to create sysfs d4keys node\n");
		return -1;
	}
	rc = device_create_file(dev, &dev_attr_cpuclk);
	if (rc) {
		dev_info(dev, "failed to create sysfs d4keys node\n");
		return -1;
	}

	return 0;
}

void d4keys_remove_node(struct device *dev)
{
	device_remove_file(dev, &dev_attr_mode);
	device_remove_file(dev, &dev_attr_shutter);
	device_remove_file(dev, &dev_attr_keymask);
	device_remove_file(dev, &dev_attr_pw);
#if defined (CONFIG_MACH_D4_NX300) || defined (CONFIG_MACH_D4_NX2000)
	device_remove_file(dev, &dev_attr_1shot);
#endif
	device_remove_file(dev, &dev_attr_adckey_value);
	device_remove_file(dev, &dev_attr_cpuclk);
}

