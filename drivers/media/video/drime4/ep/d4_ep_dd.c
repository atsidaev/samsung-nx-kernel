/**
 * @file d4_ep_dd.c
 * @brief DRIMe4 EP device driver file
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/memory.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/slab.h>
#ifdef CONFIG_PMU_SELECT
#include <linux/d4-pmu.h>
#endif

#ifdef CONFIG_RMU_DEV
#include <linux/d4_rmu.h>
#endif

#include <linux/clk.h>
#include <asm/atomic.h>
#include "d4_ep_dd.h"

struct ep_reg_info reg_info;


static void __iomem *top_addr;
static void __iomem *dma_addr;

/**< EP Core Device Info */
static struct device *ep_core_dev;

static struct completion core_irq_wait[EP_K_MAX_INTR];
static struct completion dma_irq_wait[EP_K_DMA_INTR_MAX];

/**< EP Path Info */
static DEFINE_MUTEX(path_lock);
static struct ep_path_desc ep_path_list[EP_PATH_LIST_CNT];

/**< EP Path Blocking Mode */
static DEFINE_MUTEX(ep_path_blocker);
static int ep_path_blocker_counter[EP_K_STATE_MAX];
static struct mutex ep_path_blocker_mutex[EP_K_STATE_MAX];

/**< EP Path ID Generator */
static DEFINE_MUTEX(path_id_counter_lock);
static int ep_path_id_counter = EP_PATH_ID_MIN;

/**
 * @brief ep_path_map: EP Path 들의 상관관계를 나타내는 맵
 * 	- 0: 서로 독립적으로 사용 가능
 *     - 1: 서로 독립적으로 사용 불가능
 */
const static char ep_path_map[EP_K_STATE_MAX][EP_K_STATE_MAX] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}
};

/**
 * @brief   사용가능한 Path ID를 생성하여 반환하는 함수
 * @fn      static int ep_top_path_calc_path_id(void)
 * @return  int		사용가능한 Path ID 값 반환
 *
 * @author  Wooram Son
 */
static int ep_top_path_calc_path_id(void)
{
	int i, j;
	int success = 0;
	int duplicate_check = 0;

	mutex_lock(&path_id_counter_lock);

	for (j = 0; j < EP_PATH_LIST_CNT; j++) {
		ep_path_id_counter = ep_path_id_counter + 1;
		if (ep_path_id_counter >= EP_PATH_ID_MAX)
			ep_path_id_counter = EP_PATH_ID_MIN;

		duplicate_check = 0;
		for (i = 0; i < EP_PATH_LIST_CNT; i++) {
			if (ep_path_list[i].state == EP_K_PATH_WORKING &&
				ep_path_list[i].id == ep_path_id_counter) {
				duplicate_check = 1;
				break;
			}
		}

		if (duplicate_check == 0) {
			success = 1;
			break;
		}
	}

	mutex_unlock(&path_id_counter_lock);

	if (success == 1)
		return ep_path_id_counter;

	return -1;
}

/**
 * @brief   해당 Path 사용을 등록하는 함수
 * @fn      static int ep_top_path_register(enum ep_k_path path)
 * @param   path	[in] Path의 종류를 나타내는 enum
 * @return  int		Path 할당 성공시 Path ID 반환, 실패할 경우 음수 반환
 *
 * @author  Wooram Son
 */
static int ep_top_path_register(enum ep_k_path path)
{
	int i = 0;
	int path_id = -1;
	int path_collision_check = 0;

	if (path <= EP_K_STATE_INVALID || path >= EP_K_STATE_MAX) {
		EP_DD_ERROR_MSG("[EP, %s, %d] parameter error! (path)\n", __FILE__, __LINE__);
		return -1;
	}

	mutex_lock(&path_lock);

	/**
	 * @brief 기존 Path와 충돌 검사
	 */
	for (i = 0; i < EP_PATH_LIST_CNT; i++) {
		if (ep_path_list[i].state == EP_K_PATH_WORKING) {
			if (ep_path_map[ep_path_list[i].path][path] != 0) { /* Path 충돌 체크 */
				path_collision_check = 1;
				break;
			}
		}
	}

	/**
	 * @brief 신규 Path 등록
	 */
	if (path_collision_check == 0) {
		if (ep_path_list[(int)path].state == EP_K_PATH_NOT_WORKING) {
			path_id = ep_top_path_calc_path_id();
			ep_path_list[(int)path].id = path_id;
			ep_path_list[(int)path].path = (int)path;
			ep_path_list[(int)path].state = EP_K_PATH_WORKING;
			}

		if (path_id < EP_PATH_ID_MIN) {
			EP_DD_ERROR_MSG("[EP, %s, %d] path list is full!\n", __FILE__, __LINE__);
			mutex_unlock(&path_lock);
			return -3;
		}

		mutex_unlock(&path_lock);
		return path_id;
	} else {
		EP_DD_DEBUG_MSG("[EP, %s, %d] EP Path Conflict! (PID: %d, TGID: %d)\n", __FILE__, __LINE__, current->tgid, current->pid);
		mutex_unlock(&path_lock);
		return -2;
	}
}

/**
 * @brief   해당 Path 를 등록해제 하는 함수
 * @fn      static int ep_top_path_unregister(struct ep_path_desc *path_desc)
 * @param   *path_desc	[in] 할당된 Path의 Descriptor 구조체 포인터
 * @return  int		등록해제 성공시 0 반환, 실패할 경우 음수 반환
 *
 * @author  Wooram Son
 */
