/* linux/drivers/media/video/drime4/srp/d4_srp.c
 *
 * @file d4_ep.c
 * @brief DRIMe4 SRP driver
 * @author Geunjae Yu <geunjae.yu@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
/* #include <linux/mm.h> */
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/clk.h>

#include <linux/pinctrl/consumer.h>
#include <mach/srp/d4_srp.h>
#include "media/drime4/srp/d4_srp_ioctl.h"
#include "d4_srp_if.h"


extern const struct file_operations drime4_srp_ops;

static struct miscdevice srp_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = SRP_MODULE_NAME,
	.fops = &drime4_srp_ops,
};

struct d4_srp *g_srp;
static struct srp_reg_ctrl_base_info srp_info;
static struct resource *d4_srp_mem[4];

/******************************************************************************/
/*                        Private Function Implementation                     */
/******************************************************************************/

/* device management functions */


/**
 * @brief SRP device resource register
 * @fn srp_register_iomap(struct platform_device *pdev, struct d4_srp *srp, int res_num)
 * @param struct platform_device *pdev, struct d4_srp *srp, int res_num
 * @return int
 * @author Geunjae Yu <geunjae.yu@samsung.com>
 * @note  NONE
 */
static int d4_srp_register_iomap(struct platform_device *pdev,
		struct d4_srp *srp, int res_num)
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
	d4_srp_mem[res_num] = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!d4_srp_mem[res_num]) {
		dev_err(&pdev->dev, "failed to request io memory region\n");
		ret = -ENOMEM;
		goto out;
	}

	/* ioremap for register block */
	regs = ioremap(res->start, resource_size(res));
	if (!regs) {
		dev_err(&pdev->dev, "failed to remap io region\n");
		ret = -ENOMEM;
		goto ERROR_IOREMAP;
	}

	if (res_num == 0) {
		srp_info.srp_reg_base = (unsigned int)regs;
		srp_info.srp_reg_info.reg_start_addr = (unsigned int) res->start;
		srp_info.srp_reg_info.reg_size = (unsigned int) resource_size(res);
	} else if (res_num == 1) {
		srp_info.srp_dap_base = (unsigned int)regs;
		srp_info.srp_dap_info.reg_start_addr = (unsigned int) res->start;
		srp_info.srp_dap_info.reg_size = (unsigned int) resource_size(res);
	} else if (res_num == 2) {
		srp_info.srp_commbox_base = (unsigned int)regs;
		srp_info.srp_commbox_info.reg_start_addr = (unsigned int) res->start;
		srp_info.srp_commbox_info.reg_size = (unsigned int) resource_size(res);
	} else if (res_num == 3) {
		srp_info.srp_ssram_base = (unsigned int)regs;
		srp_info.srp_ssram_info.reg_start_addr = (unsigned int) res->start;
		srp_info.srp_ssram_info.reg_size = (unsigned int) resource_size(res);
	}

	return 0;

ERROR_IOREMAP:
	release_resource(d4_srp_mem[res_num]);
	kfree(d4_srp_mem[res_num]);

	out: return ret;
}


/**
 * @brief SRP device resource release
 * @fn srp_release_iomap(struct platform_device *pdev, struct d4_srp *srp, int res_num)
 * @param struct platform_device *pdev, struct d4_srp *srp, int res_num
 * @return int
 * @author Geunjae Yu <geunjae.yu@samsung.com>
 * @note  NONE
 */
static int d4_srp_release_iomap(struct platform_device *pdev,
		struct d4_srp *srp, int res_num)
{
	struct resource *res = NULL;
	int ret = 0;

	/* get resource for io memory  */
	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region\n");
		ret = -ENODEV;
		goto out;
	}

	/* release mem region  */
	release_mem_region(res->start, resource_size(res));
	/* dev_err(&pdev->dev, "release io memory region\n"); */

	out:
	return ret;
}

 /**
  * @brief SRP device probe
  * @fn srp_probe(struct platform_device *pdev)
  * @param struct platform_device *pdev
  * @return int
 * @author Geunjae Yu <geunjae.yu@samsung.com>
  * @note  NONE
  */
