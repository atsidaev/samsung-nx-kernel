/*
 * @brief Implements hdmi video device driver,
 * @author somabha bhattacharjya<b.somabha@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#include <video/hdmi/d4_hdmi_video_type.h>
#include <video/hdmi/d4_hdmi_video_ioctl.h>

#include "d4_hdmi_regs.h"
#include "d4_hdmi.h"
#include <mach/hdmi/audio/d4_hdmi_audio.h>
#include "d4_hdmi_key_public.h"

#define VERSION "1.2" /* Driver version number */
#define HDMI_MINOR 240 /* Major 10, Minor 240, /dev/hdmi */

#define HDMI_DEBUG_TIME

#ifdef HDMI_DEBUG
#define DPRINTK(args...)		printk(args)
#else
#define DPRINTK(args...)
#endif

/** I2C device address of HDCP Rx port*/
#define HDCP_RX_DEV_ADDR		0x74

/** Ri offset on HDCP Rx port */
#define HDCP_RI_OFFSET			0x08

/** Size of Ri */
#define HDCP_RI_SIZE			2

#ifdef HDMI_DEBUG_TIME
unsigned long jstart, jend;
unsigned long ji2cstart, ji2cend;
#endif

struct hdcp_structa hdcp_struct;

/* Structure for video timing parameters	*/
static const struct hdmi_video_params HDMIVideoParams[] = {
		 /* v640x480p_60Hz	*/
		{ 800, 160, 525, 45, 16, 96, 1, 10, 2, 1, 1, 1, 0, 0, },
		/* v720x480p_60Hz	*/
		{ 858, 138, 525, 45, 16, 62, 1, 9, 6, 1, 2, 3, 0, 0, },
		 /* v1280x720p_60Hz	*/
		{ 1650, 370, 750, 30, 110, 40, 0, 5, 5, 0, 4, 4, 0, 0, },
		 /* v1920x1080i_60H	*/
		{ 2200, 280, 1125, 22, 88, 44, 0, 2, 5, 0, 5, 5, 1, 0, },
		 /* v720x480i_60Hz	*/
		{ 1716, 276, 525, 22, 38, 124, 1, 4, 3, 1, 6, 7, 1, 1, },
		 /* v720x240p_60Hz	*/
		{ 1716, 276, 262, 22, 38, 124, 1, 4, 3, 1, 8, 9, 0, 1, },
		 /* v2880x480i_60Hz	*/
		{ 3432, 552, 525, 22, 76, 248, 1, 4, 3, 1, 10, 11, 1, 1, },
		 /* v2880x240p_60Hz	*/
		{ 3432, 552, 262, 22, 76, 248, 1, 4, 3, 1, 12, 13, 0, 1, },
		 /* v1440x480p_60Hz	*/
		{ 1716, 276, 525, 45, 32, 124, 1, 9, 6, 1, 14, 15, 0, 1, },
		 /* v1920x1080p_60Hz	*/
		{ 2200, 280, 1125, 45, 88, 44, 0, 4, 5, 0, 16, 16, 0, 0, },
		 /* v720x576p_50Hz	*/
		{ 864, 144, 625, 49, 12, 64, 1, 5, 5, 1, 17, 18, 0, 0, },
		 /* v1280x720p_50Hz	*/
		{ 1980, 700, 750, 30, 440, 40, 0, 5, 5, 0, 19, 19, 0, 0, },
		 /* v1920x1080i_50H	*/
		{ 2640, 720, 1125, 22, 528, 44, 0, 2, 5, 0, 20, 20, 1, 0, },
		/* v720x576i_50Hz	*/
		{ 1728, 288, 625, 24, 24, 126, 1, 2, 3, 1, 21, 22, 1, 1, },
		 /* v720x288p_50Hz	*/
		{ 1728, 288, 312, 24, 24, 126, 1, 2, 3, 1, 23, 24, 0, 1, },
		 /* v2880x576i_50Hz	*/
		{ 3456, 576, 625, 24, 48, 252, 1, 2, 3, 1, 25, 26, 1, 1, },
		 /* v2880x288p_50Hz	*/
		{ 3456, 576, 312, 24, 48, 252, 1, 2, 3, 1, 27, 28, 0, 1, },
		 /* v1440x576p_50Hz	*/
		{ 1728, 288, 625, 49, 24, 128, 1, 5, 5, 1, 29, 30, 0, 1, },
		 /* v1920x1080p_50Hz	*/
		{ 2640, 720, 1125, 45, 528, 44, 0, 4, 5, 0, 31, 31, 0, 0, },
		 /* v1920x1080p_24Hz	*/
		{ 2750, 830, 1125, 45, 638, 44, 0, 4, 5, 0, 32, 32, 0, 0, },
		 /* v1920x1080p_25Hz	*/
		{ 2640, 720, 1125, 45, 528, 44, 0, 4, 5, 0, 33, 33, 0, 0, },
		 /* v1920x1080p_30Hz	*/
		{ 2200, 280, 1125, 45, 88, 44, 0, 4, 5, 0, 34, 34, 0, 0, },
		 /* v2880x480p_60Hz	*/
		{ 3432, 552, 525, 45, 64, 248, 1, 9, 6, 1, 35, 36, 0, 1, },
		 /* v2880x576p_50Hz	*/
		{ 3456, 576, 625, 49, 48, 256, 1, 5, 5, 1, 37, 38, 0, 1, },
		 /* v1920x1080i_50Hz(1250)	*/
		{ 2304, 384, 1250, 85, 32, 168, 0, 23, 5, 1, 39, 39, 1, 0, },
		 /* v1920x1080i_100Hz	*/
		{ 2640, 720, 1125, 22, 528, 44, 0, 2, 5, 0, 40, 40, 1, 0, },
		 /* v1280x720p_100Hz	*/
		{ 1980, 700, 750, 30, 440, 40, 0, 5, 5, 0, 41, 41, 0, 0, },
		 /* v720x576p_100Hz	*/
		{ 864, 144, 625, 49, 12, 64, 1, 5, 5, 1, 42, 43, 0, 0, },
		 /* v720x576i_100Hz	*/
		{ 1728, 288, 625, 24, 24, 126, 1, 2, 3, 1, 44, 45, 1, 1, },
		 /* v1920x1080i_120Hz	*/
		{ 2200, 280, 1125, 22, 88, 44, 0, 2, 5, 0, 46, 46, 1, 0, },
		 /* v1280x720p_120Hz	*/
		{ 1650, 370, 750, 30, 110, 40, 0, 5, 5, 0, 47, 47, 0, 0, },
		 /* v720x480p_120Hz	*/
		{ 858, 138, 525, 54, 16, 62, 1, 9, 6, 1, 48, 49, 0, 0, },
		 /* v720x480i_120Hz	*/
		{ 1716, 276, 525, 22, 38, 124, 1, 4, 3, 1, 50, 51, 1, 1, },
		 /* v720x576p_200Hz	*/
		{ 864, 144, 625, 49, 12, 64, 1, 5, 5, 1, 52, 53, 0, 0, },
		 /* v720x576i_200Hz	*/
		{ 1728, 288, 625, 24, 24, 126, 1, 2, 3, 1, 54, 55, 1, 1, },
		 /* v720x480p_240Hz	*/
		{ 858, 138, 525, 45, 16, 62, 1, 9, 6, 1, 56, 57, 0, 0, },
		 /* v720x480i_240Hz	*/
		{ 1716, 276, 525, 22, 38, 124, 1, 4, 3, 1, 58, 59, 1, 1, },
		 /* v1280x720p24Hz	*/
		{ 3300, 2020, 750, 30, 1760, 40, 0, 5, 5, 0, 60, 60, 0, 0, },
		 /* v1280x720p30Hz	*/
		{ 3300, 2020, 750, 30, 1760, 40, 0, 5, 5, 0, 62, 62, 0, 0, },

};

/**
 * N value of ACR packet.@n
 * 4096  is the N value for 32 KHz sampling frequency @n
 * 6272  is the N value for 44.1 KHz sampling frequency @n
 * 6144  is the N value for 48 KHz sampling frequency @n
 * 12544 is the N value for 88.2 KHz sampling frequency @n
 * 12288 is the N value for 96 KHz sampling frequency @n
 * 25088 is the N value for 176.4 KHz sampling frequency @n
 * 24576 is the N value for 192 KHz sampling frequency @n
 */
static const unsigned int ACR_N_params[] = { 4096, 6272, 6144, 12544, 12288,
		25088, 24576 };



static void __iomem *regs_sys;
static void __iomem *regs_core;
static void __iomem *regs_aes;

static void hdmi_avi_update_checksum(void);
static void hdmi_aui_update_checksum(void);

static void hdcp_reset(void);
static void hdcp_enable(unsigned char enable);
static void hdmi_avi_update_checksum(void);
static void hdmi_aui_update_checksum(void);
static int hdmi_set_color_space(enum ColorSpace);
static int hdmi_set_color_depth(enum ColorDepth);
static int hdmi_set_video_mode(struct HDMIVideoParameter *pVideo);
static int hdmi_set_pixel_limit(enum PixelLimit);
static int hdmi_set_pixel_aspect_ratio(enum PixelAspectRatio);
static int hdmi_set_colorimetry(enum HDMIColorimetry);

int hdmi_set_audio_mute(int mute);
int hdmi_set_audio_sample_freq(enum SamplingFreq);

static int hdmi_set_audio_packet_type(enum HDMIASPType);
static int hdmi_set_audio_number_of_channels(enum ChannelNum);
static void hdmi_enable_bluescreen(unsigned char enable);
static void hdmi_mode_select(enum HDMIMode mode);
static void hdmi_start(void);
static void hdmi_stop(void);

static void hdcp_set_ksv_list(struct hdcp_ksv_list list);
static void hdcp_set_result(unsigned char match);
static int hdcp_read_ri(void);

static void hdcp_enable_encryption(unsigned char enable);
static void hdcp_load_key(void);

