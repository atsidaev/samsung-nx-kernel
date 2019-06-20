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

#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinconf-drime4.h>
#endif

/*
 * DP is stand for Fully Interactive Mobile Display and
 * as a display controller, it transfers contents drawn on memory
 * to a LCD Panel through Display Interfaces such as RGB or
 * CPU Interface.
 */

#define get_dp_lcd_context(dev)	platform_get_drvdata(to_platform_device(dev))

struct dp_lcd_context *g_lcd_ctx;
extern int g_is_lcd_interrupt_occur;
extern wait_queue_head_t g_lcd_interrupt_wait;

struct dp_lcd_win_data {
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

struct dp_lcd_context {
	struct drime4_drm_subdrv	subdrv;
	int				irq;
	struct drm_crtc			*crtc;
	struct clk			*bus_clk;
	struct clk			*lcd_clk;
	struct clk			*mlcd_clk; /* < mlcd path :GCLKSEL5 bit 10 */
	struct clk			*mlcd_sel; /* < mlcd path :GCLKSEL5 bit 11,12 */
	struct clk			*mlcd_out_clk;

	struct clk			*slcd_clk;
	struct clk			*slcd_out_clk;

	struct resource			*regs_res;
	void __iomem			*regs;
	struct dp_lcd_win_data		win_data[MAX_DP_WINDOW];
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
#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
	struct pinctrl	*pmx;
	struct pinctrl_state	*pins_default;
#endif
};

extern struct stfb_lcd_pannel lcd_pannel;
extern wait_queue_head_t g_lcd_interrupt_wait;

static bool dp_lcd_display_is_connected(struct device *dev)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO. */

	return true;
}

static void *dp_lcd_get_panel(struct device *dev)
{
	struct dp_lcd_context *ctx = get_dp_lcd_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if(ctx == NULL)
	{
		return NULL;
	}

	if (lcd_pannel.timing.total_h_size > 0) {
		ctx->panel->timing.xres = lcd_pannel.timing.enable_h_end - lcd_pannel.timing.enable_h_start;
		ctx->panel->timing.yres = lcd_pannel.timing.enable_v_end - lcd_pannel.timing.enable_v_start;
		ctx->panel->timing.left_margin = lcd_pannel.timing.enable_h_start;
		ctx->panel->timing.right_margin = lcd_pannel.timing.total_h_size - (lcd_pannel.timing.enable_h_end + lcd_pannel.timing.h_sync_fall);
		ctx->panel->timing.upper_margin = lcd_pannel.timing.enable_v_start;
		ctx->panel->timing.lower_margin = lcd_pannel.timing.total_v_size - (lcd_pannel.timing.enable_v_end);
		ctx->panel->timing.hsync_len = lcd_pannel.timing.h_sync_fall;
		ctx->panel->timing.vsync_len = lcd_pannel.timing.v_sync_fall;
		ctx->panel->timing.refresh = lcd_pannel.pixclock/(lcd_pannel.timing.total_v_size*lcd_pannel.timing.total_h_size);
		ctx->panel->timing.pixclock = lcd_pannel.pixclock;
		ctx->pannel_buf_h_start = lcd_pannel.timing.buf_read_h_start;
		ctx->pannel_inv_dot_clk = lcd_pannel.timing.inv_dot_clk;
		ctx->pannel_inv_clk = lcd_pannel.timing.inv_enable_clk;
		ctx->pannel_inv_h_sync = lcd_pannel.timing.inv_h_sync;
		ctx->pannel_inv_v_sync = lcd_pannel.timing.inv_v_sync;
	};

	return ctx->panel;
}

static int dp_lcd_check_timing(struct device *dev, void *timing)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO. */

	return 0;
}

static int dp_lcd_display_power_on(struct device *dev, int mode)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO */

	return 0;
}