static int __devinit d4_srp_probe(struct platform_device *pdev)
{
	struct d4_srp *srp_ctx = NULL;
	int ret = 0;

	srp_ctx = kzalloc(sizeof(struct d4_srp), GFP_KERNEL);
	if (!srp_ctx) {
		dev_err(&pdev->dev, "failed to allocate d4_srp.\n");
		ret = -EINVAL;
		goto out;
	}

	srp_ctx->pd = (struct drime4_srp_dev_data *)pdev->dev.platform_data;
	if (srp_ctx->pd == NULL) {
		dev_err(&pdev->dev, "srp platform_data is empty.\n");
		ret = -ENOSPC;
		goto ERROR_PLATFORM_DATA;
	}

	srp_ctx->id = pdev->id;
	srp_ctx->dev = &pdev->dev;
	srp_ctx->name = pdev->name;

	/**< Mode Select CODEC->SRP */
#if 1	/* unblock after update pinmux fuction */
	/*d4_rmu_modecon_set(RMU_MODECON_CODEC_SRP,RMU_SEL_SRP);*/
	srp_ctx->pmx = devm_pinctrl_get(&pdev->dev);
	srp_ctx->pins_default = pinctrl_lookup_state(srp_ctx->pmx, PINCTRL_STATE_DEFAULT);
	/*modecon activate in srp init function */
	/*pinmux_enable(srp->pmx);*/
#endif

	/**< Clock Enable */
	srp_ctx->clock = clk_get(&pdev->dev, "srp");

	srp_info.dev = &pdev->dev;

	/* register iomap resources */
	ret = d4_srp_register_iomap(pdev, srp_ctx, 0); /* SRP Register Base */
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register iomap\n");
		goto ERROR_IOREMAP_0;
	}
	ret = d4_srp_register_iomap(pdev, srp_ctx, 1); /* SRP DAP Register Base */
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register iomap\n");
		goto ERROR_IOREMAP_1;
	}
	ret = d4_srp_register_iomap(pdev, srp_ctx, 2); /* SRP COMMBOX Register Base */
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register iomap\n");
		goto ERROR_IOREMAP_2;
	}
	ret = d4_srp_register_iomap(pdev, srp_ctx, 3); /* Shared sram Base */
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register iomap\n");
		goto ERROR_IOREMAP_3;
	}

	/* register irq resource */
	srp_info.irq_num = platform_get_irq(pdev, 0);
	if (srp_info.irq_num < 0) {
		dev_err(&pdev->dev, "No IRQ specified.\n");
		ret = -ENOENT;
		goto ERROR_MISC_REGISTER;
	}

	d4_srp_set_reg_ctrl_base_info(&srp_info);

	/* srp device registration */
	ret = misc_register(&srp_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		goto ERROR_MISC_REGISTER;
	}
	dev_dbg(&pdev->dev, "device registered as /dev/%s \n", SRP_MODULE_NAME);

	platform_set_drvdata(pdev, srp_ctx);

	srp_ctx->dev = &pdev->dev;
	g_srp = srp_ctx;

	dev_dbg(&pdev->dev, "probe success.\n");

	return 0;

ERROR_MISC_REGISTER:
	iounmap((void __iomem *)srp_info.srp_ssram_base);
	release_resource(d4_srp_mem[3]);
	kfree(d4_srp_mem[3]);
ERROR_IOREMAP_3:
	iounmap((void __iomem *)srp_info.srp_commbox_base);
	release_resource(d4_srp_mem[2]);
	kfree(d4_srp_mem[2]);
ERROR_IOREMAP_2:
	iounmap((void __iomem *)srp_info.srp_dap_base);
	release_resource(d4_srp_mem[1]);
	kfree(d4_srp_mem[1]);
ERROR_IOREMAP_1:
	iounmap((void __iomem *)srp_info.srp_reg_base);
	release_resource(d4_srp_mem[0]);
	kfree(d4_srp_mem[0]);
ERROR_IOREMAP_0:
ERROR_PLATFORM_DATA:
	kfree(srp_ctx);

out:
	return ret;
}


/**
 * @brief SRP device remove
 * @fn srp_remove(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Geunjae Yu <geunjae.yu@samsung.com>
 * @note  NONE
 */
static int d4_srp_remove(struct platform_device *pdev)
{
	misc_deregister(&srp_miscdev);

	iounmap((void __iomem *)srp_info.srp_reg_base);
	iounmap((void __iomem *)srp_info.srp_dap_base);
	iounmap((void __iomem *)srp_info.srp_commbox_base);
	iounmap((void __iomem *)srp_info.srp_ssram_base);

	release_resource(d4_srp_mem[0]);
	release_resource(d4_srp_mem[1]);
	release_resource(d4_srp_mem[2]);
	release_resource(d4_srp_mem[3]);

	kfree(d4_srp_mem[0]);
	kfree(d4_srp_mem[1]);
	kfree(d4_srp_mem[2]);
	kfree(d4_srp_mem[3]);

	dev_dbg(&pdev->dev, "remove success.\n");

	/** clock disable */
	clk_disable(g_srp->clock);
	clk_put(g_srp->clock);

	kfree(g_srp);
	return 0;
}

/**
 * @brief SRP device suspend
 * @fn srp_suspend(struct platform_device *pdev, pm_message_t state)
 * @param struct platform_device *pdev, pm_message_t state
 * @return int
 * @author Geunjae Yu <geunjae.yu@samsung.com>
 * @note  NONE
 */
static int d4_srp_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret = 0;
	misc_deregister(&srp_miscdev);

	iounmap((void __iomem *)srp_info.srp_reg_base);
	iounmap((void __iomem *)srp_info.srp_dap_base);
	iounmap((void __iomem *)srp_info.srp_commbox_base);
	iounmap((void __iomem *)srp_info.srp_ssram_base);

	ret = d4_srp_release_iomap(pdev, g_srp, 0);
	if (ret < 0)
		goto out;

	ret = d4_srp_release_iomap(pdev, g_srp, 1);
	if (ret < 0)
		goto out;

	ret = d4_srp_release_iomap(pdev, g_srp, 2);
	if (ret < 0)
		goto out;

	ret = d4_srp_release_iomap(pdev, g_srp, 3);
	if (ret < 0)
		goto out;

	dev_dbg(&pdev->dev, "suspend success.\n");
	kfree(g_srp);
	return ret;