static int hdmi_set_3d_mode(enum HDMI3DVideoStructure mode);
static void hdmi_set_default_value(void);
static int hdmi_set_2D_video(enum VideoFormat format);
static int hdmi_set_3D_FP_video(enum VideoFormat format);
static int hdmi_set_3D_SSH_video(enum VideoFormat format);
static int hdmi_set_3D_TB_video(enum VideoFormat format);
static int hdmi_set_3D_FA_video(enum VideoFormat format);
static int hdmi_set_3D_LA_video(enum VideoFormat format);
static int hdmi_set_3D_SSF_video(enum VideoFormat format);
static int hdmi_set_3D_LD_video(enum VideoFormat format);
static int hdmi_set_3D_LDGFX_video(enum VideoFormat format);
static int hdmi_set_oldIPversion(unsigned char enable);

static int hdmi_open(struct inode *inode, struct file *file)
{
	d4_hdmi_hdcp_interrupt_enable(1);
	return 0;
}

static int hdmi_release(struct inode *inode, struct file *file)
{
#if 1
	hdcp_enable(0);
#endif
	d4_hdmi_hdcp_interrupt_enable(0);
	return 0;
}

unsigned int hdmi_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &hdcp_struct.waitq, wait);
	if (hdcp_struct.event != 0)
		mask |= (POLLIN | POLLRDNORM);

	return mask;
}

static long hdmi_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	int ret = 0;
	int size;
	if (_IOC_TYPE(cmd) != HDMI_IOC_MAGIC) {
		return -1;
	}

	switch (cmd) {
	case HDMI_IOC_SET_COLORSPACE: {
		int space;
		if (get_user(space, (int __user *) arg))
			return -EFAULT;

		if (!hdmi_set_color_space(space)) {
			return -EFAULT;
		}
		break;
	}

	case HDMI_IOC_SET_COLORDEPTH: {
		int depth;
		if (get_user(depth, (int __user *) arg))
			return -EFAULT;

		if (!hdmi_set_color_depth(depth)) {
			return -EFAULT;
		}
		break;
	}
	case HDMI_IOC_SET_HDMIMODE: {
		int mode = 0;
		int reg2 = 0;
		mode = HDMI;
		size = _IOC_SIZE(cmd);
		ret = copy_from_user((void *) &mode, (const void *) arg, size);
		if (ret < 0) {
			return ret;
		}

		reg2 = readb(HDMI_CON_2);
		if (mode == HDMI) {
			writeb(HDMI_MODE_SEL_HDMI, HDMI_MODE_SEL);
			writeb(reg2 | ~HDMICON2_DVI, HDMI_CON_2);
		} else {
			writeb(HDMI_MODE_SEL_DVI, HDMI_MODE_SEL);
			writeb(reg2 | HDMICON2_DVI, HDMI_CON_2);
		}
	}
	break;
	case HDMI_IOC_SET_VIDEOMODE: {
		unsigned int ret;
		struct HDMIVideoParameter video_mode;
		ret = copy_from_user((void *) &video_mode, (const void *) arg,
				sizeof(struct HDMIVideoParameter));
		if (ret < 0)
			return -EFAULT;

		if (!hdmi_set_video_mode(&video_mode))
			return -EFAULT;

		break;
	}
	case HDMI_IOC_SET_BLUESCREEN: {
		unsigned char val;
		if (get_user(val, (unsigned char __user *) arg))
			return -EFAULT;

		if (val) {
			hdmi_enable_bluescreen(1);
		} else {
			hdmi_enable_bluescreen(0);
		}
		break;
	}
	case HDMI_IOC_SET_PIXEL_LIMIT: {
		int val;

		if (get_user(val, (int __user *) arg))
			return -EFAULT;

		if (!hdmi_set_pixel_limit(val)) {
			return -EFAULT;
		}
		break;
	}
	case HDMI_IOC_SET_PIXEL_ASPECT_RATIO: {
		int val;

		if (get_user(val, (int __user *) arg))
			return -EFAULT;

		if (!hdmi_set_pixel_aspect_ratio(val)) {
			return -EFAULT;
		}

		break;
	}
	case HDMI_IOC_SET_COLORIMETRY: {
		int val;

		if (get_user(val, (int __user *) arg))
			return -EFAULT;

		if (!hdmi_set_colorimetry(val)) {
			return -EFAULT;
		}

		break;
	}

	case HDMI_IOC_SET_AVMUTE: {
		unsigned char val, reg;

		if (get_user(val, (unsigned int __user *) arg))
			return -EFAULT;

		reg = readb(HDMI_MODE_SEL) & HDMI_MODE_SEL_HDMI;
		if (reg) {
			if (val) {
				/* default off */
				writeb(GCP_AVMUTE_OFF, HDMI_GCP_BYTE1);
				writeb(GCP_TRANSMIT_EVERY_VSYNC, HDMI_GCP_CON);
			} else {
				writeb(GCP_AVMUTE_OFF, HDMI_GCP_BYTE1);
				writeb(GCP_TRANSMIT_EVERY_VSYNC, HDMI_GCP_CON);
			}
		}

		break;
	}
	case HDMI_IOC_SET_AUDIOPACKETTYPE: {
		int val;

		if (get_user(val, (int __user *) arg))
			return -EFAULT;

		if (!hdmi_set_audio_packet_type(val)) {
			return -EFAULT;
		}

		break;
	}
	case HDMI_IOC_SET_AUDIOSAMPLEFREQ: {
		int val;
		unsigned char reg;

		if (get_user(val, (int __user *) arg))
			return -EFAULT;

		if (!hdmi_set_audio_sample_freq(val)) {
			return -EFAULT;
		}
		reg = readb(HDMI_CON_0);
		writeb(reg | HDMI_ASP_ENABLE, HDMI_CON_0);
		break;
	}
	case HDMI_IOC_SET_AUDIOCHANNEL: {
		int val;

		if (get_user(val, (int __user *) arg))
			return -EFAULT;

		if (!hdmi_set_audio_number_of_channels(val)) {
			return -EFAULT;
		}

		break;
	}
	case HDMI_IOC_SET_SPEAKER_ALLOCATION: {
		unsigned int val;

		if (get_user(val, (unsigned int __user *) arg))
			return -EFAULT;
		/* default value */
		writeb(/*val*/0x00, HDMI_AUI_BYTE4);
		break;
	}

	case HDMI_IOC_GET_PHYREADY: {
		unsigned char phy_status;
		phy_status = readb(HDMI_PHY_STATUS);
		put_user(phy_status, (unsigned char __user *)arg);
		break;
	}
	case HDMI_IOC_START_HDMI: {
		hdmi_start();
		break;
	}
	case HDMI_IOC_STOP_HDMI: {
		hdmi_stop();
		break;
	}

	case HDMI_IOC_GET_HDCP_EVENT: {
		spin_lock_irq(&hdcp_struct.lock);
		put_user(hdcp_struct.event, (unsigned int __user *)arg);
		hdcp_struct.event = 0;
		spin_unlock_irq(&hdcp_struct.lock);
		break;
	}

	case HDMI_IOC_STOP_HDCP: {
		spin_lock_irq(&hdcp_struct.lock);
		hdcp_struct.event |= (1 << HDCP_EVENT_STOP);
		spin_unlock_irq(&hdcp_struct.lock);
		wake_up_interruptible(&hdcp_struct.waitq);
		break;
	}
	case HDMI_IOC_ENABLE_HDCP: {
		unsigned char enable;
		if (get_user(enable, (unsigned char __user *) arg))
			return -EFAULT;

		if (enable)
			DPRINTK(" ON\n");
		else
			DPRINTK(" OFF\n");
		hdcp_enable(enable);
		break;
	}

	case HDMI_IOC_SET_BKSV: {
		struct hdcp_ksv Bksv;
		int ret;
		ret = copy_from_user((void *) &Bksv, (const void *) arg, sizeof(Bksv));
		if (ret < 0)
			return -EFAULT;

		writeb(Bksv.ksv[0], HDCP_BKSV_0);
		writeb(Bksv.ksv[1], HDCP_BKSV_1);
		writeb(Bksv.ksv[2], HDCP_BKSV_2);
		writeb(Bksv.ksv[3], HDCP_BKSV_3);
		writeb(Bksv.ksv[4], HDCP_BKSV_4);

		break;
	}
	case HDMI_IOC_SET_BCAPS: {
		unsigned char Bcaps;

		if (get_user(Bcaps, (unsigned char __user *) arg))
			return -EFAULT;
		writeb(Bcaps, HDCP_BCAPS);
		break;
	}

	case HDMI_IOC_GET_AKSV: {
		struct hdcp_ksv Aksv;
		int ret;
		Aksv.ksv[0] = readb(HDCP_AKSV_0);
		Aksv.ksv[1] = readb(HDCP_AKSV_1);
		Aksv.ksv[2] = readb(HDCP_AKSV_2);
		Aksv.ksv[3] = readb(HDCP_AKSV_3);
		Aksv.ksv[4] = readb(HDCP_AKSV_4);
		ret = copy_to_user((void *) arg, (const void *) &Aksv, sizeof(Aksv));
		if (ret < 0)
			return -EFAULT;

		break;
	}
	case HDMI_IOC_GET_AN: {
		struct hdcp_an An;
		int ret;

		An.an[0] = readb(HDCP_AN_0);
		An.an[1] = readb(HDCP_AN_1);
		An.an[2] = readb(HDCP_AN_2);
		An.an[3] = readb(HDCP_AN_3);
		An.an[4] = readb(HDCP_AN_4);
		An.an[5] = readb(HDCP_AN_5);
		An.an[6] = readb(HDCP_AN_6);
		An.an[7] = readb(HDCP_AN_7);
		ret = copy_to_user((void *) arg, (const void *) &An, sizeof(An));
		if (ret < 0)
			return -EFAULT;
		break;
	}

	case HDMI_IOC_GET_RI: /* only use if state is in first auth */
	{
		int ret;
		unsigned int result;
		unsigned char ri0, ri1;

		result = hdcp_read_ri();
		ri0 = readb(HDCP_RI_0);
		ri1 = readb(HDCP_RI_1);

		if (((result >> 8) & 0xFF) == ri0 && (result & 0xFF) == ri1) {
			ret = 1;
		} else {
			ret = 0;
		}
		if (put_user(ret, (int __user *) arg))
			return -EFAULT;
		break;
	}
	case HDMI_IOC_SET_HDCP_RESULT: {
		unsigned char match = 1;

		if (get_user(match, (unsigned char __user *) arg))
			return -EFAULT;

		hdcp_set_result(match);
		break;
	}

	case HDMI_IOC_SET_ENCRYPTION: {
		unsigned char tmp;
		unsigned int cnt = 0;

		if (get_user(tmp, (unsigned char __user *) arg))
			return -EFAULT;

		if (tmp) {
			loop: cnt++;
			if (cnt > 10)
				return -EFAULT;

			tmp = readb(HDMI_STATUS);
			if (!(tmp & (1 << HDCP_AUTHEN_ACK_NUM)))
				goto loop;
			hdcp_enable_encryption(1);
		} else {
			hdcp_enable_encryption(0);
		}
		break;
	}
	case HDMI_IOC_SET_BSTATUS: {
		struct hdcp_status Bstatus;
		int ret;
		ret = copy_from_user((void *) &Bstatus.status, (const void *) arg,
				sizeof(Bstatus));
		if (ret < 0)
			return -EFAULT;

		writeb(Bstatus.status[0], HDCP_BSTATUS_0);
		writeb(Bstatus.status[1], HDCP_BSTATUS_1);

		break;
	}
	case HDMI_IOC_SET_KSV_LIST: {
		struct hdcp_ksv_list list;
		int ret;
		ret = copy_from_user((void *) &list, (const void *) arg, sizeof(list));
		if (ret < 0)
			return -EFAULT;

		hdcp_set_ksv_list(list);
		break;
	}

	case HDMI_IOC_SET_SHA1: {
		struct hdcp_sha1 rx_sha1;
		int index, ret;
		ret = copy_from_user((void *) &rx_sha1, (const void *) arg,
				sizeof(rx_sha1));
		if (ret < 0)
			return -EFAULT;

		for (index = 0; index < HDCP_SHA1_SIZE; index++)
			writeb(rx_sha1.sha1[index], HDCP_SHA1_00 + 4 * index);

		break;
	}
	case HDMI_IOC_SET_AUDIO_ENABLE: {
		unsigned char enable;
		unsigned char reg;
		unsigned char mode;

		if (get_user(enable, (int __user *) arg))
			return -EFAULT;

		reg = readb(HDMI_CON_0);
		
		mode = readb(HDMI_MODE_SEL);
		if(!(mode & HDMI_MODE_SEL_HDMI)) // DVI MODE
			enable = 0;
		
		if (enable) {
			hdmi_aui_update_checksum();
			writeb(TRANSMIT_EVERY_VSYNC,HDMI_AUI_CON);
			writeb((1<<2), HDMI_ACR_CON);
			writeb(reg | HDMI_ASP_ENABLE, HDMI_CON_0);
		} else {
			writeb(0x00, HDMI_AUI_CON);
			writeb(DO_NOT_TRANSMIT, HDMI_ACR_CON);
			writeb(reg & ~HDMI_ASP_ENABLE, HDMI_CON_0);
		}
		break;
	}
	case HDMI_IOC_GET_SHA1_RESULT: {
		int result = 1;

		if (readb(HDCP_SHA_RESULT) & HDCP_SHA1_VALID_READY) {
			if (!(readb(HDCP_SHA_RESULT) & HDCP_SHA1_VALID))
				result = 0;
		} else
			result = 0;

		writeb(0x00, HDCP_SHA_RESULT);
		if (put_user(result, (int __user *) arg))
			return -EFAULT;
		break;
	}
	case HDMI_IOC_GET_KSV_LIST_READ_DONE: {
		unsigned char reg;
		reg = readb(HDCP_KSV_LIST_CON) & HDCP_KSV_LIST_READ_DONE;
		if (put_user(reg, (unsigned char __user *) arg))
			return -EFAULT;
		break;
	}
	case HDMI_IOC_SET_ILLEGAL_DEVICE: {
		writeb(HDCP_REVOCATION_SET, HDCP_CTRL2);
		writeb(0x00, HDCP_CTRL2);
		break;
	}
	case HDMI_IOC_SET_REPEATER_TIMEOUT: {
		unsigned char reg;
		reg = readb(HDCP_CTRL1);
		writeb(reg | HDCP_TIMEOUT, HDCP_CTRL1);
		writeb(reg, HDCP_CTRL1);
	}
	case HDMI_IOC_RESET_HDCP: /* reset hdcp state machine */
	{
		hdcp_reset();
		break;
	}

	case HDMI_IOC_SET_KSV_LIST_EMPTY: {
		writeb(HDCP_KSV_LIST_EMPTY, HDCP_KSV_LIST_CON);
		break;
	}
	case HDMI_IOC_RESET_AUISAMPLEFREQ: {
		unsigned char reg;
		reg = readb(HDMI_AUI_BYTE2) & ~HDMI_AUI_SF_MASK;
		writeb(/*reg*/0x00, HDMI_AUI_BYTE2);
		break;
	}
	case HDMI_IOC_SET_VIDEO_SOURCE: {
		int mode;

		if (get_user(mode, (int __user *) arg))
			return -EFAULT;

		if (mode == HDMI_SOURCE_EXTERNAL) {
			writeb(HDMI_EXTERNAL_VIDEO, HDMI_VIDEO_PATTERN_GEN);

		} else {
			writeb(HDMI_INTERNAL_VIDEO, HDMI_VIDEO_PATTERN_GEN);
			writeb(0x00, HDMI_VIDEO_PATTERN_GEN);
		}
		break;
	}
	case HDMI_IOC_DEBUG_HDMI_CORE: {
		/* hdmi_core_debug(); */
		break;
	}
	case HDMI_IOC_DEBUG_HDMI_CORE_VIDEO: {
		/* hdmi_core_video_debug(); */
		break;
	}
	case HDMI_IOC_SET_IP_VERSION: {
		unsigned char enable;

		if (get_user(enable, (unsigned char __user *) arg))
			return -EFAULT;

		hdmi_set_oldIPversion(enable);
		break;
	}

	case HDMI_IOC_SET_DEBUG: {
		struct hdmi_dbg dbg;
		int ret;
		ret = copy_from_user((void *) &dbg, (const void *) arg,
				sizeof(struct hdmi_dbg));
		if (ret < 0)
			return -EFAULT;

		writeb(dbg.value, regs_core + dbg.offset);
		break;
	}

	default:
		return -EINVAL;
	}

	return 0;
}


