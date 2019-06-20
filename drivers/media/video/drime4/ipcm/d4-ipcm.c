/* linux/drivers/media/video/drime4/drime4-ipc.c
 *
 * V4L2 based Samsung Drime IV IPCM driver.
 *
 * Jangwon Lee <jang_won.lee@samsung.com>
 *
 * Note: This driver supports common i2c client driver style
 * which uses i2c_board_info for backward compatibility and
 * new v4l2_subdev as well.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

/*
#define DEBUG
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/mm.h>
#include <linux/cma.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <media/v4l2-device.h>

#include "mach/ipcm/d4-ipcm.h"
#include "media/drime4/ipcm/d4_ipcm_v4l2.h"
#include "d4_ipcm_regs.h"
#include "d4_ipcm_if.h"
#include "d4_ipcm_framework.h"

struct drime4_ipcm *g_ipcm;
struct ipcm_ctx *g_ipcm_ctx;
struct ipcm_reg_ctrl_base_info ipcm_base_info;

static void drime4_ipcm_vdev_release(struct video_device *vdev)
{
	kfree(vdev);
}

static struct video_device drime4_ipcm_video_device[IPCM_MAX_VIDEO_DEVICE] = {
	[0] = {
		.fops = &drime4_ipcm_fops,
		.ioctl_ops = &drime4_ipcm_v4l2_ops,
		.release  = drime4_ipcm_vdev_release,
	},
};

static irqreturn_t drime4_ipcm_sys_irq(int irq, void *dev_id)
{
	struct drime4_ipcm *ipcm = (struct drime4_ipcm *)dev_id;

	WARN_ON(ipcm == NULL);

	d4_ipcm_sys_int_handler();

	return IRQ_HANDLED;
}

static irqreturn_t drime4_ipcm_md_irq(int irq, void *dev_id)
{
	struct drime4_ipcm *ipcm = (struct drime4_ipcm *)dev_id;

	WARN_ON(ipcm == NULL);

	d4_ipcm_md_int_handler();

	return IRQ_HANDLED;
}

static int ipcm_register_iomap(struct platform_device *pdev,
		struct drime4_ipcm *ipcm, int res_num)
{
	struct resource *res = NULL;
	void __iomem *regs;
	int ret = 0;

	/* get resource for io memory  */
	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region\n");
		ret = -ENODEV;
		goto out;
	}

	/* request mem region  */
	res = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!res) {
		dev_err(&pdev->dev, "failed to request io memory region\n");
		ret = -ENOMEM;
		goto out;
	}

	/* ioremap for register block */
	regs = ioremap(res->start, resource_size(res));
	if (!regs) {
		dev_err(&pdev->dev, "failed to remap io region\n");

		release_mem_region(res->start, resource_size(res));
		ret = -ENOMEM;
		goto out;
	}

	ipcm_base_info.top_reg_base = (unsigned int)regs;

out:
	kfree(res);
	return ret;
}