static int ep_top_path_unregister(struct ep_path_desc *path_desc)
{
	if (path_desc == NULL) {
		EP_DD_ERROR_MSG("[EP, %s, %d] parameter error! (path_desc)\n", __FILE__, __LINE__);
		return -1;
	}

	mutex_lock(&path_lock);

	path_desc->id = -1;
	path_desc->state = EP_K_PATH_NOT_WORKING;
	path_desc->path = EP_K_STATE_INVALID;

	mutex_unlock(&path_lock);

	return 0;
}

void ep_path_blocker_init(void)
{
	int i;
	for (i = 0; i < EP_K_STATE_MAX; i++) {
		ep_path_blocker_counter[i] = 0;
		mutex_init(&ep_path_blocker_mutex[i]);
	}
}


/**
 * @brief   EP Path 관리 모듈에서 관리되는 모든 Path를 초기화 하는 함수
 * @fn      int ep_top_path_init(void)
 * @param   void
 * @return  int		0: 정상적으로 셋팅완료, 그 외 값: 파라미터 오류
 * @note	EP TOP을 초기화 할 때 함께 초기화 해주어야 한다.
 * 			사용자가 Path를 사용한뒤 close 해주지 않아
 * 			Path 관리가 불가능해진 상황에서 초기화 할때도 사용한다.
 *
 * @author  Wooram Son
 */
int ep_path_init(void)
{
	int i = 0;

	mutex_lock(&path_lock);
	for (i = 0; i < EP_PATH_LIST_CNT; i++) {
		ep_path_list[i].id = -1;
		ep_path_list[i].state = EP_K_PATH_NOT_WORKING;
		ep_path_list[i].path = EP_K_STATE_INVALID;
	}
	mutex_unlock(&path_lock);
	return 0;
}

/**
 * @brief   EP Path 관리 모듈에서 관리되는 모든 Path를 초기화 하는 함수 (현재 열려있는 Path가 없을 경우만 초기화 수행)
 * @fn      int ep_top_path_try_init(void)
 * @param   void
 * @return  int		0: 정상적으로 초기화 완료, 그 외 값: 초기화 오류 발생
 * @note	현재 열려있는 Path가 없을경우 초기화를 수행하고, 하나의 Path라도 열려있는 경우 초기화를 수행하지 않고 -1을 리턴한다.
 *
 * @author  Wooram Son
 */
int ep_path_try_init(void)
{
	int i = 0;
	int ret = 0;

	mutex_lock(&path_lock);
	for (i = 0; i < EP_PATH_LIST_CNT; i++) {
		if (ep_path_list[i].state == EP_K_PATH_WORKING) {
			ret = -1;
			EP_DD_ERROR_MSG("[EP, %s, %d] path is not closed yet!\n", __FILE__, __LINE__);
			break;
		}
	}
	if (ret == 0) {
		for (i = 0; i < EP_PATH_LIST_CNT; i++) {
			ep_path_list[i].id = -1;
			ep_path_list[i].state = EP_K_PATH_NOT_WORKING;
			ep_path_list[i].path = EP_K_STATE_INVALID;
		}
	}
	mutex_unlock(&path_lock);
	return ret;
}

#ifndef ENABLE_EP_PATH_OPEN_NONBLOCKING_MODE
static void ep_path_blocker_lock(enum ep_k_path path)
{
	int i;

	EP_DD_DEBUG_MSG("##### EP Path open try - PATH: %d, PID: %d, TGID: %d\n", path, current->tgid, current->pid);

	mutex_lock(&ep_path_blocker_mutex[path]);

	mutex_lock(&ep_path_blocker);
	for (i = 0; i < EP_K_STATE_MAX; i++) {
		if (ep_path_map[path][i] != 0) {
			ep_path_blocker_counter[i]++;
			mutex_trylock(&ep_path_blocker_mutex[path]);
		}
	}
	mutex_unlock(&ep_path_blocker);
}

static void ep_path_blocker_unlock(enum ep_k_path path)
{
	int i;

	mutex_lock(&ep_path_blocker);
	for (i = 0; i < EP_K_STATE_MAX; i++) {
		if (ep_path_map[path][i] != 0) {
			if (ep_path_blocker_counter[i] > 0)
				ep_path_blocker_counter[i]--;

			if (ep_path_blocker_counter[i] == 0)
				mutex_unlock(&ep_path_blocker_mutex[i]);
		}
	}
	mutex_unlock(&ep_path_blocker);
}
#endif

/**
 * @brief   EP의 특정 Path를 사용자가 할당 받고자 할때 사용하는 함수
 * @fn      int ep_path_open(struct ep_k_path_info *path_info, bool ep_reset)
 * @param   *path_info	[in] 사용자가 할당 받고자 하는 Path의 정보를 담은 구조체
 * @param   ep_reset	[in] Path를 할당받으면서 EP Top을 SW Reset 여부 (true: SW Reset 수행)
 * @return  int		0: 정상적으로 할당 완료, 그 외 값: 할당 오류
 *
 * @author  Wooram Son
 */
