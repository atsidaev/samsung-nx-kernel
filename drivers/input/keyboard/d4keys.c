#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/pinctrl/consumer.h>

#include <mach/adc.h>
#include <mach/gpio.h>
#include <mach/d4keys.h>
#include <mach/d4keys_sysfs.h>
#include <mach/version_information.h>

struct port2_data {
	unsigned long Prestatus;
	unsigned char l_status;
	unsigned char r_status;
};

struct d4_button_data {
	struct d4_keys_button *button;
	struct input_dev *input;
	struct timer_list timer;
	struct work_struct work;
	int timer_debounce;	/* in msecs */
	struct port2_data status;
};

struct d4_keys_drvdata {
	struct input_dev *input;
	struct mutex disable_lock;
	unsigned int n_buttons;
	int (*enable)(struct device *dev);
	void (*disable)(struct device *dev);
	struct d4_button_data data[0];
};

static int mask_status = 0;
extern int m_key2;
extern int m_key3;

int d4_keys_update_keymask_state(int keymask_state)
{
	mask_status = keymask_state;
	return 0;
}
static irqreturn_t d4_gpio_keys_port1_isr(int irq, void *dev_id)
{
	struct d4_button_data *data = dev_id;
	struct d4_keys_button *bdata = data->button;

/*	printk("[%s] irq = %d, %d, %d\n", __func__,
		bdata->gpio, bdata->code, gpio_get_value_cansleep(bdata->gpio));*/

	if (data->timer_debounce) {
		mod_timer(&data->timer,
		jiffies + msecs_to_jiffies(data->timer_debounce));
	} else {
		if (gpio_get_value(bdata->gpio) == 1) {
			input_report_key(data->input, bdata->code, 0);
			input_sync(data->input);
		} else {
			input_report_key(data->input, bdata->code, 1);
			input_sync(data->input);
		}
	}
	return IRQ_HANDLED;
}
static void d4_gpio_keys_port1_timer_isr(unsigned long _data)
{
	struct d4_button_data *data = (struct d4_button_data *)_data;
	struct d4_keys_button *bdata = data->button;
	int state;

	state = gpio_get_value(bdata->gpio);

	/* check mask*/
	if (bdata->mask_id & mask_status)
		return;

	if(bdata->code == KEY_POWER && state == 0){
		input_report_key(data->input, KEY_KPEQUAL, 1);
		input_sync(data->input);
		input_report_key(data->input, KEY_KPEQUAL, 0);
		input_sync(data->input);
	}else if(bdata->code == KEY_POWER && state == 1){
		input_report_key(data->input, KEY_POWER, 1);
		input_sync(data->input);
		input_report_key(data->input, KEY_POWER, 0);
		input_sync(data->input);
	}else if (state  == 1) {
		input_report_key(data->input, bdata->code, 0);
		input_sync(data->input);
	} else {
		input_report_key(data->input, bdata->code, 1);
		input_sync(data->input);
	}

#if defined (CONFIG_MACH_D4_NX2000)
	if( state == 0 )
		m_key2 = bdata->code;
	else
		m_key2 = 0;
#elif defined (CONFIG_MACH_D4_NX300)
	if( state == 0 )
		if(m_key2 == KEY_COMPOSE)
			m_key3 = bdata->code;
		else
			m_key3 = 0;
	else
		m_key3 = 0;
#endif

	
}