/**
 * Reset the HDCP H/W state machine.
 */
static void hdcp_reset(void)
{
	hdcp_enable_encryption(0);
	writeb(HPD_SW_ENABLE | HPD_OFF, HDMI_HPD);
	writeb(HPD_SW_ENABLE | HPD_ON, HDMI_HPD);
	writeb(HPD_SW_DISABLE, HDMI_HPD);
}

/**
 * @brief Enable/Disable HDCP H/W.
 * @param enable	[in] 1 to enable, 0 to disable.
 *
 * @return
 */
void hdcp_enable(unsigned char enable)
{
	if (enable) {
		hdcp_reset();
		hdcp_load_key();
		writeb(HDCP_ENABLE, HDCP_CTRL1);
	} else {
		hdcp_enable_encryption(0);
		writeb(0x00, HDCP_CTRL1);
		writeb(0x00, HDCP_CTRL2);
	}
}


/**
 * Set the result whether Ri and Ri' are match or not.
 * @param match	[in] 1 if Ri and Ri' are match; Otherwise, 0.
 */
void hdcp_set_result(unsigned char match)
{
	if (match) {
		writeb(HDCP_RI_MATCH, HDCP_CHECK_RESULT);
		writeb(0x00, HDCP_CHECK_RESULT);
	} else {
		writeb(HDCP_RI_NOT_MATCH, HDCP_CHECK_RESULT);
		writeb(0x00, HDCP_CHECK_RESULT);
	}
}


/**
 * Set one KSV in KSV list which read from Rx on 2nd authentication.
 * @param list	[in] One KSV in KSV list and flag.
 */
void hdcp_set_ksv_list(struct hdcp_ksv_list list)
{
	writeb(list.ksv[0], HDCP_KSV_LIST_0);
	writeb(list.ksv[1], HDCP_KSV_LIST_1);
	writeb(list.ksv[2], HDCP_KSV_LIST_2);
	writeb(list.ksv[3], HDCP_KSV_LIST_3);
	writeb(list.ksv[4], HDCP_KSV_LIST_4);

	if (list.end) {
		writeb(HDCP_KSV_LIST_END | HDCP_KSV_WRITE_DONE, HDCP_KSV_LIST_CON);
	} else {
		writeb(HDCP_KSV_WRITE_DONE, HDCP_KSV_LIST_CON);
	}
}

/**
 * Set HDCP Device Private Keys. @n
 * To activate HDCP H/W, user should set AES-encrypted HDCP Device Private Keys.@n
 * If user does not set this, HDCP H/W does not work.
 */
void hdcp_load_key(void)
{
	int index;
	for (index = 0; index < HDCP_KEY_SIZE; index++) {
		writeb(HDCP_Test_key[index], AES_DATA);
	}
	writeb(0x01, AES_START);
}




/**
 * Set checksum in Audio InfoFrame Packet. @n
 * Calculate a checksum and set it in packet.
 */
void hdmi_aui_update_checksum(void)
{
	unsigned char index, checksum;

	writeb(AUI_HEADER_BYTE0, HDMI_AUI_HEADER0);
	writeb(AUI_HEADER_BYTE1, HDMI_AUI_HEADER1);
	writeb(AUI_HEADER_BYTE2, HDMI_AUI_HEADER2);

	checksum = AUI_HEADER;
	for (index = 0; index < AUI_PACKET_BYTE_LENGTH; index++) {
		checksum += readb(HDMI_AUI_BYTE1 + 4 * index);
	}
	writeb(~checksum + 1, HDMI_AUI_CHECK_SUM);
}

/**
 * Set checksum in AVI InfoFrame Packet. @n
 * Calculate a checksum and set it in packet.
 */
void hdmi_avi_update_checksum(void)
{
	unsigned char index, checksum;

	checksum = AVI_HEADER;

	writeb(AVI_HEADER_BYTE0, HDMI_AVI_HEADER0);
	writeb(AVI_HEADER_BYTE1, HDMI_AVI_HEADER1);
	writeb(AVI_HEADER_BYTE2, HDMI_AVI_HEADER2);

	for (index = 0; index < AVI_PACKET_BYTE_LENGTH; index++) {
		checksum += readb(HDMI_AVI_BYTE1 + 4 * index);
	}
	writeb(~checksum+1, HDMI_AVI_CHECK_SUM);
}


