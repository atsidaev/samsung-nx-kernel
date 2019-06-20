#ifndef __BD9361_H__
#define __BD9361_H__

#define BD9361_I2C_DEV_NAME	"bd9361"
#define BD9361_SLAVE_ADDR	0x29


struct bd9361_platform_data {
	const char	*pinmux_name;
	unsigned	sda_gpio;
	unsigned	scl_gpio;
};

#endif