static unsigned long g_ulWheelCWPrv = 1;
static unsigned char wheel_cw_status = 1;
static unsigned char wheel_ccw_status =1;
static irqreturn_t d4_gpio_keys_wheel_l_isr(int irq, void *dev_id)
{
 	struct d4_button_data *data = dev_id;
	struct d4_keys_button *bdata = data->button;

	unsigned long ulLevelCW;
	unsigned long ulLevelCCW;
	unsigned long ulLevelCCWPrv;
	int i;	

	ulLevelCW = gpio_get_value(GPIO_WHEEL_L);
	ulLevelCCW = gpio_get_value(GPIO_WHEEL_R);

	for (i = 0; i < 0x300; i++)		// 400us
	{
		ulLevelCW = gpio_get_value(GPIO_WHEEL_L);

		if (ulLevelCW == g_ulWheelCWPrv)
		{
			return IRQ_HANDLED;
		}

		ulLevelCCWPrv = ulLevelCCW;
		ulLevelCCW = gpio_get_value(GPIO_WHEEL_R);
		
		if (ulLevelCCW != ulLevelCCWPrv)
		{
			return IRQ_HANDLED;
		}
	}

	g_ulWheelCWPrv = ulLevelCW;

	if( ulLevelCW == 0)
	{
		if(ulLevelCCW == 1)
		{
			wheel_cw_status = 0;
			wheel_ccw_status = 1;
		}
		else
		{
			wheel_cw_status = 1;
			wheel_ccw_status = 0;
		}
	}else{

		if(ulLevelCCW == 0 && wheel_cw_status == 0)
		{
			/* check mask*/
			if (bdata->mask_id & mask_status)
					return IRQ_HANDLED;

			input_report_key(data->input, KEY_RIGHT, 1);
			input_sync(data->input);
			input_report_key(data->input, KEY_RIGHT, 0);
			input_sync(data->input);
			wheel_cw_status = 1;
		}
		else if(ulLevelCCW == 1 && wheel_ccw_status == 0)
		{
			/* check mask*/
			if (bdata->mask_id & mask_status)
					return IRQ_HANDLED;

			input_report_key(data->input, KEY_LEFT, 1);
			input_sync(data->input);
			input_report_key(data->input, KEY_LEFT, 0);
			input_sync(data->input);
			wheel_ccw_status = 1;
		}
	}
	return IRQ_HANDLED;
}

static irqreturn_t d4_gpio_keys_port2_isr(int irq, void *dev_id)
{
 	struct d4_button_data *data = dev_id;
	struct d4_keys_button *bdata = data->button;
 	int ulStatus = 0;
	int i;

/*	printk("[%s] irq = %d, %d, %d\n", __func__,
		data->button->gpio, bdata->code, gpio_get_value(bdata->gpio));*/

	for (i = 0; i < 0x55; i++){
		ulStatus = gpio_get_value(bdata->gpio);

		if (ulStatus == data->status.Prestatus)
			return 0;
	}
	data->status.Prestatus = ulStatus;
 
	 if( ulStatus == 1){
		ulStatus = gpio_get_value(bdata->gpio2nd);
		if(ulStatus == 0){
			data->status.l_status = 1;
			data->status.r_status = 0;
		}else{
			data->status.l_status = 0;
			data->status.r_status = 1;
		}
	}else{
		ulStatus = gpio_get_value(bdata->gpio2nd);
		if(ulStatus == 1 && data->status.l_status == 1){
			/* check mask*/
			if (bdata->mask_id & mask_status)
					return IRQ_HANDLED;

			input_report_key(data->input, bdata->code2nd, 1);
			input_sync(data->input);
			input_report_key(data->input, bdata->code2nd, 0);
			input_sync(data->input);
			data->status.l_status = 0;
		}else if(ulStatus == 0 && data->status.r_status == 1){
			/* check mask*/
			if (bdata->mask_id & mask_status)
					return IRQ_HANDLED;

			input_report_key(data->input, bdata->code, 1);
			input_sync(data->input);
			input_report_key(data->input, bdata->code, 0);
			input_sync(data->input);
			data->status.r_status = 0;
		}
	}

	return IRQ_HANDLED;
}

static int __devinit d4_gpio_keys_setup_key(struct platform_device *pdev,
	struct d4_button_data *bdata,
	struct d4_keys_button *button)
{
	char *desc = button->desc ? button->desc : "gpio_keys";
	struct device *dev = &pdev->dev;
	unsigned long irqflags;
	int irq, error;

	setup_timer(&bdata->timer, d4_gpio_keys_port1_timer_isr,
				(unsigned long)bdata);

	error = gpio_request(button->gpio, desc);
	if (error < 0) {
		dev_err(dev, "failed to request GPIO %d, error %d\n",
			button->gpio, error);
		goto fail2;
	}

	error = gpio_direction_input(button->gpio);
	if (error < 0) {
		dev_err(dev, "direction for GPIO %d, error %d\n",
			button->gpio, error);
		goto fail3;
	}
	
	bdata->timer_debounce = button->debounce_interval;

	if(button->keytype == PORT2_TYPE){
		error = gpio_request(button->gpio2nd, desc);
		if (error < 0) {
			dev_err(dev, "failed to request GPIO %d, error %d\n",
				button->gpio2nd, error);
			goto fail2;
		}

		error = gpio_direction_input(button->gpio2nd);
		if (error < 0) {
			dev_err(dev, "direction for GPIO %d, error %d\n",
				button->gpio2nd, error);
			goto fail3;
		}
	}
	
