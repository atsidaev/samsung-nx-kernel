/*-------------------------------------------------------------
 * Filename: d4dp_displayclass.c
 *
 * Contents: Implemention of display kernel module bridging to LCD driver
 *
 * Abbreviations:
 *
 * Person Involved: Byungho Ahn
 *
 * Notes:
 *
 * Copyright (c) 2009 SAMSUNG Electronics.
 --------------------------------------------------------------*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/fb.h>
#include <linux/dma-mapping.h>

#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/memory.h>

#include "config_kernel.h"

#include "img_defs.h"
#include "servicesext.h"
#include "kerneldisplay.h"

#include "video/drime4/d4_dp_ioctl.h"
#include "d4dp_gpu_if.h"

#define D4_PIXEL_FORMAT		PVRSRV_PIXEL_FORMAT_ARGB8888	/* drime4 pixel format aBGR order */
								/* PVRSRV_PIXEL_FORMAT_ABGR8888   */

#include <d4_dp_lcd_dd.h>
#include <video/drime4/d4_dp_type.h>

#include <mach/dp/d4_dp.h>
#include <mach/d4_cma.h>


#include <mach/map.h>
#include <linux/device.h>

#define D4DP_MAX_BACKBUFFERRS 2
#define D4DP_DISPLAY_FORMAT_NUM 1
#define D4DP_DISPLAY_DIM_NUM 1

/* for TEST */
/*#define CLK_GATING_TEST*/

#if 1
extern int gpu_get_dp_info(unsigned int cmd, enum edp_window window, unsigned long arg);

enum gpu_dp_info {
	GET_FSCREENINFO,
	GET_VSCREENINFO,
	SET_GRP_WINDOW,
	GET_PANNEL_INFO,
};
#endif


#define GPU_GRP_WIN	DP_WIN2

#define DC_D4DP_LCD_COMMAND_COUNT 1

static struct stgrpdisplay grp;

typedef struct D4DP_FRAME_BUFFER_TAG
{
	IMG_CPU_VIRTADDR bufferVAddr;
	IMG_SYS_PHYADDR bufferPAddr;
	IMG_UINT32 byteSize;
}D4DP_FRAME_BUFFER;


typedef struct D4DP_SWAPCHAIN_TAG
{
	IMG_UINT32 ui32Flags;
	DISPLAY_SURF_ATTRIBUTES psDstSurfAttrib;
	DISPLAY_SURF_ATTRIBUTES psSrcSurfAttrib;
	IMG_UINT32 ui32BufferCount;
	IMG_UINT32 ui32SwapChainID;
	IMG_UINT32 ui32OEMFlags;
}D4DP_SWAPCHAIN;

typedef struct D4DP_LCD_DEVINFO_TAG
{
	IMG_UINT32 			ui32DisplayID;
	DISPLAY_INFO 			sDisplayInfo;

	/* sys surface info */
	D4DP_FRAME_BUFFER		sSysBuffer;

	/* number of supported format */
	IMG_UINT32 			ui32NumFormats;

	/* list of supported display format */
	DISPLAY_FORMAT 			asDisplayForamtList[D4DP_DISPLAY_FORMAT_NUM];

	IMG_UINT32 			ui32NumDims;
	DISPLAY_DIMS			asDisplayDimList[D4DP_DISPLAY_DIM_NUM];

	/* jump table into pvr services */
	PVRSRV_DC_DISP2SRV_KMJTABLE 	sPVRJTable;

	/* jump table into DC */
	PVRSRV_DC_SRV2DISP_KMJTABLE 	sDCJTable;

	/* backbuffer info */
	D4DP_FRAME_BUFFER		asBackBuffers[D4DP_MAX_BACKBUFFERRS];

	D4DP_SWAPCHAIN			*psSwapChain;
}D4DP_LCD_DEVINFO;

static D4DP_LCD_DEVINFO *g_psLCDInfo = NULL;

void __iomem *LCDControllerBase;
void __iomem *TVControllerBase;
#ifdef CLK_GATING_TEST
void __iomem *PMUBase;
void __iomem *GPUClockControlBase;
void __iomem *GLCKBase;
#endif

extern IMG_BOOL IMG_IMPORT PVRGetDisplayClassJTable(PVRSRV_DC_DISP2SRV_KMJTABLE *psJTable);



