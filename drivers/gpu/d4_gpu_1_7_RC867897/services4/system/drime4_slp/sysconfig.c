/**********************************************************************
 *
 * Copyright (C) Imagination Technologies Ltd. All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope it will be useful but, except 
 * as otherwise stated in writing, without any warranty; without even the 
 * implied warranty of merchantability or fitness for a particular purpose. 
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * 
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * Imagination Technologies Ltd. <gpl-support@imgtec.com>
 * Home Park Estate, Kings Langley, Herts, WD4 8LZ, UK 
 *
******************************************************************************/

#include "config_kernel.h"

#include "sgxdefs.h"
#include "services_headers.h"
#include "kerneldisplay.h"
#include "oemfuncs.h"
#include "sgxinfo.h"
#include "sgxinfokm.h"
#include "pdump_km.h"
/*
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#define MAPPING_SIZE 0x10000
static struct resource		*mem;
static void __iomem		*io;
*/


#define REAL_HARDWARE 1
#define SGX540_BASEADDR 0x70040000
#define SGX540_IRQ 16		//DRIMeIV S-14, M-15, C-16(use)

#define SYS_SGX_CLOCK_SPEED				(260000000)	/* Real chip */

/* Defines for HW Recovery */
/* value is Hz!!! */
#define SYS_SGX_HWRECOVERY_TIMEOUT_FREQ		(130)	/*  100hz -> 10ms */
#define SYS_SGX_PDS_TIMER_FREQ			(1300)	/* 1000hz ->  1ms, microkernel timer frequency */
/* value is milisecond */
#define SYS_SGX_ACTIVE_POWER_LATENCY_MS		(500)


typedef struct _SYS_SPECIFIC_DATA_TAG_
{
	IMG_UINT32 ui32SysSpecificData;

} SYS_SPECIFIC_DATA;
#define SYS_SPECIFIC_DATA_ENABLE_IRQ		0x00000001UL
#define SYS_SPECIFIC_DATA_ENABLE_LISR		0x00000002UL
#define SYS_SPECIFIC_DATA_ENABLE_MISR		0x00000004UL

SYS_SPECIFIC_DATA gsSysSpecificData;

/* top level system data anchor point*/
SYS_DATA* gpsSysData = (SYS_DATA*)IMG_NULL;
SYS_DATA  gsSysData;

/* SGX structures */
static IMG_UINT32	gui32SGXDeviceID;
static SGX_DEVICE_MAP	gsSGXDeviceMap;

/* mimic slaveport and register block with contiguous memory */
IMG_CPU_VIRTADDR gsSGXRegsCPUVAddr;
IMG_CPU_VIRTADDR gsSGXSPCPUVAddr;

char version_string[] = "SGX540 DRIMeIV";

IMG_UINT32   PVRSRV_BridgeDispatchKM( IMG_UINT32  Ioctl,
					IMG_BYTE   *pInBuf,
					IMG_UINT32  InBufLen, 
					IMG_BYTE   *pOutBuf,
					IMG_UINT32  OutBufLen,
					IMG_UINT32 *pdwBytesTransferred);

