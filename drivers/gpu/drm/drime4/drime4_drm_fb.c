/* drime4_drm_fb.c
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
#include "drm_crtc_helper.h"
#include "drm_fb_helper.h"

#include "drime4_drm_drv.h"
#include "drime4_drm_fb.h"
#include "drime4_drm_gem.h"

#define to_drime4_fb(x)	container_of(x, struct drime4_drm_fb, fb)

/*
 * drime4 specific framebuffer structure.
 *
 * @fb: drm framebuffer obejct.
 * @drime4_gem_obj: array of drime4 specific gem object containing a gem object.
 */
struct drime4_drm_fb {
	struct drm_framebuffer		fb;
	struct drime4_drm_gem_obj	*drime4_gem_obj[MAX_FB_BUFFER];
};

static void drime4_drm_fb_destroy(struct drm_framebuffer *fb)
{
	struct drime4_drm_fb *drime4_fb = to_drime4_fb(fb);
	unsigned int i;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	drm_framebuffer_cleanup(fb);

	for (i = 0; i < ARRAY_SIZE(drime4_fb->drime4_gem_obj); i++) {
		struct drm_gem_object *obj;

		if (drime4_fb->drime4_gem_obj[i] == NULL)
			continue;

		obj = &drime4_fb->drime4_gem_obj[i]->base;
		drm_gem_object_unreference_unlocked(obj);
	}

	kfree(drime4_fb);
	drime4_fb = NULL;
}

static int drime4_drm_fb_create_handle(struct drm_framebuffer *fb,
					struct drm_file *file_priv,
					unsigned int *handle)
{
	struct drime4_drm_fb *drime4_fb = to_drime4_fb(fb);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	return drm_gem_handle_create(file_priv,
			&drime4_fb->drime4_gem_obj[0]->base, handle);
}

static int drime4_drm_fb_dirty(struct drm_framebuffer *fb,
				struct drm_file *file_priv, unsigned flags,
				unsigned color, struct drm_clip_rect *clips,
				unsigned num_clips)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO */

	return 0;
}

static struct drm_framebuffer_funcs drime4_drm_fb_funcs = {
	.destroy	= drime4_drm_fb_destroy,
	.create_handle	= drime4_drm_fb_create_handle,
	.dirty		= drime4_drm_fb_dirty,
};

struct drm_framebuffer *
drime4_drm_framebuffer_init(struct drm_device *dev,
			    struct drm_mode_fb_cmd2 *mode_cmd,
			    struct drm_gem_object *obj)
{
	struct drime4_drm_fb *drime4_fb;
	int ret;

	drime4_fb = kzalloc(sizeof(*drime4_fb), GFP_KERNEL);
	if (!drime4_fb) {
		DRM_ERROR("failed to allocate drime4 drm framebuffer\n");
		return ERR_PTR(-ENOMEM);
	}

	ret = drm_framebuffer_init(dev, &drime4_fb->fb, &drime4_drm_fb_funcs);
	if (ret) {
		DRM_ERROR("failed to initialize framebuffer\n");
		kfree(drime4_fb);
		return ERR_PTR(ret);
	}

	drm_helper_mode_fill_fb_struct(&drime4_fb->fb, mode_cmd);
	drime4_fb->drime4_gem_obj[0] = to_drime4_gem_obj(obj);

	return &drime4_fb->fb;
}

static struct drm_framebuffer *
drime4_user_fb_create(struct drm_device *dev, struct drm_file *file_priv,
		      struct drm_mode_fb_cmd2 *mode_cmd)
{
	struct drm_gem_object *obj;
	struct drm_framebuffer *fb;
	struct drime4_drm_fb *drime4_fb;
	int nr;
	int i;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	obj = drm_gem_object_lookup(dev, file_priv, mode_cmd->handles[0]);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object\n");
		return ERR_PTR(-ENOENT);
	}

	fb = drime4_drm_framebuffer_init(dev, mode_cmd, obj);
	if (IS_ERR(fb)) {
		drm_gem_object_unreference_unlocked(obj);
		return fb;
	}

	drime4_fb = to_drime4_fb(fb);
	nr = drime4_drm_format_num_buffers(fb->pixel_format);

	for (i = 1; i < nr; i++) {
		obj = drm_gem_object_lookup(dev, file_priv,
				mode_cmd->handles[i]);
		if (!obj) {
			DRM_ERROR("failed to lookup gem object\n");
			drime4_drm_fb_destroy(fb);
			return ERR_PTR(-ENOENT);
		}

		drime4_fb->drime4_gem_obj[i] = to_drime4_gem_obj(obj);
	}

	return fb;
}

struct drime4_drm_gem_buf *drime4_drm_fb_buffer(struct drm_framebuffer *fb,
						int index)
{
	struct drime4_drm_fb *drime4_fb = to_drime4_fb(fb);
	struct drime4_drm_gem_buf *buffer;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (index >= MAX_FB_BUFFER)
		return NULL;

	buffer = drime4_fb->drime4_gem_obj[index]->buffer;
	if (!buffer)
		return NULL;

	DRM_DEBUG_KMS("vaddr = 0x%lx, dma_addr = 0x%lx\n",
			(unsigned long)buffer->kvaddr,
			(unsigned long)buffer->dma_addr);

	return buffer;
}

static void drime4_drm_output_poll_changed(struct drm_device *dev)
{
	struct drime4_drm_private *private = dev->dev_private;
	struct drm_fb_helper *fb_helper = private->fb_helper;

	if (fb_helper)
		drm_fb_helper_hotplug_event(fb_helper);
}

static const struct drm_mode_config_funcs drime4_drm_mode_config_funcs = {
	.fb_create = drime4_user_fb_create,
	.output_poll_changed = drime4_drm_output_poll_changed,
};

void drime4_drm_mode_config_init(struct drm_device *dev)
{
	dev->mode_config.min_width = 0;
	dev->mode_config.min_height = 0;

	/*
	 * set max width and height as default value(4096x4096).
	 * this value would be used to check framebuffer size limitation
	 * at drm_mode_addfb().
	 */
	dev->mode_config.max_width = 4480;
	dev->mode_config.max_height = 4096;

	dev->mode_config.funcs = &drime4_drm_mode_config_funcs;
}
