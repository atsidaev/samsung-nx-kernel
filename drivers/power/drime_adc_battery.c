/*
 *	iPAQ h1930/h1940/rx1950 battery controller driver
 *	Copyright (c) Vasily Khoruzhick
 *	Based on h1940_battery.c by Arnaud Patard
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 */

#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/drime_adc_battery.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <../arch/arm/mach-drime4/include/mach/adc.h>

#define BAT_POLL_INTERVAL		10000 /* ms */
#define JITTER_DELAY			500 /* ms */


struct drime_adc_bat 
{
	struct power_supply			psy;
	struct drime4_adc_client		*client;
	struct drime_adc_bat_pdata		*pdata;
	int							volt_value;
	unsigned int					timestamp;
	int							level;
	int							status;
};

static struct delayed_work g_tBat_work;

static int drime_calc_full_volt(int volt_val, int cur_val, int impedance)
{
	return volt_val + cur_val * impedance / 1000;
}

static int drime_charge_finished(struct drime_adc_bat *bat)
{
	return bat->pdata->gpio_inverted ?
		!gpio_get_value(bat->pdata->gpio_charge_finished) :
		gpio_get_value(bat->pdata->gpio_charge_finished);
}


static enum power_supply_property drime_adc_main_bat_props[] = {
/*
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_EMPTY_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,

*/
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	//POWER_SUPPLY_PROP_CHARGE_FULL,
	//POWER_SUPPLY_PROP_CHARGE_NOW,
	//POWER_SUPPLY_PROP_PRESENT,
	//POWER_SUPPLY_PROP_HEALTH,

	
};

static int drime_adc_bat_get_property(struct power_supply *psy, enum power_supply_property psp, union power_supply_propval *val)
{
	struct drime_adc_bat *bat = container_of(psy, struct drime_adc_bat, psy);

	int new_level;
	int full_volt;
	const struct drime_adc_bat_thresh *lut = bat->pdata->lut_noac;
	unsigned int lut_size = bat->pdata->lut_noac_cnt;

	if (!bat) {
		dev_err(psy->dev, "no battery infos ?!\n");
		return -EINVAL;
	}

	//if (bat->volt_value < 0 || jiffies_to_msecs(jiffies - bat->timestamp) > BAT_POLL_INTERVAL)
	{
		bat->volt_value = drime4_adc_raw_read(bat->client, bat->pdata->volt_channel) * bat->pdata->volt_mult;
		
		//printk("\n>>>>>>>>>>>>>>>>>>>>>> Battery ADC[%d] : %d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", bat->pdata->volt_channel, bat->volt_value);
		//bat->timestamp = jiffies;
	}

/*
	if (bat->cable_plugged && ((bat->pdata->gpio_charge_finished < 0) ||!drime_charge_finished(bat))) 
	{
		lut = bat->pdata->lut_acin;
		lut_size = bat->pdata->lut_acin_cnt;
	}
	new_level = 100000;
	full_volt = drime_calc_full_volt((bat->volt_value / 1000), (bat->cur_value / 1000), bat->pdata->internal_impedance);

	if (full_volt < drime_calc_full_volt(lut->volt, lut->cur,	bat->pdata->internal_impedance)) 
	{
		lut_size--;
		while (lut_size--) {
			int lut_volt1;
			int lut_volt2;

			lut_volt1 = drime_calc_full_volt(lut[0].volt, lut[0].cur,
				bat->pdata->internal_impedance);
			lut_volt2 = drime_calc_full_volt(lut[1].volt, lut[1].cur,
				bat->pdata->internal_impedance);
			if (full_volt < lut_volt1 && full_volt >= lut_volt2) {
				new_level = (lut[1].level +
					(lut[0].level - lut[1].level) *
					(full_volt - lut_volt2) /
					(lut_volt1 - lut_volt2)) * 1000;
				break;
			}
			new_level = lut[1].level * 1000;
			lut++;
		}
	}

	bat->level = new_level;

	*/


	switch (psp) {

	case POWER_SUPPLY_PROP_CAPACITY:
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		val->intval = bat->volt_value;
		return 0;
	default:
		return -EINVAL;
	}
}

static struct drime_adc_bat drime_main_bat = {
	.psy = {
		.name			= "battery",
		.type			= POWER_SUPPLY_TYPE_BATTERY,
		.properties		= drime_adc_main_bat_props,
		.num_properties	= ARRAY_SIZE(drime_adc_main_bat_props),
		.get_property		= drime_adc_bat_get_property,
		.use_for_apm		= 1,
	},
};