static struct drime4_drm_display_ops dp_lcd_display_ops = {
	.type = DRIME4_DISPLAY_TYPE_LCD,
	.is_connected = dp_lcd_display_is_connected,
	.get_panel = dp_lcd_get_panel,
	.check_timing = dp_lcd_check_timing,
	.power_on = dp_lcd_display_power_on,
};

static void dp_lcd_dpms(struct device *subdrv_dev, int mode)
{
	struct dp_lcd_context *ctx = get_dp_lcd_context(subdrv_dev);

	DRM_DEBUG_KMS("%s, %d\n", __FILE__, mode);

	if(ctx == NULL)
		return;

	mutex_lock(&ctx->lock);

	switch (mode) {
	case DRM_MODE_DPMS_ON:
		/*
		 * enable dp_lcd hardware only if suspended status.
		 *
		 * P.S. dp_lcd_dpms function would be called at booting time so
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

static void dp_lcd_apply(struct device *subdrv_dev)
{
	struct dp_lcd_context *ctx = get_dp_lcd_context(subdrv_dev);
	struct drime4_drm_manager *mgr;
	struct drime4_drm_manager_ops *mgr_ops;
	struct drime4_drm_overlay_ops *ovl_ops;

	if(ctx == NULL)
		return;

	mgr = ctx->subdrv.manager;
	mgr_ops = mgr->ops;
	ovl_ops = mgr->overlay_ops;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (mgr_ops && mgr_ops->commit)
		mgr_ops->commit(subdrv_dev);
}

static void dp_lcd_commit(struct device *dev)
{
	struct dp_lcd_context *ctx = get_dp_lcd_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL || ctx->suspended)
		return;

	/* main lcd initialize*/
	dp_lcd_pannel_set();
	drime4_lcd_display_init();
	d4_lcd_video_init_display();	
}

static int dp_lcd_enable_vblank(struct device *dev)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);
	d4_dp_lcd_interrupt_onoff(DP_ON, DRM_INTERRUPT, 0); /* MLCD interrupt enable */
	return 0;
}

static void dp_lcd_disable_vblank(struct device *dev)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);
	d4_dp_lcd_interrupt_onoff(DP_OFF, DRM_INTERRUPT, 0); /* MLCD interrupt enable */
}

static struct drime4_drm_manager_ops dp_lcd_manager_ops = {
	.dpms = dp_lcd_dpms,
	.apply = dp_lcd_apply,
	.commit = dp_lcd_commit,
	.enable_vblank = dp_lcd_enable_vblank,
	.disable_vblank = dp_lcd_disable_vblank,
};

static void dp_lcd_win_mode_set(struct device *dev,
			      struct drime4_drm_overlay *overlay)
{
	struct dp_lcd_context *ctx = get_dp_lcd_context(dev);
	struct dp_lcd_win_data *win_data;
	int win;
	unsigned long offset;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if(ctx == NULL)
		return;

	if (!overlay) {
		dev_err(dev, "overlay is NULL\n");
		return;
	}

	win = overlay->zpos;
	if (win == DEFAULT_ZPOS)
		win = ctx->grp_default_win;

	if (win < 0 || win > MAX_DP_WINDOW)
		return;

	offset = overlay->fb_x * (overlay->bpp >> 3);
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


static void dp_lcd_win_commit(struct device *dev, int zpos)
{
	struct dp_lcd_context *ctx = get_dp_lcd_context(dev);
	struct dp_lcd_win_data *win_data;
	int win = zpos;
	struct stvideodisplay video;
	struct stgrpdisplay graphic;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (ctx == NULL || ctx->suspended)
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

		d4_dp_lcd_video_set(video);
		d4_dp_lcd_video_window_onoff(video.win, video.win_onoff);
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
		d4_dp_lcd_graphic_set(graphic);
	}
}

static void dp_lcd_win_disable(struct device *dev, int zpos)
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

static struct drime4_drm_overlay_ops dp_lcd_overlay_ops = {
	.mode_set = dp_lcd_win_mode_set,
	.commit = dp_lcd_win_commit,
	.disable = dp_lcd_win_disable,
};

