/**
 * @file d4_csm_ctrl_dd.c
 * @brief DRIMe4 CSM Function File
 * @author Gwon Haesu <haesu.gwon@samsung.com>
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
#include <linux/gpio.h>
#include <linux/hs_spi.h>

#include "d4_csm_if.h"

#include <linux/printk.h>

#include "../pp/pp_ssif/d4_pp_ssif_if.h"
#include "../pp/pp_core/d4_pp_core_if.h"
#include "../pp/pp_core/d4_pp_core_regs.h"


struct hs_spi_data *spi_ctx[6];
struct completion csm_int_completion[7];
struct csm_chg_wait_vd_mode wait_vd_data;
struct csm_chg_read_out_mode read_out_data;
struct csm_wdma_state csm_wdma_error_state;

int curr_cmd;
int flag_read_out;

struct hs_spi_data *hs_spi_request(int hs_spi_id);


/****************************************************************************/
/********************* TEMP : need to modify  -->> ******************************/
/****************************************************************************/
/* PP SSIF */
enum pp_ssif_bit_set{
	SSIF_LOW,
	SSIF_HIGH
};

extern unsigned int pp_ssif_ctrl_reg_base;

#define	PP_SSIF_SWRESET	(*(volatile unsigned int*)pp_ssif_ctrl_reg_base)
#define	SSIF_SWRESET_M	(0x1<<0)
#define	SSIF_SWRESET(x)	(x<<0)
#define	ssif_swreset(x)		(PP_SSIF_SWRESET = (PP_SSIF_SWRESET&(~(SSIF_SWRESET_M)))|SSIF_SWRESET(x))

extern void pp_ssif_send_interrupt(int intr_type);

#if 1
extern void pp_ssif_k_dd_sw_reset(void);

void pp_ssif_sw_reset(void)
{
	pp_ssif_k_dd_sw_reset();
}
#else
void pp_ssif_sw_reset(void)
{
	ssif_swreset(SSIF_HIGH);

	/**< delay for stabilization time */
	ssif_swreset(SSIF_HIGH);

	/* Make S/W Reset to LOW */
	ssif_swreset(SSIF_LOW);
}
#endif

/* PP Core */
enum pp_onoff {
	PP_OFF,
	PP_ON
};

extern unsigned int pp_core_common_reg_base;
extern unsigned int pp_core_dma_reg_base;
extern void pp_core_com_wait_init_wdma(void);

#define WDMA_FRAME_WRITE_DONE_INT	((0x1 << 19) | (0x1 << 21))

inline void write_pp_core_register(unsigned int offset, unsigned int val)
{
	__raw_writel(val, pp_core_common_reg_base + offset);
}

inline void write_pp_core_dma_register(unsigned int offset, unsigned int val)
{
	__raw_writel(val, pp_core_dma_reg_base + offset);
}

inline unsigned int read_pp_core_register(unsigned int offset)
{
	return __raw_readl(pp_core_common_reg_base + offset);
}

inline unsigned int read_pp_core_dma_register(unsigned int offset)
{
	return __raw_readl(pp_core_dma_reg_base + offset);
}

void pp_wdma_start(void)
{
	unsigned int pp_core_int = 0;
	unsigned int pp_vsync_wdma_enable = 0;
	unsigned int wdma_enable = 0;

	/**< Interrupt Clear */
	write_pp_core_register(PP_CORE_COMMON_INT_CLEAR, WDMA_FRAME_WRITE_DONE_INT);

	/**< Interrupt Enable */
	pp_core_int = read_pp_core_register(PP_CORE_COMMON_INT_ENABLE);
	pp_core_int |= WDMA_FRAME_WRITE_DONE_INT; 
	write_pp_core_register(PP_CORE_COMMON_INT_ENABLE, pp_core_int);

	/**< WDMA Enable */
		/**< DMA WDMA Enable */
	wdma_enable = read_pp_core_dma_register(PP_DMA_ENABLE_CTRL);
	D4_PP_DMA_ENABLE_CTRL_WDMA_ENABLE(wdma_enable, PP_ON);
	write_pp_core_dma_register(PP_DMA_ENABLE_CTRL, wdma_enable);

	/**< DMA R/W DMA Disable */
	write_pp_core_dma_register(PP_RW_DMA_ENABLE_CTRL, PP_OFF);

	/**< WDMA initialization for waiting */
	pp_core_com_wait_init_wdma();

	/**< PP WDMA by VSYNC, Enable/ Disable */
	pp_vsync_wdma_enable = read_pp_core_dma_register(PP_WDMA_CTRL);
	D4_PP_WDMA_CTRL_VSYNC_WRITE_ENABLE(pp_vsync_wdma_enable, PP_ON);
	write_pp_core_dma_register(PP_WDMA_CTRL, pp_vsync_wdma_enable);
}