out:
	dev_err(&pdev->dev, "srp_release_iomap fail : %d\n", ret);
	kfree(g_srp);
	return ret;
}


 /**
  * @brief SRP device resume
  * @fn srp_resume(struct platform_device *pdev)
  * @param struct platform_device *pdev
  * @return int
  * @author Geunjae Yu <geunjae.yu@samsung.com>
  * @note  NONE
  */
static int d4_srp_resume(struct platform_device *pdev)
{
	struct d4_srp *srp_ctx = NULL;
	int ret = 0;

	srp_ctx = kzalloc(sizeof(struct d4_srp), GFP_KERNEL);
	if (!srp_ctx) {
		dev_err(&pdev->dev, "failed to allocate srp.\n");
		ret = -EINVAL;
		goto out;
	}

	srp_ctx->pd = (struct drime4_srp_dev_data *)pdev->dev.platform_data;
	if (srp_ctx->pd == NULL) {
		dev_err(&pdev->dev, "srp platform_data is empty.\n");
		ret = -ENOSPC;
		goto ERR_PLATFORM_DATA;
	}

	srp_ctx->id = pdev->id;
	srp_ctx->dev = &pdev->dev;
	srp_ctx->name = pdev->name;

	srp_info.dev = &pdev->dev;

	/* register iomap resources */
	ret = d4_srp_register_iomap(pdev, srp_ctx, 0); /* SRP Register Base */
	if (ret < 0)
		goto ERR_IOREMAP_0;

	ret = d4_srp_register_iomap(pdev, srp_ctx, 1); /* SRP DAP Register Base */
	if (ret < 0)
		goto ERR_IOREMAP_1;

	ret = d4_srp_register_iomap(pdev, srp_ctx, 2); /* SRP COMMBOX Register Base */
	if (ret < 0)
		goto ERR_IOREMAP_2;

	ret = d4_srp_register_iomap(pdev, srp_ctx, 3); /* Shared sram Base */
	if (ret < 0)
		goto ERR_IOREMAP_3;

	/* register irq resource */
	srp_info.irq_num = platform_get_irq(pdev, 0);

	/**< SRP Core Base Register Address & IRQ Number Setting */
	d4_srp_set_reg_ctrl_base_info(&srp_info);

	/* srp device registration */
	ret = misc_register(&srp_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		goto ERR_MISC_REGISTER;
	}
	dev_err(&pdev->dev, "device registered as /dev/%s \n", SRP_MODULE_NAME);

	platform_set_drvdata(pdev, srp_ctx);

	srp_ctx->dev = &pdev->dev;
	g_srp = srp_ctx;

	dev_dbg(&pdev->dev, "resume success.\n");

	return 0;

ERR_MISC_REGISTER:
	iounmap((void __iomem *)srp_info.srp_ssram_base);
	release_resource(d4_srp_mem[3]);
	kfree(d4_srp_mem[3]);
ERR_IOREMAP_3:
	iounmap((void __iomem *)srp_info.srp_commbox_base);
	release_resource(d4_srp_mem[2]);
	kfree(d4_srp_mem[2]);
ERR_IOREMAP_2:
	iounmap((void __iomem *)srp_info.srp_dap_base);
	release_resource(d4_srp_mem[1]);
	kfree(d4_srp_mem[1]);
ERR_IOREMAP_1:
	iounmap((void __iomem *)srp_info.srp_reg_base);
	release_resource(d4_srp_mem[0]);
	kfree(d4_srp_mem[0]);
ERR_IOREMAP_0:
ERR_PLATFORM_DATA:
	kfree(srp_ctx);

out:
	return ret;
}


/* driver device registration */

static struct platform_driver d4_srp_driver = {
	.probe		= d4_srp_probe,
	.remove		= d4_srp_remove,
	.suspend	= d4_srp_suspend,
	.resume		= d4_srp_resume,
	.driver		= {
		.name	= SRP_MODULE_NAME,
		.owner	= THIS_MODULE,
		},
};


/**
 * @brief SRP device register
 * @fn d4_srp_register(void)
 * @param void
 * @return int
 * @author Geunjae Yu <geunjae.yu@samsung.com>
 * @note  NONE
 */
static int d4_srp_register(void)
{
	platform_driver_register(&d4_srp_driver);
	return 0;
}

/**
 * @brief SRP device unregister
 * @fn d4_srp_unregister(void)
 * @param void
 * @return int
 * @author Geunjae Yu <geunjae.yu@samsung.com>
 * @note  NONE
 */
static void d4_srp_unregister(void)
{
	platform_driver_unregister(&d4_srp_driver);
}

module_init(d4_srp_register);
module_exit(d4_srp_unregister);

MODULE_AUTHOR("Geunjae Yu <geunjae.yu@samsung.com>");
MODULE_DESCRIPTION("DRIMe4 SRP driver");
MODULE_LICENSE("GPL");

