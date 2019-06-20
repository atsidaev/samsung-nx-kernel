/*
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * Authors:
 * Seung-Woo Kim <sw0312.kim@samsung.com>
 * Inki Dae <inki.dae@samsung.com>
 * Joonyoung Shim <jy0922.shim@samsung.com>
 * Somabha Bhattacharjya >b.somabha@samsung.com>
 *
 * Based on drivers/media/video/s5p-tv/tv_reg.c
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "drmP.h"

#include "regs-mixer.h"
#include "regs-vp.h"

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>

#include <mach/d4_cma.h>
#include <mach/map.h>

#include <drm/drime4_drm.h>

#include "drime4_drm_drv.h"
#include "drime4_drm_hdmi.h"

#include "../../../video/drime4/tv/d4_dp_tv_dd.h"

#include <mach/dp/d4_dp.h>
#include <video/drime4/d4_dp_type.h>

#include "../../../video/drime4/global/d4_dp_global_reg.h"
#include "../../../video/drime4/global/d4_dp_global.h"

#define get_dp_tv_context(dev)	platform_get_drvdata(to_platform_device(dev))


extern struct fb_ops drime4_tv_fb_ops;
struct tvfb_get_info tvgetinfo;
extern int g_is_tv_interrupt_occur;
extern wait_queue_head_t g_tv_interrupt_wait;

struct drime4_platform_tvfb *to_fb_plat_tv(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	return (struct drime4_platform_tvfb *) pdev->dev.platform_data;
}

/* I2C driver for TV */
static struct i2c_driver tv_i2c_driver;

static int tv_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	tvgetinfo.tv_i2c_client = client;
	return 0;
}

static int tv_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id tv_i2c_id[] = { { "hdmi-phy", 0 }, { } };

static struct i2c_driver tv_i2c_driver = {
	.driver = {
		.name = "drime4_tv_i2c",
	},
	.probe = tv_i2c_probe,
	.remove = tv_i2c_remove,
	.id_table = tv_i2c_id,
};

struct hdmi_win_data {
	dma_addr_t dma_addr;
	void __iomem *vaddr;
	dma_addr_t chroma_dma_addr;
	void __iomem *chroma_vaddr;
	uint32_t pixel_format;
	unsigned int bpp;
	unsigned int crtc_x;
	unsigned int crtc_y;
	unsigned int crtc_width;
	unsigned int crtc_height;
	unsigned int fb_x;
	unsigned int fb_y;
	unsigned int fb_width;
	unsigned int fb_height;
	unsigned int src_width;
	unsigned int src_height;
	unsigned int mode_width;
	unsigned int mode_height;
	unsigned int scan_flags;
};

struct dp_tv_resources {
	int irq;
	void __iomem *dp_tv_regs;
	spinlock_t reg_slock;
#if 0
	struct clk *tv;
	struct clk *vp;
	struct clk *sclk_tv;
	struct clk *sclk_hdmi;
	struct clk *sclk_dac;
#endif
};

struct dp_tv_context {
	struct device *dev;
	int pipe;
	bool interlace;
	bool powered;
	u32 int_en;
	unsigned int vid_default_win;
	unsigned int grp_default_win;
	struct mutex dp_tv_mutex;
	struct dp_tv_resources dp_tv_res;
	struct hdmi_win_data win_data[MAX_DP_WINDOW];

};

static const u8 filter_y_horiz_tap8[] = { 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, 0, 0, 0, 0, 2, 4, 5, 6, 6, 6, 6, 6, 5, 5, 4, 3, 2, 1, 1, 0,
		-6, -12, -16, -18, -20, -21, -20, -20, -18, -16, -13, -10, -8, -5, -2,
		127, 126, 125, 121, 114, 107, 99, 89, 79, 68, 57, 46, 35, 25, 16, 8, };

static const u8 filter_y_vert_tap4[] = { 0, -3, -6, -8, -8, -8, -8, -7, -6, -5,
		-4, -3, -2, -1, -1, 0, 127, 126, 124, 118, 111, 102, 92, 81, 70, 59, 48,
		37, 27, 19, 11, 5, 0, 5, 11, 19, 27, 37, 48, 59, 70, 81, 92, 102, 111,
		118, 124, 126, 0, 0, -1, -1, -2, -3, -4, -5, -6, -7, -8, -8, -8, -8, -6,
		-3, };

