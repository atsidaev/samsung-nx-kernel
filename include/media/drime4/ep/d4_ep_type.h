/**
 * @file d4_ep_type.h
 * @brief DRIMe4 EP Register Define Header File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_EP_TYPES_H_
#define D4_EP_TYPES_H_

/**
 * @enum	ep_k_intr_flags
 * @brief   EP Core Interrupt flag 종류
 */
enum ep_k_intr_flags {
	EP_K_INTR_INVALID = -1,	/**< Invalid parameter */
	EP_K_LDC_TILE_FINISH = 0, /**< LDC Tile Finish */
	EP_K_LDC_OPERATION_FINISH = 1, /**< LDC Operation Finish */
	EP_K_LDC_ERROR = 2, /**< LDC Error */
	EP_K_HDR_TILE_FINISH = 3, /**< HDR Tile Finish */
	EP_K_HDR_OPERATION_FINISH = 4, /**< HDR Operation Finish */
	EP_K_HDR_ERROR = 5, /**< HDR Error */
	EP_K_NRm_TILE_FINISH = 6, /**< NRm Tile Finish */
	EP_K_NRm_OPERATION_FINISH = 7, /**< NRm Operation Finish */
	EP_K_NRm_ERROR = 8, /**< NRm Error */
	EP_K_MnF_TILE_FINISH = 9, /**< MnF Tile Finish */
	EP_K_MnF_OPERATION_FINISH = 10, /**< MnF Operation Finish */
	EP_K_MnF_ERROR = 11, /**< MnF Error */
	EP_K_FD_TILE_FINISH = 12, /**< FD Tile Finish */
	EP_K_FD_OPERATION_FINISH = 13, /**< FD Operation Finish */
	EP_K_FD_ERROR = 14, /**< FD Error */
	EP_K_BITBLT_OPERATION_FINISH = 15, /**< BITBLT Operation Finish */
	EP_K_BITBLT_ERROR = 16, /**< BITBLT Error */
	EP_K_LVR_VIDEO_Y_TILE_FINISH = 17, /**< LVR Video Y Tile Finish */
	EP_K_LVR_VIDEO_Y_OPERATION_FINISH = 18, /**< LVR Video Y Operation Finish */
	EP_K_LVR_VIDEO_ERROR = 19, /**< LVR Video Error */
	EP_K_LVR_GRP_OPERATION_FINISH = 20, /**< LVR Graphic Operation Finish */
	EP_K_LVR_GRP_ERROR = 21, /**< LVR Graphic Error */
	EP_K_OTF_TILE_FINISH = 22, /**< OTF Tile Finish */
	EP_K_OTF_OPERATION_FINISH = 23, /**< OTF Operation Finish */
	EP_K_TOP_ERROR = 24, /**< TOP Error */
	EP_K_LVR_VIDEO_C_TILE_FINISH = 25, /**< LVR Video C Tile Finish */
	EP_K_LVR_VIDEO_C_OPERATION_FINISH = 26, /**< LVR Video C Operation Finish */
	EP_K_MAX_INTR = 27	/**< EP Core Interrupt의 전체 개수 */
};

/**
 * @enum	ep_k_dma_intr_flags
 * @brief   EP DMA Interrupt flag 종류
 */
enum ep_k_dma_intr_flags {
	EP_K_DMA_INTR_INVALID = -1,	/**< Invalid parameter */
	EP_K_WDMA0_INPUT_END_NRm = 0, /**< WDMA 0 Channel (NRm) Input End */
	EP_K_WDMA0_BUS_END_NRm = 1, /**< WDMA 0 Channel (NRm) Bus End */
	EP_K_WDMA0_ERROR_NRm = 2, /**< WDMA 0 Channel (NRm) Error */
	EP_K_WDMA1_INPUT_END_MnF = 4, /**< WDMA 1 Channel (MnF) Input End */
	EP_K_WDMA1_BUS_END_MnF = 5, /**< WDMA 1 Channel (MnF) Bus End */
	EP_K_WDMA1_ERROR_MnF = 6, /**< WDMA 1 Channel (MnF) Error */
	EP_K_WDMA2_INPUT_END_BBLT = 8, /**< WDMA 2 Channel (BBLT) Input End */
	EP_K_WDMA2_BUS_END_BBLT = 9, /**< WDMA 2 Channel (BBLT) Bus End */
	EP_K_WDMA2_ERROR_BBLT = 10, /**< WDMA 2 Channel (BBLT) Error */
	EP_K_RDMA0_VSYNC_END_NRm = 12, /**< RDMA 0 (NRm) VSync End */
	EP_K_RDMA1_VSYNC_END_MnF1 = 13, /**< RDMA 1 (MnF1) VSync End */
	EP_K_RDMA2_VSYNC_END_MnF2 = 14, /**< RDMA 2 (MnF2) VSync End */
	EP_K_RDMA3_VSYNC_END_BBLT = 15, /**< RDMA 3 (MnF2) VSync End */
	EP_K_SM_SELF_DONE0 = 16, /**< Switching Matrix Self Done 0 */
	EP_K_SM_SELF_ERROR = 17, /**< Switching Matrix Self Error */
	EP_K_DMA_INTR_MAX = 18	/**< EP DMA Interrupt의 전체 개수 */
};

