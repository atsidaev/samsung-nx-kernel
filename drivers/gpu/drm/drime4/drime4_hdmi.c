/*
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * Authors:
 * Seung-Woo Kim <sw0312.kim@samsung.com>
 *	Inki Dae <inki.dae@samsung.com>
 *	Joonyoung Shim <jy0922.shim@samsung.com>
 *
 * Based on drivers/media/video/s5p-tv/hdmi_drv.c
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "drmP.h"
#include "drm_edid.h"
#include "drm_crtc_helper.h"

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
#include <linux/pinctrl/consumer.h>

#include <drm/drime4_drm.h>

#include "drime4_drm_drv.h"
#include "drime4_drm_hdmi.h"

#include <mach/dp/d4_dp.h>
#include <video/drime4/d4_dp_type.h>
#include <linux/gpio.h>

#include "../../../video/drime4/tv/d4_dp_tv_dd.h"

#include "drime4_hdmi.h"
#include "../../../video/drime4/hdmi/video/d4_hdmi.h"
#include "../../../../include/video/hdmi/d4_hdmi_video_type.h"
#include "../../../video/drime4/hdmi/video/d4_hdmi_regs.h"

#define DD_HDMI		"/dev/HDMI"

#define MAX_WIDTH		1920
#define MAX_HEIGHT		1080
#define get_hdmi_context(dev)	platform_get_drvdata(to_platform_device(dev))

#if 0
static int last_hpd_state;
#define HPD_LO          0
#define HPD_HI          1
#endif

struct hdmi_resources {
	struct clk *hdmi;
	struct clk *sclk_hdmi;
	struct clk *sclk_pixel;
	struct clk *sclk_hdmiphy;
	struct clk *hdmiphy;
	struct regulator_bulk_data *regul_bulk;
	int regul_count;
};

extern const struct file_operations drime4_hdmi_oper;
extern struct stfb_info tv_info;
static struct miscdevice drime4_hdmi_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "HDMI",
		.fops = &drime4_hdmi_oper, /*defined in d4_hdmi.c*/
};

struct hdmi_context {
	struct device *dev;
	struct drm_device *drm_dev;
	bool hpd;
	bool powered;
	bool dvi_mode;
	struct mutex hdmi_mutex;
	struct d4_hdmi_registers hdmi_registers;
	struct i2c_client *ddc_port;
	struct i2c_client *hdmiphy_port;
	struct HDMIVideoParameter video;
	enum edp_tv_mode tvmode;
	/* current hdmiphy conf index */
	int cur_conf;

	struct hdmi_resources res;
	void *parent_ctx;

	void (*cfg_hpd)(bool external);
	int (*get_hpd)(void);
};


#if defined(CONFIG_HDMI_HPD)
extern unsigned long hdmi_detect_changed;
extern wait_queue_head_t hdmi_hpd_wait;
unsigned long g_change_hdmi_value;
static unsigned int g_prev_hdmi_hpd_state;
#endif

struct hdmi_conf {
	int width;
	int height;
	int vrefresh;
	bool interlace;
};

static const struct hdmi_conf hdmi_confs[] = { 
	{ 720, 480, 60, false },
	{ 720, 576, 50, false },
	{ 1280, 720, 50, false },
	{ 1280, 720, 60, false },
	{ 1920, 1080, 50, false },
	{ 1920, 1080, 60, false },
};

static const struct hdmi_conf hdmi_3d_confs[] = {
	{ 1920, 1080, 30, false }, 	/* for 3d 30p fp */
	{ 1920, 1080, 60, true }, 	/* for 3d 60i sbs */
	{ 1920, 1080, 50, true }, 	/* for 3d 50i sbs */
};

void change_hdmi_setting(unsigned long value);

static int hdmi_v14_conf_index(struct drm_display_mode *mode)
{
#if 0
	int i;

	for (i = 0; i < ARRAY_SIZE(hdmi_confs); ++i)
		if (hdmi_confs[i].width == mode->hdisplay
				&& hdmi_confs[i].height == mode->vdisplay
				&& hdmi_confs[i].vrefresh == mode->vrefresh
				&& hdmi_confs[i].interlace
						== ((mode->flags & DRM_MODE_FLAG_INTERLACE) ?
								true : false))
			return i;

	return -EINVAL;
#endif
	return 0;
}