/**
 * @brief Set color space in HDMI H/W.
 * @fn int d4_hdmi_set_color_space(enum ColorSpace space)
 * @param   space[in] Color space
 *
 * @return  If argument is invalid, return 0; Otherwise return 1.
 */
int d4_hdmi_set_color_space(enum ColorSpace space)
{
	return hdmi_set_color_space(space);
}

int hdmi_set_color_space(enum ColorSpace space)
{
	unsigned char reg, aviYY;
	int ret = 1;

	reg = readb(HDMI_CON_0);
	aviYY = readb(HDMI_AVI_BYTE1);
	writeb(aviYY & ~(AVI_CS_Y422 | AVI_CS_Y444), HDMI_AVI_BYTE1);

	if (space == HDMI_CS_YCBCR422) {
		writeb(reg | HDMI_YCBCR422_ENABLE, HDMI_CON_0);
		writeb(aviYY | AVI_CS_Y422, HDMI_AVI_BYTE1);
	} else {
		writeb(reg & ~HDMI_YCBCR422_ENABLE, HDMI_CON_0);
		if (space == HDMI_CS_YCBCR444) {
			writeb(aviYY | AVI_CS_Y444, HDMI_AVI_BYTE1);
		}
		/*  aviYY for RGB = 0, nothing to set */
		else if (space != HDMI_CS_RGB) {
			ret = 0;
		}
	}

	return ret;
}

/**
 * @brief Set color depth.
 * @fn int d4_hdmi_set_color_depth(enum ColorDepth depth)
 * @param   depth[in] Color depth of input video stream
 *
 * @return  If argument is invalid, return 0; Otherwise return 1.
 */
int d4_hdmi_set_color_depth(enum ColorDepth depth)
{
	return hdmi_set_color_depth(depth);
}

int hdmi_set_color_depth(enum ColorDepth depth)
{
	int ret = 1;

	switch (depth) {
	case HDMI_CD_36:
		writeb(GCP_CD_36BPP, HDMI_GCP_BYTE2);
		writeb(HDMI_DC_CTL_12, HDMI_DC_CONTROL);
		break;

	case HDMI_CD_30:
		writeb(GCP_CD_30BPP, HDMI_GCP_BYTE2);
		writeb(HDMI_DC_CTL_10, HDMI_DC_CONTROL);
		break;

	case HDMI_CD_24:
		writeb(GCP_CD_24BPP, HDMI_GCP_BYTE2);
		writeb(HDMI_DC_CTL_8, HDMI_DC_CONTROL);
		break;

	default:
		ret = 0;
		break;
	}
	return ret;
}

/**
 * @brief Set video timing parameters.
 * @fn int d4_hdmi_set_video_mode(struct HDMIVideoParameter *pVideo)
 * @param   *pVideo[in] Video timing parameters
 *
 * @return  If any error then return 0; Otherwise return 1
 */
int d4_hdmi_set_video_mode(struct HDMIVideoParameter *pVideo)
{
	return hdmi_set_video_mode(pVideo);
}


int hdmi_set_video_mode(struct HDMIVideoParameter *pVideo)
{
	unsigned char reg;
	hdmi_set_default_value();

	/* set video source */
	writeb(HDMI_INTERNAL_VIDEO, HDMI_VIDEO_PATTERN_GEN);
	writeb(0x00, HDMI_VIDEO_PATTERN_GEN);

	switch (pVideo->hdmi_3d_format) {
	case HDMI_2D_VIDEO_FORMAT: {
		if (!hdmi_set_2D_video(pVideo->resolution))
			return 0;
		break;
	}
	case HDMI_3D_FP_FORMAT: {
		if (!hdmi_set_3D_FP_video(pVideo->resolution))
			return 0;
		break;
	}
	case HDMI_3D_SSH_FORMAT: {
		if (!hdmi_set_3D_SSH_video(pVideo->resolution))
			return 0;
		break;
	}
	case HDMI_3D_TB_FORMAT: {
		if (!hdmi_set_3D_TB_video(pVideo->resolution))
			return 0;
		break;
	}
	case HDMI_3D_FA_FORMAT: {
		if (!hdmi_set_3D_FA_video(pVideo->resolution))
			return 0;
		break;
	}
	case HDMI_3D_LA_FORMAT: {
		if (!hdmi_set_3D_LA_video(pVideo->resolution))
			return 0;
		break;
	}
	case HDMI_3D_SSF_FORMAT: {
		if (!hdmi_set_3D_SSF_video(pVideo->resolution))
			return 0;
		break;
	}
	case HDMI_3D_LD_FORMAT: {
		if (!hdmi_set_3D_LD_video(pVideo->resolution))
			return 0;
		break;
	}
	case HDMI_3D_LDGFX_FORMAT: {
		if (!hdmi_set_3D_LDGFX_video(pVideo->resolution))
			return 0;
		break;
	}
	default:
		return 0;
	}
	if (!hdmi_set_3d_mode(pVideo->hdmi_3d_format))
		return 0;

	reg = readb(HDMI_CON_1);
	if (HDMIVideoParams[pVideo->resolution].repetition) {
		/* set pixel repetition */
		writeb(reg|HDMICON1_DOUBLE_PIXEL_REPETITION,HDMI_CON_1);
		/* set avi packet */
		writeb(AVI_PIXEL_REPETITION_DOUBLE,HDMI_AVI_BYTE5);
	} else {
		/* clear pixel repetition */
		writeb(reg & ~(1<<1|1<<0),HDMI_CON_1);
		/* set avi packet */
		writeb(0x00,HDMI_AVI_BYTE5);
	}

	if (pVideo->pixelAspectRatio == HDMI_PIXEL_RATIO_16_9)
		writeb(HDMIVideoParams[pVideo->resolution].AVI_VIC_16_9, HDMI_AVI_BYTE4);
	else
		writeb(HDMIVideoParams[pVideo->resolution].AVI_VIC, HDMI_AVI_BYTE4);

	return 1;
}

/**
 * @brief Set pixel limitation.
 * @fn int d4_hdmi_set_pixel_limit(enum PixelLimit limit)
 * @param  limit[in] Pixel limitation.
 *
 * @return  If argument is invalid, return 0; Otherwise return 1.
 */
int d4_hdmi_set_pixel_limit(enum PixelLimit limit)
{
	return hdmi_set_pixel_limit(limit);
}

int hdmi_set_pixel_limit(enum PixelLimit limit)
{
	int ret = 1;
	unsigned char reg, aviQQ;

	reg = readb(HDMI_CON_1);
	reg &= ~HDMICON1_LIMIT_MASK;

	aviQQ = readb(HDMI_AVI_BYTE3);
	aviQQ &= ~AVI_QUANTIZATION_MASK;

	switch(limit)
	{
		case HDMI_FULL_RANGE:
			aviQQ |= AVI_QUANTIZATION_FULL;
			break;
		case HDMI_RGB_LIMIT_RANGE:
			reg |= HDMICON1_RGB_LIMIT;
			//aviQQ |= AVI_QUANTIZATION_LIMITED;
			break;
		case HDMI_YCBCR_LIMIT_RANGE:
			reg |= HDMICON1_YCBCR_LIMIT;
			//aviQQ |= AVI_QUANTIZATION_LIMITED;
			break;
		default:
			ret = 0;
			break;
	}

	writeb(reg, HDMI_CON_1);
	writeb(aviQQ, HDMI_AVI_BYTE3);
	return ret;
}

/**
 * @brief Set pixel aspect ratio information in AVI InfoFrame
 * @fn int d4_hdmi_set_pixel_aspect_ratio(enum PixelAspectRatio ratio)
 *
 * @param   ratio[in] Pixel Aspect Ratio
 * @return  If argument is invalid, return 0; Otherwise return 1.
 */
int d4_hdmi_set_pixel_aspect_ratio(enum PixelAspectRatio ratio)
{
	return hdmi_set_pixel_aspect_ratio(ratio);
}


int hdmi_set_pixel_aspect_ratio(enum PixelAspectRatio ratio)
{
	int ret = 1;
	unsigned char reg = AVI_FORMAT_ASPECT_AS_PICTURE;

	switch (ratio) {
	case HDMI_PIXEL_RATIO_AS_PICTURE:
		break;
	case HDMI_PIXEL_RATIO_16_9:
		reg |= AVI_PICTURE_ASPECT_RATIO_16_9;
		break;
	case HDMI_PIXEL_RATIO_4_3:
		reg |= AVI_PICTURE_ASPECT_RATIO_4_3;
		break;
	default:
		ret = 0;
		break;
	}

	writeb(reg, HDMI_AVI_BYTE2);
	return ret;
}

/**
 * @brief Set calorimetry
 * @fn int d4_hdmi_set_colorimetry(enum HDMIColorimetry colorimetry)
 *
 * @param   colorimetry[in] Pixel Aspect Ratio
 * @return  If argument is invalid, return 0; Otherwise return 1.
 */
int d4_hdmi_set_colorimetry(enum HDMIColorimetry colorimetry)
{
	return hdmi_set_colorimetry(colorimetry);
}

int hdmi_set_colorimetry(enum HDMIColorimetry colorimetry)
{
	int ret = 1;
	unsigned char avi2, avi3;
	avi2 = readb(HDMI_AVI_BYTE2);
	avi3 = readb(HDMI_AVI_BYTE3);

	avi2 &= ~AVI_COLORIMETRY_MASK;
	avi3 &= ~AVI_COLORIMETRY_EXT_MASK;

	switch (colorimetry) {
	case HDMI_COLORIMETRY_NO_DATA:
		break;

	case HDMI_COLORIMETRY_ITU601:
		avi2 |= AVI_COLORIMETRY_ITU601;
		break;

	case HDMI_COLORIMETRY_ITU709:
		avi2 |= AVI_COLORIMETRY_ITU709;
		break;

	case HDMI_COLORIMETRY_EXTENDED_xvYCC601:
		avi2 |= AVI_COLORIMETRY_EXTENDED;
		avi3 |= AVI_COLORIMETRY_EXT_xvYCC601;
		break;

	case HDMI_COLORIMETRY_EXTENDED_xvYCC709:
		avi2 |= AVI_COLORIMETRY_EXTENDED;
		avi3 |= AVI_COLORIMETRY_EXT_xvYCC709;
		break;

	default:
		ret = 0;
		break;
	}

	writeb(avi2, HDMI_AVI_BYTE2);
	writeb(avi3, HDMI_AVI_BYTE3);
	return ret;
}




