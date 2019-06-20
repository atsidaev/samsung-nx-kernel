/* linux/arch/arm/mach-drime4/include/mach/audio.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - AUDIO support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_AUDIO_H
#define __MACH_DRIME4_AUDIO_H

struct drime4_i2s {
/* If the Primary DAI has 5.1 Channels */
#define QUIRK_PRI_6CHAN		(1 << 0)
/* If the I2S block has a Stereo Overlay Channel */
#define QUIRK_SEC_DAI		(1 << 1)
/*
 * If the I2S block has no internal prescalar or MUX (I2SMOD[10] bit)
 * The Machine driver must provide suitably set clock to the I2S block.
 */
#define QUIRK_NO_MUXPSR		(1 << 2)
#define QUIRK_NEED_RSTCLR	(1 << 3)
	/* Quirks of the I2S controller */
	u32 quirks;

	/*
	 * Array of clock names that can be used to generate I2S signals.
	 * Also corresponds to clocks of I2SMOD[10]
	 */
	char **src_clk;
};

/**
 * struct s3c_audio_pdata - common platform data for audio device drivers
 * @cfg_gpio: Callback function to setup mux'ed pins in I2S/PCM/AC97 mode
 */
struct drime4_audio_pdata {
	int (*cfg_gpio)(struct platform_device *);
	union {
		struct drime4_i2s i2s;
	} type;
};
#endif
