#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/jack.h>

static struct jack_platform_data jack_data = {
	.usb_online		= 0,
	.charger_online		= -1,
	.charger1_online	= 0,
	.hdmi_online		= -1,
	.earjack_online		= -1,
	.earkey_online		= -1,
	.ums_online		= -1,
	.cdrom_online		= -1,
	.jig_online		= -1,
	.release_online	= 0,
	.mmc_online	= 1,	
};

struct platform_device d4_jack = {
	.name			= "jack",
	.id			= -1,
	.dev			= {
		.platform_data	= &jack_data,
	},
};


