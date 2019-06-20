#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/poll.h>

unsigned long hdmi_detect_changed;
wait_queue_head_t hdmi_hpd_wait;

static int hdmi_hpd_open(struct inode *inode, struct file *filp)
{
	hdmi_detect_changed = 0;
	return 0;
}

static unsigned int hdmi_hpd_poll(struct file *filp, struct poll_table_struct *wait)
{
	unsigned int mask = 0;

	poll_wait(filp, &hdmi_hpd_wait, wait);

	if (hdmi_detect_changed) {
		printk("hdmi_detect_changed %d\n", hdmi_detect_changed);

		mask |= POLLIN | POLLRDNORM;
	}

	return mask;
}

static int hdmi_hpd_read(struct file *filep, char __user * buf, size_t count,
								loff_t *f_pos)
{
	if (copy_to_user(buf, &hdmi_detect_changed, sizeof(hdmi_detect_changed)) != 0) {
		return -EIO;
	}

	hdmi_detect_changed = 0;	
	return 4;
}

static int hdmi_hpd_release(struct inode *inode, struct file *filp)
{
	return 0;
}

const struct file_operations hdmi_hpd_fops = 
{
	.owner			= THIS_MODULE,
	.open			= hdmi_hpd_open,
	.read			= hdmi_hpd_read,
	.poll			= hdmi_hpd_poll,
	.release		= hdmi_hpd_release,
};


static struct miscdevice hdmi_hpd_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "hdmi_hpd",
	.fops = &hdmi_hpd_fops,
};

static __init int hdmi_hpd_init(void)
{
	int ret = 0;
	ret = misc_register(&hdmi_hpd_miscdev);

	init_waitqueue_head(&hdmi_hpd_wait);
	
	if (ret != 0)
		return -1;
	
	return 0;
}

static __exit void hdmi_hpd_exit(void)
{
	misc_deregister(&hdmi_hpd_miscdev);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(hdmi_hpd_init);
#else
fast_dev_initcall(hdmi_hpd_init);
#endif
module_exit(hdmi_hpd_exit);

MODULE_AUTHOR("Young Rae Jo <youngrae0.cho@samsung.com>");
MODULE_DESCRIPTION("hdmi hpd driver");
MODULE_LICENSE("GPL");