static const u8 filter_cr_horiz_tap4[] = { 0, -3, -6, -8, -8, -8, -8, -7, -6,
		-5, -4, -3, -2, -1, -1, 0, 127, 126, 124, 118, 111, 102, 92, 81, 70, 59,
		48, 37, 27, 19, 11, 5, };



#if 0
static void tv_poweron(struct dp_tv_context *ctx)
{
	unsigned int set_value;

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	d4_dp_tv_clock_onoff(DP_ON);

	/* HDMI PHY Power */
	set_value = __raw_readl(global_fb.regs_global + dp_pwr_ctrl);
	__raw_writel(0x00, global_fb.regs_global + dp_pwr_ctrl);

	/* Phy reset release */
	set_value = __raw_readl(DRIME4_VA_RESET_CTRL);

	set_value |= (0x1 << 16);
	__raw_writel(set_value, DRIME4_VA_RESET_CTRL);

	set_value = __raw_readl(DRIME4_VA_RESET_CTRL);

	set_value &= ~(0x1 << 16);

	__raw_writel(set_value, DRIME4_VA_RESET_CTRL);

#if 0
	mutex_lock(&ctx->dp_tv_mutex);
	if (ctx->powered) {
		mutex_unlock(&ctx->dp_tv_mutex);
		return;
	}
	ctx->powered = true;
	mutex_unlock(&ctx->dp_tv_mutex);

	pm_runtime_get_sync(ctx->dev);

	clk_enable(res->tv);
	clk_enable(res->vp);
	clk_enable(res->sclk_tv);

	tv_reg_write(res, MXR_INT_EN, ctx->int_en);
	tv_win_reset(ctx);
#endif
}

static void tv_poweroff(struct dp_tv_context *ctx)
{
	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	d4_dp_tv_clock_onoff(DP_OFF);

#if 0
	mutex_lock(&ctx->dp_tv_mutex);
	if (!ctx->powered)
		goto out;
	mutex_unlock(&ctx->dp_tv_mutex);

	ctx->int_en = tv_reg_read(res, MXR_INT_EN);

	clk_disable(res->tv);
	clk_disable(res->vp);
	clk_disable(res->sclk_tv);

	pm_runtime_put_sync(ctx->dev);

	mutex_lock(&ctx->dp_tv_mutex);
	ctx->powered = false;

	out: mutex_unlock(&ctx->dp_tv_mutex);
#endif
}
#endif

static int tv_enable_vblank(void *ctx, int pipe)
{
	struct dp_tv_context *tv_ctx = ctx;

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	tv_ctx->pipe = pipe;
	d4_dp_tv_interrupt_onoff(DP_ON, DRM_INTERRUPT, 0);
	return 0;
}

static void tv_disable_vblank(void *ctx)
{
	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	d4_dp_tv_interrupt_onoff(DP_OFF, DRM_INTERRUPT, 0);
}

extern void d4_hdmi_stop(void);

static void tv_dpms(void *ctx, int mode)
{
	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	switch (mode) {
	case DRM_MODE_DPMS_ON:
		/* tv_poweron(tv_ctx); */
		d4_dp_tv_turn_video_on();
		break;
	case DRM_MODE_DPMS_STANDBY:
	case DRM_MODE_DPMS_SUSPEND:
	case DRM_MODE_DPMS_OFF:
		/* tv_poweroff(tv_ctx); */
		d4_hdmi_stop();
		dp_tv_off();
		break;
	default:
		DRM_DEBUG_KMS("unknown dpms mode: %d\n", mode);
		break;
	}
}

