/* linux/drivers/misc/sprotocol/protocol_ioctl.c
 *
 * Copyright (c) 2011 Samsung Electronics
 * Kyuchun han <kyuchun.han@samsung.com>
  *
 * Slave protocol device driver using SPI Connection
  *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <mach/dma.h>
#include <linux/miscdevice.h>
#include <mach/gpio.h>
#include <mach/map.h>
#include <linux/fb.h>
#include <linux/hs_spi.h>
#include <linux/spi/d4_spi_config.h>
#include <linux/spi/protocol_ioctl.h>
#include <linux/spi/protocol_config.h>
#include <linux/d4_rmu.h>
#include <mach/d4_mem.h>


#define READYBUSY_HIGH		1
#define READYBUSY_LOW		0

#define NOTIFICATION_HIGH		1
#define NOTIFICATION_LOW		0

struct hs_spi_data *spi_ch;
struct miscdevice *protocoldev;

static int protocol_ch = -1;
static int p_rb;
static int p_noti;
static int protocol_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int protocol_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct d4_rmu_device *pcu;
unsigned char *temp_buffer;
static long protocol_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	u32 size;
	int ret;

	struct spi_data_info spi_rwdata;
	struct spi_data_info spi_kdata;
	struct d4_hs_spi_config spi_data;
	unsigned int ready_busy;
	unsigned int kaddress;
	void *kvaddr;
	ret = 0;
	size = _IOC_SIZE(cmd);
	switch (cmd) {
	case PROTOCOL_IOCTL_SET:
	ret = copy_from_user((void *)&spi_data, (const void *)arg, size);
	if (ret < 0) {
		printk("ioctl fail: [%d]", cmd);
		return ret;
	}
	hs_spi_config(spi_ch, &spi_data);
	break;

	case PROTOCOL_IOCTL_INT_SREAD:
		ret = copy_from_user((void *)&spi_rwdata, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		hs_spi_slave_set(spi_ch);
		temp_buffer = kzalloc(spi_rwdata.data_len, GFP_KERNEL);

		memset(temp_buffer, 0x00, spi_rwdata.data_len);

		spi_kdata.data_len = spi_rwdata.data_len;
		spi_kdata.rbuffer = temp_buffer;
		spi_kdata.wbuffer = NULL;

		hs_spi_slave_interrupt_read(spi_ch, &spi_kdata, p_rb, p_noti, READYBUSY_HIGH, NOTIFICATION_HIGH);
		spi_rwdata.data_len = spi_kdata.data_len;

		ret = copy_to_user((void *)arg, (const void *)&spi_rwdata, size);
		if (ret)
			ret = -EFAULT;
		ret = copy_to_user((void *)spi_rwdata.rbuffer,
				spi_kdata.rbuffer, spi_kdata.data_len);
		if (ret)
			ret = -EFAULT;
		kfree(temp_buffer);
		break;

	case PROTOCOL_IOCTL_INT_SWRITE:
		ret = copy_from_user((void *)&spi_rwdata, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		hs_spi_slave_set(spi_ch);

		temp_buffer = kzalloc(spi_rwdata.data_len, GFP_KERNEL);

		memcpy(temp_buffer, spi_rwdata.wbuffer, spi_rwdata.data_len);
		spi_kdata.data_len = spi_rwdata.data_len;
		spi_kdata.rbuffer = NULL;
		spi_kdata.wbuffer = temp_buffer;
		hs_spi_slave_interrupt_write(spi_ch, &spi_kdata, p_rb, p_noti, READYBUSY_LOW, NOTIFICATION_HIGH);
		kfree(temp_buffer);
		break;

	case PROTOCOL_IOCTL_INT_FW_READ:
		ret = copy_from_user((void *)&spi_rwdata, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		hs_spi_slave_set(spi_ch);

		spi_kdata.data_len = spi_rwdata.data_len;
		spi_kdata.rbuffer = spi_rwdata.rbuffer;
		spi_kdata.wbuffer = NULL;
		hs_spi_slave_interrupt_burst_read(spi_ch, &spi_kdata, p_rb, p_noti, READYBUSY_HIGH, NOTIFICATION_HIGH);
		ret = copy_to_user((void *)arg, (const void *)&spi_rwdata, size);
		if (ret)
			ret = -EFAULT;
		break;

	case PROTOCOL_IOCTL_INT_NOTI_WRITE:
		ret = copy_from_user((void *)&spi_rwdata, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		hs_spi_slave_set(spi_ch);

		temp_buffer = kzalloc(spi_rwdata.data_len, GFP_KERNEL);

		memcpy(temp_buffer, spi_rwdata.wbuffer, spi_rwdata.data_len);
		spi_kdata.data_len = spi_rwdata.data_len;
		spi_kdata.rbuffer = NULL;
		spi_kdata.wbuffer = temp_buffer;
		hs_spi_slave_interrupt_write(spi_ch, &spi_kdata, p_rb, p_noti, READYBUSY_HIGH, NOTIFICATION_LOW);
		kfree(temp_buffer);
		break;

	case PROTOCOL_IOCTL_INT_DONE:
		hs_spi_int_done(spi_ch);
		break;

	case PROTOCOL_IOCTL_RB_CTRL:
		ret = copy_from_user((void *)&ready_busy, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		gpio_direction_output(p_rb, ready_busy);
		break;
	case PROTOCOL_IOCTL_POWER_OFF:
		printk("kernel test for power off\n");
		pcu = d4_rmu_request();
		d4_pcu_hold_set(pcu);
		d4_pcu_off_set(pcu);
		printk("kernel test for power off end\n");
		while(1);
		break;

	default:
		break;
	}
	return ret;
}


const struct file_operations protocol_ops = { .open = protocol_open,
		.release = protocol_release, .unlocked_ioctl = protocol_ioctl };

static void protocol_spi_set(struct miscdevice *mdmadev, const char *fmt, ...)
{
	va_list vargs;
	va_start(vargs, fmt);
	mdmadev->name = kvasprintf(GFP_KERNEL, fmt, vargs);
	mdmadev->minor = MISC_DYNAMIC_MINOR;
	mdmadev->fops = &protocol_ops;
}


static int protocol_spi_misc_set(void)
{
	int ret;
	protocoldev = kzalloc(sizeof(*protocoldev), GFP_KERNEL);

	if (!protocoldev) {
		printk("no memory for state\n");
		return -ENOMEM;;
	}
	protocol_spi_set(protocoldev, "protocol_spi");
	ret = misc_register(protocoldev);
	if (ret < 0) {
		printk("failed to register MDMA ioctl\n");
		kfree(protocoldev);
		return -ENODEV;
	}

	return 0;
}


void protocol_data_register(struct protocol_platform_data *info)
{
	protocol_ch = info->protocol_ch;
	p_rb = info->int_pin_num;
}


static int __init protocol_spi_init(void)
{
	int err = -1;

	if (protocol_ch == -1)
		return err;

	p_rb = GPIO_SLAVE_RB;
	p_noti = GPIO_ISP_INT;

	err = gpio_request(p_rb, "");
	err = gpio_direction_output(p_rb, 0);

	err = gpio_request(p_noti, "");
	err = gpio_direction_output(p_noti, 0);

	spi_ch = hs_spi_request(protocol_ch);

	if (IS_ERR(spi_ch)) {
		err = PTR_ERR(spi_ch);
		return err;
	}

	err = protocol_spi_misc_set();
	return err;
}


module_init(protocol_spi_init);

static void __exit protocol_spi_exit(void)
{
	misc_deregister(protocoldev);
	protocol_ch = -1;
	return;
}
module_exit(protocol_spi_init);

MODULE_AUTHOR("kyuchun han, <kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("Slave protocol Controller Driver");
MODULE_LICENSE("GPL");