/**
 * @enum	ep_k_path_state
 * @brief   EP Path의 동작 상태표현
 */
enum ep_k_path_state {
	EP_K_PATH_NOT_WORKING = 0, /**< 동작안함 (사용안하는 상태, Path가 반환된 상태) */
	EP_K_PATH_WORKING	= 1, /**< Path가 현재 사용중 */
};

/**
 * @enum	ep_path
 * @brief   EP Path의 종류
 */
enum ep_k_path {
	EP_K_STATE_INVALID = -1, /**< Invalid path */
	EP_K_STATE_LDC_NRM_PATH = 0, /**< On-The-Fly path: LDC->NRm */
	EP_K_STATE_LDC_HDR_NRM_PATH = 1, /**< On-The-Fly path: LDC->HDR->NRm */
	EP_K_STATE_LDC_NRM_MNF_PATH = 2, /**< On-The-Fly path: LDC->NRm->MnF */
	EP_K_STATE_LDC_MNF_PATH = 3, /**< On-The-Fly path: LDC->MnF */
	EP_K_STATE_LDC_HDR_MNF_PATH = 4, /**< On-The-Fly path: LDC->MnF */
	EP_K_STATE_LDC_HDR_NRM_MNF_PATH = 5, /**< On-The-Fly path: LDC->HDR->NRm->MnF */
	EP_K_STATE_NRM_MNF_PATH = 6, /**< On-The-Fly path: NRm->MnF */
	EP_K_STATE_NRM_STANDALONE = 7, /**< Standalone path: NRm (Noise Reduction for movie) */
	EP_K_STATE_MNF_STANDALONE = 8, /**< Standalone path: MnF (Mixer & Filter) */
	EP_K_STATE_FD_STANDALONE = 9, /**< Standalone path: FD (Face Detection) */
	EP_K_STATE_BBLT_STANDALONE = 10, /**< Standalone path: BBLT (BitBLT) */
	EP_K_STATE_LVR_VIDEO_STANDALONE = 11, /**< Standalone path: LVR Video (Liveview Rotator) */
	EP_K_STATE_LVR_VIDEO_OTFSYNC = 12, /**< Standalone path: LVR Video with OTF */
	EP_K_STATE_LVR_GRP_STANDALONE = 13,	/**< Standalone path: LVR Graphic */
	EP_K_STATE_LVR_VIDEO_GRP_STANDALONE = 14,	/**< Standalone path: LVR Video and Graphic */
	EP_K_STATE_MAX = 15
};

/**
 * @enum	ep_k_nlc_mode
 * @brief   NLC 모드
 */
enum ep_k_nlc_mode {
	EP_K_NLC_OFF, /**< NLC Off */
	EP_K_NLD, /**< NLC decoding mode */
	EP_K_NLE, /**< NLC encoding mode */
	EP_K_NLD_RINGPXL, /**< NLC decoding mode (NRm Only) */
	EP_K_NLE_RINGPXL,	/**< NLC encoding mode (NRm Only) */
	EP_K_NLD_NLE,	/**< NLC decoding, encoding mode */
	EP_K_NLD_NLE_RINGPXL	/**< NLC decoding, encoding mode (NRm Only) */
};

/**
 * @enum	ep_k_incr_type
 * @brief   EP Path의 타일 동작모드 종류
 */
enum ep_k_incr_type {
	EP_K_MANUAL_INCREMENT = 0, /**< Manual Increment 모드 (타일단위 처리시 사용) */
	EP_K_AUTO_INCREMENT = 1	/**< Auto Increment 모드 (프레임단위 처리시 사용) */
};

