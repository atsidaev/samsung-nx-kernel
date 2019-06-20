#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "d4_dp_global_reg.h"
#include "d4_dp_global.h"
#include "../lcd/d4_dp_lcd_dd.h"

extern int dp_lcd_set_grp_display_area(struct stfb_info *info,
		struct stgraphic_display_area *area);
extern void dp_lcd_grp_window_onoff(struct stfb_info *info,
		enum edp_window window, enum edp_onoff on_off);

/**
 * @brief dp lcd clock control function
 * @fn void dp_lcd_clock_on_off(struct stfb_info *info, enum edp_lcd_clock clock,enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param clock[in] select clock
 * @param onoff[in] Clock ON/OFF
 * @return
 *
 * @note
 */
void dp_lcd_clock_on_off(struct stfb_info *info, enum edp_lcd_clock clock,
		enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_clk_en);

	if (clock == MLCD_SYS_CLK_EN)
		D4_DP_COMMON_MLCD_SYS_CLK_EN(reg, onoff);
	else if (clock == MLCD_CLK_EN)
		D4_DP_COMMON_MLCD_CLK_EN(reg, onoff);
	else if (clock == MLCD_CLK_INV)
		D4_DP_COMMON_MLCD_CLK_INV(reg, onoff);
	else if (clock == SLCD_SYS_CLK_EN)
		D4_DP_COMMON_SLCD_SYS_CLK_EN(reg, onoff);
	else if (clock == SLCD_CLK_EN)
		D4_DP_COMMON_SLCD_CLK_EN(reg, onoff);
	else if (clock == SLCD_CLK_INV)
		D4_DP_COMMON_SLCD_CLK_INV(reg, onoff);
	else if (clock == NLC_VID_CLK_EN)
		D4_DP_COMMON_NLC_VID_CLK_EN(reg, onoff);
	else if (clock == NLC_GRP_CLK_EN)
		D4_DP_COMMON_NLC_GRP_CLK_EN(reg, onoff);

	__raw_writel(reg, info->dp_global_regs + dp_clk_en);
}

/**
 * @brief dp register reset control function
 * @fn void dp_reg_reset(struct stfb_info *info, enum edp_reset_clock selection)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] select reset register
 * @return
 *
 * @note
 */
void dp_reg_reset(struct stfb_info *info, enum edp_reset_clock selection)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_rst_ctrl);

	if (selection == TV_Reset) {
		D4_DP_COMMON_RST_TV_SYS(reg, DP_ON);
		D4_DP_COMMON_RST_TV(reg, DP_ON);
		D4_DP_COMMON_RST_TV_SYS(reg, DP_OFF);
		D4_DP_COMMON_RST_TV(reg, DP_OFF);

	} else if (selection == MLCD_Reset) {
		D4_DP_COMMON_RST_MLCD_SYS(reg, DP_ON);
		D4_DP_COMMON_RST_MLCD(reg, DP_ON);
		D4_DP_COMMON_RST_MLCD_SYS(reg, DP_OFF);
		D4_DP_COMMON_RST_MLCD(reg, DP_OFF);

	} else if (selection == SLCD_Reset) {
		D4_DP_COMMON_RST_SLCD_SYS(reg, DP_ON);
		D4_DP_COMMON_RST_SLCD(reg, DP_ON);
		D4_DP_COMMON_RST_SLCD_SYS(reg, DP_OFF);
		D4_DP_COMMON_RST_SLCD(reg, DP_OFF);

	} else if (selection == HDMI_SYS_Reset) {
		D4_DP_COMMON_RST_HDMI_SYS(reg, DP_ON);
		D4_DP_COMMON_RST_HDMI_SYS(reg, DP_OFF);

	} else if (selection == HDMI_Video_Reset) {
		D4_DP_COMMON_RST_HDMI_VIDEO(reg, DP_ON);
		D4_DP_COMMON_RST_HDMI_VIDEO(reg, DP_OFF);

	} else if (selection == HDMI_TMDS_Reset) {
		D4_DP_COMMON_RST_HDMI_TMDS(reg, DP_ON);
		D4_DP_COMMON_RST_HDMI_TMDS(reg, DP_OFF);

	} else if (selection == HDMI_Audio_Reset) {
		D4_DP_COMMON_RST_HDMI_AUDIO(reg, DP_ON);
		D4_DP_COMMON_RST_HDMI_AUDIO(reg, DP_OFF);

	} else if (selection == NLC_Reset) {
		D4_DP_COMMON_RST_NLC_VID(reg, DP_ON);
		D4_DP_COMMON_RST_NLC_GRP(reg, DP_ON);
		D4_DP_COMMON_RST_NLC_VID(reg, DP_OFF);
		D4_DP_COMMON_RST_NLC_GRP(reg, DP_OFF);
	}

	__raw_writel(reg, info->dp_global_regs + dp_rst_ctrl);
}

