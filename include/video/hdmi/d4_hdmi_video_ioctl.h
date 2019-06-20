/**
 * @file d4_hdmi_video_ioctl.h
 * @brief header file for HDMI video ioctls.
 *
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _LINUX_HDMI_H_
#define _LINUX_HDMI_H_

#define HDMI_IOC_MAGIC		'y'

#include "d4_hdmi_video_type.h"

#ifndef __HDMI_VIDEO_PARAMS__
/**
 * @struct 	hdmi_video_params
 * @brief  time generation set for different resolutions
 */
struct hdmi_video_params {
	unsigned int HTotal;
	unsigned int HBlank;
	unsigned int VTotal;
	unsigned int VBlank;
	unsigned int HFront;
	unsigned int HSync;
	unsigned int HPol;
	unsigned int VFront;
	unsigned int VSync;
	unsigned int VPol;
	unsigned int AVI_VIC;
	unsigned int AVI_VIC_16_9;
	unsigned int interlaced;/**< 0 - progressive, 1 - interlaced */
	unsigned int repetition;/**< Pixel repetition if double, set 1 */
};
#endif


#define HDCP_KSV_SIZE		5/**< Size of KSV */

#define HDCP_STATUS_SIZE	2/**< Size of Bstatus */

#define HDCP_RI_SIZE		2/**< Size of Ri */

#define HDCP_AN_SIZE		8/**< Size of An */

#define HDCP_SHA1_SIZE		20/**< Size of SHA1 result */

/**
 * @struct hdcp_ksv
 * @brief Contains KSV(Key Selection Vector).
 */
struct hdcp_ksv {
	unsigned char ksv[HDCP_KSV_SIZE];/**< KSV */
};

/**
 * @struct hdcp_status
 * @brief Contains Bstatus.
 */
struct hdcp_status {
	unsigned char status[HDCP_STATUS_SIZE];/**< Status */
};

/**
 * @struct hdcp_an
 * @brief Contains An(64-bit pseudo-random value).
 */
struct hdcp_an {
	unsigned char an[HDCP_AN_SIZE];
};

/**
 * @struct hdcp_ksv_list
 * @brief Contains one KSV and flag that indicates whether this is the last KSV @n
 * in KSV list or not
 */
struct hdcp_ksv_list {
	unsigned char end;/**< Flag that indicates structure contains the last KSV in KSV list. */
	unsigned char ksv[HDCP_KSV_SIZE];/**< KSV */
};

/**
 * @struct hdcp_sha1
 * @brief Contains SHA1 calculated result.
 */
struct hdcp_sha1 {
	unsigned char sha1[HDCP_SHA1_SIZE];/**< SHA1 calculated result */
};

/**
 * @struct hdmi_dbg
 * @brief Contains offset and value to set.
 */
struct hdmi_dbg {
	unsigned int offset;
	unsigned int value;
};

#define HDMI_IOC_SET_COLORSPACE			_IOW(HDMI_IOC_MAGIC, 0, enum ColorSpace)/**< Device request code to set color space. */

#define HDMI_IOC_SET_COLORDEPTH			_IOW(HDMI_IOC_MAGIC, 1, enum ColorDepth)/**< Device request code to set color depth */

#define HDMI_IOC_SET_HDMIMODE			_IOW(HDMI_IOC_MAGIC, 2, enum HDMIMode)/**< Device request code to set video system */

#define HDMI_IOC_SET_VIDEOMODE			_IOW(HDMI_IOC_MAGIC, 3, struct HDMIVideoParameter)/**< Device request code to set video timing parameters */

#define HDMI_IOC_SET_BLUESCREEN			_IOW(HDMI_IOC_MAGIC, 4, unsigned char)/**< Device request code to set/clear blue screen, 1 to set, 0 to clear */

#define HDMI_IOC_SET_PIXEL_LIMIT		_IOW(HDMI_IOC_MAGIC, 5, enum PixelLimit)/**< Device request code to set pixel limitation */

#define HDMI_IOC_SET_AVMUTE			_IOW(HDMI_IOC_MAGIC, 6, unsigned char)/**< Device request code to set/clear AVMute */

#define HDMI_IOC_SET_AUDIOPACKETTYPE		_IOW(HDMI_IOC_MAGIC, 7, enum HDMIASPType)/**< Device request code to set packet type of HDMI audio output */

#define HDMI_IOC_SET_AUDIOSAMPLEFREQ		_IOW(HDMI_IOC_MAGIC, 8, enum SamplingFreq)/**< Device request code to set audio sample frequency */

#define HDMI_IOC_SET_AUDIOCHANNEL		_IOW(HDMI_IOC_MAGIC, 9, enum ChannelNum)/**< Device request code to set number of channels */

#define HDMI_IOC_SET_SPEAKER_ALLOCATION		_IOW(HDMI_IOC_MAGIC, 10, unsigned int)/**< Device request code to set audio speaker allocation information */

#define HDMI_IOC_ENABLE_HDCP			_IOW(HDMI_IOC_MAGIC, 11, unsigned char)/**< Device request code to enable/disable HDCP H/W, 1 = enable, 0 = disable */

#define HDMI_IOC_SET_BKSV			_IOW(HDMI_IOC_MAGIC, 12, struct hdcp_ksv)/**< Device request code to set Bksv  */