/*!
******************************************************************************

 @Function	SysLocateDevices

 @Description specifies devices in the systems memory map

 @Input    psSysData - sys data

 @Return   PVRSRV_ERROR  : 

******************************************************************************/
static PVRSRV_ERROR SysLocateDevices(SYS_DATA *psSysData)
{
	PVR_UNREFERENCED_PARAMETER(psSysData);

/*
	mem = request_mem_region(SGX540_BASEADDR, MAPPING_SIZE, "s3c-sgx540");
	if (mem == NULL) {
		printk("failed to get memory region\n");
		return PVRSRV_ERROR_GENERIC;
	}

	io = ioremap(SGX540_BASEADDR, MAPPING_SIZE);

	if (io == NULL) {
		release_resource(mem);
		printk("ioremap() of registers failed\n");
		return PVRSRV_ERROR_GENERIC;
	}
*/
#if 0
	/* SGX Device: */
	gsSGXDeviceMap.ui32Flags = 0x0;
	sCpuPAddr.uiAddr = SGX540_BASEADDR;
	gsSGXDeviceMap.sRegsCpuPBase = sCpuPAddr;
	gsSGXDeviceMap.sRegsSysPBase = SysCpuPAddrToSysPAddr(gsSGXDeviceMap.sRegsCpuPBase);;
	gsSGXDeviceMap.ui32RegsSize = SGX_REG_SIZE;
/*	gsSGXDeviceMap.pvRegsCpuVBase = (IMG_CPU_VIRTADDR)io; */

#else

	gsSGXDeviceMap.sRegsSysPBase.uiAddr = SGX540_BASEADDR;
	gsSGXDeviceMap.sRegsCpuPBase = SysSysPAddrToCpuPAddr(gsSGXDeviceMap.sRegsSysPBase);
	gsSGXDeviceMap.ui32RegsSize = SGX_REG_SIZE;
	gsSGXDeviceMap.ui32IRQ = SGX540_IRQ;
#endif


#if defined(SGX_FEATURE_HOST_PORT)
	/* HostPort: */
	gsSGXDeviceMap.sHPSysPBase.uiAddr = 0;
	gsSGXDeviceMap.sHPCpuPBase.uiAddr = 0;
	gsSGXDeviceMap.ui32HPSize = 0;
#endif

	/* 
		Local Device Memory Region: (not present)
		Note: the device doesn't need to know about its memory 
		but keep info here for now
	*/
	gsSGXDeviceMap.sLocalMemSysPBase.uiAddr = 0;
	gsSGXDeviceMap.sLocalMemDevPBase.uiAddr = 0;
	gsSGXDeviceMap.sLocalMemCpuPBase.uiAddr = 0;
	gsSGXDeviceMap.ui32LocalMemSize = 0;

	/* 
		device interrupt IRQ
		Note: no interrupts available on No HW system
	*/
	gsSGXDeviceMap.ui32IRQ = SGX540_IRQ;

	/* add other devices here: */

	return PVRSRV_OK;
}



/*!
******************************************************************************

 @Function	SysInitialise
 
 @Description Initialises kernel services at 'driver load' time
 
 @Return   PVRSRV_ERROR  : 

******************************************************************************/
PVRSRV_ERROR SysInitialise(IMG_VOID)
{
	IMG_UINT32			i;
	PVRSRV_ERROR 		eError;
	PVRSRV_DEVICE_NODE	*psDeviceNode;
	SGX_TIMING_INFORMATION*	psTimingInfo;

	gpsSysData = &gsSysData;
	OSMemSet(gpsSysData, 0, sizeof(SYS_DATA));

	eError = OSInitEnvData(&gpsSysData->pvEnvSpecificData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to setup env structure"));
		SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}
#if defined(SGX540)
/* printk("SGX540 defined\n"); */
#endif	
/* printk("SGX_CORE_REV=%d\n",SGX_CORE_REV); */
#if defined(SGX_FEATURE_SYSTEM_CACHE)
/* printk("SGX_FEATURE_SYSTEM_CACHE defined!!!!!!!!!!!!!!\n"); */
#if defined(FIX_HW_BRN_25659)
/* printk("FIX_HW_BRN_25659 defined!!!!!!!!!!!!!!\n"); */

#endif
#endif

#if defined(SGX_BYPASS_SYSTEM_CACHE)
/*	printk("SGX_BYPASS_SYSTEM_CACHE defined!!!!!!!!!!!!!!!\n"); */
#endif

	gpsSysData->pvSysSpecificData = (IMG_PVOID)&gsSysSpecificData;
	OSMemSet(&gsSGXDeviceMap, 0, sizeof(SGX_DEVICE_MAP));
	
	/* Set up timing information*/
	psTimingInfo = &gsSGXDeviceMap.sTimingInfo;
	psTimingInfo->ui32CoreClockSpeed = SYS_SGX_CLOCK_SPEED;
	psTimingInfo->ui32HWRecoveryFreq = SYS_SGX_HWRECOVERY_TIMEOUT_FREQ; 
	psTimingInfo->ui32ActivePowManLatencyms = SYS_SGX_ACTIVE_POWER_LATENCY_MS; 
	psTimingInfo->ui32uKernelFreq = SYS_SGX_PDS_TIMER_FREQ; 

	gpsSysData->ui32NumDevices = SYS_DEVICE_COUNT;

	/* init device ID's */
	for(i=0; i<SYS_DEVICE_COUNT; i++)
	{
		gpsSysData->sDeviceID[i].uiID = i;
		gpsSysData->sDeviceID[i].bInUse = IMG_FALSE;
	}

	gpsSysData->psDeviceNodeList = IMG_NULL;
	gpsSysData->psQueueList = IMG_NULL;

	eError = SysInitialiseCommon(gpsSysData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed in SysInitialiseCommon"));
		SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}

	/*
		Locate the devices within the system, specifying 
		the physical addresses of each devices components 
		(regs, mem, ports etc.)
	*/
	eError = SysLocateDevices(gpsSysData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to locate devices"));
		SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}

	/* 
		Register devices with the system
		This also sets up their memory maps/heaps
	*/
	eError = PVRSRVRegisterDevice(gpsSysData, SGXRegisterDevice, 1, &gui32SGXDeviceID);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to register device!"));
		SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}

