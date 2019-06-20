/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 * Authors:
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
#include "drm_crtc_helper.h"

#include <drm/drime4_drm.h>
#include "drime4_drm_drv.h"
#include "drime4_drm_encoder.h"

#define MAX_EDID 512
#define to_drime4_connector(x)	container_of(x, struct drime4_drm_connector,\
				drm_connector)

struct drime4_drm_connector {
	struct drm_connector	drm_connector;
	uint32_t		encoder_id;
	struct drime4_drm_manager *manager;
};

/* convert drime4_video_timings to drm_display_mode */
static inline void
convert_to_display_mode(struct drm_display_mode *mode,
			struct drime4_drm_panel_info *panel)
{
	struct fb_videomode *timing = &panel->timing;
	DRM_DEBUG_KMS("%s\n", __FILE__);

	if(timing == NULL || mode == NULL) {
		return;
	}
		
	mode->clock = timing->pixclock / 1000;
	mode->vrefresh = timing->refresh;

	mode->hdisplay = timing->xres;
	mode->hsync_start = mode->hdisplay + timing->right_margin;
	mode->hsync_end = mode->hsync_start + timing->hsync_len;
	mode->htotal = mode->hsync_end + timing->left_margin;

	mode->vdisplay = timing->yres;
	mode->vsync_start = mode->vdisplay + timing->lower_margin;
	mode->vsync_end = mode->vsync_start + timing->vsync_len;
	mode->vtotal = mode->vsync_end + timing->upper_margin;
	mode->width_mm = panel->width_mm;
	mode->height_mm = panel->height_mm;

	if (timing->vmode & FB_VMODE_INTERLACED)
		mode->flags |= DRM_MODE_FLAG_INTERLACE;

	if (timing->vmode & FB_VMODE_DOUBLE)
		mode->flags |= DRM_MODE_FLAG_DBLSCAN;
}

/* convert drm_display_mode to drime4_video_timings */
static inline void
convert_to_video_timing(struct fb_videomode *timing,
			struct drm_display_mode *mode)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	memset(timing, 0, sizeof(*timing));

	timing->pixclock = mode->clock * 1000;
	timing->refresh = drm_mode_vrefresh(mode);

	timing->xres = mode->hdisplay;
	timing->right_margin = mode->hsync_start - mode->hdisplay;
	timing->hsync_len = mode->hsync_end - mode->hsync_start;
	timing->left_margin = mode->htotal - mode->hsync_end;

	timing->yres = mode->vdisplay;
	timing->lower_margin = mode->vsync_start - mode->vdisplay;
	timing->vsync_len = mode->vsync_end - mode->vsync_start;
	timing->upper_margin = mode->vtotal - mode->vsync_end;

	if (mode->flags & DRM_MODE_FLAG_INTERLACE)
		timing->vmode = FB_VMODE_INTERLACED;
	else
		timing->vmode = FB_VMODE_NONINTERLACED;

	if (mode->flags & DRM_MODE_FLAG_DBLSCAN)
		timing->vmode |= FB_VMODE_DOUBLE;

	if (mode->flags & ((1<<14)|(1<<15)|(1<<16)))
		timing->vmode |= (1<<16);
}

static int drime4_drm_connector_get_modes(struct drm_connector *connector)
{
	struct drime4_drm_connector *drime4_connector =
					to_drime4_connector(connector);
	struct drime4_drm_manager *manager = drime4_connector->manager;
	struct drime4_drm_display_ops *display_ops = manager->display_ops;
	unsigned int count;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (!display_ops) {
		DRM_DEBUG_KMS("display_ops is null.\n");
		return 0;
	}

	/*
	 * if get_edid() exists then get_edid() callback of hdmi side
	 * is called to get edid data through i2c interface else
	 * get timing from the FIMD driver(display controller).
	 *
	 * P.S. in case of lcd panel, count is always 1 if success
	 * because lcd panel has only one mode.
	 */
	if (display_ops->get_edid) {
		int ret;
		void *edid;

		edid = kzalloc(MAX_EDID, GFP_KERNEL);
		if (!edid) {
			DRM_ERROR("failed to allocate edid\n");
			return 0;
		}

		ret = display_ops->get_edid(manager->dev, connector,
						edid, MAX_EDID);
		if (ret < 0) {
			printk(KERN_ALERT "drime4_drm_connector_get_modes : failed to get edid data.\n");
			kfree(connector->display_info.raw_edid);
			connector->display_info.raw_edid = edid;
			return 0;
		}

		drm_mode_connector_update_edid_property(connector, edid);
		count = drm_add_edid_modes(connector, edid);

		kfree(connector->display_info.raw_edid);
		connector->display_info.raw_edid = edid;
	} else {
		struct drm_display_mode *mode = drm_mode_create(connector->dev);
		struct drime4_drm_panel_info *panel;

		if (mode == NULL) {
			return -1;
		}

		if (display_ops->get_panel)
			panel = display_ops->get_panel(manager->dev);
		else {
			drm_mode_destroy(connector->dev, mode);
			return 0;
		}

		convert_to_display_mode(mode, panel);
		connector->display_info.width_mm = mode->width_mm;
		connector->display_info.height_mm = mode->height_mm;

		mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
		drm_mode_set_name(mode);
		drm_mode_probed_add(connector, mode);

		count = 1;
	}

	return count;
}

