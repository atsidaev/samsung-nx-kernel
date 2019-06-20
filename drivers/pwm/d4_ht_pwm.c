/* linux/drivers/pwm/d4_ht_pwm.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	Kyuchun Han <kyuchun.han@samsung.com>
 *
 * DRIME4 Hardware Trigger PWM platform device driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/d4_ht_pwm.h>
#include <mach/map.h>
#include <mach/d4_ht_pwm_regs.h>
#include <asm/div64.h>
#include <linux/poll.h>
#include <linux/mm.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>


#define FRACTIONLPART(m)	(m >= 5) ? 1 : 0

#define PWM_ENABLE		1
#define PWM_DISABLE		0

#define PWM_INT_CLEAR	1

#define D4_PWM_EXT_GROUP0_SET(_pwm_no) \
		((_pwm_no < 5) ? 0x0 : 0x4)    \

#define D4_PWM_EXT_GROUP1_SET(_pwm_no) \
		(((_pwm_no-9) < 5) ? 0x0 : 0x4)    \


#define D4_PWM_EXT_OFFSET(_pwm_no) \
		((_pwm_no < 9) ? D4_PWM_EXT_GROUP0_SET(_pwm_no) : D4_PWM_EXT_GROUP1_SET(_pwm_no)) \

#define D4_PWM_ID_SET(_pwm_no) \
		((_pwm_no < 9) ? _pwm_no : (_pwm_no-9))    \

#define ADD_OFFSET 0x100
#define ADD_GROUP_OFFSET 0x1000
enum fretype {
	HTPWM_TYPE1, HTPWM_TYPE2
};

struct ht_pwm_device {
	struct list_head list;
	struct platform_device *pdev;
	void __iomem *reg;
	void __iomem *ext_reg;
	int irq;
//	struct clk *clk_div;
	struct clk *clk;
	struct pinctrl *pmx;
	struct pinctrl_state	*pins_default;
	const char *label;
	enum ht_pwm_int_enable_type int_type;
	unsigned int invert_type;
	unsigned int invert_time;
	unsigned char use_count;
	unsigned char pwm_id;
	enum ht_pwm_op_mode op_mode;
	enum ht_pwm_start_edge	 s_type;
	enum ht_pwm_end_type e_type;
	void (*int_handler)(int);
	struct completion done;
	unsigned int add_start;
	unsigned int pwm_size;
	unsigned int add_start1;
	unsigned int pwm_size1;
//	wait_queue_head_t pwm_wq;
};

static DEFINE_MUTEX(pwm_lock);
static LIST_HEAD(pwm_list);

struct pwm_conf_value {
	unsigned long long period_sec;
	unsigned long long duty_sec;
	enum fretype fre_type;
};

static void __iomem *ext0_base;
static void __iomem *ext1_base;

struct pwm_port_select {
	unsigned char id;
	unsigned char port;
};

struct pwm_port_select pwm_port_num[] = {
		{ 0, DRIME4_GPIO12(5)},
		{ 1, DRIME4_GPIO12(6)},
		{ 2, DRIME4_GPIO12(7)},
		{ 3, DRIME4_GPIO13(0)},
		{ 4, DRIME4_GPIO13(1)},
		{ 5, DRIME4_GPIO13(2)},
		{ 6, DRIME4_GPIO13(3)},
		{ 7, DRIME4_GPIO13(4)},
		{ 8, DRIME4_GPIO13(5)},
		{ 9, DRIME4_GPIO13(6)},
		{ 10, DRIME4_GPIO13(7)},
		{ 11, DRIME4_GPIO14(0)},
		{ 12, DRIME4_GPIO8(6)},
		{ 13, DRIME4_GPIO8(7)},
		{ 14, DRIME4_GPIO15(1)},
		{ 15, DRIME4_GPIO14(5)},
		{ 16, DRIME4_GPIO15(7)},
		{ 17, DRIME4_GPIO16(0)},};

static int d4_ht_pwm_get_conf_value(struct ht_pwm_device *pwm,
		struct pwm_conf_value *setval)
{
	void __iomem *base = pwm->reg;
	unsigned long long period_cycles, period_cycles_divf, fp;
	unsigned long long duty_cycle, duty_cycle_divf;
	unsigned long long period, duty, prescale;
	unsigned long long c;
	unsigned int val;
	unsigned long long div;

	if (setval->fre_type > HTPWM_TYPE2)
		return -EINVAL;

	c = clk_get_rate(pwm->clk);

	div = 1000000000;
	div = div * 10;
	do_div(div, c);

	period_cycles = setval->period_sec * 10;
	period_cycles_divf = do_div(period_cycles, div);

	fp = (period_cycles_divf * 8);
	do_div(fp, div);
	fp = FRACTIONLPART(fp);

	period_cycles = period_cycles + fp;

	if (period_cycles < 1)
		period_cycles = 1;

	prescale = (period_cycles - 1);
	do_div(prescale, 65535);
	period = period_cycles;
	do_div(period, (prescale + 1));

	duty_cycle = setval->duty_sec * 10;
	duty_cycle_divf = do_div(duty_cycle, div);

	fp = (duty_cycle_divf * 8);
	do_div(fp, div);
	fp = FRACTIONLPART(fp);

	duty = (duty_cycle + fp);
	do_div(duty, (prescale + 1));



	if (setval->fre_type == HTPWM_TYPE1) {
		val = __raw_readl(base + PWM_CH_PRE);
		PWM_CH_PRE_SET1(val, (unsigned int)prescale);
		__raw_writel(val, base + PWM_CH_PRE);

		val = __raw_readl(base + PWM_CH_CYCLE1);
		PWM_CH_CYCLE1_PERIOD(val, (unsigned int)period);
		PWM_CH_CYCLE1_DUTY(val, (unsigned int)duty);
		__raw_writel(val, base + PWM_CH_CYCLE1);

	} else {
		val = __raw_readl(base + PWM_CH_PRE);
		PWM_CH_PRE_SET2(val, (unsigned int)prescale);
		__raw_writel(val, base + PWM_CH_PRE);

		val = __raw_readl(base + PWM_CH_CYCLE2);
		PWM_CH_CYCLE2_PERIOD(val, (unsigned int)period);
		PWM_CH_CYCLE2_DUTY(val, (unsigned int)duty);
		__raw_writel(val, base + PWM_CH_CYCLE2);
	}
	return 0;
}

static void d4_ht_pwm_int_set(struct ht_pwm_device *pwm)
{
	void __iomem *base = pwm->reg;
	unsigned int val;

	val = __raw_readl(base + PWM_CH_CONFIG);

	switch (pwm->int_type) {
	case ALL_PERIOD:
		PWM_CH_CONFIG_INTEN1(val, PWM_ENABLE);
		PWM_CH_CONFIG_INTEN2(val, PWM_ENABLE);
		PWM_CH_CONFIG_INTEN(val, PWM_ENABLE);
		break;
	case MIX_FIRST_PERIOD:
		PWM_CH_CONFIG_INTEN1(val, PWM_ENABLE);
		PWM_CH_CONFIG_INTEN(val, PWM_ENABLE);
		break;
	case MIX_SECOND_PERIOD:
		PWM_CH_CONFIG_INTEN2(val, PWM_ENABLE);
		PWM_CH_CONFIG_INTEN(val, PWM_ENABLE);
		break;
	case MIX_LAST_END:
		PWM_CH_CONFIG_INTEN(val, PWM_ENABLE);
		break;
	case NOT_USED:
		PWM_CH_CONFIG_INTEN(val, PWM_DISABLE);
		PWM_CH_CONFIG_INTEN1(val, PWM_DISABLE);
		PWM_CH_CONFIG_INTEN2(val, PWM_DISABLE);
		break;
	default:
		break;
	}
	__raw_writel(val, base + PWM_CH_CONFIG);
}

/**
 * @brief  PWM의 Interrupt 동작을 설정하는 함수
 * @fn     void d4_ht_pwm_int_enalbe(struct ht_pwm_device *pwm, enum ht_pwm_int_enable_type int_type)
 * @param  *pwm		: [in]사용할 PWM의 정보를 담고 있는 포인터
 * @param  int_type : [in]어떤 형태의 인터럽트를 사용할 것인지에 관한 Type
 * @return void
 * @author kyuchun.han
 * @note
 */
