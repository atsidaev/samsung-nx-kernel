/* drime4_drm_fbdev.c
 *
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
#include "drm_crtc.h"
#include "drm_fb_helper.h"
#include "drm_crtc_helper.h"

#include "drime4_drm_drv.h"
#include "drime4_drm_fb.h"
#include "drime4_drm_gem.h"

#define MAX_CONNECTOR		4
#define PREFERRED_BPP		32

extern int drime4_fb_dp_ioctl(struct fb_info *info, unsigned int cmd,
		unsigned long arg);

#define to_drime4_fbdev(x)	container_of(x, struct drime4_drm_fbdev,\
				drm_fb_helper)

struct drime4_drm_fbdev {
	struct drm_fb_helper		drm_fb_helper;
	struct drime4_drm_gem_obj	*drime4_gem_obj;
};

static struct fb_ops drime4_drm_fb_ops = {
	.owner		= THIS_MODULE,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_check_var	= drm_fb_helper_check_var,
	.fb_set_par	= drm_fb_helper_set_par,
	.fb_blank	= drm_fb_helper_blank,
	.fb_pan_display	= drm_fb_helper_pan_display,
	.fb_setcmap	= drm_fb_helper_setcmap,
	.fb_ioctl = drime4_fb_dp_ioctl,
};

static int drime4_drm_fbdev_update(struct drm_fb_helper *helper,
				     struct drm_framebuffer *fb)
{
	struct fb_info *fbi = helper->fbdev;
	struct drm_device *dev = helper->dev;
	struct drime4_drm_gem_buf *buffer;
	unsigned int size = fb->width * fb->height * (fb->bits_per_pixel >> 3);
	unsigned long offset;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	drm_fb_helper_fill_fix(fbi, fb->pitches[0], fb->depth);
	drm_fb_helper_fill_var(fbi, helper, fb->width, fb->height);

	/* RGB formats use only one buffer */
	buffer = drime4_drm_fb_buffer(fb, 0);
	if (!buffer) {
		DRM_LOG_KMS("buffer is null.\n");
		return -EFAULT;
	}

	offset = fbi->var.xoffset * (fb->bits_per_pixel >> 3);
	offset += fbi->var.yoffset * fb->pitches[0];

	dev->mode_config.fb_base = (resource_size_t)buffer->dma_addr;
	fbi->screen_base = buffer->kvaddr + offset;
	fbi->fix.smem_start = (unsigned long)(buffer->dma_addr + offset);
	fbi->screen_size = size;
	fbi->fix.smem_len = size;

	return 0;
}

static int drime4_drm_fbdev_create(struct drm_fb_helper *helper,
				    struct drm_fb_helper_surface_size *sizes)
{
	struct drime4_drm_fbdev *drime4_fbdev = to_drime4_fbdev(helper);
	struct drime4_drm_gem_obj *drime4_gem_obj;
	struct drm_device *dev = helper->dev;
	struct fb_info *fbi;
	struct drm_mode_fb_cmd2 mode_cmd = { 0 };
	struct platform_device *pdev = dev->platformdev;
	unsigned long size;
	int ret;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	DRM_DEBUG_KMS("surface width(%d), height(%d) and bpp(%d\n",
			sizes->surface_width, sizes->surface_height,
			sizes->surface_bpp);

	mode_cmd.width = sizes->surface_width;
	mode_cmd.height = sizes->surface_height;
	mode_cmd.pitches[0] = sizes->surface_width * (sizes->surface_bpp >> 3);
	mode_cmd.pixel_format = drm_mode_legacy_fb_format(sizes->surface_bpp,
							  sizes->surface_depth);

	mutex_lock(&dev->struct_mutex);

	fbi = framebuffer_alloc(0, &pdev->dev);
	if (!fbi) {
		DRM_ERROR("failed to allocate fb info.\n");
		ret = -ENOMEM;
		goto out;
	}

	size = mode_cmd.pitches[0] * mode_cmd.height;

	/* 0 means to allocate physically continuous memory */
#if defined(CONFIG_BLK_DEV_INITRD)
	drime4_gem_obj = drime4_drm_gem_create(dev, 0, 800*480*4);
#else
	drime4_gem_obj = drime4_drm_gem_create(dev, 0, 4096);
#endif

	if (IS_ERR(drime4_gem_obj)) {
		ret = PTR_ERR(drime4_gem_obj);
		framebuffer_release(fbi);
		goto out;
	}

	drime4_fbdev->drime4_gem_obj = drime4_gem_obj;

	helper->fb = drime4_drm_framebuffer_init(dev, &mode_cmd,
			&drime4_gem_obj->base);
	if (IS_ERR_OR_NULL(helper->fb)) {
		DRM_ERROR("failed to create drm framebuffer.\n");
		ret = PTR_ERR(helper->fb);
		framebuffer_release(fbi);
		goto out;
	}

	helper->fbdev = fbi;