static PVRSRV_ERROR OpenDCDevice(IMG_UINT32 ui32DeviceID,
				 IMG_HANDLE *phDevice,
				 PVRSRV_SYNC_DATA* psSystemBufferSyncData)
{
	PVR_UNREFERENCED_PARAMETER(ui32DeviceID);

	*phDevice = g_psLCDInfo;

	return PVRSRV_OK;
}

static PVRSRV_ERROR CloseDCDevice(IMG_HANDLE hDevice)
{
	D4DP_LCD_DEVINFO *psLCDInfo = (D4DP_LCD_DEVINFO*)hDevice;

	PVR_UNREFERENCED_PARAMETER(hDevice);


	if(psLCDInfo == g_psLCDInfo)
	{}
	
	return PVRSRV_OK;
}

static PVRSRV_ERROR EnumDCFormats(IMG_HANDLE		hDevice,
				  IMG_UINT32		*pui32NumFormats,
				  DISPLAY_FORMAT	*psFormat)
{
	D4DP_LCD_DEVINFO *psLCDInfo = (D4DP_LCD_DEVINFO*)hDevice;
	int i;
	
	if(!hDevice || !pui32NumFormats)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	*pui32NumFormats = D4DP_DISPLAY_FORMAT_NUM;

	if(psFormat)
	{
		for (i = 0 ; i < D4DP_DISPLAY_FORMAT_NUM ; i++)
			psFormat[i] = psLCDInfo->asDisplayForamtList[i];
	}
	
	return PVRSRV_OK;
}

static PVRSRV_ERROR EnumDCDims(IMG_HANDLE	hDevice,
			   DISPLAY_FORMAT	*psFormat,
			   IMG_UINT32		*pui32NumDims,
			   DISPLAY_DIMS		*psDim)
{
	int i;
	
	D4DP_LCD_DEVINFO *psLCDInfo = (D4DP_LCD_DEVINFO*)hDevice;
	
	if(!hDevice || !psFormat || !pui32NumDims)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}
	
	*pui32NumDims = D4DP_DISPLAY_DIM_NUM;
	
	if(psDim)
	{
		for (i = 0 ; i < D4DP_DISPLAY_DIM_NUM ; i++)
			psDim[i] = psLCDInfo->asDisplayDimList[i];
	}
	
	return PVRSRV_OK;
}

static PVRSRV_ERROR GetDCSystemBuffer(IMG_HANDLE hDevice, IMG_HANDLE *phBuffer)
{
	D4DP_LCD_DEVINFO *psLCDInfo = (D4DP_LCD_DEVINFO*)hDevice;

	if(!hDevice || !phBuffer)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	
	*phBuffer=(IMG_HANDLE)(&(psLCDInfo->sSysBuffer));
	return PVRSRV_OK;
}

static PVRSRV_ERROR GetDCInfo(IMG_HANDLE hDevice, DISPLAY_INFO *psDCInfo)
{
	D4DP_LCD_DEVINFO *psLCDInfo = (D4DP_LCD_DEVINFO*)hDevice;

	if(!hDevice || !psDCInfo)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}
	
	*psDCInfo = psLCDInfo->sDisplayInfo;
	
	return PVRSRV_OK;
}

static PVRSRV_ERROR GetDCBufferAddr(IMG_HANDLE	hDevice,
				IMG_HANDLE	hBuffer,
				IMG_SYS_PHYADDR	**ppsSysAddr,
				IMG_UINT32	*pui32ByteSize, 
				IMG_VOID	**ppvCpuVAddr,
				IMG_HANDLE	*phOSMapInfo,
				IMG_BOOL	*pbIsContiguous)
{
	D4DP_FRAME_BUFFER *buf = (D4DP_FRAME_BUFFER *)hBuffer;
	D4DP_LCD_DEVINFO *psLCDInfo = (D4DP_LCD_DEVINFO*)hDevice;
	PVR_UNREFERENCED_PARAMETER(psLCDInfo);
	
	printk("GetDCBufferAddr+++++ hBuffer=%x\n",(int)hBuffer);
	
	if(!hDevice || !hBuffer || !ppsSysAddr || !pui32ByteSize)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}
	
	*phOSMapInfo = IMG_NULL;
	*pbIsContiguous = IMG_TRUE;

	*ppvCpuVAddr = (IMG_VOID *)buf->bufferVAddr;
	*ppsSysAddr = &(buf->bufferPAddr);
	*pui32ByteSize = buf->byteSize;
	
	return PVRSRV_OK;
}

