#ifndef __SHUTTER_H__
#define __SHUTTER_H__

typedef struct _shutter_gpio_info {
	unsigned	gpio_num;
}shutter_gpio_info;


typedef struct _shutter_pwm_info {
	unsigned int	id;
	unsigned 		shared_gpio_num;
	char*			pch_pcon_func_name;
	enum ht_pwm_extint			ext_in;
	enum ht_pwm_int_enable_type	int_type;
	struct ht_pwm_conf_info		info;
}shutter_pwm_info;

typedef struct _shutter_platform_data {
	shutter_pwm_info	*pwm_info;
	shutter_gpio_info	*gpio_info;
	int n_pwm_infos;
	int n_gpio_infos;
	int n_timer;
	int n_pi;
}shutter_platform_data;
#endif