static void drime_adc_bat_work(struct work_struct *work)
{
	struct drime_adc_bat *bat = &drime_main_bat;
	int is_charged;
	int is_plugged;
	static int was_plugged;

	is_plugged = power_supply_am_i_supplied(&bat->psy);
	
	if (is_plugged != was_plugged)
	{
		was_plugged = is_plugged;
		if (is_plugged) {
			if (bat->pdata->enable_charger)
				bat->pdata->enable_charger();
			bat->status = POWER_SUPPLY_STATUS_CHARGING;
		} else {
			if (bat->pdata->disable_charger)
				bat->pdata->disable_charger();
			bat->status = POWER_SUPPLY_STATUS_DISCHARGING;
		}
	}
	else
	{
		if ((bat->pdata->gpio_charge_finished >= 0) && is_plugged) {
			is_charged = drime_charge_finished(&drime_main_bat);
			if (is_charged) {
				if (bat->pdata->disable_charger)
					bat->pdata->disable_charger();
				bat->status = POWER_SUPPLY_STATUS_FULL;
			} else {
				if (bat->pdata->enable_charger)
					bat->pdata->enable_charger();
				bat->status = POWER_SUPPLY_STATUS_CHARGING;
			}
		}
	}

	power_supply_changed(&bat->psy);
}

static irqreturn_t drime_adc_bat_charged(int irq, void *dev_id)
{
	schedule_delayed_work(&g_tBat_work,
	msecs_to_jiffies(JITTER_DELAY));
	return IRQ_HANDLED;
}

static int __init drime_adc_bat_probe(struct platform_device *pdev)
{
	struct drime_adc_bat_pdata *pdata = pdev->dev.platform_data;
	struct drime4_adc_client *client;
	int ret;

	client = drime4_adc_register(pdev, 10, 2500, 55);
		
	if (IS_ERR(client)) {
		dev_err(&pdev->dev, "cannot register adc\n");
		return PTR_ERR(client);
	}

	platform_set_drvdata(pdev, client);

	drime_main_bat.client = client;
	drime_main_bat.pdata = pdata;
	//drime_main_bat.volt_value = -1;
	//drime_main_bat.status = POWER_SUPPLY_STATUS_DISCHARGING;

	ret = power_supply_register(&pdev->dev, &drime_main_bat.psy);

	if(ret)
	{
		goto err_reg_main;
	}

	INIT_DELAYED_WORK(&g_tBat_work, drime_adc_bat_work);

	dev_info(&pdev->dev, "successfully loaded\n");

	device_init_wakeup(&pdev->dev, 1);

	/* Schedule timer to check current status */
	schedule_delayed_work(&g_tBat_work, msecs_to_jiffies(JITTER_DELAY));

	return 0;

err_platform:
	if (pdata->gpio_charge_finished >= 0)
		free_irq(gpio_to_irq(pdata->gpio_charge_finished), NULL);
err_irq:
	if (pdata->gpio_charge_finished >= 0)
		gpio_free(pdata->gpio_charge_finished);
err_gpio:

err_reg_backup:
	power_supply_unregister(&drime_main_bat.psy);
err_reg_main:
	return ret;
}

static int drime_adc_bat_remove(struct platform_device *pdev)
{
	struct drime4_adc_client *client = platform_get_drvdata(pdev);
	struct drime_adc_bat_pdata *pdata = pdev->dev.platform_data;

	power_supply_unregister(&drime_main_bat.psy);

	drime4_adc_release(client);

	if (pdata->gpio_charge_finished >= 0) {
		free_irq(gpio_to_irq(pdata->gpio_charge_finished), NULL);
		gpio_free(pdata->gpio_charge_finished);
	}

	cancel_delayed_work(&g_tBat_work);

	if (pdata->exit)
		pdata->exit();

	return 0;
}

#ifdef CONFIG_PM
static int drime_adc_bat_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct drime_adc_bat_pdata *pdata = pdev->dev.platform_data;

	if (pdata->gpio_charge_finished >= 0) {
		if (device_may_wakeup(&pdev->dev))
			enable_irq_wake(gpio_to_irq(pdata->gpio_charge_finished));
		else {
			disable_irq(gpio_to_irq(pdata->gpio_charge_finished));
			drime_main_bat.pdata->disable_charger();
		}
	}

	return 0;
}

static int drime_adc_bat_resume(struct platform_device *pdev)
{
	struct drime_adc_bat_pdata *pdata = pdev->dev.platform_data;

	if (pdata->gpio_charge_finished >= 0) {
		if (device_may_wakeup(&pdev->dev))
			disable_irq_wake(gpio_to_irq(pdata->gpio_charge_finished));
		else
			enable_irq(gpio_to_irq(pdata->gpio_charge_finished));
	}

	/* Schedule timer to check current status */
	schedule_delayed_work(&g_tBat_work, msecs_to_jiffies(JITTER_DELAY));

	return 0;
}
#else
#define drime_adc_bat_suspend NULL
#define drime_adc_bat_resume NULL
#endif

static struct platform_driver drime_adc_bat_driver = {
	.driver		= {
		.name 	= "d4-adc-battery",
	},
	.probe		= drime_adc_bat_probe,
	.remove		= drime_adc_bat_remove,
	.suspend		= drime_adc_bat_suspend,
	.resume		= drime_adc_bat_resume,
};

static int __init drime_adc_bat_init(void)
{
	return platform_driver_register(&drime_adc_bat_driver);
}

static void __exit drime_adc_bat_exit(void)
{
	platform_driver_unregister(&drime_adc_bat_driver);
}

module_init(drime_adc_bat_init);
module_exit(drime_adc_bat_exit);