void d4_ht_pwm_int_enalbe(struct ht_pwm_device *pwm,
		enum ht_pwm_int_enable_type int_type)
{
	pwm->int_type = int_type;
}
EXPORT_SYMBOL(d4_ht_pwm_int_enalbe);

/**
 * @brief  PWM의 Interrupt 동작을 정지시키는 함수
 * @fn     void d4_ht_pwm_int_disable(struct ht_pwm_device *pwm)
 * @param  *pwm		:[in] 사용할 PWM의 정보를 담고 있는 포인터
 * @return void
 * @author kyuchun.han
 * @note
 */
void d4_ht_pwm_int_disable(struct ht_pwm_device *pwm)
{
	void __iomem *base = pwm->reg;
	unsigned int val;

	val = __raw_readl(base + PWM_CH_CONFIG);
	PWM_CH_CONFIG_INTEN(val, PWM_DISABLE);
	PWM_CH_CONFIG_INTEN1(val, PWM_DISABLE);
	PWM_CH_CONFIG_INTEN2(val, PWM_DISABLE);
	__raw_writel(val, base + PWM_CH_CONFIG);
}
EXPORT_SYMBOL(d4_ht_pwm_int_disable);

/**
 * @brief  PWM에 사용자 Interrupt Handler를 설정하는 함수
 * @fn     void d4_ht_pwm_int_func_set(struct ht_pwm_device *pwm, void (*callback_func)(void))
 * @param *pwm				: [in]사용할 PWM의 정보를 담고있는 포인터
 * @param  *callback_func	: [in]사용자 Interrupt Handler
 * @return 없음
 * @author kyuchun.han
 * @note
 */
