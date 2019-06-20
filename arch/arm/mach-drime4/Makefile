## arch/arm/mach-drime4/Makefile
#
# Copyright (c) 2011 Samsung Electronics Co., Ltd.
#		http://www.samsung.com/
#
# Licensed under GPLv2

#Core support for DRIME4 system
obj-${CONFIG_CPU_DRIME4}			+= common.o irq.o io.o gpio.o dma.o dev-padmux.o dev-i2c.o
obj-${CONFIG_DRIME4_MI2C} 			+= dev-i2c.o 
obj-${CONFIG_DRIME4_SI2C}			+= dev-si2c.o
obj-y						+= clock.o
obj-y						+= hr-timer.o

#Machine support
obj-${CONFIG_MACH_D4JIG2ND}			+= board-d4jig2nd.o
obj-${CONFIG_MACH_D4_NX300}			+= board-d4-nx300.o version_information.o
obj-${CONFIG_MACH_D4_NX2000}			+= board-d4-nx2000.o version_information_nx2000.o
obj-${CONFIG_MACH_D4_GALAXYNX}			+= board-d4-galaxynx.o version_information_galaxynx.o
obj-${CONFIG_MACH_D4_GALAXYNX_SIMULATOR}	+= board-d4-galaxynx-simulator.o version_information_galaxynx_simulator.o
obj-${CONFIG_CMA}				+= d4-cma.o
obj-y						+= d4_mem.o
#Power Management
obj-${CONFIG_PM}				+= pm.o sleep.o cache.o
obj-${CONFIG_CPU_IDLE}				+= cpuidle.o
#Device support
obj-y						+= dev-audio.o
obj-${CONFIG_DRIME4_USB3_DRD}			+= dev-usb-dwc3.o
obj-${CONFIG_DRIME4_BMA}			+= dev-bma.o
obj-${CONFIG_DRIME4_SMA}			+= dev-sma.o

obj-${CONFIG_DRIME4_PP_CORE}			+= dev-pp-core.o
obj-${CONFIG_DRIME4_MIPI}			+= dev-mipi.o
obj-${CONFIG_DRIME4_PP_3A}			+= dev-pp-3a.o
obj-${CONFIG_DRIME4_PP_SSIF}			+= dev-pp-ssif.o
obj-${CONFIG_DRIME4_IPCS}				+= dev-ipcs.o
obj-${CONFIG_DRIME4_IPCM}				+= dev-ipcm.o
obj-${CONFIG_DRIME4_EP}				+= dev-ep.o
obj-${CONFIG_PMU_SELECT}			+= dev-opener.o
obj-${CONFIG_DRIME4_BE}				+= dev-be.o
obj-${CONFIG_DRM_DRIME4_DP_TV}			+= dev-cec.o
obj-${CONFIG_DRIME4_HDMI}			+= dev-hpd.o
#obj-${CONFIG_DRIME4_HDMI}			+= dev-alsa_hdmi.o
obj-${CONFIG_DRIME4_SRP}                        += dev-srp.o
obj-${CONFIG_DRIME4_LCD_PANEL_MANAGER}		+= dev-lcd-panel-manager.o
obj-${CONFIG_DRM_DRIME4_DP_TV}			+= dev-tv.o dev-hdmi.o
obj-${CONFIG_DRM_DRIME4_DP_TV}			+= dev-alsa_hdmi.o

obj-${CONFIG_DRIME4_BWM}			+= dev-bwm.o

obj-${CONFIG_STROBE_CONTROLLER}                        += dev-str.o
obj-${CONFIG_JACK_MON}                        += dev-jack.o

obj-${CONFIG_DRIME4_JPEG}			+= dev-jpeg.o
obj-${CONFIG_SHUTTER}				+= dev-shutter.o
obj-${CONFIG_DRIME4_JPEG_XR}			+= dev-jxr.o
obj-${CONFIG_MTD_NAND_DRIME4}			+= dev-nand.o
obj-${CONFIG_VIDEO_MFC5X}			+= dev-mfc.o
obj-${CONFIG_DRIME4_SPI0}                       += dev-spi0.o
obj-${CONFIG_DRIME4_SPI1}                       += dev-spi1.o
obj-${CONFIG_DRIME4_SPI2}                       += dev-spi2.o
obj-${CONFIG_DRIME4_SPI3}                       += dev-spi3.o
obj-${CONFIG_DRIME4_SPI4}                       += dev-spi4.o
obj-${CONFIG_DRIME4_SPI5}                       += dev-spi5.o
obj-${CONFIG_DRIME4_SPI6}                       += dev-spi6.o
obj-${CONFIG_DRIME4_SPI7}                       += dev-spi7.o
obj-${CONFIG_HT_PWM4}				+= d4-dr.o
obj-${CONFIG_DRIME4_PWM}			+= dev-pwm.o
obj-${CONFIG_DRIME4_ADC}                        += dev-adc.o adc.o adc_ioctl.o

obj-${CONFIG_DRIME4_DEV_RMU}			+= dev-rmu.o

obj-${CONFIG_DRIME4_PMU}	               += dev-pmu.o

obj-${CONFIG_UDD_GROUP}				+= dev-ugpio.o
obj-${CONFIG_DRIME4_RTC}                        += dev-rtc.o
obj-${CONFIG_DRIME4_PTC}			+= dev-ptc.o
obj-${CONFIG_DRIME4_GPU}			+= dev-gpu.o

obj-${CONFIG_MMC_DW}				+= dev-sdmmc.o
obj-${CONFIG_DRIME4_SDIO}                       += dev-sdio.o
obj-${CONFIG_DRIME4_EMMC}			+= dev-emmc.o
obj-$(CONFIG_MMC_DW_SYSFS)			+= dw_mmc_sysfs.o
obj-$(CONFIG_DRIME4_DEV_DRM)			+= dev-drm.o

obj-$(CONFIG_FB_DRIME4)				+= dev-fb.o

#obj-${CONFIG_DWC_OTG}				+= dev-hsic.o
obj-${CONFIG_USB_DWCOTG}			+= dev-hsic.o

obj-${CONFIG_DRIME4_CSM}			+= dev-csm.o
obj-${CONFIG_DRIME4_BV}             += d4_utility_ioctl.o