/**
 * @enum	ep_k_yc_type
 * @brief   YC 포맷 (Chroma subsampling) 종류
 */
enum ep_k_yc_type {
	EP_K_YC_INVAL = -1, /**< Invalid type */
	EP_K_YC420 = 0, /**< 420 Chroma subsampling */
	EP_K_YC422 = 1, /**< 422 Chroma subsampling */
	EP_K_YC444 = 2	/**< 444 Chroma subsampling */
};

/**
 * @enum	ep_k_yc_10bit_mode
 * @brief   YCC Bit Width 포맷 종류
 */
enum ep_k_yc_10bit_mode {
	EP_K_YCC_BIT_INVAL = -1,	/**< Invalid type */
	EP_K_YCC_8BIT = 0, /**< 8 bit */
	EP_K_YCC_10BIT = 1	/**< 10 bit */
};

/**
 * @enum	ep_k_ip_bypass
 * @brief   EP Sub-block의 Bypass 상태
 */
enum ep_k_ip_bypass {
	EP_K_IP_BYPASS_OFF = 0, /**< Bypass Off */
	EP_K_IP_BYPASS_ON = 1, /**< Bypass On */
	EP_K_IP_BYPASS_INVAL = 2	/**< Invalid type */
};

/**
 * @enum	MnF_k_operation_mode
 * @brief Operation mode
 */
enum MnF_k_operation_mode {
	MNF_K_OPMODE_YCC_ALU1 = 0,
	MNF_K_OPMODE_BAYER_ALU1 = 1,
	MNF_K_OPMODE_YCC_ALU2 = 2,
	MNF_K_OPMODE_BAYER_ALU2 = 3,
	MNF_K_OPMODE_MASKING = 4,
	MNF_K_OPMODE_BAYER_MIXER = 5,
	MNF_K_OPMODE_RANDOM_NOISE_GEN = 6,
	MNF_K_OPMODE_MINIATURE = 7,
	MNF_K_OPMODE_YCC_MIXER = 8,
	MNF_K_OPMODE_RADIAL_BLUR = 9,
	MNF_K_OPMODE_COLOR_MODE = 10,
	MNF_K_OPMODE_HDR_REGION_MAP = 11,
	MNF_K_OPMODE_3D_SIGNAL_GEN = 12,
	MNF_K_OPMODE_DE = 13,
	MNF_K_OPMODE_BYPASS = 14,
	MNF_K_OPMODE_CONVOLUTION = 15,
	MNF_K_OPMODE_SUMUK = 16,
	MNF_K_OPMODE_ANIMATE = 18,
	MNF_K_OPMODE_HISTOGRAM,
	MNF_K_OPMODE_BAYER_SPLIT,
	MNF_K_OPMODE_CONVOLUTION_BAYER,
	MNF_K_OPMODE_YUV2RGB,
	MNF_K_OPMODE_BAYER_MIXER_SPLIT_OTF
};

/**
 * @struct	ep_reg_phys_info
 * @brief	사용자가 EP Path를 할당받기 위한 IOCTL 인터페이스에서 사용하는 구조체
 * @note	사용자는 IOCTL 인터페이스를 이용하여 (EP_IOCTL_PATH_OPEN)
 * 			구조체에 선언한 형태의 EP Path를 할당받음.
 * 			주의: ring_size 변수를 제외한 모든 변수에 값을 할당해 줘야 함. (ring_size 변수는 값 할당 금지)
 */
struct ep_reg_phys_info {
	unsigned int reg_start_addr; /**< Register physical start address */
	unsigned int reg_size;		 /**< Register size */
};

/**
 * @struct	ep_reg_info
 * @brief	사용자가 EP Path를 할당받기 위한 IOCTL 인터페이스에서 사용하는 구조체
 * @note	사용자는 IOCTL 인터페이스를 이용하여 (EP_IOCTL_PATH_OPEN)
 * 			구조체에 선언한 형태의 EP Path를 할당받음.
 * 			주의: ring_size 변수를 제외한 모든 변수에 값을 할당해 줘야 함. (ring_size 변수는 값 할당 금지)
 */