void d4_ht_pwm_int_func_set(struct ht_pwm_device *pwm, void (*callback_func)(
		int))
{
	pwm->int_handler = callback_func;
}
EXPORT_SYMBOL(d4_ht_pwm_int_func_set);


void d4_ht_pwm_pad_set(struct ht_pwm_device *pwm, unsigned int val)
{
	if (val == 1) {
		pinctrl_free_gpio(pwm_port_num[pwm->pwm_id].port);
		
		/* printk("==============pwm pad set 1:%d\n", pwm->pwm_id); */
		pwm->pmx = devm_pinctrl_get(&pwm->pdev->dev);
		
		pwm->pins_default = pinctrl_lookup_state(pwm->pmx, PINCTRL_STATE_DEFAULT);
		pinctrl_select_state(pwm->pmx, pwm->pins_default);
	} else {
		
		/* printk("==============pwm pad set 0:%d\n", pwm->pwm_id); */
		devm_pinctrl_put(pwm->pmx);
		
		pinctrl_request_gpio(pwm_port_num[pwm->pwm_id].port);
	}
}
EXPORT_SYMBOL(d4_ht_pwm_pad_set);

/**
 * @brief  PWM에 사용될 값들을 설정하는 함수
 * @fn     int d4_ht_pwm_config(struct ht_pwm_device *pwm, struct ht_pwm_conf_info *pwm_info)
 * @param  *pwm			: [in]사용할 PWM의 정보를 담고있는 포인터
 * @param  *pwm_info	: [in]PWM동작에 필요한 설정 내용을 담고 있는 포인터
 * @return int 			: 설정성공여부 전달 성공:0
 * @author kyuchun.han
 * @note
 */