/**
 * @brief dp operation function
 * @fn void dp_operation_start(struct stfb_info *info, enum edp_path path)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param path[in] select dp path, TV/MLCD/SLCD
 * @return
 *
 * @note
 */
void dp_operation_start(struct stfb_info *info, enum edp_path path)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_start_update);

	if (path == DP_TV)
		D4_DP_COMMON_TV_ON(reg, 1);
	else if (path == DP_MLCD)
		D4_DP_COMMON_MLCD_ON(reg, 1);
	else if (path == DP_SLCD)
		D4_DP_COMMON_SLCD_ON(reg, 1);

	__raw_writel(reg, info->dp_global_regs + dp_start_update);
}

/**
 * @brief dp LCD operation stop function
 * @fn void dp_operation_lcd_stop(struct stfb_info *info, enum edp_path path)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param path[in] select dp path, MLCD/SLCD
 * @return
 *
 * @note
 */
void dp_operation_lcd_stop(struct stfb_info *info, enum edp_path path)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_start_update);

	if (path == DP_MLCD) {
		D4_DP_COMMON_MLCD_ON(reg, DP_OFF);
		__raw_writel(reg, info->dp_global_regs + dp_start_update);

		/* Register Reset */
		dp_reg_reset(info, DP_MLCD);

		/* LCD Clock Off */
		dp_lcd_clock_on_off(info, MLCD_SYS_CLK_EN, DP_OFF);
	} else if (path == DP_SLCD) {
		D4_DP_COMMON_SLCD_ON(reg, DP_OFF);
		__raw_writel(reg, info->dp_global_regs + dp_start_update);

		/* Register Reset */
		dp_reg_reset(info, DP_SLCD);

		/* LCD Clock Off*/
		dp_lcd_clock_on_off(info, SLCD_SYS_CLK_EN, DP_OFF);
	}
}

/**
 * @brief dp register operation update
 * @fn void dp_reg_update(struct stfb_info *info, enum edp_path path, enum edp_onoff automatic, enum edp_onoff manual)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param path[in] select dp path, TV/MLCD/SLCD
 * @return
 *
 * @note
 */
void dp_reg_update(struct stfb_info *info, enum edp_path path,
		enum edp_onoff automatic, enum edp_onoff manual)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_start_update);

	if (path == DP_TV) {
		D4_DP_COMMON_TV_AUTO(reg, automatic);
		D4_DP_COMMON_TV_MANUAL(reg, manual);
	} else if (path == DP_MLCD) {
		D4_DP_COMMON_MLCD_AUTO(reg, automatic);
		D4_DP_COMMON_MLCD_MANUAL(reg, manual);
	} else if (path == DP_SLCD) {
		D4_DP_COMMON_SLCD_AUTO(reg, automatic);
		D4_DP_COMMON_SLCD_MANUAL(reg, manual);
	}

	__raw_writel(reg, info->dp_global_regs + dp_start_update);
}

/**
 * @brief dp Interrupt on/off
 * @fn void dp_interrupt_on_off(struct stfb_info *info, enum edp_interrupt type,enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param type[in] select interrupt type
 * @param onoff[in] interrupt on/off
 * @return
 *
 * @note
 */
void dp_interrupt_on_off(struct stfb_info *info, enum edp_interrupt type,
		enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_int_on);

	if (type == INT_TV)
		D4_DP_COMMON_TV_INT_ON(reg, onoff);
	else if (type == INT_MLCD)
		D4_DP_COMMON_MLCD_INT_ON(reg, onoff);
	else if (type == INT_SLCD)
		D4_DP_COMMON_SLCD_INT_ON(reg, onoff);
	else if (type == INT_PERIODIC_ON)
		D4_DP_COMMON_PERIODIC_INT_ON(reg, onoff);
	else if (type == INT_PERIODIC_RST)
		D4_DP_COMMON_PERIODIC_INT_RST(reg, onoff);

	__raw_writel(reg, info->dp_global_regs + dp_int_on);
}

