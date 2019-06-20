/*
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * Authors:
 *	Inki Dae <inki.dae@samsung.com>
 *	Seung-Woo Kim <sw0312.kim@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "drmP.h"

#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include <drm/drime4_drm.h>

#include "drime4_drm_drv.h"
#include "drime4_drm_hdmi.h"

#include <mach/dp/d4_dp.h>

#define to_context(dev)		platform_get_drvdata(to_platform_device(dev))
#define to_subdrv(dev)		to_context(dev)
#define get_ctx_from_subdrv(subdrv)	container_of(subdrv,\
					struct drm_hdmi_context, subdrv);

/* these callback points shoud be set by specific drivers. */
static struct drime4_hdmi_ops *hdmi_ops;
static struct drime4_tv_ops *tv_ops;

struct drm_hdmi_context {
	struct drime4_drm_subdrv	subdrv;
	struct drime4_drm_hdmi_context	*hdmi_ctx;
	struct drime4_drm_hdmi_context	*tv_ctx;

	bool	enabled[TV_WIN_NR];
};

void drime4_hdmi_ops_register(struct drime4_hdmi_ops *ops)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ops)
		hdmi_ops = ops;
}

void drime4_tv_ops_register(struct drime4_tv_ops *ops)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ops)
		tv_ops = ops;
}

static bool drm_hdmi_is_connected(struct device *dev)
{
	struct drm_hdmi_context *ctx = to_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return false;

	if (hdmi_ops && hdmi_ops->is_connected)
		return hdmi_ops->is_connected(ctx->hdmi_ctx->ctx);

	return false;
}

static int drm_hdmi_get_edid(struct device *dev,
		struct drm_connector *connector, u8 *edid, int len)
{
	struct drm_hdmi_context *ctx = to_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return -1;

	if (hdmi_ops && hdmi_ops->get_edid)
		return hdmi_ops->get_edid(ctx->hdmi_ctx->ctx, connector, edid,
					  len);

	return 0;
}

static int drm_hdmi_check_timing(struct device *dev, void *timing)
{
	struct drm_hdmi_context *ctx = to_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return -1;

	if (hdmi_ops && hdmi_ops->check_timing)
		return hdmi_ops->check_timing(ctx->hdmi_ctx->ctx, timing);

	return 0;
}

static int drm_hdmi_power_on(struct device *dev, int mode)
{
	struct drm_hdmi_context *ctx = to_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return -1;

	if (hdmi_ops && hdmi_ops->power_on)
		return hdmi_ops->power_on(ctx->hdmi_ctx->ctx, mode);

	return 0;
}

static struct drime4_drm_display_ops drm_hdmi_display_ops = {
	.type = DRIME4_DISPLAY_TYPE_HDMI,
	.is_connected = drm_hdmi_is_connected,
	.get_edid = drm_hdmi_get_edid,
	.check_timing = drm_hdmi_check_timing,
	.power_on = drm_hdmi_power_on,
};

static int drm_hdmi_enable_vblank(struct device *subdrv_dev)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);
	struct drime4_drm_subdrv *subdrv;
	struct drime4_drm_manager *manager;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return -1;

	subdrv = &ctx->subdrv;
	manager = subdrv->manager;

	if (tv_ops && tv_ops->enable_vblank)
		return tv_ops->enable_vblank(ctx->tv_ctx->ctx,
						manager->pipe);

	return 0;
}

static void drm_hdmi_disable_vblank(struct device *subdrv_dev)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return -1;

	if (tv_ops && tv_ops->disable_vblank)
		return tv_ops->disable_vblank(ctx->tv_ctx->ctx);
}

static void drm_hdmi_mode_fixup(struct device *subdrv_dev,
				struct drm_connector *connector,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return;

	if (hdmi_ops && hdmi_ops->mode_fixup)
		hdmi_ops->mode_fixup(ctx->hdmi_ctx->ctx, connector, mode,
				     adjusted_mode);
}

static void drm_hdmi_mode_set(struct device *subdrv_dev, void *mode, void *info)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return;

	if (hdmi_ops && hdmi_ops->mode_set)
		hdmi_ops->mode_set(ctx->hdmi_ctx->ctx, mode, info);
}

static void drm_hdmi_get_max_resol(struct device *subdrv_dev,
				unsigned int *width, unsigned int *height)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return;

	if (hdmi_ops && hdmi_ops->get_max_resol)
		hdmi_ops->get_max_resol(ctx->hdmi_ctx->ctx, width, height);
}

static void drm_hdmi_commit(struct device *subdrv_dev)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return;

	if (hdmi_ops && hdmi_ops->commit)
		hdmi_ops->commit(ctx->hdmi_ctx->ctx);
}

static void drm_hdmi_dpms(struct device *subdrv_dev, int mode)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return;

	if (mode == DRM_MODE_DPMS_ON) {
		if (hdmi_ops && hdmi_ops->dpms)
			hdmi_ops->dpms(ctx->hdmi_ctx->ctx, mode);

		msleep(40);

		if (tv_ops && tv_ops->dpms)
			tv_ops->dpms(ctx->tv_ctx->ctx, mode);
	} else {
		if (tv_ops && tv_ops->dpms)
			tv_ops->dpms(ctx->tv_ctx->ctx, mode);

		msleep(40);

		if (hdmi_ops && hdmi_ops->dpms)
			hdmi_ops->dpms(ctx->hdmi_ctx->ctx, mode);
		msleep(40);
	}
}

