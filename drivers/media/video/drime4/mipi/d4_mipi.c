/**
 * @file d4_mipi.c
 * @brief DRIMe4 MIPI Driver File
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
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
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <mach/mipi/d4_mipi.h>
#include "d4_mipi_ctrl_dd.h"

extern const struct file_operations mipi_ops;

static struct miscdevice mipi_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = MIPI_MODULE_NAME,
		.fops = &mipi_ops,
};

struct drime4_mipi *g_mipi;


/**< MIPI Base Register Address & IRQ Number Setting */
static struct mipi_reg_ctrl_base_info mipi_base_info;
static struct resource *d4_mipi_mem[3];

/**
 * @brief MIPI device resource register
 * @fn d4_mipi_register_iomap(struct platform_device *pdev, struct drime4_mipi *mipi_ctx, int res_num)
 * @param struct platform_device *pdev, struct drime4_mipi *mipi_ctx, int res_num
 * @return int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
static int d4_mipi_register_iomap(struct platform_device *pdev,
		struct drime4_mipi *mipi_ctx, int res_num)
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
	d4_mipi_mem[res_num] = request_mem_region(res->start,
				resource_size(res), pdev->name);
	if (!d4_mipi_mem[res_num]) {
		dev_err(&pdev->dev, "failed to request io memory region\n");
		ret = -ENOMEM;
		goto out;
	}

	/* ioremap for register block */
	regs = ioremap(res->start, resource_size(res));
	if (!regs) {
		dev_err(&pdev->dev, "failed to remap io region\n");
		ret = -ENOMEM;
		goto err_no_ioremap  ;
	}

	if (res_num == 0) {
		mipi_base_info.con_reg_base = (unsigned int) regs;
		mipi_base_info.mipi_phys_base_addr = (unsigned int) res->start;
	} else if (res_num == 1) {
		mipi_base_info.csim_reg_base = (unsigned int) regs;
	} else if (res_num == 2) {
		mipi_base_info.csis_reg_base = (unsigned int) regs;
	}

	return 0;

err_no_ioremap:
	release_resource(d4_mipi_mem[res_num]);
	kfree(d4_mipi_mem[res_num]);

out: return ret;
}

/**
 * @brief MIPI device resource release
 * @fn d4_mipi_release_iomap(struct platform_device *pdev, struct drime4_jpeg *jpeg_ctx, int res_num)
 * @param struct platform_device *pdev, struct drime4_mipi *mipi_ctx, int res_num
 * @return int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
static int d4_mipi_release_iomap(struct platform_device *pdev,
				struct drime4_mipi *mipi_ctx, int res_num)
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

	out: return ret;
}

/**
 * @brief MIPI device probe
 * @fn d4_mipi_probe(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
static int __devinit d4_mipi_probe(struct platform_device *pdev)
{
   struct drime4_mipi *mipi = NULL;
   int mipi_csim_irq;
   int mipi_csis_irq;
   int ret = 0;

   mipi = kzalloc(sizeof(struct drime4_mipi), GFP_KERNEL);
   if (!mipi) {
      dev_err(&pdev->dev, "failed to allocate drime4_mipi.\n");
      ret = -EINVAL;
	  goto out;
   }

   mipi->pd = (struct drime4_mipi_dev_data *)pdev->dev.platform_data;
   if (mipi->pd == NULL) {
      dev_err(&pdev->dev, "mipi platform_data is empty.\n");
	  ret = -ENOSPC;
	  goto err_no_platform_data ;
   }

   mipi->id = pdev->id;
   mipi->dev = &pdev->dev;
   mipi->name = pdev->name;

   mipi_base_info.dev_info = &pdev->dev;

   /* register iomap resources */
   ret = d4_mipi_register_iomap(pdev, mipi, 0); /* CON Register Base */
   if (ret < 0)
      goto err_no_iomap_0;

   ret = d4_mipi_register_iomap(pdev, mipi, 1); /* CSIM Register Base */
   if (ret < 0)
      goto err_no_iomap_1;

   ret = d4_mipi_register_iomap(pdev, mipi, 2); /* CSIS Register Base */
   if (ret < 0)
      goto err_no_iomap_2;

   /* register irq resource */
   mipi_csis_irq = platform_get_irq(pdev, 0);
   mipi_base_info.csis_irq_num = mipi_csis_irq;

   mipi_csim_irq = platform_get_irq(pdev, 1);
   mipi_base_info.csim_irq_num = mipi_csim_irq;

   /**< MIPI Base Register Address & IRQ Number Setting, Device info */
   mipi_set_reg_ctrl_base_info(&mipi_base_info);

   /* MIPI device registration */
   ret = misc_register(&mipi_miscdev);
   if (ret < 0) {
      dev_err(&pdev->dev, "Failed to register misc driver.\n");
      goto err_no_register;
   }
   dev_dbg(&pdev->dev, "device registered as /dev/%s \n", MIPI_MODULE_NAME);

   platform_set_drvdata(pdev, mipi);

   mipi->dev = &pdev->dev;
   g_mipi = mipi;

   dev_dbg(&pdev->dev, "probe success.\n");

   return 0;

   err_no_register:
	   iounmap((void __iomem *)mipi_base_info.csis_reg_base);
	   release_resource(d4_mipi_mem[2]);
	   kfree(d4_mipi_mem[2]);
   err_no_iomap_2:
	   iounmap((void __iomem *)mipi_base_info.csim_reg_base);
	   release_resource(d4_mipi_mem[1]);
	   kfree(d4_mipi_mem[1]);
   err_no_iomap_1:
	   iounmap((void __iomem *)mipi_base_info.con_reg_base);
	   release_resource(d4_mipi_mem[0]);
	   kfree(d4_mipi_mem[0]);
   err_no_iomap_0:

   err_no_platform_data:
	   kfree(mipi);

   out:
	   return ret;
}