/**
 * @brief dp Interrupt interval,
 * @fn void dp_set_interrupt_interval(struct stfb_info *info, enum edp_path path, enum edp_isr_interval interval)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param path[in] TV/MLCD/TV
 * @param interval[in] frame/field
 * @return
 *
 * @note
 */
void dp_set_interrupt_interval(struct stfb_info *info, enum edp_path path,
		enum edp_isr_interval interval)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_int_on);

	if (path == DP_TV)
		D4_DP_COMMON_TV_INT_FIELD(reg, interval);
	else if (path == DP_MLCD)
		D4_DP_COMMON_MLCD_INT_FIELD(reg, interval);
	else
		D4_DP_COMMON_SLCD_INT_FIELD(reg, interval);

	__raw_writel(reg, info->dp_global_regs + dp_int_on);

}

/**
 * @brief dp video nlc decoder on/off
 * @fn static void dp_nlc_vid_onoff(struct stfb_info *info, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param onoff[in] video nlc on/off
 * @return
 *
 * @note
 */
static void dp_nlc_vid_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_nlc_ctrl);
	D4_DP_COMMON_NLC_VID_ON(reg, onoff);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_ctrl);
}

/**
 * @brief dp graphic nlc decoder on/off
 * @fn static void dp_nlc_grp_onoff(struct stfb_info *info, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param onoff[in] graphic nlc on/off
 * @return
 *
 * @note
 */
static void dp_nlc_grp_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_nlc_ctrl);
	D4_DP_COMMON_NLC_GRP_ON(reg, onoff);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_ctrl);
}

/**
 * @brief dp main LCD nlc select
 * @fn static void dp_nlc_mlcd_select(struct stfb_info *info, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param onoff[in] Main LCD nlc on/off
 * @return
 *
 * @note
 */
static void dp_nlc_mlcd_select(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_nlc_ctrl);
	D4_DP_COMMON_NLC_MLCD_SEL(reg, onoff);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_ctrl);
}

/**
 * @brief dp Sub LCD nlc select
 * @fn static void dp_nlc_slcd_select(struct stfb_info *info, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param onoff[in] Sub LCD nlc on/off
 * @return
 *
 * @note
 */
static void dp_nlc_slcd_select(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_nlc_ctrl);
	D4_DP_COMMON_NLC_SLCD_SEL(reg, onoff);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_ctrl);
}

/**
 * @brief dp video nlc reset
 * @fn void dp_nlc_vid_control_reset(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_nlc_vid_control_reset(struct stfb_info *info)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_nlc_burst_len);
	D4_DP_COMMON_NLC_SW_RESET(reg, 1);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_burst_len);
	mdelay(100);
}

/**
 * @brief dp graphic nlc reset
 * @fn void dp_nlc_grp_control_reset(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_nlc_grp_control_reset(struct stfb_info *info)
{
	unsigned int reg0 = 0, reg1 = 0;
	reg0 = __raw_readl(info->dp_global_regs + DP_NLC_BURST_LEN_GRP0);
	reg1 = __raw_readl(info->dp_global_regs + DP_NLC_BURST_LEN_GRP1);

	D4_DP_COMMON_NLC_SW_RESET(reg0, 1);
	D4_DP_COMMON_NLC_SW_RESET(reg1, 1);

	__raw_writel(reg0, info->dp_global_regs + DP_NLC_BURST_LEN_GRP0);
	__raw_writel(reg1, info->dp_global_regs + DP_NLC_BURST_LEN_GRP1);
	mdelay(100);
}

/**
 * @brief select dp nlc path and video/graphic nlc on/off
 * @fn void dp_nlc_control_set(struct stfb_info *info, enum edp_path path, enum edpnlc onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param path[in] TV/Main LCD /Sub LCD
 * @param onoff[in] video /graphic nlc on/off
 * @return
 *
 * @note
 */
void dp_nlc_control_set(struct stfb_info *info, enum edp_path path,
		enum edpnlc onoff)
{
	if (path == DP_TV) {
		dp_nlc_mlcd_select(info, DP_OFF);
		dp_nlc_slcd_select(info, DP_OFF);
	} else if (path == DP_MLCD) {
		dp_nlc_mlcd_select(info, DP_ON);
		dp_nlc_slcd_select(info, DP_OFF);
	} else {
		dp_nlc_mlcd_select(info, DP_OFF);
		dp_nlc_slcd_select(info, DP_ON);
	}