static void tv_win_mode_set(void *ctx, struct drime4_drm_overlay *overlay)
{
	struct dp_tv_context *tv_ctx = ctx;
	struct hdmi_win_data *win_data;
	int win;
	unsigned long offset;

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	if (!overlay) {
		DRM_ERROR("overlay is NULL\n");
		return;
	}

	DRM_DEBUG_KMS("set [%d]x[%d] at (%d,%d) to [%d]x[%d] at (%d,%d)\n",
			overlay->fb_width, overlay->fb_height, overlay->fb_x, overlay->fb_y,
			overlay->crtc_width, overlay->crtc_height, overlay->crtc_x,
			overlay->crtc_y);

	win = overlay->zpos;
	if (win == DEFAULT_ZPOS)
		win = TV_DEFAULT_WIN;

	if (win < 0 || win > TV_WIN_NR) {
		DRM_ERROR("tv window[%d] is wrong\n", win);
		return;
	}

	offset = overlay->fb_x * (overlay->bpp >> 3);
	offset += overlay->fb_y * overlay->pitch;

	win_data = &tv_ctx->win_data[win];

	win_data->dma_addr = overlay->dma_addr[0] + offset;
	win_data->vaddr = overlay->vaddr[0] + offset;
	win_data->chroma_dma_addr = overlay->dma_addr[1] + offset;
	win_data->chroma_vaddr = overlay->vaddr[1] + offset;
	win_data->pixel_format = overlay->pixel_format;
	win_data->bpp = overlay->bpp;

	win_data->crtc_x = overlay->crtc_x;
	win_data->crtc_y = overlay->crtc_y;
	win_data->crtc_width = overlay->crtc_width;
	win_data->crtc_height = overlay->crtc_height;

	win_data->fb_x = overlay->fb_x;
	win_data->fb_y = overlay->fb_y;
	win_data->fb_width = overlay->fb_width;
	win_data->fb_height = overlay->fb_height;
	win_data->src_width = overlay->src_width;
	win_data->src_height = overlay->src_height;

	win_data->mode_width = overlay->mode_width;
	win_data->mode_height = overlay->mode_height;

	win_data->scan_flags = overlay->scan_flag;
}

static void tv_win_commit(void *ctx, int win)
{
	struct dp_tv_context *tv_ctx = ctx;
	struct stgrpdisplay tvgrp_display;
	enum edp_tv_mode mode;

	if (win == DEFAULT_ZPOS)
		win = TV_DEFAULT_WIN;

	if (win < 0 || win > TV_WIN_NR) {
		DRM_ERROR("tv window[%d] is wrong\n", win);
		return;
	}

	dp_tv_mode_get(&mode);

	/* video */
	if (win < 4) {

	} else {
		tvgrp_display.address = tv_ctx->win_data[win].dma_addr;
		tvgrp_display.address_mode = DP_PHYSICAL_SET;
		if ((mode == RES_1080I_60_3D_SBS) || (mode == RES_1080I_50_3D_SBS)) {
			tvgrp_display.display.H_Size = tv_ctx->win_data[win].crtc_width;
			tvgrp_display.display.V_Size = tv_ctx->win_data[win].crtc_height;
			d4_dp_tv_graphics_set_scale(SCL_X1);
			d4_dp_tv_graphics_set_vertical_scale(SCL_X2);
		} else if (tv_ctx->win_data[win].crtc_width == 960){
			tvgrp_display.display.H_Size = tv_ctx->win_data[win].crtc_width*2;
			tvgrp_display.display.V_Size = tv_ctx->win_data[win].crtc_height*2;
			d4_dp_tv_graphics_set_scale(SCL_X2);
		} else {
			tvgrp_display.display.H_Size = tv_ctx->win_data[win].crtc_width;
			tvgrp_display.display.V_Size = tv_ctx->win_data[win].crtc_height;
			d4_dp_tv_graphics_set_scale(SCL_X1);
		}
		tvgrp_display.display.H_Start = tv_ctx->win_data[win].crtc_x;
		tvgrp_display.display.V_Start = tv_ctx->win_data[win].crtc_y;
		tvgrp_display.img_width = tv_ctx->win_data[win].fb_width;
		tvgrp_display.img_height = tv_ctx->win_data[win].fb_height;
		tvgrp_display.stride = (tv_ctx->win_data[win].fb_width) * 4;
		tvgrp_display.win = win - 4;
		tvgrp_display.win_onoff = DP_ON;
		d4_dp_tv_graphics_set(tvgrp_display);
	}


}

static void tv_win_disable(void *ctx, int win)
{

	DRM_DEBUG_KMS("[%d] %s, win: %d\n", __LINE__, __func__, win);

	if (win < 4) {
		d4_dp_tv_video_window_onoff(win, DP_OFF);
	} else {
		d4_dp_tv_graphics_window_onoff((win-4), DP_OFF);
	}

}

