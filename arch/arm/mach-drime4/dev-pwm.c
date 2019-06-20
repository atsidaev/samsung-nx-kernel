/* linux/arch/arm/mach-drime4/dev-pwm.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	kyuchun han <kyuchun.han@samsung.com>
 *
 * Base DRIME4 platform device definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include <mach/map.h>



#define D4_PWM_RESOURCE_SIZE (3)
#define D4_PWM_EXTIN_RESOURCE_SIZE (3)

#define D4_PWM_OFFSET(_pwm_no)	\
	((((_pwm_no/8)*8)+(_pwm_no%8)) < 9 ? 0x100*(_pwm_no) : 0x100*(_pwm_no-9)) \

#define D4_PWM_CH_OFFSET(_pwm_no)	\
	((((_pwm_no/8)*8) + (_pwm_no%8)) < 9 ? 0x0000 : 0x1000) \

#define D4_PWM_NEXT_OFFSET(_pwm_no)	\
	((((_pwm_no/8)*8)+(_pwm_no%8)) < 9 ? 0x100*(_pwm_no+1) : 0x100*(_pwm_no-8)) \


#define D4_PWM_ADD_OFFSET(_pwm_no) \
	(D4_PWM_OFFSET(_pwm_no) + D4_PWM_CH_OFFSET(_pwm_no))

#define D4_PWM_NEXT_ADD_OFFSET(_pwm_no) \
	(D4_PWM_CH_OFFSET(_pwm_no) + D4_PWM_NEXT_OFFSET(_pwm_no))


#define D4_PWM_EXTIN_RESOURCE(_pwm_no, _add)			\
	(struct resource [D4_PWM_EXTIN_RESOURCE_SIZE]) {	\
		{					\
			.start	= _add + D4_PWM_ADD_OFFSET(_pwm_no),			\
			.end	= _add + D4_PWM_NEXT_ADD_OFFSET(_pwm_no) - 1,	\
			.flags	= IORESOURCE_MEM	\
		},					\
		{					\
			.start	= _add + D4_PWM_CH_OFFSET(_pwm_no) + 0x900,			\
			.end	= _add + D4_PWM_CH_OFFSET(_pwm_no) + 0x908 - 1,	\
			.flags	= IORESOURCE_MEM	\
		},					\
		{					\
			.start	= IRQ_PWM##_pwm_no,			\
			.end	= IRQ_PWM##_pwm_no,			\
			.flags	= IORESOURCE_IRQ	\
		},					\
	}

#define DEFINE_D4_PWM_EXTIN(_pwm_no, _add)			\
	.name		= "drime4-pwm",		\
	.id		= _pwm_no,			\
	.num_resources	= D4_PWM_EXTIN_RESOURCE_SIZE,		\
	.resource	= D4_PWM_EXTIN_RESOURCE(_pwm_no, _add),	\



struct platform_device d4_device_pwm[] = {
	[0] = { DEFINE_D4_PWM_EXTIN(0, DRIME4_PA_PWM) },
	[1] = { DEFINE_D4_PWM_EXTIN(1, DRIME4_PA_PWM) },
	[2] = { DEFINE_D4_PWM_EXTIN(2, DRIME4_PA_PWM) },
	[3] = { DEFINE_D4_PWM_EXTIN(3, DRIME4_PA_PWM) },
	[4] = { DEFINE_D4_PWM_EXTIN(4, DRIME4_PA_PWM) },
	[5] = { DEFINE_D4_PWM_EXTIN(5, DRIME4_PA_PWM) },
	[6] = { DEFINE_D4_PWM_EXTIN(6, DRIME4_PA_PWM) },
	[7] = { DEFINE_D4_PWM_EXTIN(7, DRIME4_PA_PWM) },
	[8] = { DEFINE_D4_PWM_EXTIN(8, DRIME4_PA_PWM) },
	[9] = { DEFINE_D4_PWM_EXTIN(9, DRIME4_PA_PWM) },
	[10] = { DEFINE_D4_PWM_EXTIN(10, DRIME4_PA_PWM) },
	[11] = { DEFINE_D4_PWM_EXTIN(11, DRIME4_PA_PWM) },
	[12] = { DEFINE_D4_PWM_EXTIN(12, DRIME4_PA_PWM) },
	[13] = { DEFINE_D4_PWM_EXTIN(13, DRIME4_PA_PWM) },
	[14] = { DEFINE_D4_PWM_EXTIN(14, DRIME4_PA_PWM) },
	[15] = { DEFINE_D4_PWM_EXTIN(15, DRIME4_PA_PWM) },
	[16] = { DEFINE_D4_PWM_EXTIN(16, DRIME4_PA_PWM) },
	[17] = { DEFINE_D4_PWM_EXTIN(17, DRIME4_PA_PWM) },
};


EXPORT_SYMBOL(d4_device_pwm);

