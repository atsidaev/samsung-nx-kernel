#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/input-polldev.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>

#include <mach/adc.h>
#include <mach/gpio.h>
#include <mach/d4keys.h>
#include <mach/d4keys_sysfs.h>
#include <mach/version_information.h>

#define DRV_NAME	"d4keys-polled"

#define MAX_ADC_CNT 21
#define MAX_GPIO_CNT 2
#define ADC_DIFF 10 //7
#define CheckGND(x,y) ((x)? x : y)

extern int g_adckey_value;
static int mask_status = 0;


int last_mode = -1;
int m_key1, m_key2, m_key3;

struct d4_keys_button_polled_data {
	int last_key;
	int last_state;
	int count;
	int threshold;
	int can_sleep;
	struct drime4_adc_client *adc_cid;
};


struct d4_keys_polled_dev {
	struct input_polled_dev *poll_dev;
	struct device *dev;
	struct d4_keys_platform_data_polled *pdata;
	struct d4_keys_button_polled_data data[0];
};


extern void uart_save_log(void);

int d4_keys_polled_update_keymask_state(int keymask_state)
{
	mask_status = keymask_state;
	return 0;
}

static int d4_keys_polled_adc_get(struct d4_keys_button_polled *button, int law)
{
	int rtn = -1;
	int i;

	for(i=0; i < button->key_cnt; i++){
		struct adc_info_key *key_info = &button->key_info[i];
		if(((CheckGND(key_info->adc_level , ADC_DIFF) - ADC_DIFF) <= law)
			&& (law <= (key_info->adc_level +ADC_DIFF))){
			rtn = key_info->code;
			return rtn;
		}
	}
	return rtn;
}


static void d4_keys_polled_check_state(struct input_dev *input,
				struct d4_keys_button_polled *button,
				struct d4_keys_button_polled_data *bdata)
{
	int state, key, i;
	static int log_key_count = 0, is_saved_log = 0;

	state = drime4_adc_raw_read(bdata->adc_cid, button->adc_ch);
	key = d4_keys_polled_adc_get(button, state);

	if (button->adc_ch == 1) {
		m_key1 = key;
	} else if (button->adc_ch == 2) {
#if defined (CONFIG_MACH_D4_NX300) 	// Log file key(jog push+mode) chattering
	if(m_key1 == KEY_KP2)
		m_key2 = key;
	else
		m_key2 = 0;
#endif
	}