struct ep_reg_info {
	struct ep_reg_phys_info reg_base_top;
	struct ep_reg_phys_info reg_base_ldc;
	struct ep_reg_phys_info reg_base_hdr;
	struct ep_reg_phys_info reg_base_nrm;
	struct ep_reg_phys_info reg_base_mnf;
	struct ep_reg_phys_info reg_base_fd;
	struct ep_reg_phys_info reg_base_bblt;
	struct ep_reg_phys_info reg_base_lvr;
	struct ep_reg_phys_info reg_base_dma;
};


/**
 * @struct	ep_intr_wait_info
 * @brief	특정 Interrupt의 발생을 대기하는 IOCTL 인터페이스를 이용하기 위한 구조체
 * @note	사용자는 IOCTL 인터페이스를 이용하여 (EP_IOCTL_INTR_WAIT)
 * 			intr 변수에 선언된 인터럽트가 발생할때 까지 기다림.
 * 			최대 timeout_ms에 설정된 시간 (millisecond 단위)만큼 기다리고 인터럽트가 발생하지 않는다면 timeout 발생.
 */
struct ep_k_intr_wait_info {
	int timeout_ms; /**< 타임아웃 시간 (millisecond 단위) */
	enum ep_k_intr_flags intr; /**< 인터럽트 종류 */
};

/**
 * @struct	ep_dma_intr_wait_info
 * @brief	특정 EP DMA Interrupt의 발생을 대기하는 IOCTL 인터페이스를 이용하기 위한 구조체
 * @note	사용자는 IOCTL 인터페이스를 이용하여 (EP_IOCTL_DMA_INTR_WAIT)
 * 			intr 변수에 선언된 인터럽트가 발생할때 까지 기다림.
 * 			최대 timeout_ms에 설정된 시간 (millisecond 단위)만큼 기다리고 인터럽트가 발생하지 않는다면 timeout 발생.
 */
struct ep_k_dma_intr_wait_info {
	int timeout_ms; /**< 타임아웃 시간 (millisecond 단위) */
	enum ep_k_dma_intr_flags intr; /**< 인터럽트 종류 */
};

/**
 * @struct	ep_k_path_ring_size_info
 * @brief	EP Path의 구성에 따라 계산된 Ring Pixel 정보를 담기 위한 구조체
 * @note	사용자는 IOCTL 인터페이스를 이용하여 (EP_IOCTL_PATH_GET_RINGSIZE)
 * 			현재 할당받은 Path path_id를 드라이버에게 넘겨주면,
 * 			드라이버에 의해 ldc_rsize, hdr_rsize, nrm_rsize가 계산됨
 * 			주의: ldc_rsize, hdr_rsize, nrm_rsize 변수는 사용자가 값 할당 금지
 */
struct ep_k_path_ring_size_info {
	int path_id; /**< 사용자가 할당받은 Path 의 ID */
	int ldc_rsize, hdr_rsize, nrm_rsize; /**< read only (드라이버에서 계산하는 변수, 사용자가 값 할당 금지) */
};

/**
 * @struct	ep_k_path_info
 * @brief	사용자가 EP Path를 할당받기 위한 IOCTL 인터페이스에서 사용하는 구조체
 * @note	사용자는 IOCTL 인터페이스를 이용하여 (EP_IOCTL_PATH_OPEN)
 * 			구조체에 선언한 형태의 EP Path를 할당받음.
 * 			주의: ring_size 변수를 제외한 모든 변수에 값을 할당해 줘야 함. (ring_size 변수는 값 할당 금지)
 */
struct ep_k_path_info {
	/* Path 종류 */
	enum ep_k_path path;	/**< Path 종류 */

	/* Top Control 특성 */
	enum ep_k_nlc_mode nlc_mode;	/**< BBLT, NRm 의 NLC 모드 */
	enum ep_k_incr_type incr_type;	/**< Tile의 Increment Mode 설정 (Frame 모드일 경우 Auto Increment) */
	int tile_num;	/**< Tile 개수 설정 */

	/* 영상 특성 */
	enum ep_k_yc_type yc_type;	/**< YC 영상 타입 설정 */
	enum ep_k_yc_10bit_mode yc_bit;	/**< YC 영상의 Bit 모드 설정 */

	/* IP Bypass 여부 */
	enum ep_k_ip_bypass ldc_bp;	/**< LDC의 Bypass 여부 */
	enum ep_k_ip_bypass nrm_bp;	/**< NRm의 Bypass 여부 */
	enum ep_k_ip_bypass mnf_bp;	/**< MnF의 Bypass 여부 */