	irq = gpio_to_irq(button->gpio);
	if (irq < 0) {
		error = irq;
		dev_err(dev, "Unable to get irq number for GPIO %d, error %d\n",
			button->gpio, error);
		goto fail3;
	}

	irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
			/* |IRQF_SAMPLE_RANDOM;*/

	if(button->keytype == PORT2_TYPE){
		error = request_irq(irq, d4_gpio_keys_port2_isr, irqflags, desc, bdata);
	}else if(button->keytype == PORT1_TYPE){
		error = request_irq(irq, d4_gpio_keys_port1_isr, irqflags, desc, bdata);
	}else{
		dev_err(dev, "Button Type Error %d, error %d\n",
			button->gpio, error);
	}
	if (error < 0) {
		dev_err(dev, "Unable to claim irq %d; error %d\n",
			irq, error);
		goto fail3;
	}
	return 0;

fail3:
	gpio_free(button->gpio);
fail2:
	return error;
}

static int __init d4keys_probe(struct platform_device *pdev)
{
	struct d4_keys_platform_data *pdata = pdev->dev.platform_data;
	struct d4_keys_drvdata *ddata;
	struct device *dev = &pdev->dev;
	struct input_dev *input;
	int i, error, adc_ch;

	ddata = kzalloc(sizeof(struct d4_keys_drvdata) +
			pdata->nbuttons * sizeof(struct d4_button_data),
			GFP_KERNEL);
	input = input_allocate_device();
	if (!ddata || !input) {
		dev_err(dev, "failed to allocate state\n");
		error = -ENOMEM;
		goto fail1;
	}

	ddata->input = input;
	ddata->n_buttons = pdata->nbuttons;
	ddata->enable = pdata->enable;
	ddata->disable = pdata->disable;

	input->name = pdev->name;
	input->phys = "d4key/input0";

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

    /* Enable auto repeat feature of Linux input subsystem */
	__set_bit(EV_KEY, input->evbit);

	adc_ch = -1;
	/* Gpio Key */
	d4_keys_update_keymask_state(MASK_ALL);

	for (i = 0; i < pdata->nbuttons; i++) {
		struct d4_keys_button *button = &pdata->buttons[i];
		struct d4_button_data *bdata = &ddata->data[i];
		unsigned int type = button->type ?: EV_KEY;
		
		bdata->input = input;
		bdata->button = button;

		if(button->gpio < 0){
			dev_err(dev, "%s - gpio error: %d\n",
			button->desc, button->gpio);
			continue;
		}
		
		pinctrl_request_gpio(button->gpio);

		if(button->keytype == PORT2_TYPE){
			if(button->gpio2nd < 0){
				dev_err(dev, "%s - gpio2nd error: %d\n",
				button->desc, button->gpio2nd);
				continue;
			}
			pinctrl_request_gpio(button->gpio2nd);
		}

		error = d4_gpio_keys_setup_key(pdev, bdata, button);
		if (error)
			goto fail2;

		input_set_capability(input, type, button->code);
		if(button->code2nd)
			input_set_capability(input, type, button->code2nd);
	}

	platform_set_drvdata(pdev, ddata);
	input_set_drvdata(input, ddata);

	error = input_register_device(input);
	if (error) {
		dev_err(dev, "Unable to register input device, error: %d\n",
			error);
	goto fail2;
	}

	for(i = 0; i < pdata->nbuttons; i++){
		int state;
		struct d4_keys_button *button = &pdata->buttons[i];

		if(button->gpio < 0){
			dev_err(dev, "%s - gpio error: %d\n",
			button->desc, button->gpio);
			continue;
		}
		
		state = (gpio_get_value_cansleep(button->gpio) ? 1 : 0 ) ^ button->active_low;
		if(button->code == KEY_POWER && state == 1){
			input_report_key(input, button->code2nd, 1);
			input_sync(input);
			input_report_key(input, button->code2nd, 0);
			input_sync(input);
		}else if(button->code == KEY_POWER && state == 0){
			input_report_key(input, button->code, 1);
			input_sync(input);
			input_report_key(input, button->code, 0);
			input_sync(input);
		}else{
			input_report_key(input, button->code, !!state);
			input_sync(input);
		}
	}
	return 0;

fail2:
	while (--i >= 0) {
		free_irq(gpio_to_irq(pdata->buttons[i].gpio), &ddata->data[i]);

		if (ddata->data[i].timer_debounce)
			del_timer_sync(&ddata->data[i].timer);

		cancel_work_sync(&ddata->data[i].work);
		gpio_free(pdata->buttons[i].gpio);
	}

/*	platform_set_drvdata(pdev, NULL);*/
fail1:
	if(input)
		input_free_device(input);
	if(ddata)
		kfree(ddata);

	return error;
}