	if( button->keytype == ADCKEY_TYPE ) { /*key ch*/
		/* check mask*/
		for (i = 0; i <  button->key_cnt; i++) {
			struct adc_info_key *key_info = &button->key_info[i];
			if(key_info->code == key && key != -1)
				if(key_info->mask_id & mask_status)
					return ;

			if(key_info->code == bdata->last_state &&  key == -1)
				if(key_info->mask_id & mask_status)
					return ;
		}

		if(bdata->last_key != key)
			bdata->count = 0;

	//	if(button->adc_ch == 1)
	//		printk(KERN_EMERG"%d, [%x, %x] %x, %d, %d\n",button->adc_ch, key, bdata->last_key,  bdata->last_state, bdata->count, bdata->threshold);

		if (key != bdata->last_state) {
			if (bdata->count < bdata->threshold)
				bdata->count++;
			else {
				if (key != -1) {
					if(bdata->last_state != -1){
						input_event(input, EV_KEY,
							bdata->last_state, 0);
						input_sync(input);
					}
					input_event(input, EV_KEY,
						key, 1);
					input_sync(input);

					g_adckey_value = state;
				} else {
					input_event(input, EV_KEY,
						bdata->last_state, 0);
					input_sync(input);
					g_adckey_value = 0;
				}
				bdata->count = 0;
				bdata->last_state = key;	
				//printk(KERN_EMERG"%d, %x, %x, %d, %d -\n",button->adc_ch, key, bdata->last_state, bdata->count, bdata->threshold);

			}
		}else{
			bdata->count = 0;
		}
		bdata->last_key = key;
		
	} else if( button->keytype == ADCMODE_TYPE ){/*mode ch*/
		if (key != -1 && key != last_mode) {
			if (bdata->count < bdata->threshold)
				bdata->count++;
			else {
				input_event(input, EV_KEY, key, 1);
				input_sync(input);
				input_event(input, EV_KEY, key, 0);
				input_sync(input);

				bdata->count = 0;
				g_boot_mode = last_mode = key;
//				printk("cur mode dial [%d]\n",g_boot_mode);
			}
		}
	}


#if defined (CONFIG_MACH_D4_NX2000) 	// Log file key(jog push+mode) chattering
	if((m_key2 == KEY_KPLEFTPAREN) && (m_key1 == KEY_SEARCH))
	{
		if(log_key_count > 30 && is_saved_log == 0)
		{
			uart_save_log();
			is_saved_log = 1;
		}
		else
		{
			log_key_count++;
		}
	}
	else
	{
		is_saved_log = 0;
		log_key_count = 0;
	}
#elif defined (CONFIG_MACH_D4_NX300) // Log file key(menu+down+wifi) chattering
	if((m_key1 == KEY_KP2) && (m_key2 == KEY_COMPOSE) && (m_key3 == KEY_EMAIL))
	{
		if(log_key_count > 400 && is_saved_log == 0)
		{
			uart_save_log();
			is_saved_log = 1;
		}
		else
		{
			log_key_count++;
		}
	}
	else
	{
		is_saved_log = 0;
		log_key_count = 0;
	}
#endif
}

static void d4_keys_polled_poll(struct input_polled_dev *dev)
{
	struct d4_keys_polled_dev *bdev = dev->private;
	struct d4_keys_platform_data_polled *pdata = bdev->pdata;
	struct input_dev *input = dev->input;
	int i;

	for (i = 0; i < pdata->nbuttons; i++) {
		struct d4_keys_button_polled_data *bdata = &bdev->data[i];
		d4_keys_polled_check_state(input, &pdata->buttons[i],
			bdata);
	}
}

static void d4_keys_polled_open(struct input_polled_dev *dev)
{
	struct d4_keys_polled_dev *bdev = dev->private;
	struct d4_keys_platform_data_polled *pdata = bdev->pdata;

	if (pdata->enable)
		pdata->enable(bdev->dev);
}

static void d4_keys_polled_close(struct input_polled_dev *dev)
{
	struct d4_keys_polled_dev *bdev = dev->private;
	struct d4_keys_platform_data_polled *pdata = bdev->pdata;

	if (pdata->disable)
		pdata->disable(bdev->dev);
}

