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
#include "drm.h"
#include "drm_crtc_helper.h"

#include <drm/drime4_drm.h>

#include "drime4_drm_drv.h"
#include "drime4_drm_crtc.h"
#include "drime4_drm_encoder.h"
#include "drime4_drm_fbdev.h"
#include "drime4_drm_fb.h"
#include "drime4_drm_gem.h"
#include "drime4_drm_plane.h"
#include "drime4_drm_vidi.h"
#include "drime4_drm_dmabuf.h"
#include "drime4_drm_g2d.h"
#include "drime4_drm_dp_lcd.h"

#define DRIVER_NAME	"drime4"
#define DRIVER_DESC	"Samsung SoC DRM"
#define DRIVER_DATE	"20110530"
#define DRIVER_MAJOR	1
#define DRIVER_MINOR	0

#define VBLANK_OFF_DELAY	50000

static int drime4_drm_load(struct drm_device *dev, unsigned long flags)
{
	struct drime4_drm_private *private;
	int ret;
	int nr;

	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	private = kzalloc(sizeof(struct drime4_drm_private), GFP_KERNEL);
	if (!private) {
		DRM_ERROR("failed to allocate private\n");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&private->pageflip_event_list);
	dev->dev_private = (void *)private;

	drm_mode_config_init(dev);

	/* init kms poll for handling hpd */
	drm_kms_helper_poll_init(dev);

	drime4_drm_mode_config_init(dev);

	/*
	 * DRIME44 is enough to have two CRTCs and each crtc would be used
	 * without dependency of hardware.
	 */
	for (nr = 0; nr < MAX_CRTC; nr++) {
		ret = drime4_drm_crtc_create(dev, nr);
		if (ret)
			goto err_crtc;
	}

	for (nr = 0; nr < MAX_PLANE; nr++) {
		ret = drime4_plane_init(dev, nr);
		if (ret)
			goto err_crtc;
	}

	ret = drm_vblank_init(dev, MAX_CRTC);
	if (ret)
		goto err_crtc;

	/*
	 * probe sub drivers such as display controller and hdmi driver,
	 * that were registered at probe() of platform driver
	 * to the sub driver and create encoder and connector for them.
	 */
	ret = drime4_drm_device_register(dev);
	if (ret)
		goto err_vblank;

	/* setup possible_clones. */
	drime4_drm_encoder_setup(dev);

	/*
	 * create and configure fb helper and also drime4 specific
	 * fbdev object.
	 */
	ret = drime4_drm_fbdev_init(dev);
	if (ret) {
		DRM_ERROR("failed to initialize drm fbdev\n");
		goto err_drm_device;
	}

	drm_vblank_offdelay = VBLANK_OFF_DELAY;

	return 0;

err_drm_device:
	drime4_drm_device_unregister(dev);
err_vblank:
	drm_vblank_cleanup(dev);
err_crtc:
	drm_mode_config_cleanup(dev);
	kfree(private);

	return ret;
}

static int drime4_drm_unload(struct drm_device *dev)
{
	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	drime4_drm_fbdev_fini(dev);
	drime4_drm_device_unregister(dev);
	drm_vblank_cleanup(dev);
	drm_kms_helper_poll_fini(dev);
	drm_mode_config_cleanup(dev);
	kfree(dev->dev_private);

	dev->dev_private = NULL;

	return 0;
}

static int drime4_drm_open(struct drm_device *dev, struct drm_file *file)
{
	struct drm_drime4_file_private *file_priv;

	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	file_priv = kzalloc(sizeof(*file_priv), GFP_KERNEL);
	if (!file_priv)
		return -ENOMEM;

	drm_prime_init_file_private(&file->prime);
	file->driver_priv = file_priv;

	return drime4_drm_subdrv_open(dev, file);
}

static void drime4_drm_preclose(struct drm_device *dev,
					struct drm_file *file)
{
	struct drime4_drm_private *private = dev->dev_private;
	struct drm_pending_vblank_event *e, *t;
	unsigned long flags;

	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	/* release events of current file */
	spin_lock_irqsave(&dev->event_lock, flags);
	list_for_each_entry_safe(e, t, &private->pageflip_event_list,
			base.link) {
		if (e->base.file_priv == file) {
			list_del(&e->base.link);
			e->base.destroy(&e->base);
		}
	}
	drm_prime_destroy_file_private(&file->prime);
	spin_unlock_irqrestore(&dev->event_lock, flags);

