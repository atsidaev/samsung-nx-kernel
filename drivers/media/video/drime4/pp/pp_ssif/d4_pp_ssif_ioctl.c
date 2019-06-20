/**
 * @file d4_pp_ssif_ioctl.c
 * @brief DRIMe4 PP Sensor Interface Ioctl Control Function File
 * @author Main : DeokEun Cho <de.cho@samsung.com>
 *         MIPI : Gunwoo Nam <gunwoo.nam@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include "d4_pp_ssif_if.h"
#include <media/drime4/pp/pp_ssif/d4_pp_ssif_ioctl.h>
#include <mach/pp/pp_ssif/d4_pp_ssif.h>

extern struct drime4_pp_ssif *g_pp_ssif;

/**< For sensor fps checking */
static int curr_fps_count = -1;
static int prev_fps_count = -1;
struct timer_list fps_check_timer;

struct pp_ssif_io_data {
	dev_t devt;
	struct device *dev;
	struct miscdevice *ssif_iodev;
	struct clk 		*sensor_clock;
  struct clk    *tg_clk;
	struct clk 		*sg_clock;
	struct clk 		*s0_clock;
	struct clk 		*s1_clock;
	struct clk 		*s2_clock;
	struct clk 		*s3_clock;
	struct clk 		*clock_set;
};


/**< Interrupt */
void pp_ssif_int_wait_callback_func(int int_sel);
void pp_ssif_fps_check_callback_func(int set_value);

#ifdef PP_SSIF_POLL_ENABLE

static DECLARE_WAIT_QUEUE_HEAD(PP_SSIF_WaitQueue_Read);
#define MAX_PP_SSIF_QUEUE_CNT 512

static char PP_SSIF_ReadQ[MAX_PP_SSIF_QUEUE_CNT];
static unsigned long PP_SSIF_ReadQCount;
static unsigned long PP_SSIF_ReadQHead;
static unsigned long PP_SSIF_ReadQTail;

void pp_ssif_send_interrupt(int intr_type)
{
	unsigned long flags;

	local_save_flags(flags);
	local_irq_disable();

	if (PP_SSIF_ReadQCount < MAX_PP_SSIF_QUEUE_CNT) {
		PP_SSIF_ReadQ[PP_SSIF_ReadQHead] = intr_type;
		PP_SSIF_ReadQHead = (PP_SSIF_ReadQHead + 1) % MAX_PP_SSIF_QUEUE_CNT;
		PP_SSIF_ReadQCount++;
	}
	local_irq_restore(flags);

	wake_up_interruptible(&PP_SSIF_WaitQueue_Read);
}
#endif

static void fps_check_timer_irq(unsigned long data)
{
	unsigned int live_fps = 0;

	live_fps = curr_fps_count - prev_fps_count;

	fps_check_timer.expires = jiffies + HZ;
	add_timer(&fps_check_timer);
	pr_info("Current Sensor fps :[ %d fps ]\n", live_fps);

	prev_fps_count = curr_fps_count;
}

int pp_ssif_open(struct inode *inode, struct file *filp)
{
	/* int num = MINOR(inode->i_rdev);*/
	struct drime4_pp_ssif *pp_ssif_ctx = g_pp_ssif;
	filp->private_data = pp_ssif_ctx;
	return 0;
}

static int pp_ssif_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t pp_ssif_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	unsigned int i = 0;
	return i;
}

ssize_t pp_ssif_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	unsigned int i = 0;
	return i;
}

