/*
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * Authors:
 *	Seung-Woo Kim <sw0312.kim@samsung.com>
 *	Inki Dae <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "drmP.h"

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>


#include "drime4_drm_drv.h"
#include "drime4_hdmi.h"

static int d4_ddc_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
	hdmi_attach_ddc_client(client);

	dev_info(&client->adapter->dev, "attached d4_ddc "
		"into i2c adapter successfully\n");

	return 0;
}

static int d4_ddc_remove(struct i2c_client *client)
{
	dev_info(&client->adapter->dev, "detached d4_ddc "
		"from i2c adapter successfully\n");

	return 0;
}

static struct i2c_device_id ddc_idtable[] = {
	{"d4_ddc", 0},
	{ },
};

struct i2c_driver ddc_driver = {
	.driver = {
		.name = "d4_ddc",
		.owner = THIS_MODULE,
	},
	.id_table	= ddc_idtable,
	.probe		= d4_ddc_probe,
	.remove		= __devexit_p(d4_ddc_remove),
	.command		= NULL,
};
