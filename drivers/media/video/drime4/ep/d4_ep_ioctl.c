/**
 * @file d4_ep_ioctl.c
 * @brief DRIMe4 EP IOCTL Interface
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#include "d4_ep_if.h"
#include <media/drime4/ep/d4_ep_ioctl.h>

#ifdef CONFIG_PMU_SELECT
#include <media/drime4/opener/d4_opener_ioctl.h>
#endif

/**
 * @brief EP의 Context를 저장하는 변수 (d4_ep.c 파일에 전역변수로 할당)
 */
extern struct drime4_ep *g_ep;

/**
 * @brief 멀티프로세스 환경에서 UDD에게 Locking Mechanism을 제공해주기 위한 Mutex
 */
static DEFINE_MUTEX(udd_mutex);

/**
 * @brief   EP 디바이스를 사용자가 Open 하였을때 불리우는 함수로 EP 컨텍스트 변수를 전달해주는 역할
 * @fn      static int drime4_ep_open(struct inode *inode, struct file *filp)
 * @param   *inode	[in] inode struct
 * @param	*filp	[in] file struct
 * @return  int
 *
 * @author  Wooram Son
 */
static int drime4_ep_open(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_open_flag ret;
#endif

	struct drime4_ep *ep = g_ep;
	filp->private_data = ep;

#ifdef CONFIG_PMU_SELECT
	ret = d4_kdd_open(KDD_EP);
	switch (ret) {
	case KDD_OPEN_FIRST:
#ifdef CONFIG_PMU_SELECT
/*
		ep_pmu_on_off(EP_DD_OFF);
*/
#endif
		EP_IOCTL_DEBUG_MSG("EP KDD is opened! count: %d\n", d4_get_kdd_open_count(KDD_EP));
		break;
	case KDD_OPEN:
		EP_IOCTL_DEBUG_MSG("EP KDD is already opened! count: %d\n", d4_get_kdd_open_count(KDD_EP));
		break;
	case KDD_OPEN_EXCEED_MAXIMUM_OPEN:
		EP_IOCTL_DEBUG_MSG("EP KDD has exceeded the maximum number of open! count: %d\n", d4_get_kdd_open_count(KDD_EP));
		break;
	default:
		break;
	}
#endif

	return 0;
}

/**
 * @brief   EP 디바이스를 사용자가 close 하였을때 불리우는 함수
 * @fn      static int drime4_ep_release(struct inode *inode, struct file *filp)
 * @param   *inode	[in] inode struct
 * @param	*filp	[in] file struct
 * @return  int
 *
 * @author  Wooram Son
 */
static int drime4_ep_release(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_close_flag ret = d4_kdd_close(KDD_EP);

	switch (ret) {
	case KDD_CLOSE_FINAL:
#ifdef CONFIG_PMU_SELECT
/*
		ep_pmu_on_off(EP_DD_ON);
*/
#endif
		EP_IOCTL_DEBUG_MSG("EP KDD/fd is closed! count: %d\n", d4_get_kdd_open_count(KDD_EP));
		break;
	case KDD_CLOSE:
		EP_IOCTL_DEBUG_MSG("EP KDD is closed! count: %d\n", d4_get_kdd_open_count(KDD_EP));
		break;
	case KDD_CLOSE_ERROR:
	default:
		EP_IOCTL_DEBUG_MSG("EP KDD close is failed! count: %d\n", d4_get_kdd_open_count(KDD_EP));
		break;
	}
#endif
	return 0;
}

/**
 * @brief   EP의 IOCTL 인터페이스 처리 함수
 * @fn      long drime4_ep_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param	*filp	[in] file struct
 * @param   cmd		[in] cmd
 * @param   arg		[in] arg
 * @return  long	커맨트 수행 실패시 음수 값 반환(<0)
 *
 * @author  Wooram Son
 */
