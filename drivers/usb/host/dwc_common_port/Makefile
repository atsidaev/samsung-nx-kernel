#
# Makefile for DWC_common library
#

ifneq ($(KERNELRELEASE),)

ccflags-$(CONFIG_USB_DWCOTG) += -DDWC_LINUX
ccflags-$(CONFIG_USB_DWCOTG) += -DDEBUG
ccflags-$(CONFIG_USB_DWCOTG) += -DDWC_DEBUG_REGS
#ccflags-$(CONFIG_USB_DWCOTG) += -DDWC_DEBUG_MEMORY

ccflags-$(CONFIG_USB_DWCOTG) += -DDWC_LIBMODULE
ccflags-$(CONFIG_USB_DWCOTG) += -DDWC_CCLIB
#ccflags-$(CONFIG_USB_DWCOTG) += -DDWC_CRYPTOLIB
ccflags-$(CONFIG_USB_DWCOTG) += -DDWC_NOTIFYLIB
ccflags-$(CONFIG_USB_DWCOTG) += -DDWC_UTFLIB

obj-$(CONFIG_USB_DWCOTG)	+= dwc_common_port_lib.o
dwc_common_port_lib-objs := dwc_cc.o dwc_modpow.o dwc_dh.o \
			    dwc_crypto.o dwc_notifier.o \
			    dwc_common_linux.o dwc_mem.o

kernrelwd := $(subst ., ,$(KERNELRELEASE))
kernrel3 := $(word 1,$(kernrelwd)).$(word 2,$(kernrelwd)).$(word 3,$(kernrelwd))
 
ifneq ($(kernrel3),2.6.20)
# grayg - I only know that we use EXTRA_CFLAGS in 2.6.31 actually
EXTRA_CFLAGS += $(CPPFLAGS)
endif

else

ifeq ($(KDIR),)
$(error Must give "KDIR=/path/to/kernel/source" on command line or in environment)
endif

ifeq ($(ARCH),)
$(error Must give "ARCH=<arch>" on command line or in environment. Also, if \
 cross-compiling, must give "CROSS_COMPILE=/path/to/compiler/plus/tool-prefix-")
endif

ifeq ($(DOXYGEN),)
DOXYGEN		:= doxygen
endif

default:
	$(MAKE) -C$(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

docs:	$(wildcard *.[hc]) doc/doxygen.cfg
	$(DOXYGEN) doc/doxygen.cfg

tags:	$(wildcard *.[hc])
	$(CTAGS) -e $(wildcard *.[hc]) $(wildcard linux/*.[hc]) $(wildcard $(KDIR)/include/linux/usb*.h)

endif

clean:
	rm -rf *.o *.ko .*.cmd *.mod.c .*.o.d .*.o.tmp modules.order Module.markers Module.symvers .tmp_versions/