	/* 개별 IP 특성 */
	enum MnF_k_operation_mode mnf_mode; /**< MnF 동작모드 설정 (MnF 블럭의 설정내용과 동일하게 입력), MnF 모드에 따라 다른 링 픽셀 정보를 구하기 위해 필요 */

	/* 링 픽셀 정보 (드라이버에 의해 계산) */
	struct ep_k_path_ring_size_info ring_size; /**< internal use only (드라이버에서 계산하는 변수, 사용자가 값 할당 금지) */
};

/**
 * @struct	ep_path_desc
 * @brief	EP에서 현재 할당되어 사용중인 Path 정보를 담고 있는 구조체
 * @note	디바이스 드라이버 내부에서 사용하는 구조체
 */
struct ep_path_desc {
	int id; /**< descriptor id */
	enum ep_k_path_state state; /**< 현재 패스를 동작(사용) 여부 */
	enum ep_k_path path; /**< 해당 패스의 타입 */
	struct ep_k_path_info info; /**< 해당 패스의 정보를 담은 구조체 */
};

/**
 * @enum ep_onoff
 * @brief ON/OFF및 동작 설정을 위한 Enumeration
 */
enum ep_dd_onoff {
	EP_DD_OFF,		/**< 동작  중지 및 설정 OFF */
	EP_DD_ON		/**< 동작  중지 및 설정 ON*/
};

/**
 * @struct	ep_path_block_start_info
 * @brief	EP_IOCTL_PATH_FRAME_BLOCK_START, EP_IOCTL_PATH_TILE_BLOCK_START IOCTL 인터페이스를 이용하기 위한 구조체
 * @note	사용자는 IOCTL 인터페이스를 이용하여 (EP_IOCTL_PATH_FRAME_BLOCK_START, EP_IOCTL_PATH_TILE_BLOCK_START)
 * 			core_wait_info, dma_wait_info 변수에 선언된 인터럽트가 발생할때 까지 기다림.
 */
struct ep_k_path_block_start_info {
	int path_id; /**< 사용자가 할당받은 Path 의 ID */
	int tile_idx; /**< Manual Tile Mode 일 경우 현재 Start 하려는 타일의 번호 입력 (주의: Manual Tile Mode 일 경우, 필히 입력해야함, 0번 부터 시작) */
	struct ep_k_intr_wait_info core_wait_info; /**< 대기하려는 Core Interrupt와 Timeout 설정 */
	struct ep_k_dma_intr_wait_info dma_wait_info; /**< 대기하려는 DMA Interrupt와 Timeout 설정 */
};

/**
 * @enum	ep_path_start_type
 * @brief   EP Path의 Start 종류 (Tile 혹은 Frame)
 */
enum ep_k_path_start_type {
	EP_K_PATH_TILE_START,
	EP_K_PATH_FRAME_START
};

/**
 * @enum	ep_start_frame
 * @brief   Sub-IP별 Frame Start 신호 값
 * 			OR 연산으로 묶어서 사용가능
 * 			예시) LDC와 HDR을 동시에 Frame Start 할 경우 => START_FRAME_LDC | START_FRAME_HDR
 */
enum ep_k_start_frame {
	START_K_FRAME_INVALID = 0x0,
	START_K_FRAME_LDC = 0x1,
	START_K_FRAME_HDR = 0x2,
	START_K_FRAME_NRM = 0x4,
	START_K_FRAME_MNF = 0x8,
	START_K_FRAME_FD = 0x10,
	START_K_FRAME_BBLT = 0x20,
	START_K_FRAME_LVR_VIDEO = 0x40,
	START_K_FRAME_LVR_GRP = 0x80
};

/**
 * @enum	ep_start_tile
 * @brief   Sub-IP별 Tile Start 신호 값
 * 			OR 연산으로 묶어서 사용가능
 * 			예시) LDC와 HDR을 동시에 Tile Start 할 경우 => START_TILE_LDC | START_TILE_HDR
 */
enum ep_k_start_tile {
	START_K_TILE_INVAILD = 0x0,
	START_K_TILE_LDC = 0x1,
	START_K_TILE_HDR = 0x2,
	START_K_TILE_NRM = 0x4,
	START_K_TILE_MNF = 0x8,
	START_K_TILE_LVR_VIDEO = 0x40
};

#endif
