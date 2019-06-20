/* drime4_drm_dp_lcd.c
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 * Authors:
 *	sejong <sejong55.oh@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include "drmP.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>

#include <drm/drime4_drm.h>

#include "drime4_drm_drv.h"
#include "drime4_drm_fbdev.h"
#include "drime4_drm_crtc.h"
#include "../../../video/drime4/lcd/d4_dp_lcd_dd.h"

#include <mach/dp/d4_dp.h>
#include <video/drime4/d4_dp_type.h>

/*
 * DP is stand for Fully Interactive Mobile Display and
 * as a display controller, it transfers contents drawn on memory
 * to a LCD Panel through Display Interfaces such as RGB or
 * CPU Interface.
 */

#define get_dp_sublcd_context(dev)	platform_get_drvdata(to_platform_device(dev))
extern struct stfb_lcd_pannel sublcd_pannel;

struct dp_sublcd_win_data {
	unsigned int		offset_x;
	unsigned int		offset_y;
	unsigned int		ovl_width;
	unsigned int		ovl_height;
	unsigned int		fb_width;
	unsigned int		fb_height;
	unsigned int		bpp;
	unsigned int		buf_offsize;
	unsigned int		line_size;	/* bytes */
	bool			enabled;
	struct stdp_image_address	img_addr;
};

struct dp_sublcd_context {
	struct drime4_drm_subdrv	subdrv;
	int				irq;
	struct drm_crtc			*crtc;
	struct clk			*bus_clk;
	struct clk			*lcd_clk;
	struct clk			*slcd_clk; /* < mlcd path :GCLKSEL5 bit 10 */
	struct clk			*slcd_sel; /* < mlcd path :GCLKSEL5 bit 11,12 */
	struct clk			*slcd_out_clk;

	struct resource			*regs_res;
	void __iomem			*regs;
	struct dp_sublcd_win_data		win_data[MAX_DP_WINDOW];
	unsigned int			vid_default_win;
	unsigned int			grp_default_win;
	unsigned long			irq_flags;
	bool				suspended;
	struct mutex			lock;

	struct drime4_drm_panel_info *panel;
	unsigned int pannel_buf_h_start;
	unsigned int pannel_inv_dot_clk;
	unsigned int pannel_inv_clk;
	unsigned int pannel_inv_h_sync;
	unsigned int pannel_inv_v_sync;
	enum drime4_lcd_datawidth data_width;
	enum drime4_lcd_type panel_type;
	enum drime4_lcd_dis_seq even_seq;
	enum drime4_lcd_dis_seq odd_seq;
};

static bool dp_sublcd_display_is_connected(struct device *dev)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO. */

	return true;
}

static void *dp_sublcd_get_panel(struct device *dev)
{
	struct dp_sublcd_context *ctx = get_dp_sublcd_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);
	if (sublcd_pannel.timing.total_h_size > 0) {
		ctx->panel->timing.xres = sublcd_pannel.timing.enable_h_end - sublcd_pannel.timing.enable_h_start;
		ctx->panel->timing.yres = sublcd_pannel.timing.enable_v_end - sublcd_pannel.timing.enable_v_start;
		ctx->panel->timing.left_margin = sublcd_pannel.timing.enable_h_start;
		ctx->panel->timing.right_margin = sublcd_pannel.timing.total_h_size
			-(sublcd_pannel.timing.enable_h_end + sublcd_pannel.timing.h_sync_fall) ;
		ctx->panel->timing.upper_margin = sublcd_pannel.timing.enable_v_start;
		ctx->panel->timing.lower_margin = sublcd_pannel.timing.total_v_size - (sublcd_pannel.timing.enable_v_end+1);
		ctx->panel->timing.hsync_len = sublcd_pannel.timing.h_sync_fall;
		ctx->panel->timing.vsync_len = sublcd_pannel.timing.v_sync_fall;
		ctx->panel->timing.refresh = sublcd_pannel.pixclock/(sublcd_pannel.timing.total_v_size*sublcd_pannel.timing.total_h_size);
		ctx->panel->timing.pixclock = sublcd_pannel.pixclock;
	};
	return ctx->panel;
}