	if (onoff == NLC_VID_ON)
		dp_nlc_vid_onoff(info, DP_ON);
	else if (onoff == NLC_GRP_ON)
		dp_nlc_grp_onoff(info, DP_ON);
	else {
		dp_nlc_vid_onoff(info, DP_OFF);
		dp_nlc_grp_onoff(info, DP_OFF);
	}

	mdelay(100);

}

/**
 * @brief dp nlc, tile address offset set function
 * @fn static void dp_nlc_vid_addressoffset(struct stfb_info *info, unsigned int y_height_tileSize, unsigned int c_height_tileSize)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param y_height_tileSize[in] y tile height
 * @param c_height_tileSize[in] c tile height
 * @return
 *
 * @note
 */
static void dp_nlc_vid_addressoffset(struct stfb_info *info,
		unsigned int y_height_tileSize, unsigned int c_height_tileSize)
{
	unsigned int reg = 0;
	D4_DP_COMMON_NLC_ADDR_OFFSET(reg, y_height_tileSize);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_y_addr_offset);

	D4_DP_COMMON_NLC_ADDR_OFFSET(reg, c_height_tileSize);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_c_addr_offset);
}

/**
 * @brief dp video nlc init
 * @fn void dp_nlc_vid_init(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_nlc_vid_init(struct stfb_info *info)
{
	unsigned int reg = 0, reg1 = 0, reg2 = 0;

	dp_nlc_vid_control_reset(info);

	reg = __raw_readl(info->dp_global_regs + dp_nlc_buffer_check);
	D4_DP_COMMON_NLC_SBLEV_TH(reg, 20);
	D4_DP_COMMON_NLC_BUFCHK_TH(reg, 8);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_buffer_check);

	reg = __raw_readl(info->dp_global_regs + dp_nlc_rate_ctrl);
	D4_DP_COMMON_NLC_SBLEV_TH(reg, 20);
	D4_DP_COMMON_NLC_BUFCHK_TH(reg, 8);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_rate_ctrl);

	reg1 = __raw_readl(info->dp_global_regs + dp_nlc_burst_len);
	D4_DP_COMMON_NLC_SW_RESET(reg1, 0);
	D4_DP_COMMON_NLC_BURSTLEN(reg1, 15);
	D4_DP_COMMON_NLC_ENDIAN(reg1, 0);
	__raw_writel(reg1, info->dp_global_regs + dp_nlc_burst_len);

	reg2 = __raw_readl(info->dp_global_regs + dp_nlc_fmt);
	D4_DP_COMMON_NLC_FMT(reg2, 8);
	__raw_writel(reg2, info->dp_global_regs + dp_nlc_fmt);

}

/**
 * @brief dp graphic nlc init
 * @fn void dp_nlc_grp_init(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_nlc_grp_init(struct stfb_info *info)
{
	unsigned int reg = 0, reg1 = 0, reg2 = 0, reg3 = 0, reg4 = 0;

	dp_nlc_grp_control_reset(info);

	reg = __raw_readl(info->dp_global_regs + dp_nlc_buffer_check_grp0);
	D4_DP_COMMON_NLC_SBLEV_TH(reg, 20);
	D4_DP_COMMON_NLC_BUFCHK_TH(reg, 8);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_buffer_check_grp0);

	reg = __raw_readl(info->dp_global_regs + DP_NLC_BUFFER_CHECK_GRP1);
	D4_DP_COMMON_NLC_SBLEV_TH(reg, 20);
	D4_DP_COMMON_NLC_BUFCHK_TH(reg, 8);
	__raw_writel(reg, info->dp_global_regs + DP_NLC_BUFFER_CHECK_GRP1);

	reg = __raw_readl(info->dp_global_regs + DP_NLC_RATE_CTRL_GRP0);
	D4_DP_COMMON_NLC_SBLEV_TH(reg, 20);
	D4_DP_COMMON_NLC_BUFCHK_TH(reg, 8);
	__raw_writel(reg, info->dp_global_regs + DP_NLC_RATE_CTRL_GRP0);

	reg = __raw_readl(info->dp_global_regs + DP_NLC_RATE_CTRL_GRP1);
	D4_DP_COMMON_NLC_SBLEV_TH(reg, 20);
	D4_DP_COMMON_NLC_BUFCHK_TH(reg, 8);
	__raw_writel(reg, info->dp_global_regs + DP_NLC_RATE_CTRL_GRP1);

	reg1 = __raw_readl(info->dp_global_regs + DP_NLC_BURST_LEN_GRP0);
	D4_DP_COMMON_NLC_SW_RESET(reg1, 0);
	D4_DP_COMMON_NLC_BURSTLEN(reg1, 15);
	D4_DP_COMMON_NLC_ENDIAN(reg1, 0);
	__raw_writel(reg1, info->dp_global_regs + DP_NLC_BURST_LEN_GRP0);

	reg2 = __raw_readl(info->dp_global_regs + DP_NLC_BURST_LEN_GRP1);
	D4_DP_COMMON_NLC_SW_RESET(reg2, 0);
	D4_DP_COMMON_NLC_BURSTLEN(reg2, 15);
	D4_DP_COMMON_NLC_ENDIAN(reg2, 0);
	__raw_writel(reg2, info->dp_global_regs + DP_NLC_BURST_LEN_GRP1);

	reg3 = __raw_readl(info->dp_global_regs + DP_NLC_FMT_GRP0);
	D4_DP_COMMON_NLC_FMT(reg3, 8);
	__raw_writel(reg3, info->dp_global_regs + DP_NLC_FMT_GRP0);

	reg4 = __raw_readl(info->dp_global_regs + DP_NLC_FMT_GRP1);
	D4_DP_COMMON_NLC_FMT(reg4, 8);
	__raw_writel(reg4, info->dp_global_regs + DP_NLC_FMT_GRP1);

}

/**
 * @brief dp video nlc inpu image size set
 * @fn static void dp_nlc_vid_input_image_size(struct stfb_info *info,unsigned int y_Height_Size, unsigned int c_Height_Size)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param y_Height_Size[in] y height
 * @param c_Height_Size[in] c height
 * @return
 *
 * @note
 */
