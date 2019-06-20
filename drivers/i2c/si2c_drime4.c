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
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>

#include <mach/map.h>
#include <mach/d4_si2c.h>
#include <mach/d4_si2c_regs.h>
#include <mach/d4_si2c_type.h>
#include <linux/si2c_drime4.h>

#include <linux/kthread.h>
#include <linux/pinctrl/consumer.h>



/**
 * @brief  Slave I2C 해당 channel에 대한 사용을 요청하는 함수
 * @fn     struct drime4_si2c *drime4_si2c_request(int si2c_id)
 * @param  *pwm	 	:[in]사용할 SI2C의 정보를 담고있는 포인터
 * @return 없음
 * @author kyuchun.han
 * @note
 */
struct drime4_si2c *drime4_si2c_request(int si2c_id)
{
	struct drime4_si2c *si2c;
	int found = 0;

	mutex_lock(&si2c_lock);

	list_for_each_entry(si2c, &si2c_list, list) {
		if (si2c->id == si2c_id) {
			found = 1;
			break;
		}
	}
	if (found != 1)
			si2c = ERR_PTR(-ENOENT);

	mutex_unlock(&si2c_lock);
	return si2c;
}
EXPORT_SYMBOL(drime4_si2c_request);


static void drime4_si2c_clear_irq(struct drime4_si2c *si2c)
{
	unsigned long tmp;

	tmp = readl(si2c->reg_base + SI2C_CON);
/*	tmp = (1<<8)|tmp; */
	SI2C_CON_IRQCLEAR(tmp, SI2C_ENABLE);
	__raw_writel(tmp, si2c->reg_base + SI2C_CON);
}

static void drime4_si2c_pendclear_irq(struct drime4_si2c *si2c)
{
	unsigned long tmp;

	tmp = __raw_readl(si2c->reg_base + SI2C_CON);
/*	tmp &= ~(1<<4); */
	SI2C_CON_IRQPEND(tmp, SI2C_DISABLE);
	__raw_writel(tmp, si2c->reg_base + SI2C_CON);
}

static void drime4_si2c_stop(struct drime4_si2c *si2c)
{
	if (si2c == NULL)
		return;

	if (si2c->si2c_mode == SI2C_SRECEIVE)
		__raw_writel(SI2C_RSTOPCON, si2c->reg_base + SI2C_STOP);
	else
		__raw_writel(SI2C_TSTOPCON, si2c->reg_base + SI2C_STOP);
}


int drime4_si2c_config(struct drime4_si2c *si2c, enum si2c_tran_mode value, unsigned char saddr)
{
	void __iomem *base;
	unsigned int val;
	unsigned int addr;
	if (si2c == NULL)
		return -1;

	base = si2c->reg_base;
	val = 0;

	SI2C_CON_ACKEN(val,	SI2C_ENABLE);
	SI2C_CON_IRQEN(val,	SI2C_ENABLE);

	__raw_writel(val, base + SI2C_CON);

	addr = (saddr << 1);

	__raw_writel(addr, base + SI2C_ADD);

	val = 0;

	SI2C_STAT_TXRXEN(val, SI2C_ENABLE);
	SI2C_STAT_BUSBUSY(val, SI2C_ENABLE);
	SI2C_STAT_MODESEL(val, value);
	__raw_writel(val, base + SI2C_STAT);

	si2c->si2c_mode = value;
	si2c->addr = addr;
	si2c->addr_check = 0;
	return 0;
}



unsigned char drime4_si2c_byte_read(struct drime4_si2c *si2c)
{
	unsigned char data;
	data = __raw_readl(si2c->reg_base + SI2C_DS);
	drime4_si2c_pendclear_irq(si2c);
	return data;
}

unsigned int drime4_si2c_recive_addr_check(struct drime4_si2c *si2c)
{
	unsigned char data;
	data = __raw_readl(si2c->reg_base + SI2C_DS);
	drime4_si2c_pendclear_irq(si2c);
		if (si2c->addr == data)
		return 0;
	else
		return -1;
}



