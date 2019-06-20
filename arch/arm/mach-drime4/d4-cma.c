/**
 * @file d4-cma.c
 * @brief DRIMe4 CMA definitions
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <mach/d4_cma.h>
#include <linux/mutex.h>
#include <linux/slab.h>

struct cma_node {
	struct list_head list;
	dma_addr_t dma_addr;
	void *cpu_addr;
	struct device *dev;
	unsigned int size;
};

static struct list_head allocated_list;

static DEFINE_MUTEX(mutex);

/**
  * @brief 연속적인 메모리 공간을 할당하는 함수
  * @fn dma_addr_t d4_cma_alloc(struct device *dev, size_t size, dma_addr_t alignment)
  * @param *dev[in] 할당을 받고자 하는 device에 대한 정보 <br>
  * @param size[in] 할당을 받고자 하는 크기 <br>
  * @return On error returns a negative error cast to dma_addr_t. <br>
  *         Use IS_ERR_VALUE() to check if returned value is indeed an error. <br>
  *         Otherwise bus address of the chunk is returned. <br>
  *
  * @author Junkwon Choi
  * @note
  */
dma_addr_t d4_cma_alloc(struct device *dev, size_t size)
{
	dma_addr_t phys_addr;
	void *virt_addr;
	struct cma_node *pNode;

	static unsigned int isStart = 1;

	if (isStart) {
		INIT_LIST_HEAD(&allocated_list);
		isStart = 0;
	}

	mutex_lock(&mutex);

	virt_addr = dma_alloc_writecombine(dev, size, &phys_addr, GFP_KERNEL);
	if (virt_addr == NULL) {
		printk("d4_cma_alloc failed. Device is null or invalid argument \n");
		mutex_unlock(&mutex);
		return -EINVAL;
	}

	pNode = (struct cma_node *)kmalloc(sizeof(struct cma_node), GFP_KERNEL);

	if (pNode == NULL)
		return -EINVAL;

	pNode->size = size;
	pNode->cpu_addr = virt_addr;
	pNode->dma_addr = phys_addr;
	pNode->dev = dev;

	/*
	DMPRINTF("dma_addr : 0x%x , cpu_addr : 0x%x, size : 0x%x \n",
		(unsigned int)(pNode->dma_addr), (unsigned int)(pNode->cpu_addr), pNode->size);
	*/

	list_add(&pNode->list, &allocated_list);

	mutex_unlock(&mutex);

	return phys_addr;
}

/**
  * @brief 메모리 공간을 해제하는 함수
  * @fn int d4_cma_free(struct device *dev, dma_addr_t addr)
  * @param *dev[in] 할당을 받고자 하는 device에 대한 정보 <br>
  * @param addr[in] 해제하고자 하는 메모리 영역의 start address <br>
  * @return -ENOENT if there is no chunk at given location; otherwise zero.
  *         In the former case issues a warning.
  *
  * @author Junkwon Choi
  * @note
  */
int d4_cma_free(struct device *dev, dma_addr_t addr)
{
	void *cpu_addr = NULL;
	struct list_head *pList;
	struct cma_node *pNode;
	unsigned int size;
	int ret = 0;

	mutex_lock(&mutex);

	pList = allocated_list.next;

	while (1) {
		if (pList == &allocated_list)
			break;

		pNode = list_entry(pList, struct cma_node, list);

		if (pNode->dma_addr == addr) {
			cpu_addr = pNode->cpu_addr;
			size = pNode->size;
			list_del(&pNode->list);
			kfree(pNode);
			break;
		}
		pList = pList->next;
	}

	if (cpu_addr) {
		dma_free_writecombine(dev, size, cpu_addr, addr);
	} else {
		printk("It's wrong address. cma_free is failed \n");
		ret = -ENOENT;
	}

	mutex_unlock(&mutex);
	return ret;
}

/**
  * @brief 현재까지 할당된 메모리의 총 합을 리턴하는 함
  * @fn unsigned int d4_cma_get_allocated_memory_size(struct device *dev)
  * @param dev[in] 메모리 총 합을 알아보고 싶은 region를 결정하기 위한 device info
  * @return 현재까지 할당된 sma메모리의 총 합
  *
  * @author Junkwon Choi
  * @note
  */
unsigned int d4_cma_get_allocated_memory_size(struct device *dev)
{
	struct list_head *pList;
	struct cma_node *pNode;
	unsigned int size = 0;

	mutex_lock(&mutex);

	pList = allocated_list.next;

	while (1) {
		if (pList == &allocated_list)
			break;

		pNode = list_entry(pList, struct cma_node, list);

		if (pNode->dev == dev)
			size += pNode->size;
		pList = pList->next;
	}

	mutex_unlock(&mutex);
	return size;
}

