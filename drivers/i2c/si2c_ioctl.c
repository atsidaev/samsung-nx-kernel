/**
 * @file si2c_drime4.c
 * @brief DRIMe4 Slave I2C Platform Driver
 * @author Kyuchun han <kyuchun.han@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#include <mach/d4_si2c.h>
#include <mach/d4_si2c_type.h>
#include <linux/si2c_drime4.h>
#include <linux/d4_si2c_config.h>
#include <linux/si2c_ioctl.h>
#include <linux/miscdevice.h>

struct devsi2c_dev_data {
	dev_t devt;
	struct drime4_si2c * si2c_ch;
	struct device *dev;
	struct list_head device_entry;
};

static LIST_HEAD(device_list);

#define msecs_to_loops(t) (loops_per_jiffy / 1000 * HZ * t)

static int drime4_si2c_open(struct inode *inode, struct file *filp)
{
	struct devsi2c_dev_data *data;
	int status = -1;
	list_for_each_entry(data, &device_list, device_entry) {
		if (data->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}

	if (status == 0) {
		nonseekable_open(inode, filp);
		filp->private_data = data;
	}
	return status;
}

static int drime4_si2c_release(struct inode *inode, struct file *filp)
{
	printk("si2c_close test\n");
	return 0;
}

int d4_si2c_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size = 0;

	size = (vma->vm_end - vma->vm_start);

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
					vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}


static long drime4_si2c_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	u32 size;
	u32 ret;
	unsigned char value;
	int time;
	int wait_time;
	struct d4_si2c_config conf;
	struct devsi2c_dev_data *data = (struct devsi2c_dev_data *) filp->private_data;

	value = 0;
	time = 0;
	wait_time = -1;
	size = _IOC_SIZE(cmd);
	switch (cmd) {
	case SI2C_SIOCTL_SET:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		drime4_si2c_config(data->si2c_ch, SI2C_SRECEIVE, value);
		break;
	case SI2C_ACK_WAIT:
		ret = copy_from_user((void *) &time, (const void *) arg, size);

		if (time != -1) {
			wait_time = msecs_to_loops(time);
		}
		drime4_si2c_ack_waiting(data->si2c_ch, wait_time);
		break;
	case SI2C_RECIVE_ADD_CHECK:
		ret = drime4_si2c_recive_addr_check(data->si2c_ch);
		copy_to_user((void *) arg, (const void *) &ret, size);
		break;
	case SI2C_BYTE_READ:
		ret = drime4_si2c_byte_read(data->si2c_ch);
		copy_to_user((void *) arg, (const void *) &ret, size);
		break;
	case SI2C_RECIVE_STOP:
		drime4_si2c_recive_stop(data->si2c_ch);
		break;

	case SI2C_TRANSMIT_ADD_CHECK:
		drime4_si2c_transmit_addr_check(data->si2c_ch);
		copy_to_user((void *) arg, (const void *) &ret, size);
		break;

	case SI2C_BYTE_WRITE:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		drime4_si2c_byte_write(data->si2c_ch, value);
		break;

	case SI2C_LAST_WRITE:
		ret = copy_from_user((void *) &value, (const void *) arg, size);
		drime4_si2c_last_byte_write(data->si2c_ch, value);
		break;

	case SI2C_TRANSMIT_STOP:
		drime4_si2c_transmit_stop(data->si2c_ch);
		break;

	case SI2C_RECIVE_SLAVE:
		ret = copy_from_user((void *) &conf, (const void *) arg, size);
		drime4_si2c_ack_waiting(data->si2c_ch, conf.wait_time);
		ret = drime4_si2c_recive_addr_check(data->si2c_ch);
		conf.ret_val = ret;
		copy_to_user((void *) arg, (const void *) &conf, size);
		break;

	case SI2C_TRANSMIT_SLAVE:
		ret = copy_from_user((void *) &conf, (const void *) arg, size);
		drime4_si2c_ack_waiting(data->si2c_ch, conf.wait_time);
		ret = drime4_si2c_transmit_addr_check(data->si2c_ch);
		conf.ret_val = ret;
		copy_to_user((void *) arg, (const void *) &conf, size);
		break;

	default:
		break;
	}
	return 0;
}



static ssize_t drime4_si2c_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	u8 *buffer;
	int ret;
	unsigned int cnt;
	struct devsi2c_dev_data *data = (struct devsi2c_dev_data *) filp->private_data;

	buffer = kmalloc(count, GFP_KERNEL);

	cnt = 0;
	ret = 0;

	while(1) {
		drime4_si2c_ack_waiting(data->si2c_ch, msecs_to_loops(500));
		ret = drime4_status_check(data->si2c_ch);
		if (ret) {
			drime4_si2c_err_stop(data->si2c_ch);
			return -1;
		}
		buffer[cnt] = drime4_si2c_byte_read(data->si2c_ch);

		if (cnt == (count-1))
			break;

		cnt++;
	}
	copy_to_user(buf, buffer, count);
	kfree(buffer);
	return 0;
}

/* Write-only message with current device setup */
static ssize_t drime4_si2c_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	u8 *buffer;
	int ret;
	unsigned int cnt;
	struct devsi2c_dev_data *data = (struct devsi2c_dev_data *) filp->private_data;
	buffer = kmalloc(count, GFP_KERNEL);
	cnt = 0;
	copy_from_user(buffer, buf, count);

	while(1) {
		if(cnt < (count-1)) {
			drime4_si2c_byte_write(data->si2c_ch, buffer[cnt]);
			drime4_si2c_ack_waiting(data->si2c_ch, msecs_to_loops(500));
			ret = drime4_status_check(data->si2c_ch);
			if (ret) {
				drime4_si2c_err_stop(data->si2c_ch);
				return -1;
			}
			cnt++;
		} else {
			drime4_si2c_last_byte_write(data->si2c_ch, buffer[cnt]);
			drime4_si2c_ack_waiting(data->si2c_ch, msecs_to_loops(500));
			break;
		}
	}
	kfree(buffer);
	return 0;
}