static bool hdmi_is_connected(void *ctx)
{
	struct hdmi_context *hdata = ctx;
	return hdata->hpd;
}

static int hdmi_get_edid(void *ctx, struct drm_connector *connector, u8 *edid,
		int len)
{
	struct edid *raw_edid;
	struct hdmi_context *hdata = ctx;

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	if (!hdata->ddc_port)
		return -ENODEV;

	raw_edid = drm_get_edid(connector, hdata->ddc_port->adapter);
	connector->display_info.raw_edid = (char *)raw_edid;

	if (!ZERO_OR_NULL_PTR(raw_edid)) {
		hdata->dvi_mode = !drm_detect_hdmi_monitor(raw_edid);
		memcpy(edid, raw_edid,
				min((1 + raw_edid->extensions) * EDID_LENGTH, len));
		DRM_DEBUG_KMS("%s : width[%d] x height[%d]\n",
				(hdata->dvi_mode ? "dvi monitor" : "hdmi monitor"),
				raw_edid->width_cm, raw_edid->height_cm);

	} else {
		return -ENODEV;
	}

	return 0;
}

static int hdmi_v14_check_timing(struct fb_videomode *check_timing)
{
	int i;

	DRM_DEBUG_KMS("valid mode : xres=%d, yres=%d, refresh=%d, intl=%d\n",
			check_timing->xres, check_timing->yres, check_timing->refresh,
			(check_timing->vmode & FB_VMODE_INTERLACED) ? true : false);

	for (i = 0; i < ARRAY_SIZE(hdmi_confs); i++) {
		if (hdmi_confs[i].width == check_timing->xres
				&& hdmi_confs[i].height == check_timing->yres
				&& hdmi_confs[i].vrefresh == check_timing->refresh
				&& hdmi_confs[i].interlace == ((check_timing->vmode & FB_VMODE_INTERLACED) ?true : false)) {
			return 0;			
		}
	}

	if(check_timing->vmode & (1<<16))
	{
		for (i = 0; i < ARRAY_SIZE(hdmi_3d_confs); i++) {
			if (hdmi_3d_confs[i].width == check_timing->xres
					&& hdmi_3d_confs[i].height == check_timing->yres
					&& hdmi_3d_confs[i].vrefresh == check_timing->refresh
					&& hdmi_3d_confs[i].interlace == ((check_timing->vmode & FB_VMODE_INTERLACED) ?true : false)) {
				return 0;
			}
		}
	}

	return -EINVAL;
}

static int hdmi_check_timing(void *ctx, void *timing)
{
	struct fb_videomode *check_timing = timing;

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	return hdmi_v14_check_timing(check_timing);
}

static void hdmi_mode_fixup(void *ctx, struct drm_connector *connector,
		struct drm_display_mode *mode, struct drm_display_mode *adjusted_mode)
{
	struct drm_display_mode *m;
	int index;

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	drm_mode_set_crtcinfo(adjusted_mode, 0);

	index = hdmi_v14_conf_index(adjusted_mode);

	/* just return if user desired mode exists. */
	if (index >= 0)
		return;

	/*
	 * otherwise, find the most suitable mode among modes and change it
	 * to adjusted_mode.
	 */
	list_for_each_entry(m, &connector->modes, head)
	{
		index = hdmi_v14_conf_index(m);

		if (index >= 0) {
			DRM_INFO("desired mode doesn't exist so\n");
			DRM_INFO("use the most suitable mode among modes.\n");
			memcpy(adjusted_mode, m, sizeof(*m));
			break;
		}
	}
}

extern void set_ts_hdmi_info(int is_hdmi_connected, int res_x, int res_y);

