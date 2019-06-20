/* linux/drivers/media/video/drime4/ipcm/d4_ipcm_v4l2.c
 *
 * V4L2 based Samsung Drime IV IPCM Interface driver.
 *
 * Jangwon Lee <jang_won.lee@samsung.com>
 *
 * Note: This driver supports common i2c client driver style
 * which uses i2c_board_info for backward compatibility and
 * new v4l2_subdev as well.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

/*
#define DEBUG
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/completion.h>
#include <linux/i2c.h>
#include <linux/videodev2_drime4.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>

#include "mach/ipcm/d4-ipcm.h"
#include "media/drime4/ipcm/d4_ipcm_type.h"
#include "media/drime4/ipcm/d4_ipcm_v4l2.h"
#include "d4_ipcm_framework.h"

static DECLARE_WAIT_QUEUE_HEAD(ipcm_wq_read);
#define MAX_IPCM_READ_QUEUE_CNT 512

static char ipcm_read_queue[MAX_IPCM_READ_QUEUE_CNT];
static unsigned long ipcm_read_q_cnt;
static unsigned long ipcm_read_q_head;
static unsigned long ipcm_read_q_tail;

/*
 * present liveview operation
 * liveview off: IPCM_LIVEVIEW_OFF
 * liveview on: IPCM_LIVEVIEW_ON
 */
unsigned int ipcm_liveview_operation;

static struct ipcm_fmt formats[] = {
	{
		.name		= "BAYER BGBG.. GRGR..",
		.fourcc		= V4L2_PIX_FMT_SBGGR8,
		.depth		= 8,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER GBGB.. RGRG..",
		.fourcc		= V4L2_PIX_FMT_SGBRG8,
		.depth		= 8,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER GRGR.. BGBG..",
		.fourcc		= V4L2_PIX_FMT_SGRBG8,
		.depth		= 8,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER RGRG.. GBGB..",
		.fourcc		= V4L2_PIX_FMT_SRGGB8,
		.depth		= 8,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER BGBG.. GRGR..",
		.fourcc		= V4L2_PIX_FMT_SBGGR10,
		.depth		= 10,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER GBGB.. RGRG..",
		.fourcc		= V4L2_PIX_FMT_SGBRG10,
		.depth		= 10,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER GRGR.. BGBG..",
		.fourcc		= V4L2_PIX_FMT_SGRBG10,
		.depth		= 10,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER RGRG.. GBGB..",
		.fourcc		= V4L2_PIX_FMT_SRGGB10,
		.depth		= 10,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER BGBG.. GRGR..",
		.fourcc		= V4L2_PIX_FMT_SBGGR12,
		.depth		= 12,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER GBGB.. RGRG..",
		.fourcc		= V4L2_PIX_FMT_SGBRG12,
		.depth		= 12,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER GRGR.. BGBG..",
		.fourcc		= V4L2_PIX_FMT_SGRBG12,
		.depth		= 12,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "BAYER RGRG.. GBGB..",
		.fourcc		= V4L2_PIX_FMT_SRGGB12,
		.depth		= 12,
		.buff_cnt	= 1,
		.plane_cnt	= 1,
	}, {
		.name		= "YUV 4:2:0, Y/Cb/Cr",
		.fourcc		= V4L2_PIX_FMT_YUV420,
		.depth		= 16,
		.buff_cnt	= 1,
		.plane_cnt	= 3
	}, {
		.name		= "YUV 4:2:2, Y/Cb/Cr",
		.fourcc		= V4L2_PIX_FMT_YUV422P,
		.depth		= 16,
		.buff_cnt	= 1,
		.plane_cnt	= 3
	}, {
		.name		= "YUV 4:2:2, Y/Cb/Cr",
		.fourcc		= V4L2_PIX_FMT_YUYV,
		.depth		= 16,
		.buff_cnt	= 1,
		.plane_cnt	= 3
	}, {
		.name		= "Y/CbCr 4:2:0, Y/CbCr",
		.fourcc		= V4L2_PIX_FMT_NV12,
		.depth		= 16,
		.buff_cnt	= 1,
		.plane_cnt	= 2
	}, {
		.name		= "Y/CbCr 4:2:2, Y/CbCr",
		.fourcc		= V4L2_PIX_FMT_NV16,
		.depth		= 16,
		.buff_cnt	= 1,
		.plane_cnt	= 2
	}, {
		.name		= "Y/CbCr 4:2:0 planar, Y/CbCr",
		.fourcc		= V4L2_PIX_FMT_NV12M,
		.depth		= 16,
		.buff_cnt	= 2,
		.plane_cnt	= 2
	}, {
		.name		= "Y/CbCr 4:2:2 planar, Y/CbCr",
		.fourcc		= V4L2_PIX_FMT_NV16M,
		.depth		= 16,
		.buff_cnt	= 2,
		.plane_cnt	= 2
	}
};

#define NUM_FORMATS ARRAY_SIZE(formats)

static int drime4_ipcm_convert_fourcc_to_ipc(struct drime4_ipcm *ipcm,
	struct ipcm_frame *frame)
{
	struct ipcm_fmt *fmt = NULL;

