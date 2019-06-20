#ifndef __TSU6111A_H__
#define __TSU6111A_H__

#define TSU6111A_I2C_DEV_NAME	"tsu6111a"
#define TSU6111A_SLAVE_ADDR	0x25


struct tsu6111a_platform_data {
	const char	*pinmux_name;
	unsigned	sda_gpio;
	unsigned	scl_gpio;
	unsigned	intb;
	int		manual_mode;
	int		(*attach_callback)(int device_type);
	int		(*detach_callback)(int device_type);
};

#endif