static void hdmi_mode_set(void *ctx, void *mode, void *info)
{
	struct hdmi_context *hdata = ctx;
	struct drm_display_mode *d_mode = (struct drm_display_mode *)mode;
	struct drm_display_info *d_info = (struct drm_display_info *)info;

	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);
	hdata->video.hdmi_3d_format = HDMI_2D_VIDEO_FORMAT;
	
	if (720 == d_mode->vdisplay) {
		if (60 == d_mode->vrefresh) {
			hdata->tvmode = RES_720P_60;
			hdata->video.resolution = v1280x720p_60Hz;
		} else if (50 == d_mode->vrefresh) {
			hdata->tvmode = RES_720P_50;
			hdata->video.resolution = v1280x720p_50Hz;
		}
	} else if (480 == d_mode->vdisplay) {
		hdata->tvmode = RES_480P;
		hdata->video.resolution = v720x480p_60Hz;
	} else if (576 == d_mode->vdisplay) {
		hdata->tvmode = RES_576P;
		hdata->video.resolution = v720x576p_50Hz;
	} else if (540 == d_mode->vdisplay) {
		if (60 == d_mode->vrefresh) {
			if (d_mode->flags & DRM_MODE_FLAG_INTERLACE) {
				hdata->tvmode = RES_1080I_60_3D_SBS;
				hdata->video.resolution = v1920x1080i_60Hz;
				hdata->video.hdmi_3d_format = HDMI_3D_SSH_FORMAT;
			}
			else {
				hdata->tvmode = RES_1080P_60;
				hdata->video.resolution = v1920x1080p_60Hz;
			}
		} else if (50 == d_mode->vrefresh) {
			if (d_mode->flags & DRM_MODE_FLAG_INTERLACE) {
				hdata->tvmode = RES_1080I_50_3D_SBS;
				hdata->video.resolution = v1920x1080i_50Hz;
				hdata->video.hdmi_3d_format = HDMI_3D_SSH_FORMAT;
			}
			else {
				hdata->tvmode = RES_1080P_50;
				hdata->video.resolution = v1920x1080p_50Hz;
			}
		} 
		else if (30 == d_mode->vrefresh) {
			hdata->tvmode = RES_1080P_30_3D_FP;
			hdata->video.resolution = v1920x1080p_30Hz;
			hdata->video.hdmi_3d_format = HDMI_3D_FP_FORMAT;
		}
	}

	if(hdata->dvi_mode)
		hdata->video.mode = DVI;
	else
		hdata->video.mode = HDMI;
	
	hdata->video.colorDepth = HDMI_CD_24;
	hdata->video.colorimetry = HDMI_COLORIMETRY_NO_DATA;
	hdata->video.pixelAspectRatio = HDMI_PIXEL_RATIO_16_9;
	hdata->video.videoSrc = HDMI_SOURCE_INTERNAL;

	if(d_info->color_formats & DRM_COLOR_FORMAT_YCRCB422)
		hdata->video.colorSpace = HDMI_CS_YCBCR422;
	else if(d_info->color_formats & DRM_COLOR_FORMAT_YCRCB444)
		hdata->video.colorSpace = HDMI_CS_YCBCR444;
	else
		hdata->video.colorSpace = HDMI_CS_RGB;
	
	if(hdata->video.hdmi_3d_format != HDMI_2D_VIDEO_FORMAT)
		hdata->video.colorSpace = HDMI_CS_RGB;
	
	dp_tv_mode_global_variable_set(hdata->tvmode);

#if defined(CONFIG_TOUCHSCREEN_MELFAS)
	set_ts_hdmi_info(1, d_mode->hdisplay, d_mode->vdisplay);
#endif
}

static void hdmi_get_max_resol(void *ctx, unsigned int *width,
		unsigned int *height)
{
	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	*width = MAX_WIDTH;
	*height = MAX_HEIGHT;
}

