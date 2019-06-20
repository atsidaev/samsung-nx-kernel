/*
 *
 * adc_ioctl.c
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
#include <mach/adc.h>
#include <mach/adc_ioctl.h>

#define HS_ADC_MODULE_NAME		"adcdev"


struct adc_data {
	dev_t devt;
	struct drime4_adc_client *adc;
	struct device *dev;
	struct list_head device_entry;
};


struct adc_data *adc_data;


static long adc_io_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	unsigned int size;
	int ret;
	struct adc_value value;
	struct adc_data *data = (struct adc_data *) filp->private_data;
	size = _IOC_SIZE(cmd);

	value.read_value = 0;
	value.channel = 0;

	switch (cmd) {
		case ADC_IOCTL_GET_VALUE:

			ret = copy_from_user((void *) &value, (const void *) arg, size);
			if (ret < 0) {
				printk("ioctl fail: [%d]", cmd);
				return ret;
			}
			value.read_value = drime4_adc_raw_read(data->adc, value.channel);
			copy_to_user((void *) arg, (const void *) &value, size);
			break;

		default:
			break;
	}
	return 0;
}

static int adc_io_open(struct inode *inode, struct file *filp)
{
	filp->private_data = adc_data;
	return 0;

}

static int adc_io_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t adc_io_read(struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos)
{
	return 0;
}

/* Write-only message with current device setup */
static ssize_t adc_io_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	return 0;
}



const struct file_operations adc_io_ops = { .open = adc_io_open,
		.release = adc_io_release, .read = adc_io_read,
		.write = adc_io_write, .unlocked_ioctl = adc_io_ioctl
		 };

static void adcdev_set(struct miscdevice *spidev)
{
	spidev->name = "adcdev";
	spidev->minor = MISC_DYNAMIC_MINOR;
	spidev->fops = &adc_io_ops;
}

static int adc_io_probe(struct platform_device *pdev)
{
	struct adc_data *data;
	struct miscdevice *adcdev;
	int ret = 0;

	adcdev = kzalloc(sizeof(*adcdev), GFP_KERNEL);

	if (!adcdev) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_allocdev;
	}

	adcdev_set(adcdev);

	data = kzalloc(sizeof(*data), GFP_KERNEL);

	if (!data) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	data->dev = &pdev->dev;

	data->adc = drime4_adc_register(pdev, 10, 10, 10);
	if (data->adc == NULL) {
		dev_err(&pdev->dev, "failed to register ioctl malloc\n");
		ret = -ENOMEM;
		goto err_spi;
	} else if (IS_ERR(data->adc)) {
		dev_err(&pdev->dev, "failed to register ioctl setting\n");
		ret = -ENOMEM;
		goto err_spi;
	} else {
		dev_err(&pdev->dev, "success register\n");
	}

	ret = misc_register(adcdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register misc io ctrl\n");
		goto err_bl;
	}
	data->devt = MKDEV(10, adcdev->minor);
	platform_set_drvdata(pdev, data);
	adc_data = data;
	return 0;

	err_bl:drime4_adc_release(data->adc);

	err_spi: kfree(data);

	err_alloc: kfree(adcdev);

	err_allocdev: return ret;
}

static int adc_io_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_PM
static int adc_io_suspend(struct platform_device *pdev,
		pm_message_t state)
{
	return 0;
}

static int adc_io_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define adc_io_suspend	NULL
#define adc_io_resume	NULL
#endif

static struct platform_driver adc_io_driver = { .driver = {
	.name = "adcdev",
	.owner = THIS_MODULE,
}, .probe = adc_io_probe, .remove = adc_io_remove,
		.suspend = adc_io_suspend, .resume = adc_io_resume, };

static int __init adc_io_init(void)
{
	return platform_driver_register(&adc_io_driver);
}
module_init(adc_io_init);

static void __exit adc_io_exit(void)
{
	platform_driver_unregister(&adc_io_driver);
}
module_exit(adc_io_exit);

MODULE_DESCRIPTION("hs_spi_io based Test Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:hs_spi_io");

