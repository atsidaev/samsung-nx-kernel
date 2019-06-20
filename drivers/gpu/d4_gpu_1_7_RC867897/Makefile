#
# Makefile for the DRIMe4 GPU drivers
#

#include ./drivers/media/video/drime4/gpu/config.mk

ccflags-y += \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/include4/ \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/services4/include/ \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/services4/system/include/ \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/services4/system/drime4_slp/ \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/services4/srvkm/bridged/ \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/services4/srvkm/bridged/sgx/ \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/services4/srvkm/common/ \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/services4/srvkm/devices/sgx/ \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/services4/srvkm/env/linux/ \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/services4/srvkm/hwdefs/ \
	-I./drivers/gpu/d4_gpu_1_7_RC867897/services4/srvkm/include/ \
	-I./drivers/video/drime4/lcd/ 

obj-$(CONFIG_DRIME4_GPU_1_7_RC867897) += \
	services4/srvkm/env/linux/module.o \
	services4/srvkm/env/linux/osfunc.o \
	services4/srvkm/env/linux/mutils.o \
	services4/srvkm/env/linux/mmap.o \
	services4/srvkm/env/linux/pdump.o \
	services4/srvkm/env/linux/proc.o \
	services4/srvkm/env/linux/pvr_bridge_k.o \
	services4/srvkm/env/linux/pvr_debug.o \
	services4/srvkm/env/linux/mm.o \
	services4/srvkm/env/linux/mutex.o \
	services4/srvkm/env/linux/event.o \
	services4/srvkm/env/linux/osperproc.o \
	services4/srvkm/common/buffer_manager.o \
	services4/srvkm/common/devicemem.o \
	services4/srvkm/common/deviceclass.o \
	services4/srvkm/common/handle.o \
	services4/srvkm/common/hash.o \
	services4/srvkm/common/metrics.o \
	services4/srvkm/common/pvrsrv.o \
	services4/srvkm/common/queue.o \
	services4/srvkm/common/ra.o \
	services4/srvkm/common/resman.o \
	services4/srvkm/common/power.o \
	services4/srvkm/common/mem.o \
	services4/srvkm/common/pdump_common.o \
	services4/srvkm/bridged/bridged_support.o \
	services4/srvkm/bridged/bridged_pvr_bridge.o \
	services4/srvkm/common/perproc.o \
	services4/system/drime4_slp/sysconfig.o \
	services4/system/drime4_slp/sysutils.o \
	services4/srvkm/common/lists.o \
	services4/srvkm/common/mem_debug.o \
	services4/srvkm/common/osfunc_common.o \
	services4/srvkm/bridged/sgx/bridged_sgx_bridge.o \
	services4/srvkm/devices/sgx/sgxinit.o \
	services4/srvkm/devices/sgx/sgxpower.o \
	services4/srvkm/devices/sgx/sgxreset.o \
	services4/srvkm/devices/sgx/sgxutils.o \
	services4/srvkm/devices/sgx/sgxkick.o \
	services4/srvkm/devices/sgx/sgxtransfer.o \
	services4/srvkm/devices/sgx/mmu.o \
	services4/srvkm/env/linux/d4dp_displayclass.o \
	services4/srvkm/devices/sgx/pb.o
#	services4/srvkm/env/linux/d4dp_displayclass.o \