const struct file_operations drime4_si2c_ops = {
		.owner = THIS_MODULE,
		.open = drime4_si2c_open,
		.release = drime4_si2c_release,
		.read = drime4_si2c_read,
		.write = drime4_si2c_write,
		.unlocked_ioctl = drime4_si2c_ioctl,
		.mmap = d4_si2c_mmap
};


static void si2cdev_set(struct miscdevice *spidev, const char *fmt, ...)
{

	va_list vargs;
	va_start(vargs, fmt);
	spidev->name = kvasprintf(GFP_KERNEL, fmt, vargs);
	spidev->minor = MISC_DYNAMIC_MINOR;
	spidev->fops = &drime4_si2c_ops;


}

static int devsi2c_io_probe(struct platform_device *pdev)
{

	struct devsi2c_dev_data *data;
	struct miscdevice *si2cdev;
	int ret = 0;

	si2cdev = kzalloc(sizeof(*si2cdev), GFP_KERNEL);

	if (!si2cdev) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_allocdev;
	}

	si2cdev_set(si2cdev, "si2cdev.%d", pdev->id);

	data = kzalloc(sizeof(*data), GFP_KERNEL);

	if (!data) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	data->dev = &pdev->dev;
	INIT_LIST_HEAD(&data->device_entry);

	data->si2c_ch = drime4_si2c_request(pdev->id);

	if (IS_ERR(data->si2c_ch)) {
		dev_err(&pdev->dev, "unable to request i2c\n");
		ret = PTR_ERR(data->si2c_ch);
		goto err_si2c;
	} else
		dev_dbg(&pdev->dev, "got i2c channel\n");

	ret = misc_register(si2cdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register i2c io ctrl\n");
		goto err_bl;
	}
	data->devt = MKDEV(10, si2cdev->minor);
	list_add(&data->device_entry, &device_list);
	platform_set_drvdata(pdev, data);

	return 0;

err_bl:

err_si2c:
	kfree(data);

err_alloc:
	kfree(si2cdev);

err_allocdev:
	return ret;
}

static struct platform_driver si2c_io_driver = { .driver = {
	.name = "si2cdev",
	.owner = THIS_MODULE,
}, .probe = devsi2c_io_probe, .remove = NULL,
		.suspend = NULL, .resume = NULL, };

static int __init si2c_io_init(void)
{
	return platform_driver_register(&si2c_io_driver);
}
module_init(si2c_io_init);

static void __exit si2c_io_exit(void)
{
	platform_driver_unregister(&si2c_io_driver);
}
module_exit(si2c_io_exit);


MODULE_AUTHOR("Kyuchun Han <kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("Samsung Drime IV SI2C Driver");
MODULE_LICENSE("GPL");