#define HDMI_IOC_SET_BCAPS			_IOW(HDMI_IOC_MAGIC, 13, unsigned char)/**< Device request code to set Bcaps  */

#define HDMI_IOC_SET_HDCP_RESULT		_IOW(HDMI_IOC_MAGIC, 14, unsigned char)/**< Device request code to set the result whether Ri and Ri' are match or not */

#define HDMI_IOC_SET_BSTATUS			_IOW(HDMI_IOC_MAGIC, 15, struct hdcp_status)/**< Device request code to set BStatus  */

#define HDMI_IOC_SET_KSV_LIST			_IOW(HDMI_IOC_MAGIC, 16, struct hdcp_ksv_list)/**< Device request code to set KSV list  */

#define HDMI_IOC_SET_SHA1			_IOW(HDMI_IOC_MAGIC, 17, struct hdcp_sha1)/**< Device request code to set Rx SHA1 calculated result  */

#define HDMI_IOC_SET_AUDIO_ENABLE		_IOW(HDMI_IOC_MAGIC, 18, unsigned char)/**< Device request code to start/stop sending audio packet */

#define HDMI_IOC_SET_PIXEL_ASPECT_RATIO		_IOW(HDMI_IOC_MAGIC, 19, enum PixelAspectRatio)/**< Device request code to set pixel aspect ratio */

#define HDMI_IOC_SET_COLORIMETRY		_IOW(HDMI_IOC_MAGIC, 20, enum HDMIColorimetry)/**< Device request code to set colorimetry */

#define HDMI_IOC_SET_VIDEO_SOURCE		_IOW(HDMI_IOC_MAGIC, 21, enum HDMIVideoSource)/**< Device request code to set HDMI video source */

#define HDMI_IOC_SET_IP_VERSION			_IOW(HDMI_IOC_MAGIC, 23, unsigned char)/**< Device request code to set HDMI IP version */

#define HDMI_IOC_SET_ENCRYPTION			_IOW(HDMI_IOC_MAGIC, 24, unsigned char)/**< Device request code to start/stop sending EESS */


/* for debugging	*/
#define HDMI_IOC_SET_DEBUG			_IOW(HDMI_IOC_MAGIC, 25, struct hdmi_dbg)/**< Device request code to set HDMI debug */

#define HDMI_IOC_GET_PHYREADY			_IOR(HDMI_IOC_MAGIC, 26, unsigned char)/**< Device request code to get status of HDMI PHY H/W	*/

#define HDMI_IOC_GET_HDCP_EVENT			_IOR(HDMI_IOC_MAGIC, 27, enum hdcp_event)/**< Device request code to get hdcp_event */

#define HDMI_IOC_GET_AKSV			_IOR(HDMI_IOC_MAGIC, 28, struct hdcp_ksv)/**< Device request code to get Aksv */

#define HDMI_IOC_GET_AN				_IOR(HDMI_IOC_MAGIC, 29, struct hdcp_an)/**< Device request code to get An */

#define HDMI_IOC_GET_RI				_IOR(HDMI_IOC_MAGIC, 30, int)/**< Device requset code to get Ri */

#define HDMI_IOC_GET_SHA1_RESULT		_IOR(HDMI_IOC_MAGIC, 31, int)/**< Device request code to get SHA1 result	*/

#define HDMI_IOC_GET_KSV_LIST_READ_DONE		_IOR(HDMI_IOC_MAGIC, 32, unsigned char)/**< Device request code to get whether HDCP H/W finishes reading KSV or not */

#define HDMI_IOC_START_HDMI			_IO(HDMI_IOC_MAGIC, 33)/**< Device request code to start sending HDMI output */

#define HDMI_IOC_STOP_HDMI			_IO(HDMI_IOC_MAGIC, 34)/**< Device request code to stop sending HDMI output */

#define HDMI_IOC_STOP_HDCP			_IO(HDMI_IOC_MAGIC, 35)/**< Device request code to stop processing HDCP */

#define HDMI_IOC_SET_ILLEGAL_DEVICE		_IO(HDMI_IOC_MAGIC, 36)/**< Device request code to notify that one of downstream receivers is illegal */

#define HDMI_IOC_SET_REPEATER_TIMEOUT		_IO(HDMI_IOC_MAGIC, 37)/**< Device request code to notify that repeater is not ready within 5 seconds */

#define HDMI_IOC_RESET_HDCP			_IO(HDMI_IOC_MAGIC, 38)/**< Device request code to reset HDCP H/W  */

#define HDMI_IOC_SET_KSV_LIST_EMPTY		_IO(HDMI_IOC_MAGIC, 39)/**< Device request code to notify if Tx is repeater */

#define HDMI_IOC_RESET_AUISAMPLEFREQ		_IO(HDMI_IOC_MAGIC, 40)/**< Device request code to reset AUI Sampling Frequency Fields  */

#define HDMI_IOC_DEBUG_HDMI_CORE		_IO(HDMI_IOC_MAGIC, 41)/**< for checking register map*/

#define HDMI_IOC_DEBUG_HDMI_CORE_VIDEO		_IO(HDMI_IOC_MAGIC, 42)/**< for checking video register */

#endif /* _LINUX_HDMI_H_ */