static int dp_sublcd_check_timing(struct device *dev, void *timing)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO. */

	return 0;
}

static int dp_sublcd_display_power_on(struct device *dev, int mode)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO */

	return 0;
}

static struct drime4_drm_display_ops dp_sublcd_display_ops = {
	.type = DRIME4_DISPLAY_TYPE_LCD,
	.is_connected = dp_sublcd_display_is_connected,
	.get_panel = dp_sublcd_get_panel,
	.check_timing = dp_sublcd_check_timing,
	.power_on = dp_sublcd_display_power_on,
};

static void dp_sublcd_dpms(struct device *subdrv_dev, int mode)
{
	struct dp_sublcd_context *ctx = get_dp_sublcd_context(subdrv_dev);

	DRM_DEBUG_KMS("%s, %d\n", __FILE__, mode);

	mutex_lock(&ctx->lock);

	switch (mode) {
	case DRM_MODE_DPMS_ON:
		/*
		 * enable dp_lcd hardware only if suspended status.
		 *
		 * P.S. dp_sublcd_dpms function would be called at booting time so
		 * clk_enable could be called double time.
		 */
		if (ctx->suspended)
			pm_runtime_get_sync(subdrv_dev);
		break;
	case DRM_MODE_DPMS_STANDBY:
	case DRM_MODE_DPMS_SUSPEND:
	case DRM_MODE_DPMS_OFF:
		if (!ctx->suspended)
			pm_runtime_put_sync(subdrv_dev);
		break;
	default:
		DRM_DEBUG_KMS("unspecified mode %d\n", mode);
		break;
	}
	mutex_unlock(&ctx->lock);
}

static void dp_sublcd_apply(struct device *subdrv_dev)
{
	struct dp_sublcd_context *ctx = get_dp_sublcd_context(subdrv_dev);
	struct drime4_drm_manager *mgr = ctx->subdrv.manager;
	struct drime4_drm_manager_ops *mgr_ops = mgr->ops;
	struct drime4_drm_overlay_ops *ovl_ops = mgr->overlay_ops;
	struct dp_sublcd_win_data *win_data;
	int i;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	for (i = 0; i < MAX_DP_WINDOW; i++) {
		win_data = &ctx->win_data[i];
		if (win_data->enabled && (ovl_ops && ovl_ops->commit))
			ovl_ops->commit(subdrv_dev, i);
	}

	if (mgr_ops && mgr_ops->commit)
		mgr_ops->commit(subdrv_dev);
}