static int __devinit d4_keys_polled_probe(struct platform_device *pdev)
{
	struct d4_keys_platform_data_polled *pdata = pdev->dev.platform_data;
	struct device *dev = &pdev->dev;
	struct d4_keys_polled_dev *bdev;
	struct input_polled_dev *poll_dev;
	struct input_dev *input;
	int i,j, error;
	int adc_ch = -1;

	if (!pdata || !pdata->poll_interval)
		return -EINVAL;

	bdev = kzalloc(sizeof(struct d4_keys_polled_dev) +
		pdata->nbuttons * sizeof(struct d4_keys_button_polled_data),
		GFP_KERNEL);
	if (!bdev) {
		dev_err(dev, "no memory for private data\n");
		error = -ENOMEM;
		goto err_free_bdev;
	}

	poll_dev = input_allocate_polled_device();
	if (!poll_dev) {
		dev_err(dev, "no memory for polled device\n");
		error = -ENOMEM;
		goto err_free_bdev;
	}

	poll_dev->private = bdev;
	poll_dev->poll = d4_keys_polled_poll;
	poll_dev->poll_interval = pdata->poll_interval;
	poll_dev->open = d4_keys_polled_open;
	poll_dev->close = d4_keys_polled_close;

	input = poll_dev->input;

	input->evbit[0] = BIT(EV_KEY);
	input->name = pdev->name;
	input->phys = DRV_NAME"/input1";
	input->dev.parent = &pdev->dev;

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	d4_keys_polled_update_keymask_state(MASK_ALL);


	for (i = 0; i < pdata->nbuttons; i++) {
		struct d4_keys_button_polled *button = &pdata->buttons[i];
		struct d4_keys_button_polled_data *bdata = &bdev->data[i];
		unsigned int type = button->type ?: EV_KEY;

		if (adc_ch != button->adc_ch) {
			bdata->adc_cid = drime4_adc_register(pdev,
				button->ref_mvolt,
				button->time_margin,
				button->data_diff);

			if ((bdata->adc_cid == -22) || (bdata->adc_cid == -12)) {
				error = -ENOMEM;
				goto err_free_gpio;
			}

			drime4_adc_start(bdata->adc_cid,
				button->adc_ch);
			adc_ch = button->adc_ch;
		} else {
			bdata->adc_cid = NULL;
		}

		bdata->count = 0;
		bdata->last_state = -1;
		bdata->threshold = 1;
		bdata->last_key = -1;
		for (j = 0; j < button->key_cnt; j++){
			struct adc_info_key *key_info = &button->key_info[j];
			input_set_capability(input, EV_KEY, key_info->code);
		}
	}

	bdev->poll_dev = poll_dev;
	bdev->dev = dev;
	bdev->pdata = pdata;
	platform_set_drvdata(pdev, bdev);

	error = input_register_polled_device(poll_dev);
	if (error) {
		dev_err(dev, "unable to register polled device, err=%d\n",
			error);
		goto err_free_gpio;
	}

	/* report initial state of the buttons */
	for (i = 0; i < pdata->nbuttons; i++) {
		struct d4_keys_button_polled *button = &pdata->buttons[i];
		for (j = 0; j < button->key_cnt ; j++) {
			struct adc_info_key *key_info = &button->key_info[j];
			input_event(input, EV_KEY, key_info->code, 0);
			input_sync(input);
		}
	}

	d4keys_create_node(&pdev->dev);

	return 0;

err_free_gpio:
	for (i = 0; i < pdata->nbuttons; i++) {
		struct d4_keys_button_polled *button = &pdata->buttons[i];
		struct d4_keys_button_polled_data *bdata = &bdev->data[i];

		drime4_adc_release(bdata->adc_cid);
	}
	
	input_free_polled_device(poll_dev);

err_free_bdev:
	if(bdev)
		kfree(bdev);

	platform_set_drvdata(pdev, NULL);
	return error;
}

static int __devexit d4_keys_polled_remove(struct platform_device *pdev)
{
	struct d4_keys_polled_dev *bdev = platform_get_drvdata(pdev);
	struct d4_keys_platform_data_polled *pdata = pdev->dev.platform_data;
	int i;

	if(bdev != NULL){
			
		input_unregister_polled_device(bdev->poll_dev);
		input_free_polled_device(bdev->poll_dev);

		kfree(bdev);
		platform_set_drvdata(pdev, NULL);
	}
	return 0;
}

static int d4keys_polled_resume(struct platform_device *pdev)
{
	d4_keys_polled_update_keymask_state(MASK_ALL);
	return 0;
}


static struct platform_driver d4_keys_polled_driver = {
	.probe	= d4_keys_polled_probe,
	.remove	= __devexit_p(d4_keys_polled_remove),
	.resume		= d4keys_polled_resume,
	.driver	= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init d4_keys_polled_init(void)
{
	return platform_driver_register(&d4_keys_polled_driver);
}

static void __exit d4_keys_polled_exit(void)
{
	platform_driver_unregister(&d4_keys_polled_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(d4_keys_polled_init);
#else
fast_subsys_initcall(d4_keys_polled_init);
#endif
module_exit(d4_keys_polled_exit);