static void dp_nlc_vid_input_image_size(struct stfb_info *info,
		unsigned int y_Height_Size, unsigned int c_Height_Size)
{
	unsigned int y_Size = 0, c_Size = 0;
	y_Size = y_Height_Size * 128;
	c_Size = c_Height_Size * 128;
	dp_nlc_vid_addressoffset(info, y_Size, c_Size);
}

/**
 * @brief dp video nlc image address set
 * @fn static void dp_nlc_vid_image_address_set(struct stfb_info *info, struct stnlc_address address)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param address[in] stnlc_address reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_nlc_vid_image_address_set(struct stfb_info *info,
		struct stnlc_address address)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_nlc_y_start_addr);
	D4_DP_COMMON_NLC_ADDRESS(reg, address.Addr_Y0);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_y_start_addr);

	reg = __raw_readl(info->dp_global_regs + dp_nlc_c_start_addr);
	D4_DP_COMMON_NLC_ADDRESS(reg, address.Addr_C0);
	__raw_writel(reg, info->dp_global_regs + dp_nlc_c_start_addr);

}

/**
 * @brief dp video nlc image address set
 * @fn static void dp_nlc_grp_image_address_set(struct stfb_info *info, struct stnlc_grp_wrap grp_wrap, unsigned int y_Height_Size,
 unsigned short tilenum)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param grp_wrap[in] stnlc_grp_wrap reference linux/include/vide/drime4/d4_dp_type.h
 * @param y_Height_Size[in] Y height
 * @param tilenum[in] tile conunt
 * @return
 *
 * @note
 */