#if 0
	/*
		Once all devices are registered, specify the backing store
		and, if required, customise the memory heap config
	*/	
	psDeviceNode = gpsSysData->psDeviceNodeList;
	while(psDeviceNode)
	{
		/* perform any OEM SOC address space customisations here */
		switch(psDeviceNode->sDevId.eDeviceType)
		{
			case PVRSRV_DEVICE_TYPE_SGX:
			{
				DEVICE_MEMORY_INFO *psDevMemoryInfo;
				DEVICE_MEMORY_HEAP_INFO *psDeviceMemoryHeap;

				/*
					specify the backing store to use for the devices MMU PT/PDs
					- the PT/PDs are always UMA in this system
				*/
				psDeviceNode->psLocalDevMemArena = IMG_NULL;

				/* useful pointers */
				psDevMemoryInfo = &psDeviceNode->sDevMemoryInfo;
				psDeviceMemoryHeap = psDevMemoryInfo->psDeviceMemoryHeap;

				/* specify the backing store for all SGX heaps */
				for(i=0; i<psDevMemoryInfo->ui32HeapCount; i++)
				{
					psDeviceMemoryHeap[i].ui32Attribs |= PVRSRV_BACKINGSTORE_SYSMEM_NONCONTIG;
				}

				break;
			}
			default:
				PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to find SGX device node!"));
				return PVRSRV_ERROR_INIT_FAILURE;
		}

		/* advance to next device */
		psDeviceNode = psDeviceNode->psNext;
	}