static void hdmi_commit(void *ctx)
{
	struct hdmi_context *hdata = ctx;
	enum scan_type type;
	enum PixelLimit limit;

	if (hdata->video.hdmi_3d_format == HDMI_3D_FP_FORMAT) {
		type = fp_progressive;
	} else if (hdata->video.hdmi_3d_format == HDMI_3D_SSH_FORMAT) {
		type = ssh;
	} else if (hdata->video.hdmi_3d_format == HDMI_3D_TB_FORMAT) {
		type = tb;
	} else {
		type = none;
	}
	
	d4_hdmi_stop();
	msleep(1000);
	dp_tv_3d_onoff(&tv_info, type);
	if(hdata->video.colorSpace == HDMI_CS_RGB)
	{
		d4_dp_link_set(hdata->tvmode, DP_OFF);	/* TODO: does DRM pass output bit? */
		limit = HDMI_RGB_LIMIT_RANGE;
	}
	else
	{
		d4_dp_link_set(hdata->tvmode, DP_ON);	/* TODO: does DRM pass output bit? */
		limit = HDMI_YCBCR_LIMIT_RANGE;
	}
	msleep(50);

	/* set pixel aspect ratio */
	if (!d4_hdmi_set_pixel_aspect_ratio(hdata->video.pixelAspectRatio)) {
		printk("setting aspect ratio failed!\n");
		return;
	}

	/* set colorimetry */
	if (!d4_hdmi_set_colorimetry(hdata->video.colorimetry)) {
		printk("setting calorimetry failed!\n");
		return;
	}

	/* set color space */
	if (!d4_hdmi_set_color_space(hdata->video.colorSpace)) {
		printk("setting color space failed!\n");
		return;
	}

	/* set pixel limitation */
	if (!d4_hdmi_set_pixel_limit(limit)) {
		printk("setting pixel limit failed!\n");
		return;
	}

	/* set color depth */
	if (!d4_hdmi_set_color_depth(hdata->video.colorDepth)) {
		printk("setting color depth failed!\n");
		return;
	}

	/* set video parameters */
	if (!d4_hdmi_set_video_mode(&hdata->video)) {
		printk("setting video mode failed!\n");
		return;
	}

	/* set HDMI Mode */
	d4_hdmi_mode_select(hdata->video.mode);
	/* start HDMI */
	d4_hdmi_start();
	return;
}

extern void dp_bus_clock_change(unsigned long isHighRate);

static void hdmi_poweron(struct hdmi_context *hdata)
{
	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);
	
//	dp_bus_clock_change(DP_ON);
//	msleep(30);
	d4_dp_tv_clock_onoff(DP_ON);

	d4_hdmi_global_interrupt_enable(1);
	d4_hdmi_cec_interrupt_enable(1);
	hdata->powered = true;
}

unsigned long isdpmsoff = 1;

static void hdmi_poweroff(struct hdmi_context *hdata)
{
	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);
	
#if defined(CONFIG_TOUCHSCREEN_MELFAS)	
	set_ts_hdmi_info(0, 0, 0);
#endif
	d4_hdmi_cec_interrupt_enable(0);
	d4_hdmi_global_interrupt_enable(0);

	d4_dp_tv_clock_onoff(DP_OFF);
//	dp_bus_clock_change(DP_OFF);
	hdata->powered = false;
}

static void hdmi_dpms(void *ctx, int mode)
{
	struct hdmi_context *hdata = ctx;
	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	switch (mode) {
	case DRM_MODE_DPMS_ON:
		isdpmsoff = 0;
		hdmi_poweron(hdata);
		break;
	case DRM_MODE_DPMS_STANDBY:
	case DRM_MODE_DPMS_SUSPEND:
	case DRM_MODE_DPMS_OFF:
		if(hdmi_is_connected(hdata))
			msleep(1000);
		hdmi_poweroff(hdata);
		isdpmsoff = 1;
		break;
	default:
		DRM_DEBUG_KMS("unknown dpms mode: %d\n", mode);
		break;
	}
}

static struct drime4_hdmi_ops hdmi_ops = {
	/* Display */
	.is_connected = hdmi_is_connected,
	.get_edid = hdmi_get_edid,
	.check_timing =	hdmi_check_timing,
	/* manager */
	.mode_fixup = hdmi_mode_fixup,
	.mode_set = hdmi_mode_set,
	.get_max_resol = hdmi_get_max_resol,
	.commit = hdmi_commit,
	.dpms = hdmi_dpms,
};


