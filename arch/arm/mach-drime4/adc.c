/* linux/arch/arm/mach-drime4/adc.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - ADC device core support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define DEBUG

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/io.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/slab.h>

#include <mach/adc.h>

/* Register map */
#define DRIME4_ADC_STBY		0x0
#define DRIME4_ADC_INTR_PENDING	0x4
#define DRIME4_ADC_INTR_MASK	0x8
#define DRIME4_ADC_INTR_CLEAR	0xc
#define DRIME4_ADC_CHATT_INT(x)	(0x14 + ((x) * 4))
#define DRIME4_ADC_RAW(x)	(0x3c + ((x) * 4))
#define DRIME4_ADC_CUR_VOLT(x)	(0x64 + ((x) * 4))
#define DRIME4_ADC_CHANNEL_SEL	0x8c

#define DRIME4_ADC_STBY_PD_ENABLE	(0)
#define DRIME4_ADC_STBY_PD_DISABLE	(1)
#define DRIME4_ADC_INT_ALL_DISABLE	(0x3FF)

#define DRIME4_ADC_CHANNEL_COUNT	10

enum adc_state {
	ADC_POWER_ON, ADC_POWER_OFF,
};

static int_handler adc_uhandler[10] = { NULL, };
static void *adc_param[10] = { NULL, };

struct drime4_adc_client {
	struct platform_device *pdev;
	struct drime4_adc_device *adc_dev;
	wait_queue_head_t *wait;

	ktime_t start_time;
	unsigned int ref_mvolt;
	unsigned int time_margin;
	unsigned int data_diff;
	unsigned int channel;
	int result;

};

struct drime4_adc_device {
	struct platform_device *pdev;
	struct clk *clk;
	void __iomem *mmio_base;
	spinlock_t lock;
	int irq;
	int_handler *adc_usr_handler;
	void **usr_param;
};

static struct drime4_adc_device *adc_dev;

/* 1.8 refv = 4095 */
static inline unsigned int convert_to_refvolt(unsigned int mvolt)
{
	return 2275 * mvolt / 100;
}

/* 1.8 refv = 4095 */
static inline unsigned int convert_to_mvolt(unsigned int refvolt)
{
	return refvolt * 100 / 2275;
}

/* 55 mv = 127 */
static inline unsigned int convert_to_datadiff(unsigned int mvolt)
{
	return 127 * mvolt / 55;
}

static inline void drime4_adc_power(struct drime4_adc_device *adc,
		enum adc_state power)
{
	if (power == ADC_POWER_ON)
		writel(DRIME4_ADC_STBY_PD_ENABLE, adc->mmio_base + DRIME4_ADC_STBY);
	else
		writel(DRIME4_ADC_STBY_PD_DISABLE, adc->mmio_base + DRIME4_ADC_STBY);
}

static inline void drime4_adc_int_all_disable(struct drime4_adc_device *adc)
{
	writel(DRIME4_ADC_INT_ALL_DISABLE, adc->mmio_base + DRIME4_ADC_INTR_MASK);
}

static void drime4_adc_conf_init(struct drime4_adc_device *adc)
{
	drime4_adc_int_all_disable(adc);
	drime4_adc_power(adc, ADC_POWER_ON);
	adc->adc_usr_handler = adc_uhandler;
	adc->usr_param = adc_param;
}

/**
 * @brief  ADC에 사용자 Interrupt Handler를 설정하는 함수
 * @fn     void drime4_adc_request_irq(
 struct drime4_adc_client *client,
 int_handler uhander,
 void *dev,
 unsigned int channel)
 * @param  *client	: [in]설정값이 저장되어있는 Client 포인터
 * @param  uhander	: [in]사용자 Interrupt Handler
 * @param  *dev		: [in]사용자 설정값의 포인터
 * @param  channel	: [in]사용할 ADC channel(channel:0~8)
 * @return 없음
 * @author kyuchun.han
 * @note
 */