/**
 * @brief Set Audio Mute.
 * @fn 	int hdmi_set_audio_mute(int mute)
 *
 * @param   mute[in] enable/disable mute
 * @return  If argument is invalid, return 0; Otherwise return 1.
 */

int hdmi_set_audio_mute(int mute)
{
	unsigned char val=0;
	unsigned int offset=(unsigned int)HDMI_CON_0;
	
	val = readb(HDMI_CON_0);
	if (mute) {	/* audio mute */
		writeb(val&(~((unsigned char)1<<2)), HDMI_CON_0);
	} else {
		writeb(val|((unsigned char)1<<2), HDMI_CON_0);
	}
	return 1;
}


/**
 * @brief Set Audio Clock Recovery and Audio Infoframe packet -@n
 * 		based on sampling frequency.
 * @fn 	int hdmi_set_audio_sample_freq(enum SamplingFreq freq)
 *
 * @param   freq[in] Sampling frequency
 * @return  If argument is invalid, return 0; Otherwise return 1.
 */
int hdmi_set_audio_sample_freq(enum SamplingFreq freq)
{

	unsigned char reg;
	unsigned int n;
	int ret = 1;

	if (freq > sizeof(ACR_N_params) / sizeof(unsigned int) || freq < 0)
		return 0;

	n = ACR_N_params[freq];
	reg = n & 0xff;
	writeb(reg, HDMI_ACR_N0);

	reg = (n >> 8) & 0xff;
	writeb(reg, HDMI_ACR_N1);

	reg = (n >> 16) & 0xff;
	writeb(reg, HDMI_ACR_N2);

	/* set as measure cts mode	*/
	writeb((1<<2), HDMI_ACR_CON);

	/* set AUI packet	*/
	reg = readb(HDMI_AUI_BYTE2) & ~HDMI_AUI_SF_MASK;

	switch (freq) {
	case SF_32KHZ:
		reg |= HDMI_AUI_SF_SF_32KHZ;
		break;

	case SF_44KHZ:
		reg |= HDMI_AUI_SF_SF_44KHZ;
		break;

	case SF_48KHZ:
		reg |= HDMI_AUI_SF_SF_48KHZ;
		break;

	case SF_88KHZ:
		reg |= HDMI_AUI_SF_SF_88KHZ;
		break;

	case SF_96KHZ:
		reg |= HDMI_AUI_SF_SF_96KHZ;
		break;

	case SF_176KHZ:
		reg |= HDMI_AUI_SF_SF_176KHZ;
		break;

	case SF_192KHZ:
		reg |= HDMI_AUI_SF_SF_192KHZ;
		break;

	default:
		ret = 0;
		break;
	}

	//writeb(reg, HDMI_AUI_BYTE2);

	return ret;
}

/**
 * @brief Set HDMI audio output packet type.
 * @fn int hdmi_set_audio_packet_type(enum HDMIASPType packet)
 * @param   packet[in] Audio packet type
 *
 * @return  If argument is invalid, return 0; Otherwise return 1.
 */
int hdmi_set_audio_packet_type(enum HDMIASPType packet)
{
	int ret = 1;
	unsigned char reg;

	reg = readb(HDMI_ASP_CON);
	reg &= ~ASP_TYPE_MASK;

	switch (packet) {
	case HDMI_ASP:
		break;
	case HDMI_DSD:
		reg |= ASP_DSD_TYPE;
		break;
	case HDMI_HBR:
		reg |= ASP_HBR_TYPE;
		reg |= ASP_LAYOUT_1 | ASP_SP_0 | ASP_SP_1 | ASP_SP_2 | ASP_SP_3;
		break;
	case HDMI_DST:
		reg |= ASP_DST_TYPE;
		break;
	default:
		ret = 0;
		break;
	}

	writeb(reg,HDMI_ASP_CON);
	return ret;

}

/**
 * @brief Set layout and sample present fields in Audio Sample Packet -@n
 *			and channel number field in Audio InfoFrame packet.
 * @fn		int hdmi_set_audio_number_of_channels(enum ChannelNum channel)
 *
 * @param   channel[in]  Number of channels
 * @return  If argument is invalid, return 0; Otherwise return 1.
 */
int hdmi_set_audio_number_of_channels(enum ChannelNum channel)
{
	int ret = 1;
	unsigned char reg;

	reg = readb(HDMI_ASP_CON);
	reg &= ~(ASP_MODE_MASK | ASP_SP_MASK);

	writeb(0x00, HDMI_ASP_SP_FLAT);

	switch (channel) {
	case CH_2:
		reg |= (ASP_LAYOUT_0);
		writeb(AUI_CC_2CH, HDMI_AUI_BYTE1);
		break;
	case CH_3:
		reg |= (ASP_LAYOUT_1 | ASP_SP_0 | ASP_SP_1);
		writeb(AUI_CC_3CH, HDMI_AUI_BYTE1);
		break;
	case CH_4:
		reg |= (ASP_LAYOUT_1 | ASP_SP_0 | ASP_SP_1);
		writeb(AUI_CC_4CH, HDMI_AUI_BYTE1);
		break;
	case CH_5:
		reg |= (ASP_LAYOUT_1 | ASP_SP_0 | ASP_SP_1 | ASP_SP_2);
		writeb(AUI_CC_5CH, HDMI_AUI_BYTE1);
		break;
	case CH_6:
		reg |= (ASP_LAYOUT_1 | ASP_SP_0 | ASP_SP_1 | ASP_SP_2);
		writeb(/*AUI_CC_6CH*/0x00, HDMI_AUI_BYTE1);
		break;
	case CH_7:
		reg |= (ASP_LAYOUT_1 | ASP_SP_0 | ASP_SP_1 | ASP_SP_2 | ASP_SP_3);
		writeb(AUI_CC_7CH, HDMI_AUI_BYTE1);
		break;
	case CH_8:
		reg |= (ASP_LAYOUT_1 | ASP_SP_0 | ASP_SP_1 | ASP_SP_2 | ASP_SP_3);
		writeb(AUI_CC_8CH, HDMI_AUI_BYTE1);
		break;
	default:
		ret = 0;
		break;
	}
	writeb(reg, HDMI_ASP_CON);
	return ret;

}


/**
 * @brief Enable HDMI output.
 * @fn void d4_hdmi_start(void)
 *
 * @param	none
 * @return	none
 */
void d4_hdmi_mode_select(enum HDMIMode mode)
{
	hdmi_mode_select(mode);
}

void hdmi_mode_select(enum HDMIMode mode)
{
	if(mode == HDMI)
	{
		hdmi_avi_update_checksum();
		writeb(TRANSMIT_EVERY_VSYNC, HDMI_AVI_CON);
		writeb(DO_NOT_TRANSMIT, HDMI_AUI_CON);

		/* Set HDMI Mode */
		writeb(HDMI_MODE_SEL_HDMI, HDMI_MODE_SEL);

		/* Set video preamble and guard band for hdmi*/
		writeb(HDMICON2_HDMI, HDMI_CON_2);
	}
	else
	{
		writeb(DO_NOT_TRANSMIT, HDMI_AVI_CON);
		writeb(DO_NOT_TRANSMIT, HDMI_AUI_CON);
		
		/* Set HDMI Mode */
		writeb(HDMI_MODE_SEL_DVI, HDMI_MODE_SEL);

		/* Set video preamble and guard band for hdmi*/
		writeb(HDMICON2_DVI, HDMI_CON_2);
	}
}

void d4_hdmi_start(void)
{
	hdmi_start();
}

void hdmi_start(void)
{
	unsigned char reg;

	reg = readb(HDMI_CON_0);
	writeb(reg | HDMI_SYS_ENABLE, HDMI_CON_0);
}

/**
 * @brief Enable HDMI output.
 * @fn void d4_hdmi_stop(void)
 *
 * @param	none
 * @return	none
 */

void d4_hdmi_stop(void)
{
	hdmi_stop();
}


void hdmi_stop(void)
{
	unsigned char reg;

	/* stop HDMI */
	reg = readb(HDMI_CON_0);
	writeb(reg & ~HDMI_SYS_ENABLE, HDMI_CON_0);
}

/**
 * @brief Read Ri' in  HDCP Rx port. The length of Ri' is 2 bytes.
 *			Stores LSB first.
 *			[0 : 0 : Ri'[1] : Ri'[0]]
 * @fn	int hdcp_read_ri(void)
 * @param	none
 *
 * @return	Ri' value.
 */
int hdcp_read_ri(void)
{
	int result;
	struct i2c_adapter *adap;
	struct i2c_msg msgs[2];
	unsigned char buffer[2] = {0, };
	unsigned char offset = HDCP_RI_OFFSET;

	adap = i2c_get_adapter(3);
	if (!adap) {

		return 0;
	}

	msgs[0].addr = HDCP_RX_DEV_ADDR >> 1;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &offset;

	msgs[1].addr = HDCP_RX_DEV_ADDR >> 1;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = 2;
	msgs[1].buf = buffer;

	if (i2c_transfer(adap, msgs, 2) < 0) {
		return 0;
	}
	result = (buffer[0] << 8) | buffer[1];
	return result;
}


void hdcp_enable_encryption(unsigned char enable)
{
	if (enable) {
		writeb(HDCP_EESS_START, HDCP_ENC_EN);
		hdmi_enable_bluescreen(0);
	} else {
		writeb(HDCP_EESS_STOP, HDCP_ENC_EN);
		hdmi_enable_bluescreen(1);
	}
}

/**
 * @brief nable blue screen
 * @fn void hdmi_enable_bluescreen(unsigned char enable)
 * @param enable[in] to turn on/off
 *
 * @return	none
 *
 */
void hdmi_enable_bluescreen(unsigned char enable)
{
	unsigned char reg = readb(HDMI_CON_0);
	if (enable) {
		reg |= HDMI_BLUE_SCR_ENABLE;
	} else {
		reg &= ~HDMI_BLUE_SCR_ENABLE;
	}
	/* writeb(reg,HDMI_CON_0); */
}