static struct i2c_client *hdmi_ddc, *hdmi_hdmiphy;

void hdmi_attach_ddc_client(struct i2c_client *ddc)
{
	DRM_DEBUG_KMS("[%d]\n", __LINE__);
	if (ddc)
		hdmi_ddc = ddc;
}

void hdmi_attach_hdmiphy_client(struct i2c_client *hdmiphy)
{
	DRM_DEBUG_KMS("[%d]\n", __LINE__);
	if (hdmiphy)
		hdmi_hdmiphy = hdmiphy;
}

irqreturn_t drime4_gpio_hpd_isr(int irq, void *dev_id)
{
	if (g_change_hdmi_value == 0) {
		change_hdmi_setting(0);
	}

	return IRQ_HANDLED;	
}

int hdmi_setup_gpio_interrupt(struct platform_device *pdev)
{
	struct drime4_drm_hdmi_pdata *pdata	= pdev->dev.platform_data;
	int ret = -1;
	char *desc = "hpd_gpio";
	unsigned long irqflags;		
	int irq;

	ret = pinctrl_request_gpio(pdata->hpd_gpio);
	if (ret) {
		printk("hdmi_setup_gpio_interrupt pinmux request gpio fail: %d\n", ret);
		return -EINVAL;
	}

	ret = pinctrl_gpio_direction_input(pdata->hpd_gpio);
	if (ret < 0) {
		return ret;
	}

	irq = gpio_to_irq(pdata->hpd_gpio);
	if (irq < 0) {
		ret = irq;
		return ret;
	}

	irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
	ret = request_irq(irq, drime4_gpio_hpd_isr, irqflags, desc, pdev);
	return ret;
}

static int __devinit _d4_hdmi_resume(struct platform_device *pdev)
{
	hdmi_setup_gpio_interrupt(pdev);

	g_change_hdmi_value = 0;
	g_prev_hdmi_hpd_state = 0;
	change_hdmi_setting(0);
	return 0;
}

struct platform_device *hdmi_pdev;

static int __devinit d4_hdmi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct drime4_drm_hdmi_context *drm_hdmi_ctx;
	struct hdmi_context *hdata;
	struct drime4_drm_hdmi_pdata *pdata;
	int ret = -1;

	DRM_DEBUG_KMS("[%d]\n", __LINE__);

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		DRM_ERROR("no platform data specified\n");
		return -EINVAL;
	}

	drm_hdmi_ctx = kzalloc(sizeof(*drm_hdmi_ctx), GFP_KERNEL);
	if (!drm_hdmi_ctx) {
		DRM_ERROR("failed to allocate common hdmi context.\n");
		return -ENOMEM;
	}

	hdata = kzalloc(sizeof(struct hdmi_context), GFP_KERNEL);
	if (!hdata) {
		DRM_ERROR("out of memory\n");
		kfree(drm_hdmi_ctx);
		return -ENOMEM;
	}

	hdmi_pdev = pdev;

	mutex_init(&hdata->hdmi_mutex);

	drm_hdmi_ctx->ctx = (void *)hdata;
	hdata->parent_ctx = (void *)drm_hdmi_ctx;

	platform_set_drvdata(pdev, drm_hdmi_ctx);

	hdata->cfg_hpd = pdata->cfg_hpd;
	hdata->get_hpd = pdata->get_hpd;
	hdata->dev = dev;

	/* D4 HDMI registers settings */
	hdata->hdmi_registers.register_sys = ioremap(0x50100000, 0x10000);
	if (hdata->hdmi_registers.register_sys == NULL) {
		goto err_iomap;
	}

	hdata->hdmi_registers.register_core = ioremap(0x50110000, 0x10000);
	if (hdata->hdmi_registers.register_core == NULL) {
		goto err_iomap;
	}

	hdata->hdmi_registers.register_aes = ioremap(0x50120000, 0x10000);
	if (hdata->hdmi_registers.register_aes == NULL) {
		goto err_iomap;
	}