int d4_ht_pwm_config(struct ht_pwm_device *pwm,
		struct ht_pwm_conf_info *pwm_info)
{
	void __iomem *base;

	struct ht_pwm_conf_info *ch;
	struct pwm_conf_value conf_freq1, conf_freq2;
	unsigned int ctrl, conf, val;

	if (pwm == NULL)
		return -1;

	if (pwm_info == NULL)
		return -1;

	ch = pwm_info;
	base = pwm->reg;
/*
	if (ch->opmode > CONTINUE_PULSE)
		return -EINVAL;
*/
	/* Channel Stop & Control Register Clear */
	__raw_writel(PWM_DISABLE, base + PWM_CH_CTRL);
	__raw_writel(PWM_DISABLE, base + PWM_CH_CONFIG);
	__raw_writel(PWM_DISABLE, base + PWM_CH_PRE);
	__raw_writel(PWM_DISABLE, base + PWM_CH_CYCLE1);
	__raw_writel(PWM_DISABLE, base + PWM_CH_CYCLE2);

	ctrl = 0;
	conf = 0;

	pwm->op_mode = ch->opmode;
	pwm->e_type = ch->endtype;
	pwm->s_type = ch->starttype;

	switch (ch->opmode) {
	case CONTINUE_PULSE:
		conf_freq1.duty_sec = ch->freq1.duty;
		conf_freq1.period_sec = ch->freq1.period;
		conf_freq1.fre_type = HTPWM_TYPE1;
		d4_ht_pwm_get_conf_value(pwm, &conf_freq1);
		PWM_CH_CTRL_CONTINUE(ctrl, PWM_ENABLE);
		__raw_writel(ctrl, base + PWM_CH_CTRL);
		break;
	case MIX_PULSE:
		conf_freq1.duty_sec = ch->freq1.duty;
		conf_freq1.period_sec = ch->freq1.period;
		conf_freq1.fre_type = HTPWM_TYPE1;
		d4_ht_pwm_get_conf_value(pwm, &conf_freq1);
		PWM_CH_CONFIG_NUM_CYCLE1(conf, ch->freq1.count);
		conf_freq2.duty_sec = ch->freq2.duty;
		conf_freq2.period_sec = ch->freq2.period;
		conf_freq2.fre_type = HTPWM_TYPE2;
		d4_ht_pwm_get_conf_value(pwm, &conf_freq2);
		PWM_CH_CONFIG_NUM_CYCLE2(conf, ch->freq2.count);
		break;
	case ONETYPE_PULSE:
		conf_freq2.duty_sec = ch->freq2.duty;
		conf_freq2.period_sec = ch->freq2.period;
		conf_freq2.fre_type = HTPWM_TYPE2;
		d4_ht_pwm_get_conf_value(pwm, &conf_freq2);
		PWM_CH_CONFIG_NUM_CYCLE2(conf, ch->freq2.count);
		break;

	default:
		break;
	}

	pwm->invert_type = ch->invert;
	pwm->invert_time = ch->ivt_time;
	if (pwm->invert_time == INVERT_CONFIG) {
	val = __raw_readl(base + PWM_CH_CTRL);
	PWM_CH_CTRL_INVERT(val, pwm->invert_type);
	__raw_writel(val, base + PWM_CH_CTRL);
	}

	PWM_CH_CONFIG_MODE0(conf, ch->starttype);
	PWM_CH_CONFIG_MODE1(conf, ch->resource);
	PWM_CH_CONFIG_MODE2(conf, ch->trigtype);

	if (pwm->invert_type == INVERT_ON)
		PWM_CH_CONFIG_MODE3(conf, ~ch->endtype);
	else
	PWM_CH_CONFIG_MODE3(conf, ch->endtype);