#else
	/*
		Once all devices are registered, specify the backing store
		and, if required, customise the memory heap config
	*/	
	psDeviceNode = gpsSysData->psDeviceNodeList;
	while(psDeviceNode)
	{
		/* perform any OEM SOC address space customisations here */
		switch(psDeviceNode->sDevId.eDeviceType)
		{
			case PVRSRV_DEVICE_TYPE_SGX:
			{
				DEVICE_MEMORY_INFO *psDevMemoryInfo;
				DEVICE_MEMORY_HEAP_INFO *psDeviceMemoryHeap;
				IMG_UINT32 ui32MemConfig;

				if(gpsSysData->apsLocalDevMemArena[0] != IMG_NULL)
				{
					/* specify the backing store to use for the device's MMU PT/PDs */
					psDeviceNode->psLocalDevMemArena = gpsSysData->apsLocalDevMemArena[0];
					ui32MemConfig = PVRSRV_BACKINGSTORE_LOCALMEM_CONTIG;
				}
				else
				{
					/*
						specify the backing store to use for the devices MMU PT/PDs
						- the PT/PDs are always UMA in this system
					*/
					psDeviceNode->psLocalDevMemArena = IMG_NULL;
					ui32MemConfig = PVRSRV_BACKINGSTORE_SYSMEM_NONCONTIG;
				}

				/* useful pointers */
				psDevMemoryInfo = &psDeviceNode->sDevMemoryInfo;
				psDeviceMemoryHeap = psDevMemoryInfo->psDeviceMemoryHeap;

				/* specify the backing store for all SGX heaps */
				for(i=0; i<psDevMemoryInfo->ui32HeapCount; i++)
				{
#if defined(SGX_FEATURE_VARIABLE_MMU_PAGE_SIZE)
					IMG_CHAR *pStr;
								
					switch(psDeviceMemoryHeap[i].ui32HeapID)
					{
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_GENERAL_HEAP_ID):
						{
							pStr = "GeneralHeapPageSize";
							break;
						}
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_GENERAL_MAPPING_HEAP_ID):
						{
							pStr = "GeneralMappingHeapPageSize";
							break;
						}
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_TADATA_HEAP_ID):
						{
							pStr = "TADataHeapPageSize";
							break;
						}
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_KERNEL_CODE_HEAP_ID):
						{
							pStr = "KernelCodeHeapPageSize";
							break;
						}
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_KERNEL_DATA_HEAP_ID):
						{
							pStr = "KernelDataHeapPageSize";
							break;
						}
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_PIXELSHADER_HEAP_ID):
						{
							pStr = "PixelShaderHeapPageSize";
							break;
						}
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_VERTEXSHADER_HEAP_ID):
						{
							pStr = "VertexShaderHeapPageSize";
							break;
						}
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_PDSPIXEL_CODEDATA_HEAP_ID):
						{
							pStr = "PDSPixelHeapPageSize";
							break;
						}
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_PDSVERTEX_CODEDATA_HEAP_ID):
						{
							pStr = "PDSVertexHeapPageSize";
							break;
						}
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_SYNCINFO_HEAP_ID):
						{
							pStr = "SyncInfoHeapPageSize";
							break;
						}
						case HEAP_ID(PVRSRV_DEVICE_TYPE_SGX, SGX_3DPARAMETERS_HEAP_ID):
						{
							pStr = "3DParametersHeapPageSize";
							break;
						}
						default:
						{
							/* not interested in other heaps */
							pStr = IMG_NULL;
							break;	
						}
					}
					if (pStr 
					&&	OSReadRegistryDWORDFromString(0,
														PVRSRV_REGISTRY_ROOT,
														pStr,
														&psDeviceMemoryHeap[i].ui32DataPageSize) == IMG_TRUE)
					{
						PVR_DPF((PVR_DBG_VERBOSE,"SysInitialise: set Heap %s page size to %d", pStr, psDeviceMemoryHeap[i].ui32DataPageSize));
					}
#endif
					/*
						map the device memory allocator(s) onto
						the device memory heaps as required
					*/
					psDeviceMemoryHeap[i].psLocalDevMemArena = gpsSysData->apsLocalDevMemArena[0];

					/* set the memory config (uma | non-uma) */
					psDeviceMemoryHeap[i].ui32Attribs |= ui32MemConfig;
				}

				break;
			}
			default:
				PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to find SGX device node!"));
				return PVRSRV_ERROR_INIT_FAILURE;
		}

		/* advance to next device */
		psDeviceNode = psDeviceNode->psNext;
	}


#endif

	PDUMPINIT();

	/*
		Initialise all devices 'managed' by services:
	*/
	eError = PVRSRVInitialiseDevice (gui32SGXDeviceID);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to initialise device!"));
		SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}

	return PVRSRV_OK;
}


