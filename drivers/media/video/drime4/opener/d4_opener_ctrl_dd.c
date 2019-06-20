/**
 * @file d4_opener_ctrl_dd.c
 * @brief DRIMe4 OPENER Function File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
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
#include <linux/sched.h>

#include "d4_opener_if.h"

static struct device *opener_dev;

/**
 * @brief 멀티프로세스 환경에서 KDD에게 Locking Mechanism을 제공해주기 위한 Mutex
 */
static struct mutex kdd_open_cnt_mutex[KDD_DEVICES_MAX];
static int kdd_open_cnt[KDD_DEVICES_MAX];
static int kdd_open_pid[KDD_DEVICES_MAX][MAX_KDD_OPEN_CNT];
/* static int kdd_open_pid_idx; */

/**
 * @brief set device info
 * @fn   void d4_opener_set_dev_info(struct device *info)
 * @param struct device *info
 * @return void
 * @author Wooram Son <wooram.son@samsung.com>
 * @note  NONE
 */
void d4_opener_set_dev_info(struct device *info)
{
	opener_dev = info;
}

void d4_opener_init_mutex(void)
{
	int i, j;
	for (i = 0; i < KDD_DEVICES_MAX; i++) {
		mutex_init(&(kdd_open_cnt_mutex[i]));
		kdd_open_cnt[i] = 0;
		for (j = 0; j < MAX_KDD_OPEN_CNT; j++)
			kdd_open_pid[i][j] = 0;
	}
}

enum kdd_open_flag d4_kdd_open(enum kdd_devices dev)
{
	int i;
	enum kdd_open_flag ret = KDD_OPEN_ERROR;

	mutex_lock(&(kdd_open_cnt_mutex[(int)dev]));

	if (kdd_open_cnt[dev] >= MAX_KDD_OPEN_CNT) {
		/**
		 * @brief 최대 오픈 가능한 횟수 초과
		 */
		ret = KDD_OPEN_EXCEED_MAXIMUM_OPEN;
	} else {
		/**
		 * @brief 디바이스 드라이버 오픈 시도
		 */
		if (kdd_open_cnt[dev] == 0)
			ret = KDD_OPEN_FIRST;
		else
			ret = KDD_OPEN;

		kdd_open_cnt[dev]++;

		/**
		 * @brief 드라이버를 오픈한 쓰레드 ID 관리
		 */

		/*
		 kdd_open_pid[dev][kdd_open_pid_idx] = current->tgid;
		 kdd_open_pid_idx = (kdd_open_pid_idx + 1) % MAX_KDD_OPEN_CNT;
		 */

		for (i = 0; i < MAX_KDD_OPEN_CNT; i++)
			if (kdd_open_pid[dev][i] == 0) {
				kdd_open_pid[dev][i] = (current->tgid * KDD_CLOSE_MAGIC)
						+ current->pid;
				break;
			}

		/* printk("The process is \"%s\" (tgid: %i, pid %i) %d(%d)\n", current->comm, current->tgid, current->pid,dev,kdd_open_cnt[dev]); */
	}
	mutex_unlock(&(kdd_open_cnt_mutex[(int)dev]));

	return ret;
}

enum kdd_close_flag d4_kdd_close(enum kdd_devices dev)
{
	int i;
	enum kdd_close_flag ret = KDD_CLOSE_ERROR;

	mutex_lock(&(kdd_open_cnt_mutex[(int)dev]));

	if (kdd_open_cnt[dev] == 1) {
		ret = KDD_CLOSE_FINAL;
	} else if (kdd_open_cnt[dev] == 0) {
		ret = KDD_CLOSE_ERROR;
	} else {
		ret = KDD_CLOSE;
	}

	if (kdd_open_cnt[dev] > 0) {
		kdd_open_cnt[dev]--;

		/**
		 * @brief EP 드라이버를 닫은 쓰레드 ID 제거
		 */

		/*
		 kdd_open_pid[dev][kdd_open_pid_idx] = current->tgid + KDD_CLOSE_MAGIC;
		 kdd_open_pid_idx = (kdd_open_pid_idx + 1) % MAX_KDD_OPEN_CNT;
		 */

		for (i = 0; i < MAX_KDD_OPEN_CNT; i++) {
			if ((kdd_open_pid[dev][i] / KDD_CLOSE_MAGIC) == current->tgid) {
				kdd_open_pid[dev][i] = 0;
				break;
			}
		}
	}

	mutex_unlock(&(kdd_open_cnt_mutex[(int)dev]));

	return ret;
}

int d4_get_kdd_open_count(enum kdd_devices dev)
{
	return kdd_open_cnt[dev];
}

void d4_reset_kdd_open_count(enum kdd_devices dev)
{
	kdd_open_cnt[dev] = 1;	/* TODO: 0으로 리셋하면 close() 호출시 PMU ON (Power Off)를 수행하지 않으므로, 1로 초기화 */
}

void d4_print_kdd_open_pid_list(enum kdd_devices dev)
{
	int i;
	printk("========================\n");
	for (i = 0; i < MAX_KDD_OPEN_CNT; i++)
		if (kdd_open_pid[dev][i] != 0)
			printk("Process ID: %d\n", kdd_open_pid[dev][i]);
	printk("========================\n");
}

int d4_get_kdd_open_pid(struct kdd_open_pid_list *pid_list)
{
	int i;

	if (pid_list == NULL)
		return -1;

	if (pid_list->dev < 0 || pid_list->dev >= KDD_DEVICES_MAX)
		return -2;

	mutex_lock(&(kdd_open_cnt_mutex[pid_list->dev]));

	pid_list->open_cnt = kdd_open_cnt[pid_list->dev];

	for (i = 0; i < MAX_KDD_OPEN_CNT; i++)
		pid_list->kdd_open_pid[i] = kdd_open_pid[pid_list->dev][i];

	mutex_unlock(&(kdd_open_cnt_mutex[pid_list->dev]));

	return 0;
}

