/* linux/drivers/spi/hs_spi_ioctl.c
 *
 * Copyright (c) 2011 Samsung Electronics
 * Kyuchun han <kyuchun.han@samsung.com>
 *
 * DRIME4 SPI platform device driver using SPI framework
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <mach/map.h>
#include <linux/hs_spi.h>
#include <linux/hs_spi_ioctl.h>
#include <linux/pinctrl/pinmux.h>

#include <linux/mm.h>

#include <linux/gpio.h>
#include <mach/gpio.h>

#include <linux/d4-pmu.h>

#define HS_SPI_MODULE_NAME		"hs_spidev"

#define msecs_to_loops(t) (loops_per_jiffy / 1000 * HZ * t)


struct hs_spi_dev_data {
	dev_t devt;
	struct hs_spi_data *spi_ch;
	struct device *dev;
	unsigned int period;
	struct list_head device_entry;
};

/*#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))*/

/*#define DRIME4_VA_GPIO2 0xFFFFFFFF*/
static LIST_HEAD(device_list);

static long hs_spi_io_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	unsigned int value;
	u32 size;
	int ret;
	unsigned int wait_time;
	struct hs_spi_dev_data *data =
			(struct hs_spi_dev_data *) filp->private_data;

	struct d4_hs_spi_config spi_data;
	struct spi_data_info spi_rwdata;
	struct spi_phys_reg_info spi_phyinfo;
	struct spi_pin_info pin_ctrl;
	struct spi_data_info spi_kdata;

	unsigned char *temp_buffer;

	value = 0;
	ret = 0;
	size = _IOC_SIZE(cmd);
	switch (cmd) {
	case HS_SPI_IOCTL_SET:
		ret = copy_from_user((void *)&spi_data, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		hs_spi_config(data->spi_ch, &spi_data);
		break;

	case HS_SPI_PHY_INFO:
		hs_spi_get_phy_info(data->spi_ch, &spi_phyinfo);
		ret = copy_to_user((void *) arg, (const void *) &spi_phyinfo, size);
		if (ret)
			ret = -EFAULT;
		break;

	case HS_SPI_WAIT_TIME:
		ret = copy_from_user((void *)&wait_time, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		wait_time = msecs_to_loops(wait_time);
		ret = copy_to_user((void *) arg, (const void *) &wait_time, size);
		if (ret)
			ret = -EFAULT;
		break;

	case HS_SPI_RESET:
		hs_spi_pad_reset(data->spi_ch);
		break;

	case HS_SPI_BURST:
		ret = copy_from_user((void *) &value, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		hs_spi_burst_set(data->spi_ch, value);
		break;


	case HS_SPI_PAD_SET:
		ret = copy_from_user((void *)&pin_ctrl, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		hs_spi_pin_set(data->spi_ch, &pin_ctrl);
		break;

	case HS_SPI_IOCTL_INT_DONE:
		hs_spi_int_done(data->spi_ch);
		break;

	default:
		break;
	}
	return ret;
}

static int hs_spi_io_open(struct inode *inode, struct file *filp)
{
	struct hs_spi_dev_data *data;
	int status = -1;
	list_for_each_entry(data, &device_list, device_entry) {
		if (data->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}
	if (status == 0) {
		filp->private_data = data;
		nonseekable_open(inode, filp);
	}
	return status;

}

static int hs_spi_io_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t hs_spi_io_read(struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos)
{
	return 0;
}

/* Write-only message with current device setup */
static ssize_t hs_spi_io_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	return 0;
}

int hs_spi_io_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size = 0;

	size = (vma->vm_end - vma->vm_start);

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
			vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}


const struct file_operations hs_spi_io_ops = { .open = hs_spi_io_open,
		.release = hs_spi_io_release, .read = hs_spi_io_read,
		.write = hs_spi_io_write, .unlocked_ioctl = hs_spi_io_ioctl,
		.mmap = hs_spi_io_mmap };

static void hs_spidev_set(struct miscdevice *spidev, const char *fmt, ...)
{

	va_list vargs;
	va_start(vargs, fmt);
	spidev->name = kvasprintf(GFP_KERNEL, fmt, vargs);
	spidev->minor = MISC_DYNAMIC_MINOR;
	spidev->fops = &hs_spi_io_ops;
	printk("hs_spidev %s\n", spidev->name);

}

static int hs_spi_io_probe(struct platform_device *pdev)
{
	struct hs_spi_dev_data *data;
	struct miscdevice *hs_spidev;
	int ret = 0;

	hs_spidev = kzalloc(sizeof(*hs_spidev), GFP_KERNEL);

	if (!hs_spidev) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_allocdev;
	}

	hs_spidev_set(hs_spidev, "hs_spidev.%d", pdev->id);

	data = kzalloc(sizeof(*data), GFP_KERNEL);

	if (!data) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	data->dev = &pdev->dev;
	INIT_LIST_HEAD(&data->device_entry);

	data->spi_ch = hs_spi_request(pdev->id);

	if (IS_ERR(data->spi_ch)) {
		dev_err(&pdev->dev, "unable to request spi\n");
		ret = PTR_ERR(data->spi_ch);
		goto err_spi;
	} else
		dev_dbg(&pdev->dev, "got spi channel\n");

	ret = misc_register(hs_spidev);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register spi io ctrl\n");
		goto err_bl;
	}
	data->devt = MKDEV(10, hs_spidev->minor);
	list_add(&data->device_entry, &device_list);
	platform_set_drvdata(pdev, data);

	/*
	 if ( pdev->id == 2) {
	 printk("spi channel : %d\n", pdev->id);
	 test_spi(data->spi_ch);
	 }
	 */
	return 0;

	err_bl:

	err_spi: kfree(data);

	err_alloc: kfree(hs_spidev);

	err_allocdev: return ret;
}

static int hs_spi_io_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_PM
static int hs_spi_io_suspend(struct platform_device *pdev,
		pm_message_t state)
{
	return 0;
}

static int hs_spi_io_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define hs_spi_io_suspend	NULL
#define hs_spi_io_resume	NULL
#endif

static struct platform_driver hs_spi_io_driver = { .driver = {
	.name = "hs_spidev",
	.owner = THIS_MODULE,
}, .probe = hs_spi_io_probe, .remove = hs_spi_io_remove,
		.suspend = hs_spi_io_suspend, .resume = hs_spi_io_resume, };

static int __init hs_spi_io_init(void)
{
	return platform_driver_register(&hs_spi_io_driver);
}
module_init(hs_spi_io_init);

static void __exit hs_spi_io_exit(void)
{
	platform_driver_unregister(&hs_spi_io_driver);
}
module_exit(hs_spi_io_exit);

MODULE_DESCRIPTION("hs_spi_io based Test Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:hs_spi_io");