		__raw_writel(conf, base + PWM_CH_CONFIG);
	return 0;
}
EXPORT_SYMBOL(d4_ht_pwm_config);

/**
 * @brief  PWM에 설정을 초기화 하는 함수
 * @fn     void d4_ht_pwm_clear(struct ht_pwm_device *pwm)
 * @param  *pwm	 :[in]사용할 PWM의 정보를 담고있는 포인터
 * @return 없음
 * @author kyuchun.han
 * @note
 */
void d4_ht_pwm_clear(struct ht_pwm_device *pwm)
{
	void __iomem *base = pwm->reg;

	/* Channel Stop & Control Register Clear */
	__raw_writel(PWM_DISABLE, base + PWM_CH_CTRL);
	__raw_writel(PWM_DISABLE, base + PWM_CH_CONFIG);
	__raw_writel(PWM_DISABLE, base + PWM_CH_PRE);
	__raw_writel(PWM_DISABLE, base + PWM_CH_CYCLE1);
	__raw_writel(PWM_DISABLE, base + PWM_CH_CYCLE2);
}
EXPORT_SYMBOL(d4_ht_pwm_clear);


/**
 * @brief  PWM에 동작을 수행 시키는 함수
 * @fn     void d4_ht_pwm_enable(struct ht_pwm_device *pwm)
 * @param  *pwm	 :[in]사용할 PWM의 정보를 담고있는 포인터
 * @return 없음
 * @author kyuchun.han
 * @note
 */
void d4_ht_pwm_enable(struct ht_pwm_device *pwm)
{
	void __iomem *base = pwm->reg;
	unsigned int val;

	val = 0;

	val = __raw_readl(base + PWM_CH_CTRL);
	PWM_CH_CTRL_INTCLEAR(val, PWM_ENABLE);

	__raw_writel(val, base + PWM_CH_CTRL);

	d4_ht_pwm_int_set(pwm);


	val = __raw_readl(base + PWM_CH_CTRL);

	if (pwm->invert_time == INVERT_START) {
		PWM_CH_CTRL_INVERT(val, pwm->invert_type);
	}

	PWM_CH_CTRL_START(val, PWM_ENABLE);
	__raw_writel(val, base + PWM_CH_CTRL);
}
EXPORT_SYMBOL(d4_ht_pwm_enable);

/**
 * @brief  PWM에 동작을 중단 시키는 함수
 * @fn     void d4_ht_pwm_disable(struct ht_pwm_device *pwm)
 * @param  *pwm	 :[in]사용할 PWM의 정보를 담고있는 포인터
 * @return 없음
 * @author kyuchun.han
 * @note
 */
void d4_ht_pwm_disable(struct ht_pwm_device *pwm)
{
	void __iomem *base = pwm->reg;
	unsigned int val;

	val = __raw_readl(base + PWM_CH_CTRL);
	PWM_CH_CTRL_START(val, PWM_DISABLE);
	PWM_CH_CTRL_INTCLEAR(val, PWM_ENABLE);
	PWM_CH_CTRL_INVERT(val, PWM_DISABLE);
	__raw_writel(val, base + PWM_CH_CTRL);
}
EXPORT_SYMBOL(d4_ht_pwm_disable);

/**
 * @brief  PWM에 동작 Trigger외부로 부터 받을수 있도록 설정하는 함수
 * @fn     void d4_ht_pwm_extinput_set(struct ht_pwm_device *pwm, enum ht_pwm_extint input_num)
 * @param  *pwm	 :[in]사용할 PWM의 정보를 담고있는 포인터
 * @return 없음
 * @author kyuchun.han
 * @note
 */
void d4_ht_pwm_extinput_set(struct ht_pwm_device *pwm,
		enum ht_pwm_extint input_num)
{
	void __iomem *base = pwm->ext_reg;
	unsigned int offset;
	unsigned int val;
	unsigned int ch_id;
	offset = D4_PWM_EXT_OFFSET(pwm->pwm_id);