int ep_path_open(struct ep_k_path_info *path_info)
{
	int path_id = -1;
	enum ep_k_path path = 0;
	struct ep_path_desc *path_desc = NULL;

	if (path_info == NULL) {
		EP_DD_ERROR_MSG("[EP, %s, %d] parameter error! (path_info)\n", __FILE__, __LINE__);
		return -1;
	}

	/**< 신규 Path 등록 (실패시 음수 반환) */
	path = path_info->path;

#ifdef ENABLE_EP_PATH_OPEN_NONBLOCKING_MODE
	path_id = ep_top_path_register(path);
	if (path_id >= EP_PATH_ID_MIN && path_id < EP_PATH_ID_MAX) {
		EP_DD_DEBUG_MSG("[EP, %s, %d] ep path allocation success! (path: %d, path_id: %d)\n", __FILE__, __LINE__, path, path_id);
	} else {
		EP_DD_ERROR_MSG("[EP, %s, %d] ep path allocation fail! \n", __FILE__, __LINE__);
		return -2;
	}
#else
	/**< TODO (2012-11-08): NX300 양산팀의 요청으로 ep_path_open() 함수를 Blocking Mode로 변경 */
	while (1) {
		ep_path_blocker_lock(path);
		path_id = ep_top_path_register(path);
		if (path_id >= EP_PATH_ID_MIN && path_id < EP_PATH_ID_MAX) {
			EP_DD_DEBUG_MSG("[EP, %s, %d] ep path allocation success! (path: %d, path_id: %d)\n", __FILE__, __LINE__, path, path_id);
			break;
		} else {
			EP_DD_ERROR_MSG("[EP, %s, %d] ep path allocation fail! \n", __FILE__, __LINE__);
			ep_path_blocker_unlock(path);
		}
	}
#endif

	/**< 신규 Path의 Descriptor 가져오기 */
	path_desc = ep_path_get_descriptor(path_id);
	if (path_desc == NULL) {
		EP_DD_ERROR_MSG("[EP, %s, %d] ep path allocation fail!, ep_path_get_descriptor() error! \n", __FILE__, __LINE__);
		return -3;
	}

	/**< Path Descriptor 설정 */
	path_desc->info.ldc_bp = path_info->ldc_bp;
	path_desc->info.nrm_bp = path_info->nrm_bp;
	path_desc->info.mnf_bp = path_info->mnf_bp;

	path_desc->info.mnf_mode = path_info->mnf_mode;	/**< MnF Mode */
	path_desc->info.nlc_mode = path_info->nlc_mode;	/**< NLC Mode */

	path_desc->info.yc_type = path_info->yc_type;
	path_desc->info.yc_bit = path_info->yc_bit;
	path_desc->info.tile_num = path_info->tile_num;
	path_desc->info.incr_type = path_info->incr_type;

	path_desc->info.ring_size.path_id = path_id;
	path_desc->info.ring_size = path_info->ring_size;

	return path_id;
}

/**
 * @brief   사용자가 이전에 할당받은 Path를 반환하고자 할때 사용하는 함수
 * @fn      int ep_path_close(int path_id)
 * @param   path_id	[in] 이전에 할당받은 Path의 path_id 값
 * @return  int		0: 정상적으로 반환완료, 그 외 값: 반환 에러발생
 *
 * @author  Wooram Son
 */
int ep_path_close(int path_id)
{
	int ret = 0;
	enum ep_k_path path = 0;
	struct ep_path_desc *path_desc = NULL;

	path_desc = ep_path_get_descriptor(path_id);
	if (path_desc == NULL) {
		EP_DD_ERROR_MSG("[EP, %s, %d] ep_path_close fail!, ep_path_get_descriptor() error! \n", __FILE__, __LINE__);
		return -1;
	}

	/* ep_top_ctrl_end(path_desc);	*/
	path = path_desc->path;
	ret = ep_top_path_unregister(path_desc);

	if (ret == 0) {
		EP_DD_DEBUG_MSG("[EP, %s, %d] ep path close success!\n", __FILE__, __LINE__);
#ifndef ENABLE_EP_PATH_OPEN_NONBLOCKING_MODE
		ep_path_blocker_unlock(path);
#endif
	} else {
		EP_DD_ERROR_MSG("[EP, %s, %d] ep path close fail!\n", __FILE__, __LINE__);
	}

	return ret;
}

/**
 * @brief   사용자가 이전에 할당받 Path의 Descriptor를 가져오는 함수
 * @fn      struct ep_path_desc *ep_path_get_descriptor(int path_id)
 * @param   path_id	[in] 사용자가 특정 Path를 Open하여 얻은 path_id 변수값을 입력으로 한다.
 * @return  *ep_path_desc	path_id에 해당하는 ep_path_desc 구조체의 주소를 반환한다.
 * 							해당하는 descriptor가 없다면 NULL을 반환한다.
 *
 * @author  Wooram Son
 */
struct ep_path_desc *ep_path_get_descriptor(int path_id)
{
	int i;
	struct ep_path_desc *desc = NULL;

	if (path_id < EP_PATH_ID_MIN || path_id >= EP_PATH_ID_MAX) {
		EP_DD_ERROR_MSG("[EP, %s, %d] ep_path_get_descriptor fail (path_id is wrong!)\n", __FILE__, __LINE__);
		return NULL;
	}
	mutex_lock(&path_lock);

	for (i = 0; i < EP_PATH_LIST_CNT; i++)
		if (ep_path_list[i].id == path_id)
			desc = &ep_path_list[i];

	mutex_unlock(&path_lock);

	return desc;
}

void ep_set_top_reg_info(unsigned int addr, unsigned int size)
{
	reg_info.reg_base_top.reg_start_addr = addr;
	reg_info.reg_base_top.reg_size = size;
}