void drime4_adc_request_irq(struct drime4_adc_client *client,
		int_handler uhander, void *dev, unsigned int channel)
{
	struct drime4_adc_device *adc;

	if (client == ERR_PTR(-EINVAL))
		return;

	if (client == NULL)
		return;

	if (client->adc_dev == NULL)
		return;

	adc = client->adc_dev;
	if (adc != NULL) {
		adc->adc_usr_handler[channel] = uhander;
		adc->usr_param[channel] = dev;
	}
}
EXPORT_SYMBOL_GPL(drime4_adc_request_irq);

/**
 * @brief  ADC에서 Interrupt를 사용여부를 설정하는 함수
 * @fn     void drime4_adc_int_set(
 struct drime4_adc_client *client,
 enum adc_int_state val,
 unsigned int channel)
 * @param  *client	: [in]설정값이 저장되어있는 Client 포인터
 * @param  val		: [in]Interrupt Enable / Disable 설정 값
 * @param  channel	: [in]사용할 ADC channel(channel:0~8)
 * @return 없음
 * @author kyuchun.han
 * @note
 */
void drime4_adc_int_set(struct drime4_adc_client *client,
		enum adc_int_state val, unsigned int channel)
{
	unsigned int pending;

	if (client == NULL)
		return;
	if (IS_ERR(client))
		return;

	struct drime4_adc_device *adc = client->adc_dev;

	pending = readl(adc->mmio_base + DRIME4_ADC_INTR_MASK);
	if (val == ADC_INT_ON)
		pending &= ~(1 << channel);
	else
		pending |= (1 << channel);
	writel(pending, adc->mmio_base + DRIME4_ADC_INTR_MASK);
}
EXPORT_SYMBOL_GPL(drime4_adc_int_set);

/**
 * @brief  ADC의 동작을 Run하는 함수
 * @fn     void drime4_adc_start(struct drime4_adc_client *client, unsigned int channel)
 * @param  *client : [in]설정값이 저장되어있는 Client 포인터
 * @param  channel : [in]사용할 ADC channel(channel:0~8)
 * @return 없음
 * @author kyuchun.han
 * @note
 */
void drime4_adc_start(struct drime4_adc_client *client, unsigned int channel)
{
	unsigned int chatt_int;
	unsigned int refvolt;
	unsigned int data_diff;
	unsigned long flags;

	if (IS_ERR(client))
		return;

	struct drime4_adc_device *adc = client->adc_dev;


	refvolt = convert_to_refvolt(client->ref_mvolt);
	data_diff = convert_to_datadiff(client->data_diff);

	chatt_int = (refvolt & 0xfff) | (client->time_margin & 0x1fff) << 12
			| (data_diff & 0x7f) << 25;
	channel = (channel > 8) ? 8 : channel;

	spin_lock_irqsave(&adc->lock, flags);

	writel(chatt_int, adc->mmio_base + DRIME4_ADC_CHATT_INT(channel));
	spin_unlock_irqrestore(&adc->lock, flags);
}
EXPORT_SYMBOL_GPL(drime4_adc_start);

/**
 * @brief  ADC의 인터럽트 발생 후 현재의 Voltage를 얻는 함수
 * @fn     int drime4_adc_read(struct drime4_adc_client *client, unsigned int channel)
 * @param  *client : [in]설정값이 저장되어있는 Client 포인터
 * @param  channel : [in]사용할 ADC channel(channel:0~8)
 * @return int     : voltage 값 (단위: mv)
 * @author kyuchun.han
 * @note
 */
int drime4_adc_read(struct drime4_adc_client *client, unsigned int channel)
{
	int ret;
	struct drime4_adc_device *adc = client->adc_dev;

	ret = readl(adc->mmio_base + DRIME4_ADC_CUR_VOLT(channel));
	ret = convert_to_mvolt(ret);
	return ret;
}
EXPORT_SYMBOL_GPL(drime4_adc_read);

/**
 * @brief  ADC의 인터럽트와 관계없이 현재 동작된 Voltage를 얻는 함수
 * @fn     int drime4_adc_read(struct drime4_adc_client *client, unsigned int channel)
 * @param  *client : [in]설정값이 저장되어있는 Client 포인터
 * @param  channel : [in]사용할 ADC channel(channel:0~8)
 * @return int     : voltage 값 (단위: mv)
 * @author kyuchun.han
 * @note
 */