long pp_ssif_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size = 0 ;
	int err  = -1;
	long ret = -1;
	unsigned int sensor_clock_set = 0;
	unsigned int set_delay = 0;
	enum ssif_dd_interrupt_selection int_select;

	if (_IOC_TYPE(cmd) != PP_SSIF_MAGIC)
		return -1;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *) arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *) arg, size);

	if (!err)
		return -1;

	switch (cmd) {
	case PP_SSIF_TEST:
		break;
	case PP_SSIF_IOCTL_OPEN_IRQ:
		pp_ssif_com_request_irq();
		break;
	case PP_SSIF_IOCTL_CLOSE_IRQ:
		pp_ssif_com_free_irq();
		break;
	case PP_SSIF_IOCTL_WAIT_INT:
		ret = copy_from_user((void *)&int_select, (const void *)arg, size);
		if (ret < 0) {
			pr_err("ioctl fail: [%d]", cmd);
			return ret;
		}
		pp_ssif_set_callback_func(int_select, pp_ssif_int_wait_callback_func);
		wait_for_completion(&pp_ssif_int_completion[int_select]);
		break;
	case PP_SSIF_IOCTL_SET_SENSOR_CLOCK:
		ret = copy_from_user((void *)&sensor_clock_set, (const void *)arg, size);
		if (ret < 0) {
			pr_err("ioctl fail: [%d]", cmd);
			return ret;
		}
		pp_ssif_k_dd_set_sensor_clock(sensor_clock_set);
		break;
	case PP_SSIF_IOCTL_CHECK_FPS:
		pr_debug("SSIF VSYNC Interrupt Callback Registration\n");
		pp_ssif_set_callback_func(SSIF_DD_INT_USER_DEFINE_6, pp_ssif_fps_check_callback_func);

		pr_debug("SSIF VSYNC Interrupt Enable\n");
		k_pp_ssif_set_interrupt_enable(SSIF_DD_INT_USER_DEFINE_6, K_SSIF_ON);

		/* timer for fps check */
		init_timer(&fps_check_timer);
		fps_check_timer.expires = jiffies + HZ;
		fps_check_timer.data = 0;
		fps_check_timer.function = &fps_check_timer_irq;

		add_timer(&fps_check_timer);
		break;
	case PP_SSIF_IOCTL_SET_DELAY:
		ret = copy_from_user((void *)&set_delay, (const void *)arg, size);
		if (ret < 0) {
			pr_err("ioctl fail: [%d]", cmd);
			return ret;
		}

		mdelay(set_delay);
		break;
	default:
		break;
	}
	return 0;
}

int pp_ssif_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size = 0;

	size     = (vma->vm_end - vma->vm_start);

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
			vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

#ifdef PP_SSIF_POLL_ENABLE
static u32 pp_ssif_poll_wait(struct file *filp, poll_table *wait)

{
	unsigned int mask = 0;

	poll_wait(filp, &PP_SSIF_WaitQueue_Read, wait);

	if (PP_SSIF_ReadQCount > 0) {
		mask |= POLLIN | POLLRDNORM;
	}

	return mask;
}

ssize_t pp_ssif_poll_read(struct file *filp, char __user *num, size_t count, loff_t *f_pos)
{
	unsigned long flags;
	int retstate;

	if ((!PP_SSIF_ReadQCount) && (filp->f_flags & O_NONBLOCK))
		return -EAGAIN;

	retstate = wait_event_interruptible(PP_SSIF_WaitQueue_Read, PP_SSIF_ReadQCount);

	if (retstate)
		return retstate;

	local_save_flags(flags);
	local_irq_disable();

	if (PP_SSIF_ReadQCount > 0) {
		put_user(PP_SSIF_ReadQ[PP_SSIF_ReadQTail], num);
		PP_SSIF_ReadQTail = (PP_SSIF_ReadQTail + 1) % MAX_PP_SSIF_QUEUE_CNT;
		PP_SSIF_ReadQCount--;
	}

	local_irq_restore(flags);

	return PP_SSIF_ReadQCount;
}

const struct file_operations pp_ssif_ops = {
	.open				= pp_ssif_open,
	.release			= pp_ssif_release,
	.write				= pp_ssif_write,
	.mmap 				= pp_ssif_mmap,
	.unlocked_ioctl		= pp_ssif_ioctl,
	.poll 				= pp_ssif_poll_wait,
	.read 				= pp_ssif_poll_read,
};
#else
const struct file_operations pp_ssif_ops = {
	.open				= pp_ssif_open,
	.release			= pp_ssif_release,
	.read				= pp_ssif_read,
	.write				= pp_ssif_write,
	.mmap 				= pp_ssif_mmap,
	.unlocked_ioctl		= pp_ssif_ioctl,
};
#endif

void pp_ssif_int_wait_callback_func(int int_sel)
{
	/**< printk("int: %d \n", int_sel); */
	complete(&pp_ssif_int_completion[int_sel]);

#ifndef PP_SSIF_POLL_ENABLE
	pp_ssif_set_callback_func(int_sel, NULL);
#endif
}

void pp_ssif_fps_check_callback_func(int set_value)
{
	curr_fps_count++;
}

static void pp_ssif_iodev_set(struct miscdevice *spidev, const char *fmt, ...)
{

	va_list vargs;
	va_start(vargs, fmt);
	spidev->name = kvasprintf(GFP_KERNEL, fmt, vargs);
	spidev->minor = MISC_DYNAMIC_MINOR;
	spidev->fops = &pp_ssif_ops;

}


