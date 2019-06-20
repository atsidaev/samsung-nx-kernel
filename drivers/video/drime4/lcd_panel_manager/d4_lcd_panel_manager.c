/**
 * @file d4_lcd_panel_manager.c
 * @brief DRIMe4 LCD Panel Manager
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
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
#include <linux/clk.h>
#include <mach/lcd_panel_manager/d4_lcd_panel_manager.h>
#include "d4_lcd_panel_manager_ctrl.h"

#if defined(CONFIG_MACH_D4_NX2000)
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/sched.h>
#endif

extern const struct file_operations lcd_panel_manager_ops;

static struct miscdevice lcd_panel_manager_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name  = LCD_PANEL_MANAGER_MODULE_NAME,
		.fops  = &lcd_panel_manager_ops,
};

#if defined(CONFIG_MACH_D4_NX2000)

static int irq_descriptor = 0;
static int irq_check = 0;
static struct work_struct lcd_work;

static void d4_lcd_panel_reset()
{
	mlcd_panel_reset();
	mlcd_panel_init();
	mlcd_panel_standby_off();	
}

static void d4_lcd_work_handler()
{
	mdelay(500);
	irq_check = 0;	
}

static irqreturn_t lcd_panel_reset_isr(int irq, void *handle)
{	
	struct drime4_lcd_panel_reset *lcd = (struct drime4_lcd_panel_reset *)handle;

	unsigned long ret;
	ret = gpio_get_value(DRIME4_GPIO16(1));
	
	if((lcd == NULL) || (ret == 0)) // check USB det
	{
		return IRQ_NONE;
	}
	if (irq_check==0)
	{
		irq_check = 1;
		d4_lcd_panel_reset();
		schedule_work(&lcd_work);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

#endif

/**
 * @brief LCD Panel Manager device probe
 * @fn lcd_panel_manager_probe(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int __devinit lcd_panel_manager_probe(struct platform_device *pdev)
{
	int ret = 0;

	/* LCD panel manager device registration */
	ret = misc_register(&lcd_panel_manager_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
	}

	lcd_panel_set_init_data((struct d4_lcd_panel_manager_data *)pdev->dev.platform_data);

	return 0;
}


/**
 * @brief LCD Panel Manager device remove
 * @fn lcd_panel_manager_remove(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int lcd_panel_manager_remove(struct platform_device *pdev)
{
	misc_deregister(&lcd_panel_manager_miscdev);

	return 0;
}

/**
 * @brief LCD Panel Manager device suspend
 * @fn sma_suspend(struct platform_device *pdev, pm_message_t state)
 * @param struct platform_device *pdev, pm_message_t state
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int lcd_panel_manager_suspend(struct platform_device *pdev, pm_message_t state)
{
#if defined(CONFIG_MACH_D4_NX2000)	
	disable_irq(irq_descriptor);
	free_irq(irq_descriptor, &pdev->dev);
#endif
	lcd_panel_suspend();
	return 0;
}

/**
 * @brief LCD Panel Manager device resume
 * @fn lcd_panel_manager_resume(struct platform_device *pdev)
 * @param struct platform_device *pdev
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int lcd_panel_manager_resume(struct platform_device *pdev)
{
#if defined(CONFIG_MACH_D4_NX2000)
	irq_descriptor = gpio_to_irq(GPIO_LCD_ESD_RESET);

	INIT_WORK(&lcd_work, d4_lcd_work_handler);		

	request_irq(irq_descriptor, lcd_panel_reset_isr, IRQF_DISABLED, "LCD PANEL RESET", &pdev->dev);
	irq_set_irq_type(irq_descriptor, IRQ_TYPE_EDGE_FALLING);
#endif	
	lcd_panel_resume();
	return 0;
}

static struct platform_driver lcd_panel_manager_driver = {
	.probe   = lcd_panel_manager_probe,
	.remove  = lcd_panel_manager_remove,
	.suspend = lcd_panel_manager_suspend,
	.resume  = lcd_panel_manager_resume,
	.driver  = {
		.name  = LCD_PANEL_MANAGER_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

/**
 * @brief LCD Panel Manager device register
 * @fn lcd_panel_manager_register(void)
 * @param void
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int lcd_panel_manager_register(void)
{
	platform_driver_register(&lcd_panel_manager_driver);
	return 0;
}

/**
 * @brief LCD Panel Manager device unregister
 * @fn lcd_panel_manager_unregister(void)
 * @param void
 * @return void
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static void lcd_panel_manager_unregister(void)
{
	platform_driver_unregister(&lcd_panel_manager_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(lcd_panel_manager_register);
#else
fast_late_initcall(lcd_panel_manager_register);
#endif
module_exit(lcd_panel_manager_unregister);

MODULE_AUTHOR("Kim Sunghoon <bluesay.kim@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 LCD Panel Manager driver File");
MODULE_LICENSE("GPL");