static void dp_sublcd_commit(struct device *dev)
{
	struct dp_sublcd_context *ctx = get_dp_sublcd_context(dev);
	struct drime4_drm_panel_info *panel = ctx->panel;
	struct stfb_lcd_pannel pannel_info;
	struct stfb_video_info video;
	struct stfb_graphic_info graphic;
	unsigned int multi = 1;

	if (ctx->suspended)
		return;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* pannel information get */
	if (ctx->panel_type == D4_DELTA && ctx->data_width == D4_DATA_8)
		multi = 3;
	pannel_info.timing.h_sync_rise = 0;
	pannel_info.timing.h_sync_fall = panel->timing.hsync_len*multi;
	pannel_info.timing.v_sync_rise = 0;
	pannel_info.timing.v_sync_fall = panel->timing.vsync_len;
	pannel_info.timing.enable_h_start = panel->timing.left_margin*multi;
	pannel_info.timing.enable_h_end = pannel_info.timing.enable_h_start+panel->timing.xres*multi;
	pannel_info.timing.enable_v_start = panel->timing.upper_margin;
	pannel_info.timing.enable_v_end = panel->timing.yres+panel->timing.upper_margin-1;
	pannel_info.timing.total_h_size = (panel->timing.xres+panel->timing.hsync_len+panel->timing.left_margin+
		panel->timing.right_margin)*multi;
	pannel_info.timing.total_v_size = panel->timing.yres+panel->timing.upper_margin+panel->timing.lower_margin;

	pannel_info.timing.buf_read_h_start = ctx->pannel_buf_h_start;
	pannel_info.timing.inv_dot_clk = ctx->pannel_inv_dot_clk;
	pannel_info.timing.inv_enable_clk = ctx->pannel_inv_clk;
	pannel_info.timing.inv_h_sync = ctx->pannel_inv_h_sync;
	pannel_info.timing.inv_v_sync = ctx->pannel_inv_v_sync;

	pannel_info.h_size = panel->timing.xres*multi;
	pannel_info.v_size = panel->timing.yres;
	pannel_info.lcd_data_width = ctx->data_width;
	pannel_info.type = ctx->panel_type;
	pannel_info.even_seq = ctx->even_seq;
	pannel_info.odd_seq = ctx->odd_seq;
	/* dp lcd control initialize  */
	video.win = 0;
	video.format = DP_YUV_420;
	video.bit = vid8bit;
	video.vid_stride = panel->timing.xres;
	video.image.image_width = panel->timing.xres;
	video.image.image_height = panel->timing.yres;

	video.display.H_Start = 0;
	video.display.H_Size = panel->timing.xres;
	video.display.V_Start = 0;
	video.display.V_Size = panel->timing.yres;
	video.address.y0_address = 0;
	video.address.c0_address = 0;

	graphic.win = 0;
	graphic.grp_stride = (panel->timing.xres*4);
	graphic.image.image_width = panel->timing.xres;
	graphic.image.image_height = panel->timing.yres;
	graphic.display.H_Start = 0;
	graphic.display.H_Size = panel->timing.xres;
	graphic.display.V_Start = 0;
	graphic.display.V_Size = panel->timing.xres;
	graphic.address = 0;

	/* sub lcd initialize*/
	drime4_sublcd_display_init();
	d4_sublcd_pannel_manager_set(&pannel_info);
	d4_sublcd_video_init_display(&video);
}

static int dp_sublcd_enable_vblank(struct device *dev)
{

	DRM_DEBUG_KMS("%s\n", __FILE__);
	d4_dp_sublcd_interrupt_onoff(DP_ON); /* SLCD interrupt enable */

	return 0;
}

static void dp_sublcd_disable_vblank(struct device *dev)
{

	DRM_DEBUG_KMS("%s\n", __FILE__);
	d4_dp_sublcd_interrupt_onoff(DP_OFF); /* SLCD interrupt disable */
}

static struct drime4_drm_manager_ops dp_sublcd_manager_ops = {
	.dpms = dp_sublcd_dpms,
	.apply = dp_sublcd_apply,
	.commit = dp_sublcd_commit,
	.enable_vblank = dp_sublcd_enable_vblank,
	.disable_vblank = dp_sublcd_disable_vblank,
};

static void dp_sublcd_win_mode_set(struct device *dev,
			      struct drime4_drm_overlay *overlay)
{
	struct dp_sublcd_context *ctx = get_dp_sublcd_context(dev);
	struct dp_sublcd_win_data *win_data;
	int win;
	unsigned long offset;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (!overlay) {
		dev_err(dev, "overlay is NULL\n");
		return;
	}

	win = overlay->zpos;
	if (win == DEFAULT_ZPOS)
		win = ctx->grp_default_win;

	if (win < 0 || win > MAX_DP_WINDOW)
		return;

	offset = overlay->fb_x * overlay->pitch;
	offset += overlay->fb_y * overlay->pitch;
	win_data = &ctx->win_data[win];