static int drime4_drm_connector_mode_valid(struct drm_connector *connector,
					    struct drm_display_mode *mode)
{
	struct drime4_drm_connector *drime4_connector =
					to_drime4_connector(connector);
	struct drime4_drm_manager *manager = drime4_connector->manager;
	struct drime4_drm_display_ops *display_ops = manager->display_ops;
	struct fb_videomode timing;
	int ret = MODE_BAD;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	convert_to_video_timing(&timing, mode);

	if (display_ops && display_ops->check_timing)
		if (!display_ops->check_timing(manager->dev, (void *)&timing))
			ret = MODE_OK;

	return ret;
}

struct drm_encoder *drime4_drm_best_encoder(struct drm_connector *connector)
{
	struct drm_device *dev = connector->dev;
	struct drime4_drm_connector *drime4_connector =
					to_drime4_connector(connector);
	struct drm_mode_object *obj;
	struct drm_encoder *encoder;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	obj = drm_mode_object_find(dev, drime4_connector->encoder_id,
				   DRM_MODE_OBJECT_ENCODER);
	if (!obj) {
		DRM_DEBUG_KMS("Unknown ENCODER ID %d\n",
				drime4_connector->encoder_id);
		return NULL;
	}

	encoder = obj_to_encoder(obj);

	return encoder;
}

static struct drm_connector_helper_funcs drime4_connector_helper_funcs = {
	.get_modes	= drime4_drm_connector_get_modes,
	.mode_valid	= drime4_drm_connector_mode_valid,
	.best_encoder	= drime4_drm_best_encoder,
};

static int drime4_drm_connector_fill_modes(struct drm_connector *connector,
				unsigned int max_width, unsigned int max_height)
{
	struct drime4_drm_connector *drime4_connector = to_drime4_connector(connector);
	struct drime4_drm_manager *manager = drime4_connector->manager;
	struct drime4_drm_manager_ops *ops = manager->ops;
	unsigned int width, height;

	width = max_width;
	height = max_height;

	/*
	 * if specific driver want to find desired_mode using maxmum
	 * resolution then get max width and height from that driver.
	 */
	if (ops && ops->get_max_resol)
		ops->get_max_resol(manager->dev, &width, &height);

	return drm_helper_probe_single_connector_modes(connector, width, height);
}

/* get detection status of display device. */
static enum drm_connector_status
drime4_drm_connector_detect(struct drm_connector *connector, bool force)
{
	struct drime4_drm_connector *drime4_connector =
					to_drime4_connector(connector);
	struct drime4_drm_manager *manager = drime4_connector->manager;
	struct drime4_drm_display_ops *display_ops =
					manager->display_ops;
	enum drm_connector_status status = connector_status_disconnected;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (display_ops && display_ops->is_connected) {
		if (display_ops->is_connected(manager->dev))
			status = connector_status_connected;
		else
			status = connector_status_disconnected;
	}

	return status;
}

static void drime4_drm_connector_destroy(struct drm_connector *connector)
{
	struct drime4_drm_connector *drime4_connector =
		to_drime4_connector(connector);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	drm_sysfs_connector_remove(connector);
	drm_connector_cleanup(connector);
	kfree(drime4_connector);
}

static struct drm_connector_funcs drime4_connector_funcs = {
	.dpms		= drm_helper_connector_dpms,
	.fill_modes	= drime4_drm_connector_fill_modes,
	.detect		= drime4_drm_connector_detect,
	.destroy	= drime4_drm_connector_destroy,
};

struct drm_connector *drime4_drm_connector_create(struct drm_device *dev,
						   struct drm_encoder *encoder)
{
	struct drime4_drm_connector *drime4_connector;
	struct drime4_drm_manager *manager = drime4_drm_get_manager(encoder);
	struct drm_connector *connector;
	int type;
	int err;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	drime4_connector = kzalloc(sizeof(*drime4_connector), GFP_KERNEL);
	if (!drime4_connector) {
		DRM_ERROR("failed to allocate connector\n");
		return NULL;
	}

	connector = &drime4_connector->drm_connector;

	switch (manager->display_ops->type) {
	case DRIME4_DISPLAY_TYPE_HDMI:
		type = DRM_MODE_CONNECTOR_HDMIA;
		connector->interlace_allowed = true;
		connector->polled = DRM_CONNECTOR_POLL_HPD;
		break;
	case DRIME4_DISPLAY_TYPE_VIDI:
		type = DRM_MODE_CONNECTOR_VIRTUAL;
		connector->polled = DRM_CONNECTOR_POLL_HPD;
		break;
	case DRIME4_DISPLAY_TYPE_LCD:
		type = DRM_MODE_CONNECTOR_LVDS;
		break;
	default:
		type = DRM_MODE_CONNECTOR_Unknown;
		break;
	}

	drm_connector_init(dev, connector, &drime4_connector_funcs, type);
	drm_connector_helper_add(connector, &drime4_connector_helper_funcs);

	err = drm_sysfs_connector_add(connector);
	if (err)
		goto err_connector;

	drime4_connector->encoder_id = encoder->base.id;
	drime4_connector->manager = manager;
	connector->encoder = encoder;

	err = drm_mode_connector_attach_encoder(connector, encoder);
	if (err) {
		DRM_ERROR("failed to attach a connector to a encoder\n");
		goto err_sysfs;
	}

	DRM_DEBUG_KMS("connector has been created\n");

	return connector;

err_sysfs:
	drm_sysfs_connector_remove(connector);
err_connector:
	drm_connector_cleanup(connector);
	kfree(drime4_connector);
	return NULL;
}