int hdmi_set_3d_mode(enum HDMI3DVideoStructure mode)
{
	if (mode == HDMI_2D_VIDEO_FORMAT){
		writeb(DO_NOT_TRANSMIT, HDMI_VSI_CON);
		return 1;
	} else {
		writeb(0x81, HDMI_VSI_HEADER0);
		writeb(0x01, HDMI_VSI_HEADER1);
		writeb(0x05, HDMI_VSI_HEADER2);

		writeb(0x03, HDMI_VSI_DATA01);
		writeb(0x0C, HDMI_VSI_DATA02);
		writeb(0x00, HDMI_VSI_DATA03);
		writeb(0x40, HDMI_VSI_DATA04);

		switch (mode) {
		case HDMI_3D_FP_FORMAT: {
			writeb(0x2a, HDMI_VSI_DATA00);
			writeb(0x00, HDMI_VSI_DATA05);
			break;
		}
		case HDMI_3D_SSH_FORMAT: {
			writeb(0x06, HDMI_VSI_HEADER2);
			writeb(0x99, HDMI_VSI_DATA00);
			writeb(0x80, HDMI_VSI_DATA05);
			writeb(0x10, HDMI_VSI_DATA06);

			break;
		}
		case HDMI_3D_TB_FORMAT: {
			writeb(0xCA, HDMI_VSI_DATA00);
			writeb(0x60, HDMI_VSI_DATA05);
			break;
		}
		case HDMI_3D_FA_FORMAT: {
			writeb(0x1A, HDMI_VSI_DATA00);
			writeb(0x10, HDMI_VSI_DATA05);
			break;
		}
		case HDMI_3D_LA_FORMAT: {
			writeb(0x0A, HDMI_VSI_DATA00);
			writeb(0x20, HDMI_VSI_DATA05);
			break;
		}
		case HDMI_3D_SSF_FORMAT: {
			writeb(0x09, HDMI_VSI_DATA00);
			writeb(0x03, HDMI_VSI_DATA05);
			break;
		}
		case HDMI_3D_LD_FORMAT: {
			writeb(0xEA, HDMI_VSI_DATA00);
			writeb(0x40, HDMI_VSI_DATA05);
			break;
		}
		case HDMI_3D_LDGFX_FORMAT: {
			writeb(0xE9, HDMI_VSI_DATA00);
			writeb(0x05, HDMI_VSI_DATA05);
			break;
		}
		default:
			return 0;
		}

		writeb(TRANSMIT_EVERY_VSYNC, HDMI_VSI_CON);
	}
	return 1;
}

void hdmi_set_default_value(void)
{
	unsigned int *base_addr = HDMI_V_BLANK_F0_0;
	unsigned int *final_addr = HDMI_VACT_SPACE6_1;

	writeb(0xff, HDMI_H_BLANK_0);
	writeb(0xff, HDMI_H_BLANK_1);
	writeb(0xff, HDMI_V1_BLANK_0);
	writeb(0xff, HDMI_V1_BLANK_1);
	writeb(0xff, HDMI_V2_BLANK_0);
	writeb(0xff, HDMI_V2_BLANK_1);
	writeb(0xff, HDMI_H_LINE_0);
	writeb(0xff, HDMI_H_LINE_1);
	writeb(0xff, HDMI_V_LINE_0);
	writeb(0xff, HDMI_V_LINE_1);

	for (; base_addr <= final_addr; base_addr++)
		writeb(0xFF, base_addr);
}

int hdmi_set_2D_video(enum VideoFormat format)
{
	unsigned int temp;
	writeb(HDMIVideoParams[format].HBlank & 0xFF, HDMI_H_BLANK_0);
	writeb((HDMIVideoParams[format].HBlank >> 8) & 0xFF, HDMI_H_BLANK_1);

	writeb(HDMIVideoParams[format].VBlank & 0xFF, HDMI_V1_BLANK_0);
	writeb((HDMIVideoParams[format].VBlank >> 8) & 0xFF, HDMI_V1_BLANK_1);

	/* HTotal	*/
	writeb(HDMIVideoParams[format].HTotal & 0xFF, HDMI_H_LINE_0);
	writeb((HDMIVideoParams[format].HTotal >> 8) & 0xFF, HDMI_H_LINE_1);

	writeb(HDMIVideoParams[format].VTotal & 0xFF, HDMI_V_LINE_0);
	writeb((HDMIVideoParams[format].VTotal >> 8) & 0xFF, HDMI_V_LINE_1);

	writeb(HDMIVideoParams[format].HPol, HDMI_HSYNC_POL);

	writeb(HDMIVideoParams[format].VPol, HDMI_VSYNC_POL);

	writeb((HDMIVideoParams[format].HFront - 2) & 0xFF, HDMI_H_SYNC_START_0);
	writeb(((HDMIVideoParams[format].HFront - 2) >> 8) & 0xFF, HDMI_H_SYNC_START_1);

	writeb(((HDMIVideoParams[format].HFront - 2) + HDMIVideoParams[format].HSync) & 0xFF, HDMI_H_SYNC_END_0);
	writeb(((HDMIVideoParams[format].HFront - 2) + HDMIVideoParams[format].HSync) >> 8 & 0xFF, HDMI_H_SYNC_END_1);

	writeb(HDMIVideoParams[format].VFront & 0xFF, HDMI_V_SYNC_LINE_BEF_1_0);
	writeb((HDMIVideoParams[format].VFront >> 8) & 0xFF, HDMI_V_SYNC_LINE_BEF_1_1);

	writeb((HDMIVideoParams[format].VFront + HDMIVideoParams[format].VSync) & 0xFF, HDMI_V_SYNC_LINE_BEF_2_0);
	writeb(((HDMIVideoParams[format].VFront + HDMIVideoParams[format].VSync) >> 8) & 0xFF, HDMI_V_SYNC_LINE_BEF_2_1);

	if (HDMIVideoParams[format].interlaced) {
		writeb(0x1, HDMI_INT_PRO_MODE);

		if (format == v1920x1080i_50Hz_1250) {
			temp = HDMIVideoParams[format].VTotal / 2;
			writeb(temp & 0xFF, HDMI_V2_BLANK_0);
			writeb((temp >> 8) & 0xFF, HDMI_V2_BLANK_1);
			writeb((temp + HDMIVideoParams[format].VBlank) & 0xFF, HDMI_V_BLANK_F0_0);
			writeb(((temp + HDMIVideoParams[format].VBlank) >> 8) & 0xFF,
					HDMI_V_BLANK_F0_1);
		} else {
			temp = (HDMIVideoParams[format].VTotal - HDMIVideoParams[format].VBlank * 2
					- 1) / 2 + HDMIVideoParams[format].VBlank;
			writeb(temp & 0xFF, HDMI_V2_BLANK_0);
			writeb((temp >> 8) & 0xFF, HDMI_V2_BLANK_1);

			writeb(
					((HDMIVideoParams[format].VTotal
							+ HDMIVideoParams[format].VBlank * 2 + 1) / 2) & 0xFF,
							HDMI_V_BLANK_F0_0);
			writeb(
					(((HDMIVideoParams[format].VTotal
							+ HDMIVideoParams[format].VBlank * 2 + 1) / 2) >> 8) & 0xFF,
							HDMI_V_BLANK_F0_1);
		}

		temp = temp + HDMIVideoParams[format].VFront;
		writeb(temp & 0xFF, HDMI_V_SYNC_LINE_AFT_1_0);
		writeb((temp >> 8 & 0xFF), HDMI_V_SYNC_LINE_AFT_1_1);

		temp = temp + HDMIVideoParams[format].VSync;
		writeb(temp & 0xFF, HDMI_V_SYNC_LINE_AFT_2_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_SYNC_LINE_AFT_2_1);

		writeb(HDMIVideoParams[format].VTotal & 0xFF, HDMI_V_BLANK_F1_0);
		writeb((HDMIVideoParams[format].VTotal >> 8) & 0xFF, HDMI_V_BLANK_F1_1);

		temp = HDMIVideoParams[format].HTotal / 2 + HDMIVideoParams[format].HFront;
		writeb(temp & 0xFF, HDMI_V_SYNC_LINE_AFT_PXL_1_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_SYNC_LINE_AFT_PXL_1_1);
		writeb(temp & 0xFF, HDMI_V_SYNC_LINE_AFT_PXL_2_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_SYNC_LINE_AFT_PXL_2_1);
	} else {
		writeb(0x0, HDMI_INT_PRO_MODE);

		writeb(HDMIVideoParams[format].VTotal & 0xFF, HDMI_V2_BLANK_0);
		writeb((HDMIVideoParams[format].VTotal >> 8) & 0xFF, HDMI_V2_BLANK_1);
	}
	return 1;
}