void drime4_si2c_recive_stop(struct drime4_si2c *si2c)
{
	unsigned char data;
	drime4_si2c_stop(si2c);
	drime4_si2c_pendclear_irq(si2c);
	data = __raw_readl(si2c->reg_base + SI2C_STAT);
	SI2C_STAT_TXRXEN(data, SI2C_DISABLE);
	SI2C_STAT_BUSBUSY(data, SI2C_DISABLE);
	__raw_writel(data, si2c->reg_base + SI2C_STAT);
}


void drime4_si2c_byte_write(struct drime4_si2c *si2c, unsigned char val)
{
	__raw_writel(val, si2c->reg_base + SI2C_DS);
	drime4_si2c_pendclear_irq(si2c);
}


unsigned int drime4_si2c_transmit_addr_check(struct drime4_si2c *si2c)
{
	unsigned char data = 0;
	if (!si2c->addr_check) {
		data = __raw_readl(si2c->reg_base + SI2C_DS);
		if (data != (si2c->addr + 1))
			return -1;
		else
			si2c->addr_check = SI2C_ENABLE;

	}
	return 0;
}




int drime4_si2c_ack_waiting(struct drime4_si2c *si2c, int time_out)
{
	int ret = 0;

	if (time_out == -1) {
		wait_for_completion(&si2c->done);
	} else {
		if (!wait_for_completion_timeout(&si2c->done,
					msecs_to_jiffies(time_out))) {
			ret = -EIO;
		}
	}
	return ret;
}


void drime4_si2c_last_byte_write(struct drime4_si2c *si2c, unsigned char val)
{
	drime4_si2c_stop(si2c);
	drime4_si2c_pendclear_irq(si2c);
	__raw_writel(val, si2c->reg_base + SI2C_DS);
	drime4_si2c_pendclear_irq(si2c);
	si2c->addr_check = 0;

}

void drime4_si2c_transmit_stop(struct drime4_si2c *si2c)
{
	unsigned char data;
	drime4_si2c_pendclear_irq(si2c);

	data = __raw_readl(si2c->reg_base + SI2C_STAT);
	SI2C_STAT_TXRXEN(data, SI2C_DISABLE);
	SI2C_STAT_BUSBUSY(data, SI2C_DISABLE);
	__raw_writel(data, si2c->reg_base + SI2C_STAT);
}


int drime4_status_check(struct drime4_si2c *si2c)
{
	unsigned int data;
	data = __raw_readl(si2c->reg_base + SI2C_STAT);
	if (data &= (1<<0)) {
		return -1;
	}
	return 0;
}


void drime4_si2c_err_stop(struct drime4_si2c *si2c)
{
	unsigned char data;
	drime4_si2c_stop(si2c);
	data = __raw_readl(si2c->reg_base + SI2C_STAT);
	SI2C_STAT_TXRXEN(data, SI2C_DISABLE);
	SI2C_STAT_BUSBUSY(data, SI2C_DISABLE);
	__raw_writel(data, si2c->reg_base + SI2C_STAT);
}


static int drime4_si2c_register_iomap(struct platform_device *pdev,
		struct drime4_si2c *si2c, int res_num)
{
	struct resource *res = NULL;
	void __iomem *regs;
	int ret = 0;

	/**
	 * @brief get resource for io memory
	 */
	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (!res) {
		dev_err(&pdev->dev, "Unable to get slave I2C resource\n");
		ret = -ENODEV;
		goto err_out;
	}

	res = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!res) {
		dev_err(&pdev->dev, "failed to request io memory region\n");
		ret = -ENOMEM;
		goto err_out;
	}


	regs = ioremap(res->start, resource_size(res));
	if (!res) {
		dev_err(&pdev->dev, "failed to request ioremap\n");
		ret = -ENOMEM;
		goto err_ioremap;
	}

	si2c->reg_base = regs;
	return 0;

err_ioremap:
	release_mem_region(res->start, resource_size(res));

