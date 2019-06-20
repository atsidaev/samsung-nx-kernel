/* linux/arch/arm/mach-drime4/include/mach/devs.h
 *
 * Header file for  DRIME4 standard platform devices
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#ifndef	__PLAT_DRIME4_DEVS_H
#define	__PLAT_DRIME4_DEVS_H

#include <linux/platform_device.h>

extern struct platform_device drime4_device_padmux;
extern struct platform_device drime4_device_i2s0;
extern struct platform_device drime4_device_i2s2;
extern struct platform_device drime4_asoc_dma;

#ifdef CONFIG_DRIME4_SPI0
extern struct platform_device drime4_device_spi0;
#endif
#ifdef CONFIG_DRIME4_SPI1
extern struct platform_device drime4_device_spi1;
#endif

#ifdef CONFIG_DRM_DRIME4_DP_TV
extern struct platform_device drime4_device_tv;
extern struct platform_device drime4_device_hdmi;
extern struct platform_device drime4_device_cec;
extern struct platform_device drime4_drm_hdmi_device;
extern struct platform_device drime4_device_alsa_hdmi;
#endif

#ifdef CONFIG_DRIME4_SPI2
extern struct platform_device drime4_device_spi2;
#endif
#ifdef CONFIG_DRIME4_SPI3
extern struct platform_device drime4_device_spi3;
#endif

#ifdef CONFIG_DRIME4_SPI4
extern struct platform_device drime4_device_spi4;
#endif

#ifdef CONFIG_DRIME4_SPI5
extern struct platform_device drime4_device_spi5;
#endif

#ifdef CONFIG_DRIME4_SPI6
extern struct platform_device drime4_device_spi6;
#endif

#ifdef CONFIG_DRIME4_SPI7
extern struct platform_device drime4_device_spi7;
#endif

#ifdef CONFIG_FB_DRIME4
extern struct platform_device drime4_device_fb;
#endif

#ifdef CONFIG_DRIME4_PWM
extern struct platform_device d4_device_pwm[];
#endif

#ifdef CONFIG_DRIME4_PTC
extern struct platform_device d4_device_ptc[];
#endif

#ifdef CONFIG_DRIME4_DEV_RMU
extern struct platform_device drime4_device_rmu;
#endif

#ifdef CONFIG_DRIME4_ADC
extern struct platform_device drime4_device_adc;
#endif

#ifdef CONFIG_UDD_GROUP
extern struct platform_device gpio_udevice[];
#endif

#ifdef CONFIG_DRIME4_PMU
extern struct platform_device d4_device_pmu;
#endif

#ifdef CONFIG_DRIME4_RTC
extern struct platform_device drime4_device_rtc;
#endif

#ifdef CONFIG_MMC_DW
#ifdef CONFIG_MMC_DW
extern struct platform_device drime4_device_sdmmc;
#endif
#ifdef CONFIG_DRIME4_EMMC
extern struct platform_device drime4_device_emmc;
#endif
#ifdef CONFIG_DRIME4_SDIO
extern struct platform_device drime4_device_sdio;
#endif
#endif


#ifdef CONFIG_DRIME4_BMA
extern struct platform_device drime4_device_bma;
#endif

#ifdef CONFIG_DRIME4_SMA
extern struct platform_device drime4_device_sma;
#endif

#ifdef CONFIG_DRIME4_PP_CORE
extern struct platform_device drime4_device_pp_core;
#endif

#ifdef CONFIG_DRIME4_MIPI
extern struct platform_device drime4_device_mipi;
#endif

#ifdef CONFIG_DRIME4_PP_3A
extern struct platform_device drime4_device_pp_3a;
#endif

#ifdef CONFIG_DRIME4_PP_SSIF
extern struct platform_device drime4_device_pp_ssif;
#endif

#ifdef CONFIG_DRIME4_JPEG
extern struct platform_device drime4_device_jpeg;
#endif

#ifdef CONFIG_DRIME4_JPEG_XR
extern struct platform_device drime4_device_jxr;
#endif

#ifdef CONFIG_DRIME4_SRP
extern struct platform_device drime4_device_srp;
#endif

#ifdef CONFIG_DRIME4_IPCM
extern struct platform_device drime4_device_ipcm;
#endif

#ifdef CONFIG_DRIME4_IPCS
extern struct platform_device drime4_device_ipcs;
#endif

#ifdef CONFIG_DRIME4_HDMI
extern struct platform_device drime4_device_hdmi;
extern struct platform_device drime4_device_hdmi_cec;
extern struct platform_device drime4_device_hdmi_hpd;
extern struct platform_device drime4_device_alsa_hdmi;
#endif
#ifdef CONFIG_DRIME4_EP
extern struct platform_device drime4_device_ep;
#endif

#ifdef CONFIG_DRIME4_USB3_DRD
extern struct platform_device drime4_device_usb_dwc3;
extern struct platform_device s3c_device_android_usb;
extern struct platform_device s3c_device_usb_mass_storage;
#endif
#ifdef CONFIG_STROBE_CONTROLLER
extern struct platform_device drime4_str_device;
#endif
#if defined(CONFIG_HT_PWM4)
extern struct platform_device d4_device_dr_ioctl;
#endif
#if defined(CONFIG_JACK_MON)
extern struct platform_device d4_jack;
#endif
#ifdef CONFIG_PMU_SELECT
extern struct platform_device drime4_device_opener;
#endif

#ifdef CONFIG_DRIME4_BE
extern struct platform_device drime4_device_be;
#endif

#ifdef CONFIG_RIME4_RTC
extern struct platform_device drime4_device_rtc;
#endif

#ifdef CONFIG_VIDEO_MFC5X
extern struct platform_device drime4_device_mfc;
#endif

#ifdef CONFIG_DRIME4_BWM
extern struct platform_device drime4_device_bwm;
#endif

#ifdef CONFIG_MTD_NAND_DRIME4
extern struct platform_device drime4_device_nand;
#endif

/* HSIC_SISO */
#ifdef CONFIG_USB_DWCOTG
extern struct platform_device drime4_device_hsic;
#endif

