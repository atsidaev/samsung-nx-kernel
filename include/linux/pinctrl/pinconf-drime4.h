/*
 * Interface the drime4's specific pinconfig portions of the pinctrl subsystem
 *
 * Copyright (C) 2011 Samsung Electronics.
 *
 * This interface is used in the drime4's specific config to keep track of pins.
 *
 * Author: Chanho Park <chanho61.park@samsung.com>
 *
 * License terms: GNU General Public License (GPL) version 2
 */
#ifndef __LINUX_PINCTRL_PINCONF_DRIME4_H
#define __LINUX_PINCTRL_PINCONF_DRIME4_H

/**
 * enum pin_config_param - possible pin configuration parameters
 * @PIN_CONFIG_INPUT: the pin enable/disable input, if you want to disable
 *	input this pin, use PIN_CONFIG_INPUT_DISABLE, otherwise use
 *	PIN_CONFIG_INPUT_ENABLE.
 * @PIN_CONFIG_INPUT_MODE : select a input modes of the pin.
 *	PIN_CONFIG_INPUT_MODE_[CMOS|SCHEMITT]
 * @PIN_CONFIG_SLEW_RATE : select a slew rate of the pin.
 *	PIN_CONFIG_SLEW_RATE_[FAST|SLOW]
 * @PIN_CONFIG_PULLUP_DOWN : enable pull up/down of the pin.
 *	PIN_CONFIG_PULLUP_DOWN_[ENABLE|DISABLE]
 * @PIN_CONFIG_PULLUP_DOWN_SEL : select the pin is whether pullup or pulldown.
 *	PIN_CONFIG_PULL[UP|DOWN]_SEL
 * @PIN_CONFIG_DRIVE_STRENGTH : control the drive strength of the pin.
 *	PIN_CONFIG_DRIVE_STRENGTH_X[1|2|4|6]
 * @PIN_CONFIG_MODE_CTRL_I2S_GPIO : disconnect or connect signals between
 *	i2s2 ch2 and gpio.
 * @PIN_CONFIG_MODE_CTRL_I2S_HDMI : disconnect or connect signals between
 *	i2s2 ch2 and hdmi.
 * @PIN_CONFIG_MODE_CTRL_SLCD_RESET : reset signal of the SLCD.
 * @PIN_CONFIG_MODE_CTRL_MLCD_RESET : reset signal of the MLCD.
 * @PIN_CONFIG_MODE_CTRL_CODEC_SRP_SEL : select codec or srp.
 * @PIN_CONFIG_VSEL18_INIT_VAL : inital count value for vsel18 counter
 * @PIN_CONFIG_VSEL18_COUNT : vsel18 counter
 * @PIN_CONFIG_VSEL18_CTRL_SD1_CLK_INV : enable or disable inversion of sd1 clk.
 * @PIN_CONFIG_VSEL18_CTRL_SD0_SEL : select a source of vsel18 for sd0.
 * @PIN_CONFIG_VSEL18_CTRL_SD0_CPU : vsel18 for sd0 by cpu.
 * @PIN_CONFIG_VSEL18_CTRL_SD1 : vsel18 for sd1.
 * @PIN_CONFIG_VSEL18_CTRL_SD0 : vsel18 for sd0 by VOLT signal.
 * @PIN_CONFIG_VSEL18_CTRL_ATA : vsel18 for ATA.
 * @PIN_CONFIG_VSEL18_CTRL_SLCD : vsel18 for SLCD.
 * @PIN_CONFIG_VSEL18_CTRL_MLCD : vsel18 for MLCD.
 * @PIN_CONFIG_SYS_DET_USB : USB_DET Pad Signal Level.
 * @PIN_CONFIG_SYS_DET_SD1 : SD1_CDX Pad Signal Level.
 * @PIN_CONFIG_SYS_DET_SD0 : SD0_CDX Pad Signal Level.
 * @PIN_CONFIG_END: this is the last enumerator for pin configurations, if
 *	you need to pass in custom configurations to the pin controller, use
 *	PIN_CONFIG_END+1 as the base offset.
 */