void ep_set_ldc_reg_info(unsigned int addr, unsigned int size)
{
	reg_info.reg_base_ldc.reg_start_addr = addr;
	reg_info.reg_base_ldc.reg_size = size;
}

void ep_set_hdr_reg_info(unsigned int addr, unsigned int size)
{
	reg_info.reg_base_hdr.reg_start_addr = addr;
	reg_info.reg_base_hdr.reg_size = size;
}

void ep_set_nrm_reg_info(unsigned int addr, unsigned int size)
{
	reg_info.reg_base_nrm.reg_start_addr = addr;
	reg_info.reg_base_nrm.reg_size = size;
}

void ep_set_mnf_reg_info(unsigned int addr, unsigned int size)
{
	reg_info.reg_base_mnf.reg_start_addr = addr;
	reg_info.reg_base_mnf.reg_size = size;
}

void ep_set_fd_reg_info(unsigned int addr, unsigned int size)
{
	reg_info.reg_base_fd.reg_start_addr = addr;
	reg_info.reg_base_fd.reg_size = size;
}

void ep_set_bblt_reg_info(unsigned int addr, unsigned int size)
{
	reg_info.reg_base_bblt.reg_start_addr = addr;
	reg_info.reg_base_bblt.reg_size = size;
}

void ep_set_lvr_reg_info(unsigned int addr, unsigned int size)
{
	reg_info.reg_base_lvr.reg_start_addr = addr;
	reg_info.reg_base_lvr.reg_size = size;
}

void ep_set_dma_reg_info(unsigned int addr, unsigned int size)
{
	reg_info.reg_base_dma.reg_start_addr = addr;
	reg_info.reg_base_dma.reg_size = size;
}

int ep_get_reg_info(struct ep_reg_info *ep_reg_info)
{
	memcpy(ep_reg_info, &reg_info, sizeof(struct ep_reg_info));
	return 0;
}

int ep_set_device_info(struct device *dev)
{
	ep_core_dev = dev;
	return 0;
}


/**
 * @brief   EP TOP의 Core Interrupt마다 할당된 Wait Queue를 초기화 하는 함수
 * @fn      void ep_top_init_wait_queue(void)
 * @param   void
 * @return  void
 * @note	EP 드라이버를 시스템에 등록할때, EP TOP, Interrupt 관련 레지스터를 초기화 할때 사용
 *
 * @author  Wooram Son
 */
void ep_top_init_wait_queue(void)
{
	int i = 0;
	for (i = 0; i < EP_K_MAX_INTR; i++)
		init_completion(&core_irq_wait[i]);

	for (i = 0; i < EP_K_DMA_INTR_MAX; i++)
		init_completion(&dma_irq_wait[i]);
}

/**
 * @brief   EP TOP의 Core Interrupt마다 할당된 특정 Wait Queue를 초기화 하는 함수
 * @fn      void ep_top_init_core_intr_wait_queue(enum ep_k_intr_flags intr)
 * @param   intr	[in]	인터럽트 플래그
 * @return  void
 * @note	특정 Interrupt의 Wait Queue를 초기화 할때 사용
 *
 * @author  Wooram Son
 */
void ep_top_init_core_intr_wait_queue(enum ep_k_intr_flags intr)
{
	if (intr < 0 || intr >= EP_K_MAX_INTR) {
		EP_DD_ERROR_MSG("[EP, %s, %d] Invalid interrupt name!\n", __FILE__, __LINE__);
		return;
	}

	init_completion(&core_irq_wait[intr]);
}

/**
 * @brief   EP TOP의 DMA Interrupt마다 할당된 특정 Wait Queue를 초기화 하는 함수
 * @fn      void ep_top_init_dma_intr_wait_queue(enum ep_k_dma_intr_flags intr)
 * @param   intr	[in]	인터럽트 플래그
 * @return  void
 * @note	특정 Interrupt의 Wait Queue를 초기화 할때 사용
 *
 * @author  Wooram Son
 */
void ep_top_init_dma_intr_wait_queue(enum ep_k_dma_intr_flags intr)
{
	if (intr < 0 || intr >= EP_K_DMA_INTR_MAX) {
		EP_DD_ERROR_MSG("[EP, %s, %d] Invalid interrupt name!\n", __FILE__, __LINE__);
		return;
	}

	init_completion(&dma_irq_wait[intr]);
}

/**
 * @brief   EP TOP의 특정 Core Interrupt가 발생하기를 기다리는 함수
 * @fn      int ep_top_wait_core_intr(enum ep_k_intr_flags intr, int timeout_ms)
 * @param   intr		[in] 인터럽트 종류
 * @param   timeout_ms	[in] Timeout 값 (millisecond 단위)
 * @note	Core Interrupt의 발생여부를 확인하는 방법은
 * 			Interrupt Handler에서 해당 Interrupt의 Wait Queue를 Wake up 함으로 써 알 수 있다.
 * @return  void
 *
 * @author  Wooram Son
 */
int ep_top_wait_core_intr(enum ep_k_intr_flags intr, int timeout_ms)
{
	int wait_time = 0;
	if (intr < 0 || intr >= EP_K_MAX_INTR) {
		EP_DD_ERROR_MSG("[EP, %s, %d] Invalid interrupt name!\n", __FILE__, __LINE__);
		return -1;
	}

	wait_time = wait_for_completion_timeout(&core_irq_wait[intr], msecs_to_jiffies(timeout_ms));
	if (!wait_time) {
		EP_DD_ERROR_MSG("[EP, %s, %d] wait_for_completion_timeout error! (ret: %d)\n", __FILE__, __LINE__, wait_time);
		return -ETIMEDOUT;
	}

	return jiffies_to_msecs(wait_time);
}