static struct drime4_tv_ops tv_ops = {
		/* manager */
		.enable_vblank = tv_enable_vblank,
		.disable_vblank = tv_disable_vblank,
		.dpms =	tv_dpms,
		/* overlay */
		.win_mode_set = tv_win_mode_set,
		.win_commit = tv_win_commit,
		.win_disable =	tv_win_disable,
};

/* for pageflip event */
static void tv_finish_pageflip(struct drm_device *drm_dev, int crtc)
{
	struct drime4_drm_private *dev_priv = drm_dev->dev_private;
	struct drm_pending_vblank_event *e, *t;
	struct timeval now;
	unsigned long flags;
	bool is_checked = false;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	spin_lock_irqsave(&drm_dev->event_lock, flags);

	list_for_each_entry_safe(e, t, &dev_priv->pageflip_event_list, base.link) {
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

extern void dp_tv_video_isr(void);

static irqreturn_t tv_irq_handler(int irq, void *arg)
{
	struct drime4_drm_hdmi_context *drm_hdmi_ctx = arg;
	struct dp_tv_context *ctx = drm_hdmi_ctx->ctx;
	int dp_tv_interrupt_state = d4_dp_tv_interrupt_state();

	/* tv frame interrupt */
	if (dptv_interrupt_check() < 0)
		return IRQ_NONE;

	if (dp_tv_interrupt_state & (1<<DRM_INTERRUPT)) {
		/* check the crtc is detached already from encoder */
		drm_handle_vblank(drm_hdmi_ctx->drm_dev, ctx->pipe);
		tv_finish_pageflip(drm_hdmi_ctx->drm_dev, ctx->pipe);
	}
	
	if (dp_tv_interrupt_state & 0x0F) {
		dp_tv_video_isr();
	}

	g_is_tv_interrupt_occur = 1;
	wake_up_interruptible(&g_tv_interrupt_wait);
	return IRQ_HANDLED;
}

static int __devinit tv_resources_init(struct drime4_drm_hdmi_context *ctx,
		struct platform_device *pdev)
{
	struct dp_tv_context *tv_ctx = ctx->ctx;
	struct device *dev = &pdev->dev;
	struct dp_tv_resources *tv_res = &tv_ctx->dp_tv_res;
	struct resource *res = NULL;
	int ret;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "get memory resource failed.\n");
		ret = -ENXIO;
		goto fail;
	}

	tv_res->dp_tv_regs = ioremap(res->start, resource_size(res));
	if (tv_res->dp_tv_regs == NULL) {
		dev_err(dev, "register mapping failed.\n");
		ret = -ENXIO;
		goto fail_tv_regs;
	}


	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		dev_err(dev, "get interrupt resource failed.\n");
		ret = -ENXIO;
		goto fail;
	}

	tv_res->irq = res->start;

	ret = request_irq(tv_res->irq, tv_irq_handler, IRQF_SHARED, TV_MODULE_NAME, ctx);
	if (ret) {
		dev_err(dev, "request interrupt failed.\n");
		return ret;
	}

	/* Set TV */
	tvgetinfo.dp_global = (unsigned int)tv_res->dp_tv_regs;
	tvgetinfo.dp_tv = (unsigned int)tv_res->dp_tv_regs + DP_TV_BASE;


	tvgetinfo.video.win = 0;
	tvgetinfo.video.format = DP_YUV_420;
	tvgetinfo.video.bit = vid8bit;


	tvgetinfo.video.vid_stride = 1920*4; /* tv_ctx->win_data[0].fb_x; */
	tvgetinfo.video.image.image_width = 1920; /* tv_ctx->win_data[0].fb_width; */
	tvgetinfo.video.image.image_height = 1080; /* tv_ctx->win_data[0].fb_height; */

	tvgetinfo.video.display.H_Start = 0;
	tvgetinfo.video.display.H_Size = 1920; /* tv_ctx->win_data[0].fb_width; */
	tvgetinfo.video.display.V_Start = 0;
	tvgetinfo.video.display.V_Size = 1080; /* tv_ctx->win_data[0].fb_height; */

	tvgetinfo.video.address.y0_address = 0;
	tvgetinfo.video.address.c0_address = 0;

	tvgetinfo.graphic.win = 0;
	tvgetinfo.graphic.grp_stride = 7680; /* tv_ctx->win_data[0].fb_x*4; */
	tvgetinfo.graphic.image.image_width = 1920; /* tv_ctx->win_data[0].fb_width; */
	tvgetinfo.graphic.image.image_height = 1080; /* tv_ctx->win_data[0].fb_height; */
	tvgetinfo.graphic.display.H_Start = 0;
	tvgetinfo.graphic.display.H_Size = 1920; /* tv_ctx->win_data[0].fb_width; */
	tvgetinfo.graphic.display.V_Start = 0;
	tvgetinfo.graphic.display.V_Size = 1080; /* tv_ctx->win_data[0].fb_height; */
	tvgetinfo.graphic.address = 0;

	/* I2C initialization */
	ret = i2c_add_driver(&tv_i2c_driver);
	if (ret < 0) {
		printk("\n\nfailed to add TV I2C driver\n\n");
		return ret;
	}

	d4_dp_tv_set_info(&tvgetinfo);
	return 0;

	fail_tv_regs:
		iounmap(tv_res->dp_tv_regs);

	fail:
		return ret;
}