#ifdef CONFIG_DRIME4_GPU
extern struct platform_device powervr_device;
#endif

#ifdef CONFIG_DRM_DRIME4
extern struct platform_device drime4_device_drm;
#endif

#ifdef CONFIG_DRIME4_SI2C
extern struct platform_device drime4_device_si2c[];
#endif

#ifdef CONFIG_DRIME4_CSM
extern struct platform_device drime4_device_csm;
#endif

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
/* soonyong.cho : Define samsung product id and config string.
 *                Sources such as 'android.c' and 'devs.c' refered below define
 */
#  define SAMSUNG_VENDOR_ID		0x04e8

#  ifdef CONFIG_USB_ANDROID_SAMSUNG_ESCAPE
	/* USE DEVGURU HOST DRIVER */
	/* 0x6860 : MTP(0) + MS Composite (UMS) */
	/* 0x685E : UMS(0) + MS Composite (ADB) */
#  ifdef CONFIG_USB_ANDROID_SAMSUNG_KIES_UMS
#    define SAMSUNG_KIES_PRODUCT_ID	0x685e	/* ums(0) + acm(1,2) */
#  else
#    define SAMSUNG_KIES_PRODUCT_ID	0x6860	/* mtp(0) + acm(1,2) */
#  endif
#    define SAMSUNG_DEBUG_PRODUCT_ID	0x685e	/* ums(0) + acm(1,2) + adb(3) (with MS Composite) */
#    define SAMSUNG_UMS_PRODUCT_ID	0x685B  /* UMS Only */
#    define SAMSUNG_MTP_PRODUCT_ID	0x685C  /* MTP Only */
#    ifdef CONFIG_USB_ANDROID_SAMSUNG_RNDIS_WITH_MS_COMPOSITE
#      define SAMSUNG_RNDIS_PRODUCT_ID	0x6861  /* RNDIS(0,1) + UMS (2) + MS Composite */
#    else
#      define SAMSUNG_RNDIS_PRODUCT_ID	0x6863  /* RNDIS only */
#    endif
#  ifdef CONFIG_USB_ANDROID_ECM
#    define       SAMSUNG_ECM_PRODUCT_ID	0x685D /* ECM only */
#  endif
#    define ANDROID_DEBUG_CONFIG_STRING	 "UMS + ACM + ADB (Debugging mode)"
#  ifdef CONFIG_USB_ANDROID_SAMSUNG_KIES_UMS
#    define ANDROID_KIES_CONFIG_STRING	 "UMS + ACM (SAMSUNG KIES mode)"
#  else
#	ifdef CONFIG_USB_ANDROID_SAMSUNG_SDB
#    define ANDROID_KIES_CONFIG_STRING	 "MTP + SDB + ACM (SAMSUNG KIES mode)"
#	else
#	 define ANDROID_KIES_CONFIG_STRING	 "MTP + ACM (SAMSUNG KIES mode)"
#	endif
#  endif
#  else /* USE MCCI HOST DRIVER */
#    define SAMSUNG_KIES_PRODUCT_ID	0x6877	/* Shrewbury ACM+MTP*/
#    define SAMSUNG_DEBUG_PRODUCT_ID	0x681C	/* Shrewbury ACM+UMS+ADB*/
#    define SAMSUNG_UMS_PRODUCT_ID	0x681D
#    define SAMSUNG_MTP_PRODUCT_ID	0x68A9
#    define SAMSUNG_RNDIS_PRODUCT_ID	0x6863
#    define ANDROID_DEBUG_CONFIG_STRING	 "ACM + UMS + ADB (Debugging mode)"
#    define ANDROID_KIES_CONFIG_STRING	 "ACM + MTP (SAMSUNG KIES mode)"
#  endif
#  define       ANDROID_UMS_CONFIG_STRING	 "UMS Only (Not debugging mode)"
#  define       ANDROID_MTP_CONFIG_STRING	 "MTP Only (Not debugging mode)"
#  ifdef CONFIG_USB_ANDROID_SAMSUNG_RNDIS_WITH_MS_COMPOSITE
#    define       ANDROID_RNDIS_CONFIG_STRING	 "RNDIS + UMS (Not debugging mode)"
#  else
#    define       ANDROID_RNDIS_CONFIG_STRING	 "RNDIS Only (Not debugging mode)"
#  endif
#  ifdef CONFIG_USB_ANDROID_ECM
#    define       ANDROID_ECM_CONFIG_STRING	 "ECM Only (Not debugging mode)"
#  endif
	/* Refered from S1, P1 */
#  define USBSTATUS_UMS				0x0
#  define USBSTATUS_SAMSUNG_KIES 		0x1
#  define USBSTATUS_MTPONLY			0x2
#  define USBSTATUS_ASKON			0x4
#  define USBSTATUS_VTP				0x8
#  define USBSTATUS_ADB				0x10
#  define USBSTATUS_DM				0x20
#  define USBSTATUS_ACM				0x40
#  define USBSTATUS_SAMSUNG_KIES_REAL		0x80
#ifdef CONFIG_USB_ANDROID_ECM
#  define USBSTATUS_ECM				0x100
#endif
/* soonyong.cho : This is for setting unique serial number */
void __init s3c_usb_set_serial(void);
#endif /* CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE */

#endif /* __PLAT_DRIME4_DEVS_H */
