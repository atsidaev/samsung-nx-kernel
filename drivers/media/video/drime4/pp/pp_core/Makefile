#
# Makefile for the DRIMe4 PP core drivers
#

obj-$(CONFIG_DRIME4_PP_CORE) += d4_pp_core_dd.o d4_pp_core.o d4_pp_core_ioctl.o

ifeq ($(CONFIG_DRIME4_PP_V4L2),)
	obj-$(CONFIG_DRIME4_PP_CORE) += d4_pp_core.o d4_pp_core_ioctl.o
endif