/**
 * @brief   EP TOP의 특정 Core Interrupt가 발생하였을때 해당 Wait Queue를 wakeup 해주기 위한 함수
 * @fn      void ep_top_wakeup_core_intr(enum ep_k_intr_flags intr)
 * @param   intr	[in] 인터럽트 종류
 * @note	ISR 혹은 Interrupt Handler에서 사용되며 해당 인터럽트의 Wait Queue를 Wake up 해주는 역할을 한다.
 * @return  void
 *
 * @author  Wooram Son
 */
void ep_top_wakeup_core_intr(enum ep_k_intr_flags intr)
{
	if (intr < 0 || intr >= EP_K_MAX_INTR) {
		EP_DD_ERROR_MSG("[EP, %s, %d] Invalid interrupt name!\n", __FILE__, __LINE__);
		return;
	}

	complete(&core_irq_wait[intr]);
}

/**
 * @brief   EP TOP의 특정 DMA Interrupt가 발생하기를 기다리는 함수
 * @fn      int ep_top_wait_dma_intr(enum ep_k_dma_intr_flags intr, int timeout_ms)
 * @param   intr		[in] 인터럽트 종류
 * @param   timeout_ms	[in] Timeout 값 (millisecond 단위)
 * @note	DMA Interrupt의 발생여부를 확인하는 방법은
 * 			Interrupt Handler에서 해당 Interrupt의 Wait Queue를 Wake up 함으로 써 알 수 있다.
 * @return  void
 *
 * @author  Wooram Son
 */
int ep_top_wait_dma_intr(enum ep_k_dma_intr_flags intr, int timeout_ms)
{
	int wait_time = 0;
	if (intr < 0 || intr >= EP_K_DMA_INTR_MAX) {
		EP_DD_ERROR_MSG("[EP, %s, %d] Invalid interrupt name!\n", __FILE__, __LINE__);
		return -1;
	}

	wait_time = wait_for_completion_timeout(&dma_irq_wait[intr], msecs_to_jiffies(timeout_ms));
	if (!wait_time) {
		EP_DD_ERROR_MSG("[EP, %s, %d] wait_for_completion_timeout error! (ret: %d)\n", __FILE__, __LINE__, wait_time);
		return -ETIMEDOUT;
	}

	return jiffies_to_msecs(wait_time);
}

/**
 * @brief   EP TOP의 특정 DMA Interrupt가 발생하였을때 해당 Wait Queue를 wakeup 해주기 위한 함수
 * @fn      void ep_top_wakeup_dma_intr(enum ep_k_dma_intr_flags intr)
 * @param   intr	[in] 인터럽트 종류
 * @note	ISR 혹은 Interrupt Handler에서 사용되며 해당 인터럽트의 Wait Queue를 Wake up 해주는 역할을 한다.
 * @return  void
 *
 * @author  Wooram Son
 */
void ep_top_wakeup_dma_intr(enum ep_k_dma_intr_flags intr)
{
	if (intr < 0 || intr >= EP_K_DMA_INTR_MAX) {
		EP_DD_ERROR_MSG("[EP, %s, %d] Invalid interrupt name!\n", __FILE__, __LINE__);
		return;
	}

	complete(&dma_irq_wait[intr]);
}








/**
 * @brief EP 각 IP 에 대한 Frame Start Command
 * @fn void ep_top_start_frame(unsigned int value)
 * @param value[in] 설정값
 * @return void
 *
 * @author Wooram Son
 * @note  ep_start_frame 에 Define 된 값으로 value 를 설정한다.
 * 		여러 IP 를 동시에 Start 시킬경우 다음과 같이 Oring 하여 설정할 수 있다.
 * 		ex) ep_top_start_frame(START_FRAME_LDC | START_FRAME_HDR | START_FRAME_MNF)
 */
void ep_k_top_start_frame(unsigned int value)
{
	if (value == (unsigned int) START_K_FRAME_INVALID) {
		EP_DD_ERROR_MSG(" [%s, %d] invalid parameter.\n ", __FILE__, __LINE__);
	}

	__raw_writel(value, (top_addr + 0x40));
}

/**
 * @brief EP 각 IP 에 대한 Tile Start Command
 * @fn void ep_top_start_tile(unsigned int value)
 * @param value[in] 설정값
 * @return void
 *
 * @author Wooram Son
 * @note  ep_start_tile 에 Define 된 값으로 value 를 설정한다.
 * 		여러 IP 를 동시에 Start 시킬경우 다음과 같이 Oring 하여 설정할 수 있다.
 * 		ex) ep_top_start_tile(START_TILE_LDC | START_TILE_HDR | START_TILE_MNF)
 */
void ep_k_top_start_tile(unsigned int value)
{
	if (value == (unsigned int) START_K_TILE_INVAILD) {
		EP_DD_ERROR_MSG(" [%s, %d] invalid parameter.\n ", __FILE__, __LINE__);
	}

	__raw_writel(value, (top_addr + 0x48));
}