	win_data->offset_x = overlay->crtc_x;  /* offset x */
	win_data->offset_y = overlay->crtc_y;  /* offset y */
	win_data->ovl_width = overlay->crtc_width;   /* display width */
	win_data->ovl_height = overlay->crtc_height; /* display height */
	win_data->fb_width = overlay->fb_width;
	win_data->fb_height = overlay->fb_height;	 /* image height */
	win_data->img_addr.y0_address = overlay->dma_addr[0] + offset; /* physical address */
	win_data->img_addr.y1_address = overlay->dma_addr[1] + offset; /* physical address */
	win_data->img_addr.c0_address = overlay->dma_addr[2] + offset; /* physical address */
	win_data->img_addr.c1_address = overlay->dma_addr[3] + offset; /* physical address */
	win_data->bpp = overlay->bpp;                /* bit per pixel */
	win_data->buf_offsize = (overlay->fb_width - overlay->crtc_width) *
				(overlay->bpp >> 3); /* image size - display size */
	win_data->line_size = overlay->pitch;/* stride */
}


static void dp_sublcd_win_commit(struct device *dev, int zpos)
{
	struct dp_sublcd_context *ctx = get_dp_sublcd_context(dev);
	struct dp_sublcd_win_data *win_data;
	int win = zpos;
	struct stvideodisplay video;
	struct stgrpdisplay graphic;

	DRM_DEBUG_KMS("%s\n", __FILE__);


	if (ctx->suspended)
		return;

	if (win == DEFAULT_ZPOS)
		win = ctx->grp_default_win;

	if (win < 0 || win > 8) /*0~3:video, 4~7:graphic */
		return;

	win_data = &ctx->win_data[win];

	 /* video */
	if (win < 4) {
		video.win = win;
		video.win_onoff = DP_ON;
		video.format = Ycbcr_420;
		video.bit = _8bit;
		video.address_mode = DP_PHYSICAL_SET;
		video.stride = win_data->line_size;
		video.img_width = win_data->fb_width;
		video.img_height = win_data->fb_height;
		video.display.H_Start = win_data->offset_x;
		video.display.H_Size = win_data->ovl_width;
		video.display.V_Start = win_data->offset_y;
		video.display.V_Size = win_data->ovl_height;
		video.address = win_data->img_addr;

		d4_dp_sublcd_video_set(video);
		d4_dp_sublcd_video_window_onoff(video.win, video.win_onoff);
	} else {
		graphic.win = win - 4;
		graphic.win_onoff = DP_ON;
		graphic.stride = win_data->line_size;
		graphic.img_width = win_data->fb_width;
		graphic.img_height = win_data->fb_height;
		graphic.display.H_Start = win_data->offset_x;
		graphic.display.H_Size = win_data->ovl_width;
		graphic.display.V_Start = win_data->offset_y;
		graphic.display.V_Size = win_data->ovl_height;

		graphic.address_mode = DP_PHYSICAL_SET;
		graphic.address = win_data->img_addr.y0_address;
		d4_dp_sublcd_graphic_set(graphic);
	}
}

static void dp_sublcd_win_disable(struct device *dev, int zpos)
{
	int win = zpos;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* video */
	if (win < 4) {
		d4_dp_lcd_video_window_onoff(win, DP_OFF);
	} else {
		d4_dp_lcd_graphic_window_onoff(win, DP_OFF);
	}
}

static struct drime4_drm_overlay_ops dp_sublcd_overlay_ops = {
	.mode_set = dp_sublcd_win_mode_set,
	.commit = dp_sublcd_win_commit,
	.disable = dp_sublcd_win_disable,
};

static struct drime4_drm_manager dp_sublcd_manager = {
	.pipe		= -1,
	.ops		= &dp_sublcd_manager_ops,
	.overlay_ops	= &dp_sublcd_overlay_ops,
	.display_ops	= &dp_sublcd_display_ops,
};

