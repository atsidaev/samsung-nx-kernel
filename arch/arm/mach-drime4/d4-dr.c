#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pinctrl/consumer.h>

#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/devs.h>
#include <linux/miscdevice.h>
#include <mach/d4-dr-ioctl.h>
#include <mach/gpio.h>

#define D4DR_IOCTL_DEV_NAME "d4_dr_ioctl"


int d4_dr_ioctl_open(struct inode* inode, struct file *filp)
{
	return 0;
}
static int d4_dr_ioctl_read(struct file *filep, char __user *buf, 
	size_t count, loff_t *f_pos)
{
	return 0;
}

int d4_dr_ioctl_release(struct inode *inode, struct file *filp)
{
	return 0;
}
long d4_dr_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size = 0;
	unsigned int value = 0;
	int ret = 0;

	if (_IOC_TYPE(cmd) != D4_DR_IOCTL_MAGIC)
		goto fail;

	size = _IOC_SIZE(cmd);

	switch (cmd) {
	case DR_INVERTER_SET:
		if (copy_from_user((void *) &value, (const void *) arg, size))
			goto fail;

		printk("DR_INVERTER_SET %d\n", value);
		gpio_set_value(GPIO_DRS_INV_EN, value); /*Set inverter HIGH*/
		break;
	}

	return ret;
fail:
	return ret;

}



const struct file_operations d4_dr_ioctl_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = d4_dr_ioctl,
	.open = d4_dr_ioctl_open,
	.read = d4_dr_ioctl_read,
	.release = d4_dr_ioctl_release,
};

static struct miscdevice drime4_dr_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = D4DR_IOCTL_DEV_NAME,
	.fops = &d4_dr_ioctl_fops,
};

/****************************************************/

static int  __init d4_dr_probe(struct platform_device *pdev)
{
	int ret;

	/* request gpio */
	pinctrl_request_gpio(GPIO_DRS_INV_EN);
	ret = gpio_request(GPIO_DRS_INV_EN, "inv_en");

	if (ret < 0) {
		printk("error0 %x\n", ret);
	}
	ret = gpio_direction_output(GPIO_DRS_INV_EN, 0);
	if (ret < 0) {
		printk("error1 %x\n", ret);
	}

	ret = misc_register(&drime4_dr_miscdev);
	if (ret < 0) {
		printk("error2 %x\n", ret);
	}

	return ret;
}

static int d4_dr_remove(struct platform_device *pdev)
{
	int ret;
	ret = misc_deregister(&drime4_dr_miscdev);

	return 0;
}

static int d4_dr_suspend(struct platform_device *dev, pm_message_t state)
{
	int ret;
	ret = misc_deregister(&drime4_dr_miscdev);

	return 0;
}

static int d4_dr_resume(struct platform_device *pdev)
{
	int ret;
	ret = misc_register(&drime4_dr_miscdev);

	return 0;
}


static struct platform_driver	d4_dr_ioctl_driver = {
	.probe		= d4_dr_probe,
	.remove		= d4_dr_remove,
	.suspend		= d4_dr_suspend,
	.resume		= d4_dr_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "d4-dr",
	},
};

int __init d4_dr_init(void)
{
	return platform_driver_register(&d4_dr_ioctl_driver);
}

void __exit d4_dr_exit(void)
{
	platform_driver_unregister(&d4_dr_ioctl_driver);
}

/*******************************************************/
struct platform_device d4_device_dr_ioctl = {
	.name		= "d4-dr",
	.id		= -1,
};

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(d4_dr_init);
#else
fast_dev_initcall(d4_dr_init);
#endif
module_exit(d4_dr_exit);
MODULE_LICENSE("Dual BS/GPL");