	drime4_drm_subdrv_close(dev, file);
}

static void drime4_drm_postclose(struct drm_device *dev, struct drm_file *file)
{
	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	if (!file->driver_priv)
		return;

	kfree(file->driver_priv);
	file->driver_priv = NULL;
}

static void drime4_drm_lastclose(struct drm_device *dev)
{
	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	drime4_drm_fbdev_restore_mode(dev);
}

static const struct vm_operations_struct drime4_drm_gem_vm_ops = {
	.fault = drime4_drm_gem_fault,
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};

static struct drm_ioctl_desc drime4_ioctls[] = {
	DRM_IOCTL_DEF_DRV(DRIME4_GEM_CREATE, drime4_drm_gem_create_ioctl,
			DRM_UNLOCKED | DRM_AUTH),
	DRM_IOCTL_DEF_DRV(DRIME4_GEM_MAP_OFFSET,
			drime4_drm_gem_map_offset_ioctl, DRM_UNLOCKED |
			DRM_AUTH),
	DRM_IOCTL_DEF_DRV(DRIME4_GEM_MMAP,
			drime4_drm_gem_mmap_ioctl, DRM_UNLOCKED | DRM_AUTH),
	DRM_IOCTL_DEF_DRV(DRIME4_GEM_GET,
			drime4_drm_gem_get_ioctl, DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(DRIME4_GEM_GET_PHY,
			drime4_drm_gem_get_phy_ioctl, DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(DRIME4_PLANE_SET_ZPOS, drime4_plane_set_zpos_ioctl,
			DRM_UNLOCKED | DRM_AUTH),
	DRM_IOCTL_DEF_DRV(DRIME4_VIDI_CONNECTION,
			vidi_connection_ioctl, DRM_UNLOCKED | DRM_AUTH),
	DRM_IOCTL_DEF_DRV(DRIME4_G2D_GET_VER,
			drime4_g2d_get_ver_ioctl, DRM_UNLOCKED | DRM_AUTH),
	DRM_IOCTL_DEF_DRV(DRIME4_G2D_SET_CMDLIST,
			drime4_g2d_set_cmdlist_ioctl, DRM_UNLOCKED | DRM_AUTH),
	DRM_IOCTL_DEF_DRV(DRIME4_G2D_EXEC,
			drime4_g2d_exec_ioctl, DRM_UNLOCKED | DRM_AUTH),
};

static const struct file_operations drime4_drm_driver_fops = {
	.owner		= THIS_MODULE,
	.open		= drm_open,
	.mmap		= drime4_drm_gem_mmap,
	.poll		= drm_poll,
	.read		= drm_read,
	.unlocked_ioctl	= drm_ioctl,
	.release	= drm_release,
};

static struct drm_driver drime4_drm_driver = {
	.driver_features	= DRIVER_HAVE_IRQ | DRIVER_MODESET |
					DRIVER_GEM | DRIVER_PRIME,
	.load			= drime4_drm_load,
	.unload			= drime4_drm_unload,
	.open			= drime4_drm_open,
	.preclose		= drime4_drm_preclose,
	.lastclose		= drime4_drm_lastclose,
	.postclose		= drime4_drm_postclose,
	.get_vblank_counter	= drm_vblank_count,
	.enable_vblank		= drime4_drm_crtc_enable_vblank,
	.disable_vblank		= drime4_drm_crtc_disable_vblank,
	.gem_init_object	= drime4_drm_gem_init_object,
	.gem_free_object	= drime4_drm_gem_free_object,
	.gem_vm_ops		= &drime4_drm_gem_vm_ops,
	.dumb_create		= drime4_drm_gem_dumb_create,
	.dumb_map_offset	= drime4_drm_gem_dumb_map_offset,
	.dumb_destroy		= drime4_drm_gem_dumb_destroy,
	.prime_handle_to_fd	= drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle	= drm_gem_prime_fd_to_handle,
	.gem_prime_export	= drime4_dmabuf_prime_export,
	.gem_prime_import	= drime4_dmabuf_prime_import,
	.ioctls			= drime4_ioctls,
	.fops			= &drime4_drm_driver_fops,
	.name	= DRIVER_NAME,
	.desc	= DRIVER_DESC,
	.date	= DRIVER_DATE,
	.major	= DRIVER_MAJOR,
	.minor	= DRIVER_MINOR,
};

static int drime4_drm_platform_probe(struct platform_device *pdev)
{
	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	drime4_drm_driver.num_ioctls = DRM_ARRAY_SIZE(drime4_ioctls);

	return drm_platform_init(&drime4_drm_driver, pdev);
}

static int drime4_drm_platform_remove(struct platform_device *pdev)
{
	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	drm_platform_exit(&drime4_drm_driver, pdev);

	return 0;
}

static struct platform_driver drime4_drm_platform_driver = {
	.probe		= drime4_drm_platform_probe,
	.remove		= __devexit_p(drime4_drm_platform_remove),
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "drime4-drm",
	},
};

static int __init drime4_drm_init(void)
{
	int ret;

	DRM_DEBUG_DRIVER("%s\n", __FILE__);

#ifdef CONFIG_DRM_DRIME4_DP_LCD
	ret = platform_driver_register(&dp_lcd_driver);
	if (ret < 0)
		goto out_dp_lcd;
#endif

#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
	ret = platform_driver_register(&dp_sublcd_driver);
	if (ret < 0) {
		goto out_dp_lcd;
	}
#endif


#ifdef CONFIG_DRM_DRIME4_DP_TV
	ret = platform_driver_register(&dp_tv_driver);
	if (ret < 0)
		goto out_mixer;
	ret = platform_driver_register(&drime4_hdmi_driver);
	if (ret < 0)
		goto out_hdmi;
	ret = platform_driver_register(&drime4_cec_driver);
	if (ret < 0)
	    goto out_hdmi;
	ret = platform_driver_register(&drime4_drm_common_hdmi_driver);
	if (ret < 0)
		goto out_common_hdmi;
#endif

#ifdef CONFIG_DRM_DRIME4_VIDI
	ret = platform_driver_register(&vidi_driver);
	if (ret < 0)
		goto out_vidi;
#endif

#ifdef CONFIG_DRM_DRIME4_G2D
	ret = platform_driver_register(&g2d_driver);
	if (ret < 0)
		goto out_g2d;
#endif

	ret = platform_driver_register(&drime4_drm_platform_driver);
	if (ret < 0)
		goto out;

	return 0;

out:
#ifdef CONFIG_DRM_DRIME4_G2D
	platform_driver_unregister(&g2d_driver);
out_g2d:
#endif

#ifdef CONFIG_DRM_DRIME4_VIDI
out_vidi:
	platform_driver_unregister(&vidi_driver);
#endif

#ifdef CONFIG_DRM_DRIME4_DP_TV
	platform_driver_unregister(&drime4_drm_common_hdmi_driver);
out_common_hdmi:

out_mixer:
	platform_driver_unregister(&dp_tv_driver);

out_hdmi:
	platform_driver_unregister(&drime4_hdmi_driver);
	platform_driver_unregister(&drime4_cec_driver);
#endif

#ifdef CONFIG_DRM_DRIME4_DP_LCD
	platform_driver_unregister(&dp_lcd_driver);
#endif

#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
	platform_driver_unregister(&dp_sublcd_driver);
#endif
 out_dp_lcd:

	return ret;
}

static void __exit drime4_drm_exit(void)
{
	DRM_DEBUG_DRIVER("%s\n", __FILE__);

	platform_driver_unregister(&drime4_drm_platform_driver);

#ifdef CONFIG_DRM_DRIME4_G2D
	platform_driver_unregister(&g2d_driver);
#endif

#ifdef CONFIG_DRM_DRIME4_DP_TV
	platform_driver_unregister(&drime4_drm_common_hdmi_driver);
	platform_driver_unregister(&dp_tv_driver);
	platform_driver_unregister(&drime4_hdmi_driver);
	platform_driver_unregister(&drime4_cec_driver);
#endif

#ifdef CONFIG_DRM_DRIME4_VIDI
	platform_driver_unregister(&vidi_driver);
#endif

#ifdef CONFIG_DRM_DRIME4_DP_LCD
	platform_driver_unregister(&dp_lcd_driver);
#endif
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(drime4_drm_init);
#else
fast_late_initcall(drime4_drm_init);
#endif
module_exit(drime4_drm_exit);

MODULE_AUTHOR("Inki Dae <inki.dae@samsung.com>");
MODULE_AUTHOR("Joonyoung Shim <jy0922.shim@samsung.com>");
MODULE_AUTHOR("Seung-Woo Kim <sw0312.kim@samsung.com>");
MODULE_DESCRIPTION("Samsung SoC DRM Driver");
MODULE_LICENSE("GPL");