static void dp_nlc_grp_image_address_set(struct stfb_info *info,
		struct stnlc_grp_wrap grp_wrap, unsigned int y_Height_Size,
		unsigned short tilenum)
{

	unsigned int height_Size = y_Height_Size * 128;
	unsigned int reg = 0, reg1 = 0, reg2 = 0, reg3 = 0, reg4 = 0, reg5 = 0, reg6 = 0;

	reg = __raw_readl(info->dp_global_regs + DP_NLC_Y_ADDR_OFFSET_GRP1);
	D4_DP_COMMON_NLC_ADDR_OFFSET(reg, height_Size);
	__raw_writel(reg, info->dp_global_regs + DP_NLC_Y_ADDR_OFFSET_GRP1);

	reg = __raw_readl(info->dp_global_regs + DP_NLC_C_ADDR_OFFSET_GRP1);
	D4_DP_COMMON_NLC_ADDR_OFFSET(reg, height_Size);
	__raw_writel(reg, info->dp_global_regs + DP_NLC_C_ADDR_OFFSET_GRP1);

	reg1 = __raw_readl(info->dp_global_regs + dp_nlc_y_addr_offset_grp0);
	D4_DP_COMMON_NLC_ADDR_OFFSET(reg1, height_Size);
	__raw_writel(reg1, info->dp_global_regs + dp_nlc_y_addr_offset_grp0);

	reg2 = __raw_readl(info->dp_global_regs + DP_NLC_C_ADDR_OFFSET_GRP0);
	D4_DP_COMMON_NLC_ADDR_OFFSET(reg2, height_Size);
	__raw_writel(reg2, info->dp_global_regs + DP_NLC_C_ADDR_OFFSET_GRP0);

	reg6 = grp_wrap.e_address;
	__raw_writel(reg6, info->dp_global_regs + dp_nlc_y_start_addr_grp0);

	reg3 = grp_wrap.o_address;
	__raw_writel(reg3, info->dp_global_regs + DP_NLC_C_START_ADDR_GRP0);

	reg4 = grp_wrap.e_address + height_Size * (tilenum + 1);
	__raw_writel(reg4, info->dp_global_regs + DP_NLC_Y_START_ADDR_GRP1);

	reg5 = grp_wrap.o_address + height_Size * (tilenum + 1);
	__raw_writel(reg5, info->dp_global_regs + DP_NLC_C_START_ADDR_GRP1);

}

/**
 * @brief dp graphic wraper
 * @fn void dp_nlc_argb_wraper_set(struct stfb_info *info, struct stnlc_grp_wrap wrapset)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param wrapset[in] stnlc_grp_wrap reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_nlc_argb_wraper_set(struct stfb_info *info,
		struct stnlc_grp_wrap wrapset)
{
	unsigned int reg = 0;
	reg = (wrapset.tile_size[0] & 0x1ff) | (wrapset.tile_size[1] << 16
			& 0x1ff0000);
	/* 1st && 2st tile size*/
	__raw_writel(reg, info->dp_global_regs + DP_NLC_TILE_WIDTH_GROUP0);

	reg = (wrapset.tile_size[2] & 0x1ff) | (wrapset.tile_size[3] << 16
			& 0x1ff0000);
	/* 3,4 tile size*/
	__raw_writel(reg, info->dp_global_regs + DP_NLC_TILE_WIDTH_GROUP1);

	/* 4,5  tile size*/
	reg = (wrapset.tile_size[4] & 0x1ff) | (wrapset.tile_size[5] << 16
			& 0x1ff0000);
	__raw_writel(reg, info->dp_global_regs + DP_NLC_TILE_WIDTH_GROUP2);
}