/*!
******************************************************************************

 @Function	SysFinalise
 
 @Description Final part of initialisation
 

 @Return   PVRSRV_ERROR  : 

******************************************************************************/
PVRSRV_ERROR SysFinalise(IMG_VOID)
{
#if defined(SYS_USING_INTERRUPTS)
	PVRSRV_ERROR eError;    

	eError = OSInstallMISR(gpsSysData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"OSInstallMISR: Failed to install MISR"));
		SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}
	gsSysSpecificData.ui32SysSpecificData |= SYS_SPECIFIC_DATA_ENABLE_MISR;

	/* install a system ISR */
	eError = OSInstallSystemLISR(gpsSysData, gsSGXDeviceMap.ui32IRQ);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"OSInstallSystemLISR: Failed to install ISR"));
		OSUninstallMISR(gpsSysData);
		SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}
	gsSysSpecificData.ui32SysSpecificData |= SYS_SPECIFIC_DATA_ENABLE_LISR;
	
/*	SysEnableInterrupts(gpsSysData); */
	gsSysSpecificData.ui32SysSpecificData |= SYS_SPECIFIC_DATA_ENABLE_IRQ;
#endif /* defined(SYS_USING_INTERRUPTS) */

	/* Create a human readable version string for this system */
#if 0
	gpsSysData->pszVersionString = SysCreateVersionString(gsSGXDeviceMap.sRegsCpuPBase);
#else
	gpsSysData->pszVersionString=version_string;
#endif
	if (!gpsSysData->pszVersionString)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysFinalise: Failed to create a system version string"));
    }
	else
	{
		PVR_DPF((PVR_DBG_WARNING, "SysFinalise: Version string: %s", gpsSysData->pszVersionString));
	}

	
	return PVRSRV_OK;
}


/*!
******************************************************************************

 @Function	SysDeinitialise

 @Description De-initialises kernel services at 'driver unload' time

 @Return   PVRSRV_ERROR  : 

******************************************************************************/
PVRSRV_ERROR SysDeinitialise (SYS_DATA *psSysData)
{
	SYS_SPECIFIC_DATA * psSysSpecData;
	PVRSRV_ERROR eError;


	PVR_UNREFERENCED_PARAMETER(psSysData);

	if (psSysData == IMG_NULL) {
		PVR_DPF((PVR_DBG_ERROR, "SysDeinitialise: Called with NULL SYS_DATA pointer.  Probably called before."));
		return PVRSRV_OK;
	}

#if defined(SYS_USING_INTERRUPTS)

	psSysSpecData = (SYS_SPECIFIC_DATA *) psSysData->pvSysSpecificData;

	if (psSysSpecData->ui32SysSpecificData & SYS_SPECIFIC_DATA_ENABLE_IRQ) 	
	{
/*		SysDisableInterrupts(psSysData); */
	}
	if (psSysSpecData->ui32SysSpecificData & SYS_SPECIFIC_DATA_ENABLE_LISR)
	{	
		eError = OSUninstallSystemLISR(psSysData);
		if (eError != PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR,"SysDeinitialise: OSUninstallSystemLISR failed"));
			return eError;
		}
	}
	if (psSysSpecData->ui32SysSpecificData & SYS_SPECIFIC_DATA_ENABLE_MISR)
	{
		eError = OSUninstallMISR(psSysData);
		if (eError != PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR,"SysDeinitialise: OSUninstallMISR failed"));
			return eError;
		}
	}
#endif

	/* de-initialise all services managed devices */
	eError = PVRSRVDeinitialiseDevice (gui32SGXDeviceID);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysDeinitialise: failed to de-init the device"));
		return eError;
	}

	eError = OSDeInitEnvData(gpsSysData->pvEnvSpecificData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysDeinitialise: failed to de-init env structure"));
		return eError;
	}

	SysDeinitialiseCommon(gpsSysData);


	#if REAL_HARDWARE
