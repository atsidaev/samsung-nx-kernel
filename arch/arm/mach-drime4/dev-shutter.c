#include <linux/platform_device.h>
#include <linux/io.h>

#include <mach/map.h>
#include <mach/gpio.h>
#include <mach/d4_ht_pwm_type.h>

#if defined(CONFIG_SHUTTER_COPAL_AFE3141)
#include <linux/shutter.h>

shutter_pwm_info shutter_pwm_infos[] = {
	{
		.id = 0,
		.shared_gpio_num = GPIO_SH_ACT_IN1,
		.pch_pcon_func_name = "",
		.int_type = MIX_LAST_END,
		.ext_in = EXTINPUT_0,
		.info = {
			.opmode = ONETYPE_PULSE,
			.trigtype = SW_TRIGGER,
			.freq2 = {
				.duty = 0,
				.count = 1,
			},
			.resource = VD_TRIGGER,
			.starttype = RISING_EDGE,
			.endtype = LOW_END,
			.invert = INVERT_OFF,
		},
	},
	{
		.id = 1,
		.shared_gpio_num = GPIO_SH_ACT_IN2,
		.pch_pcon_func_name = "",
		.int_type = MIX_LAST_END,
		.ext_in = EXTINPUT_1,
		.info = {
			.opmode = ONETYPE_PULSE,
			.trigtype = SW_TRIGGER,
			.freq2 = {
				.duty = 0,
				.count = 1,
			},
			.resource = VD_TRIGGER,
			.starttype = RISING_EDGE,
			.endtype = LOW_END,
			.invert = INVERT_OFF,
		},
	},
	{
		.id = 3,
		.shared_gpio_num = GPIO_SHUTTER_MG2_ON,
		.pch_pcon_func_name = "PWM3",
		.int_type = MIX_LAST_END,
		.ext_in = EXTINPUT_3,
		.info = {
			.opmode = ONETYPE_PULSE,
			.trigtype = HW_TRIGGER,
			.freq2 = {
				.duty = 0,
				.count = 1,
			},
			.resource = EXT_TRIGGER,
			.starttype = FALLING_EDGE,
			.endtype = LOW_END,
			.invert = INVERT_OFF,
		},
	},
};
shutter_gpio_info shutter_gpio_infos[] = {
	GPIO_SH_PW_ON,
	GPIO_SHUTTER_MG1_ON,
	GPIO_SHUTTER_MG2_ON,
	GPIO_SH_MOT_IN1,
	GPIO_SH_MOT_IN2,
	GPIO_SH_ACT_IN1,
	GPIO_SH_ACT_IN2,
	GPIO_SH_PI_OUT,
};


static shutter_platform_data shutter_pdata = {
	.n_pwm_infos = ARRAY_SIZE(shutter_pwm_infos),
	.n_gpio_infos = ARRAY_SIZE(shutter_gpio_infos),
	.n_timer 	 = 2,
	.n_pi		 = 1,
	.pwm_info 	 = shutter_pwm_infos,
	.gpio_info 	 = shutter_gpio_infos,
};

struct platform_device shutter_device = {
	.name = "shutter",
	.id = -1,
	.dev = {
		.platform_data = &shutter_pdata,
	},
};
#endif