#if 0
	if (request_irq(IRQ_HDMI, hdmi_handler, IRQF_SHARED, "HDMI", drm_hdmi_ctx)) {
		printk(KERN_WARNING "HDMI: IRQ %d is not free.\n", IRQ_HDMI);
		return -EIO;
	}
#endif

	/**
	 * @brief HDMI Device registration
	 */
	ret = misc_register(&drime4_hdmi_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		kfree(drm_hdmi_ctx);
		return ret;
	}
	dev_dbg(&pdev->dev, "device registered as /dev/HDMI\n");

	init_waitqueue_head(&hdcp_struct.waitq);

	//hdmi_setup_gpio_interrupt(pdev);
	/* DDC i2c driver */
	if (i2c_add_driver(&ddc_driver)) {
		DRM_ERROR("failed to register ddc i2c driver\n");
		ret = -ENOENT;
		goto err_ddc;
	}

	hdata->ddc_port = hdmi_ddc;
#if 0
	/* hdmiphy i2c driver */
	if (i2c_add_driver(&hdmiphy_driver)) {
		DRM_ERROR("failed to register hdmiphy i2c driver\n");
		ret = -ENOENT;
		goto err_ddc;
	}

	hdata->hdmiphy_port = hdmi_hdmiphy;
#endif

	/* register specific callbacks to common hdmi. */
	drime4_hdmi_ops_register(&hdmi_ops);
	d4_hdmi_set_info(&hdata->hdmi_registers);
	return 0;

err_ddc:
	i2c_del_driver(&ddc_driver);

err_iomap:
	iounmap(hdata->hdmi_registers.register_sys);
	iounmap(hdata->hdmi_registers.register_core);
	iounmap(hdata->hdmi_registers.register_aes);

	kfree(hdata);
	kfree(drm_hdmi_ctx);
	return ret;
}

static int __devexit _d4_hdmi_suspend(struct platform_device *pdev)
{
	struct drime4_drm_hdmi_pdata *pdata;
	pdata = pdev->dev.platform_data;	
	free_irq(gpio_to_irq(pdata->hpd_gpio), pdev);
	pinctrl_free_gpio(pdata->hpd_gpio);
	d4_dp_tv_clock_onoff(DP_OFF);	
	return 0;
}

static int __devexit d4_hdmi_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct drime4_drm_hdmi_context *ctx = platform_get_drvdata(pdev);
	struct hdmi_context *hdata;

	if (ctx == NULL)
		return -1;

	hdata = ctx->ctx;
	
	DRM_DEBUG_KMS("[%d] %s\n", __LINE__, __func__);

	pm_runtime_disable(dev);

	iounmap(hdata->hdmi_registers.register_sys);
	iounmap(hdata->hdmi_registers.register_core);
	iounmap(hdata->hdmi_registers.register_aes);

	/* hdmiphy i2c driver */
	i2c_del_driver(&hdmiphy_driver);

	/* DDC i2c driver */
	i2c_del_driver(&ddc_driver);

	kfree(hdata);
	kfree(ctx);
	return 0;
}

#if 0
/**
 * @brief HDMI Interrupt Handler
 * @fn static irqreturn_t hdmi_handler(int irq, void *dev_id)
 * @param irq[in] irq number
 * @param dev_id[in] device id
 *
 * @return irq information
 */