/*	iounmap(io);
	release_resource(mem);
	kfree(mem);
*/
	#else
	/* Free hardware resources. */
	OSBaseFreeContigMemory(SGX_REG_SIZE, gsSGXRegsCPUVAddr, gsSGXDeviceMap.sRegsCpuPBase);
	OSBaseFreeContigMemory(SGX_SP_SIZE, gsSGXSPCPUVAddr, gsSGXDeviceMap.sSPCpuPBase);
	#endif

	gpsSysData = IMG_NULL;

	PDUMPDEINIT();

	return PVRSRV_OK;
}


/*!
******************************************************************************

 @Function		SysGetDeviceMemoryMap

 @Description	returns a device address map for the specified device

 @Input			eDeviceType - device type
 @Input			ppvDeviceMap - void ptr to receive device specific info.

 @Return   		PVRSRV_ERROR

******************************************************************************/
PVRSRV_ERROR SysGetDeviceMemoryMap(PVRSRV_DEVICE_TYPE eDeviceType,
									IMG_VOID **ppvDeviceMap)
{

	switch(eDeviceType)
	{
		case PVRSRV_DEVICE_TYPE_SGX:
		{
			/* just return a pointer to the structure */
			*ppvDeviceMap = (IMG_VOID*)&gsSGXDeviceMap;

			break;
		}
		default:
		{
			PVR_DPF((PVR_DBG_ERROR,"SysGetDeviceMemoryMap: unsupported device type"));
		}
	}
	return PVRSRV_OK;
}


/*----------------------------------------------------------------------------
<function>
	FUNCTION:   SysCpuPAddrToDevPAddr

	PURPOSE:    Compute a device physical address from a cpu physical
	            address. Relevant when 

	PARAMETERS:	In:  cpu_paddr - cpu physical address.
				In:  eDeviceType - device type required if DevPAddr 
									address spaces vary across devices 
									in the same system
	RETURNS:	device physical address.

</function>
------------------------------------------------------------------------------*/
IMG_DEV_PHYADDR SysCpuPAddrToDevPAddr (PVRSRV_DEVICE_TYPE eDeviceType, 
										IMG_CPU_PHYADDR CpuPAddr)
{
	IMG_DEV_PHYADDR DevPAddr;

	PVR_UNREFERENCED_PARAMETER(eDeviceType);

	/* Note: for no HW UMA system we assume DevP == CpuP */
	DevPAddr.uiAddr = CpuPAddr.uiAddr;
	
	return DevPAddr;
}

/*----------------------------------------------------------------------------
<function>
	FUNCTION:   SysSysPAddrToCpuPAddr

	PURPOSE:    Compute a cpu physical address from a system physical
	            address.

	PARAMETERS:	In:  sys_paddr - system physical address.
	RETURNS:	cpu physical address.

</function>
------------------------------------------------------------------------------*/
IMG_CPU_PHYADDR SysSysPAddrToCpuPAddr (IMG_SYS_PHYADDR sys_paddr)
{
	IMG_CPU_PHYADDR cpu_paddr;

	/* This would only be an inequality if the CPU's MMU did not point to sys address 0, 
	   ie. multi CPU system */
	cpu_paddr.uiAddr = sys_paddr.uiAddr;
	return cpu_paddr;
}

/*----------------------------------------------------------------------------
<function>
	FUNCTION:   SysCpuPAddrToSysPAddr

	PURPOSE:    Compute a system physical address from a cpu physical
	            address.

	PARAMETERS:	In:  cpu_paddr - cpu physical address.
	RETURNS:	device physical address.

</function>
------------------------------------------------------------------------------*/
IMG_SYS_PHYADDR SysCpuPAddrToSysPAddr (IMG_CPU_PHYADDR cpu_paddr)
{
	IMG_SYS_PHYADDR sys_paddr;

	/* This would only be an inequality if the CPU's MMU did not point to sys address 0, 
	   ie. multi CPU system */
	sys_paddr.uiAddr = cpu_paddr.uiAddr;
	return sys_paddr;
}