static int __devinit pp_ssif_ioctl_probe(struct platform_device *pdev)
{
	struct pp_ssif_io_data *data;
	struct miscdevice *pp_ssif_iodev;
	struct drimet_pp_ssif_ext_tg_data *pltdata = pdev->dev.platform_data;
	int ret = 0;
	pp_ssif_iodev = kzalloc(sizeof(*pp_ssif_iodev), GFP_KERNEL);

	if (!pp_ssif_iodev) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_allocdev;
	}

	pp_ssif_iodev_set(pp_ssif_iodev, "d4_pp_ssif");

	data = kzalloc(sizeof(*data), GFP_KERNEL);

	if (!data) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	data->dev = &pdev->dev;

	/**< PP SSIF Clock Enable */
	data->sensor_clock = clk_get(&pdev->dev, "ssif_sensor");
	if (data->sensor_clock == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_enable(data->sensor_clock);

	data->sg_clock = clk_get(&pdev->dev, "ssif_sg");
	if (data->sg_clock == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_enable(data->sg_clock);

	data->s0_clock = clk_get(&pdev->dev, "ssif_s0");
	if (data->s0_clock == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_enable(data->s0_clock);


	data->s1_clock = clk_get(&pdev->dev, "ssif_s1");
	if (data->s1_clock == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_enable(data->s1_clock);

	data->s2_clock = clk_get(&pdev->dev, "ssif_s2");
	if (data->s2_clock == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_enable(data->s2_clock);

	data->s3_clock = clk_get(&pdev->dev, "ssif_s3");
	if (data->s3_clock == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_enable(data->s3_clock);

	data->tg_clk = clk_get(NULL, "ext_tg");
	if (data->tg_clk == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_set_rate(data->tg_clk, pltdata->ext_tg_clk);

	data->tg_clk = clk_get(NULL, pltdata->sg_cksel);
	if (data->tg_clk == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_set_parent(data->sg_clock, data->tg_clk);

	data->tg_clk = clk_get(NULL, pltdata->s0_cksel);
	if (data->tg_clk == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_set_parent(data->s0_clock, data->tg_clk);

	data->tg_clk = clk_get(NULL, pltdata->s1_cksel);
	if (data->tg_clk == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_set_parent(data->s1_clock, data->tg_clk);

	data->tg_clk = clk_get(NULL, pltdata->s2_cksel);
	if (data->tg_clk == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_set_parent(data->s2_clock, data->tg_clk);

	data->tg_clk = clk_get(NULL, pltdata->s3_cksel);
	if (data->tg_clk == ERR_PTR(-ENOENT)) {
		ret = -EINVAL;
		goto err_bl;
	}
	clk_set_parent(data->s3_clock, data->tg_clk);

	ret = misc_register(pp_ssif_iodev);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register ssif io ctrl\n");
		goto err_bl;
	}
	data->ssif_iodev = pp_ssif_iodev;
	data->devt = MKDEV(10, pp_ssif_iodev->minor);
	platform_set_drvdata(pdev, data);
	return 0;
err_bl: kfree(data);

err_alloc: kfree(pp_ssif_iodev);

err_allocdev: return ret;
}

static int pp_ssif_io_remove(struct platform_device *pdev)
{
	struct pp_ssif_io_data *data = platform_get_drvdata(pdev);

	misc_deregister(data->ssif_iodev);
	/**< PP SSIF Clock Disable */
	clk_disable(data->sensor_clock);
	clk_put(data->sensor_clock);

	clk_disable(data->sg_clock);
	clk_put(data->sg_clock);

	clk_disable(data->s0_clock);
	clk_put(data->s0_clock);

	clk_disable(data->s1_clock);
	clk_put(data->s1_clock);

	clk_disable(data->s2_clock);
	clk_put(data->s2_clock);

	clk_disable(data->s3_clock);
	clk_put(data->s3_clock);

	kfree(data);
	return 0;
}

#define pp_ssif_io_suspend	NULL
#define pp_ssif_io_resume	NULL

static struct platform_driver pp_ssif_io_driver = { .driver = {
	.name = "pp_ssif_io",
	.owner = THIS_MODULE,
}, .probe = pp_ssif_ioctl_probe, .remove = pp_ssif_io_remove,
		.suspend = pp_ssif_io_suspend, .resume = pp_ssif_io_resume, };

static int __init pp_ssif_io_init(void)
{
	return platform_driver_register(&pp_ssif_io_driver);
}
#ifndef CONFIG_SCORE_FAST_RESUME
module_init(pp_ssif_io_init);
#else
fast_dev_initcall(pp_ssif_io_init);
#endif

static void __exit pp_ssif_io_exit(void)
{
	platform_driver_unregister(&pp_ssif_io_driver);
}
module_exit(pp_ssif_io_exit);


MODULE_AUTHOR("DeokEun Cho <de.cho@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 PP Sensor Interface using ioctl");
MODULE_LICENSE("GPL");