static struct drime4_drm_manager dp_lcd_manager = {
	.pipe		= -1,
	.ops		= &dp_lcd_manager_ops,
	.overlay_ops	= &dp_lcd_overlay_ops,
	.display_ops	= &dp_lcd_display_ops,
};

static void dp_lcd_finish_pageflip(struct drm_device *drm_dev, int crtc)
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



static int dp_lcd_subdrv_probe(struct drm_device *drm_dev, struct device *dev)
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
	drm_dev->vblank_disable_allowed = 0;

	return 0;
}

static void dp_lcd_subdrv_remove(struct drm_device *drm_dev)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO. */
}

static int dp_lcd_power_on(struct dp_lcd_context *ctx, bool enable)
{
	struct drime4_drm_subdrv *subdrv = &ctx->subdrv;
	struct device *dev = subdrv->dev;

	DRM_DEBUG_KMS("%s\n", __FILE__);


	if (enable != false && enable != true)
		return -EINVAL;

	if (enable) {
		int ret;
#ifndef CONFIG_PMU_SELECT
		ret = clk_enable(ctx->bus_clk);
		if (ret < 0)
			return ret;

		/* bus clock */
/*
		clk_set_rate(ctx->bus_clk, 162000000);
*/

		ret = clk_enable(ctx->lcd_clk);
		if  (ret < 0) {
			clk_disable(ctx->bus_clk);
			return ret;
		}
#else
#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
		clk_enable(ctx->slcd_clk);
		clk_enable(ctx->slcd_out_clk);
#endif
		dp_pmu_on_off(DP_OFF);
#endif
		ctx->suspended = false;

		/* if vblank was enabled status, enable it again. */
		if (test_and_clear_bit(0, &ctx->irq_flags))
			dp_lcd_enable_vblank(dev);

		dp_lcd_apply(dev);
	} else {
		d4_dp_lcd_off();
#ifndef CONFIG_PMU_SELECT
		clk_disable(ctx->lcd_clk);
		clk_disable(ctx->bus_clk);
#else
#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
		clk_disable(ctx->slcd_clk);
		clk_disable(ctx->slcd_out_clk);
#endif
		dp_pmu_on_off(DP_ON);
#endif
		ctx->suspended = true;
	}

	return 0;
}

extern void dp_lcd_video_isr(void);

static irqreturn_t drime4_lcd_irq_frame(int irq, void *dev_id)
{
	struct dp_lcd_context *ctx = (struct dp_lcd_context *)dev_id;
	struct drime4_drm_subdrv *subdrv = &ctx->subdrv;
	struct drm_device *drm_dev = subdrv->drm_dev;
	struct drime4_drm_manager *manager = subdrv->manager;
	int dp_lcd_interrupt_state = d4_dp_lcd_interrupt_state();

	/* disable_irq_nosync(irq); */

	/* lcd frame interrupt */
	if (dp_mlcd_ISR_pending_clear() < 0)
		goto out;

	if (dp_lcd_interrupt_state & (1<<DRM_INTERRUPT)) {
		/* check the crtc is detached already from encoder */
		if (manager->pipe < 0)
			goto out;

		drm_handle_vblank(drm_dev, manager->pipe);
		dp_lcd_finish_pageflip(drm_dev, manager->pipe);
	}
	
	if (dp_lcd_interrupt_state & 0x0F) {
		dp_lcd_video_isr();
	}

	if (dp_lcd_interrupt_state & (1<<BUSCLOCK_CHANGE_INTERRUPT)) {
		clk_disable(ctx->bus_clk);
		if (d4_dp_lcd_interrupt_arg(BUSCLOCK_CHANGE_INTERRUPT) == 1) {
			clk_set_rate(ctx->bus_clk, 162000000);
		} else {
			clk_set_rate(ctx->bus_clk, 50000000);
		}
		clk_enable(ctx->bus_clk);
		d4_dp_lcd_interrupt_onoff(DP_OFF, BUSCLOCK_CHANGE_INTERRUPT, 0);
	}

	g_is_lcd_interrupt_occur = 1;
	wake_up_interruptible(&g_lcd_interrupt_wait);
out:
	return IRQ_HANDLED;
}