/*----------------------------------------------------------------------------
<function>
	FUNCTION:   SysSysPAddrToDevPAddr

	PURPOSE:    Compute a device physical address from a system physical
	            address.

	PARAMETERS: In:  SysPAddr - system physical address.
				In:  eDeviceType - device type required if DevPAddr 
									address spaces vary across devices 
									in the same system

	RETURNS:    Device physical address.

</function>
-----------------------------------------------------------------------------*/
IMG_DEV_PHYADDR SysSysPAddrToDevPAddr (PVRSRV_DEVICE_TYPE eDeviceType, IMG_SYS_PHYADDR SysPAddr)
{
	IMG_DEV_PHYADDR DevPAddr;

	PVR_UNREFERENCED_PARAMETER(eDeviceType);

	/* Note: for no HW UMA system we assume DevP == CpuP */
	DevPAddr.uiAddr = SysPAddr.uiAddr;

	return DevPAddr;
}


/*----------------------------------------------------------------------------
<function>
	FUNCTION:   SysDevPAddrToSysPAddr

	PURPOSE:    Compute a device physical address from a system physical
	            address.

	PARAMETERS: In:  DevPAddr - device physical address.
				In:  eDeviceType - device type required if DevPAddr 
									address spaces vary across devices 
									in the same system

	RETURNS:    System physical address.

</function>
-----------------------------------------------------------------------------*/
IMG_SYS_PHYADDR SysDevPAddrToSysPAddr (PVRSRV_DEVICE_TYPE eDeviceType, IMG_DEV_PHYADDR DevPAddr)
{
	IMG_SYS_PHYADDR SysPAddr;

	PVR_UNREFERENCED_PARAMETER(eDeviceType);

	/* Note: for no HW UMA system we assume DevP == SysP */
	SysPAddr.uiAddr = DevPAddr.uiAddr;

	return SysPAddr;
}


/*****************************************************************************
 FUNCTION	: SysRegisterExternalDevice

 PURPOSE	: Called when a 3rd party device registers with services

 PARAMETERS: In:  psDeviceNode - the new device node.

 RETURNS	: IMG_VOID
*****************************************************************************/
IMG_VOID SysRegisterExternalDevice(PVRSRV_DEVICE_NODE *psDeviceNode)
{
	PVR_UNREFERENCED_PARAMETER(psDeviceNode);
}


/*****************************************************************************
 FUNCTION	: SysRemoveExternalDevice

 PURPOSE	: Called when a 3rd party device unregisters from services

 PARAMETERS: In:  psDeviceNode - the device node being removed.

 RETURNS	: IMG_VOID
*****************************************************************************/
IMG_VOID SysRemoveExternalDevice(PVRSRV_DEVICE_NODE *psDeviceNode)
{
	PVR_UNREFERENCED_PARAMETER(psDeviceNode);
}


/*----------------------------------------------------------------------------
<function>
	FUNCTION:   SysGetInterruptSource

	PURPOSE:    Returns System specific information about the device(s) that 
				generated the interrupt in the system

	PARAMETERS: In:  psSysData
				In:  psDeviceNode

	RETURNS:    System specific information indicating which device(s) 
				generated the interrupt

</function>
-----------------------------------------------------------------------------*/
IMG_UINT32 SysGetInterruptSource(SYS_DATA* psSysData,
								 PVRSRV_DEVICE_NODE *psDeviceNode)
{	
	PVR_UNREFERENCED_PARAMETER(psSysData);
	PVR_UNREFERENCED_PARAMETER(psDeviceNode);

	/* no interrupts in no_hw system just return all bits */
	return 0x1;
}


/*----------------------------------------------------------------------------
<function>
	FUNCTION:	SysGetInterruptSource

	PURPOSE:	Clears specified system interrupts

	PARAMETERS: psSysData
				ui32ClearBits

	RETURNS:	IMG_VOID

</function>
-----------------------------------------------------------------------------*/
IMG_VOID SysClearInterrupts(SYS_DATA* psSysData, IMG_UINT32 ui32ClearBits)
{
	PVR_UNREFERENCED_PARAMETER(psSysData);
	PVR_UNREFERENCED_PARAMETER(ui32ClearBits);

	/* no interrupts in no_hw system, nothing to do */
}