int drime4_adc_raw_read(struct drime4_adc_client *client, unsigned int channel)
{
	int ret;
	struct drime4_adc_device *adc = client->adc_dev;

	ret = readl(adc->mmio_base + DRIME4_ADC_RAW(channel));
	ret = convert_to_mvolt(ret);
	return ret;
}
EXPORT_SYMBOL_GPL(drime4_adc_raw_read);


unsigned int drime4_adc_raw_read_hax(struct drime4_adc_client *client, unsigned int channel)
{
	int ret;
	struct drime4_adc_device *adc = client->adc_dev;

	ret = readl(adc->mmio_base + DRIME4_ADC_RAW(channel));
	return ret;
}
EXPORT_SYMBOL_GPL(drime4_adc_raw_read_hax);

/**
 * @brief  ADC에 사용될 설정값을 Client 공간에 저장하는 함수
 * @fn     struct drime4_adc_client *drime4_adc_register(
 struct platform_device *pdev,
 unsigned int ref_mvolt, unsigned int time_margin,
 unsigned int data_diff)
 * @param  *pdev		: [in]사용자 platform device 포인터
 * @param  ref_mvolt	: [in]참조 Voltage로 전압의 최소 변화량 설정값(단위: mv)
 * @param  time_margin	: [in]설정한 참조 Voltage 만큰의 변화 값이 안정적으로 발생하는 횟수 설정값으로 이후 interrrupt가 발생함(범위:1~8192)
 * @param  data_diff	: [in]전압 변화 시 Chattering 의 마진 설정값(전압 변화시 data_diff 이상이면 Chattering으로 봄)
 * @return drime4_adc_client * :설정값을 저장하는 client address value
 * @author kyuchun.han
 * @note
 */
struct drime4_adc_client *drime4_adc_register(struct platform_device *pdev,
		unsigned int ref_mvolt, unsigned int time_margin,
		unsigned int data_diff)
{
	struct drime4_adc_client *client;

	if (ref_mvolt > 180 || time_margin > 0x1fff || data_diff > 55) {
		dev_err(&pdev->dev, "Invalid parameter\n");
		return ERR_PTR(-EINVAL);
	}

	client = kzalloc(sizeof(struct drime4_adc_client), GFP_KERNEL);
	if (!client) {
		dev_err(&pdev->dev, "no memory for adc client\n");
		return NULL;
	}

	client->pdev = pdev;
	client->adc_dev = adc_dev;
	client->ref_mvolt = ref_mvolt;
	client->time_margin = time_margin;
	client->data_diff = data_diff;
	client->start_time.tv64 = 0;

	return client;
}
EXPORT_SYMBOL_GPL(drime4_adc_register);

/**
 * @brief  ADC register함수에 의해 할당된 저장소를 해지하는 함수
 * @fn     void drime4_adc_release(struct drime4_adc_client *client)
 * @param  *client : [in]설정값이 저장되어있는 Client 포인터
 * @return 없음
 * @author kyuchun.han
 * @note
 */
void drime4_adc_release(struct drime4_adc_client *client)
{
	if ((client != NULL) && (!IS_ERR(client)))
		kfree(client);
}

static irqreturn_t drime4_adc_irq(int irq, void *param)
{
	struct drime4_adc_device *adc = param;
	unsigned long pending;
	int offset;

	pending = readl(adc->mmio_base + DRIME4_ADC_INTR_PENDING);
	writel(pending, adc->mmio_base + DRIME4_ADC_INTR_CLEAR);

	for_each_set_bit(offset, &pending, DRIME4_ADC_CHANNEL_COUNT) {
		if (pending & (1 << offset)) {
			writel(1 << offset, adc->mmio_base + DRIME4_ADC_INTR_CLEAR);
			if ((adc->adc_usr_handler[offset] == NULL)
					|| (adc->usr_param[offset] == NULL))
			break;
			adc->adc_usr_handler[offset](adc->usr_param[offset]);
		}
	}
	return IRQ_HANDLED;
}