	fbi->par = helper;
	fbi->flags = FBINFO_FLAG_DEFAULT;
	fbi->fbops = &drime4_drm_fb_ops;

	ret = fb_alloc_cmap(&fbi->cmap, 256, 0);
	if (ret) {
		DRM_ERROR("failed to allocate cmap.\n");
		framebuffer_release(fbi);
		goto out;
	}

	ret = drime4_drm_fbdev_update(helper, helper->fb);
	if (ret < 0) {
		fb_dealloc_cmap(&fbi->cmap);
		framebuffer_release(fbi);
		goto out;
	}

/*
 * if failed, all resources allocated above would be released by
 * drm_mode_config_cleanup() when drm_load() had been called prior
 * to any specific driver such as dp_lcd or hdmi driver.
 */
out:
	mutex_unlock(&dev->struct_mutex);
	return ret;
}

static int drime4_drm_fbdev_probe(struct drm_fb_helper *helper,
				   struct drm_fb_helper_surface_size *sizes)
{
	int ret = 0;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	/*
	 * with !helper->fb, it means that this funcion is called first time
	 * and after that, the helper->fb would be used as clone mode.
	 */
	if (!helper->fb) {
		ret = drime4_drm_fbdev_create(helper, sizes);
		if (ret < 0) {
			DRM_ERROR("failed to create fbdev.\n");
			return ret;
		}

		/*
		 * fb_helper expects a value more than 1 if succeed
		 * because register_framebuffer() should be called.
		 */
		ret = 1;
	}

	return ret;
}

static struct drm_fb_helper_funcs drime4_drm_fb_helper_funcs = {
	.fb_probe =	drime4_drm_fbdev_probe,
};

int drime4_drm_fbdev_init(struct drm_device *dev)
{
	struct drime4_drm_fbdev *fbdev;
	struct drime4_drm_private *private = dev->dev_private;
	struct drm_fb_helper *helper;
	unsigned int num_crtc;
	int ret;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (!dev->mode_config.num_crtc || !dev->mode_config.num_connector)
		return 0;

	fbdev = kzalloc(sizeof(*fbdev), GFP_KERNEL);
	if (!fbdev) {
		DRM_ERROR("failed to allocate drm fbdev.\n");
		return -ENOMEM;
	}

	private->fb_helper = helper = &fbdev->drm_fb_helper;
	helper->funcs = &drime4_drm_fb_helper_funcs;

	num_crtc = dev->mode_config.num_crtc;

	ret = drm_fb_helper_init(dev, helper, num_crtc, MAX_CONNECTOR);
	if (ret < 0) {
		DRM_ERROR("failed to initialize drm fb helper.\n");
		goto err_init;
	}

	ret = drm_fb_helper_single_add_all_connectors(helper);
	if (ret < 0) {
		DRM_ERROR("failed to register drm_fb_helper_connector.\n");
		goto err_setup;

	}

	ret = drm_fb_helper_initial_config(helper, PREFERRED_BPP);
	if (ret < 0) {
		DRM_ERROR("failed to set up hw configuration.\n");
		goto err_setup;
	}

	return 0;

err_setup:
	drm_fb_helper_fini(helper);

err_init:
	private->fb_helper = NULL;
	kfree(fbdev);

	return ret;
}

static void drime4_drm_fbdev_destroy(struct drm_device *dev,
				      struct drm_fb_helper *fb_helper)
{
	struct drm_framebuffer *fb;

	/* release drm framebuffer and real buffer */
	if (fb_helper->fb && fb_helper->fb->funcs) {
		fb = fb_helper->fb;
		if (fb->funcs->destroy)
			fb->funcs->destroy(fb);
	}

	/* release linux framebuffer */
	if (fb_helper->fbdev) {
		struct fb_info *info;
		int ret;

		info = fb_helper->fbdev;
		ret = unregister_framebuffer(info);
		if (ret < 0)
			DRM_DEBUG_KMS("failed unregister_framebuffer()\n");

		if (info->cmap.len)
			fb_dealloc_cmap(&info->cmap);

		framebuffer_release(info);
	}

	drm_fb_helper_fini(fb_helper);
}

void drime4_drm_fbdev_fini(struct drm_device *dev)
{
	struct drime4_drm_private *private = dev->dev_private;
	struct drime4_drm_fbdev *fbdev;

	if (!private || !private->fb_helper)
		return;

	fbdev = to_drime4_fbdev(private->fb_helper);

	if (fbdev->drime4_gem_obj)
		drime4_drm_gem_destroy(fbdev->drime4_gem_obj);

	drime4_drm_fbdev_destroy(dev, private->fb_helper);
	kfree(fbdev);
	private->fb_helper = NULL;
}

void drime4_drm_fbdev_restore_mode(struct drm_device *dev)
{
	struct drime4_drm_private *private = dev->dev_private;

	if (!private || !private->fb_helper)
		return;

	drm_fb_helper_restore_fbdev_mode(private->fb_helper);
}