static PVRSRV_ERROR CreateDCSwapChain(IMG_HANDLE hDevice,
				  IMG_UINT32 ui32Flags,
				  DISPLAY_SURF_ATTRIBUTES *psDstSurfAttrib,
				  DISPLAY_SURF_ATTRIBUTES *psSrcSurfAttrib,
				  IMG_UINT32 ui32BufferCount,
				  PVRSRV_SYNC_DATA **ppsSyncData,
				  IMG_UINT32 ui32OEMFlags,
				  IMG_HANDLE *phSwapChain,
				  IMG_UINT32 *pui32SwapChainID)
{
	D4DP_SWAPCHAIN *sc = (D4DP_SWAPCHAIN *)kmalloc(sizeof(D4DP_SWAPCHAIN),GFP_KERNEL);
	D4DP_LCD_DEVINFO *psLCDInfo = (D4DP_LCD_DEVINFO*)hDevice;
	PVR_UNREFERENCED_PARAMETER(psLCDInfo);
	
	printk("CreateDCSwapChain:ui32BufferCount=%d\n",(int)ui32BufferCount);

	PVR_UNREFERENCED_PARAMETER(ui32OEMFlags);
	PVR_UNREFERENCED_PARAMETER(pui32SwapChainID);

	/* check parameters */
	if(!hDevice
	|| !psDstSurfAttrib
	|| !psSrcSurfAttrib
	|| !ppsSyncData
	|| !phSwapChain)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}
	
	/* check the buffer count */
	if(ui32BufferCount > 3)
	{
		return PVRSRV_ERROR_TOOMANYBUFFERS;
	}


	sc->psDstSurfAttrib = *psDstSurfAttrib;
	sc->psSrcSurfAttrib = *psSrcSurfAttrib;
	sc->ui32BufferCount = ui32BufferCount;
	sc->ui32Flags = ui32Flags;
	sc->ui32OEMFlags = ui32OEMFlags;
	sc->ui32SwapChainID = (IMG_UINT32)sc;
	printk("CreateDCSwapChain:swapchain.buffercount=%d,sc=%p\n",(int)sc->ui32BufferCount,sc);
	
	*phSwapChain = (IMG_HANDLE)sc;
	*pui32SwapChainID = sc->ui32SwapChainID;

	g_psLCDInfo->psSwapChain = sc;

	return PVRSRV_OK;
}


static PVRSRV_ERROR DestroyDCSwapChain(IMG_HANDLE hDevice,
				   IMG_HANDLE hSwapChain)
{
	D4DP_SWAPCHAIN *sc = (D4DP_SWAPCHAIN *)hSwapChain;
	D4DP_LCD_DEVINFO *psLCDInfo = (D4DP_LCD_DEVINFO*)hDevice;

	/* check parameters */
	if(!hDevice 
	|| !hSwapChain)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	printk("DestroyDCSwapChain:sc=%p\n",(void*)hSwapChain);
	
	if (psLCDInfo->psSwapChain == sc)
		psLCDInfo->psSwapChain = NULL;

	kfree(sc);

	return PVRSRV_OK;
}

static PVRSRV_ERROR SetDCDstRect(IMG_HANDLE	hDevice,
				 IMG_HANDLE	hSwapChain,
				 IMG_RECT	*psRect)
{

	
	PVR_UNREFERENCED_PARAMETER(hDevice);
	PVR_UNREFERENCED_PARAMETER(hSwapChain);
	PVR_UNREFERENCED_PARAMETER(psRect);

	return PVRSRV_ERROR_NOT_SUPPORTED;
}


static PVRSRV_ERROR SetDCSrcRect(IMG_HANDLE	hDevice,
				 IMG_HANDLE	hSwapChain,
				 IMG_RECT	*psRect)
{


	PVR_UNREFERENCED_PARAMETER(hDevice);
	PVR_UNREFERENCED_PARAMETER(hSwapChain);
	PVR_UNREFERENCED_PARAMETER(psRect);

	return PVRSRV_ERROR_NOT_SUPPORTED;
}


static PVRSRV_ERROR SetDCDstColourKey(IMG_HANDLE	hDevice,
				  IMG_HANDLE	hSwapChain,
				  IMG_UINT32	ui32CKColour)
{
	PVR_UNREFERENCED_PARAMETER(hDevice);
	PVR_UNREFERENCED_PARAMETER(hSwapChain);
	PVR_UNREFERENCED_PARAMETER(ui32CKColour);

	return PVRSRV_ERROR_NOT_SUPPORTED;
}

