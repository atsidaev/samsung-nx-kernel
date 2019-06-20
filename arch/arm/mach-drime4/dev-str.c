#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/str_ctrl.h>

#include <mach/gpio.h>
#include <mach/map.h>
#include <mach/irqs.h>


static struct str_gpio str_gpio_info[] = {
	{
		.name		= "det",
		.port		= (unsigned)-1, /* GPIO_EXT_STR_DET, */
		.dir		= GPIO_DIR_IN,
		.value		= 0,
	},
	{
		.name		= "trig",
		.port		= GPIO_STR_CTL,
		.dir		= GPIO_DIR_OUT,
		.value		= 0,

	},
	{
		.name		= "vccon",
		.port		= (unsigned)-1, /* GPIO_STR_VCC_ON, */
		.dir		= GPIO_DIR_OUT,
		.value		= 0,

	},
};


static struct platform_str_ctrl_data strdev_data = {
	.spio_ch = SPIODEV_CH4,
	.gpios = str_gpio_info ,
	.ngpio = ARRAY_SIZE(str_gpio_info),
};

struct platform_device drime4_str_device = {
	.name			= "str-ctrl",
	.id				= -1,
	.dev = {
		.platform_data = &strdev_data,
	},
};

