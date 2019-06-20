/* drime4_drm_core.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 * Author:
 *	Inki Dae <inki.dae@samsung.com>
 *	Joonyoung Shim <jy0922.shim@samsung.com>
 *	Seung-Woo Kim <sw0312.kim@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "drmP.h"
#include "drime4_drm_drv.h"
#include "drime4_drm_encoder.h"
#include "drime4_drm_connector.h"
#include "drime4_drm_fbdev.h"

static LIST_HEAD(drime4_drm_subdrv_list);
static struct drm_device *drm_dev;

static int drime4_drm_subdrv_probe(struct drm_device *dev,
					struct drime4_drm_subdrv *subdrv)
{
	struct drm_encoder *encoder;
	struct drm_connector *connector;

	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	if (subdrv->probe) {
		int ret;

		/*
		 * this probe callback would be called by sub driver
		 * after setting of all resources to this sub driver,
		 * such as clock, irq and register map are done or by load()
		 * of drime4 drm driver.
		 *
		 * P.S. note that this driver is considered for modularization.
		 */
		ret = subdrv->probe(dev, subdrv->dev);
		if (ret)
			return ret;
	}

	if (!subdrv->manager)
		return 0;

	subdrv->manager->dev = subdrv->dev;

	/* create and initialize a encoder for this sub driver. */
	encoder = drime4_drm_encoder_create(dev, subdrv->manager,
			(1 << MAX_CRTC) - 1);
	if (!encoder) {
		DRM_ERROR("failed to create encoder\n");
		return -EFAULT;
	}

	/*
	 * create and initialize a connector for this sub driver and
	 * attach the encoder created above to the connector.
	 */
	connector = drime4_drm_connector_create(dev, encoder);
	if (!connector) {
		DRM_ERROR("failed to create connector\n");
		encoder->funcs->destroy(encoder);
		return -EFAULT;
	}

	subdrv->encoder = encoder;
	subdrv->connector = connector;

	return 0;
}

static void drime4_drm_subdrv_remove(struct drm_device *dev,
				      struct drime4_drm_subdrv *subdrv)
{
	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	if (subdrv->remove)
		subdrv->remove(dev);

	if (subdrv->encoder) {
		struct drm_encoder *encoder = subdrv->encoder;
		encoder->funcs->destroy(encoder);
		subdrv->encoder = NULL;
	}

	if (subdrv->connector) {
		struct drm_connector *connector = subdrv->connector;
		connector->funcs->destroy(connector);
		subdrv->connector = NULL;
	}
}

int drime4_drm_device_register(struct drm_device *dev)
{
	struct drime4_drm_subdrv *subdrv, *n;
	int err;

	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	if (!dev)
		return -EINVAL;

	drm_dev = dev;

	list_for_each_entry_safe(subdrv, n, &drime4_drm_subdrv_list, list) {
		subdrv->drm_dev = dev;
		err = drime4_drm_subdrv_probe(dev, subdrv);
		if (err) {
			DRM_DEBUG("drime4 drm subdrv probe failed.\n");
			list_del(&subdrv->list);
		}
	}

	return 0;
}
EXPORT_SYMBOL_GPL(drime4_drm_device_register);

int drime4_drm_device_unregister(struct drm_device *dev)
{
	struct drime4_drm_subdrv *subdrv;

	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	if (!dev) {
		WARN(1, "Unexpected drm device unregister!\n");
		return -EINVAL;
	}

	list_for_each_entry(subdrv, &drime4_drm_subdrv_list, list)
		drime4_drm_subdrv_remove(dev, subdrv);

	drm_dev = NULL;

	return 0;
}
EXPORT_SYMBOL_GPL(drime4_drm_device_unregister);

int drime4_drm_subdrv_register(struct drime4_drm_subdrv *subdrv)
{
	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	if (!subdrv)
		return -EINVAL;

	list_add_tail(&subdrv->list, &drime4_drm_subdrv_list);

	return 0;
}
EXPORT_SYMBOL_GPL(drime4_drm_subdrv_register);

int drime4_drm_subdrv_unregister(struct drime4_drm_subdrv *subdrv)
{
	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	if (!subdrv)
		return -EINVAL;

	list_del(&subdrv->list);

	return 0;
}
EXPORT_SYMBOL_GPL(drime4_drm_subdrv_unregister);

int drime4_drm_subdrv_open(struct drm_device *dev, struct drm_file *file)
{
	struct drime4_drm_subdrv *subdrv;
	int ret;

	list_for_each_entry(subdrv, &drime4_drm_subdrv_list, list) {
		if (subdrv->open) {
			ret = subdrv->open(dev, subdrv->dev, file);
			if (ret)
				goto err;
		}
	}

	return 0;

err:
	list_for_each_entry_reverse(subdrv, &subdrv->list, list) {
		if (subdrv->close)
			subdrv->close(dev, subdrv->dev, file);
	}
	return ret;
}
EXPORT_SYMBOL_GPL(drime4_drm_subdrv_open);

void drime4_drm_subdrv_close(struct drm_device *dev, struct drm_file *file)
{
	struct drime4_drm_subdrv *subdrv;

	list_for_each_entry(subdrv, &drime4_drm_subdrv_list, list) {
		if (subdrv->close)
			subdrv->close(dev, subdrv->dev, file);
	}
}
EXPORT_SYMBOL_GPL(drime4_drm_subdrv_close);
