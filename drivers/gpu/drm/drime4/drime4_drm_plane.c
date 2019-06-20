/*
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * Authors: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "drmP.h"

#include "drime4_drm.h"
#include "drime4_drm_crtc.h"
#include "drime4_drm_drv.h"
#include "drime4_drm_encoder.h"

struct drime4_plane {
	struct drm_plane		base;
	struct drime4_drm_overlay	overlay;
	bool				enabled;
};

static const uint32_t formats[] = {
	DRM_FORMAT_YYCC,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_NV12,
	DRM_FORMAT_NV12M,
	DRM_FORMAT_NV12MT,
};

static int
drime4_update_plane(struct drm_plane *plane, struct drm_crtc *crtc,
		     struct drm_framebuffer *fb, int crtc_x, int crtc_y,
		     unsigned int crtc_w, unsigned int crtc_h,
		     uint32_t src_x, uint32_t src_y,
		     uint32_t src_w, uint32_t src_h)
{
	struct drime4_plane *drime4_plane =
		container_of(plane, struct drime4_plane, base);
	struct drime4_drm_overlay *overlay = &drime4_plane->overlay;
	struct drime4_drm_crtc_pos pos;
	int ret;

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	memset(&pos, 0, sizeof(struct drime4_drm_crtc_pos));
	pos.crtc_x = crtc_x;
	pos.crtc_y = crtc_y;
	pos.crtc_w = crtc_w;
	pos.crtc_h = crtc_h;

	/* considering 16.16 fixed point of source values */
	pos.fb_x = src_x >> 16;
	pos.fb_y = src_y >> 16;
	pos.src_w = src_w >> 16;
	pos.src_h = src_h >> 16;

	ret = drime4_drm_overlay_update(overlay, fb, &crtc->mode, &pos);
	if (ret < 0)
		return ret;

	drime4_drm_fn_encoder(crtc, overlay,
			drime4_drm_encoder_crtc_mode_set);
	drime4_drm_fn_encoder(crtc, &overlay->zpos,
			drime4_drm_encoder_crtc_plane_commit);

	drime4_plane->enabled = true;

	return 0;
}

static int drime4_disable_plane(struct drm_plane *plane)
{
	struct drime4_plane *drime4_plane =
		container_of(plane, struct drime4_plane, base);
	struct drime4_drm_overlay *overlay = &drime4_plane->overlay;

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	if (!drime4_plane->enabled)
		return 0;

	drime4_drm_fn_encoder(plane->crtc, &overlay->zpos,
			drime4_drm_encoder_crtc_disable);

	drime4_plane->enabled = false;
	drime4_plane->overlay.zpos = DEFAULT_ZPOS;

	return 0;
}

static void drime4_plane_destroy(struct drm_plane *plane)
{
	struct drime4_plane *drime4_plane =
		container_of(plane, struct drime4_plane, base);

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	drime4_disable_plane(plane);
	drm_plane_cleanup(plane);
	kfree(drime4_plane);
}

static struct drm_plane_funcs drime4_plane_funcs = {
	.update_plane	= drime4_update_plane,
	.disable_plane	= drime4_disable_plane,
	.destroy	= drime4_plane_destroy,
};

int drime4_plane_init(struct drm_device *dev, unsigned int nr)
{
	struct drime4_plane *drime4_plane;
	uint32_t possible_crtcs;

	drime4_plane = kzalloc(sizeof(struct drime4_plane), GFP_KERNEL);
	if (!drime4_plane)
		return -ENOMEM;

	/* all CRTCs are available */
	possible_crtcs = (1 << MAX_CRTC) - 1;

	drime4_plane->overlay.zpos = DEFAULT_ZPOS;

	return drm_plane_init(dev, &drime4_plane->base, possible_crtcs,
			      &drime4_plane_funcs, formats, ARRAY_SIZE(formats),
			      false);
}

int drime4_plane_set_zpos_ioctl(struct drm_device *dev, void *data,
				struct drm_file *file_priv)
{
	struct drm_drime4_plane_set_zpos *zpos_req = data;
	struct drm_mode_object *obj;
	struct drm_plane *plane;
	struct drime4_plane *drime4_plane;
	int ret = 0;

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	if (!drm_core_check_feature(dev, DRIVER_MODESET))
		return -EINVAL;
#if 0
	if (zpos_req->zpos < 0 || zpos_req->zpos >= MAX_PLANE) {
		if (zpos_req->zpos != DEFAULT_ZPOS) {
			DRM_ERROR("zpos not within limits\n");
			return -EINVAL;
		}
	}
#endif

	mutex_lock(&dev->mode_config.mutex);

	obj = drm_mode_object_find(dev, zpos_req->plane_id,
			DRM_MODE_OBJECT_PLANE);
	if (!obj) {
		DRM_DEBUG_KMS("Unknown plane ID %d\n",
			      zpos_req->plane_id);
		ret = -EINVAL;
		goto out;
	}

	plane = obj_to_plane(obj);
	drime4_plane = container_of(plane, struct drime4_plane, base);

	drime4_plane->overlay.zpos = zpos_req->zpos;

out:
	mutex_unlock(&dev->mode_config.mutex);
	return ret;
}