static int __devinit dp_lcd_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dp_lcd_context *ctx;
	struct drime4_drm_subdrv *subdrv;
	struct drime4_drm_dp_lcd_pdata *pdata;
	struct drime4_drm_panel_info *panel;
	struct resource *res;
	struct stfb_get_info getinfo;
#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
	unsigned long pin_config;
#endif
	int ret = -EINVAL;

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
	ctx->mlcd_clk = clk_get(NULL, "mlcd"); /* GCLKSEL5:MLCD_PATHSEL:1bit, 0x3012_0010(10bit) */
	if (IS_ERR(ctx->mlcd_clk)) {
			dev_err(dev, "failed to get bus clock\n");
			ret = PTR_ERR(ctx->mlcd_clk);
			goto err_clk_get;
	}
	ctx->mlcd_out_clk = clk_get(NULL, "mlcd_out"); /* GCLKSEL5:MLCD_PATHSEL:0bit , 0x3012_0010(9bit) */
	if (IS_ERR(ctx->mlcd_out_clk)) {
			dev_err(dev, "failed to get bus clock\n");
			ret = PTR_ERR(ctx->mlcd_out_clk);
			goto err_mlcd_clk;
	}
	ctx->mlcd_sel = clk_get(NULL, "mlcd_sel"); /* GCLKSEL5:MLCD_PATHSEL:2~3bit , 0x3012_0010(11~12bit) */
	if (IS_ERR(ctx->mlcd_sel)) {
			dev_err(dev, "failed to get bus clock\n");
			ret = PTR_ERR(ctx->mlcd_sel);
			goto err_mlcd_out_clk;
	}
	clk_enable(ctx->mlcd_clk);
	clk_enable(ctx->mlcd_out_clk);

	/* lcd bus clock set */
	ctx->bus_clk = clk_get(&pdev->dev, "dp");
	if (IS_ERR(ctx->bus_clk)) {
			dev_err(dev, "failed to get bus clock\n");
			ret = PTR_ERR(ctx->bus_clk);
			goto err_mlcd_sel;
	}
	clk_enable(ctx->bus_clk);
	clk_set_rate(ctx->bus_clk, 162000000); /* bus clock */

	/* mlcd clock set */
	ctx->lcd_clk = clk_get(NULL, "mlcd_sel1"); /* GCLKSEL5:MLCD_CKSEL1:0~5bit, 0x3012_0010(0~5bit), clock division 27Mhz */
	if (IS_ERR(ctx->lcd_clk)) {
			dev_err(dev, "failed to get bus clock\n");
			ret = PTR_ERR(ctx->lcd_clk);
			goto err_bus_clk;
	}
	clk_enable(ctx->lcd_clk);
	clk_set_rate(ctx->lcd_clk, 27000000); /* lcd clock */

#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
	ctx->slcd_clk = clk_get(NULL, "slcd");
	if (IS_ERR(ctx->slcd_clk)) {
			dev_err(dev, "failed to get slcd clock\n");
			ret = PTR_ERR(ctx->slcd_clk);
			goto err_clk;
	}

	ctx->slcd_out_clk = clk_get(NULL, "slcd_out");
	if (IS_ERR(ctx->slcd_out_clk)) {
			dev_err(dev, "failed to get slcd out clock\n");
			ret = PTR_ERR(ctx->slcd_out_clk);
			goto err_slcd_clk;
	}
	clk_enable(ctx->slcd_clk);
	clk_enable(ctx->slcd_out_clk);


	ctx->pmx = devm_pinctrl_get(&pdev->dev);
	ctx->pins_default = pinctrl_lookup_state(ctx->pmx, PINCTRL_STATE_DEFAULT);
	pinctrl_select_state(ctx->pmx, ctx->pins_default);


	pin_config = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
			PIN_CONFIG_DRIVE_STRENGTH_X1);
	ret = pin_config_group_set("drime4-pinmux", "slcdgrp", pin_config);


	pin_config = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
			PIN_CONFIG_DRIVE_STRENGTH_X4);
	ret = pin_config_group_set("drime4-pinmux", "slcdgrp_clk", pin_config);