/**
 * @brief   사용자가 할당받은 Path에 포함되는 IP들에 Frame Start 신호를 주는 함수
 * @fn      int ep_k_path_start_frame(struct ep_path_desc *path_desc)
 * @param   *path_desc	[in] 이전에 할당받은 Path의 descriptor 구조체
 * @return  int		0: 정상적으로 Start 신호인가, 그 외 값: 오류 발생
 *
 * @author  Wooram Son
 */
int ep_k_path_start_frame(struct ep_path_desc *path_desc)
{
	enum ep_k_path path = EP_K_STATE_INVALID;

	if (path_desc == NULL)
		return -1;

	path = path_desc->path;

	switch (path) {
	case EP_K_STATE_LDC_NRM_MNF_PATH:
		ep_k_top_start_frame(START_K_FRAME_LDC | START_K_FRAME_NRM | START_K_FRAME_MNF);
		break;
	case EP_K_STATE_LDC_NRM_PATH:
		ep_k_top_start_frame(START_K_FRAME_LDC | START_K_FRAME_NRM);
		break;
	case EP_K_STATE_LDC_MNF_PATH:
		ep_k_top_start_frame(START_K_FRAME_LDC | START_K_FRAME_MNF);
		break;
	case EP_K_STATE_LDC_HDR_NRM_PATH:
		ep_k_top_start_frame(START_K_FRAME_LDC | START_K_FRAME_HDR | START_K_FRAME_NRM); /* TODO 새로 추가 */
		break;
	case EP_K_STATE_LDC_HDR_MNF_PATH:
		ep_k_top_start_frame(START_K_FRAME_LDC | START_K_FRAME_HDR | START_K_FRAME_MNF);
		break;
	case EP_K_STATE_LDC_HDR_NRM_MNF_PATH:
		ep_k_top_start_frame(
				START_K_FRAME_LDC | START_K_FRAME_HDR | START_K_FRAME_NRM
						| START_K_FRAME_MNF);
		break;
	case EP_K_STATE_NRM_MNF_PATH:
		ep_k_top_start_frame(START_K_FRAME_NRM | START_K_FRAME_MNF);
		break;
	case EP_K_STATE_NRM_STANDALONE:
		ep_k_top_start_frame(START_K_FRAME_NRM);
		break;
	case EP_K_STATE_MNF_STANDALONE:
		ep_k_top_start_frame(START_K_FRAME_MNF);
		break;
	case EP_K_STATE_FD_STANDALONE:
		ep_k_top_start_frame(START_K_FRAME_FD);
		break;
	case EP_K_STATE_BBLT_STANDALONE:
		ep_k_top_start_frame(START_K_FRAME_BBLT);
		break;
	case EP_K_STATE_LVR_VIDEO_STANDALONE:
		ep_k_top_start_frame(START_K_FRAME_LVR_VIDEO);
		break;
	case EP_K_STATE_LVR_VIDEO_OTFSYNC:
		/* TODO */
		break;
	case EP_K_STATE_LVR_GRP_STANDALONE:
		ep_k_top_start_frame(START_K_FRAME_LVR_GRP);
		break;
	case EP_K_STATE_LVR_VIDEO_GRP_STANDALONE:
		ep_k_top_start_frame(START_K_FRAME_LVR_VIDEO | START_K_FRAME_LVR_GRP);
		break;
	default:
		EP_DD_ERROR_MSG(" [%s, %d] invalid parameter. %d\n ", __FILE__, __LINE__, path);
		break;
	}

	return 0;
}

/**
 * @brief   사용자가 할당받은 Path에 포함되는 IP들에 Tile Start 신호를 주는 함수
 * @fn      int ep_k_path_start_tile(struct ep_path_desc *path_desc)
 * @param   *path_desc	[in] 이전에 할당받은 Path의 descriptor 구조체
 * @return  int		0: 정상적으로 Start 신호인가, 그 외 값: 오류 발생
 *
 * @author  Wooram Son
 */
int ep_k_path_start_tile(struct ep_path_desc *path_desc)
{
	enum ep_k_path path = EP_K_STATE_INVALID;

	if (path_desc == NULL)
		return -1;

	path = path_desc->path;

	switch (path) {
	case EP_K_STATE_LDC_NRM_MNF_PATH:
		ep_k_top_start_tile(START_K_TILE_LDC | START_K_TILE_NRM | START_K_TILE_MNF);
		break;
	case EP_K_STATE_LDC_NRM_PATH:
		ep_k_top_start_tile(START_K_TILE_LDC | START_K_TILE_NRM);
		break;
	case EP_K_STATE_LDC_MNF_PATH:
		ep_k_top_start_tile(START_K_TILE_LDC | START_K_TILE_MNF);
		break;
	case EP_K_STATE_LDC_HDR_NRM_PATH:
		ep_k_top_start_tile(START_K_TILE_LDC | START_K_TILE_HDR | START_K_TILE_NRM);
		break;
	case EP_K_STATE_LDC_HDR_MNF_PATH:
		ep_k_top_start_tile(START_K_TILE_LDC | START_K_TILE_HDR | START_K_TILE_MNF);
		break;
	case EP_K_STATE_LDC_HDR_NRM_MNF_PATH:
		ep_k_top_start_tile(
				START_K_TILE_LDC | START_K_TILE_HDR | START_K_TILE_NRM
						| START_K_TILE_MNF);
		break;
	case EP_K_STATE_NRM_MNF_PATH:
		ep_k_top_start_tile(START_K_TILE_NRM | START_K_TILE_MNF);
		break;
	case EP_K_STATE_NRM_STANDALONE:
		ep_k_top_start_tile(START_K_FRAME_NRM);
		break;
	case EP_K_STATE_MNF_STANDALONE:
		ep_k_top_start_tile(START_K_FRAME_MNF);
		break;
	case EP_K_STATE_FD_STANDALONE:
		ep_k_top_start_tile(START_K_FRAME_FD);
		break;
	case EP_K_STATE_BBLT_STANDALONE:
		ep_k_top_start_tile(START_K_FRAME_BBLT);
		break;
	case EP_K_STATE_LVR_VIDEO_STANDALONE:
		ep_k_top_start_tile(START_K_TILE_LVR_VIDEO);
		break;
	case EP_K_STATE_LVR_VIDEO_OTFSYNC:
		/* TODO */
		break;
	case EP_K_STATE_LVR_GRP_STANDALONE:
		/* TODO */
		break;
	default:
		EP_DD_ERROR_MSG(" [%s, %d] invalid parameter.\n ", __FILE__, __LINE__);
		break;
	}
	return 0;
}