	mutex_lock(&pwm_lock);

	val = __raw_readl(base + offset);

	ch_id = D4_PWM_ID_SET(pwm->pwm_id);
	if (ch_id  > 4)
		ch_id = ch_id - 5;

	PWM_CH_EXTIN_SET(val, input_num, (ch_id*4));
	__raw_writel(val, base + offset);
	mutex_unlock(&pwm_lock);
}
EXPORT_SYMBOL(d4_ht_pwm_extinput_set);

/**
 * @brief  PWM의 해당 channel에 대한 사용을 요청하는 함수
 * @fn     struct ht_pwm_device *d4_ht_pwm_request(int pwm_id, const char *label)
 * @param  *pwm	 	:[in]사용할 PWM의 정보를 담고있는 포인터
 * @param  *label	:[in]PWM을 사용할 Dev의 이름
 * @return 없음
 * @author kyuchun.han
 * @note
 */
struct ht_pwm_device *d4_ht_pwm_request(int pwm_id, const char *label)
{
	struct ht_pwm_device *pwm;
	int found = 0;

	mutex_lock(&pwm_lock);

	list_for_each_entry(pwm, &pwm_list, list) {
		if (pwm->pwm_id == pwm_id) {
			found = 1;
			break;
		}
	}
	if (found) {
		if (pwm->use_count == 0) {
			pwm->use_count = 1;
			pwm->label = label;
		} else
			pwm = ERR_PTR(-EBUSY);
	} else
		pwm = ERR_PTR(-ENOENT);

	mutex_unlock(&pwm_lock);
	return pwm;
}
EXPORT_SYMBOL(d4_ht_pwm_request);

/**
 * @brief  PWM의 해당 channel에 대한 사용을 넘겨주는 함수
 * @fn     void d4_ht_pwm_free(struct ht_pwm_device *pwm)
 * @param  *pwm	 :[in]사용할 PWM의 정보를 담고있는 포인터
 * @return 없음
 * @author kyuchun.han
 * @note
 */
void d4_ht_pwm_free(struct ht_pwm_device *pwm)
{
	mutex_lock(&pwm_lock);

	if (pwm->use_count) {
		pwm->use_count--;
		pwm->label = NULL;
	} else
	    printk(KERN_ERR "PWM%d device already freed\n", pwm->pwm_id);

	mutex_unlock(&pwm_lock);
}
EXPORT_SYMBOL(d4_ht_pwm_free);

void d4_pwm_get_phy_info(struct ht_pwm_device *pwm, struct ht_pwm_phys_reg_info *info)
{
	info->reg_ch_addr = pwm->add_start;
	info->reg_ch_size = pwm->pwm_size;
}
EXPORT_SYMBOL(d4_pwm_get_phy_info);


static int d4_pwm_register(struct ht_pwm_device *pwm)
{
	mutex_lock(&pwm_lock);
	list_add_tail(&pwm->list, &pwm_list);
	mutex_unlock(&pwm_lock);
	return 0;
}

static int d4_pwm_unregister(struct ht_pwm_device *pwm)
{
	mutex_lock(&pwm_lock);
	list_del(&pwm->list);
	mutex_unlock(&pwm_lock);
	return 0;
}


static irqreturn_t d4_pwm_irq(int irq, void *dev)
{
	struct ht_pwm_device *pwm = dev;
	void __iomem *base = pwm->reg;
	unsigned int val;
	irqreturn_t rtval;

	val = __raw_readl(base + PWM_CH_CTRL);
	PWM_CH_CTRL_INTCLEAR(val, PWM_INT_CLEAR);
	__raw_writel(val, base + PWM_CH_CTRL);

	if (pwm->int_handler == NULL) {
		rtval = IRQ_NONE;
	} else {
		pwm->int_handler(pwm->pwm_id);
		rtval = IRQ_HANDLED;
	}
	return rtval;
}