static void drm_hdmi_apply(struct device *subdrv_dev)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);
	int i;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return;

	for (i = 0; i < TV_WIN_NR; i++) {
		if (!ctx->enabled[i])
			continue;
		if (tv_ops && tv_ops->win_commit)
			tv_ops->win_commit(ctx->tv_ctx->ctx, i);
	}

	if (hdmi_ops && hdmi_ops->commit)
		hdmi_ops->commit(ctx->hdmi_ctx->ctx);
}

static struct drime4_drm_manager_ops drm_hdmi_manager_ops = {
	.dpms = drm_hdmi_dpms,
	.apply = drm_hdmi_apply,
	.enable_vblank = drm_hdmi_enable_vblank,
	.disable_vblank = drm_hdmi_disable_vblank,
	.mode_fixup = drm_hdmi_mode_fixup,
	.mode_set = drm_hdmi_mode_set,
	.get_max_resol = drm_hdmi_get_max_resol,
	.commit = drm_hdmi_commit,
};

static void drm_tv_mode_set(struct device *subdrv_dev,
		struct drime4_drm_overlay *overlay)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return;

	if (tv_ops && tv_ops->win_mode_set)
		tv_ops->win_mode_set(ctx->tv_ctx->ctx, overlay);
}

static void drm_tv_commit(struct device *subdrv_dev, int zpos)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);
	int win = (zpos == DEFAULT_ZPOS) ? TV_DEFAULT_WIN : zpos;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return;

	if (win < 0 || win > TV_WIN_NR) {
		DRM_ERROR("tv window[%d] is wrong\n", win);
		return;
	}

	if (tv_ops && tv_ops->win_commit)
		tv_ops->win_commit(ctx->tv_ctx->ctx, win);

	ctx->enabled[win] = true;
}

static void drm_tv_disable(struct device *subdrv_dev, int zpos)
{
	struct drm_hdmi_context *ctx = to_context(subdrv_dev);
	int win = (zpos == DEFAULT_ZPOS) ? TV_DEFAULT_WIN : zpos;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL)
		return;

	if (win < 0 || win > TV_WIN_NR) {
		DRM_ERROR("tv window[%d] is wrong\n", win);
		return;
	}

	if (tv_ops && tv_ops->win_disable)
		tv_ops->win_disable(ctx->tv_ctx->ctx, win);

	ctx->enabled[win] = false;
}

static struct drime4_drm_overlay_ops drm_hdmi_overlay_ops = {
	.mode_set = drm_tv_mode_set,
	.commit = drm_tv_commit,
	.disable = drm_tv_disable,
};

static struct drime4_drm_manager hdmi_manager = {
	.pipe		= -1,
	.ops		= &drm_hdmi_manager_ops,
	.overlay_ops	= &drm_hdmi_overlay_ops,
	.display_ops	= &drm_hdmi_display_ops,
};

static int hdmi_subdrv_probe(struct drm_device *drm_dev,
		struct device *dev)
{
	struct drime4_drm_subdrv *subdrv = to_subdrv(dev);
	struct drm_hdmi_context *ctx;
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_drm_common_hdmi_pd *pd;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	pd = pdev->dev.platform_data;

	if (!pd) {
		DRM_DEBUG_KMS("platform data is null.\n");
		return -EFAULT;
	}

	if (!pd->hdmi_dev) {
		DRM_DEBUG_KMS("hdmi device is null.\n");
		return -EFAULT;
	}

	if (!pd->tv_dev) {
		DRM_DEBUG_KMS("tv device is null.\n");
		return -EFAULT;
	}

	ctx = get_ctx_from_subdrv(subdrv);

	if (ctx == NULL) {
		DRM_DEBUG_KMS("context is null.\n");
		return -EFAULT;
	}

	ctx->hdmi_ctx = (struct drime4_drm_hdmi_context *)
				to_context(pd->hdmi_dev);
	if (!ctx->hdmi_ctx) {
		DRM_DEBUG_KMS("hdmi context is null.\n");
		return -EFAULT;
	}

	ctx->hdmi_ctx->drm_dev = drm_dev;

	ctx->tv_ctx = (struct drime4_drm_hdmi_context *)
				to_context(pd->tv_dev);
	if (!ctx->tv_ctx) {
		DRM_DEBUG_KMS("tv context is null.\n");
		return -EFAULT;
	}

	ctx->tv_ctx->drm_dev = drm_dev;
	return 0;
}

static int __devinit drime4_drm_hdmi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct drime4_drm_subdrv *subdrv;
	struct drm_hdmi_context *ctx;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		DRM_LOG_KMS("failed to alloc common hdmi context.\n");
		return -ENOMEM;
	}

	subdrv = &ctx->subdrv;

	subdrv->dev = dev;
	subdrv->manager = &hdmi_manager;
	subdrv->probe = hdmi_subdrv_probe;

	platform_set_drvdata(pdev, subdrv);

	drime4_drm_subdrv_register(subdrv);
	return 0;
}

static int __devexit drime4_drm_hdmi_remove(struct platform_device *pdev)
{
	struct drm_hdmi_context *ctx = platform_get_drvdata(pdev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL) {
		return -1;
	}

	drime4_drm_subdrv_unregister(&ctx->subdrv);
	kfree(ctx);

	return 0;
}

struct platform_driver drime4_drm_common_hdmi_driver = {
	.probe		= drime4_drm_hdmi_probe,
	.remove		= __devexit_p(drime4_drm_hdmi_remove),
	.driver		= {
		.name	= "drime4-drm-hdmi",
		.owner	= THIS_MODULE,
	},
};