int hdmi_set_3D_FP_video(enum VideoFormat format)
{
	unsigned int temp;

	writeb(HDMIVideoParams[format].HBlank & 0xFF, HDMI_H_BLANK_0);
	writeb((HDMIVideoParams[format].HBlank >> 8) & 0xFF, HDMI_H_BLANK_1);

	writeb(HDMIVideoParams[format].VBlank & 0xFF, HDMI_V1_BLANK_0);
	writeb((HDMIVideoParams[format].VBlank >> 8) & 0xFF, HDMI_V1_BLANK_1);

	writeb(HDMIVideoParams[format].HTotal & 0xFF, HDMI_H_LINE_0);
	writeb((HDMIVideoParams[format].HTotal >> 8) & 0xFF, HDMI_H_LINE_1);

	temp = HDMIVideoParams[format].VTotal * 2;

	writeb(temp & 0xFF, HDMI_V_LINE_0);
	writeb((temp >> 8) & 0xFF, HDMI_V_LINE_1);

	writeb(temp & 0xFF, HDMI_V2_BLANK_0);
	writeb((temp >> 8) & 0xFF, HDMI_V2_BLANK_1);

	writeb(HDMIVideoParams[format].HPol, HDMI_HSYNC_POL);

	writeb(HDMIVideoParams[format].VPol, HDMI_VSYNC_POL);

	writeb((HDMIVideoParams[format].HFront - 2) & 0xFF, HDMI_H_SYNC_START_0);
	writeb(((HDMIVideoParams[format].HFront - 2) >> 8) & 0xFF, HDMI_H_SYNC_START_1);

	writeb(
			((HDMIVideoParams[format].HFront - 2) + HDMIVideoParams[format].HSync)
			& 0xFF, HDMI_H_SYNC_END_0);
	writeb(
			(((HDMIVideoParams[format].HFront - 2) + HDMIVideoParams[format].HSync) >> 8)
			& 0xFF, HDMI_H_SYNC_END_1);

	writeb(HDMIVideoParams[format].VFront & 0xFF, HDMI_V_SYNC_LINE_BEF_1_0);
	writeb((HDMIVideoParams[format].VFront >> 8) & 0xFF, HDMI_V_SYNC_LINE_BEF_1_1);

	writeb((HDMIVideoParams[format].VFront + HDMIVideoParams[format].VSync) & 0xFF,
			HDMI_V_SYNC_LINE_BEF_2_0);
	writeb(
			((HDMIVideoParams[format].VFront + HDMIVideoParams[format].VSync) >> 8)
			& 0xFF, HDMI_V_SYNC_LINE_BEF_2_1);

	if (HDMIVideoParams[format].interlaced) {
		writeb(0x1, HDMI_INT_PRO_MODE);

		if (format == v1920x1080i_50Hz_1250) {
			temp = HDMIVideoParams[format].VTotal / 2;
			writeb((temp)&0xFF, HDMI_VACT_SPACE1_0);
			writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE1_1);

			temp += HDMIVideoParams[format].VBlank;
			writeb(temp&0xFF, HDMI_VACT_SPACE2_0);
			writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE2_1);

			temp = (HDMIVideoParams[format].VTotal * 3) / 2;
			writeb((temp)&0xFF, HDMI_VACT_SPACE5_0);
			writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE5_1);

			temp = (HDMIVideoParams[format].VTotal * 3
					+ HDMIVideoParams[format].VBlank * 2) / 2;
			writeb((temp)&0xFF, HDMI_VACT_SPACE6_0);
			writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE6_1);

		} else {
			temp = (HDMIVideoParams[format].VTotal - HDMIVideoParams[format].VBlank * 2
					- 1) / 2 + HDMIVideoParams[format].VBlank;

			writeb((temp)&0xFF, HDMI_VACT_SPACE1_0);
			writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE1_1);

			temp = (HDMIVideoParams[format].VTotal + HDMIVideoParams[format].VBlank * 2
					+ 1) / 2;
			writeb((temp)&0xFF, HDMI_VACT_SPACE2_0);
			writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE2_1);

			temp = (HDMIVideoParams[format].VTotal * 3 - 1) / 2;
			writeb((temp)&0xFF, HDMI_VACT_SPACE5_0);
			writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE5_1);

			temp = (HDMIVideoParams[format].VTotal * 3
					+ HDMIVideoParams[format].VBlank * 2 + 1) / 2;
			writeb((temp)&0xFF, HDMI_VACT_SPACE6_0);
			writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE6_1);
		}

		temp = HDMIVideoParams[format].VTotal;
		writeb((temp)&0xFF, HDMI_VACT_SPACE3_0);
		writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE3_1);

		temp = HDMIVideoParams[format].VTotal + HDMIVideoParams[format].VBlank;
		writeb((temp)&0xFF, HDMI_VACT_SPACE4_0);
		writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE4_1);
	} else {
		writeb(0x0, HDMI_INT_PRO_MODE);

		temp = HDMIVideoParams[format].VTotal;
		writeb(temp&0xFF, HDMI_VACT_SPACE1_0);
		writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE1_1);

		temp += HDMIVideoParams[format].VBlank;
		writeb(temp&0xFF, HDMI_VACT_SPACE2_0);
		writeb((temp >> 8)&0xFF, HDMI_VACT_SPACE2_1);
	}

	return 1;
}

int hdmi_set_3D_SSH_video(enum VideoFormat format)
{
	return hdmi_set_2D_video(format);
}

int hdmi_set_3D_TB_video(enum VideoFormat format)
{
	return hdmi_set_2D_video(format);
}

int hdmi_set_3D_FA_video(enum VideoFormat format)
{
	unsigned int temp;

	if (HDMIVideoParams[format].interlaced) {
		writeb(0x1, HDMI_INT_PRO_MODE);

		writeb(HDMIVideoParams[format].HBlank & 0xFF, HDMI_H_BLANK_0);
		writeb((HDMIVideoParams[format].HBlank >> 8) & 0xFF, HDMI_H_BLANK_1);

		writeb(HDMIVideoParams[format].VBlank & 0xFF, HDMI_V1_BLANK_0);
		writeb((HDMIVideoParams[format].VBlank >> 8) & 0xFF, HDMI_V1_BLANK_1);

		writeb(HDMIVideoParams[format].HTotal & 0xFF, HDMI_H_LINE_0);
		writeb((HDMIVideoParams[format].HTotal >> 8) & 0xFF, HDMI_H_LINE_1);

		temp = HDMIVideoParams[format].VTotal * 2;

		writeb(HDMIVideoParams[format].VTotal & 0xFF, HDMI_V_LINE_0);
		writeb((HDMIVideoParams[format].VTotal >> 8) & 0xFF, HDMI_V_LINE_1);

		writeb(HDMIVideoParams[format].HPol, HDMI_HSYNC_POL);

		writeb(HDMIVideoParams[format].VPol, HDMI_VSYNC_POL);

		writeb((HDMIVideoParams[format].HFront - 2) & 0xFF, HDMI_H_SYNC_START_0);
		writeb(((HDMIVideoParams[format].HFront - 2) >> 8) & 0xFF, HDMI_H_SYNC_START_1);

		writeb(
				((HDMIVideoParams[format].HFront - 2) + HDMIVideoParams[format].HSync)
				& 0xFF, HDMI_H_SYNC_END_0);
		writeb(
				(((HDMIVideoParams[format].HFront - 2) + HDMIVideoParams[format].HSync)
						>> 8) & 0xFF, HDMI_H_SYNC_END_1);

		writeb(HDMIVideoParams[format].VFront & 0xFF, HDMI_V_SYNC_LINE_BEF_1_0);
		writeb((HDMIVideoParams[format].VFront >> 8) & 0xFF, HDMI_V_SYNC_LINE_BEF_1_1);

		writeb((HDMIVideoParams[format].VFront + HDMIVideoParams[format].VSync) & 0xFF,
				HDMI_V_SYNC_LINE_BEF_2_0);
		writeb(
				((HDMIVideoParams[format].VFront + HDMIVideoParams[format].VSync) >> 8)
				& 0xFF, HDMI_V_SYNC_LINE_BEF_2_1);

		if (format == v1920x1080i_50Hz_1250) {
			temp = HDMIVideoParams[format].VTotal / 2;
			writeb((temp) & 0xFF, HDMI_V2_BLANK_0);
			writeb((temp >> 8) & 0xFF, HDMI_V2_BLANK_1);

			temp += HDMIVideoParams[format].VBlank;
			writeb(temp & 0xFF, HDMI_V_BLANK_F0_0);
			writeb((temp >> 8) & 0xFF, HDMI_V_BLANK_F0_1);

			temp = HDMIVideoParams[format].VTotal - HDMIVideoParams[format].VBlank / 2;
			writeb((temp) & 0xFF, HDMI_V_BLANK_F3_0);
			writeb((temp >> 8) & 0xFF, HDMI_V_BLANK_F3_1);

			temp = (HDMIVideoParams[format].VTotal * 3
					+ HDMIVideoParams[format].VBlank * 2) / 2;
			writeb((temp) & 0xFF, HDMI_V_BLANK_F4_0);
			writeb((temp >> 8) & 0xFF, HDMI_V_BLANK_F4_1);
		} else {
			temp = (HDMIVideoParams[format].VTotal - HDMIVideoParams[format].VBlank * 2
					- 1) / 2 + HDMIVideoParams[format].VBlank;

			writeb((temp) & 0xFF, HDMI_V2_BLANK_0);
			writeb((temp >> 8) & 0xFF, HDMI_V2_BLANK_1);

			temp = (HDMIVideoParams[format].VTotal + HDMIVideoParams[format].VBlank * 2
					+ 1) / 2;
			writeb((temp) & 0xFF, HDMI_V_BLANK_F0_0);
			writeb((temp >> 8) & 0xFF, HDMI_V_BLANK_F0_1);

			temp = (HDMIVideoParams[format].VTotal
					- (HDMIVideoParams[format].VBlank + 1)) / 2;
			writeb((temp) & 0xFF, HDMI_V_BLANK_F3_0);
			writeb((temp >> 8) & 0xFF, HDMI_V_BLANK_F3_1);

			temp = (HDMIVideoParams[format].VTotal * 3
					+ HDMIVideoParams[format].VBlank * 2 + 1) / 2;
			writeb((temp) & 0xFF, HDMI_V_BLANK_F4_0);
			writeb((temp >> 8) & 0xFF, HDMI_V_BLANK_F4_1);
		}

		temp = HDMIVideoParams[format].VTotal;
		writeb((temp) & 0xFF, HDMI_V_BLANK_F1_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_BLANK_F1_1);

		temp += HDMIVideoParams[format].VFront;
		writeb((temp) & 0xFF, HDMI_V_SYNC_LINE_AFT_1_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_SYNC_LINE_AFT_1_1);

		temp += HDMIVideoParams[format].VSync;
		writeb((temp) & 0xFF, HDMI_V_SYNC_LINE_AFT_2_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_SYNC_LINE_AFT_2_1);

		temp = HDMIVideoParams[format].HTotal / 2 + HDMIVideoParams[format].HFront;
		writeb(temp & 0xFF, HDMI_V_SYNC_LINE_AFT_PXL_1_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_SYNC_LINE_AFT_PXL_1_1);
		writeb(temp & 0xFF, HDMI_V_SYNC_LINE_AFT_PXL_1_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_SYNC_LINE_AFT_PXL_1_1);

		temp = HDMIVideoParams[format].VTotal + HDMIVideoParams[format].VBlank;
		writeb((temp) & 0xFF, HDMI_V_BLANK_F2_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_BLANK_F2_1);

		temp = HDMIVideoParams[format].VTotal * 2;
		writeb((temp) & 0xFF, HDMI_V_BLANK_F5_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_BLANK_F5_1);
	} else {
		return 0;
	}

	return 1;
}