static int ht_pwm_register_iomap(struct platform_device *pdev,
		struct ht_pwm_device *pwm)
{
	struct resource *res;
	int ret = 0;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		ret = -ENODEV;
		goto out;
	}

	res = request_mem_region(res->start, resource_size(res), pdev->name);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to request memory resource\n");
		ret = -EBUSY;
		goto out;
	}

	pwm->ext_reg = ioremap(res->start, resource_size(res));
	if (pwm->ext_reg == NULL) {
		dev_err(&pdev->dev, "failed to ioremap() registers\n");
		ret = -ENODEV;
		goto err_ext_free_mem;
	}

err_ext_free_mem:
	release_mem_region(res->start, resource_size(res));

out:
	return ret;
}

static int d4_pwm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ht_pwm_device *pwm;
	struct resource *res;
	int ret;

	if (!pdev) {
		dev_err(dev, "No Platform Data\n");
		return -ENXIO;
	}

	pwm = kzalloc(sizeof(struct ht_pwm_device), GFP_KERNEL);
	if (pwm == NULL) {
		dev_err(dev, "failed to allocate ht_pwm_device\n");
		return -ENOMEM;
	}

	pwm->pdev = pdev;
	pwm->pwm_id = pdev->id;
	pwm->int_type = NOT_USED;
	pwm->clk = clk_get(dev, "pwm");

	if (pwm->clk == -2) {
		ret = -ENOMEM;
		goto err_alloc;
	}

	clk_enable(pwm->clk);

	if(pwm->pwm_id == 13) {
	pwm->pmx = devm_pinctrl_get(&pdev->dev);
	pwm->pins_default = pinctrl_lookup_state(pwm->pmx, PINCTRL_STATE_DEFAULT);
	pinctrl_select_state(pwm->pmx, pwm->pins_default);
	}

	if (IS_ERR(pwm->clk)) {
		dev_err(dev, "failed to get pwm clk\n");
		ret = PTR_ERR(pwm->clk);
		goto err_alloc;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		ret = -ENODEV;
		goto err_free_clk;
	}

	res = request_mem_region(res->start, resource_size(res), pdev->name);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to request memory resource\n");
		ret = -EBUSY;
		goto err_free_clk;
	}

	if (pdev->id < 9)
		pwm->add_start = (res->start) - (pdev->id*ADD_OFFSET);
	else
		pwm->add_start = (res->start) - ((pdev->id-9)*ADD_OFFSET) - 0x1000;

	pwm->pwm_size = ADD_GROUP_OFFSET*2;

	pwm->reg = ioremap(res->start, resource_size(res));
	if (pwm->reg == NULL) {
		dev_err(&pdev->dev, "failed to ioremap() registers\n");
		ret = -ENODEV;
		goto err_free_mem;
	}

	if (pdev->id < 9) {
		if (ext0_base == NULL) {
			ret = ht_pwm_register_iomap(pdev, pwm);
			if (res < 0) {
				dev_err(&pdev->dev, "failed to request memory resource\n");
				ret = -EBUSY;
				goto err_free_mem;
			}
			ext0_base = pwm->ext_reg;
		} else {
			pwm->ext_reg = ext0_base;
		}
	} else {
		if (ext1_base == NULL) {
			ret = ht_pwm_register_iomap(pdev, pwm);
			if (res < 0) {
				dev_err(&pdev->dev, "failed to request memory resource\n");
				ret = -EBUSY;
				goto err_free_mem;
			}
			ext1_base = pwm->ext_reg;
		} else {
			pwm->ext_reg = ext1_base;
		}
	}

	init_completion(&pwm->done);

	pwm->irq = platform_get_irq(pdev, 0);
	if (pwm->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		ret = -ENOENT;
		goto err_irq;
	}

	ret = request_irq(pwm->irq, d4_pwm_irq, 0, pdev->name, pwm);
	if (ret) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_irq;
	}

	ret = d4_pwm_register(pwm);
	if (ret) {
		dev_err(dev, "failed to register pwm\n");
		goto err_irq;
	}

	platform_set_drvdata(pdev, pwm);
	return 0;

	err_irq: iounmap((void *) pwm->reg);

	err_free_mem: release_mem_region(res->start, resource_size(res));

	err_free_clk: clk_put(pwm->clk);

