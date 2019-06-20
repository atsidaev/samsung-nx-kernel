#ifndef __EXTERN_STROBE_PWM_CTRL_H__
#define __EXTERN_STROBE_PWM_CTRL_H__

#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/workqueue.h>
#include <linux/timer.h>


enum spiodev_ch {
	SPIODEV_CH0,
	SPIODEV_CH1,
	SPIODEV_CH2,
	SPIODEV_CH3,
	SPIODEV_CH4,
	SPIODEV_CH5,
	SPIODEV_CH6,
	SPIODEV_CH7,
	SPIODEV_MAX,
};

enum gpio_dir {
	GPIO_DIR_IN,
	GPIO_DIR_OUT,
	GPIO_DIR_MAX,
};


struct str_gpio{
	char *name;
	unsigned int port;
	int value;
	enum  gpio_dir dir;
};



struct platform_str_ctrl_data {
	enum spiodev_ch spio_ch;
	struct str_gpio *gpios;
	int ngpio;
};





struct extern_strobe_pwm_info {
	
	const char		*name;
	const char		dev_name[5];
	unsigned int	id;

//	enum ht_pwm_extint			ext_in;
//	enum ht_pwm_int_enable_type	int_type;
//	struct ht_pwm_conf_info		info;
};



#endif /* __EXTERN_STROBE_PWM_CTRL_H__ */