static irqreturn_t hdmi_handler(int irq, void *dev_id)
{
	unsigned char flag;
	unsigned char reg;

	struct drime4_drm_hdmi_context *ctx =
			(struct drime4_drm_hdmi_context *) dev_id;
	struct hdmi_context *hdata = ctx->ctx;

	/* read flag register */
	flag = readb(reg_sys + 0x04);

	/* is this our interrupt? */
	if (!(flag & (1 << HDMI_IRQ_HPD_PLUG | 1 << HDMI_IRQ_HPD_UNPLUG)))
		return IRQ_HANDLED;


	/* ignore HPD IRQ caused by reseting HDCP engine */
	if (readb(reg_core + 0x0030) & HPD_SW_ENABLE) {
		writeb(HPD_SW_DISABLE, reg_core + 0x0030);
		/* clear pending bit */
		writeb(1 << HDMI_IRQ_HPD_UNPLUG, reg_sys + 0x04);
		writeb(1 << HDMI_IRQ_HPD_PLUG, reg_sys + 0x04);
		return IRQ_HANDLED;
	}

	if (flag == (1 << HDMI_IRQ_HPD_PLUG | 1 << HDMI_IRQ_HPD_UNPLUG)) {

		if (last_hpd_state == HPD_HI && readb(reg_sys + 0x0C))
			flag = 1 << HDMI_IRQ_HPD_UNPLUG;
		else
			flag = 1 << HDMI_IRQ_HPD_PLUG;
	}

	if (flag & (1 << HDMI_IRQ_HPD_PLUG)) {

		hdata->hpd = true;

		/* clear pending bit */
		writeb(1 << HDMI_IRQ_HPD_PLUG, reg_sys + 0x04);

		/* enable HDMI_IRQ_HPD_UNPLUG interrupt */
		reg = readb(reg_sys + 0x00);
		reg |= 1 << HDMI_IRQ_HPD_UNPLUG;

		last_hpd_state = HPD_HI;
		writeb(reg, reg_sys + 0x00);

	} else if (flag & (1 << HDMI_IRQ_HPD_UNPLUG)) {

		hdata->hpd = false;

		/* clear pending bit */
		writeb(1 << HDMI_IRQ_HPD_UNPLUG, reg_sys + 0x04);

		/* disable HDMI_IRQ_HPD_UNPLUG interrupt */
		reg = readb(reg_sys + 0x00);
		reg &= ~(1 << HDMI_IRQ_HPD_UNPLUG);

		last_hpd_state = HPD_LO;
		writeb(reg, reg_sys + 0x00);
	}
	if (ctx->drm_dev)
		drm_helper_hpd_irq_event(ctx->drm_dev);

#if defined(CONFIG_HDMI_HPD)
	hdmi_detect_changed = 1;
	wake_up_interruptible(&hdmi_hpd_wait);
#endif
	return IRQ_HANDLED;
}
#endif

void change_hdmi_setting(unsigned long value)
{
	struct drime4_drm_hdmi_pdata *pdata	= hdmi_pdev->dev.platform_data;
	struct drime4_drm_hdmi_context *ctx = platform_get_drvdata(hdmi_pdev);
	struct hdmi_context *hdata;

	if (NULL == ctx)
		return;

	hdata = ctx->ctx;

	switch (value)
	{
	case 0:
		hdata->hpd = gpio_get_value(pdata->hpd_gpio);		
		break;
	case 1:
		hdata->hpd = 0;
		break;
	case 2:
		hdata->hpd = 1;
		break;
	default:
		printk(KERN_ALERT"g_change_hdmi_value error\n\n");
	}

	if (g_prev_hdmi_hpd_state != hdata->hpd) {
		g_prev_hdmi_hpd_state = hdata->hpd;

		if (ctx->drm_dev)
			drm_helper_hpd_irq_event(ctx->drm_dev);

#if defined(CONFIG_HDMI_HPD)
		hdmi_detect_changed = hdata->hpd + 1;
		wake_up_interruptible(&hdmi_hpd_wait);
#endif
		g_change_hdmi_value = 3;
	}
}

static int d4_hdmi_suspend(struct platform_device *pdev, pm_message_t state)
{
	return _d4_hdmi_suspend(pdev);
}

static int d4_hdmi_resume(struct platform_device *pdev)
{
	return _d4_hdmi_resume(pdev);
}


struct platform_driver drime4_hdmi_driver = {
		.probe = d4_hdmi_probe,
		.remove = d4_hdmi_remove,
		.suspend = d4_hdmi_suspend,
		.resume = d4_hdmi_resume,
		.driver = {
				.name = "drime4_hdmi",
				.owner = THIS_MODULE,
		},
};

