/* include/media/drime4/ipcm/d4_ipcm_v4l2.h
 *
 * V4L2 based Samsung Drime IV IPCM Interface header.
 *
 * Copyright (c) 2011 Samsung Electronics
 *	Jangwon Lee <jangwon_lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __D4_IPCM_V4L2_H__
#define __D4_IPCM_V4L2_H__

#define IPCM_MAX_PLANES	2

/*
 * IPCM current buffer type
 */
enum ipcm_curr_buf_type {
	IPCM_SRC_BUFFER = 1,
	IPCM_DST_MAIN_BUFFER,
	IPCM_DST_RSZ_BUFFER,
	IPCM_DST_SRSZ_BUFFER
};

/*
 * IPCM image format
 */
enum ipcm_img_format {
	IPCM_BAYER,
	IPCM_YCC
};

/*
 * IPCM liveview operation
 */
enum ipcm_liveview_op {
	IPCM_LIVEVIEW_OFF,
	IPCM_LIVEVIEW_ON
};

/*
 * IPCM v4l2 operation status
 */
enum ipcm_v4l2_op_status {
	IPCM_V4L2_STREAM_ON,
	IPCM_V4L2_STREAM_OFF,
	IPCM_V4L2_COMPLETE
};

/*
 * ipcm_fmt: change v4l2 style format to ipc style data format.
 */
struct ipcm_fmt {
	char		*name;
	unsigned int	fourcc;
	unsigned int	color;
	unsigned int	depth;
	unsigned int	buff_cnt;
	unsigned int	plane_cnt;
};

/*
 * ipcm_mem_info: memory information for mmap
 */
struct ipcm_mem_info {
	int		id;
	unsigned int	size;
	unsigned int	vir_addr;
	unsigned int	phy_addr;
	unsigned int	mmap_addr;
};

/*
 * ipcm_buffer: to handle ipcs memory buffer
 */
struct ipcm_buffer {
	unsigned int is_allocated;
	unsigned int index;
	unsigned int buf_cnt;

	struct ipcm_mem_info **meminfo;

	unsigned int flag;
};

/*
 * ipcm_frame: ipcm current frame information.
 */
struct ipcm_frame {
	enum ipcm_img_format		img_format;
	struct ipcm_bayer_info		bayer_info;
	struct ipcm_ycc_info		ycc_info;
	struct ipcm_fmt				*fmt;
	unsigned int				plane_buf_size[IPCM_MAX_PLANES];
};

extern struct v4l2_file_operations drime4_ipcm_fops;

extern struct v4l2_ioctl_ops drime4_ipcm_v4l2_ops;

#endif