/*	err_clk_tdiv: clk_put(pwm->clk_div); */

	err_alloc: kfree(pwm);

	return ret;
}

static int __devexit d4_pwm_remove(struct platform_device *pdev)
{
	struct ht_pwm_device *pwm = platform_get_drvdata(pdev);
	struct resource *mem_res;

	if (pwm == NULL)
		return -1;

	clk_disable(pwm->clk);
	clk_put(pwm->clk);

	iounmap((void *) pwm->reg);

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem_res != NULL)
		release_mem_region(mem_res->start, resource_size(mem_res));


	if (pdev->id < 9) {
		if (ext0_base != NULL) {
			iounmap((void *) pwm->ext_reg);
			mem_res  = platform_get_resource(pdev, IORESOURCE_MEM, 1);
			if (mem_res != NULL)
				release_mem_region(mem_res->start, resource_size(mem_res));
			ext0_base = NULL;
		}
	} else {
		if (ext1_base != NULL) {
			iounmap((void *) pwm->ext_reg);
			mem_res  = platform_get_resource(pdev, IORESOURCE_MEM, 1);
			if (mem_res != NULL)
				release_mem_region(mem_res->start, resource_size(mem_res));
			ext1_base = NULL;
		}
	}

	platform_set_drvdata(pdev, NULL);
	d4_pwm_unregister(pwm);
	kfree(pwm);
	return 0;
}


#ifdef CONFIG_PM
static int d4_pwm_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct ht_pwm_device *pwm = platform_get_drvdata(pdev);

	if (pwm == NULL)
		return -1;
	/* Disable the clock */
	clk_disable(pwm->clk);

	return 0;
}

static int d4_pwm_resume(struct platform_device *pdev)
{
	int ret;
	struct ht_pwm_device *pwm = platform_get_drvdata(pdev);

	if (pwm == NULL)
		return -1;

		/* Disable the clock */
	clk_enable(pwm->clk);
	printk("pwm resueme:%d\n", pwm->pwm_id);
//	pwm->pmx = devm_pinctrl_get(&pdev->dev);
//	pwm->pins_default = pinctrl_lookup_state(pwm->pmx, PINCTRL_STATE_DEFAULT);
//	pinctrl_select_state(pwm->pmx, pwm->pins_default);


	return 0;
}

#else
#define d4_pwm_suspend NULL
#define d4_pwm_resume NULL
#endif

static struct platform_driver d4_pwm_driver = { .driver = {
	.name = "drime4-pwm",
	.owner = THIS_MODULE,
}, .probe = d4_pwm_probe, .remove = __devexit_p(d4_pwm_remove),
		.suspend = d4_pwm_suspend, .resume = d4_pwm_resume, };

static int __init d4_pwm_init(void)
{
	int ret;
	ret = platform_driver_register(&d4_pwm_driver);
	ext0_base = NULL;
	ext1_base = NULL;
	if (ret)
	    printk(KERN_ERR "%s: failed to add pwm driver\n", __func__);

	return ret;
}

arch_initcall(d4_pwm_init);

static void __exit d4_pwm_exit(void)
{
	platform_driver_unregister(&d4_pwm_driver);
}

module_exit(d4_pwm_exit);

MODULE_AUTHOR("Kyuchun han, <kyuchun.han@samsung.com>");
MODULE_DESCRIPTION("drime4 Hardware Trigger PWM Controller Driver");
MODULE_LICENSE("GPL");

