/**
 * @file d4_hdmi_video_type.h
 * @brief Definition for video
 *
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/**
 * @struct hdcp_struct
 * @brief Structure for processing hdcp
 */

#ifndef _VIDEO_H_
#define _VIDEO_H_


#ifndef __HDMI_VIDEO_VIDEOFORMAT__
#define __HDMI_VIDEO_VIDEOFORMAT__


/**
 * @enum VideoFormat
 * @brief Video formats
 */
enum VideoFormat {

	v640x480p_60Hz = 0,
	v720x480p_60Hz,
	v1280x720p_60Hz,
	v1920x1080i_60Hz,
	v720x480i_60Hz,
	v720x240p_60Hz,
	v2880x480i_60Hz,
	v2880x240p_60Hz,
	v1440x480p_60Hz,
	v1920x1080p_60Hz,
	v720x576p_50Hz,
	v1280x720p_50Hz,
	v1920x1080i_50Hz,
	v720x576i_50Hz,
	v720x288p_50Hz,
	v2880x576i_50Hz,
	v2880x288p_50Hz,
	v1440x576p_50Hz,
	v1920x1080p_50Hz,
	v1920x1080p_24Hz,
	v1920x1080p_25Hz,
	v1920x1080p_30Hz,
	v2880x480p_60Hz,
	v2880x576p_50Hz,
	v1920x1080i_50Hz_1250,
	v1920x1080i_100Hz,
	v1280x720p_100Hz,
	v720x576p_100Hz,
	v720x576i_100Hz,
	v1920x1080i_120Hz,
	v1280x720p_120Hz,
	v720x480p_120Hz,
	v720x480i_120Hz,
	v720x576p_200Hz,
	v720x576i_200Hz,
	v720x480p_240Hz,
	v720x480i_240Hz,
	v1280x720p_24Hz,
	v1280x720p_30Hz,
};
#endif /* __HDMI_VIDEO_VIDEOFORMAT__ */
#ifndef __HDMI_VIDEO_COLORSPACE__
#define __HDMI_VIDEO_COLORSPACE__

/**
 * @enum ColorSpace
 * @brief Color space of video stream.
 */
enum ColorSpace {
	HDMI_CS_RGB,/**< RGB color space */
	HDMI_CS_YCBCR444,/**< YCbCr 4:4:4 color space */
	HDMI_CS_YCBCR422/**< YCbCr 4:2:2 color space */
};

#endif /* __HDMI_VIDEO_COLORSPACE__ */

#ifndef __HDMI_VIDEO_COLORDEPTH__
#define __HDMI_VIDEO_COLORDEPTH__

/**
 * @enum ColorDepth
 * @brief Color depth per pixel of video stream
 */
enum ColorDepth {
	HDMI_CD_36,/**< 36 bit color depth per pixel */
	HDMI_CD_30,/**< 30 bit color depth per pixel */
	HDMI_CD_24/**< 24 bit color depth per pixel */
};
#endif /* __HDMI_VIDEO_COLORDEPTH__ */

#ifndef __HDMI_VIDEO_HDMIMODE__
#define __HDMI_VIDEO_HDMIMODE__

/**
 * @enum HDMIMode
 * @brief System mode
 */
enum HDMIMode {
	DVI = 0,/**< DVI mode */
	HDMI/**< HDMI mode */
};
#endif /* __HDMI_VIDEO_HDMIMODE__ */

#ifndef __HDMI_VIDEO_PIXELLIMIT__
#define __HDMI_VIDEO_PIXELLIMIT__
/**
 * @enum PixelLimit
 * @brief Pixel limitation of video stream
 */
enum PixelLimit {
	HDMI_FULL_RANGE,/**< Full range */
	HDMI_RGB_LIMIT_RANGE,/**< Limit range for RGB color space */
	HDMI_YCBCR_LIMIT_RANGE/**< Limit range for YCbCr color space */
};
#endif /* __HDMI_VIDEO_PIXELLIMIT__ */

#ifndef __HDMI_VIDEO_COLORIMETRY__
#define __HDMI_VIDEO_COLORIMETRY__
/**
 * @enum HDMIColorimetry
 * @brief Colorimetry of video stream
 */
enum HDMIColorimetry {
	HDMI_COLORIMETRY_NO_DATA,/**< Colorimetry is not defined */
	HDMI_COLORIMETRY_ITU601,/**< ITU601 colorimetry */
	HDMI_COLORIMETRY_ITU709,/**< ITU709 colorimetry */
	HDMI_COLORIMETRY_EXTENDED_xvYCC601,/**< Extended ITU601 colorimetry */
	HDMI_COLORIMETRY_EXTENDED_xvYCC709/**< Extended ITU709 colorimetry */
};
#endif /* __HDMI_VIDEO_COLORIMETRY__ */

#ifndef __HDMI_VIDEO_PIXELASPECTRATIO__
#define __HDMI_VIDEO_PIXELASPECTRATIO__
/**
 * @enum PixelAspectRatio
 * @brief Pixel aspect ratio of video stream
 */
enum PixelAspectRatio {
	HDMI_PIXEL_RATIO_AS_PICTURE,/**< as picture pixel ratio */
	HDMI_PIXEL_RATIO_4_3,/**< 4:3 pixel ratio */
	HDMI_PIXEL_RATIO_16_9/**< 16:9 pixel ratio */
};
#endif /* __HDMI_VIDEO_PIXELASPECTRATIO__ */