static void dp_sublcd_finish_pageflip(struct drm_device *drm_dev, int crtc)
{
	struct drime4_drm_private *dev_priv = drm_dev->dev_private;
	struct drm_pending_vblank_event *e, *t;
	struct timeval now;
	unsigned long flags;
	bool is_checked = false;

	spin_lock_irqsave(&drm_dev->event_lock, flags);

	list_for_each_entry_safe(e, t, &dev_priv->pageflip_event_list,
			base.link) {
		/* if event's pipe isn't same as crtc then ignore it. */
		if (crtc != e->pipe)
			continue;

		is_checked = true;

		do_gettimeofday(&now);
		e->event.sequence = 0;
		e->event.tv_sec = now.tv_sec;
		e->event.tv_usec = now.tv_usec;

		list_move_tail(&e->base.link, &e->base.file_priv->event_list);
		wake_up_interruptible(&e->base.file_priv->event_wait);
	}

	if (is_checked) {
		/*
		 * call drm_vblank_put only in case that drm_vblank_get was
		 * called.
		 */
		if (atomic_read(&drm_dev->vblank_refcount[crtc]) > 0)
			drm_vblank_put(drm_dev, crtc);

		/*
		 * don't off vblank if vblank_disable_allowed is 1,
		 * because vblank would be off by timer handler.
		 */
		if (!drm_dev->vblank_disable_allowed)
			drm_vblank_off(drm_dev, crtc);
	}

	spin_unlock_irqrestore(&drm_dev->event_lock, flags);
}



static int dp_sublcd_subdrv_probe(struct drm_device *drm_dev, struct device *dev)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);
	/*
	 * enable drm irq mode.
	 * - with irq_enabled = 1, we can use the vblank feature.
	 *
	 * P.S. note that we wouldn't use drm irq handler but
	 *	just specific driver own one instead because
	 *	drm framework supports only one irq handler.
	 */
	drm_dev->irq_enabled = 1;

	/*
	 * with vblank_disable_allowed = 1, vblank interrupt will be disabled
	 * by drm timer once a current process gives up ownership of
	 * vblank event.(after drm_vblank_put function is called)
	 */
	drm_dev->vblank_disable_allowed = 1;

	return 0;
}

static void dp_sublcd_subdrv_remove(struct drm_device *drm_dev)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO. */
}

static int dp_sublcd_power_on(struct dp_sublcd_context *ctx, bool enable)
{
	struct drime4_drm_subdrv *subdrv = &ctx->subdrv;
	struct device *dev = subdrv->dev;

	DRM_DEBUG_KMS("%s\n", __FILE__);


	if (enable != false && enable != true)
		return -EINVAL;

	if (enable) {
		int ret;

		ret = clk_enable(ctx->bus_clk);
		if (ret < 0)
			return ret;

		ret = clk_enable(ctx->lcd_clk);
		if  (ret < 0) {
			clk_disable(ctx->bus_clk);
			return ret;
		}

		ctx->suspended = false;

		/* if vblank was enabled status, enable it again. */
		if (test_and_clear_bit(0, &ctx->irq_flags))
			dp_sublcd_enable_vblank(dev);

		dp_sublcd_apply(dev);
	} else {
		d4_dp_sublcd_off();
		clk_disable(ctx->lcd_clk);
		clk_disable(ctx->bus_clk);

		ctx->suspended = true;
	}

	return 0;
}


static irqreturn_t drime4_sublcd_irq_frame(int irq, void *dev_id)
{
	struct dp_sublcd_context *ctx = (struct dp_sublcd_context *)dev_id;
	struct drime4_drm_subdrv *subdrv = &ctx->subdrv;
	struct drm_device *drm_dev = subdrv->drm_dev;
	struct drime4_drm_manager *manager = subdrv->manager;

	/* disable_irq_nosync(irq); */

	/* lcd frame interrupt */
	if (dp_slcd_ISR_pending_clear() < 0) {
		printk("sublcd isr not work\n");
		goto out;
	}

	/* check the crtc is detached already from encoder */
	if (manager->pipe < 0)
		goto out;

	drm_handle_vblank(drm_dev, manager->pipe);
	dp_sublcd_finish_pageflip(drm_dev, manager->pipe);
out:
	return IRQ_HANDLED;
}