#endif

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region.\n");
		ret = -EINVAL;
		goto err_slcd_out_clk;
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
	ret = request_irq(ctx->irq, drime4_lcd_irq_frame, IRQF_SHARED, LCD_MODULE_NAME, ctx);
	if (ret < 0) {
		dev_err(dev, "irq request failed.\n");
		goto err_req_irq;
	}

	ctx->vid_default_win = pdata->default_vid_win;
	ctx->grp_default_win = pdata->default_grp_win;
	ctx->panel = panel;

	subdrv = &ctx->subdrv;

	subdrv->dev = dev;
	subdrv->manager = &dp_lcd_manager;
	subdrv->probe = dp_lcd_subdrv_probe;
	subdrv->remove = dp_lcd_subdrv_remove;


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
	g_lcd_ctx = ctx;

	/* dp lcd control initialize  */
	getinfo.dp_global = (unsigned int)ctx->regs;
	getinfo.dp_lcd = (unsigned int)ctx->regs + DP_LCD_BASE;
	getinfo.video.win = 0;
	getinfo.video.format = DP_YUV_420;
	getinfo.video.bit = vid8bit;
	getinfo.video.vid_stride = panel->timing.xres;
	getinfo.video.image.image_width = panel->timing.xres;
	getinfo.video.image.image_height = panel->timing.yres;

	getinfo.video.display.H_Start = 0;
	getinfo.video.display.H_Size = panel->timing.xres;
	getinfo.video.display.V_Start = 0;
	getinfo.video.display.V_Size = panel->timing.yres;
	getinfo.video.address.y0_address = 0;
	getinfo.video.address.c0_address = 0;

	getinfo.graphic.win = 0;
	getinfo.graphic.grp_stride = panel->timing.xres*4;
	getinfo.graphic.image.image_width = panel->timing.xres;
	getinfo.graphic.image.image_height = panel->timing.yres;
	getinfo.graphic.display.H_Start = 0;
	getinfo.graphic.display.H_Size = panel->timing.xres;
	getinfo.graphic.display.V_Start = 0;
	getinfo.graphic.display.V_Size = panel->timing.yres;
	getinfo.graphic.address = 0;
	dp_lcd_set_info(&getinfo);

	mutex_init(&ctx->lock);
	platform_set_drvdata(pdev, ctx);


	pm_runtime_enable(dev);
	pm_runtime_get_sync(dev);

	drime4_drm_subdrv_register(subdrv);
	init_waitqueue_head(&g_lcd_interrupt_wait);
	return 0;

err_req_irq:
err_req_region_irq:
	iounmap(ctx->regs);

err_req_region_io:
	release_resource(ctx->regs_res);
	kfree(ctx->regs_res);

err_slcd_out_clk:
#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
	clk_disable(ctx->slcd_out_clk);
	clk_put(ctx->slcd_out_clk);
#endif

err_slcd_clk:
#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
	clk_disable(ctx->slcd_clk);
	clk_put(ctx->slcd_clk);
#endif

err_clk:
	clk_disable(ctx->lcd_clk);
	clk_put(ctx->lcd_clk);

err_bus_clk:
	clk_disable(ctx->bus_clk);
	clk_put(ctx->bus_clk);

err_mlcd_sel:
	clk_disable(ctx->mlcd_sel);
	clk_put(ctx->mlcd_sel);