/**
 * @brief dp graphic nlc set
 * @fn void dp_nlc_grp_set(struct stfb_info *info, struct stnlc_graphic nlcset)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param nlcset[in] stnlc_graphic reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_nlc_grp_set(struct stfb_info *info, struct stnlc_graphic nlcset)
{

	unsigned int reg0 = 0, reg1 = 0, reg2 = 0, reg3 = 0;
	unsigned int tilenum0 = 0, tilenum1 = 0, tilenum3 = 0;

	tilenum3 = (nlcset.grp_wrap.tile_num - 1) * 4
			+ (nlcset.grp_wrap.tile_size[nlcset.grp_wrap.tile_num - 1] + 63)
					/ 64;

	if (tilenum3 < 4)
		tilenum3 = 4;

	tilenum0 = tilenum3 / 2 - 1;
	tilenum1 = tilenum3 - (tilenum0 + 1) - 1;

	D4_DP_COMMON_NLC_TILE_HEIGHT(reg0, nlcset.display.V_Size);
	D4_DP_COMMON_NLC_TILE_NUM(reg0, tilenum0);
	D4_DP_COMMON_NLC_MAX_BUFFER(reg0, 28);

	D4_DP_COMMON_NLC_FMT(reg2, 0xf);

	D4_DP_COMMON_NLC_TILE_HEIGHT(reg1, nlcset.display.V_Size);
	D4_DP_COMMON_NLC_TILE_NUM(reg1, tilenum0);
	D4_DP_COMMON_NLC_MAX_BUFFER(reg1, 28);

	D4_DP_COMMON_NLC_FMT(reg3, 0xf);
	D4_DP_COMMON_NLC_DISPLAY_H_SIZE(reg3, 0);
	D4_DP_COMMON_NLC_YUV_420(reg1, 0);

	__raw_writel(reg0, info->dp_global_regs + dp_nlc_y_tile_heigh_grp0);
	__raw_writel(reg1, info->dp_global_regs + dp_nlc_c_tile_heigh_grp0);

	reg0 = 0;
	D4_DP_COMMON_NLC_TILE_HEIGHT(reg0, nlcset.display.V_Size);
	D4_DP_COMMON_NLC_TILE_NUM(reg0, tilenum1);
	D4_DP_COMMON_NLC_MAX_BUFFER(reg0, 28);

	reg1 = 0;
	D4_DP_COMMON_NLC_TILE_HEIGHT(reg1, nlcset.display.V_Size);
	D4_DP_COMMON_NLC_YUV_420(reg1, 0);
	D4_DP_COMMON_NLC_TILE_NUM(reg1, tilenum1);
	D4_DP_COMMON_NLC_MAX_BUFFER(reg1, 28);

	__raw_writel(reg0, info->dp_global_regs + DP_NLC_Y_TILE_HEIGH_GRP1);
	__raw_writel(reg1, info->dp_global_regs + DP_NLC_C_TILE_HEIGH_GRP1);

	__raw_writel(reg2, info->dp_global_regs + DP_NLC_FMT_GRP0);
	__raw_writel(reg3, info->dp_global_regs + DP_NLC_FMT_GRP1);

	dp_nlc_grp_image_address_set(info, nlcset.grp_wrap, nlcset.image_height,
			tilenum0);
	dp_nlc_argb_wraper_set(info, nlcset.grp_wrap);
	__raw_writel(nlcset.display.H_Size, info->dp_global_regs
			+ DP_NLC_TILE_WIDTH_GROUP3);

}

/**
 * @brief dp video nlc set
 * @fn int dp_nlc_vid_set(struct stfb_info *info, struct sttnlc_video nlcSet)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param nlcset[in] sttnlc_video reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
int dp_nlc_vid_set(struct stfb_info *info, struct sttnlc_video nlcSet)
{
	unsigned int reg0 = 0, reg1 = 0, reg2 = 0;
	unsigned char tilenum = 0;
	struct stdp_display_area display;

	display.H_Start = nlcSet.display.H_Start;
	display.H_Size = nlcSet.display.H_Size;
	display.V_Start = nlcSet.display.V_Start;
	display.V_Size = nlcSet.display.V_Size;

	if (nlcSet.path == DP_TV)
		;/* TV Display area set */
	else if (nlcSet.path == DP_MLCD)
		dp_lcd_set_vid_display_area(info, DP_WIN0, _8bit, &display);

	reg0 = __raw_readl(info->dp_global_regs + dp_nlc_y_tile_heigh);
	reg2 = __raw_readl(info->dp_global_regs + dp_nlc_fmt);

	tilenum = (nlcSet.display.H_Size + 127) / 128 - 1;

	D4_DP_COMMON_NLC_TILE_NUM(reg0, tilenum);
	D4_DP_COMMON_NLC_MAX_BUFFER(reg0, 28);

	if (nlcSet.display.H_Size > nlcSet.inputImage_width) {
		printk("%s, Display H Size Set Outrange \n", __FUNCTION__);
		return -1;
	}

	if (nlcSet.display.V_Size > nlcSet.inputImage_height) {
		printk("%s, Display V Size Set Outrange \n", __FUNCTION__);
		return -1;
	}

	if (nlcSet.path == DP_TV) {
		/*// tv_set
		 DP_TV_Mode_Get(&dis_mode);

		 if(dis_mode==RES_1080I_60 ||dis_mode==RES_1080I_50 )
		 {
		 fmt->bit.interlace=1;
		 comp0->bit.height =nlcSet.display.V_Size*2;
		 }
		 else
		 {
		 fmt->bit.interlace=0;
		 comp0->bit.height =nlcSet.display.V_Size;
		 }
		 */
	} else {

		D4_DP_COMMON_NLC_TILE_HEIGHT(reg0, nlcSet.display.V_Size);

	}

	if (nlcSet.format == Ycbcr_420) {
		/*  //tv_set
		 if(dis_mode==RES_1080I_60 ||dis_mode==RES_1080I_50 )
		 comp1->bit.height = nlcSet.display.V_Size;
		 else
		 */
		D4_DP_COMMON_NLC_TILE_HEIGHT(reg1, nlcSet.display.V_Size / 2);
		D4_DP_COMMON_NLC_YUV_420(reg1, 1);
		D4_DP_COMMON_NLC_TILE_NUM(reg1, tilenum);
		D4_DP_COMMON_NLC_MAX_BUFFER(reg1, 28);
		D4_DP_COMMON_NLC_DISPLAY_H_SIZE(reg2, nlcSet.display.H_Size);

		__raw_writel(reg0, info->dp_global_regs + dp_nlc_y_tile_heigh);
		__raw_writel(reg1, info->dp_global_regs + dp_nlc_c_tile_heigh);
		__raw_writel(reg2, info->dp_global_regs + dp_nlc_fmt);

		dp_nlc_vid_input_image_size(info, nlcSet.inputImage_height,
				nlcSet.inputImage_height / 2);
	} else {
		/*// tv_set
		 if(dis_mode == RES_1080I_60 || dis_mode == RES_1080I_50)
		 comp1->bit.height = nlcSet.display.V_Size*2;
		 else
		 */
		D4_DP_COMMON_NLC_TILE_HEIGHT(reg1, nlcSet.display.V_Size);
		D4_DP_COMMON_NLC_YUV_420(reg1, 0);
		D4_DP_COMMON_NLC_TILE_NUM(reg1, tilenum);
		D4_DP_COMMON_NLC_MAX_BUFFER(reg1, 28);
		D4_DP_COMMON_NLC_DISPLAY_H_SIZE(reg2, nlcSet.display.H_Size);

		__raw_writel(reg0, info->dp_global_regs + dp_nlc_y_tile_heigh);
		__raw_writel(reg1, info->dp_global_regs + dp_nlc_c_tile_heigh);
		__raw_writel(reg2, info->dp_global_regs + dp_nlc_fmt);

		dp_nlc_vid_input_image_size(info, nlcSet.inputImage_height,
				nlcSet.inputImage_height);
	}

	dp_nlc_vid_image_address_set(info, nlcSet.address);

	return 0;
}