static void tv_resources_cleanup(struct dp_tv_context *ctx)
{
	struct dp_tv_resources *res = &ctx->dp_tv_res;

	free_irq(res->irq, ctx);
	iounmap(res->dp_tv_regs);
}

static int __devinit _dp_tv_resume(struct platform_device *pdev)
{
 	return 0;
 }

static int __devinit dp_tv_probe(struct platform_device *pdev)
{
	struct drime4_platform_tvfb *pdata = NULL;
	struct device *dev = &pdev->dev;
	struct drime4_drm_hdmi_context *drm_hdmi_ctx;
	struct dp_tv_context *ctx;
	int ret;
	unsigned int set_value = 0;


	dev_info(dev, "probe start\n");

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(dev, "no platform data specified\n");
		return -EINVAL;
	}


	/* DP Reset */
	set_value = __raw_readl(DRIME4_VA_RESET_CTRL);
	__raw_writel(0x00, DRIME4_VA_RESET_CTRL);


	drm_hdmi_ctx = kzalloc(sizeof(*drm_hdmi_ctx), GFP_KERNEL);
	if (!drm_hdmi_ctx) {
		DRM_ERROR("failed to allocate common hdmi context.\n");
		return -ENOMEM;
	}

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		DRM_ERROR("failed to alloc tv context.\n");
		kfree(drm_hdmi_ctx);
		return -ENOMEM;
	}

	mutex_init(&ctx->dp_tv_mutex);

	ctx->dev = &pdev->dev;
	drm_hdmi_ctx->ctx = (void *)ctx;

	platform_set_drvdata(pdev, drm_hdmi_ctx);

	/* acquire resources: regs, irqs, clocks */
	ret = tv_resources_init(drm_hdmi_ctx, pdev);
	if (ret)
		goto fail;

	/* register specific callback point to common hdmi. */
	drime4_tv_ops_register(&tv_ops);
	init_waitqueue_head(&g_tv_interrupt_wait);
	return 0;

	fail:
		dev_info(dev, "probe failed\n");
	return ret;
}

/**
 * @brief TV Suspend/Release
 * @fn      static int _drime4_tv_suspend(struct platform_device *pdev)
 * @param   *pdev	[in] platform device
 * @return  실패시 음수값 반환
 *
 * @author  Somabha B
 */
static int _dp_tv_suspend(struct platform_device *pdev)
{
	return 0;
}

static int dp_tv_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct drime4_drm_hdmi_context *drm_hdmi_ctx = platform_get_drvdata(pdev);
	struct dp_tv_context *ctx;

	if (drm_hdmi_ctx == NULL) {
		return -1;
	}
	
	ctx = drm_hdmi_ctx->ctx;

	dev_info(dev, "remove successful\n");

	pm_runtime_disable(&pdev->dev);

	tv_resources_cleanup(ctx);

	return 0;
}

static int dp_tv_suspend(struct platform_device *pdev)
{
	return _dp_tv_suspend(pdev);
}

static int dp_tv_resume(struct platform_device *pdev)
{
	return _dp_tv_resume(pdev);
}

struct platform_driver dp_tv_driver = {
		.probe = dp_tv_probe,
		.remove = dp_tv_remove,
		.suspend = dp_tv_suspend,
		.resume = dp_tv_resume,
		.driver = {
				.name = TV_MODULE_NAME,
				.owner = THIS_MODULE,
		},
};