int ep_k_path_start(int path_id, enum ep_k_path_start_type start_type)
{
	int ret = 0;
	struct ep_path_desc *path_desc;

	path_desc = ep_path_get_descriptor(path_id);
	if (path_desc == NULL) {
		EP_DD_ERROR_MSG("[EP, %s, %d] ep_path_get_descriptor() fail\n", __FILE__, __LINE__);
		ret = -1;
		goto func_end;
	}

	if (start_type == EP_K_PATH_TILE_START) {
		ret = ep_k_path_start_tile(path_desc);
		if (ret < 0)
			EP_DD_ERROR_MSG("[EP, %s, %d] ep_path_start_tile fail\n", __FILE__, __LINE__);
	} else if (start_type == EP_K_PATH_FRAME_START) {
		ret = ep_k_path_start_frame(path_desc);
		if (ret < 0)
			EP_DD_ERROR_MSG("[EP, %s, %d] ep_path_start_frame fail\n", __FILE__, __LINE__);
	}

func_end:
	return ret;
}

int ep_k_path_block_start(struct ep_k_path_block_start_info *block_start_info,
		enum ep_k_path_start_type start_type)
{
	int ret = 0;
	struct ep_path_desc *path_desc;

	if (block_start_info == NULL) {
		EP_DD_ERROR_MSG("[EP, %s, %d] parameter error! (block_start_info)\n", __FILE__, __LINE__);
		return -1;
	}

	path_desc = ep_path_get_descriptor(block_start_info->path_id);
	if (path_desc == NULL) {
		EP_DD_ERROR_MSG("[EP, %s, %d] ep_path_get_descriptor() fail\n", __FILE__, __LINE__);
		return ret;
	}

	/* Initialize Core wait queue */
	ep_top_init_core_intr_wait_queue(block_start_info->core_wait_info.intr);

	/* Initialize DMA wait queue */
	if (block_start_info->dma_wait_info.intr >= 0 && block_start_info->dma_wait_info.intr < EP_K_DMA_INTR_MAX) {
		ep_top_init_dma_intr_wait_queue(block_start_info->dma_wait_info.intr);
	}

	/* Start */
	if (start_type == EP_K_PATH_TILE_START && block_start_info->tile_idx == 0) {
		ret = ep_k_path_start(block_start_info->path_id, EP_K_PATH_FRAME_START);
		if (ret < 0) {
			EP_DD_ERROR_MSG("[EP, %s, %d] ep_udd_path_start (EP_PATH_FRAME_START)\n", __FILE__, __LINE__);
			return ret;
		}
	}
	ret = ep_k_path_start(block_start_info->path_id, start_type);
	if (ret < 0) {
		EP_DD_ERROR_MSG("[EP, %s, %d] ep_udd_path_start\n", __FILE__, __LINE__);
		return ret;
	}

	/* Wait for Core interrupt */
	ret = ep_top_wait_core_intr(block_start_info->core_wait_info.intr, block_start_info->core_wait_info.timeout_ms);
	if (ret < 0) {
		EP_DD_ERROR_MSG("[EP, %s, %d] EP_IOCTL_PATH_FRAME_BLOCK_START core intr wait fail\n", __FILE__, __LINE__);
		return ret;
	}

	/* Wait for DMA interrupt */
	if (block_start_info->dma_wait_info.intr >= 0 && block_start_info->dma_wait_info.intr < EP_K_DMA_INTR_MAX) {
		ret = ep_top_wait_dma_intr(block_start_info->dma_wait_info.intr, block_start_info->dma_wait_info.timeout_ms);
		if (ret < 0) {
			EP_DD_ERROR_MSG("[EP, %s, %d] EP_IOCTL_PATH_FRAME_BLOCK_START dma intr wait fail\n", __FILE__, __LINE__);
			return ret;
		}
	}
	return ret;
}







void ep_dd_set_top_reg(void __iomem *addr)
{
	top_addr = addr;
}

void ep_dd_set_dma_reg(void __iomem *addr)
{
	dma_addr = addr;
}

void ep_top_rmu_bus_dma(void)
{
	int i;
	/*DMA start*/
	__raw_writel(0x000000ff, (top_addr + 0x8));
	__raw_writel(0x0000017f, (top_addr + 0xc));
	__raw_writel(0x0000002d, (top_addr + 0x10));
	__raw_writel(0x80, (dma_addr + 0x244));
	__raw_writel(0x80000000, (dma_addr + 0x23C));
	__raw_writel(0x0, (dma_addr + 0x254));
	__raw_writel(0x00010000, (dma_addr + 0x248));

	/* Wait for Err INT */
	for (i = 0; i < 100; i++)
		udelay(50);

}