/*!
******************************************************************************

 @Function	SysSystemPrePowerState

 @Description	Perform system-level processing required before a power transition

 @Input	   eNewPowerState : 

 @Return   PVRSRV_ERROR : 

******************************************************************************/
PVRSRV_ERROR SysSystemPrePowerState(PVRSRV_SYS_POWER_STATE eNewPowerState)
{
	PVR_UNREFERENCED_PARAMETER(eNewPowerState);
	return PVRSRV_OK;
}

/*!
******************************************************************************

 @Function	SysSystemPostPowerState

 @Description	Perform system-level processing required after a power transition

 @Input	   eNewPowerState :

 @Return   PVRSRV_ERROR :

******************************************************************************/
PVRSRV_ERROR SysSystemPostPowerState(PVRSRV_SYS_POWER_STATE eNewPowerState)
{
	PVR_UNREFERENCED_PARAMETER(eNewPowerState);
	return PVRSRV_OK;
}


/*!
******************************************************************************

 @Function	SysDevicePrePowerState

 @Description	Perform system-level processing required before a device power
 				transition

 @Input		ui32DeviceIndex :
 @Input		eNewPowerState :
 @Input		eCurrentPowerState :

 @Return	PVRSRV_ERROR

******************************************************************************/
PVRSRV_ERROR SysDevicePrePowerState(IMG_UINT32			ui32DeviceIndex,
									PVRSRV_DEV_POWER_STATE		eNewPowerState,
									PVRSRV_DEV_POWER_STATE		eCurrentPowerState)
{
	PVR_UNREFERENCED_PARAMETER(ui32DeviceIndex);
	PVR_UNREFERENCED_PARAMETER(eNewPowerState);
	PVR_UNREFERENCED_PARAMETER(eCurrentPowerState);

	return PVRSRV_OK;
}


/*!
******************************************************************************

 @Function	SysDevicePostPowerState

 @Description	Perform system-level processing required after a device power
 				transition

 @Input		ui32DeviceIndex :
 @Input		eNewPowerState :
 @Input		eCurrentPowerState :

 @Return	PVRSRV_ERROR

******************************************************************************/
PVRSRV_ERROR SysDevicePostPowerState(IMG_UINT32			ui32DeviceIndex,
									 PVRSRV_DEV_POWER_STATE	eNewPowerState,
									 PVRSRV_DEV_POWER_STATE	eCurrentPowerState)
{
	PVR_UNREFERENCED_PARAMETER(ui32DeviceIndex);
	PVR_UNREFERENCED_PARAMETER(eNewPowerState);
	PVR_UNREFERENCED_PARAMETER(eCurrentPowerState);

	return PVRSRV_OK;
}


/*****************************************************************************
 FUNCTION	: SysOEMFunction

 PURPOSE	: marshalling function for custom OEM functions

 PARAMETERS	: ui32ID  - function ID
			  pvIn - in data
			  pvOut - out data

 RETURNS	: PVRSRV_ERROR
*****************************************************************************/
PVRSRV_ERROR SysOEMFunction(IMG_UINT32	ui32ID, 
							IMG_VOID	*pvIn,
							IMG_UINT32	ulInSize,
							IMG_VOID	*pvOut,
							IMG_UINT32	ulOutSize)
{
	if (ulInSize || pvIn);

	if ((ui32ID == OEM_GET_EXT_FUNCS) &&
		(ulOutSize == sizeof(PVRSRV_DC_OEM_JTABLE)))
	{
		PVRSRV_DC_OEM_JTABLE *psOEMJTable = (PVRSRV_DC_OEM_JTABLE*)pvOut;
		psOEMJTable->pfnOEMBridgeDispatch = &PVRSRV_BridgeDispatchKM;

		return PVRSRV_OK;
	}

	return PVRSRV_ERROR_INVALID_PARAMS;
}
/******************************************************************************
 End of file (sysconfig.c)
******************************************************************************/