/**
 * @brief get dp interrupt status function
 * @fn enum edp_onoff dp_get_interrupt_status(struct stfb_info *info, enum edp_interrupt type)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param type[in] interrupt type
 * @return
 *
 * @note
 */
enum edp_onoff dp_get_interrupt_status(struct stfb_info *info,
		enum edp_interrupt type)
{
	unsigned int reg = 0;
	enum edp_onoff status_result = DP_OFF;

	reg = __raw_readl(info->dp_global_regs + dp_int_set_clr);

	if (type == INT_TV)
		status_result = (reg & 0x01) ? DP_ON : DP_OFF;
	else if (type == INT_MLCD)
		status_result = (reg & 0x10) ? DP_ON : DP_OFF;
	else if (type == INT_SLCD)
		status_result = (reg & 0x100) ? DP_ON : DP_OFF;
	else if (type == INT_PERIODIC_ON)
		status_result = (reg & 0x1000) ? DP_ON : DP_OFF;
	else
		status_result = DP_OFF;

	return status_result;

}

/**
 * @brief dp interrupt, pending clear
 * @fn void clear_dp_interrupt(struct stfb_info *info, enum edp_interrupt type)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param type[in] interrupt type
 * @return
 *
 * @note
 */
void clear_dp_interrupt(struct stfb_info *info, enum edp_interrupt type)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_int_set_clr);

	if (type == INT_TV)
		D4_DP_COMMON_TV_INT_CLR(reg, DP_ON);
	else if (type == INT_MLCD)
		D4_DP_COMMON_MLCD_INT_CLR(reg, DP_ON);
	else if (type == INT_SLCD)
		D4_DP_COMMON_SLCD_INT_CLR(reg, DP_ON);
	else if (type == INT_PERIODIC_ON)
		D4_DP_COMMON_PERIODIC_INT_CLR(reg, DP_ON);

	__raw_writel(reg, info->dp_global_regs + dp_int_set_clr);
}