static int __devinit dp_sublcd_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dp_sublcd_context *ctx;
	struct drime4_drm_subdrv *subdrv;
	struct drime4_drm_dp_lcd_pdata *pdata;
	struct drime4_drm_panel_info *panel;
	struct resource *res;
	int win;
	int ret = -EINVAL;

	struct stfb_lcd_pannel pannel_info;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(dev, "no platform data specified\n");
		return -EINVAL;
	}

	panel = &pdata->panel;
	if (!panel) {
		dev_err(dev, "panel is null.\n");
		return -EINVAL;
	}

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	/* mlcd path set */
	ctx->slcd_clk = clk_get(NULL, "slcd"); /* GCLKSEL5:SLCD_PATHSEL */
	if (IS_ERR(ctx->slcd_clk)) {
			dev_err(dev, "failed to get bus clock\n");
			ret = PTR_ERR(ctx->slcd_clk);
			goto err_clk_get;
	}
	ctx->slcd_out_clk = clk_get(NULL, "slcd_out"); /* GCLKSEL5:SLCD_PATHSEL */
	if (IS_ERR(ctx->slcd_out_clk)) {
			dev_err(dev, "failed to get bus clock\n");
			ret = PTR_ERR(ctx->slcd_out_clk);
			goto err_slcd_clk;
	}
	ctx->slcd_sel = clk_get(NULL, "slcd_sel"); /* GCLKSEL5:SLCD_PATHSEL */
	if (IS_ERR(ctx->slcd_sel)) {
			dev_err(dev, "failed to get bus clock\n");
			ret = PTR_ERR(ctx->slcd_sel);
			goto err_slcd_out_clk;
	}
	clk_enable(ctx->slcd_clk);
	clk_enable(ctx->slcd_out_clk);

	/* slcd clock set */
	ctx->lcd_clk = clk_get(NULL, "slcd_sel1"); /* clock division 13Mhz */
	if (IS_ERR(ctx->lcd_clk)) {
			dev_err(dev, "failed to get bus clock\n");
			ret = PTR_ERR(ctx->lcd_clk);
			goto err_bus_clk;
	}
	clk_enable(ctx->lcd_clk);
	clk_set_rate(ctx->lcd_clk, 13500000); /* lcd clock */


	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region.\n");
		ret = -EINVAL;
		goto err_clk;
	}

	ctx->regs_res = request_mem_region(res->start, resource_size(res),
					   dev_name(dev));
	if (!ctx->regs_res) {
		dev_err(dev, "failed to claim register region\n");
		ret = -ENOENT;
		goto err_clk;
	}

	/* ioremap for register regions. */
	ctx->regs = ioremap(res->start, res->end - res->start + 1);
	if (!ctx->regs) {
		ret = -EINVAL;
		dev_err(&pdev->dev, "failed to remap io region.\n");
		goto err_req_region_io;
	}
	/* lcd interrupt  */
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(dev, "irq request failed.\n");
		goto err_req_region_irq;
	}

	ctx->irq = res->start;
	ret = request_irq(ctx->irq, drime4_sublcd_irq_frame, IRQF_SHARED, SUBLCD_MODULE_NAME, ctx);
	if (ret < 0) {
		dev_err(dev, "irq request failed.\n");
		goto err_req_irq;
	}

	ctx->vid_default_win = pdata->default_vid_win;
	ctx->grp_default_win = pdata->default_grp_win;
	ctx->panel = panel;

	subdrv = &ctx->subdrv;

	subdrv->dev = dev;
	subdrv->manager = &dp_sublcd_manager;
	subdrv->probe = dp_sublcd_subdrv_probe;
	subdrv->remove = dp_sublcd_subdrv_remove;


	/* pannel information get */
	ctx->pannel_buf_h_start = pdata->pannel_buf_h_start;
	ctx->pannel_inv_dot_clk = pdata->pannel_inv_dot_clk;
	ctx->pannel_inv_clk = pdata->pannel_inv_clk;
	ctx->pannel_inv_h_sync = pdata->pannel_inv_h_sync;
	ctx->pannel_inv_v_sync = pdata->pannel_inv_v_sync;
	ctx->data_width = pdata->data_width;
	ctx->panel_type = pdata->panel_type;
	ctx->even_seq = pdata->even_seq;
	ctx->odd_seq = pdata->odd_seq;

	mutex_init(&ctx->lock);
	platform_set_drvdata(pdev, ctx);


	pm_runtime_enable(dev);
	pm_runtime_get_sync(dev);

	drime4_drm_subdrv_register(subdrv);

	return 0;