/**
 * @brief MIPI device remove
 * @fn d4_mipi_remove(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
static int d4_mipi_remove(struct platform_device *pdev)
{
   misc_deregister(&mipi_miscdev);

   iounmap((void __iomem *)mipi_base_info.con_reg_base);
   iounmap((void __iomem *)mipi_base_info.csim_reg_base);
   iounmap((void __iomem *)mipi_base_info.csis_reg_base);

   release_resource(d4_mipi_mem[0]);
   release_resource(d4_mipi_mem[1]);
   release_resource(d4_mipi_mem[2]);

   kfree(d4_mipi_mem[0]);
   kfree(d4_mipi_mem[1]);
   kfree(d4_mipi_mem[2]);

   kfree(g_mipi);

   return 0;
}

/**
 * @brief MIPI device suspend
 * @fn d4_mipi_suspend(struct platform_device *pdev, pm_message_t state)
 * @param struct platform_device *pdev, pm_message_t state
 * @return int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
static int d4_mipi_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret = 0;
	printk("\n\n [ mipi suspend start ]\n\n");
	misc_deregister(&mipi_miscdev);

	iounmap((void __iomem *)mipi_base_info.con_reg_base);
	iounmap((void __iomem *)mipi_base_info.csim_reg_base);
	iounmap((void __iomem *)mipi_base_info.csis_reg_base);

	ret = d4_mipi_release_iomap(pdev, g_mipi, 0);
	if (ret < 0)
		goto out;

	ret = d4_mipi_release_iomap(pdev, g_mipi, 1);
	if (ret < 0)
		goto out;

	ret = d4_mipi_release_iomap(pdev, g_mipi, 2);

	if (ret < 0)
		goto out;

	dev_err(&pdev->dev, "mipi_release_iomap ret : %d\n", ret);

	kfree(g_mipi);
	printk("\n\n [ mipi suspend end ]\n\n");
	return ret;

	out:
	printk(KERN_DEBUG "mipi_release_iomap fail : %d\n", ret);
	kfree(g_mipi);
	return ret;
}

/**
 * @brief MIPI device resume
 * @fn d4_mipi_resume(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
static int d4_mipi_resume(struct platform_device *pdev)
{
	struct drime4_mipi *mipi = NULL;
	int mipi_csim_irq;
	int mipi_csis_irq;
	int ret = 0;

	printk("\n\n [ mipi resume start ]\n\n");

	mipi = kzalloc(sizeof(struct drime4_mipi), GFP_KERNEL);
	if (!mipi) {
	   dev_err(&pdev->dev, "failed to allocate drime4_mipi.\n");
	   ret = -EINVAL;
	   goto out;
	}

	mipi->pd = (struct drime4_mipi_dev_data *)pdev->dev.platform_data;
	if (mipi->pd == NULL) {
	   dev_err(&pdev->dev, "mipi platform_data is empty.\n");
	   ret = -ENOSPC;
	   goto err_no_platform_data;
	}

	mipi->id = pdev->id;
	mipi->dev = &pdev->dev;
	mipi->name = pdev->name;

	mipi_base_info.dev_info = &pdev->dev;

	/* register iomap resources */
	ret = d4_mipi_register_iomap(pdev, mipi, 0); /* CON Register Base */
	if (ret < 0)
	   goto err_no_iomap_0;

	ret = d4_mipi_register_iomap(pdev, mipi, 1); /* CSIM Register Base */
	if (ret < 0)
	   goto err_no_iomap_1;

	ret = d4_mipi_register_iomap(pdev, mipi, 2); /* CSIS Register Base */
	if (ret < 0)
	   goto err_no_iomap_2;

	/* register irq resource */
	mipi_csis_irq = platform_get_irq(pdev, 0);
	mipi_base_info.csis_irq_num = mipi_csis_irq;

	mipi_csim_irq = platform_get_irq(pdev, 1);
	mipi_base_info.csim_irq_num = mipi_csim_irq;

	/**< MIPI Base Register Address & IRQ Number Setting, Device info */
	mipi_set_reg_ctrl_base_info(&mipi_base_info);

	/* MIPI device registration */
	ret = misc_register(&mipi_miscdev);
	if (ret < 0) {
	   dev_err(&pdev->dev, "Failed to register misc driver.\n");
	   goto err_no_register;
	}
	dev_dbg(&pdev->dev, "device registered as /dev/%s \n", MIPI_MODULE_NAME);

	platform_set_drvdata(pdev, mipi);

	mipi->dev = &pdev->dev;
	g_mipi = mipi;

	dev_dbg(&pdev->dev, "probe success.\n");
	printk("\n\n [ mipi resume end ]\n\n");

	return 0;

	err_no_register:
		iounmap((void __iomem *)mipi_base_info.csis_reg_base);
		release_resource(d4_mipi_mem[2]);
		kfree(d4_mipi_mem[2]);
	err_no_iomap_2:
		iounmap((void __iomem *)mipi_base_info.csim_reg_base);
		release_resource(d4_mipi_mem[1]);
		kfree(d4_mipi_mem[1]);
	err_no_iomap_1:
		iounmap((void __iomem *)mipi_base_info.con_reg_base);
		release_resource(d4_mipi_mem[0]);
		kfree(d4_mipi_mem[0]);
	err_no_iomap_0:

	err_no_platform_data:
		kfree(mipi);

	out:
		return ret;
}

static struct platform_driver mipi_driver = {
		.probe  = d4_mipi_probe,
		.remove = d4_mipi_remove,
		.suspend = d4_mipi_suspend,
		.resume = d4_mipi_resume,
		.driver = {
		.name = MIPI_MODULE_NAME,
		.owner = THIS_MODULE,
		},
};

/**
 * @brief MIPI device register
 * @fn d4_mipi_register(void)
 * @param void
 * @return int
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
static int d4_mipi_register(void)
{
   platform_driver_register(&mipi_driver);
   return 0;
}

/**
 * @brief MIPI device unregister
 * @fn d4_mipi_unregister(void)
 * @param void
 * @return void
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * @note  NONE
 */
static void d4_mipi_unregister(void)
{
   platform_driver_unregister(&mipi_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(d4_mipi_register);
#else
fast_dev_initcall(d4_mipi_register);
#endif
module_exit(d4_mipi_unregister);

MODULE_AUTHOR("Gunwoo Nam <gunwoo.nam@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 MIPI driver File");
MODULE_LICENSE("GPL");
