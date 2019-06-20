
#ifndef _DRIME4_ADC_BATTERY_H
#define _DRIME4_ADC_BATTERY_H

struct drime_adc_bat_thresh {
	int volt; /* mV */
	int cur; /* mA */
	int level; /* percent */
};

struct drime_adc_bat_pdata {
	int (*init)(void);
	void (*exit)(void);
	void (*enable_charger)(void);
	void (*disable_charger)(void);

	int gpio_charge_finished;
	int gpio_inverted;

	const struct drime_adc_bat_thresh *lut_noac;
	unsigned int lut_noac_cnt;
	const struct drime_adc_bat_thresh *lut_acin;
	unsigned int lut_acin_cnt;

	const unsigned int volt_channel;
	const unsigned int current_channel;
	const unsigned int backup_volt_channel;

	const unsigned int volt_mult;
	const unsigned int current_mult;
	const unsigned int backup_volt_mult;
	const unsigned int internal_impedance;

	const unsigned int backup_volt_max;
	const unsigned int backup_volt_min;
};

#endif