static PVRSRV_ERROR SetDCSrcColourKey(IMG_HANDLE	hDevice,
				  IMG_HANDLE	hSwapChain,
				  IMG_UINT32	ui32CKColour)
{
	PVR_UNREFERENCED_PARAMETER(hDevice);
	PVR_UNREFERENCED_PARAMETER(hSwapChain);
	PVR_UNREFERENCED_PARAMETER(ui32CKColour);



	return PVRSRV_ERROR_NOT_SUPPORTED;
}


static PVRSRV_ERROR GetDCBuffers(IMG_HANDLE hDevice,
				 IMG_HANDLE hSwapChain,
				 IMG_UINT32 *pui32BufferCount,
				 IMG_HANDLE *phBuffer)
{
	D4DP_LCD_DEVINFO *psLCDInfo = (D4DP_LCD_DEVINFO*)hDevice;
	
	/* check parameters */
	if(!hDevice
	|| !hSwapChain
	|| !pui32BufferCount
	|| !phBuffer)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;	
	}

	
	printk("GetDCBuffers:hSwapChain=%p ui32BufferCount=%d\n",(void*)hSwapChain,(int)*pui32BufferCount);

	*pui32BufferCount=3;
	phBuffer[0] = (IMG_HANDLE)(&(psLCDInfo->sSysBuffer));
	phBuffer[1] = (IMG_HANDLE)(&(psLCDInfo->asBackBuffers[0]));
	phBuffer[2] = (IMG_HANDLE)(&(psLCDInfo->asBackBuffers[1]));

	return PVRSRV_OK;
}

static PVRSRV_ERROR SwapToDCBuffer(IMG_HANDLE	hDevice,
				   IMG_HANDLE	hBuffer,
				   IMG_UINT32	ui32SwapInterval,
				   IMG_HANDLE	hPrivateTag,
				   IMG_UINT32	ui32ClipRectCount,
				   IMG_RECT	*psClipRect)
{
	int i;

	printk("SwapToDCBuffer+++\n");

	PVR_UNREFERENCED_PARAMETER(ui32SwapInterval);
	PVR_UNREFERENCED_PARAMETER(hPrivateTag);
	PVR_UNREFERENCED_PARAMETER(psClipRect);

	if(!hDevice
	|| !hBuffer
	|| (ui32ClipRectCount != 0))
	{
		return PVRSRV_ERROR_INVALID_PARAMS;	
	}


	printk("SwapToDCBuffer:swapinterval=%d,cliprectcount=%d\n", (int)ui32SwapInterval, (int)ui32ClipRectCount);

	for(i=0;i<ui32ClipRectCount;i++)
	{
		printk("SwapToDCBuffer:[%d](%d,%d,%d,%d)\n",i,(int)psClipRect[i].x0,(int)psClipRect[i].y0,(int)psClipRect[i].x1,(int)psClipRect[i].y1);
	}

	/* nothing to do for no hw */
	return PVRSRV_OK;
}

static PVRSRV_ERROR SwapToDCSystem(IMG_HANDLE hDevice,
				   IMG_HANDLE hSwapChain)
{
	printk("SwapToDCSystem++++++++++++++\n");

	if(!hDevice 
	|| !hSwapChain)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;	
	}
	
	
	/* FIXME: nothing to do here - deprecate API? */
	printk(__FUNCTION__);

	return PVRSRV_OK;
}


