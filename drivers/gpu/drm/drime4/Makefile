#
# Makefile for the drm device driver.  This driver provides support for the
# Direct Rendering Infrastructure (DRI) in XFree86 4.1.0 and higher.

ccflags-y := -Iinclude/drm -Idrivers/gpu/drm/drime4
drime4drm-y := drime4_drm_drv.o drime4_drm_encoder.o drime4_drm_connector.o \
		drime4_drm_crtc.o drime4_drm_fbdev.o drime4_drm_fb.o \
		drime4_drm_buf.o drime4_drm_gem.o drime4_drm_core.o \
		drime4_drm_plane.o

drime4drm-$(CONFIG_DRM_DRIME4_DMABUF) += drime4_drm_dmabuf.o
drime4drm-$(CONFIG_DRM_DRIME4_DP_LCD)	+= drime4_drm_dp_lcd.o
drime4drm-$(CONFIG_DRM_DRIME4_DP_SUBLCD)	+= drime4_drm_dp_sublcd.o
drime4drm-$(CONFIG_DRM_DRIME4_DP_LCD)	+= drime4_dp_ioctl.o
drime4drm-$(CONFIG_DRM_DRIME4_DP_TV)	+= drime4_hdmi.o drime4_tv.o \
					   drime4_ddc.o drime4_hdmiphy.o \
					   drime4_drm_hdmi.o \
					   drime4_cec.o
drime4drm-$(CONFIG_DRM_DRIME4_VIDI)	+= drime4_drm_vidi.o
drime4drm-$(CONFIG_DRM_DRIME4_G2D)	+= drime4_drm_g2d.o

obj-$(CONFIG_DRM_DRIME4)		+= drime4drm.o