int hdmi_set_3D_LA_video(enum VideoFormat format)
{
	if (HDMIVideoParams[format].interlaced) {
		return 0;
	} else {
		unsigned int temp;

		temp = HDMIVideoParams[format].HBlank;
		writeb(temp & 0xFF, HDMI_H_BLANK_0);
		writeb((temp >> 8) & 0xFF, HDMI_H_BLANK_1);

		temp = (unsigned int) HDMIVideoParams[format].VBlank * 2;
		writeb(temp & 0xFF, HDMI_V1_BLANK_0);
		writeb((temp >> 8) & 0xFF, HDMI_V1_BLANK_1);

		temp = HDMIVideoParams[format].VTotal * 2;
		writeb(temp & 0xFF, HDMI_V2_BLANK_0);
		writeb((temp >> 8) & 0xFF, HDMI_V2_BLANK_1);

		writeb(temp & 0xFF, HDMI_V_LINE_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_LINE_1);

		writeb(HDMIVideoParams[format].HTotal & 0xFF, HDMI_H_LINE_0);
		writeb((HDMIVideoParams[format].HTotal >> 8) & 0xFF, HDMI_H_LINE_1);

		writeb(HDMIVideoParams[format].HPol, HDMI_HSYNC_POL);

		writeb(HDMIVideoParams[format].VPol, HDMI_VSYNC_POL);

		writeb((HDMIVideoParams[format].HFront - 2) & 0xFF, HDMI_H_SYNC_START_0);
		writeb(((HDMIVideoParams[format].HFront - 2) >> 8) & 0xFF, HDMI_H_SYNC_START_1);

		writeb(
				((HDMIVideoParams[format].HFront - 2) + HDMIVideoParams[format].HSync)
				& 0xFF, HDMI_H_SYNC_END_0);
		writeb(
				(((HDMIVideoParams[format].HFront - 2) + HDMIVideoParams[format].HSync)
						>> 8) & 0xFF, HDMI_H_SYNC_END_1);

		temp = HDMIVideoParams[format].VFront * 2;
		writeb(temp & 0xFF, HDMI_V_SYNC_LINE_BEF_1_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_SYNC_LINE_BEF_1_1);

		temp += HDMIVideoParams[format].VSync * 2;
		writeb(temp & 0xFF, HDMI_V_SYNC_LINE_BEF_2_0);
		writeb((temp >> 8) & 0xFF, HDMI_V_SYNC_LINE_BEF_2_1);
	}

	return 1;
}

int hdmi_set_3D_SSF_video(enum VideoFormat format)
{
	unsigned int temp;
	hdmi_set_2D_video(format);

	temp = HDMIVideoParams[format].HBlank * 2;
	writeb(temp & 0xFF, HDMI_H_BLANK_0);
	writeb((temp >> 8) & 0xFF, HDMI_H_BLANK_1);

	temp = HDMIVideoParams[format].HTotal * 2;
	writeb(temp & 0xFF, HDMI_H_LINE_0);
	writeb((temp >> 8) & 0xFF, HDMI_H_LINE_1);

	temp = HDMIVideoParams[format].HFront * 2 - 2;
	writeb(temp & 0xFF, HDMI_H_SYNC_START_0);
	writeb((temp >> 8) & 0xFF, HDMI_H_SYNC_START_1);

	temp += HDMIVideoParams[format].HSync * 2;
	writeb(temp & 0xFF, HDMI_H_SYNC_END_0);
	writeb((temp >> 8) & 0xFF, HDMI_H_SYNC_END_1);

	return 1;
}

int hdmi_set_3D_LD_video(enum VideoFormat format)
{
	if (HDMIVideoParams[format].interlaced) {

		return 0;
	} else {
		return hdmi_set_3D_FP_video(format);
	}
}

int hdmi_set_3D_LDGFX_video(enum VideoFormat format)
{
	unsigned int temp;

	if (HDMIVideoParams[format].interlaced) {
		return 0;
	} else {
		writeb(HDMIVideoParams[format].HBlank & 0xFF, HDMI_H_BLANK_0);
		writeb((HDMIVideoParams[format].HBlank >> 8) & 0xFF, HDMI_H_BLANK_1);

		writeb(HDMIVideoParams[format].VBlank & 0xFF, HDMI_V1_BLANK_0);
		writeb((HDMIVideoParams[format].VBlank >> 8) & 0xFF, HDMI_V1_BLANK_1);

		writeb(HDMIVideoParams[format].HTotal & 0xFF, HDMI_H_LINE_0);
		writeb((HDMIVideoParams[format].HTotal >> 8) & 0xFF, HDMI_H_LINE_1);

		temp = HDMIVideoParams[format].VTotal * 4;

		writeb(HDMIVideoParams[format].VTotal & 0xFF, HDMI_V_LINE_0);
		writeb((HDMIVideoParams[format].VTotal >> 8) & 0xFF, HDMI_V_LINE_1);

		writeb(temp & 0xFF, HDMI_V2_BLANK_0);
		writeb((temp >> 8) & 0xFF, HDMI_V2_BLANK_1);

		writeb(HDMIVideoParams[format].HPol, HDMI_HSYNC_POL);

		writeb(HDMIVideoParams[format].VPol, HDMI_VSYNC_POL);

		writeb((HDMIVideoParams[format].HFront - 2) & 0xFF, HDMI_H_SYNC_START_0);
		writeb(((HDMIVideoParams[format].HFront - 2) >> 8) & 0xFF, HDMI_H_SYNC_START_1);

		writeb(
				((HDMIVideoParams[format].HFront - 2) + HDMIVideoParams[format].HSync)
				& 0xFF, HDMI_H_SYNC_END_0);
		writeb(
				(((HDMIVideoParams[format].HFront - 2) + HDMIVideoParams[format].HSync)
						>> 8) & 0xFF, HDMI_H_SYNC_END_1);

		writeb(HDMIVideoParams[format].VFront & 0xFF, HDMI_V_SYNC_LINE_BEF_1_0);
		writeb((HDMIVideoParams[format].VFront >> 8) & 0xFF, HDMI_V_SYNC_LINE_BEF_1_1);

		writeb((HDMIVideoParams[format].VFront + HDMIVideoParams[format].VSync) & 0xFF,
				HDMI_V_SYNC_LINE_BEF_2_0);
		writeb(
				((HDMIVideoParams[format].VFront + HDMIVideoParams[format].VSync) >> 8)
				& 0xFF, HDMI_V_SYNC_LINE_BEF_2_1);

		writeb(0x0, HDMI_INT_PRO_MODE);

		temp = HDMIVideoParams[format].VTotal;

		writeb(temp & 0xFF, HDMI_VACT_SPACE1_0);
		writeb((temp >> 8) & 0xFF, HDMI_VACT_SPACE1_1);

		temp += HDMIVideoParams[format].VBlank;
		writeb(temp & 0xFF, HDMI_VACT_SPACE2_0);
		writeb((temp >> 8) & 0xFF, HDMI_VACT_SPACE2_1);

		temp = HDMIVideoParams[format].VTotal * 2;
		writeb(temp & 0xFF, HDMI_VACT_SPACE3_0);
		writeb((temp >> 8) & 0xFF, HDMI_VACT_SPACE3_1);

		temp += HDMIVideoParams[format].VBlank;
		writeb(temp & 0xFF, HDMI_VACT_SPACE4_0);
		writeb((temp >> 8) & 0xFF, HDMI_VACT_SPACE4_1);

		temp = HDMIVideoParams[format].VTotal * 3;
		writeb(temp & 0xFF, HDMI_VACT_SPACE5_0);
		writeb((temp >> 8) & 0xFF, HDMI_VACT_SPACE5_1);

		temp += HDMIVideoParams[format].VBlank;
		writeb(temp & 0xFF, HDMI_VACT_SPACE6_0);
		writeb((temp >> 8) & 0xFF, HDMI_VACT_SPACE6_1);
	}

	return 1;
}

#define HDMI_OLD_IP_VER                 0x80
#define HDMI_NEW_IP_VER                 0x00


int hdmi_set_oldIPversion(unsigned char enable)
{
	if (enable) {
		writeb(HDMI_OLD_IP_VER, HDMI_IP_VER);
	} else {
		writeb(HDMI_NEW_IP_VER, HDMI_IP_VER);
	}
	return 1;
}

void d4_hdmi_set_info(struct d4_hdmi_registers *hdmi_regs)
{
	regs_sys = hdmi_regs->register_sys;
	regs_core = hdmi_regs->register_core;
	regs_aes = hdmi_regs->register_aes;

#if 0
	/* adjust the duration of HPD detection */
	writeb(0xFF, regs_core + 0x0D08);

	reg = readb(regs_sys + 0x00);
	writeb((reg | (1<<HDMI_IRQ_GLOBAL) | (1 << HDMI_IRQ_HPD_PLUG)
			| (1 << HDMI_IRQ_HPD_UNPLUG)), regs_sys + 0x00);
#endif	
}

void d4_hdmi_global_interrupt_enable(int is_enable)
{
	unsigned char reg;
	reg = readb(HDMI_SS_INTC_CON_0);	

	if (is_enable) {
		writeb((reg | (1 << HDMI_IRQ_GLOBAL)), HDMI_SS_INTC_CON_0);
	} else {
		writeb(reg & ~ (1 << HDMI_IRQ_GLOBAL), HDMI_SS_INTC_CON_0);	
	}
}

void d4_hdmi_hdcp_interrupt_enable(int is_enable)
{
	unsigned char reg;
	reg = readb(HDMI_SS_INTC_CON_0);	

	if (is_enable) {
		writeb((reg | (1 << HDMI_IRQ_HDCP)), HDMI_SS_INTC_CON_0);
	} else {
		writeb(reg & ~ (1 << HDMI_IRQ_HDCP), HDMI_SS_INTC_CON_0);	
	}
}

void d4_hdmi_cec_interrupt_enable(int is_enable)
{
	unsigned char reg;
	reg = readb(HDMI_SS_INTC_CON_0);	

	if (is_enable) {
		writeb((reg | (1 << HDMI_IRQ_CEC)), HDMI_SS_INTC_CON_0);
	} else {
		writeb(reg & ~ (1 << HDMI_IRQ_CEC), HDMI_SS_INTC_CON_0);	
	}
}


const struct file_operations drime4_hdmi_oper = {
		.owner = THIS_MODULE,
		.open = hdmi_open,
		.release = hdmi_release,
		.unlocked_ioctl = hdmi_ioctl,
		.poll = hdmi_poll,
};