static int d4keys_remove(struct platform_device *pdev)
{
	return 0;
}

static int d4keys_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct d4_keys_platform_data *pdata = pdev->dev.platform_data;
	struct d4_keys_drvdata *ddata = platform_get_drvdata(pdev);
	int i;

	if(ddata != NULL){
		for (i = 0; i < pdata->nbuttons; i++) {
			if(pdata->buttons[i].gpio < 0){
				printk("%s - gpio error: %d\n",
					pdata->buttons[i].desc, pdata->buttons[i].gpio);
				continue;
			}

			free_irq(gpio_to_irq(pdata->buttons[i].gpio), &ddata->data[i]);
			if (ddata->data[i].timer_debounce)
				del_timer_sync(&ddata->data[i].timer);

			cancel_work_sync(&ddata->data[i].work);
			gpio_free(pdata->buttons[i].gpio);

			if(pdata->buttons[i].keytype == PORT2_TYPE){
				if(pdata->buttons[i].gpio2nd < 0){
					printk("%s - gpio2nd error: %d\n",
						pdata->buttons[i].desc, pdata->buttons[i].gpio2nd);
					continue;
				}
				gpio_free(pdata->buttons[i].gpio2nd);
			}

		}
	}
	return 0;
}

static int d4keys_resume(struct platform_device *pdev)
{
	struct d4_keys_drvdata *ddata = platform_get_drvdata(pdev);
	struct d4_keys_platform_data *pdata = pdev->dev.platform_data;
	int i;

	d4_keys_update_keymask_state(MASK_POWER);

	for(i = 0; i < pdata->nbuttons; i++){
		struct d4_keys_button *button = &pdata->buttons[i];
		struct d4_button_data *bdata = &ddata->data[i];
		//int state;

		if(button->gpio < 0){
			printk("%s - gpio error: %d\n",
				button->desc, button->gpio);
			continue;
		}

		if(button->keytype == PORT2_TYPE){
			if(button->gpio < 0){
				printk("%s - gpio2nd error: %d\n",
					button->desc, button->gpio2nd);
				continue;
			}
		}

		d4_gpio_keys_setup_key(pdev, bdata, button);
	}

#if 1
	for(i = 0; i < pdata->nbuttons; i++){
		int state;
		struct d4_keys_button *button = &pdata->buttons[i];
		struct d4_button_data *bdata = &ddata->data[i];

		if(bdata == NULL)
			continue;

		if(button->gpio < 0){
			printk(KERN_EMERG"%s - gpio error: %d\n",
			button->desc, button->gpio);
			continue;
		}
		
		state = (gpio_get_value_cansleep(button->gpio) ? 1 : 0 ) ^ button->active_low;
		if(button->code == KEY_POWER/* && state == 1*/){
			input_report_key(bdata->input, button->code2nd, 1);
			input_sync(bdata->input);
			input_report_key(bdata->input, button->code2nd, 0);
			input_sync(bdata->input);
		}/*else if(button->code == KEY_POWER && state == 0){
			input_report_key(bdata->input, button->code, 1);
			input_sync(bdata->input);
			input_report_key(bdata->input, button->code, 0);
			input_sync(bdata->input);
		}*/else{
			input_report_key(bdata->input, button->code, !!state);
			input_sync(bdata->input);
		}
	}

#endif
	return 0;
}

static struct platform_driver	d4keys_driver = {
	.probe		= d4keys_probe,
	.remove		= d4keys_remove,
	.suspend		= d4keys_suspend,
	.resume		= d4keys_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "d4-keys",
	},
};

int __init d4keys_init(void)
{
	return platform_driver_register(&d4keys_driver);
}

void __exit d4keys_exit(void)
{
	platform_driver_unregister(&d4keys_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(d4keys_init);
#else
fast_subsys_initcall(d4keys_init);
#endif
module_exit(d4keys_exit);