/****************************************************************************/
/********************* <<-- TEMP : need to modify  ******************************/
/****************************************************************************/

void csm_tp_a(int dir)
{
	gpio_set_value(DRIME4_GPIO0(4), dir);
}

void csm_tp_b(int dir)
{
	gpio_set_value(DRIME4_GPIO24(4), dir);
}

void csm_tp_c(int dir)
{
	gpio_set_value(DRIME4_GPIO12(4), dir);
}

void csm_init(void)
{
	int i;

#ifdef CONFIG_MACH_D4_GALAXYNX
	spi_ctx[4] = hs_spi_request(4);
#else
	spi_ctx[1] = hs_spi_request(1);
#endif

	spi_ctx[5] = hs_spi_request(5);

	for (i = 0 ; i < 7 ; i++)
		init_completion(&csm_int_completion[i]);
}

void csm_spi_write(unsigned int addr, unsigned int data, unsigned int ch, unsigned int data_len)
{
	struct spi_data_info spi_rwdata;

	unsigned int spi_cmd;
	unsigned short spi_buffer_short[2] = {0, };
	unsigned int spi_buffer_int[2] = {0, };

	if (data_len == 1) { /* CT3 sensor */
		spi_buffer_short[0] = data;
		spi_buffer_short[1] = addr;

		spi_rwdata.wbuffer = spi_buffer_short;
	} else if (data_len == 2) { /* D4C */
		spi_cmd = ((1 << 16) & 0x00010000) | (addr & 0x0000ffff);

		spi_buffer_int[0] = spi_cmd;
		spi_buffer_int[1] = data;

		spi_rwdata.wbuffer = spi_buffer_int;
	} else {
	}

	spi_rwdata.data_len = data_len;
	spi_rwdata.rbuffer = NULL;

	if (spi_ctx[ch] != NULL)
		hs_spi_polling_write(spi_ctx[ch], &spi_rwdata);
}

struct csm_wdma_state* csm_get_wdma_error_state(void)
{
	return &csm_wdma_error_state;
}

void csm_change_still_wait_vd_mode(void)
{
	int i;

	for (i = 0 ; i < 2 ; i++) {
		csm_spi_write(wait_vd_data.sensor_spi_data[i].addr,
			wait_vd_data.sensor_spi_data[i].data, wait_vd_data.sensor_spi_data[i].ch, 1);
	}
}

void csm_change_still_read_out_mode(void)
{
	int i;

	if (read_out_data.d4c_enable) {
		for (i = 0 ; i < 11 ; i++) {
			csm_spi_write(read_out_data.d4c_spi_data[i].addr,
				read_out_data.d4c_spi_data[i].data, read_out_data.d4c_spi_data[i].ch, 2);
		}
	}

	pp_ssif_sw_reset();
	pp_wdma_start();

	csm_spi_write(read_out_data.sensor_spi_data[0].addr,
			read_out_data.sensor_spi_data[0].data, read_out_data.sensor_spi_data[0].ch, 1);
}

void csm_print_log(int flag)
{
	int saved_console_loglevel;

	/* Make console verbose. */
	saved_console_loglevel = console_loglevel;
	console_loglevel = 2;

	if (flag == 0)
		printk(KERN_DEBUG "Kern set wait vd mode\n");
	else
		printk(KERN_DEBUG "Kern set read out vd mode\n");

	/* Restore console. */
	console_loglevel = saved_console_loglevel;

	if (flag == 0)
		printk(KERN_DEBUG "Kern set wait vd mode\n");
	else
		printk(KERN_DEBUG "Kern set read out vd mode\n");
}