unsigned int virt_addr;
static IMG_BOOL ProcessFlip(IMG_HANDLE	hCmdCookie,
			IMG_UINT32	ui32DataSize,
			IMG_VOID	*pvData)
{
	DISPLAYCLASS_FLIP_COMMAND *psFlipCmd;
	D4DP_FRAME_BUFFER *fb;
	unsigned volatile int *fb_addr;

	/* check parameters */
	if(!hCmdCookie || !pvData) {
		return IMG_FALSE;
	}

	/* validate data packet */
	psFlipCmd = (DISPLAYCLASS_FLIP_COMMAND*)pvData;
	if (psFlipCmd == IMG_NULL || sizeof(DISPLAYCLASS_FLIP_COMMAND) != ui32DataSize)	{
		return IMG_FALSE;
	}
	
	fb = (D4DP_FRAME_BUFFER *)(psFlipCmd->hExtBuffer);	
	fb_addr = (unsigned volatile int *)(fb->bufferVAddr);

	virt_addr = (unsigned int)phys_to_virt((unsigned long)fb->bufferVAddr);

	/* DRIME4 LCD address Set */
	__raw_writel(fb->bufferPAddr.uiAddr, LCDControllerBase + 0x20 + 0x10*(grp.win));
//	__raw_writel(fb->bufferPAddr.uiAddr, TVControllerBase + 0x20 + 0x10*(grp.win));

#ifdef CLK_GATING_TEST
	/* dynamic clock control */
	{
		static unsigned int gpu_clk = 1; /*8=27MHz, 0=260MHz*/
		unsigned int gclksel3;
	
/*		__raw_writel(1, PMUBase+0x10);*/
	
		gclksel3 = readl(GPUClockControlBase) & (~(0xF00));
		gclksel3 = gclksel3 | (gpu_clk<<8);
/*		while(1) {
			if (readl(PMUBase+0x14)&0x1) {*/
				__raw_writel(readl(GLCKBase)&(~(1<<21)), GLCKBase);	/*clk off*/
				__raw_writel(gclksel3, GPUClockControlBase);		/*clk change*/
				__raw_writel(readl(GLCKBase)|(1<<21), GLCKBase);	/*clk on*/
/*				break;
			}
		}
		__raw_writel(0, PMUBase+0x10);*/
		gpu_clk = gpu_clk+1;
		if (gpu_clk == 9)	gpu_clk = 0;
	}
#endif

	/* call command complete Callback */
	g_psLCDInfo->sPVRJTable.pfnPVRSRVCmdComplete(hCmdCookie, IMG_FALSE);

	return IMG_TRUE;
}

unsigned int gpu_virt_addr(void)
{
	return virt_addr;
}
EXPORT_SYMBOL(gpu_virt_addr);


#include <media/drime4/bma/d4_bma_type.h>
extern int d4_bma_alloc_buf(struct BMA_Buffer_Info *info);
extern int d4_bma_free_buf(unsigned int addr);