err_out:
	return ret;

}


static int drime4_si2c_register(struct drime4_si2c *si2c)
{
	mutex_lock(&si2c_lock);
	list_add_tail(&si2c->list, &si2c_list);
	mutex_unlock(&si2c_lock);
	return 0;
}


static int drime4_si2c_unregister(struct drime4_si2c *si2c)
{
	mutex_lock(&si2c_lock);
	list_del(&si2c->list);
	mutex_unlock(&si2c_lock);
	return 0;
}



static irqreturn_t si2c_test_irq(int irq, void *dev)
{
	struct drime4_si2c *si2c;
	si2c = dev;
	drime4_si2c_clear_irq(si2c);
	complete(&si2c->done);
	return IRQ_HANDLED;
}


static int _drime4_si2c_resume(struct platform_device *pdev, int is_probe)
{
	int ret = 0;
	struct drime4_si2c *si2c = NULL;
	unsigned int pad_val;

	si2c = kzalloc(sizeof(struct drime4_si2c), GFP_KERNEL);
	if (si2c == NULL) {
		dev_err(&pdev->dev, "failed to allocate drime4_si2c.\n");
		return -EINVAL;
	}

	si2c->id = pdev->id;
	si2c->dev = &pdev->dev;
	si2c->name = pdev->name;

	if (is_probe == 1) {
		/* Clock Enable */
		si2c->clk = clk_get(&pdev->dev, "i2c0");
		clk_enable(si2c->clk);
	}

	ret = drime4_si2c_register_iomap(pdev, si2c, 0);

	if (ret < 0) {
		ret = -ENOENT;
		goto err_out;
	}

	init_completion(&si2c->done);

	si2c->si2c_irq = platform_get_irq(pdev, 0);

	if (si2c->si2c_irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		ret = -ENOENT;
		goto err_out;
	}

	ret = request_irq(si2c->si2c_irq, si2c_test_irq, 0, pdev->name, si2c);
	if (ret) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_irq;
	}

	ret = drime4_si2c_register(si2c);
	if (ret) {
		dev_err(&pdev->dev, "failed to register si2c\n");
		goto err_irq;
	}

	pad_val = __raw_readl(DRIME4_VA_PLATFORM_CTRL + 0x420);

	pad_val &= ~(1 << pdev->id);

	__raw_writel(pad_val, DRIME4_VA_PLATFORM_CTRL + 0x420);

	/* setup pad configuration */
	si2c->pinctrl = devm_pinctrl_get(&pdev->dev);
	si2c->pinstate = pinctrl_lookup_state(si2c->pinctrl,
			PINCTRL_STATE_DEFAULT);

	ret = pinctrl_select_state(si2c->pinctrl, si2c->pinstate);
	



	platform_set_drvdata(pdev, si2c);
	return 0;

err_irq:
iounmap((void *) si2c->reg_base);

err_out:
	kfree(si2c);
	return ret;
}


/**
 * @brief   Slave I2C 드라이버 Probe시 호출되는 함수
 * @fn      static int __devinit drime4_si2c_probe(struct platform_device *pdev)
 * @param   *pdev	[in] platform device 구조체
 * @return  실패시 음수값 반환
 *
 * @author  kyuchun han
 */
static int __devinit drime4_si2c_probe(struct platform_device *pdev)
{
	return _drime4_si2c_resume(pdev, 1);
}


static struct platform_driver drime4_si2c_driver = {
	.probe = drime4_si2c_probe,
	.driver = {
		.name = SI2C_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};



static int __init drime4_si2c_init(void)
{
	platform_driver_register(&drime4_si2c_driver);
	return 0;
}


static void __exit drime4_si2c_exit(void)
{
	platform_driver_unregister(&drime4_si2c_driver);
}

module_init(drime4_si2c_init);
module_exit(drime4_si2c_exit);

MODULE_AUTHOR("Kyuchun Han<kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("Samsung Drime IV Slave I2C Driver");
MODULE_LICENSE("GPL");