	if (ipcm == NULL) {
		dev_dbg(ipcm->dev,
			"%s: ipcm indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	if (frame == NULL)
		v4l2_err(&ipcm->v4l2_dev,
			"frame indicates NULL pointer.\n");
	else
		fmt = frame->fmt;

	if (fmt == NULL) {
		dev_dbg(ipcm->dev,
			"%s: fmt indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	switch (fmt->fourcc) {
	case V4L2_PIX_FMT_SBGGR8:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_BGGR;
		frame->bayer_info.BitPerPix = IPCM_8BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_BGGR.\n", __func__);
		break;
	case V4L2_PIX_FMT_SGBRG8:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_GBRG;
		frame->bayer_info.BitPerPix = IPCM_8BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_GBRG.\n", __func__);
		break;
	case V4L2_PIX_FMT_SGRBG8:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_GRBG;
		frame->bayer_info.BitPerPix = IPCM_8BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_GRBG.\n", __func__);
		break;
	case V4L2_PIX_FMT_SRGGB8:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_RGGB;
		frame->bayer_info.BitPerPix = IPCM_8BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_RGGB.\n", __func__);
		break;
	case V4L2_PIX_FMT_SBGGR10:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_BGGR;
		frame->bayer_info.BitPerPix = IPCM_10BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_BGGR.\n", __func__);
		break;
	case V4L2_PIX_FMT_SGBRG10:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_GBRG;
		frame->bayer_info.BitPerPix = IPCM_10BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_GBRG.\n", __func__);
		break;
	case V4L2_PIX_FMT_SGRBG10:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_GRBG;
		frame->bayer_info.BitPerPix = IPCM_10BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_GRBG.\n", __func__);
		break;
	case V4L2_PIX_FMT_SRGGB10:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_RGGB;
		frame->bayer_info.BitPerPix = IPCM_10BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_RGGB.\n", __func__);
		break;
	case V4L2_PIX_FMT_SBGGR12:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_BGGR;
		frame->bayer_info.BitPerPix = IPCM_12BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_BGGR.\n", __func__);
		break;
	case V4L2_PIX_FMT_SGBRG12:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_GBRG;
		frame->bayer_info.BitPerPix = IPCM_12BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_GBRG.\n", __func__);
		break;
	case V4L2_PIX_FMT_SGRBG12:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_GRBG;
		frame->bayer_info.BitPerPix = IPCM_12BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_GRBG.\n", __func__);
		break;
	case V4L2_PIX_FMT_SRGGB12:
		frame->img_format = IPCM_BAYER;
		frame->bayer_info.head_color = IPCM_RGGB;
		frame->bayer_info.BitPerPix = IPCM_12BIT;

		dev_dbg(ipcm->dev, "%s:IPCM_BAYER -> IPCM_RGGB.\n", __func__);
		break;
	case V4L2_PIX_FMT_YUV420:
		frame->img_format = IPCM_YCC;
		frame->ycc_info.ycc_type = IPCM_YCC420;

		dev_dbg(ipcm->dev, "%s:IPCM_YCC_420 -> IPC_CC_UNPACKED.\n",
			__func__);
		break;
	case V4L2_PIX_FMT_YUV422P:
		frame->img_format = IPCM_YCC;
		frame->ycc_info.ycc_type = IPCM_YCC422;

		dev_dbg(ipcm->dev, "%s:IPCM_YCC_422 -> IPC_CC_UNPACKED.\n",
			__func__);
		break;
	case V4L2_PIX_FMT_YUYV:
		frame->img_format = IPCM_YCC;
		frame->ycc_info.ycc_type = IPCM_YCC422;

		dev_dbg(ipcm->dev, "%s:IPCM_YCC_422 -> IPC_CC_UNPACKED.\n",
			__func__);
		break;
	case V4L2_PIX_FMT_NV12:
		frame->img_format = IPCM_YCC;
		frame->ycc_info.ycc_type = IPCM_YCC420;

		dev_dbg(ipcm->dev, "%s:IPCM_YCC_420 -> IPC_CC_PACKED.\n",
			__func__);
		break;
	case V4L2_PIX_FMT_NV16:
		frame->img_format = IPCM_YCC;
		frame->ycc_info.ycc_type = IPCM_YCC422;

		dev_dbg(ipcm->dev, "%s:IPCM_YCC_422 -> IPC_CC_PACKED.\n",
			__func__);
		break;
	case V4L2_PIX_FMT_NV12M:
		frame->img_format = IPCM_YCC;
		frame->ycc_info.ycc_type = IPCM_YCC420;

		dev_dbg(ipcm->dev, "%s:IPCM_YCC_420 Mutltiplanar -> IPC_CC_PACKED.\n",
			__func__);
		break;
	case V4L2_PIX_FMT_NV16M:
		frame->img_format = IPCM_YCC;
		frame->ycc_info.ycc_type = IPCM_YCC422;

		dev_dbg(ipcm->dev, "%s:IPCM_YCC_422 Multiplanar-> IPC_CC_PACKED.\n",
			__func__);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int drime4_ipcm_check_buffer_type(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx)
{
	if (ctx->curr_buf_type < IPCM_SRC_BUFFER ||
			ctx->curr_buf_type > IPCM_DST_SRSZ_BUFFER) {
		v4l2_err(&ipcm->v4l2_dev,
			"wrong buffer or video queue type(%d)\n",
			ctx->curr_buf_type);
		return -EINVAL;
	}
	return 0;
}

static int drime4_ipcm_check_memory_type(struct drime4_ipcm *ipcm, struct ipcm_ctx *ctx,
	enum v4l2_memory memory)
{
	if (memory != V4L2_MEMORY_MMAP &&
			memory != V4L2_MEMORY_USERPTR &&
			memory != V4L2_MEMORY_OVERLAY) {
		v4l2_err(&ipcm->v4l2_dev,
			"wrong memory type(%d)\n", memory);
		return -EINVAL;
	}

	return 0;
}

static int drime4_ipcm_open(struct file *filp)
{
	int ret = 0;
	struct drime4_ipcm *ipcm = video_drvdata(filp);
	struct ipcm_ctx *ctx = NULL;

	dev_dbg(ipcm->dev, "%s\n", __func__);

	ctx = kzalloc(sizeof(struct ipcm_ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	filp->private_data = ctx;
	ctx->ipcm_dev = ipcm;

	ctx->v4l2_op_status = IPCM_V4L2_STREAM_OFF;

	ipcm->ipcm_ctx = ctx;

	mutex_init(&ctx->lock);
	mutex_init(&ctx->s_lock);
	mutex_init(&ctx->d_lock);

	if (ipcm->ref_count.counter < 1) {
		ret = clk_enable(ipcm->clock);
		if (ret < 0) {
			dev_err(ipcm->dev, "failed to enable ipcm clock.\n");
			return -EFAULT;
		}
		/* IPCM Clock freq. */
		clk_set_rate(ipcm->clock, 200000000);
		/* reset ipcm and load register sets. */
		drime4_ipcm_fw_init(ipcm);
	}

	/* initialize interrupt type */
	ctx->interrupt_wdma0 = IPCM_INTR_ERROR_WDMA0;
	ctx->interrupt_wdma1 = IPCM_INTR_ERROR_WDMA1;
	ctx->interrupt_wdma2 = IPCM_INTR_ERROR_WDMA2;

	atomic_inc(&ipcm->ref_count);
	dev_dbg(ipcm->dev, "%s:ref_count = %d\n", __func__,
		ipcm->ref_count.counter);

	return ret;
}

static int drime4_ipcm_release(struct file *filp)
{
	int ret = 0;
	struct ipcm_ctx *ctx = (struct ipcm_ctx *)filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;

	WARN_ON(ctx == NULL);

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);
	/* free all buffer info */
	ret = drime4_ipcm_fw_free_buffer_info(ipcm, ctx);

	if (ret < 0)
		dev_err(ipcm->dev, "failed to free buffers.\n");

	if (ctx)
		kfree(ctx);

	if (ipcm->ref_count.counter <= 0) {

		/* disable clocks for IPCM. */
		clk_disable(ipcm->clock);
	}

	atomic_dec(&ipcm->ref_count);
	dev_dbg(ipcm->dev, "%s:ref_count = %d\n", __func__,
		ipcm->ref_count.counter);

	return ret;
}

static int drime4_ipcm_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned int page_frame_no = 0, vm_size = 0, align = 4096;
	struct ipcm_ctx *ctx = (struct ipcm_ctx *)filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;
	struct ipcm_mem_info *mem_info = NULL;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	mutex_lock(&ctx->lock);

	vma->vm_flags |= VM_RESERVED | VM_IO;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	vm_size = vma->vm_end - vma->vm_start;

	vm_size =  ALIGN(vm_size, align);

	/* get mem_info to current index and operation mode. */
	mem_info = drime4_ipcm_fw_get_buffer_info(ipcm, ctx);

	if (mem_info == NULL) {
		dev_err(ipcm->dev, "failed to mmap buffer.\n");
		mutex_unlock(&ctx->lock);
		return -EFAULT;
	}

	/* reserved memory size should be bigger then vm_size. */
	if (vm_size > mem_info->size) {
		dev_err(ipcm->dev,
			"reserved memory size is less then vm_size.\n");
		mutex_unlock(&ctx->lock);
		return -EINVAL;
	}

	page_frame_no = __phys_to_pfn(mem_info->phy_addr);

	if ((remap_pfn_range(vma, vma->vm_start, page_frame_no,
			vm_size, vma->vm_page_prot)) != 0) {
		dev_err(ipcm->dev, "failed to mmap.\n");
		mutex_unlock(&ctx->lock);
		return -EINVAL;
	}

	mem_info->mmap_addr = vma->vm_start;

	dev_dbg(ipcm->dev, "id = %d, mmaped address = 0x%x\n",
		mem_info->id, mem_info->mmap_addr);

	mutex_unlock(&ctx->lock);

	return 0;
}

static u32 drime4_ipcm_poll(struct file *filp, poll_table *wait)
{
	struct ipcm_ctx *ctx = filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;
	u32 mask = 0;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	if (ipcm_read_q_cnt > 0)
		mask |= (POLLIN | POLLRDNORM);
	else {
		poll_wait(filp, &ipcm_wq_read, wait);
		/* mask = POLLERR; */
	}

	return mask;
}

size_t drime4_ipcm_read(struct file *filp, int *num, size_t count, loff_t *f_pos)
{
	unsigned long flags;
	int retstate;

	if ((!ipcm_read_q_cnt) && (filp->f_flags & O_NONBLOCK))
		return -EAGAIN;

	retstate = wait_event_interruptible(ipcm_wq_read, ipcm_read_q_cnt);

	if (retstate)
		return retstate;

	local_save_flags(flags);
	local_irq_disable();

	if (ipcm_read_q_cnt > 0) {
		put_user(ipcm_read_queue[ipcm_read_q_tail], num);
		ipcm_read_q_tail = (ipcm_read_q_tail + 1) % MAX_IPCM_READ_QUEUE_CNT;
		ipcm_read_q_cnt--;
	}

	local_irq_restore(flags);

	return ipcm_read_q_cnt;
}

void drime4_ipcm_send_interrupt(unsigned int intr_type)
{
	unsigned long flags;

	local_save_flags(flags);
	local_irq_disable();

	if (ipcm_read_q_cnt < MAX_IPCM_READ_QUEUE_CNT) {
		ipcm_read_queue[ipcm_read_q_head] = intr_type;
		ipcm_read_q_head = (ipcm_read_q_head + 1) % MAX_IPCM_READ_QUEUE_CNT;
		ipcm_read_q_cnt++;
	}

	local_irq_restore(flags);

	wake_up_interruptible(&ipcm_wq_read);
}

struct v4l2_file_operations drime4_ipcm_fops = {
	.owner		= THIS_MODULE,
	.open		= drime4_ipcm_open,
	.release	= drime4_ipcm_release,
	.ioctl		= video_ioctl2,
	.mmap		= drime4_ipcm_mmap,
	.poll		= drime4_ipcm_poll,
	.read		= drime4_ipcm_read,
};

static int drime4_ipcm_querycap(struct file *filp, void *fh,
	struct v4l2_capability *cap)
{
	struct ipcm_ctx *ctx = filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	strncpy(cap->driver, ipcm->name, sizeof(cap->driver) - 1);
	strncpy(cap->card, ipcm->name, sizeof(cap->card) - 1);
	cap->bus_info[0] = 0;
	cap->version = KERNEL_VERSION(2, 6, 39);
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_VIDEO_OVERLAY |
		V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_VIDEO_OUTPUT_MPLANE |
		V4L2_CAP_VIDEO_OUTPUT_OVERLAY;

	return 0;
}

static int drime4_ipcm_check_ctrl_val(struct ipcm_ctx *ctx,
	struct v4l2_control *ctrl)
{
	if (ctrl->id < V4L2_CID_DRIME_IPC_MIN ||
			ctrl->id > V4L2_CID_DRIME_IPC_MAX) {
		v4l2_err(&ctx->ipcm_dev->v4l2_dev, "Invalid control id.\n");
		return -ERANGE;
	}

	return 0;
}

static int drime4_ipcm_s_ctrl(struct file *filp, void *fh,
	struct v4l2_control *ctrl)
{
	int ret = -1;
	struct ipcm_ctx *ctx = filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;
	struct ipcm_liveview_info *liveview_param = NULL;
	struct ipcm_ycc_combi_op_info *combi_op_param = NULL;
	struct ipcm_ycc_resize_info *resize_param = NULL;
	struct ipcm_md_info *md_param = NULL;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	dev_dbg(ipcm->dev, "%s, ipcm_ctx: %x0x\n", __func__, ctx);

	mutex_lock(&ctx->lock);
	liveview_param = &ctx->liveview_info;
	combi_op_param = &ctx->combi_op_info;
	resize_param = &ctx->resize_info;
	md_param = &ctx->md_info;

	/* check control id and value. */
	ret = drime4_ipcm_check_ctrl_val(ctx, ctrl);
	if (ret < 0) {
		mutex_unlock(&ctx->lock);
		return ret;
	}

	switch (ctrl->id) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		dev_dbg(ipcm->dev, "V4L2_CID_DRIME_IPC_LIVEVIEW.\n");

		switch (ctrl->value) {

		/* PP(Bayer) -> IPCM -> Main Resize -> wdma1 -> memory. */
		case V4L2_IPC_RESIZE_MAIN:
			liveview_param->Rsz_In_Sel = IPCM_RSZ_INSEL_MAINOUT;
			liveview_param->RSZ_SW = IPCM_SW_ON;
			break;

		/* PP(Bayer) -> IPCM -> Sub Resize -> wdma2 -> memory. */
		case V4L2_IPC_RESIZE_SUB1:
			liveview_param->Srsz_In_Sel = IPCM_SRSZ_INSEL_MAINOUT;
			liveview_param->SRSZ_SW = IPCM_SW_ON;
			break;

		case V4L2_IPC_RESIZE_MAIN_SUB1:
			liveview_param->Rsz_In_Sel = IPCM_RSZ_INSEL_MAINOUT;
			liveview_param->RSZ_SW = IPCM_SW_ON;
			liveview_param->Srsz_In_Sel = IPCM_SRSZ_INSEL_RSZOUT;
			liveview_param->SRSZ_SW = IPCM_SW_ON;
			break;


		default:
			liveview_param->MainOut_SW = IPCM_SW_ON;
			break;
		}
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		dev_dbg(ipcm->dev, "V4L2_CID_DRIME_IPC_RESIZE.\n");

		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON) {

			switch (ctrl->value) {

			/* memory(ycc)-> rdma0 -> IPCM(Main Resize)-> wdma1 -> memory. */
			case V4L2_IPC_RESIZE_MAIN:
				combi_op_param->RSZ_SW = IPCM_SW_ON;
				combi_op_param->SRSZ_SW = IPCM_SW_PRESERVE;
				break;

			/* memory(ycc)-> rdma0 -> IPCM(Resize) -> wdma1 -> memory. */
			/* memory(ycc)-> rdma0 -> IPCM(Resize) ->
			   IPCM(Sub Resize) -> wdma2 -> memory.(serial) */
			case V4L2_IPC_RESIZE_MAIN_SUB1:
				combi_op_param->RSZ_SW = IPCM_SW_ON;
				combi_op_param->SRSZ_SW = IPCM_SW_ON;
				/* IPCM(Resize)-> IPCM(Sub-Resize) (serial) */
				combi_op_param->Srsz_In_Sel = IPCM_SRSZ_INSEL_RSZOUT;
				break;

			default:
				break;
			}

			/* set resize parameters */
			combi_op_param->RSZ_MODE = IPCM_RSZ_NORMAL;
			/* set LPF parameter automatically */
			combi_op_param->srsz_fir_x_autoset = 1;
			combi_op_param->srsz_fir_y_autoset = 1;
			combi_op_param->srsz_fir_x = 0;
			combi_op_param->srsz_fir_y = 0;
		/* resize only (without liveview)*/
		} else {

			switch (ctrl->value) {

			/* memory(ycc)-> rdma0 -> IPCM(Main) ->
			   IPCM(Resize) -> wdma1 -> memory. */
			case V4L2_IPC_RESIZE_MAIN:
				resize_param->RSZ_SW = IPCM_SW_ON;
				/* using RDMA input */
				resize_param->Rsz_In_Sel = IPCM_RSZ_INSEL_RDMA;
				break;

			/* memory(ycc)-> rdma0 -> IPCM(Main) ->
			   IPCM(Resize) -> wdma1 -> memory. */
			/* memory(ycc)-> rdma0 -> IPCM(Main) ->
			   IPCM(Resize) -> IPCM(Sub Resize) -> wdma2 -> memory.(serial) */
			case V4L2_IPC_RESIZE_MAIN_SUB1:
				resize_param->RSZ_SW = IPCM_SW_ON;
				resize_param->SRSZ_SW = IPCM_SW_ON;
				/* IPCM(Main) -> IPCM(Resize) -> IPCM(Sub-Resize) (serial) */
				resize_param->Srsz_In_Sel = IPCM_SRSZ_INSEL_RSZOUT;
				break;

			/* memory(ycc)-> rdma0 -> IPCM(Main) -> No Resize -> wdma0 -> memory. */
			default:
				resize_param->MainOut_SW = IPCM_SW_ON;
				break;
			}

			/* set sub-module sw */
			resize_param->YCC2RGB_SW = 0;
			resize_param->PCSE_SW = 0;
			resize_param->PCSE_GHC_SP_SW = 0;
			resize_param->PCSE_GSC_SP_SW = 0;
			resize_param->PCSE_PSHC_SW = 0;
			resize_param->PCSE_PHC_SP_SW = 0;
			resize_param->PCSE_PSC_SP_SW = 0;
			resize_param->PCSE_SE_SW = 0;

			resize_param->YCNR_SW = 0;
			resize_param->Edge_NR_SW = 0;
			resize_param->YLUT_SW = 0;
			resize_param->MONO_SW = 0;
			resize_param->NP_SW = 0;
			resize_param->CNR_SW = 0;

			resize_param->CHM_LUT_SW = 0;
			/* Color Correction : normal out */
			resize_param->GC_Sel = IPCM_CC_OUT;

			/* resize mode */
			resize_param->RSZ_MODE = IPCM_RSZ_NORMAL;
			/* resize effect */
			resize_param->no_effect_rsz = 1;
			resize_param->no_effect_srsz = 1;
			/* set LPF parameter automatically */
			resize_param->srsz_fir_x_autoset = 1;
			resize_param->srsz_fir_y_autoset = 1;
			resize_param->srsz_fir_x = 0;
			resize_param->srsz_fir_y = 0;
		}
		break;

	case V4L2_CID_DRIME_IPC_MD:
		dev_dbg(ipcm->dev, "V4L2_CID_DRIME_IPC_MD.\n");

		switch (ctrl->value) {
		case V4L2_IPCM_MD_LIVEVIEW_INPUT:
			md_param->in_sel = IPCM_MD_IN_LIVEVIEW;
			break;
		case V4L2_IPCM_MD_RDMA_INPUT:
			md_param->in_sel = IPCM_MD_IN_YCC;
			break;

		default:
			v4l2_err(&ipcm->v4l2_dev, "Invalid control value.\n");
			mutex_unlock(&ctx->lock);
			return -EINVAL;
		}
		break;

	default:
		v4l2_err(&ipcm->v4l2_dev, "Invalid control ID.\n");
		mutex_unlock(&ctx->lock);

		return -EINVAL;
	}

	/*
	 * this would be used to indentity
	 * what is current operation mode based v4l2 interface.
	 */
	ctx->v4l2_op_mode = ctrl->id;
	ctx->v4l2_ctrl_value = ctrl->value;

	mutex_unlock(&ctx->lock);

	return 0;
}

static int drime4_ipcm_s_ext_ctrls(struct file *filp, void *fh,
	struct v4l2_ext_controls *ctrls)
{
	struct ipcm_ctx *ctx = filp->private_data;
	struct v4l2_ext_control *controls = ctrls->controls;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;
	struct ipcm_liveview_info *liveview_param = NULL;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s: start\n", __func__);

	if ((ctx == NULL) || (ipcm == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ctx/ipcm indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	mutex_lock(&ctx->lock);
	liveview_param = &ctx->liveview_info;

	/* ipcm sub-module control */
	switch (ctrls->ctrl_class) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:

		switch (controls->id) {
		/* Main out Switch */
		case V4L2_IPCM_LIVEVIEW_MAIN_OUT_SW:
			if (controls->value)
				liveview_param->MainOut_SW = IPCM_SW_ON;
			else
				liveview_param->MainOut_SW = IPCM_SW_OFF;
			break;
		/* Resize out Switch */
		case V4L2_IPCM_LIVEVIEW_RSZ_OUT_SW:
			if (controls->value)
				liveview_param->RSZ_SW = IPCM_SW_ON;
			else
				liveview_param->RSZ_SW = IPCM_SW_OFF;
			break;
		/* Sub-Resize out Switch */
		case V4L2_IPCM_LIVEVIEW_SRSZ_OUT_SW:
			if (controls->value)
				liveview_param->SRSZ_SW = IPCM_SW_ON;
			else
				liveview_param->SRSZ_SW = IPCM_SW_OFF;
			break;
		/* HD Resize Switch */
		case V4L2_IPCM_LIVEVIEW_HD_RSZ_SW:
			if (controls->value) {
				liveview_param->HDRSZ_x_on = 1;
				liveview_param->HDRSZ_y_on = 1;
			} else {
				liveview_param->HDRSZ_x_on = 0;
				liveview_param->HDRSZ_y_on = 0;
			}
			break;

		default:
			break;
		}
		break;

	default:
		v4l2_err(&ipcm->v4l2_dev, "Invalid control ID.\n");
		mutex_unlock(&ctx->lock);

		return -EINVAL;
	}

	mutex_unlock(&ctx->lock);
	return 0;
}

static int drime4_ipcm_g_ctrl(struct file *filp, void *fh,
	struct v4l2_control *ctrl)
{
	struct ipcm_ctx *ctx = filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	return 0;
}

static int drime4_ipcm_qbuf(struct file *filp, void *fh, struct v4l2_buffer *b)
{
	struct ipcm_ctx *ctx = filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;
	int ret = 0;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s: start\n", __func__);

	WARN_ON(ctx == NULL);

	if (ctx->curr_buf_type < IPCM_SRC_BUFFER ||
			ctx->curr_buf_type > IPCM_DST_SRSZ_BUFFER) {
		v4l2_err(&ipcm->v4l2_dev,
			"wrong buffer or video queue type(%d)\n", b->type);
		return -EINVAL;
	}

	/* check memory type. */
	ret = drime4_ipcm_check_memory_type(ipcm, ctx, b->memory);
	if (ret < 0)
		return -EINVAL;

	mutex_lock(&ctx->lock);

	if (ctx->curr_buf_type == IPCM_SRC_BUFFER)
		ret = drime4_ipcm_fw_src_enqueue_buffer(ipcm, ctx, b);
	else
		ret = drime4_ipcm_fw_dst_enqueue_buffer(ipcm, ctx, b);

	mutex_unlock(&ctx->lock);

	return ret;

}

static int drime4_ipcm_dqbuf(struct file *filp, void *fh, struct v4l2_buffer *b)
{
	struct ipcm_ctx *ctx = filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;
	int ret = 0;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s: start\n", __func__);

	WARN_ON(ctx == NULL);

	if (ctx->curr_buf_type < IPCM_SRC_BUFFER ||
			ctx->curr_buf_type > IPCM_DST_SRSZ_BUFFER) {
		v4l2_err(&ipcm->v4l2_dev,
			"wrong buffer or video queue type(%d)\n", b->type);
		return -EINVAL;
	}

	/* check memory type. */
	ret = drime4_ipcm_check_memory_type(ipcm, ctx, b->memory);
	if (ret < 0)
		return -EINVAL;

	mutex_lock(&ctx->lock);

	if (ctx->curr_buf_type == IPCM_SRC_BUFFER)
		ret = drime4_ipcm_fw_src_dequeue_buffer(ipcm, ctx, b);
	else
		ret = drime4_ipcm_fw_dst_dequeue_buffer(ipcm, ctx, b);

	/* set the buffer index got above to ipcs context. */
	drime4_ipcm_fw_set_buffer_to_ctx(ipcm, ctx, b);

	mutex_unlock(&ctx->lock);
	return ret;
}

static int drime4_ipcm_g_parm(struct file *filp, void *fh,
	struct v4l2_streamparm *parms)
{
	struct ipcm_ctx *ctx = filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s: start\n", __func__);

	return 0;
}

static int drime4_ipcm_s_parm(struct file *filp, void *fh,
	struct v4l2_streamparm *parms)
{
	struct ipcm_ctx *ctx = filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;
	struct v4l2_captureparm	*capture = &parms->parm.capture;
	struct v4l2_outputparm *output = &parms->parm.output;
	int ret = 0;

	ipcm->ipcm_ctx = ctx;

	/* V4L2_BUF_TYPE_PRIVATE would be used for PP preview mode. */
	if (parms->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ||
			parms->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ||
			parms->type == V4L2_BUF_TYPE_PRIVATE) {
		ret = drime4_ipcm_fw_change_src_addr(ipcm, output);

	} else if (parms->type == V4L2_BUF_TYPE_VIDEO_CAPTURE ||
			parms->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		ret = drime4_ipcm_fw_change_dst_addr(ipcm, capture);
	} else {
		v4l2_err(&ipcm->v4l2_dev,
			"wrong buffer type(%d)\n", parms->type);
		return -EINVAL;
	}

	return ret;
}

static struct ipcm_fmt *find_format(struct v4l2_format *f)
{
	struct ipcm_fmt *fmt;
	unsigned int i;

	for (i = 0; i < NUM_FORMATS; i++) {
		fmt = &formats[i];
		if (fmt->fourcc == f->fmt.pix.pixelformat)
			break;
	}

	if (i == NUM_FORMATS)
		return NULL;

	return fmt;
}

static int drime4_ipcm_try_fmt(struct file *fime, void *priv,
	struct v4l2_format *f)
{
	struct ipcm_fmt *fmt;
	struct ipcm_ctx *ctx = (struct ipcm_ctx *)priv;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_pix_format_mplane *pix_mp = &f->fmt.pix_mp;
	enum v4l2_field field;
	enum ipcm_scan_type scan_type = IPCM_PROGRESSIVE;

	ipcm->ipcm_ctx = ctx;

	fmt = find_format(f);

	if (!fmt) {
		v4l2_err(&ipcm->v4l2_dev, "fourcc format (0x%x) invaild.\n",
			pix->pixelformat);
		return -EINVAL;
	}

	/* check multi plane use */
	if (fmt->buff_cnt > 1)
		field = pix_mp->field;
	else
		field = pix->field;

	if (field == V4L2_FIELD_ANY) {
		pix->field = V4L2_FIELD_NONE;
		pix_mp->field = V4L2_FIELD_NONE;
	} else if (field == V4L2_FIELD_NONE)
		scan_type = IPCM_PROGRESSIVE;
	else if (field == V4L2_FIELD_INTERLACED)
		scan_type = IPCM_INTERLACE;
	else {
		v4l2_err(&ctx->ipcm_dev->v4l2_dev, "Invalid value.\n");
		return -EINVAL;
	}

	/* set ipcm current frame format */
	if (ctx->curr_buf_type == IPCM_SRC_BUFFER) {
		ctx->src_frame.fmt = fmt;
		ctx->src_frame.ycc_info.scan_type = scan_type;
	} else {
		ctx->dst_frame.fmt = fmt;
		ctx->dst_frame.ycc_info.scan_type = scan_type;
	}

	return 0;
}

static int drime4_ipcm_s_fmt(struct file *file, void *fh, struct v4l2_format *f)
{
	struct ipcm_frame *frame;
	struct v4l2_pix_format *pix;
	struct v4l2_pix_format_mplane *pix_mp;
	struct ipcm_ctx *ctx = file->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;
	int ret = -1;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	mutex_lock(&ctx->lock);

	pix = &f->fmt.pix;
	pix_mp = &f->fmt.pix_mp;

	/* V4L2_BUF_TYPE_PRIVATE would be used for PP preview mode. */
	if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ||
			f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ||
			f->type == V4L2_BUF_TYPE_PRIVATE) {
		frame = &ctx->src_frame;
		ctx->curr_buf_type = IPCM_SRC_BUFFER;
	} else if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE ||
			f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		frame = &ctx->dst_frame;
		/* check destination buffer type */
		if (pix->priv == V4L2_IPC_RESIZE_MAIN)
			ctx->curr_buf_type = IPCM_DST_RSZ_BUFFER;
		else if (pix->priv == V4L2_IPC_RESIZE_SUB1)
			ctx->curr_buf_type = IPCM_DST_SRSZ_BUFFER;
		else
			ctx->curr_buf_type = IPCM_DST_MAIN_BUFFER;
	} else {
		v4l2_err(&ipcm->v4l2_dev,
			"wrong buffer or video queue type(%d)\n", f->type);
		goto err_type;
	}

	ret = drime4_ipcm_try_fmt(file, ctx, f);
	if (ret) {
		mutex_unlock(&ctx->lock);
		return ret;
	}

	ret = drime4_ipcm_convert_fourcc_to_ipc(ipcm, frame);
	if (ret < 0)
	    goto err_type;

	/* check multi plane use*/
	if (frame->fmt->buff_cnt > 1) {
		/* set image size */
		if (frame->img_format == IPCM_BAYER) {
			frame->bayer_info.width = pix_mp->width;
			frame->bayer_info.height = pix_mp->height;
			frame->bayer_info.stride = pix_mp->plane_fmt[0].bytesperline;
		} else {
			frame->ycc_info.width = pix_mp->width;
			frame->ycc_info.height = pix_mp->height;
			frame->ycc_info.stride_y = pix_mp->plane_fmt[0].bytesperline;
			frame->ycc_info.stride_c = pix_mp->plane_fmt[1].bytesperline;
			frame->plane_buf_size[0] = pix_mp->plane_fmt[0].sizeimage;
			frame->plane_buf_size[1] = pix_mp->plane_fmt[1].sizeimage;
		}
		/* check width & stride size. */
		dev_dbg(ipcm->dev, "%s:pix_mp->width = %d, stride_size = %d, plane_buf_size[0] = %d, plane_buf_size[1] = %d\n",
			__func__, pix_mp->width, pix_mp->plane_fmt[0].bytesperline,
			frame->plane_buf_size[0], frame->plane_buf_size[1]);
	} else {
		/* set image size */
		if (frame->img_format == IPCM_BAYER) {
			frame->bayer_info.width = pix->width;
			frame->bayer_info.height = pix->height;
			frame->bayer_info.stride = pix->bytesperline;
		} else {
			frame->ycc_info.width = pix->width;
			frame->ycc_info.height = pix->height;
			frame->ycc_info.stride_y = pix->bytesperline;
			frame->ycc_info.stride_c = pix->bytesperline;
		}
		/* check width & stride_size */
		dev_dbg(ipcm->dev, "%s:pix->width = %d, stride_size = %d\n",
			__func__, pix->width, pix->bytesperline);
	}

	/* for source. */
	if (ctx->curr_buf_type == IPCM_SRC_BUFFER)
		drime4_ipcm_fw_set_src_param(ctx, frame);
	/* for destination. */
	else
		drime4_ipcm_fw_set_dst_param(ctx, frame);

	dev_dbg(ipcm->dev, "width = %d, height = %d\n", pix->width,
							pix->height);

	mutex_unlock(&ctx->lock);
	return 0;

err_type:
	mutex_unlock(&ctx->lock);
	return -EINVAL;
}

static int drime4_ipcm_g_fmt(struct file *file, void *fh, struct v4l2_format *f)
{
	return 0;
}

static int drime4_ipcm_reqbufs(struct file *filp, void *fh,
	struct v4l2_requestbuffers *b)
{
	struct ipcm_ctx *ctx = filp->private_data;
	struct drime4_ipcm *ipcm = ctx->ipcm_dev;
	int ret = -1;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	/* check buffer type. */
	ret = drime4_ipcm_check_buffer_type(ipcm, ctx);
	if (ret < 0)
		return -EINVAL;

	/* check memory type. */
	ret = drime4_ipcm_check_memory_type(ipcm, ctx, b->memory);
	if (ret < 0)
		return -EINVAL;

	/* check free buffer. */
	if (b->count == 0) {
		ret = drime4_ipcm_fw_free_buffer(ipcm, b);
		return ret;
	}

	/* for source. */
	if (ctx->curr_buf_type == IPCM_SRC_BUFFER) {
		mutex_lock(&ctx->lock);

		switch (ctx->v4l2_op_mode) {
		/* it doesn't need to allocate source buffer. */
		case V4L2_CID_DRIME_IPC_LIVEVIEW:
			break;
		case V4L2_CID_DRIME_IPC_RESIZE:
			drime4_ipcm_fw_set_ycc_inbuf(ipcm, b);
			break;
		default:
			break;
		}

		mutex_unlock(&ctx->lock);
	/* for destination. */
	} else {
		mutex_lock(&ctx->lock);

		/* No Resize Image */
		if (ctx->curr_buf_type == IPCM_DST_MAIN_BUFFER)
			drime4_ipcm_fw_set_main_outbuf(ipcm, b);
		/* Main Resize Image */
		else if (ctx->curr_buf_type == IPCM_DST_RSZ_BUFFER)
			drime4_ipcm_fw_set_rsz_outbuf(ipcm, b);
		/* Sub Resize Image */
		else if (ctx->curr_buf_type == IPCM_DST_SRSZ_BUFFER)
			drime4_ipcm_fw_set_subrsz_outbuf(ipcm, b);

		mutex_unlock(&ctx->lock);
	}

	return 0;
}

static int drime4_ipcm_querybuf(struct file *filp, void *fh,
	struct v4l2_buffer *b)
{
	struct drime4_ipcm *ipcm = video_drvdata(filp);
	struct ipcm_ctx *ctx = (struct ipcm_ctx *)filp->private_data;
	int ret = -1;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	if (ctx->curr_buf_type < IPCM_SRC_BUFFER ||
			ctx->curr_buf_type > IPCM_DST_SRSZ_BUFFER) {
		v4l2_err(&ipcm->v4l2_dev,
			"wrong buffer or video queue type(%d)\n", b->type);
		return -EINVAL;
	}

	/* for source. */
	if (ctx->curr_buf_type == IPCM_SRC_BUFFER) {
		mutex_lock(&ctx->lock);

		switch (ctx->v4l2_op_mode) {
		/* Liveview */
		case V4L2_CID_DRIME_IPC_LIVEVIEW:
			break;
		/* Resize */
		case V4L2_CID_DRIME_IPC_RESIZE:
			/* check buffer count. */
			if (ctx->ycc_in_buf.buf_cnt < b->index) {
				dev_err(ipcm->dev, "%s:exceeded max buffer.\n",
					__func__);
				ret = -EINVAL;
				goto out;
			}

			ctx->ycc_in_buf.index = b->index;
			b->length =
				ctx->ycc_in_buf.meminfo[b->index]->size;
			/* store physcial address */
			b->reserved =
				ctx->ycc_in_buf.meminfo[b->index]->phy_addr;
			break;
		default:
			break;
		}

		mutex_unlock(&ctx->lock);
	/* for destination. */
	} else if (ctx->curr_buf_type == IPCM_DST_MAIN_BUFFER) {

		mutex_lock(&ctx->lock);

		/* check buffer count. */
		if (ctx->main_out_buf.buf_cnt < b->index) {
			dev_err(ipcm->dev, "%s:exceeded max buffer.\n",
				__func__);
			ret = -EINVAL;
			goto out;
		}
		ctx->main_out_buf.index = b->index;
		b->length =
			ctx->main_out_buf.meminfo[b->index]->size;
		/* store physcial address */
		b->reserved =
			ctx->main_out_buf.meminfo[b->index]->phy_addr;

		mutex_unlock(&ctx->lock);
	} else if (ctx->curr_buf_type == IPCM_DST_RSZ_BUFFER) {

		mutex_lock(&ctx->lock);

		/* check buffer count. */
		if (ctx->rsz_out_buf.buf_cnt < b->index) {
			dev_err(ipcm->dev, "%s:exceeded max buffer.\n",
				__func__);
			ret = -EINVAL;
			goto out;
		}
		ctx->rsz_out_buf.index = b->index;
		b->length =
			ctx->rsz_out_buf.meminfo[b->index]->size;
		/* store physcial address */
		b->reserved =
			ctx->rsz_out_buf.meminfo[b->index]->phy_addr;

		mutex_unlock(&ctx->lock);
	} else if (ctx->curr_buf_type == IPCM_DST_SRSZ_BUFFER) {

		mutex_lock(&ctx->lock);

		/* check buffer count. */
		if (ctx->subrsz_out_buf.buf_cnt < b->index) {
			dev_err(ipcm->dev, "%s:exceeded max buffer.\n",
				__func__);
			ret = -EINVAL;
			goto out;
		}
		ctx->subrsz_out_buf.index = b->index;
		b->length =
			ctx->subrsz_out_buf.meminfo[b->index]->size;
		/* store physcial address */
		b->reserved =
			ctx->subrsz_out_buf.meminfo[b->index]->phy_addr;

		mutex_unlock(&ctx->lock);
	}

	return 0;

out:
	mutex_unlock(&ctx->lock);
	return ret;
}

static int drime4_ipcm_streamon(struct file *filp, void *fh,
	enum v4l2_buf_type buf_type)
{
	struct drime4_ipcm *ipcm = video_drvdata(filp);
	struct ipcm_ctx *ctx = (struct ipcm_ctx *)filp->private_data;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	if ((ctx == NULL) || (ipcm == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ctx/ipcm indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	if (ctx->curr_buf_type < IPCM_SRC_BUFFER ||
			ctx->curr_buf_type > IPCM_DST_SRSZ_BUFFER) {
		v4l2_err(&ipcm->v4l2_dev,
			"wrong buffer or video queue type.\n");
		return -EINVAL;
	}

	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		ipcm_liveview_operation = IPCM_LIVEVIEW_ON;
		drime4_ipcm_fw_start_liveview(ipcm, ctx);
		break;
	case V4L2_CID_DRIME_IPC_RESIZE:
		drime4_ipcm_fw_start_resize(ipcm, ctx);
		break;
	case V4L2_CID_DRIME_IPC_BYPASS:
		break;
	case V4L2_CID_DRIME_IPC_MD:
		drime4_ipcm_fw_start_md(ipcm, ctx);
		break;
	default:
		dev_warn(ipcm->dev, "%s:wrong ipcm op mode.\n", __func__);
		break;
	}

	ctx->v4l2_op_status = IPCM_V4L2_STREAM_ON;

	return 0;
}

static int drime4_ipcm_streamoff(struct file *filp, void *fh,
	enum v4l2_buf_type i)
{
	int ret = 0;
	struct drime4_ipcm *ipcm = video_drvdata(filp);
	struct ipcm_ctx *ctx = (struct ipcm_ctx *)filp->private_data;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	if ((ctx == NULL) || (ipcm == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ctx/ipcm indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	/* sleep when liveview operation is working */
	if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON) {
		ipcm_liveview_operation = IPCM_LIVEVIEW_OFF;
		/* stop liveview */
		drime4_ipcm_fw_stop_liveview(ipcm, ctx);
	}

	ctx->v4l2_op_status = IPCM_V4L2_STREAM_OFF;

	return ret;
}

static long drime4_ipcm_private_ctrl(struct file *filp, void *fh,
	bool valid_prio, int cmd, void *arg)
{
	long ret = 0;
	struct drime4_ipcm *ipcm = video_drvdata(filp);
	struct ipcm_ctx *ctx = (struct ipcm_ctx *)filp->private_data;
	struct v4l2_d4_private_control *d4_private_ctrl;
	d4_private_ctrl = (struct v4l2_d4_private_control *)arg;

	ipcm->ipcm_ctx = ctx;
	dev_dbg(ipcm->dev, "%s\n", __func__);

	if (d4_private_ctrl->id < V4L2_CID_IPCM_WB ||
			d4_private_ctrl->id > V4L2_CID_IPCM_TONE_LUT) {
		v4l2_err(&ipcm->v4l2_dev,
			"Invalid control ID: (0x%x)\n", d4_private_ctrl->id);
		return -EINVAL;
	} else {
		ret = drime4_ipcm_fw_sub_module_ctrl(ipcm, d4_private_ctrl);
	}

	return ret;
}

struct v4l2_ioctl_ops drime4_ipcm_v4l2_ops = {
	.vidioc_querycap				= drime4_ipcm_querycap,
	.vidioc_s_ctrl					= drime4_ipcm_s_ctrl,
	.vidioc_s_ext_ctrls				= drime4_ipcm_s_ext_ctrls,
	.vidioc_g_ctrl					= drime4_ipcm_g_ctrl,
	.vidioc_qbuf					= drime4_ipcm_qbuf,
	.vidioc_dqbuf					= drime4_ipcm_dqbuf,
	.vidioc_g_parm					= drime4_ipcm_g_parm,
	.vidioc_s_parm					= drime4_ipcm_s_parm,
	.vidioc_s_fmt_vid_out			= drime4_ipcm_s_fmt,
	.vidioc_g_fmt_vid_out			= drime4_ipcm_g_fmt,
	.vidioc_s_fmt_vid_cap			= drime4_ipcm_s_fmt,
	.vidioc_g_fmt_vid_cap			= drime4_ipcm_g_fmt,
	.vidioc_s_fmt_type_private		= drime4_ipcm_s_fmt,
	.vidioc_g_fmt_type_private		= drime4_ipcm_g_fmt,
	.vidioc_s_fmt_vid_out_mplane	= drime4_ipcm_s_fmt,
	.vidioc_g_fmt_vid_out_mplane	= drime4_ipcm_g_fmt,
	.vidioc_s_fmt_vid_cap_mplane	= drime4_ipcm_s_fmt,
	.vidioc_g_fmt_vid_cap_mplane	= drime4_ipcm_g_fmt,
	.vidioc_try_fmt_vid_cap_mplane	= drime4_ipcm_try_fmt,
	.vidioc_try_fmt_vid_out_mplane	= drime4_ipcm_try_fmt,
	.vidioc_reqbufs					= drime4_ipcm_reqbufs,
	.vidioc_querybuf				= drime4_ipcm_querybuf,
	.vidioc_streamon				= drime4_ipcm_streamon,
	.vidioc_streamoff				= drime4_ipcm_streamoff,
	.vidioc_default					= drime4_ipcm_private_ctrl,
};

MODULE_AUTHOR("Jangwon Lee <jang_won.lee@samsung.com>");
MODULE_DESCRIPTION("Samsung Drime IV Image Processing Chain for Movie driver using V4L2");
MODULE_LICENSE("GPL");