int D4dpLcdBridgeInit(void)
{
	IMG_UINT32 screen_w, screen_h;
	IMG_UINT32 pa_fb, va_fb;
	IMG_UINT32 byteSize;

	int rgb_format, bytes_per_pixel;
	int i;

	struct BMA_Buffer_Info gpu_buf_info;

#if 0
	struct fb_fix_screeninfo fix;
	struct fb_var_screeninfo var;
	struct stfb_lcd_pannel pannel;

	gpu_get_dp_info(GET_FSCREENINFO, GPU_GRP_WIN, (unsigned long)&fix);
	gpu_get_dp_info(GET_VSCREENINFO, GPU_GRP_WIN, (unsigned long)&var);
	gpu_get_dp_info(GET_PANNEL_INFO, GPU_GRP_WIN, (unsigned long)&pannel);

	screen_w = pannel.h_size;
	screen_h = pannel.v_size;
#else
	screen_w = 640;
	screen_h = 480;
#endif

#if 0 
	pa_fb = fix.smem_start;
	printk("PA FB = 0x%X, bits per pixel = %d\n", (unsigned int)fix.smem_start, (unsigned int)var.bits_per_pixel);

	va_fb = (unsigned long)phys_to_virt(pa_fb);
	printk("screen width=%d height=%d va=0x%x pa=0x%x\n", (int)screen_w, (int)screen_h, (unsigned int)va_fb, (unsigned int)pa_fb);
#endif

	if (g_psLCDInfo == NULL)
	{
		PFN_CMD_PROC	pfnCmdProcList[DC_D4DP_LCD_COMMAND_COUNT];
		IMG_UINT32	aui32SyncCountList[DC_D4DP_LCD_COMMAND_COUNT][2];

		g_psLCDInfo = (D4DP_LCD_DEVINFO*)kmalloc(sizeof(D4DP_LCD_DEVINFO),GFP_KERNEL);
		g_psLCDInfo->ui32NumFormats = D4DP_DISPLAY_FORMAT_NUM;

#if 0
		switch (var.bits_per_pixel){
		case 16:
			rgb_format = PVRSRV_PIXEL_FORMAT_RGB565;
			bytes_per_pixel = 2;
			break;
		case 32:
			rgb_format = D4_PIXEL_FORMAT;
			bytes_per_pixel = 4;
			break;
		default:
			rgb_format = D4_PIXEL_FORMAT;
			bytes_per_pixel = 4;
			break;
		}
#else
		rgb_format = D4_PIXEL_FORMAT;
		bytes_per_pixel = 4;
#endif

		g_psLCDInfo->asDisplayForamtList[0].pixelformat = rgb_format;
		g_psLCDInfo->ui32NumDims = D4DP_DISPLAY_DIM_NUM;
		g_psLCDInfo->asDisplayDimList[0].ui32ByteStride = (bytes_per_pixel) * screen_w;
		g_psLCDInfo->asDisplayDimList[0].ui32Height = screen_h;
		g_psLCDInfo->asDisplayDimList[0].ui32Width = screen_w;
		
		gpu_buf_info.size = screen_w * screen_h * bytes_per_pixel;
		gpu_buf_info.addr = 0;
		if (d4_bma_alloc_buf(&gpu_buf_info) < 0) {
			printk("GPU buffer alloc ERROR!\n");
			return -1;
		}
		va_fb = pa_fb = gpu_buf_info.addr;
		g_psLCDInfo->sSysBuffer.bufferPAddr.uiAddr = pa_fb;
		g_psLCDInfo->sSysBuffer.bufferVAddr = (IMG_CPU_VIRTADDR)va_fb;
		byteSize = screen_w * screen_h * bytes_per_pixel;
		g_psLCDInfo->sSysBuffer.byteSize = (IMG_UINT32)byteSize;

		/* gpu back buffer set */
		for (i=0; i<D4DP_MAX_BACKBUFFERRS; i++) {
			g_psLCDInfo->asBackBuffers[i].byteSize = g_psLCDInfo->sSysBuffer.byteSize;

			d4_bma_alloc_buf(&gpu_buf_info);
			g_psLCDInfo->asBackBuffers[i].bufferPAddr.uiAddr = gpu_buf_info.addr;

			g_psLCDInfo->asBackBuffers[i].bufferVAddr	 = (IMG_CPU_VIRTADDR)g_psLCDInfo->asBackBuffers[i].bufferPAddr.uiAddr;

			printk("frameBuffer[%d].VAddr=%p PAddr=%p size=%d bytes\n",
				i,
				(void*)g_psLCDInfo->asBackBuffers[i].bufferVAddr,
				(void*)g_psLCDInfo->asBackBuffers[i].bufferPAddr.uiAddr,
				(int)g_psLCDInfo->asBackBuffers[i].byteSize);
		}

		g_psLCDInfo->psSwapChain = NULL;

		PVRGetDisplayClassJTable(&(g_psLCDInfo->sPVRJTable));

		g_psLCDInfo->sDCJTable.ui32TableSize = sizeof(PVRSRV_DC_SRV2DISP_KMJTABLE);
		g_psLCDInfo->sDCJTable.pfnOpenDCDevice = OpenDCDevice;
		g_psLCDInfo->sDCJTable.pfnCloseDCDevice = CloseDCDevice;
		g_psLCDInfo->sDCJTable.pfnEnumDCFormats = EnumDCFormats;
		g_psLCDInfo->sDCJTable.pfnEnumDCDims = EnumDCDims;
		g_psLCDInfo->sDCJTable.pfnGetDCSystemBuffer = GetDCSystemBuffer;
		g_psLCDInfo->sDCJTable.pfnGetDCInfo = GetDCInfo;
		g_psLCDInfo->sDCJTable.pfnGetBufferAddr = (PFN_GET_BUFFER_ADDR)GetDCBufferAddr;
		g_psLCDInfo->sDCJTable.pfnCreateDCSwapChain = CreateDCSwapChain;
		g_psLCDInfo->sDCJTable.pfnDestroyDCSwapChain = DestroyDCSwapChain;
		g_psLCDInfo->sDCJTable.pfnSetDCDstRect = SetDCDstRect;
		g_psLCDInfo->sDCJTable.pfnSetDCSrcRect = SetDCSrcRect;
		g_psLCDInfo->sDCJTable.pfnSetDCDstColourKey = SetDCDstColourKey;
		g_psLCDInfo->sDCJTable.pfnSetDCSrcColourKey = SetDCSrcColourKey;
		g_psLCDInfo->sDCJTable.pfnGetDCBuffers = GetDCBuffers;
		g_psLCDInfo->sDCJTable.pfnSwapToDCBuffer = SwapToDCBuffer;
		g_psLCDInfo->sDCJTable.pfnSwapToDCSystem = SwapToDCSystem;
		g_psLCDInfo->sDCJTable.pfnSetDCState =  (PFN_SET_DC_STATE)IMG_NULL;

		g_psLCDInfo->sDisplayInfo.ui32MinSwapInterval=0;
		g_psLCDInfo->sDisplayInfo.ui32MaxSwapInterval=0;
		g_psLCDInfo->sDisplayInfo.ui32MaxSwapChains=1;

		g_psLCDInfo->sDisplayInfo.ui32MaxSwapChainBuffers = D4DP_MAX_BACKBUFFERRS + 1;
		strncpy(g_psLCDInfo->sDisplayInfo.szDisplayName, "d4dp_lcd", MAX_DISPLAY_NAME_SIZE);


		if(g_psLCDInfo->sPVRJTable.pfnPVRSRVRegisterDCDevice	(&(g_psLCDInfo->sDCJTable),
			(IMG_UINT32 *)(&(g_psLCDInfo->ui32DisplayID))) != PVRSRV_OK)
		{
			return 1;
		}

		printk("deviceID:%d\n",(int)g_psLCDInfo->ui32DisplayID);

		/* register flip command */
		pfnCmdProcList[DC_FLIP_COMMAND] = ProcessFlip;
		aui32SyncCountList[DC_FLIP_COMMAND][0] = 0;/* no writes */
		aui32SyncCountList[DC_FLIP_COMMAND][1] = 2;/* 2 reads: To / From */

		if (g_psLCDInfo->sPVRJTable.pfnPVRSRVRegisterCmdProcList(g_psLCDInfo->ui32DisplayID,
			&pfnCmdProcList[0], aui32SyncCountList, DC_D4DP_LCD_COMMAND_COUNT)
			!= PVRSRV_OK)
		{
			printk("failing register commmand proc list   deviceID:%d\n",(int)g_psLCDInfo->ui32DisplayID);
			return PVRSRV_ERROR_CANT_REGISTER_CALLBACK;
		}

		LCDControllerBase = ioremap(0x500B0900,1024);
		TVControllerBase  = ioremap(0x500B0400,1024);
	}

#if 0	/*pixel format register set to aRGB*/
	{
		unsigned int reg_val;
		reg_val = readl(LCDControllerBase);
		reg_val = reg_val | (1<<12);
		__raw_writel(reg_val, LCDControllerBase);
	}
#endif

#if 0
	/* default DP set */
	grp.win = GPU_GRP_WIN;
	grp.win_onoff = DP_ON;
	grp.stride = screen_w * 4;      /* width*4bytes */
	grp.display.H_Start = 0;
	grp.display.H_Size = screen_w;
	grp.display.V_Start = 0;
	grp.display.V_Size = screen_h;
	grp.img_width = screen_w * 4;
	grp.img_height = screen_h;
	grp.address_mode = DP_PHYSICAL_SET;
	grp.address = fix.smem_start;
	gpu_get_dp_info(SET_GRP_WINDOW, grp.win, (unsigned long)&grp);
#endif

#ifdef CLK_GATING_TEST
	/* dynamic clock control */
	GPUClockControlBase	= ioremap(0x30120008, 0x4);
	GLCKBase		= ioremap(0x30120020, 0x4);
	PMUBase			= ioremap(0x30150000, 0x20);
#endif

	return 0;
	
}



void D4dpLcdBridgeExit(void)
{
	int i;

	printk("d4dp_displayclass deinit++\n");
	
	g_psLCDInfo->sPVRJTable.pfnPVRSRVRemoveDCDevice(g_psLCDInfo->ui32DisplayID);

	for (i=0; i<D4DP_MAX_BACKBUFFERRS; i++) {
		d4_bma_free_buf(g_psLCDInfo->asBackBuffers[i].bufferPAddr.uiAddr);
	}

	if (g_psLCDInfo)
		kfree(g_psLCDInfo);

	g_psLCDInfo = NULL;
	
	if(LCDControllerBase)
		iounmap(LCDControllerBase);
	
	if(TVControllerBase)
		iounmap(TVControllerBase);

#ifdef CLK_GATING_TEST
	iounmap(GPUClockControlBase);
	iounmap(GLCKBase);
	iounmap(PMUBase);
#endif
}