err_mlcd_out_clk:
	clk_disable(ctx->mlcd_out_clk);
	clk_put(ctx->mlcd_out_clk);

err_mlcd_clk:
	clk_disable(ctx->mlcd_clk);
	clk_put(ctx->mlcd_clk);

err_clk_get:
	kfree(ctx);
	return ret;
}

static int __devexit dp_lcd_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dp_lcd_context *ctx = platform_get_drvdata(pdev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if(ctx == NULL)
		return -1;	

	drime4_drm_subdrv_unregister(&ctx->subdrv);

	if (ctx->suspended)
		goto out;

	clk_disable(ctx->lcd_clk);
	clk_disable(ctx->bus_clk);
	clk_disable(ctx->mlcd_clk);
	clk_disable(ctx->mlcd_out_clk);
	clk_disable(ctx->mlcd_sel);

	pm_runtime_set_suspended(dev);
	pm_runtime_put_sync(dev);

out:
	pm_runtime_disable(dev);

	clk_put(ctx->lcd_clk);
	clk_put(ctx->bus_clk);
	clk_put(ctx->mlcd_clk);
	clk_put(ctx->mlcd_out_clk);
	clk_put(ctx->mlcd_sel);

	iounmap(ctx->regs);
	release_resource(ctx->regs_res);
	kfree(ctx->regs_res);
	free_irq(ctx->irq, ctx);

	kfree(ctx);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int dp_lcd_suspend(struct device *dev)
{
	struct dp_lcd_context *ctx = get_dp_lcd_context(dev);

	if(ctx == NULL)
		return -1;
	
	if (pm_runtime_suspended(dev))
		return 0;

	dp_lcd_save_reg();

	/*
	 * do not use pm_runtime_suspend(). if pm_runtime_suspend() is
	 * called here, an error would be returned by that interface
	 * because the usage_count of pm runtime is more than 1.
	 */
	return dp_lcd_power_on(ctx, false);
}

static int dp_lcd_resume(struct device *dev)
{
	//SW_DBG_P0_HIGH();

	struct dp_lcd_context *ctx = get_dp_lcd_context(dev);

	if(ctx == NULL)
	{
		//SW_DBG_P0_LOW();
		return -1;
	}

	/*
	 * if entered to sleep when lcd panel was on, the usage_count
	 * of pm runtime would still be 1 so in this case, dp_lcd driver
	 * should be on directly not drawing on pm runtime interface.
	 */
	if (!pm_runtime_suspended(dev)) {
		if (dp_lcd_power_on(ctx, true) < 0)
		{
			//SW_DBG_P0_LOW();
			return -1;
		}

		dp_lcd_pannel_set();
		dp_lcd_restore_reg();
	}
	
	//SW_DBG_P0_LOW();
	return 0;
}
#endif

#ifdef CONFIG_PM_RUNTIME
static int dp_lcd_runtime_suspend(struct device *dev)
{
	struct dp_lcd_context *ctx = get_dp_lcd_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	dp_lcd_save_reg();
	return dp_lcd_power_on(ctx, false);
}

static int dp_lcd_runtime_resume(struct device *dev)
{
	struct dp_lcd_context *ctx = get_dp_lcd_context(dev);

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (dp_lcd_power_on(ctx, true) < 0)
		return -1;
		dp_lcd_restore_reg();
	return 0;
}
#endif

static const struct dev_pm_ops dp_lcd_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dp_lcd_suspend, dp_lcd_resume)
	SET_RUNTIME_PM_OPS(dp_lcd_runtime_suspend, dp_lcd_runtime_resume, NULL)
};

struct platform_driver dp_lcd_driver = {
	.probe		= dp_lcd_probe,
	.remove		= __devexit_p(dp_lcd_remove),
	.driver		= {
		.name	= LCD_MODULE_NAME,
		.owner	= THIS_MODULE,
		.pm	= &dp_lcd_pm_ops,
	},
};
