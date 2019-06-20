#ifndef __TSU6712_H__
#define __TSU6712_H__

#define TSU6712_I2C_DEV_NAME	"tsu6712"
#define TSU6712_SLAVE_ADDR	0x25


struct tsu6712_platform_data {
	const char	*pinmux_name;
	unsigned	sda_gpio;
	unsigned	scl_gpio;
	unsigned	intb;
	int		manual_mode;
	int		(*attach_callback)(int device_type);
	int		(*detach_callback)(int device_type);
};

#endif