long drime4_ep_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int size = 0;
	int err = -1;
	int path_id = 0;
	int clk_rate = 260000000;

	struct ep_k_path_info path_info;
	struct ep_path_desc path_desc;
	struct ep_path_desc *ret_path_desc = NULL;
	struct ep_reg_info ep_reg_info;
	struct ep_k_intr_wait_info wait_info;
	struct ep_k_dma_intr_wait_info dma_wait_info;
	struct ep_k_path_block_start_info block_start_info;

	memset(&path_info, 0x0, sizeof(struct ep_k_path_info));
	memset(&path_desc, 0x0, sizeof(struct ep_path_desc));
	memset(&ep_reg_info, 0x0, sizeof(struct ep_reg_info));
	memset(&wait_info, 0x0, sizeof(struct ep_k_intr_wait_info));
	memset(&dma_wait_info, 0x0, sizeof(struct ep_k_dma_intr_wait_info));
	memset(&block_start_info, 0x0, sizeof(struct ep_k_path_block_start_info));

	if (_IOC_TYPE(cmd) != EP_IOCTL_CMD_MAGIC)
		return -1;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *) arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *) arg, size);

	if (!err) {
		EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] access_ok() error! (ret: %d)\n", __FILE__, __LINE__, err);
		return -1;
	}

	switch (cmd) {
	case EP_IOCTL_GET_PHYS_REG_INFO:
		if (ep_get_reg_info(&ep_reg_info) < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] ep_get_reg_info() error!\n", __FILE__, __LINE__);
			return -EFAULT;
		}

		if (copy_to_user((void *) arg, (const void *) &ep_reg_info, size)) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_to_user() error!\n", __FILE__, __LINE__);
			return -EFAULT;
		}
		break;

	case EP_IOCTL_PATH_INIT:
		ret = ep_path_init();
		break;

	case EP_IOCTL_PATH_TRY_INIT:
		ret = ep_path_try_init();
		break;

	case EP_IOCTL_PATH_OPEN:
		ret = copy_from_user((void *) &path_info, (const void *) arg, size);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_from_user() fail\n", __FILE__, __LINE__);
			return ret;
		}
		ret = ep_path_open(&path_info);
		break;

	case EP_IOCTL_PATH_CLOSE:
		ret = copy_from_user((void *) &path_id, (const void *) arg, size);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_from_user() fail\n", __FILE__, __LINE__);
			return ret;
		}
		ret = ep_path_close(path_id);
		break;

	case EP_IOCTL_PATH_GET_DESCRIPTOR:
		ret = copy_from_user((void *) &path_desc, (const void *) arg, size);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_from_user() fail\n", __FILE__, __LINE__);
			return ret;
		}
		ret_path_desc = ep_path_get_descriptor(path_desc.id);
		if (ret_path_desc == NULL) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] ep_path_get_descriptor() fail\n", __FILE__, __LINE__);
			ret = -1;
		} else {
			ret = copy_to_user((void *) arg, (const void *)ret_path_desc, sizeof(struct ep_path_desc));
			if (ret < 0) {
				EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_to_user fail\n", __FILE__, __LINE__);
				return ret;
			}
		}
		break;

	case EP_IOCTL_PATH_BLOCK_FRAME_START:
		ret = copy_from_user((void *) &block_start_info, (const void *) arg, size);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_from_user() fail\n", __FILE__, __LINE__);
			return ret;
		}
		ret = ep_k_path_block_start(&block_start_info, EP_K_PATH_FRAME_START);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP, %s, %d] EP_IOCTL_PATH_BLOCK_FRAME_START fail\n", __FILE__, __LINE__);
			return ret;
		}
		break;

	case EP_IOCTL_PATH_BLOCK_TILE_START:
		ret = copy_from_user((void *) &block_start_info, (const void *) arg, size);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_from_user() fail\n", __FILE__, __LINE__);
			return ret;
		}
		ret = ep_k_path_block_start(&block_start_info, EP_K_PATH_TILE_START);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP, %s, %d] EP_IOCTL_PATH_BLOCK_FRAME_START fail\n", __FILE__, __LINE__);
			return ret;
		}
		break;

	case EP_IOCTL_INTR_WAIT:
		ret = copy_from_user((void *) &wait_info, (const void *) arg, size);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_from_user() fail\n", __FILE__, __LINE__);
			return ret;
		}

		if (wait_info.timeout_ms < 0 || wait_info.intr < 0 || wait_info.intr
				>= EP_K_MAX_INTR) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] Invalid parameters!\n", __FILE__, __LINE__);
			ret = -EINVAL;
		} else if (filp->f_flags & O_NONBLOCK) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] O_NONBLOCK error!\n", __FILE__, __LINE__);
			ret = -ENOTBLK;
		} else {
			ret = ep_top_wait_core_intr(wait_info.intr, wait_info.timeout_ms);
		}

		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] EP_IOCTL_INTR_WAIT fail, error code: %d\n", __FILE__, __LINE__, ret);
			return ret;
		}
		break;

	case EP_IOCTL_INTR_WAIT_QUEUE_INIT:
		ret = copy_from_user((void *) &wait_info, (const void *) arg, size);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_from_user() fail\n", __FILE__, __LINE__);
			return ret;
		}

		if (wait_info.intr < 0 || wait_info.intr >= EP_K_MAX_INTR) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] Invalid parameters!\n", __FILE__, __LINE__);
			ret = -EINVAL;
		} else if (filp->f_flags & O_NONBLOCK) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] O_NONBLOCK error!\n", __FILE__, __LINE__);
			ret = -ENOTBLK;
		} else
			ep_top_init_core_intr_wait_queue(wait_info.intr);

		break;

	case EP_IOCTL_DMA_INTR_WAIT:
		ret = copy_from_user((void *) &dma_wait_info, (const void *) arg, size);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_from_user() fail\n", __FILE__, __LINE__);
			return ret;
		}

		if (dma_wait_info.timeout_ms < 0 || dma_wait_info.intr < 0
				|| dma_wait_info.intr >= EP_K_DMA_INTR_MAX) {
			ret = -EINVAL;
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] Invalid parameters!\n", __FILE__, __LINE__);
		} else if (filp->f_flags & O_NONBLOCK) {
			ret = -ENOTBLK;
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] O_NONBLOCK error!\n", __FILE__, __LINE__);
		} else {
			ret = ep_top_wait_dma_intr(dma_wait_info.intr,
					dma_wait_info.timeout_ms);
		}

		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] EP_IOCTL_DMA_INTR_WAIT fail\n", __FILE__, __LINE__);
			return ret;
		}
		break;

	case EP_IOCTL_DMA_INTR_WAIT_QUEUE_INIT:
		ret = copy_from_user((void *) &dma_wait_info, (const void *) arg, size);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_from_user() fail\n", __FILE__, __LINE__);
			return ret;
		}

		if (dma_wait_info.intr < 0 || dma_wait_info.intr >= EP_K_DMA_INTR_MAX) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] EP_IOCTL_DMA_INTR_WAIT_QUEUE_INIT ignore\n", __FILE__, __LINE__);
			ret = -EINVAL;
		} else if (filp->f_flags & O_NONBLOCK) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] EP_IOCTL_DMA_INTR_WAIT_QUEUE_INIT O_NONBLOCK\n", __FILE__, __LINE__);
			ret = -ENOTBLK;
		} else
			ep_top_init_dma_intr_wait_queue(dma_wait_info.intr);
		break;

	case EP_IOCTL_UDD_LOCK:
		mutex_lock(&udd_mutex);
		break;

	case EP_IOCTL_UDD_UNLOCK:
		mutex_unlock(&udd_mutex);
		break;

	case EP_IOCTL_SET_CLK_RATE:
		ret = copy_from_user((void *) &clk_rate, (const void *) arg, size);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] copy_from_user() fail\n", __FILE__, __LINE__);
			return ret;
		}
		ret = ep_set_clk_rate(clk_rate);
		if (ret < 0) {
			EP_IOCTL_ERROR_MSG("[EP IOCTL, %s, %d] ep_set_clk_rate() fail (%d)\n", __FILE__, __LINE__, ret);
			return ret;
		}
		break;

	default:
		break;
	}
	return ret;
}

/**
 * @brief cma로 할당된 영역에 대한 application 접근용 virtual address를 생성해주는 함수
 * @fn int d4_ep_mmap(struct file *file, struct vm_area_struct *vma)
 * @param *file[in] driver file discripter
 * @param *vma[in/out] virtual memory map을 구성하기 위한 정보 structure<br>
 * @return On error returns a negative error, zero otherwise. <br>
 *
 * @author Wooram Son
 * @note
 */
int d4_ep_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size = 0;

	size = (vma->vm_end - vma->vm_start);

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
					vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}

/**
 * @brief EP 드라이버의 IOCTL 인터페이스의 file_operations 설정
 */
const struct file_operations drime4_ep_ops = {
		.owner = THIS_MODULE,
		.open = drime4_ep_open,
		.release = drime4_ep_release,
		.unlocked_ioctl = drime4_ep_ioctl,
		.mmap = d4_ep_mmap
};

MODULE_AUTHOR("Wooram Son <wooram.son@samsung.com>");
MODULE_DESCRIPTION("Samsung Drime IV EP Driver");
MODULE_LICENSE("GPL");