static int drime4_adc_probe(struct platform_device *pdev)
{
	struct drime4_adc_device *adc;
	struct resource *r;
	int ret_value;
	int ret;

	ret_value = 0;
	adc = devm_kzalloc(&pdev->dev, sizeof(struct drime4_adc_device), 	GFP_KERNEL);
	if (!adc) {
		dev_err(&pdev->dev, "failed to allocate\n");
		return -ENOMEM;
	}

	adc_dev = adc;

	spin_lock_init(&adc->lock);
	adc->pdev = pdev;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r) {
		ret_value = -ENODEV;
		goto err_free;
	}

	r = devm_request_mem_region(&pdev->dev, r->start, resource_size(r),
			pdev->name);
	if (!r) {
		ret_value = -EBUSY;
		goto err_free;
	}

	adc->mmio_base = devm_ioremap(&pdev->dev, r->start, resource_size(r));
	if (!adc->mmio_base) {
		dev_err(&pdev->dev, "failed to ioremap\n");
		ret_value = -ENOMEM;
		goto err_ioremap;

	}

	adc->irq = platform_get_irq(pdev, 0);
	if (adc->irq < 0) {
		dev_err(&pdev->dev, "no irq resource specified\n");
		ret_value = -ENOENT;
		goto err_irq;
	}

	ret = devm_request_irq(&pdev->dev, adc->irq, drime4_adc_irq, 0, pdev->name,
			adc);
	if (ret) {
		dev_err(&pdev->dev, "failed to request irq:%d (%d)\n", adc->irq, ret);
		ret_value = ret;
		goto err_irq;
	}

	adc->clk = clk_get(&pdev->dev, "adc");
	if (IS_ERR(adc->clk)) {
		dev_err(&pdev->dev, "failed to get adc clock\n");
		ret_value = PTR_ERR(adc->clk);
		goto err_free_clk;
	}

	clk_enable(adc->clk);

	platform_set_drvdata(pdev, adc);

	drime4_adc_conf_init(adc);

	return ret_value;

	err_free_clk: clk_put(adc->clk);

	err_irq: iounmap((void *) adc->mmio_base);

	err_ioremap: devm_release_mem_region(&pdev->dev, r->start, resource_size(r));

	err_free: devm_kfree(&pdev->dev, adc);
	return ret_value;
}

static int __devexit drime4_adc_remove(struct platform_device *pdev)
{
	struct drime4_adc_device *adc = platform_get_drvdata(pdev);

	if (adc != NULL) {
		clk_disable(adc->clk);
		clk_put(adc->clk);
		return 0;
	}

	return -1;
}

#ifdef CONFIG_PM
static int drime4_adc_suspend(struct platform_device *pdev, pm_message_t state)
{
	/* TODO: */
	return 0;
}

static int drime4_adc_resume(struct platform_device *pdev)
{
	/* TODO: */
	return 0;
}
#else
#define drime4_adc_suspend NULL
#define drime4_adc_resume NULL
#endif /* CONFIG_PM */

static struct platform_driver drime4_adc_driver = { .driver = {
	.name = "drime4-adc",
	.owner = THIS_MODULE,
}, .probe = drime4_adc_probe, .remove = __devexit_p(drime4_adc_remove),
		.suspend = drime4_adc_suspend, .resume = drime4_adc_resume, };

static int __init drime4_adc_init(void)
{
	int ret;

	ret = platform_driver_register(&drime4_adc_driver);
	if (ret)
	printk(KERN_ERR "%s: failed to add adc driver\n", __func__);

	return ret;
}
arch_initcall(drime4_adc_init);

static void __exit drime4_adc_exit(void)
{
	platform_driver_unregister(&drime4_adc_driver);
}
module_exit(drime4_adc_exit);

MODULE_AUTHOR("Kyuchun han, <kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("drime4 Hardware ADC Driver");
MODULE_LICENSE("GPL");