static int __devinit ipcm_probe(struct platform_device *pdev)
{
	struct drime4_ipcm *ipcm = NULL;
	int ipcm_sys_irq_num;
	int ipcm_md_irq_num;
	int ret = 0;
	int id = 0;
	unsigned int i = 0;

	ipcm = kzalloc(sizeof(struct drime4_ipcm), GFP_KERNEL);
	if (!ipcm) {
		dev_err(&pdev->dev, "failed to allocate drime4_ipcm.\n");
		return -EINVAL;
	}
	g_ipcm = ipcm;

	ipcm->id = pdev->id;
	ipcm->dev = &pdev->dev;
	ipcm->name = pdev->name;

	/* Get IPCM clock  */
	ipcm->clock = clk_get(&pdev->dev, "ipcm");
	if (IS_ERR(ipcm->clock)) {
		dev_err(&pdev->dev, "failed to get ipcm clock source.\n");
		return -EFAULT;
	}

	spin_lock_init(&ipcm->irqlock);

	ipcm_base_info.dev_info = &pdev->dev;

	/* register iomap resources */
	ret = ipcm_register_iomap(pdev, ipcm, 0); /* get ipcm register base */
	if (ret < 0)
		goto out;

	ipcm_sys_irq_num = platform_get_irq(pdev, 0);
	ipcm_base_info.irq_num = ipcm_sys_irq_num;

	/* requset irq */
	if (request_irq(ipcm_sys_irq_num, drime4_ipcm_sys_irq, IRQF_DISABLED, IPCM_MODULE_NAME, ipcm)) {
		dev_err(&pdev->dev, "failed to request_irq failed\n");
		ret = -ENODEV;
		goto out;
	}

	ipcm_md_irq_num = platform_get_irq(pdev, 1);
	ipcm_base_info.md_irq_num = ipcm_md_irq_num;

	/* requset irq */
	if (request_irq(ipcm_md_irq_num, drime4_ipcm_md_irq, IRQF_DISABLED, IPCM_MODULE_NAME, ipcm)) {
		dev_err(&pdev->dev, "failed to request_irq failed\n");
		ret = -ENODEV;
		goto out;
	}

	/* set ipcm device info to global structure for internal functions. */
	d4_ipcm_set_dev_info(&ipcm_base_info);

	/* init completion to handle interrupt*/
	init_completion(&ipcm->wdma0_completion);
	init_completion(&ipcm->wdma1_completion);
	init_completion(&ipcm->wdma2_completion);

	/* V4L2 device-subdev registration */
	ret = v4l2_device_register(&pdev->dev, &ipcm->v4l2_dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "v4l2 device register failed\n");
		ret = -ENODEV;
		goto err_irq;
	}
	for (i = 0; i < IPCM_MAX_VIDEO_DEVICE; i++) {
		ipcm->vfd[i] = &drime4_ipcm_video_device[i];

		snprintf(ipcm->vfd[i]->name, sizeof(ipcm->vfd[i]->name), "%s%d",
			IPCM_MODULE_NAME, ipcm->id);

		/* register video devices. */
		ret = video_register_device(ipcm->vfd[i], VFL_TYPE_GRABBER, id);
		if (ret < 0) {
			dev_err(&pdev->dev, "cannot register video driver\n");
			video_device_release(ipcm->vfd[i]);
			goto err_irq;
		}

		video_set_drvdata(ipcm->vfd[i], ipcm);
		v4l2_info(&ipcm->v4l2_dev, "device registered as /dev/video%d\n",
			ipcm->vfd[i]->num);

	}

	platform_set_drvdata(pdev, ipcm);

	dev_dbg(&pdev->dev, "ipcm: probe success.\n");

	return 0;

err_irq:
	free_irq(ipcm->irq, ipcm);

out:
	return ret;
}

static int ipcm_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct drime4_ipcm *ipcm = platform_get_drvdata(pdev);
	struct ipcm_ctx *ctx = NULL;
	int ret = 0;

	ctx = ipcm->ipcm_ctx;

	/* sleep IPCM */
	ret = drime4_ipcm_fw_sleep(ipcm, ctx);

	return ret;
}

static int ipcm_resume(struct platform_device *pdev)
{
	struct drime4_ipcm *ipcm = platform_get_drvdata(pdev);
	struct ipcm_ctx *ctx = NULL;
	int ret = 0;

	ctx = ipcm->ipcm_ctx;

	/* wake up IPCM */
	ret = drime4_ipcm_fw_wakeup(ipcm, ctx);

	return ret;
}

static int ipcm_remove(struct platform_device *pdev)
{
	struct resource *res = NULL;
	struct drime4_ipcm *ipcm = platform_get_drvdata(pdev);

	iounmap(ipcm->reg_base);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res)
		release_mem_region(res->start, resource_size(res));

	/* Put IPCM clock  */
	clk_put(ipcm->clock);

	kfree(ipcm);
	return 0;
}

static struct platform_driver ipcm_driver = {
	.probe = ipcm_probe,
	.remove = ipcm_remove,
	.suspend = ipcm_suspend,
	.resume = ipcm_resume,
	.driver = {
		 .name = IPCM_MODULE_NAME,
		 .owner = THIS_MODULE,
	},
};

static int ipcm_register(void)
{
	platform_driver_register(&ipcm_driver);
	return 0;
}

static void ipcm_unregister(void)
{
	platform_driver_unregister(&ipcm_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(ipcm_register);
#else
fast_dev_initcall(ipcm_register);
#endif
module_exit(ipcm_unregister);

MODULE_AUTHOR("Jangwon Lee <jang_won.lee@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 IPCM driver using V4L2");
MODULE_LICENSE("GPL");