void ep_dma_reset(void)
{
	/* DMA Reset & INT Clear */
	__raw_writel(0xFFFFFFFF, (top_addr + 0x0));
	__raw_writel(0xFFFFFFFF, (top_addr + 0x28));
}

int ep_pmu_requeset(void)
{
#ifdef CONFIG_PMU_SELECT
	int ret;
	/* DMA Start */
	__raw_writel(0x000000ff, (top_addr + 0x8));
	__raw_writel(0x0000017f, (top_addr + 0xc));
	__raw_writel(0x0000002d, (top_addr + 0x10));

	__raw_writel(0x1, (dma_addr + 0x25C));
	ret = pmu_wait_for_ack(PMU_EP);
	if (ret != 0) {
		return ret;
	}
	return ret;
#else
	return -1;
#endif
}


void ep_pmu_clear(void)
{
	__raw_writel(0x0, (dma_addr + 0x25C));
}

/**
 * @brief	EP 의 PMU(Power Management Unit) 설정을 하는 함수
 * @fn      void ep_pmu_on_off(enum ep_dd_onoff onoff)
 * @param	onoff	[in] On/Off
 * @return	void
 *
 * @author  Wooram Son
 */
#ifdef CONFIG_PMU_SELECT
static enum EP_PMU_STATE ep_pmu_state;
static DEFINE_MUTEX(ep_pmu_mutex);
#endif

void ep_pmu_on_off(enum ep_dd_onoff onoff)
{
#ifdef CONFIG_PMU_SELECT
	int reval;
	struct clk *clock;
	struct d4_rmu_device *rmu;

	mutex_lock(&ep_pmu_mutex);

	if (onoff == EP_DD_ON) {
		if (ep_pmu_state != EP_PMU_INVALID && ep_pmu_state == EP_PMU_ON) {
			mutex_unlock(&ep_pmu_mutex);
			return;
		}

		reval = d4_pmu_check(PMU_EP);
		if (reval != 0) {
			reval = ep_pmu_requeset();
			if (reval)
				goto out;

			rmu = d4_rmu_request();
			if (rmu == NULL)
				return;

			d4_pmu_isoen_set(PMU_EP, PMU_CTRL_ON);
			d4_sw_isp_reset(rmu, RST_EP);

			clock = clk_get(ep_core_dev, "ep");

			if (clock == -2) {
				if (rmu != NULL)
					kfree(rmu);
				return;
			}

			clk_disable(clock);
			clk_put(clock);

			d4_pmu_scpre_set(PMU_EP, PMU_CTRL_ON);
			d4_pmu_scall_set(PMU_EP, PMU_CTRL_ON);
			d4_rmu_release(rmu);

			ep_pmu_state = EP_PMU_ON;
		}
	} else {
		if (ep_pmu_state != EP_PMU_INVALID && ep_pmu_state == EP_PMU_OFF) {
			mutex_unlock(&ep_pmu_mutex);
			return;
		}
		rmu = d4_rmu_request();
		if (rmu == NULL)
			return;

		d4_pmu_scpre_set(PMU_EP, PMU_CTRL_OFF);
		d4_pmu_scall_set(PMU_EP, PMU_CTRL_OFF);
		reval = wait_for_stable(PMU_EP);
		if (reval) {
			d4_rmu_release(rmu);
			d4_pmu_scpre_set(PMU_EP, PMU_CTRL_ON);
			d4_pmu_scall_set(PMU_EP, PMU_CTRL_ON);
			goto out;
		}
		clock = clk_get(ep_core_dev, "ep");

		if (clock == -2) {
			if (rmu != NULL)
				kfree(rmu);
			return;
		}

		d4_sw_isp_reset(rmu, RST_EP);
		udelay(1);

		clk_enable(clock);
		clk_set_rate(clock, 260000000); /* EP Clock freq. */
		d4_pmu_isoen_set(PMU_EP, PMU_CTRL_OFF);

		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_EP);

		/*2~4*/
		d4_pmu_bus_reset(PMU_EP);
		ep_top_rmu_bus_dma();

		d4_sw_isp_reset(rmu, RST_EP);
		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_EP);

		/*2~4*/
		d4_pmu_bus_reset(PMU_EP);
		ep_top_rmu_bus_dma();

		/*DMA Reset*/
		ep_dma_reset();

		/* bus Reset */
		d4_pmu_bus_reset(PMU_EP);


		d4_rmu_release(rmu);

		ep_pmu_state = EP_PMU_OFF;
	}

out:
	mutex_unlock(&ep_pmu_mutex);
#endif
}

int ep_set_clk_rate(int rate)
{
	struct clk *ep_clk_set;

	if (rate < 0 || rate > 260000000) {
		EP_DD_ERROR_MSG("[EP, %s, %d] Invalid clock rate!\n", __FILE__, __LINE__);
		return -1;
	}

	ep_clk_set = clk_get(ep_core_dev, "ep");
	if (ep_clk_set == -2) {
		EP_DD_ERROR_MSG("[EP, %s, %d] clk_get error!\n", __FILE__, __LINE__);
		return -2;
	}

	/* clk_enable(ep_clk_set); */
	clk_set_rate(ep_clk_set, rate);

	return 0;
}