void csm_handle_vsync_isr(int intr_type)
{
	switch (intr_type) {
	case SSIF_DD_INT_VSYNC: /* vsync falling */
		break;
	case SSIF_DD_INT_USER_DEFINE_0: /* vsync rising */
		switch (curr_cmd) {
		case CSM_CMD_CHG_WAIT_VD_MODE:
			csm_change_still_wait_vd_mode();
			break;
		case CSM_CMD_CHG_READ_OUT_MODE:
			if (flag_read_out == 0) {
				flag_read_out = 1;
				csm_change_still_read_out_mode();
			}
			break;
		default:
			break;
		}
		break;
	case SSIF_DD_INT_USER_DEFINE_1:
	case SSIF_DD_INT_USER_DEFINE_2:
	case SSIF_DD_INT_USER_DEFINE_3:
	case SSIF_DD_INT_USER_DEFINE_4:
	case SSIF_DD_INT_USER_DEFINE_5:
	case SSIF_DD_INT_USER_DEFINE_6:
		break;
	default:
		break;
	}

	csm_deregister_vsync_isr(curr_cmd);

	complete(&csm_int_completion[SSIF_DD_INT_USER_DEFINE_0]);
}

void csm_deregister_vsync_isr(enum csm_cmd_list cmd)
{
	switch (cmd) {
	case CSM_CMD_CHG_WAIT_VD_MODE:
		pp_ssif_set_callback_func(SSIF_DD_INT_USER_DEFINE_0, pp_ssif_send_interrupt);
		break;
	case CSM_CMD_CHG_READ_OUT_MODE:
		pp_ssif_set_callback_func(SSIF_DD_INT_USER_DEFINE_0, pp_ssif_send_interrupt);
		break;
	default:
		break;
	}
}

int csm_register_vsync_isr(enum csm_cmd_list cmd, void *data)
{
	curr_cmd = (int)cmd;
	unsigned long result = 0;
	int ret_value = 0;

	switch (cmd) {
	case CSM_CMD_CHG_WAIT_VD_MODE:
		if (data != NULL) {
			pp_ssif_set_callback_func(((struct csm_chg_wait_vd_mode *)data)->ref_vsync, csm_handle_vsync_isr);

			memcpy(&wait_vd_data, (struct csm_chg_wait_vd_mode *)data, sizeof(struct csm_chg_wait_vd_mode));
			init_completion(&csm_int_completion[wait_vd_data.ref_vsync]);

			result = wait_for_completion_timeout(&csm_int_completion[wait_vd_data.ref_vsync], 200);
		} else {
			printk("[%s] csm data is not set!!!\n", __func__);
		}
		break;
	case CSM_CMD_CHG_READ_OUT_MODE:
		if (data != NULL) {
			pp_ssif_set_callback_func(((struct csm_chg_read_out_mode *)data)->ref_vsync, csm_handle_vsync_isr);

			flag_read_out = 0;
			memset(&csm_wdma_error_state, 0, sizeof(struct csm_wdma_state));
			memcpy(&read_out_data, (struct csm_chg_read_out_mode *)data, sizeof(struct csm_chg_read_out_mode));
			init_completion(&csm_int_completion[read_out_data.ref_vsync]);

			result = wait_for_completion_timeout(&csm_int_completion[read_out_data.ref_vsync], 1000);

			mdelay(5);
			csm_spi_write(read_out_data.sensor_spi_data[1].addr,
					read_out_data.sensor_spi_data[1].data, read_out_data.sensor_spi_data[1].ch, 1);

			if ((result <= 0) && (flag_read_out == 0)) {
				flag_read_out = 1;
				csm_deregister_vsync_isr(curr_cmd);
				csm_change_still_read_out_mode();
				ret_value = 1;
				printk("csm time out. [cmd:%d]\n", (int)cmd);
			}
		} else {
			printk("[%s] csm data is not set!!!\n", __func__);
		}
		break;
	default:
		printk("[%s] invalid command\n", __func__);
		break;
	}

	return ret_value;
}