#ifndef __HDMI_VIDEO_PIXELFREQUENCY__
#define __HDMI_VIDEO_PIXELFREQUENCY__
/**
 * @enum PixelFreq
 * @brief Pixel Frequency
 */
enum PixelFreq {
	PIXEL_FREQ_25_200 = 2520,/**< 25.2 MHz pixel frequency */
	PIXEL_FREQ_25_175 = 2517,/**< 25.175 MHz pixel frequency */
	PIXEL_FREQ_27 = 2700,/**< 27 MHz pixel frequency */
	PIXEL_FREQ_27_027 = 2702,/**< 27.027 MHz pixel frequency */
	PIXEL_FREQ_54 = 5400,/**< 54 MHz pixel frequency */
	PIXEL_FREQ_54_054 = 5405,/**< 54.054 MHz pixel frequency */
	PIXEL_FREQ_74_250 = 7425,/**< 74.25 MHz pixel frequency */
	PIXEL_FREQ_74_176 = 7417,/**< 74.176 MHz pixel frequency */
	PIXEL_FREQ_148_500 = 14850,/**< 148.5 MHz pixel frequency */
	PIXEL_FREQ_148_352 = 14835,/**< 148.352 MHz pixel frequency */
	PIXEL_FREQ_108_108 = 10810,/**< 108.108 MHz pixel frequency */
	PIXEL_FREQ_72 = 7200,/**< 72 MHz pixel frequency */
	PIXEL_FREQ_25 = 2500,/**< 25 MHz pixel frequency */
	PIXEL_FREQ_65 = 6500,/**< 65 MHz pixel frequency */
	PIXEL_FREQ_108 = 10800,/**< 108 MHz pixel frequency */
	PIXEL_FREQ_162 = 16200,/**< 162 MHz pixel frequency */
	PIXEL_FREQ_59_400 = 5940,/**< 59.4 MHz pixel frequency */
};
#endif /* __HDMI_VIDEO_PIXELFREQUENCY__ */

#ifndef __HDMI_PHY_PIXELFREQUENCY__
#define __HDMI_PHY_PIXELFREQUENCY__

/**
 * @enum PHYFreq
 * @brief PHY Frequency
 */
enum PHYFreq {
	PHY_FREQ_NOT_SUPPORTED = -1,
	PHY_FREQ_25_200 = 0,
	PHY_FREQ_25_175,
	PHY_FREQ_27,
	PHY_FREQ_27_027,
	PHY_FREQ_54,
	PHY_FREQ_54_054,
	PHY_FREQ_74_250,
	PHY_FREQ_74_176,
	PHY_FREQ_148_500,
	PHY_FREQ_148_352,
	PHY_FREQ_108_108,
	PHY_FREQ_72,
	PHY_FREQ_25,
	PHY_FREQ_65,
	PHY_FREQ_108,
	PHY_FREQ_162,
	PHY_FREQ_59_400,
};

#endif /* __HDMI_PHY_PIXELFREQUENCY__ */

#ifndef __HDMI_VIDEO_SOURCE__
#define __HDMI_VIDEO_SOURCE__
/**
 * @enum HDMIVideoSource
 * @brief Type of video source.
 */
enum HDMIVideoSource {
	HDMI_SOURCE_INTERNAL,
	HDMI_SOURCE_EXTERNAL,
};
#endif /* __HDMI_VIDEO_SOURCE__ */

#ifndef __HDMI_3D_VIDEO_STRUCTURE__
#define __HDMI_3D_VIDEO_STRUCTURE__

/**
 * @enum HDMI3DVideoStructure
 * @brief Type of 3D Video Structure
 */
enum HDMI3DVideoStructure {
	HDMI_2D_VIDEO_FORMAT = -1,
	HDMI_3D_FP_FORMAT = 0,
	HDMI_3D_SSH_FORMAT,
	HDMI_3D_TB_FORMAT,
	HDMI_3D_FA_FORMAT,
	HDMI_3D_LA_FORMAT,
	HDMI_3D_SSF_FORMAT,
	HDMI_3D_LD_FORMAT,
	HDMI_3D_LDGFX_FORMAT,
};
#endif /* __HDMI_3D_VIDEO_STRUCTURE__ */

#ifndef __HDMI_VIDEO_PARAMETER__
#define __HDMI_VIDEO_PARAMETER__
/**
 * @struct HDMIVideoParameter
 * @brief Structure for HDMI video
 */
struct HDMIVideoParameter {
	enum HDMIMode mode;/**< Video interface */
	enum VideoFormat resolution;/**< Video format */
	enum ColorSpace colorSpace;/**< Color space */
	enum ColorDepth colorDepth;/**< Color depth */
	enum HDMIColorimetry colorimetry;/**< Colorimetry */
	enum PixelAspectRatio pixelAspectRatio;/**< Pixel aspect ratio */
	enum HDMIVideoSource videoSrc;/**< Video Source */
	enum HDMI3DVideoStructure hdmi_3d_format;/**< 3D Video Structure */
};
#endif /* __HDMI_VIDEO_PARAMETER__*/

#endif /* _VIDEO_H_ */
