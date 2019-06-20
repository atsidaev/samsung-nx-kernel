/**
 * @file d4_sma_ctrl_dd.c
 * @brief DRIMe4 SMA Function File
 * @author Junkwon Choi <Junkwon.choi@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/list.h>

#include <mach/d4_cma.h>
#include "d4_sma_if.h"

static struct device *sma_dev;

#if defined( SMA_TEMP_RESERVED_AREA )
struct SMA_Reserved_Buffer_Info {
   unsigned int top_addr;
   unsigned int current_addr;
   unsigned int size;
};

struct SMA_Reserved_Buffer_List {
	struct SMA_Buffer_Info   info;
	struct list_head	list;
};

static struct list_head g_res_buffer_list_head;
static struct SMA_Reserved_Buffer_List g_res_buffer_list[10];
static int g_res_buffer_cnt;
static struct SMA_Reserved_Buffer_Info gTemp_remain_buf = { 0, 0 };

int d4_sma_get_remain_buf(struct SMA_Buffer_Info *info);
#endif

/**
 * @brief set device info
 * @fn   void d4_sma_set_dev_info(struct device *info)
 * @param struct device *info
 * @return void
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
void d4_sma_set_dev_info(struct device *info)
{
	sma_dev = info;
}

int d4_sma_alloc_buf(struct SMA_Buffer_Info *info)
{
	if (info == NULL)
		return -1;

	if (info->size == 0)
		return -1;

	info->addr = d4_cma_alloc(sma_dev, info->size);

	if (info->addr == -EINVAL) {
		printk("Device is NULL or Invalid argument\n");
		return -1;
	} else if (info->addr == -ENOMEM) {
		printk("Out of memory\n");
		return -1;
	}
	return 0;
}

int d4_sma_free_buf(unsigned int addr)
{
	int result;
	if (addr) {
#if defined( SMA_TEMP_RESERVED_AREA )
		if ((gTemp_remain_buf.top_addr <= addr) &&
			(addr < gTemp_remain_buf.top_addr + gTemp_remain_buf.size)) {
			struct SMA_Reserved_Buffer_List* res_buffer_list;
			list_for_each_entry(res_buffer_list, &g_res_buffer_list_head, list) {
				if (res_buffer_list->info.addr == addr) {
					list_del(&res_buffer_list->list);
					if(list_empty(&g_res_buffer_list_head)) {
						gTemp_remain_buf.current_addr = gTemp_remain_buf.top_addr;
						g_res_buffer_cnt = 0;
					}
					printk(KERN_EMERG"d4_sma_free_buf : remove remain buf...\n");
					break;
				}
			}
			result = 0;
		}
		else
#endif
		result = d4_cma_free(sma_dev, addr);
	} else
		result = -1;

	return result;
}

unsigned int d4_sma_get_allocated_memory_size(void)
{
	return d4_cma_get_allocated_memory_size(sma_dev);
}

#if defined( SMA_TEMP_RESERVED_AREA )

void d4_sma_set_remain_buf(struct SMA_Buffer_Info *info)
{
	if (info == NULL)
	{
		return ;
	}

	if( info->addr )
	{
		INIT_LIST_HEAD(&g_res_buffer_list_head);
		gTemp_remain_buf.top_addr = info->addr + info->size;
		gTemp_remain_buf.current_addr = gTemp_remain_buf.top_addr;
		gTemp_remain_buf.size = info->size;
		g_res_buffer_cnt = 0;
		printk(KERN_EMERG"SET :: %x, %d\n", gTemp_remain_buf.top_addr, gTemp_remain_buf.size );
	}
	else
	{
		gTemp_remain_buf.top_addr = 0;
		gTemp_remain_buf.current_addr = 0;
		gTemp_remain_buf.size = 0;
		g_res_buffer_cnt = 0;
		printk(KERN_EMERG"CLEAR :: %x, %d\n", gTemp_remain_buf.top_addr, gTemp_remain_buf.size );
	}

}

int d4_sma_get_remain_buf(struct SMA_Buffer_Info *info)
{
	if (info == NULL)
	{
		return -ENOMEM;
	}
	if( info->size <= (gTemp_remain_buf.size - (gTemp_remain_buf.current_addr - gTemp_remain_buf.top_addr))
		&& g_res_buffer_cnt < 10)
	{
		info->addr = gTemp_remain_buf.current_addr;
		gTemp_remain_buf.current_addr += info->size;
		printk("Get addr = %x, size = %d\n", info->addr, info->size);
		
		g_res_buffer_list[g_res_buffer_cnt].info = *info;
		list_add(&g_res_buffer_list[g_res_buffer_cnt++].list, &g_res_buffer_list_head);
		return 0;
	}
	else
	{
		return -ENOMEM;
	}
}
#endif
