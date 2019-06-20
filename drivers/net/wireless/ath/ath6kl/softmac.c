/*
 * Copyright (c) 2011 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "core.h"
#include "debug.h"
#define MAC_FILE "ath6k/AR6003/hw2.1.1/softmac"
#if 1 /* 20120820 Matt Softmac */
#include <linux/random.h>
#include <linux/vmalloc.h>
#endif /* CONFIG_ARCH_S5C7380_BCM4325 */

/* Bleh, same offsets. */
#define AR6003_MAC_ADDRESS_OFFSET 0x16
#define AR6004_MAC_ADDRESS_OFFSET 0x16

/* Global variables, sane coding be damned. */
u8 *ath6kl_softmac;
size_t ath6kl_softmac_len;

#if 1 /* 20120820 Matt Softmac */
#define GET_INODE_FROM_FILEP(filp) \
        (filp)->f_path.dentry->d_inode

int ath6kl_readwrite_file(const char *filename, char *rbuf, const char *wbuf, size_t length)
{
    int ret = 0;
    struct file *filp = (struct file *)-ENOENT;
    mm_segment_t oldfs;
    oldfs = get_fs();
    set_fs(KERNEL_DS);

    do {
        int mode = (wbuf) ? O_RDWR|O_CREAT : O_RDONLY;
        filp = filp_open(filename, mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

        if (IS_ERR(filp) || !filp->f_op) {
            ath6kl_err("%s: file %s filp_open error (file might not be existed)\n", __func__, filename);
            ret = -ENOENT;
            break;
        }

        if (length == 0) {
            /* Read the length of the file only */
            struct inode *inode;

            inode = GET_INODE_FROM_FILEP(filp);
            if (!inode) {
                ath6kl_err("%s: Get inode from %s failed\n", __func__, filename);
                ret = -ENOENT;
                break;
            }
            ret = i_size_read(inode->i_mapping->host);
            break;
        }

        if (wbuf) {
            if ((ret = filp->f_op->write(filp, wbuf, length, &filp->f_pos)) < 0) {
                ath6kl_err("%s: Write %u bytes to file %s error %d\n", __func__, length, filename, ret);
                break;
            }
        } else {
            if ((ret = filp->f_op->read(filp, rbuf, length, &filp->f_pos)) < 0) {
                ath6kl_err("%s: Read %u bytes from file %s error %d\n", __func__, length, filename, ret);
                break;
            }
        }
    } while (0);

    if (!IS_ERR(filp)) {
        filp_close(filp, NULL);
    }
    set_fs(oldfs);

    return ret;
}
#endif /* CONFIG_ARCH_S5C7380_BCM4325 */

static void ath6kl_calculate_crc(u32 target_type, u8 *data, size_t len)
{
	u16 *crc, *data_idx;
	u16 checksum;
	int i;

	if (target_type == TARGET_TYPE_AR6003) {
		crc = (u16 *)(data + 0x04);
	} else if (target_type == TARGET_TYPE_AR6004) {
		len = 1024;
		crc = (u16 *)(data + 0x04);
	} else {
		ath6kl_err("Invalid target type\n");
		return;
	}

	ath6kl_dbg(ATH6KL_DBG_BOOT, "Old Checksum: %u\n", *crc);

	*crc = 0;
	checksum = 0;
	data_idx = (u16 *)data;

	for (i = 0; i < len; i += 2) {
		checksum = checksum ^ (*data_idx);
		data_idx++;
	}

	*crc = cpu_to_le16(checksum);

	ath6kl_dbg(ATH6KL_DBG_BOOT, "New Checksum: %u\n", checksum);
}

#if 0 /* 20120820 Matt Softmac */
static int ath6kl_fetch_mac_file(struct ath6kl *ar)
{
	const struct firmware *fw_entry;
	int ret = 0;


	ret = request_firmware(&fw_entry, MAC_FILE, ar->dev);
	if (ret)
		return ret;

	ath6kl_softmac_len = fw_entry->size;
	ath6kl_softmac = kmemdup(fw_entry->data, fw_entry->size, GFP_KERNEL);

	if (ath6kl_softmac == NULL)
		ret = -ENOMEM;

	release_firmware(fw_entry);

	return ret;
}
#endif /* CONFIG_ARCH_S5C7380_BCM4325 */

void ath6kl_mangle_mac_address(struct ath6kl *ar, u8 locally_administered_bit)
{
	u8 *ptr_mac;
	int i, ret;

	switch (ar->target_type) {
	case TARGET_TYPE_AR6003:
		ptr_mac = ar->fw_board + AR6003_MAC_ADDRESS_OFFSET;
		break;
	case TARGET_TYPE_AR6004:
		ptr_mac = ar->fw_board + AR6004_MAC_ADDRESS_OFFSET;
		break;
	default:
		ath6kl_err("Invalid Target Type\n");
		return;
	}

	ath6kl_dbg(ATH6KL_DBG_BOOT,
		   "MAC from EEPROM %02X:%02X:%02X:%02X:%02X:%02X\n",
		   ptr_mac[0], ptr_mac[1], ptr_mac[2],
		   ptr_mac[3], ptr_mac[4], ptr_mac[5]);


#if 0 /* 20120820 Matt Softmac */
	ret = ath6kl_fetch_mac_file(ar);
	if (ret) {
		ath6kl_err("MAC address file not found\n");
		return;
	}

	for (i = 0; i < ETH_ALEN; ++i) {
		ptr_mac[i] = ath6kl_softmac[i] & 0xff;
	}
#else
	{
#if 1
		char *softmac_filename = "/data/misc/wifi/mac.info";
#else
		char *softmac_filename = "/etc/mac.info";
#endif
		char softmac_temp[64];
		int ismac_file = 0;
		unsigned int softmac[6];

		ismac_file = ath6kl_readwrite_file(softmac_filename, NULL, NULL, 0);

		if (ismac_file >= 17) {
			ret = ath6kl_readwrite_file(softmac_filename, (char *)softmac_temp, NULL, ismac_file);
			ath6kl_err("%s: Read Mac Address on %s - %d\n", __func__, softmac_filename, ret);
		} else {
			sprintf(softmac_temp,
						"20:13:e0:%02x:%02x:%02x\n",
						random32() & 0xff,
						random32() & 0xff,
						random32() & 0xff);
			ath6kl_err("MAC file (%s) is not found. Use random MAC address\n", softmac_filename);
			ret = ath6kl_readwrite_file(softmac_filename, NULL, (char *)softmac_temp, strlen(softmac_temp));
			ath6kl_err("%s: Write Random Mac on %s - %d\n", __func__, softmac_filename, ret);
		}

		ret = ath6kl_readwrite_file(softmac_filename, NULL, NULL, 0);
		if (ret < 0) {
			ath6kl_err("%s: file read error, length %d\n", __func__, ret);
			return;
		} else {
			ath6kl_softmac_len = ret;
		}

		ath6kl_softmac = kmalloc(ath6kl_softmac_len, GFP_KERNEL);
		if (!ath6kl_softmac) {
			ath6kl_err("%s: Cannot allocate buffer for %s (%d)\n", __func__, softmac_filename, ath6kl_softmac_len);
			ret = -ENOMEM;
			return;
		}

		ret = ath6kl_readwrite_file(softmac_filename, (char *)ath6kl_softmac, NULL, ath6kl_softmac_len);
        
		if (sscanf(ath6kl_softmac, "%02x:%02x:%02x:%02x:%02x:%02x",
						&softmac[0], &softmac[1], &softmac[2], &softmac[3], &softmac[4], &softmac[5]) == ETH_ALEN) {

		for (i = 0; i < ETH_ALEN; ++i)
				ptr_mac[i] = (u8)(softmac[i] & 0xff);
		}
	}
#endif /* CONFIG_ARCH_S5C7380_BCM4325 */

	kfree(ath6kl_softmac);

	if (locally_administered_bit)
		ptr_mac[0] |= 0x02;

	ath6kl_calculate_crc(ar->target_type, ar->fw_board, ar->fw_board_len);
}