enum pin_config_param {
	PIN_CONFIG_INPUT,
	PIN_CONFIG_INPUT_MODE,
	PIN_CONFIG_SLEW_RATE,
	PIN_CONFIG_PULLUP_DOWN,
	PIN_CONFIG_PULLUP_DOWN_SEL,
	PIN_CONFIG_DRIVE_STRENGTH,
	PIN_CONFIG_MODE_CTRL_I2S_GPIO,
	PIN_CONFIG_MODE_CTRL_I2S_HDMI,
	PIN_CONFIG_MODE_CTRL_SLCD_RESET,
	PIN_CONFIG_MODE_CTRL_MLCD_RESET,
	PIN_CONFIG_MODE_CTRL_CODEC_SRP_SEL,
	PIN_CONFIG_VSEL18_INIT_VAL,
	PIN_CONFIG_VSEL18_COUNT,
	PIN_CONFIG_VSEL18_CTRL_SD1_CLK_INV,
	PIN_CONFIG_VSEL18_CTRL_SD0_SEL,
	PIN_CONFIG_VSEL18_CTRL_SD0_CPU,
	PIN_CONFIG_VSEL18_CTRL_SD1,
	PIN_CONFIG_VSEL18_CTRL_SD0,
	PIN_CONFIG_VSEL18_CTRL_ATA,
	PIN_CONFIG_VSEL18_CTRL_SLCD,
	PIN_CONFIG_VSEL18_CTRL_MLCD,
	PIN_CONFIG_SYS_DET_USB,
	PIN_CONFIG_SYS_DET_SD1,
	PIN_CONFIG_SYS_DET_SD0,
	PIN_CONFIG_END,
};

enum pin_config_input {
	PIN_CONFIG_INPUT_DISABLE = 0,
	PIN_CONFIG_INPUT_ENABLE = 1,
};

enum pin_config_input_mode {
	PIN_CONFIG_INPUT_MODE_CMOS = 0,
	PIN_CONFIG_INPUT_MODE_SCHEMITT = 1,
};

enum pin_config_slew_rate {
	PIN_CONFIG_SLEW_RATE_FAST = 0,
	PIN_CONFIG_SLEW_RATE_SLOW = 1,
};

enum pin_config_pullup_down {
	PIN_CONFIG_PULLUP_DOWN_DISABLE = 0,
	PIN_CONFIG_PULLUP_DOWN_ENABLE = 1,
};

enum pin_config_pullup_down_sel {
	PIN_CONFIG_PULLDOWN_SEL = 0,
	PIN_CONFIG_PULLUP_SEL = 1,
};

enum pin_config_drive_strength {
	PIN_CONFIG_DRIVE_STRENGTH_X1 = 0,
	PIN_CONFIG_DRIVE_STRENGTH_X2 = 1,
	PIN_CONFIG_DRIVE_STRENGTH_X4 = 2,
	PIN_CONFIG_DRIVE_STRENGTH_X6 = 3,
};

enum pin_config_mode_ctrl {
	PIN_CONFIG_MODE_CTRL_CONNECT = 0,
	PIN_CONFIG_MODE_CTRL_DISCONNECT = 1,
};

enum pin_config_mode_ctrl_sel {
	PIN_CONFIG_MODE_CTRL_SRP = 0,
	PIN_CONFIG_MODE_CTRL_CODEC = 1,
};

enum pin_config_vsel18_ctrl_sd1_clk_inv {
	PIN_CONFIG_VSEL18_CTRL_SD1_CLK_INV_DISABLE = 0,
	PIN_CONFIG_VSEL18_CTRL_SD1_CLK_INV_ENABLE = 1,
};

enum pin_config_vsel18_ctrl_sd0_sel {
	PIN_CONFIG_VSEL18_CTRL_SD0_BIT3 = 0,
	PIN_CONFIG_VSEL18_CTRL_SD0CPU_BIT8 = 1,
};

/*
 * The following inlines stuffs a configuration parameter and data value
 * into and out of an unsigned long argument, as used by the generic pin config
 * system. We put the parameter in the lower 16 bits and the argument in the
 * upper 16 bits.
 */

static inline enum pin_config_param to_config_param(unsigned long config)
{
	return (enum pin_config_param) (config & 0xffffUL);
}

static inline u16 to_config_argument(unsigned long config)
{
	return (enum pin_config_param) ((config >> 16) & 0xffffUL);
}

static inline unsigned long to_config_packed(enum pin_config_param param,
					     u16 argument)
{
	return (argument << 16) | ((unsigned long) param & 0xffffUL);
}

#endif /* __LINUX_PINCTRL_PINCONF_GENERIC_H */