err_req_irq:
err_req_region_irq:
	iounmap(ctx->regs);

err_req_region_io:
	release_resource(ctx->regs_res);
	kfree(ctx->regs_res);
err_clk:
	clk_disable(ctx->lcd_clk);
	clk_put(ctx->lcd_clk);

err_bus_clk:
	clk_disable(ctx->bus_clk);
	clk_put(ctx->bus_clk);
err_slcd_sel:
	clk_disable(ctx->slcd_sel);
	clk_put(ctx->slcd_sel);

err_slcd_out_clk:
	clk_disable(ctx->slcd_out_clk);
	clk_put(ctx->slcd_out_clk);

err_slcd_clk:
	clk_disable(ctx->slcd_clk);
	clk_put(ctx->slcd_clk);

err_clk_get:
	kfree(ctx);
	return ret;
}

static int __devexit dp_sublcd_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dp_sublcd_context *ctx = platform_get_drvdata(pdev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	drime4_drm_subdrv_unregister(&ctx->subdrv);

	if (ctx->suspended)
		goto out;

	clk_disable(ctx->lcd_clk);
	clk_disable(ctx->bus_clk);
	clk_disable(ctx->slcd_clk);
	clk_disable(ctx->slcd_out_clk);
	clk_disable(ctx->slcd_sel);

	pm_runtime_set_suspended(dev);
	pm_runtime_put_sync(dev);

out:
	pm_runtime_disable(dev);

	clk_put(ctx->lcd_clk);
	clk_put(ctx->bus_clk);
	clk_put(ctx->slcd_clk);
	clk_put(ctx->slcd_out_clk);
	clk_put(ctx->slcd_sel);

	iounmap(ctx->regs);
	release_resource(ctx->regs_res);
	kfree(ctx->regs_res);
	free_irq(ctx->irq, ctx);

	kfree(ctx);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int dp_sublcd_suspend(struct device *dev)
{
	struct dp_sublcd_context *ctx = get_dp_sublcd_context(dev);

	if (pm_runtime_suspended(dev))
		return 0;
	/*
	 * do not use pm_runtime_suspend(). if pm_runtime_suspend() is
	 * called here, an error would be returned by that interface
	 * because the usage_count of pm runtime is more than 1.
	 */
	return dp_sublcd_power_on(ctx, false);
}

static int dp_sublcd_resume(struct device *dev)
{
	struct dp_sublcd_context *ctx = get_dp_sublcd_context(dev);

	/*
	 * if entered to sleep when lcd panel was on, the usage_count
	 * of pm runtime would still be 1 so in this case, dp_lcd driver
	 * should be on directly not drawing on pm runtime interface.
	 */
	if (!pm_runtime_suspended(dev)) {
		if (dp_sublcd_power_on(ctx, true) < 0)
			return -1;
	}

	return 0;
}
#endif

#ifdef CONFIG_PM_RUNTIME
static int dp_sublcd_runtime_suspend(struct device *dev)
{
	struct dp_sublcd_context *ctx = get_dp_sublcd_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);
	return dp_sublcd_power_on(ctx, false);
}

static int dp_sublcd_runtime_resume(struct device *dev)
{
	struct dp_sublcd_context *ctx = get_dp_sublcd_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (dp_sublcd_power_on(ctx, true) < 0)
		return -1;
	return 0;
}
#endif

static const struct dev_pm_ops dp_sublcd_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dp_sublcd_suspend, dp_sublcd_resume)
	SET_RUNTIME_PM_OPS(dp_sublcd_runtime_suspend, dp_sublcd_runtime_resume, NULL)
};

struct platform_driver dp_sublcd_driver = {
	.probe		= dp_sublcd_probe,
	.remove		= __devexit_p(dp_sublcd_remove),
	.driver		= {
		.name	= SUBLCD_MODULE_NAME,
		.owner	= THIS_MODULE,
		.pm	= &dp_sublcd_pm_ops,
	},
};
