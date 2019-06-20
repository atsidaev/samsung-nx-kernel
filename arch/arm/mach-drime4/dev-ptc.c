/* linux/arch/arm/mach-drime4/dev-ptc.c
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


#define D4_PTC_RESOURCE_SIZE (2)


#define D4_PTC_ADD_OFFSET(_ptc_no) \
	(_ptc_no * 0x100)

#define D4_PTC_NEXT_ADD_OFFSET(_ptc_no) \
	(D4_PTC_ADD_OFFSET(_ptc_no) + 0x100)




#define D4_PTC_RESOURCE(_ptc_no, _add)			\
	(struct resource [D4_PTC_RESOURCE_SIZE]) {	\
		{					\
			.start	= _add + D4_PTC_ADD_OFFSET(_ptc_no),			\
			.end	= _add + D4_PTC_NEXT_ADD_OFFSET(_ptc_no) - 1,	\
			.flags	= IORESOURCE_MEM	\
		},					\
		{					\
			.start	= IRQ_PTC##_ptc_no,			\
			.end	= IRQ_PTC##_ptc_no,			\
			.flags	= IORESOURCE_IRQ	\
		}					\
	}



#define DEFINE_D4_PTC(_ptc_no, _add)			\
	.name		= "drime4-ptc",		\
	.id		= _ptc_no,			\
	.num_resources	= D4_PTC_RESOURCE_SIZE,		\
	.resource	= D4_PTC_RESOURCE(_ptc_no, _add),	\



struct platform_device d4_device_ptc[] = {
	[0] = { DEFINE_D4_PTC(0, DRIME4_PA_PTC) },
	[1] = { DEFINE_D4_PTC(1, DRIME4_PA_PTC) },
};


EXPORT_SYMBOL(d4_device_ptc);

