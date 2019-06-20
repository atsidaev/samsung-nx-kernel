#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/page.h>

#include "d4_dp_lcd_reg.h"
#include "d4_dp_lcd_dd.h"
#include "../global/d4_dp_global.h"

#include <linux/pinctrl/pinmux.h>
#include <asm/mmu_context.h>
#define ARRAY_SIZE_TEST(x) (sizeof(x) / sizeof((x)[0]))

struct stfb_info lcd_info;
static struct stfb_video_info lcd_video;
static struct stfb_graphic_info lcd_graphic;
static struct stfb_lcd_pannel lcd_pannel;
static struct stfb_lcd_pannel sublcd_pannel;

static struct stdp_filter_set hrz_filter_set[DP_BYPASS + 1] = { { 0, 1, 2, 1,
		0, 2 }, /**< LPF1 */
{ 0, 2, 3, 2, 0, 3 }, /**< LPF2 */
{ 1, 2, 2, 2, 1, 3 }, /**< LPF3 */
{ 0, -1, 3, -1, 0, 1 }, /**< HPF1 */
{ -1, -1, 4, -1, -1, 2 }, /**< HPF2 */
{ -1, -2, 4, -2, -1, 1 }, /**< HPF3 */
{ 0, 0, 1, 0, 0, 0 } /**< Bypass */
};

#ifdef CONFIG_PM

static struct lcd_sleep_save lcd_save[] = { { DP_LCD_PATH_CTRL, 0 }, {
		DP_LCD_SIZE, 0 }, { DP_LCD_STRIDE, 0 }, { DP_LCD_GRP_MIX_ON, 0 },

{ DP_LCD_WIN0_F0_Y_ADDR, 0 }, { DP_LCD_WIN0_F0_C_ADDR, 0 }, {
		DP_LCD_WIN0_F1_Y_ADDR, 0 }, { DP_LCD_WIN0_F1_C_ADDR, 0 }, {
		DP_LCD_WIN0_H_POS, 0 }, { DP_LCD_WIN0_V_POS, 0 },

{ DP_LCD_GRP1_DMA_CTRL, 0 }, { DP_LCD_GRP1_DMA_SCALER, 0 }, { DP_LCD_GRP1_BKG,
		0 }, { DP_LCD_GRP1_WIN_FLAT_ALPHA, 0 },
		{ DP_LCD_GRP1_WIN_PRIORITY, 0 },

		{ DP_LCD_GRP1_WIN0_F0_ADDR, 0 }, { DP_LCD_GRP1_WIN0_F1_ADDR, 0 }, {
				DP_LCD_GRP1_WIN0_H_POS, 0 }, { DP_LCD_GRP1_WIN0_V_POS, 0 },

		{ DP_LCD_GRP1_WIN1_F0_ADDR, 0 }, { DP_LCD_GRP1_WIN1_F1_ADDR, 0 }, {
				DP_LCD_GRP1_WIN1_H_POS, 0 }, { DP_LCD_GRP1_WIN1_V_POS, 0 },

		{ DP_LCD_GRP1_WIN2_F0_ADDR, 0 }, { DP_LCD_GRP1_WIN2_F1_ADDR, 0 }, {
				DP_LCD_GRP1_WIN2_H_POS, 0 }, { DP_LCD_GRP1_WIN2_V_POS, 0 },

		{ DP_LCD_GRP1_WIN3_F0_ADDR, 0 }, { DP_LCD_GRP1_WIN3_F1_ADDR, 0 }, {
				DP_LCD_GRP1_WIN3_H_POS, 0 }, { DP_LCD_GRP1_WIN3_V_POS, 0 } };

/**
 * @brief dp lcd register save function in suspend mode
 * @fn static void dp_lcd_do_save(struct stfb_info *info, struct lcd_sleep_save *ptr, int count)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param ptr[in] register, value
 * @return
 *
 * @note
 */
static void dp_lcd_do_save(struct stfb_info *info, struct lcd_sleep_save *ptr,
		int count)
{
	DpPRINTK("%s(),%d\n", __FUNCTION__, __LINE__);

	for (; count > 0; count--, ptr++) {
		ptr->val = __raw_readl(info->dp_lcd_regs + ptr->reg);
		DpPRINTK("%s(),%d, ptr->reg = 0x%x, ptr->val =0x%x\n", __FUNCTION__, __LINE__ , ptr->reg , ptr->val);
	}
}

/**
 * @brief dp lcd register resotore function in resume mode
 * @fn static void dp_lcd_do_restore(struct stfb_info *info, struct lcd_sleep_save *ptr, int count)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param ptr[in] register, value
 * @param count[in] register number
 * @return
 *
 * @note
 */
static void dp_lcd_do_restore(struct stfb_info *info,
		struct lcd_sleep_save *ptr, int count)
{
	DpPRINTK("%s(),%d\n", __FUNCTION__, __LINE__);

	for (; count > 0; count--, ptr++) {
		__raw_writel(ptr->val, info->dp_lcd_regs + ptr->reg);
	}
}

/**
 * @brief dp lcd register save function in suspend mode
 * @fn void dp_lcd_save_reg(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_lcd_save_reg(struct stfb_info *info)
{
	DpPRINTK("%s(),%d\n", __FUNCTION__, __LINE__);
	dp_lcd_do_save(info, lcd_save, ARRAY_SIZE_TEST(lcd_save));
}

/**
 * @brief dp lcd register restore function in resume mode
 * @fn void dp_lcd_restroe_reg(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_lcd_restroe_reg(struct stfb_info *info)
{
	DpPRINTK("%s(),%d\n", __FUNCTION__, __LINE__);
	dp_lcd_do_restore(info, lcd_save, ARRAY_SIZE_TEST(lcd_save));
}

#endif

/**
 * @brief dp lcd timming generating , it's related to a  panel
 * @fn static void dp_lcd_set_timming_sig(struct stfb_info *info, struct stdp_lcd_tg tg)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param tg[in] struct stdp_lcd_tg  reference linux/drivers/video/drime4
 * @return
 *
 * @note
 */
static void dp_lcd_set_timming_sig(struct stfb_info *info,
		struct stdp_lcd_tg tg)
{

	unsigned int reg = 0;
	D4_DP_LCD_TOTAL_PIXEL_NUM(reg, tg.total_h_size);
	D4_DP_LCD_TOTAL_LINE_NUM(reg, tg.total_v_size);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_SIZE);

	reg = 0;
	D4_DP_LCD_H_SYNC_BLK(reg, tg.h_sync_fall);
	D4_DP_LCD_H_SYNC_ACT(reg, tg.h_sync_rise);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_HSYNC);

	reg = 0;
	D4_DP_LCD_V_SYNC_BLK0(reg, tg.v_sync_fall);
	D4_DP_LCD_V_SYNC_ACT0(reg, tg.v_sync_rise);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_F0_VSYNC);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_F1_VSYNC);

	reg = 0;
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_FSYNC0);

	reg = 0;
	D4_DP_LCD_FRAME_INIT_LINE(reg, 0x0002);
	D4_DP_LCD_FRAME_FINAL_LINE(reg, tg.total_v_size);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_INFO0);

	reg = 0;
	D4_DP_LCD_FIELD_INIT_LINE(reg, 0x0002);
	D4_DP_LCD_BUF_READ_START_PIXEL(reg, tg.buf_read_h_start);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_INFO1);

	reg = 0;
	D4_DP_LCD_PRE_PIXEL_START(reg, 0x000A);
	D4_DP_LCD_PRE_PIXEL_END(reg, tg.total_h_size);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_INFO2);

	reg = 0;
	D4_DP_LCD_ACTIVE_PIXEL_START(reg, tg.enable_h_start);
	D4_DP_LCD_ACTIVE_PIXEL_END(reg, tg.enable_h_end);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_ACTIVE_PIXEL);

	reg = 0;
	D4_DP_LCD_ACTIVE_LINE_START(reg, tg.enable_v_start);
	D4_DP_LCD_ACTIVE_LINE_END(reg, tg.enable_v_end);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_ACTIVE_LINE0);

	reg = 0;
	D4_DP_LCD_ACTIVE_LINE_START(reg, tg.enable_v_start);
	D4_DP_LCD_ACTIVE_LINE_END(reg, tg.enable_v_end);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_TG_ACTIVE_LINE1);

	reg = 0;
	D4_DP_LCD_INV_DOT_CLK(reg, tg.inv_dot_clk);
	D4_DP_LCD_INV_ENABLE(reg, tg.inv_enable_clk);
	D4_DP_LCD_INV_H_SYNC(reg, tg.inv_h_sync);
	D4_DP_LCD_INV_V_SYNC(reg, tg.inv_v_sync);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_OUT_INV);

}

/**
 * @brief dp lcd video/graphic stride set
 * @fn static int dp_lcd_set_stride(struct stfb_info *info, enum edp_layer layer, unsigned int stride)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param tg[in] struct stdp_lcd_tg  reference linux/drivers/video/drime4
 * @param layer[in] select video/graphic
 * @param stride[in] the stride value must be the multiple of 8
 * @return
 *
 * @note
 */
static int dp_lcd_set_stride(struct stfb_info *info, enum edp_layer layer,
		unsigned int stride)
{
	unsigned int stride_value, reg = 0;

	if (layer == DP_VIDEO)
		stride_value = stride;
	else
		stride_value = stride;

	if (stride_value == 0) {
		printk("[dp lcd] stride value = 0\n");
		return -1;
	}

	if (stride_value > 16384) {
		printk("[dp lcd] stride limits 16383\n");
		return -1;
	}
	stride_value = stride_value / 8;

	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_STRIDE);

	if (layer == DP_VIDEO)
		D4_DP_LCD_PATH_VID_STRIDE(reg, stride_value);
	else
		D4_DP_LCD_PATH_GRP_STRIDE(reg, stride_value);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_STRIDE);

	return 0;
}

/**
 * @brief internal function, LCD Scantype
 * @fn static void dp_lcd_set_scantype(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_lcd_set_scantype(struct stfb_info *info)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_PATH_CTRL);

	D4_DP_LCD_PATH_VID_SCAN_ON(reg, 1);
	D4_DP_LCD_PATH_GRP_SCAN_ON(reg, 1);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_PATH_CTRL);
}

/**
 * @brief video background color set function
 * @fn static void dp_lcd_set_vid_bkg_color(struct stfb_info *info,	struct stdp_ycbcr *color)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *color[in]  struct stdp_ycbcr reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_lcd_set_vid_bkg_color(struct stfb_info *info,
		struct stdp_ycbcr *color)
{
	unsigned int reg = 0;
	D4_DP_LCD_PATH_BKG_Y(reg, color->DP_Y);
	D4_DP_LCD_PATH_BKG_Cb(reg, color->DP_Cb);
	D4_DP_LCD_PATH_BKG_Cr(reg, color->DP_Cr);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_DMA_MODE2);
}

/**
 * @brief video display position set
 * @fn void dp_lcd_set_vid_display_area(struct stfb_info *info, enum edp_window win, enum edp_video_bit vid_bit, struct stdp_display_area *display)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param win[in]  select display window
 * @param vid_bit[in]  video bit select 8/10bit
 * @param *display[in]  struct stdp_display_area reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note the horizontal start/end position must be the multiple of 2 in video 8bit <br>
 *	the horizontal start/end position must be the multiple of 16 in video 10bit
 */
void dp_lcd_set_vid_display_area(struct stfb_info *info, enum edp_window win,
		enum edp_video_bit vid_bit, struct stdp_display_area *display)
{
	unsigned int base_offset = 0, reg = 0;
	unsigned int h_start = display->H_Start;
	unsigned int h_end = display->H_Size + display->H_Start;
	unsigned int v_start = display->V_Start;
	unsigned int v_end = display->V_Size + display->V_Start;

	if (win == DP_WIN0)
		base_offset = DP_LCD_WIN0_H_POS;
	else if (win == DP_WIN1)
		base_offset = DP_LCD_WIN1_H_POS;
	else if (win == DP_WIN2)
		base_offset = DP_LCD_WIN2_H_POS;
	else
		base_offset = DP_LCD_WIN3_H_POS;

	if (vid_bit == _8bit) {
		if (h_start % 2 != 0)
			h_start--;
		if (h_end % 2 != 0)
			h_end--;
	} else {
		h_start = (h_start / 16) * 16;
		h_end = (h_end / 16) * 16;
	}

	D4_DP_LCD_POS_H_START(reg, h_start);
	D4_DP_LCD_POS_H_END(reg, h_end);
	__raw_writel(reg, info->dp_lcd_regs + base_offset);

	reg = 0;
	D4_DP_LCD_POS_V_START(reg, v_start);
	D4_DP_LCD_POS_V_END(reg, v_end);
	__raw_writel(reg, info->dp_lcd_regs + base_offset + 0x04);
}

/**
 * @brief video display image address set
 * @fn static void dp_lcd_set_vid_image_address(struct stfb_info *info,	struct stfb_video_info *video)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *video[in]  struct stfb_video_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_lcd_set_vid_image_address(struct stfb_info *info,
		struct stfb_video_info *video)
{
	unsigned int base_offset = 0;
	unsigned int y0_addr = 0, y1_addr = 0, c0_addr = 0, c1_addr = 0;

	if (video->win == DP_WIN0)
		base_offset = DP_LCD_WIN0_F0_Y_ADDR;
	else if (video->win == DP_WIN1)
		base_offset = DP_LCD_WIN1_F0_Y_ADDR;
	else if (video->win == DP_WIN2)
		base_offset = DP_LCD_WIN2_F0_Y_ADDR;
	else
		base_offset = DP_LCD_WIN3_F0_Y_ADDR;

	if (video->address.y0_address % 2 != 0)
		video->address.y0_address++;

	if (video->address.c0_address % 2 != 0)
		video->address.c0_address++;

	if (video->address.y1_address % 2 != 0)
		video->address.y1_address++;

	if (video->address.c1_address % 2 != 0)
		video->address.c1_address++;

	/* y0 address */
	y0_addr = (unsigned int) video->address.y0_address;
	__raw_writel(y0_addr, info->dp_lcd_regs + base_offset);

	/* y1 address */
	y1_addr = (unsigned int) video->address.y1_address;
	__raw_writel(y1_addr, info->dp_lcd_regs + base_offset + 0x08);

	/* c0 address */
	c0_addr = video->address.c0_address;
	__raw_writel(c0_addr, info->dp_lcd_regs + base_offset + 0x04);

	/* c1 address */
	c1_addr = video->address.c1_address;
	__raw_writel(c1_addr, info->dp_lcd_regs + base_offset + 0x0c);

}

/**
 * @brief lcd path control
 * @fn static void dp_lcd_set_path_onoff(struct stfb_info *info, enum elcd_path_onoff selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] lcd path
 * @param on_off[in] lcd path on/off
 * @return
 *
 * @note
 */
static void dp_lcd_set_path_onoff(struct stfb_info *info,
		enum elcd_path_onoff selection, enum edp_onoff on_off)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_PATH_CTRL);

	if (selection == LCD_CTRL_LCD_DMAC_ON)
		D4_DP_LCD_PATH_DMA_ON(reg, on_off);

	else if (selection == LCD_CTRL_VID_HRZFLT_ON)
		D4_DP_LCD_PATH_HRZ_FILTER_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_MIX_ON)
		D4_DP_LCD_PATH_GRP_MIX_ON(reg, on_off);

	else if (selection == LCD_CTRL_RGB2YCBCR_ON)
		D4_DP_LCD_PATH_GRP_RGB2YCBCR_ON(reg, on_off);

	else if (selection == LCD_CTRL_ZEBRA_ON)
		D4_DP_LCD_PATH_GRP_ZEBRA_ON(reg, on_off);

	else if (selection == LCD_CTRL_DITHER_ON)
		D4_DP_LCD_PATH_GRP_DITHER_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_DMAC_ON)
		D4_DP_LCD_PATH_GRP_DMA_CTRL_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_HRZFLT_ON)
		D4_DP_LCD_PATH_GRP_HRZ_FILTER_ON(reg, on_off);

	else if (selection == LCD_CTRL_OLED_GM_ON)
		D4_DP_LCD_PATH_OLED_GM_ON(reg, on_off);

	else if (selection == LCD_CTRL_TCC_ON)
		D4_DP_LCD_PATH_TCC_ON(reg, on_off);

	else if (selection == LCD_CTRL_3D_VID)
		D4_DP_LCD_PATH_3D_ON(reg, on_off);

	else if (selection == LCD_CTRL_SD_ON)
		D4_DP_LCD_PATH_SD_ON(reg, on_off);

	else if (selection == LCD_CTRL_YCBCR422)
		D4_DP_LCD_PATH_YCBCR422_ON(reg, on_off);

	else if (selection == LCD_CTRL_ROTATE_LR)
		D4_DP_LCD_PATH_ROTATE_ON(reg, on_off);

	else if (selection == LCD_CTRL_VID_10BIT)
		D4_DP_LCD_PATH_VID_10BIT_ON(reg, on_off);

	else if (selection == LCD_CTRL_TCC_VID)
		D4_DP_LCD_PATH_TCC_VID_ON(reg, on_off);

	else if (selection == LCD_CTRL_VID_SCAN)
		D4_DP_LCD_PATH_VID_SCAN_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_SCAN)
		D4_DP_LCD_PATH_GRP_SCAN_ON(reg, on_off);

	else if (selection == LCD_CTRL_BBOX_2X)
		D4_DP_LCD_PATH_BBOX_2X_ON(reg, on_off);

	else if (selection == LCD_CTRL_LINE_INV)
		D4_DP_LCD_PATH_LINE_INV_ON(reg, on_off);

	else if (selection == LCD_CTRL_VID_HSWAP)
		D4_DP_LCD_PATH_LINE_VID_H_SWAP_ON(reg, on_off);

	else if (selection == LCD_CTRL_VID_VSWAP)
		D4_DP_LCD_PATH_LINE_VID_V_SWAP_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_HSWAP)
		D4_DP_LCD_PATH_LINE_GRP_H_SWAP_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_VSWAP)
		D4_DP_LCD_PATH_LINE_GRP_V_SWAP_ON(reg, on_off);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_PATH_CTRL);
}

/**
 * @brief lcd video window on/off set
 * @fn static void dp_lcd_vid_window_onoff(struct stfb_info *info, enum edp_window window, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param window[in] select video window
 * @param on_off[in] lcd path on/off
 * @return
 *
 * @note
 */
static void dp_lcd_vid_window_onoff(struct stfb_info *info,
		enum edp_window window, enum edp_onoff on_off)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_DMA_MODE0);

	if (window == DP_WIN0)
		D4_DP_LCD_DMA_WIN0_ON(reg, on_off);
	else if (window == DP_WIN1)
		D4_DP_LCD_DMA_WIN1_ON(reg, on_off);
	else if (window == DP_WIN2)
		D4_DP_LCD_DMA_WIN2_ON(reg, on_off);
	else
		D4_DP_LCD_DMA_WIN3_ON(reg, on_off);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_DMA_MODE0);
}

/**
 * @brief lcd graphic window on/off set
 * @fn void dp_lcd_grp_window_onoff(struct stfb_info *info, enum edp_window window, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param window[in] select video window
 * @param on_off[in] lcd path on/off
 * @return
 *
 * @note
 */
void dp_lcd_grp_window_onoff(struct stfb_info *info, enum edp_window window,
		enum edp_onoff on_off)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_GRP1_DMA_CTRL);

	if (window == DP_WIN0)
		D4_DP_LCD_DMA_WIN0_ON(reg, on_off);
	else if (window == DP_WIN1)
		D4_DP_LCD_DMA_WIN1_ON(reg, on_off);
	else if (window == DP_WIN2)
		D4_DP_LCD_DMA_WIN2_ON(reg, on_off);
	else
		D4_DP_LCD_DMA_WIN3_ON(reg, on_off);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_GRP1_DMA_CTRL);
}

/**
 * @brief boundary box color table set
 * @fn static void dp_lcd_vid_set_bb_color_table(struct stfb_info *info,	enum edp_bb_color_table table, struct stdp_rgb *rgb_info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param table[in] select boudary box color table
 * @param *rgb_info[in] boundary box color table  set color
 * @return
 *
 * @note
 */
static void dp_lcd_vid_set_bb_color_table(struct stfb_info *info,
		enum edp_bb_color_table table, struct stdp_rgb *rgb_info)
{
	unsigned int reg = 0;

	D4_DP_LCD_BB_B_COLOR(reg, rgb_info->DP_B);
	D4_DP_LCD_BB_G_COLOR(reg, rgb_info->DP_G);
	D4_DP_LCD_BB_R_COLOR(reg, rgb_info->DP_R);

	if (table == DP_BB_COLOR_TABLE_0)
		__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_COLOR_0);
	else if (table == DP_BB_COLOR_TABLE_1)
		__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_COLOR_1);
	else if (table == DP_BB_COLOR_TABLE_2)
		__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_COLOR_2);
	else
		__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_COLOR_3);
}

/**
 * @brief boundary box boundary horizontal, vertical gap size set
 * @fn static void dp_lcd_set_bb_width(struct stfb_info *info, unsigned char h_width, unsigned char v_width)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param h_width[in] boudary box boudary horizontal gap size
 * @param v_width[in] boudary box boudary vertical gap size
 * @return
 *
 * @note
 */
static void dp_lcd_set_bb_width(struct stfb_info *info, unsigned char h_width,
		unsigned char v_width)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_BB_WIDTH_ALPHA);
	D4_DP_LCD_BB_H_WIDTH(reg, h_width);
	D4_DP_LCD_BB_V_WIDTH(reg, v_width);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_WIDTH_ALPHA);
}

/**
 * @brief boundary box boundary transparent set
 * @fn static void dp_lcd_set_bb_alpha(struct stfb_info *info, unsigned char alpha)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param alpha[in] transparent value
 * @return
 *
 * @note
 */
static void dp_lcd_set_bb_alpha(struct stfb_info *info, unsigned char alpha)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_BB_WIDTH_ALPHA);
	D4_DP_LCD_BB_ALPHA(reg, alpha);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_WIDTH_ALPHA);
}

/**
 * @brief graphic window prority set
 * @fn static void dp_lcd_grp_win_prority(struct stfb_info *info, unsigned char win0, unsigned char win1, unsigned char win2, unsigned char win3)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param win0[in] graphic window0
 * @param win1[in] graphic window1
 * @param win2[in] graphic window2
 * @param win3[in] graphic window3
 * @return
 *
 * @note
 */
static void dp_lcd_grp_win_prority(struct stfb_info *info, unsigned char win0,
		unsigned char win1, unsigned char win2, unsigned char win3)
{
	unsigned int reg = 0;
	D4_DP_LCD_WIN0_PRORITY(reg, win0);
	D4_DP_LCD_WIN1_PRORITY(reg, win1);
	D4_DP_LCD_WIN2_PRORITY(reg, win2);
	D4_DP_LCD_WIN3_PRORITY(reg, win3);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_GRP1_WIN_PRIORITY);

}

/**
 * @brief graphic window prority set
 * @fnstatic void dp_lcd_set_grp_priority(struct stfb_info *info, struct stdp_grp_prority *priority)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *priority[in] prority winow set
 * @return
 *
 * @note
 */
static void dp_lcd_set_grp_priority(struct stfb_info *info,
		struct stdp_grp_prority *priority)
{
	unsigned char win_num, Sequence[4] = { 0, }, Max = 3;

	for (win_num = 0; win_num < 4; win_num++) {
		if (priority->First_Priority == win_num)
			Sequence[win_num] = Max;
		if (priority->Second_Priority == win_num)
			Sequence[win_num] = Max - 1;
		if (priority->Third_Priotrity == win_num)
			Sequence[win_num] = Max - 2;
		if (priority->Fourth_Priority == win_num)
			Sequence[win_num] = Max - 3;
	}

	dp_lcd_grp_win_prority(info, Sequence[0], Sequence[1], Sequence[2],
			Sequence[3]);
}

/**
 * @brief boundary box outline color
 * @fn static void dp_lcd_set_bb_outline_color(struct stfb_info *info, struct stdp_rgb *bb_out_color)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *bb_out_color[in] struct stdp_rgb reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_lcd_set_bb_outline_color(struct stfb_info *info,
		struct stdp_rgb *bb_out_color)
{
	unsigned int reg = 0;
	D4_DP_LCD_BB_OUTLINE_B(reg, bb_out_color->DP_B);
	D4_DP_LCD_BB_OUTLINE_G(reg, bb_out_color->DP_G);
	D4_DP_LCD_BB_OUTLINE_R(reg, bb_out_color->DP_R);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_OUTLINE_COLOR);
}

/**
 * @brief boundary box outline on/off
 * @fn static void dp_lcd_set_bb_outline_onoff(struct stfb_info *info, enum edp_bb_box selection, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] boundary box select
 * @param onoff[in] boundary box outline on/off
 * @return
 *
 * @note
 */
static void dp_lcd_set_bb_outline_onoff(struct stfb_info *info,
		enum edp_bb_box selection, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_BB_WIDTH_ALPHA);

	if (selection == DP_BB_00)
		D4_DP_LCD_BB_OUTLINE_00(reg, onoff);
	else if (selection == DP_BB_01)
		D4_DP_LCD_BB_OUTLINE_01(reg, onoff);
	else if (selection == DP_BB_02)
		D4_DP_LCD_BB_OUTLINE_02(reg, onoff);
	else if (selection == DP_BB_03)
		D4_DP_LCD_BB_OUTLINE_03(reg, onoff);
	else if (selection == DP_BB_04)
		D4_DP_LCD_BB_OUTLINE_04(reg, onoff);
	else if (selection == DP_BB_05)
		D4_DP_LCD_BB_OUTLINE_05(reg, onoff);
	else if (selection == DP_BB_06)
		D4_DP_LCD_BB_OUTLINE_06(reg, onoff);
	else if (selection == DP_BB_07)
		D4_DP_LCD_BB_OUTLINE_07(reg, onoff);
	else if (selection == DP_BB_08)
		D4_DP_LCD_BB_OUTLINE_08(reg, onoff);
	else if (selection == DP_BB_09)
		D4_DP_LCD_BB_OUTLINE_09(reg, onoff);
	else if (selection == DP_BB_10)
		D4_DP_LCD_BB_OUTLINE_10(reg, onoff);
	else if (selection == DP_BB_11)
		D4_DP_LCD_BB_OUTLINE_11(reg, onoff);
	else if (selection == DP_BB_12)
		D4_DP_LCD_BB_OUTLINE_12(reg, onoff);
	else if (selection == DP_BB_13)
		D4_DP_LCD_BB_OUTLINE_13(reg, onoff);
	else if (selection == DP_BB_14)
		D4_DP_LCD_BB_OUTLINE_14(reg, onoff);
	else
		D4_DP_LCD_BB_OUTLINE_15(reg, onoff);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_WIDTH_ALPHA);
}

/**
 * @brief boundary box rectangle style select
 * @fn static void dp_lcd_set_bb_mode(struct stfb_info *info,	enum edp_bb_box selection, enum edp_bb_style_set style)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] boundary box select
 * @param onoff[in] boundary box outline on/off
 * @return
 *
 * @note
 */
static void dp_lcd_set_bb_mode(struct stfb_info *info,
		enum edp_bb_box selection, enum edp_bb_style_set style)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_BB_MODE_ON);

	if (selection == DP_BB_00)
		D4_DP_LCD_BB00_MODE_ON(reg, style);
	else if (selection == DP_BB_01)
		D4_DP_LCD_BB01_MODE_ON(reg, style);
	else if (selection == DP_BB_02)
		D4_DP_LCD_BB02_MODE_ON(reg, style);
	else if (selection == DP_BB_03)
		D4_DP_LCD_BB03_MODE_ON(reg, style);
	else if (selection == DP_BB_04)
		D4_DP_LCD_BB04_MODE_ON(reg, style);
	else if (selection == DP_BB_05)
		D4_DP_LCD_BB05_MODE_ON(reg, style);
	else if (selection == DP_BB_06)
		D4_DP_LCD_BB06_MODE_ON(reg, style);
	else if (selection == DP_BB_07)
		D4_DP_LCD_BB07_MODE_ON(reg, style);
	else if (selection == DP_BB_08)
		D4_DP_LCD_BB08_MODE_ON(reg, style);
	else if (selection == DP_BB_09)
		D4_DP_LCD_BB09_MODE_ON(reg, style);
	else if (selection == DP_BB_10)
		D4_DP_LCD_BB10_MODE_ON(reg, style);
	else if (selection == DP_BB_11)
		D4_DP_LCD_BB11_MODE_ON(reg, style);
	else if (selection == DP_BB_12)
		D4_DP_LCD_BB12_MODE_ON(reg, style);
	else if (selection == DP_BB_13)
		D4_DP_LCD_BB13_MODE_ON(reg, style);
	else if (selection == DP_BB_14)
		D4_DP_LCD_BB14_MODE_ON(reg, style);
	else
		D4_DP_LCD_BB15_MODE_ON(reg, style);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_MODE_ON);
}

/**
 * @brief boundary box select color table select
 * @fn static void dp_lcd_set_bb_color(struct stfb_info *info, enum edp_bb_box selection, enum edp_bb_color_table color)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] boundary box select
 * @param color[in] color table select
 * @return
 *
 * @note
 */
static void dp_lcd_set_bb_color(struct stfb_info *info,
		enum edp_bb_box selection, enum edp_bb_color_table color)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_BB_COLOR_SELECT);

	if (selection == DP_BB_00)
		D4_DP_LCD_BB_COLOR00(reg, color);
	else if (selection == DP_BB_01)
		D4_DP_LCD_BB_COLOR01(reg, color);
	else if (selection == DP_BB_02)
		D4_DP_LCD_BB_COLOR02(reg, color);
	else if (selection == DP_BB_03)
		D4_DP_LCD_BB_COLOR03(reg, color);
	else if (selection == DP_BB_04)
		D4_DP_LCD_BB_COLOR04(reg, color);
	else if (selection == DP_BB_05)
		D4_DP_LCD_BB_COLOR05(reg, color);
	else if (selection == DP_BB_06)
		D4_DP_LCD_BB_COLOR06(reg, color);
	else if (selection == DP_BB_07)
		D4_DP_LCD_BB_COLOR07(reg, color);
	else if (selection == DP_BB_08)
		D4_DP_LCD_BB_COLOR08(reg, color);
	else if (selection == DP_BB_09)
		D4_DP_LCD_BB_COLOR09(reg, color);
	else if (selection == DP_BB_10)
		D4_DP_LCD_BB_COLOR10(reg, color);
	else if (selection == DP_BB_11)
		D4_DP_LCD_BB_COLOR11(reg, color);
	else if (selection == DP_BB_12)
		D4_DP_LCD_BB_COLOR12(reg, color);
	else if (selection == DP_BB_13)
		D4_DP_LCD_BB_COLOR13(reg, color);
	else if (selection == DP_BB_14)
		D4_DP_LCD_BB_COLOR14(reg, color);
	else
		D4_DP_LCD_BB_COLOR15(reg, color);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_COLOR_SELECT);
}

/**
 * @brief boundary box dislpay area set
 * @fn static void dp_lcd_set_bb_display_area(struct stfb_info *info, enum edp_bb_box selection, struct stdp_display_area area)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] boundary box select
 * @param area[in] display area set
 * @return
 *
 * @note
 */
static void dp_lcd_set_bb_display_area(struct stfb_info *info,
		enum edp_bb_box selection, struct stdp_display_area area)
{
	unsigned int h_position = 0, v_position = 0;

	D4_DP_LCD_BB_H_START(h_position, area.H_Start);
	D4_DP_LCD_BB_H_END(h_position, area.H_Size + area.H_Start);

	D4_DP_LCD_BB_V_START(v_position, area.V_Start);
	D4_DP_LCD_BB_V_END(v_position, area.V_Size + area.V_Start);

	switch (selection) {
	case DP_BB_00:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB00_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB00_V_POS);
		break;
	case DP_BB_01:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB01_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB01_V_POS);
		break;
	case DP_BB_02:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB02_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB02_V_POS);
		break;
	case DP_BB_03:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB03_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB03_V_POS);
		break;
	case DP_BB_04:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB04_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB04_V_POS);
		break;
	case DP_BB_05:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB05_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB05_V_POS);
		break;
	case DP_BB_06:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB06_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB06_V_POS);
		break;
	case DP_BB_07:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB07_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB07_V_POS);
		break;
	case DP_BB_08:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB08_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB08_V_POS);
		break;
	case DP_BB_09:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB09_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB09_V_POS);
		break;
	case DP_BB_10:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB10_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB10_V_POS);
		break;
	case DP_BB_11:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB11_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB11_V_POS);
		break;
	case DP_BB_12:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB12_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB12_V_POS);
		break;
	case DP_BB_13:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB13_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB13_V_POS);
		break;
	case DP_BB_14:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB14_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB14_V_POS);
		break;
	case DP_BB_15:
		__raw_writel(h_position, info->dp_lcd_regs + DP_LCD_BB15_H_POS);
		__raw_writel(v_position, info->dp_lcd_regs + DP_LCD_BB15_V_POS);
		break;
	}
}

/**
 * @brief boundary box on/off
 * @fn static void dp_lcd_vid_bb_onoff(struct stfb_info *info,	enum edp_bb_box selection, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] boundary box select
 * @param onoff[in] on/off
 * @return
 *
 * @note
 */
static void dp_lcd_vid_bb_onoff(struct stfb_info *info,
		enum edp_bb_box selection, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_BB_MODE_ON);

	if (selection == DP_BB_00)
		D4_DP_LCD_BB00_ON(reg, onoff);
	else if (selection == DP_BB_01)
		D4_DP_LCD_BB01_ON(reg, onoff);
	else if (selection == DP_BB_02)
		D4_DP_LCD_BB02_ON(reg, onoff);
	else if (selection == DP_BB_03)
		D4_DP_LCD_BB03_ON(reg, onoff);
	else if (selection == DP_BB_04)
		D4_DP_LCD_BB04_ON(reg, onoff);
	else if (selection == DP_BB_05)
		D4_DP_LCD_BB05_ON(reg, onoff);
	else if (selection == DP_BB_06)
		D4_DP_LCD_BB06_ON(reg, onoff);
	else if (selection == DP_BB_07)
		D4_DP_LCD_BB07_ON(reg, onoff);
	else if (selection == DP_BB_08)
		D4_DP_LCD_BB08_ON(reg, onoff);
	else if (selection == DP_BB_09)
		D4_DP_LCD_BB09_ON(reg, onoff);
	else if (selection == DP_BB_10)
		D4_DP_LCD_BB10_ON(reg, onoff);
	else if (selection == DP_BB_11)
		D4_DP_LCD_BB11_ON(reg, onoff);
	else if (selection == DP_BB_12)
		D4_DP_LCD_BB12_ON(reg, onoff);
	else if (selection == DP_BB_13)
		D4_DP_LCD_BB13_ON(reg, onoff);
	else if (selection == DP_BB_14)
		D4_DP_LCD_BB14_ON(reg, onoff);
	else
		D4_DP_LCD_BB15_ON(reg, onoff);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_BB_MODE_ON);
}


/**
 * @brief graphic color limit set
 * @fn static void dp_lcd_set_grp_limtval(struct stfb_info *info, struct stdp_rgb_range limit_range)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param limit_range[in] limit range set
 * @return
 *
 * @note
 */
static void dp_lcd_set_grp_limtval(struct stfb_info *info,
		struct stdp_rgb_range limit_range)
{
	unsigned int r_range = 0, g_range = 0, b_range = 0;

	D4_DP_LCD_GRP_MIX_RANGE_LOW(r_range, limit_range.R_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(r_range, limit_range.R_Range.Upper_Range);

	D4_DP_LCD_GRP_MIX_RANGE_LOW(g_range, limit_range.G_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(g_range, limit_range.G_Range.Upper_Range);

	D4_DP_LCD_GRP_MIX_RANGE_LOW(b_range, limit_range.B_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(b_range, limit_range.B_Range.Upper_Range);

	__raw_writel(r_range, info->dp_lcd_regs + DP_LCD_GRP_MIX_LIMIT_R);
	__raw_writel(g_range, info->dp_lcd_regs + DP_LCD_GRP_MIX_LIMIT_G);
	__raw_writel(b_range, info->dp_lcd_regs + DP_LCD_GRP_MIX_LIMIT_B);
}

/**
 * @brief graphic mix control
 * @fn static void dp_lcd_set_grp_mix_channel_onoff(struct stfb_info *info,	enum edp_layer layer, enum egrp_mix_channel_onoff selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param layer[in] select video/graphic
 * @param selection[in] selection graphic mix channel
 * @param on_off[in] graphic mix channel on/off
 * @return
 *
 * @note
 */
static void dp_lcd_set_grp_mix_channel_onoff(struct stfb_info *info,
		enum edp_layer layer, enum egrp_mix_channel_onoff selection,
		enum edp_onoff on_off)
{
	unsigned int reg = 0;
	if (DP_VIDEO == layer)
		reg = __raw_readl(info->dp_lcd_regs + DP_LCD_GRP_MIX_CH0);
	else
		reg = __raw_readl(info->dp_lcd_regs + DP_LCD_GRP_MIX_CH1);

	if (selection == GRP_MIX_CH_R_ON)
		D4_DP_LCD_GRP_MIX_R_ON(reg, on_off);
	else if (selection == GRP_MIX_CH_G_ON)
		D4_DP_LCD_GRP_MIX_G_ON(reg, on_off);
	else if (selection == GRP_MIX_CH_B_ON)
		D4_DP_LCD_GRP_MIX_B_ON(reg, on_off);
	else if (selection == GRP_MIX_CH_R_INV_ON)
		D4_DP_LCD_GRP_MIX_R_INV(reg, on_off);
	else if (selection == GRP_MIX_CH_G_INV_ON)
		D4_DP_LCD_GRP_MIX_G_INV(reg, on_off);
	else if (selection == GRP_MIX_CH_B_INV_ON)
		D4_DP_LCD_GRP_MIX_B_INV(reg, on_off);
	else if (selection == GRP_MIX_CH_R_OFS_ON)
		D4_DP_LCD_GRP_MIX_R_OFFSET(reg, on_off);
	else if (selection == GRP_MIX_CH_G_OFS_ON)
		D4_DP_LCD_GRP_MIX_G_OFFSET(reg, on_off);
	else
		D4_DP_LCD_GRP_MIX_B_OFFSET(reg, on_off);

	if (DP_VIDEO == layer)
		__raw_writel(reg, info->dp_lcd_regs + DP_LCD_GRP_MIX_CH0);
	else
		__raw_writel(reg, info->dp_lcd_regs + DP_LCD_GRP_MIX_CH1);

}

/**
 * @brief graphic mix on/off
 * @fn static void dp_lcd_set_grp_mix_onoff(struct stfb_info *info,	enum egrp_mix_onoff selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] select graphic mix control
 * @param on_off[in] graphic mix channel on/off
 * @return
 *
 * @note
 */
static void dp_lcd_set_grp_mix_onoff(struct stfb_info *info,
		enum egrp_mix_onoff selection, enum edp_onoff on_off)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_GRP_MIX_ON);

	if (selection == GRP_MIX_ALP_BLD_ON)
		D4_DP_LCD_ALPHA_ON(reg, on_off);
	else if (selection == GRP_MIX_LIMIT_ON)
		D4_DP_LCD_LIMIT_ON(reg, on_off);
	else if (selection == GRP_MIX_FLAG_R_ON)
		D4_DP_LCD_FLAG_R_ON(reg, on_off);
	else if (selection == GRP_MIX_FLAG_G_ON)
		D4_DP_LCD_FLAG_G_ON(reg, on_off);
	else if (selection == GRP_MIX_FLAG_B_ON)
		D4_DP_LCD_FLAG_B_ON(reg, on_off);
	else if (selection == GRP_MIX_VID_RANGE_DET)
		D4_DP_LCD_RANGE_DET_VID(reg, on_off);
	else if (selection == GRP_MIX_GRP_RANGE_DET)
		D4_DP_LCD_RANGE_DET_GRP(reg, on_off);
	else if (selection == GRP_MIX_RANGE_TARGET)
		D4_DP_LCD_RANGE_TARGET(reg, on_off);
	else if (selection == GRP_MIX_LAYER_SWAP)
		D4_DP_LCD_CH_SWAP(reg, on_off);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_GRP_MIX_ON);

	DpPRINTK(" write grp mix reg =0x%x\n", reg);
}

/**
 * @brief graphic range set
 * @fn static void dp_lcd_set_rangedetection_range(struct stfb_info *info, struct stdp_rgb_range range)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param range[in] graphic mix range set
 * @return
 *
 * @note
 */
static void dp_lcd_set_rangedetection_range(struct stfb_info *info,
		struct stdp_rgb_range range)
{
	unsigned int r_range = 0, g_range = 0, b_range = 0;

	D4_DP_LCD_GRP_MIX_RANGE_LOW(r_range, range.R_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(r_range, range.R_Range.Upper_Range);

	D4_DP_LCD_GRP_MIX_RANGE_LOW(g_range, range.G_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(g_range, range.G_Range.Upper_Range);

	D4_DP_LCD_GRP_MIX_RANGE_LOW(b_range, range.B_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(b_range, range.B_Range.Upper_Range);

	__raw_writel(r_range, info->dp_lcd_regs + DP_LCD_R_RANGE_CH0);
	__raw_writel(g_range, info->dp_lcd_regs + DP_LCD_R_RANGE_CH0 + 0x4);
	__raw_writel(b_range, info->dp_lcd_regs + DP_LCD_R_RANGE_CH0 + 0x8);
}

/**
 * @brief graphic mix alpha offset
 * @fn static void dp_lcd_set_grp_alpha_offset_val(struct stfb_info *info, unsigned char alpha_offset_val)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param alpha_offset_val[in] graphic alpha offset value
 * @return
 *
 * @note
 */
static void dp_lcd_set_grp_alpha_offset_val(struct stfb_info *info,
		unsigned char alpha_offset_val)
{
	unsigned int alpha_off_set_val = 0;

	alpha_off_set_val = __raw_readl(info->dp_lcd_regs + DP_LCD_MIX_ALPHA_CTRL);

	if (alpha_off_set_val & 0x04)
	D4_DP_LCD_PATH_ALP_OFS_4BIT(alpha_off_set_val, alpha_offset_val);
	else
	D4_DP_LCD_PATH_ALP_OFS_4BIT(alpha_off_set_val, 0);

	__raw_writel(alpha_off_set_val, info->dp_lcd_regs + DP_LCD_MIX_ALPHA_CTRL);
}


/**
 * @brief lcd video/graphic horizontal filter set
 * @fn static void dp_lcd_hrz_filter_onoff(struct stfb_info *info, enum ehrz_filter_type type, enum edp_filter filer_value, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param type[in] filter type
 * @param filer_value[in] value
 * @param on_off[in] on/off
 * @return
 *
 * @note
 */
static void dp_lcd_hrz_filter_onoff(struct stfb_info *info,
		enum ehrz_filter_type type, enum edp_filter filer_value,
		enum edp_onoff on_off)
{
	unsigned int reg = 0;

	if (on_off == DP_OFF)
		filer_value = DP_BYPASS;

	if (hrz_filter_set[filer_value].Tap0_Coef < 0) {
		D4_DP_LCD_PATH_T0_SIGN(reg, DP_ON);
		D4_DP_LCD_PATH_T0_COEF(reg, -1 * hrz_filter_set[filer_value].Tap0_Coef);
	} else {
		D4_DP_LCD_PATH_T0_SIGN(reg, DP_OFF);
		D4_DP_LCD_PATH_T0_COEF(reg, hrz_filter_set[filer_value].Tap0_Coef);
	}

	if (hrz_filter_set[filer_value].Tap1_Coef < 0) {
		D4_DP_LCD_PATH_T1_SIGN(reg, DP_ON);
		D4_DP_LCD_PATH_T1_COEF(reg, -1 * hrz_filter_set[filer_value].Tap1_Coef);
	} else {
		D4_DP_LCD_PATH_T1_SIGN(reg, DP_OFF);
		D4_DP_LCD_PATH_T1_COEF(reg, hrz_filter_set[filer_value].Tap1_Coef);
	}
	D4_DP_LCD_PATH_T2_COEF(reg, hrz_filter_set[filer_value].Tap2_Coef);

	if (hrz_filter_set[filer_value].Tap3_Coef < 0) {
		D4_DP_LCD_PATH_T3_SIGN(reg, DP_ON);
		D4_DP_LCD_PATH_T3_COEF(reg, -1 * hrz_filter_set[filer_value].Tap3_Coef);
	} else {
		D4_DP_LCD_PATH_T3_SIGN(reg, DP_OFF);
		D4_DP_LCD_PATH_T3_COEF(reg, hrz_filter_set[filer_value].Tap3_Coef);
	}

	if (hrz_filter_set[filer_value].Tap4_Coef < 0) {
		D4_DP_LCD_PATH_T4_SIGN(reg, DP_ON);
		D4_DP_LCD_PATH_T4_COEF(reg, -1 * hrz_filter_set[filer_value].Tap4_Coef);
	} else {
		D4_DP_LCD_PATH_T4_SIGN(reg, DP_OFF);
		D4_DP_LCD_PATH_T4_COEF(reg, hrz_filter_set[filer_value].Tap4_Coef);
	}
	D4_DP_LCD_PATH_POST_COEF(reg, hrz_filter_set[filer_value].Post_Coef);

	if (type == HRZ_FLT_TYPE_VIDEO) {
		__raw_writel(reg, info->dp_lcd_regs + DP_LCD_HRZ_FLT);
		dp_lcd_set_path_onoff(info, LCD_CTRL_VID_HRZFLT_ON, on_off);
	} else {
		__raw_writel(reg, info->dp_lcd_regs + DP_LCD_GRP1_HRZ_FLT);
		dp_lcd_set_path_onoff(info, LCD_CTRL_GRP_HRZFLT_ON, on_off);
	}
}

/**
 * @brief lcd color sapce matrix set
 * @fn static void dp_lcd_set_csc_matrix(struct stfb_info *info, enum edp_layer layer,	enum elcd_csc_type type)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param layer[in] video/graphic
 * @param type[in] color space type
 * @return
 *
 * @note
 */
static void dp_lcd_set_csc_matrix(struct stfb_info *info, enum edp_layer layer,
		enum elcd_csc_type type)
{
	unsigned int CSC_Matrix_Y_Cb_offset = 0;
	unsigned int CSC_Matrix_Cr_offset = 0;
	unsigned int CSC_Matrix_00_01 = 0;

	unsigned int CSC_Matrix_02_10 = 0;
	unsigned int CSC_Matrix_11_12 = 0;
	unsigned int CSC_Matrix_20_21 = 0;
	unsigned int CSC_Matrix_22 = 0;

	unsigned int CSC_Matrix_range = 0;
	unsigned int CSC_Matrix_range_y = 0;
	unsigned int CSC_Matrix_range_c = 0;

	if (DP_VIDEO == layer) {
		switch (type) {
		case LCD_CSC_SD_STANDARD:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x200);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x2BE);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x200);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x165);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x0AC);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x200);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x377);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x000);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x200);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x200);

			D4_DP_LCD_YCC2RGB_UNDER_RGB(CSC_Matrix_range, 0x58);
			D4_DP_LCD_YCC2RGB_OVER_RGB(CSC_Matrix_range, 0x3ac);
			break;

		case LCD_CSC_SD_FULL:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x331);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x1A0);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x0C9);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x409);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x040);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x200);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x200);

			D4_DP_LCD_YCC2RGB_UNDER_RGB(CSC_Matrix_range, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_RGB(CSC_Matrix_range, 0x3ff);
			break;

		case LCD_CSC_HD_STANDARD:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x200);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x314);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x000);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x200);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x0EB);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x05E);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x200);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x3A2);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x000);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x200);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x200);

			D4_DP_LCD_YCC2RGB_UNDER_RGB(CSC_Matrix_range, 0x40);
			D4_DP_LCD_YCC2RGB_OVER_RGB(CSC_Matrix_range, 0x3ac);
			break;

		case LCD_CSC_HD_FULL:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x396);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x111);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x06D);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x43B);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x040);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x200);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x200);

			D4_DP_LCD_YCC2RGB_UNDER_RGB(CSC_Matrix_range, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_RGB(CSC_Matrix_range, 0x3ff);
			break;
		}

		__raw_writel(CSC_Matrix_00_01, info->dp_lcd_regs + DP_LCD_CSC0);
		__raw_writel(CSC_Matrix_02_10, info->dp_lcd_regs + DP_LCD_CSC1);
		__raw_writel(CSC_Matrix_11_12, info->dp_lcd_regs + DP_LCD_CSC2);
		__raw_writel(CSC_Matrix_20_21, info->dp_lcd_regs + DP_LCD_CSC3);
		__raw_writel(CSC_Matrix_22, info->dp_lcd_regs + DP_LCD_CSC4);
		__raw_writel(CSC_Matrix_Y_Cb_offset, info->dp_lcd_regs + DP_LCD_CSC5);
		__raw_writel(CSC_Matrix_Cr_offset, info->dp_lcd_regs + DP_LCD_CSC6);
		__raw_writel(CSC_Matrix_range, info->dp_lcd_regs + DP_LCD_CSC7);

	} else {
		dp_lcd_set_path_onoff(info, LCD_CTRL_RGB2YCBCR_ON, DP_ON);
		switch (type) {
		case LCD_CSC_SD_STANDARD:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x099);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x12C);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x058);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x03A);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x0AD);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x105);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x105);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x0DB);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x02A);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x000);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x080);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x080);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_y, 0x10);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_y, 0xeb);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_c, 0x10);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_c, 0xf0);
			break;

		case LCD_CSC_SD_FULL:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x083);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x102);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x04B);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x032);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x094);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x0E0);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x0E0);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x0BC);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x024);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x010);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x080);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x080);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_y, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_y, 0xff);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_c, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_c, 0xff);
			break;

		case LCD_CSC_HD_STANDARD:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x06D);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x16E);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x03B);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x024);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x0C9);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x105);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x105);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x0ED);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x018);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x000);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x080);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x080);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_y, 0x00);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_y, 0xff);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_c, 0x00);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_c, 0xff);
			break;

		case LCD_CSC_HD_FULL:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x05D);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x13A);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x033);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x01F);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x0AD);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x0E0);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x0E0);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x0CC);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x014);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x010);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x080);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x080);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_y, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_y, 0xff);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_c, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_c, 0xff);
			break;

		}
		__raw_writel(CSC_Matrix_00_01, info->dp_lcd_regs + DP_GRP_CSC0);
		__raw_writel(CSC_Matrix_02_10, info->dp_lcd_regs + DP_GRP_CSC1);
		__raw_writel(CSC_Matrix_11_12, info->dp_lcd_regs + DP_GRP_CSC2);
		__raw_writel(CSC_Matrix_20_21, info->dp_lcd_regs + DP_GRP_CSC3);
		__raw_writel(CSC_Matrix_22, info->dp_lcd_regs + DP_GRP_CSC4);
		__raw_writel(CSC_Matrix_Y_Cb_offset, info->dp_lcd_regs + DP_GRP_CSC5);
		__raw_writel(CSC_Matrix_Cr_offset, info->dp_lcd_regs + DP_GRP_CSC6);
		__raw_writel(CSC_Matrix_range_y, info->dp_lcd_regs + DP_GRP_CSC7);
		__raw_writel(CSC_Matrix_range_c, info->dp_lcd_regs + DP_GRP_CSC8);
	}
}

/**
 * @brief lcd start function
 * @fn static void dp_lcd_start(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_lcd_start(struct stfb_info *info)
{
	dp_reg_update(info, DP_MLCD, DP_ON, DP_OFF);
	dp_operation_start(info, DP_MLCD);
	dp_reg_update(info, DP_MLCD, DP_ON, DP_ON);
	dp_reg_update(info, DP_MLCD, DP_ON, DP_OFF);
}

/**
 * @brief sublcd start function
 * @fn static void dp_sublcd_start(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_sublcd_start(struct stfb_info *info)
{
	dp_reg_update(info, DP_SLCD, DP_ON, DP_OFF);

	dp_operation_start(info, DP_SLCD);

	dp_reg_update(info, DP_SLCD, DP_ON, DP_ON);
	dp_reg_update(info, DP_SLCD, DP_ON, DP_OFF);
}

/**
 * @brief lcd stop function
 * @fn static void dp_lcd_stop(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_lcd_stop(struct stfb_info *info)
{
	dp_operation_lcd_stop(info, DP_MLCD);
}

static void dp_sublcd_stop(struct stfb_info *info)
{
	dp_operation_lcd_stop(info, DP_SLCD);
}

/**
 * @brief lcd video/graphic layer swap
 * @fn static void dp_lcd_set_channel_swap(struct stfb_info *info,	enum eDPOnOff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param type[in] swap type
 * @return
 *
 * @note
 */
static void dp_lcd_set_channel_swap(struct stfb_info *info,
		enum edp_onoff type)
{
	unsigned int grp_mix_on_off = 0;
	grp_mix_on_off = __raw_readl(info->dp_lcd_regs + DP_LCD_GRP_MIX_ON);

	if (type ==  0) /* Graphic/video layer, default*/
		D4_DP_LCD_CH_SWAP(grp_mix_on_off, 0);
	else
		D4_DP_LCD_CH_SWAP(grp_mix_on_off, 1);/* video layer/Graphic layer change*/
	__raw_writel(grp_mix_on_off, info->dp_lcd_regs + DP_LCD_GRP_MIX_ON);
}

/**
 * @brief gm mode select
 * @fn static void dp_lcd_set_gm_mode(struct stfb_info *info,	enum elcd_gm_ctrl_mode Mode, enum edp_onoff OnOff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param Mode[in] select mode
 * @param OnOff[in] mode on/off
 * @return
 *
 * @note
 */
static void dp_lcd_set_gm_mode(struct stfb_info *info,
		enum elcd_gm_ctrl_mode Mode, enum edp_onoff OnOff)
{
	unsigned int gmCtrl = 0;
	gmCtrl = __raw_readl(info->dp_lcd_regs + DP_LCD_GM_CTRL);

	if (Mode == GM_FICT_SW)
		D4_DP_LCD_GM_FICT_SW(gmCtrl, OnOff);
	else if (Mode == GM_BLOACK_C)
		D4_DP_LCD_GM_BLACK(gmCtrl, OnOff);
	else if (Mode == GM_GAMMA_C)
		D4_DP_LCD_GM_GAMMA(gmCtrl, OnOff);
	else if (Mode == GM_CHROMA_C)
		D4_DP_LCD_GM_CHROMA(gmCtrl, OnOff);

	__raw_writel(gmCtrl, info->dp_lcd_regs + DP_LCD_GM_CTRL);
}

/**
 * @brief Fictive/Standard GM select, gm matrix set
 * @fn static void dp_lcd_set_gm_fict_sw_type(struct stfb_info *info, struct stlcd_gm_lcd *val)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *val[in] struct stlcd_gm_lcd reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_lcd_set_gm_fict_sw_type(struct stfb_info *info,
		struct stlcd_gm_lcd *val)
{
	unsigned int fict_sw_type = 0;
	unsigned int GMfic0 = 0;
	unsigned int GMfic1 = 0;
	unsigned int GMfic2 = 0;
	unsigned int GMfic3 = 0;
	unsigned int GMfic4 = 0;
	unsigned int GMOled0 = 0;
	unsigned int GMOled1 = 0;
	unsigned int GMOled2 = 0;
	unsigned int GMOled3 = 0;
	unsigned int GMOled4 = 0;

	fict_sw_type = __raw_readl(info->dp_lcd_regs + DP_LCD_GM_CTRL);
	D4_DP_LCD_GM_FICT_SW(fict_sw_type, val->gm_fict);
	__raw_writel(fict_sw_type, info->dp_lcd_regs + DP_LCD_GM_CTRL);

	if (val->gm_fict) {
		D4_DP_LCD_GM_FIC_LOW(GMfic0, val->gm_value[0]);/* A00 */
		D4_DP_LCD_GM_FIC_UP(GMfic0, val->gm_value[1]); /* A01 */

		D4_DP_LCD_GM_FIC_LOW(GMfic1, val->gm_value[2]);/*A02*/
		D4_DP_LCD_GM_FIC_UP(GMfic1, val->gm_value[3]);/*A10*/

		D4_DP_LCD_GM_FIC_LOW(GMfic2, val->gm_value[4]);/*A11*/
		D4_DP_LCD_GM_FIC_UP(GMfic2, val->gm_value[5]);/*A12*/

		D4_DP_LCD_GM_FIC_LOW(GMfic3, val->gm_value[6]);/*A20*/
		D4_DP_LCD_GM_FIC_UP(GMfic3, val->gm_value[7]);/*A21*/

		D4_DP_LCD_GM_FIC_LOW(GMfic4, val->gm_value[8]);/*A22*/

		__raw_writel(GMfic0, info->dp_lcd_regs + DP_LCD_GM_FIC0);
		__raw_writel(GMfic1, info->dp_lcd_regs + DP_LCD_GM_FIC1);
		__raw_writel(GMfic2, info->dp_lcd_regs + DP_LCD_GM_FIC2);
		__raw_writel(GMfic3, info->dp_lcd_regs + DP_LCD_GM_FIC3);
		__raw_writel(GMfic4, info->dp_lcd_regs + DP_LCD_GM_FIC4);
	} else {
		D4_DP_LCD_GM_FIC_LOW(GMOled0, val->gm_value[0]);
		D4_DP_LCD_GM_FIC_UP(GMOled0, val->gm_value[1]);

		D4_DP_LCD_GM_FIC_LOW(GMOled1, val->gm_value[2]);
		D4_DP_LCD_GM_FIC_UP(GMOled1, val->gm_value[3]);

		D4_DP_LCD_GM_FIC_LOW(GMOled2, val->gm_value[4]);
		D4_DP_LCD_GM_FIC_UP(GMOled2, val->gm_value[5]);

		D4_DP_LCD_GM_FIC_LOW(GMOled3, val->gm_value[6]);
		D4_DP_LCD_GM_FIC_UP(GMOled3, val->gm_value[7]);

		D4_DP_LCD_GM_FIC_LOW(GMOled3, val->gm_value[8]);

		__raw_writel(GMOled0, info->dp_lcd_regs + DP_LCD_GM_OLED0);
		__raw_writel(GMOled1, info->dp_lcd_regs + DP_LCD_GM_OLED1);
		__raw_writel(GMOled2, info->dp_lcd_regs + DP_LCD_GM_OLED2);
		__raw_writel(GMOled3, info->dp_lcd_regs + DP_LCD_GM_OLED3);
		__raw_writel(GMOled4, info->dp_lcd_regs + DP_LCD_GM_OLED4);

	}
}

/**
 * @brief chroma expand gain set
 * @fn static void dp_lcd_set_gm_cg_gb_rb_gr(struct stfb_info *info,	enum elcd_gm_chroma_gain_mode chroma_gain, unsigned char value)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param chroma_gain[in] chroma gain select
 * @param value[in] value set
 * @return
 *
 * @note
 */
static void dp_lcd_set_gm_cg_gb_rb_gr(struct stfb_info *info,
		enum elcd_gm_chroma_gain_mode chroma_gain, unsigned char value)
{
	unsigned int region_chroma_gain = 0;
	region_chroma_gain = __raw_readl(info->dp_lcd_regs + DP_LCD_GM_GAIN);

	if (chroma_gain == GM_CG_GB)
		D4_DP_LCD_GM_CG_GB(region_chroma_gain, value);
	else if (chroma_gain == GM_CG_RB)
		D4_DP_LCD_GM_CG_RB(region_chroma_gain, value);
	else
		D4_DP_LCD_GM_CG_GR(region_chroma_gain, value);

	__raw_writel(region_chroma_gain, info->dp_lcd_regs + DP_LCD_GM_GAIN);
}

/**
 * @brief chroma expand gain set
 * @fn static void dp_lcd_set_gm_hth_gb_rb_gr(struct stfb_info *info,	enum elcd_gm_hue_threshold_mode hth_mode, unsigned char value)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param hth_mode[in] threshold gain select
 * @param value[in] value set
 * @return
 *
 * @note
 */
static void dp_lcd_set_gm_hth_gb_rb_gr(struct stfb_info *info,
		enum elcd_gm_hue_threshold_mode hth_mode, unsigned char value)
{
	unsigned int gm_HTH = 0;
	unsigned int GM_DELGB = 0;

	gm_HTH = __raw_readl(info->dp_lcd_regs + DP_LCD_GM_HTH);

	switch (hth_mode) {
	case GM_HTH_GB:
		D4_DP_LCD_GM_HTH_GB(gm_HTH, value);
		D4_DP_LCD_GM_DEL_LOW(GM_DELGB, 32768 / value);
		D4_DP_LCD_GM_DEL_UP(GM_DELGB, 32768 / (256 - value));
		__raw_writel(GM_DELGB, info->dp_lcd_regs + DP_LCD_GM_DELGB);
		break;

	case GM_HTH_RB:
		D4_DP_LCD_GM_HTH_RB(gm_HTH, value);
		D4_DP_LCD_GM_DEL_LOW(GM_DELGB, 32768 / value);
		D4_DP_LCD_GM_DEL_UP(GM_DELGB, 32768 / (256 - value));
		__raw_writel(GM_DELGB, info->dp_lcd_regs + DP_LCD_GM_DELRB);
		break;

	case GM_HTH_GR:
		D4_DP_LCD_GM_HTH_GR(gm_HTH, value);
		D4_DP_LCD_GM_DEL_LOW(GM_DELGB, 32768 / value);
		D4_DP_LCD_GM_DEL_UP(GM_DELGB, 32768 / (256 - value));
		__raw_writel(GM_DELGB, info->dp_lcd_regs + DP_LCD_GM_DELGR);
		break;
	}

	__raw_writel(gm_HTH, info->dp_lcd_regs + DP_LCD_GM_HTH);
}

/**
 * @brief gm fine
 * @fn static void dp_lcd_set_gm_fine(struct stfb_info *info,	enum elcd_gm_fine fine_mode, unsigned char value)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param fine_mode[in] select gm fine mode
 * @param value[in] value set
 * @return
 *
 * @note
 */
static void dp_lcd_set_gm_fine(struct stfb_info *info,
		enum elcd_gm_fine fine_mode, unsigned char value)
{
	unsigned int gmFine = 0;
	gmFine = __raw_readl(info->dp_lcd_regs + DP_LCD_GM_FINE);

	if (fine_mode == GM_BO_FINE)
		D4_DP_LCD_GM_BO_FINE(gmFine, value);
	else if (fine_mode == GM_CCG_FINE)
		D4_DP_LCD_GM_CCG_FINE(gmFine, value);
	else
		D4_DP_LCD_GM_GAM_FINE(gmFine, value);

	__raw_writel(gmFine, info->dp_lcd_regs + DP_LCD_GM_FINE);
}

/**
 * @brief gm lux
 * @fn static void dp_lcd_set_gm_lux(struct stfb_info *info, unsigned char value)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param value[in] value set
 * @return
 *
 * @note
 */
static void dp_lcd_set_gm_lux(struct stfb_info *info, unsigned char value)
{
	unsigned int gmLux = 0;

	gmLux = __raw_readl(info->dp_lcd_regs + DP_LCD_GM_LUX);
	D4_DP_LCD_GM_LUX(gmLux, value);
	__raw_writel(gmLux, info->dp_lcd_regs + DP_LCD_GM_LUX);
}

/**
 * @brief display video mirroring
 * @fn static void dp_lcd_set_vid_h_v_swap(struct stfb_info *info, enum eswap direct)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param direct[in] h/v display swap set
 * @return
 *
 * @note
 */
static void dp_lcd_set_vid_h_v_swap(struct stfb_info *info, enum eswap direct)
{
	unsigned int lcd_path_ctrl = 0;
	lcd_path_ctrl = __raw_readl(info->dp_lcd_regs + DP_LCD_PATH_CTRL);

	if (direct == DP_V_SWAP)
		D4_DP_LCD_PATH_LINE_VID_V_SWAP_ON(lcd_path_ctrl, 1);
	else if (direct == DP_H_SWAP)
		D4_DP_LCD_PATH_LINE_VID_H_SWAP_ON(lcd_path_ctrl, 1);
	else {
		D4_DP_LCD_PATH_LINE_VID_V_SWAP_ON(lcd_path_ctrl, 0);
		D4_DP_LCD_PATH_LINE_VID_H_SWAP_ON(lcd_path_ctrl, 0);
	}

	__raw_writel(lcd_path_ctrl, info->dp_lcd_regs + DP_LCD_PATH_CTRL);
}

/**
 * @brief display graphic mirroring
 * @fn static void dp_lcd_set_grp_h_v_swap(struct stfb_info *info, enum eswap direct)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param direct[in] h/v display swap set
 * @return
 *
 * @note
 */
static void dp_lcd_set_grp_h_v_swap(struct stfb_info *info, enum eswap direct)
{
	unsigned int lcd_path_ctrl = 0;

	lcd_path_ctrl = __raw_readl(info->dp_lcd_regs + DP_LCD_PATH_CTRL);

	if (direct == DP_V_SWAP)
		D4_DP_LCD_PATH_LINE_GRP_V_SWAP_ON(lcd_path_ctrl, 1);
	else if (direct == DP_H_SWAP)
		D4_DP_LCD_PATH_LINE_GRP_H_SWAP_ON(lcd_path_ctrl, 1);
	else {
		D4_DP_LCD_PATH_LINE_GRP_V_SWAP_ON(lcd_path_ctrl, 0);
		D4_DP_LCD_PATH_LINE_GRP_H_SWAP_ON(lcd_path_ctrl, 0);
	}

	__raw_writel(lcd_path_ctrl, info->dp_lcd_regs + DP_LCD_PATH_CTRL);
}

/**
 * @brief lcd dma control
 * @fn static void dp_lcd_set_mdma_control(struct stfb_info *info,	enum emdma_select select, enum edp_onoff burst_onoff, unsigned char req_num)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param select[in] select mdma
 * @param burst_onoff[in] on/off
 * @param req_num[in] set value
 * @return
 *
 * @note
 */
static void dp_lcd_set_mdma_control(struct stfb_info *info,
		enum emdma_select select, enum edp_onoff burst_onoff,
		unsigned char req_num)
{
	unsigned int mdma_control = 0;
	mdma_control = __raw_readl(info->dp_lcd_regs + DP_LCD_DMA_MODE1);

	if (select == MDMA_Y) {
		D4_DP_LCD_Y_8BURST(mdma_control, burst_onoff);
		D4_DP_LCD_PATH_Y_MOALEN(mdma_control, req_num);
	} else if (select == MDMA_C) {
		D4_DP_LCD_C_8BURST(mdma_control, burst_onoff);
		D4_DP_LCD_PATH_C_MOALEN(mdma_control, req_num);
	} else {
		D4_DP_LCD_G_8BURST(mdma_control, burst_onoff);
		D4_DP_LCD_PATH_G_MOALEN(mdma_control, req_num);
	}

	__raw_writel(mdma_control, info->dp_lcd_regs + DP_LCD_DMA_MODE1);
}

/**
 * @brief graphic background color
 * @fn static void dp_lcd_grp_set_bkg_color(struct stfb_info *info,	struct stdp_argb *argb)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *argb[in] set graphic alpah/R/G/B
 * @return
 *
 * @note
 */
static void dp_lcd_grp_set_bkg_color(struct stfb_info *info,
		struct stdp_argb *argb)
{
	unsigned int reg = 0;

	D4_DP_LCD_DMA_BKG_R(reg, argb->DP_R);
	D4_DP_LCD_DMA_BKG_G(reg, argb->DP_G);
	D4_DP_LCD_DMA_BKG_B(reg, argb->DP_B);
	D4_DP_LCD_DMA_BKG_A(reg, argb->DP_A);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_GRP1_BKG);
}

/**
 * @brief graphic alpha control
 * @fn static void dp_lcd_set_grp_alpha_ctrl_onoff(struct stfb_info *info, enum egrp_alpha_ctrl selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] set graphic alpah/R/G/B
 * @return
 *
 * @note
 */
static void dp_lcd_set_grp_alpha_ctrl_onoff(struct stfb_info *info,
		enum egrp_alpha_ctrl selection, enum edp_onoff on_off)
{
	unsigned int alpha_ctrl_onoff = 0;

	alpha_ctrl_onoff = __raw_readl(info->dp_lcd_regs + DP_LCD_MIX_ALPHA_CTRL);

	if (selection == ALP_CTRL_ON)
		D4_DP_LCD_GRP_ALP_ON(alpha_ctrl_onoff, on_off);
	else if (selection == ALP_CTRL_INV)
		D4_DP_LCD_GRP_ALP_INV_ON(alpha_ctrl_onoff, on_off);
	else if (selection == ALP_CTRL_OFS)
		D4_DP_LCD_GRP_ALP_OFS(alpha_ctrl_onoff, on_off);

	__raw_writel(alpha_ctrl_onoff, info->dp_lcd_regs + DP_LCD_MIX_ALPHA_CTRL);
}

/**
 * @brief graphic control
 * @fn static void dp_lcd_grp_set_ctrl_onoff(struct stfb_info *info,	enum egrp_ctrl_onoff selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] select graphic control Item
 * @param on_off[in] control on/off
 * @return
 *
 * @note
 */
static void dp_lcd_grp_set_ctrl_onoff(struct stfb_info *info,
		enum egrp_ctrl_onoff selection, enum edp_onoff on_off)
{
	unsigned int lcd_grp_ctrl = 0;

	lcd_grp_ctrl = __raw_readl(info->dp_lcd_regs + DP_LCD_GRP1_DMA_CTRL);

	if (selection == GRP_CTRL_ARGB_ORDER)
		D4_DP_LCD_DMA_ARGB_ORDER(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN0_FLAT_ALPHA)
		D4_DP_LCD_DMA_WIN0_FLAT_ALPHA(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN1_FLAT_ALPHA)
		D4_DP_LCD_DMA_WIN1_FLAT_ALPHA(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN2_FLAT_ALPHA)
		D4_DP_LCD_DMA_WIN2_FLAT_ALPHA(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN3_FLAT_ALPHA)
		D4_DP_LCD_DMA_WIN3_FLAT_ALPHA(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN0_ADDR_SWAP)
		D4_DP_LCD_DMA_WIN0_SWAP(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN1_ADDR_SWAP)
		D4_DP_LCD_DMA_WIN0_SWAP(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN2_ADDR_SWAP)
		D4_DP_LCD_DMA_WIN0_SWAP(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN3_ADDR_SWAP)
		D4_DP_LCD_DMA_WIN0_SWAP(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN0_ON)
		D4_DP_LCD_DMA_WIN0_ON(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN1_ON)
		D4_DP_LCD_DMA_WIN0_ON(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN2_ON)
		D4_DP_LCD_DMA_WIN0_ON(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN3_ON)
		D4_DP_LCD_DMA_WIN0_ON(lcd_grp_ctrl, on_off);

	__raw_writel(lcd_grp_ctrl, info->dp_lcd_regs + DP_LCD_GRP1_DMA_CTRL);

}

/**
 * @brief graphic Image addres set
 * @fn static void dp_lcd_set_grp_image_address(struct stfb_info *info,	enum edp_window window, unsigned int address)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param window[in] select graphic window
 * @param address[in] graphic address
 * @return
 *
 * @note
 */
static void dp_lcd_set_grp_image_address(struct stfb_info *info,
		enum edp_window window, unsigned int address)
{
	unsigned int grp_f0_addr = 0;
	unsigned int grp_f1_addr = 0;
	unsigned int base_offset = 0;

	unsigned int reg = 0;
	unsigned int stride = 0;

	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_STRIDE);
	reg = (reg >> 16) & 0x7ff;
	stride = reg * 8;

	if (window == DP_WIN0)
		base_offset = DP_LCD_GRP1_WIN0_F0_ADDR;
	else if (window == DP_WIN1)
		base_offset = DP_LCD_GRP1_WIN1_F0_ADDR;
	else if (window == DP_WIN2)
		base_offset = DP_LCD_GRP1_WIN2_F0_ADDR;
	else
		base_offset = DP_LCD_GRP1_WIN3_F0_ADDR;

	grp_f0_addr = address;
	grp_f1_addr = grp_f0_addr + stride;

	__raw_writel(grp_f0_addr, info->dp_lcd_regs + base_offset);
	__raw_writel(grp_f1_addr, info->dp_lcd_regs + base_offset + 0x4);
}

/**
 * @brief graphic Display area set
 * @fn int dp_lcd_set_grp_display_area(struct stfb_info *info,	struct stgraphic_display_area *area)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *area[in] struct stgraphic_display_area reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note Horizontal Start/End position be setted to the multiple of 2.
 */
int dp_lcd_set_grp_display_area(struct stfb_info *info,
		struct stgraphic_display_area *area)
{
	unsigned int reg = 0, base_offset = 0;
	unsigned int h_start = area->display.H_Start;
	unsigned int h_end = area->display.H_Size + area->display.H_Start;
	unsigned int v_start = area->display.V_Start;
	unsigned int v_end = area->display.V_Size + area->display.V_Start;

	if (area->win == DP_WIN0)
		base_offset = DP_LCD_GRP1_WIN0_H_POS;
	else if (area->win == DP_WIN1)
		base_offset = DP_LCD_GRP1_WIN1_H_POS;
	else if (area->win == DP_WIN2)
		base_offset = DP_LCD_GRP1_WIN2_H_POS;
	else
		base_offset = DP_LCD_GRP1_WIN3_H_POS;

	if (h_start % 2 != 0 || h_end % 2 != 0) {
		printk("H Start,H End must be the multiple of 2\n");
		return -1;
	}

	D4_DP_LCD_POS_H_START(reg, h_start);
	D4_DP_LCD_POS_H_END(reg, h_end);
	__raw_writel(reg, info->dp_lcd_regs + base_offset);
	reg = 0;
	D4_DP_LCD_POS_V_START(reg, v_start);
	D4_DP_LCD_POS_V_END(reg, v_end);
	__raw_writel(reg, info->dp_lcd_regs + base_offset + 0x04);

	return 0;

}

/**
 * @brief graphic alpha select a pixel alpha or flat alpha
 * @fn static void dp_lcd_set_grp_flat_alpha_onoff(struct stfb_info *info, enum edp_window win, unsigned char alpha_value, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param win[in] select graphic window
 * @param alpha_value[in] alpha value set
 * @param on_off[in] on= flat alpha, off =pixel alpha
 * @return
 *
 * @note
 */
static void dp_lcd_set_grp_flat_alpha_onoff(struct stfb_info *info,
		enum edp_window win, unsigned char alpha_value, enum edp_onoff on_off)
{

	unsigned int win_flat_alpha = 0;
	win_flat_alpha
			= __raw_readl(info->dp_lcd_regs + DP_LCD_GRP1_WIN_FLAT_ALPHA);

	if (win == DP_WIN0) {
		D4_DP_LCD_GRP_WIN0_ALPHA(win_flat_alpha, alpha_value);
		dp_lcd_grp_set_ctrl_onoff(info, GRP_CTRL_WIN0_FLAT_ALPHA, on_off);
	} else if (win == DP_WIN1) {
		D4_DP_LCD_GRP_WIN1_ALPHA(win_flat_alpha, alpha_value);
		dp_lcd_grp_set_ctrl_onoff(info, GRP_CTRL_WIN1_FLAT_ALPHA, on_off);
	} else if (win == DP_WIN2) {
		D4_DP_LCD_GRP_WIN2_ALPHA(win_flat_alpha, alpha_value);
		dp_lcd_grp_set_ctrl_onoff(info, GRP_CTRL_WIN2_FLAT_ALPHA, on_off);
	} else {
		D4_DP_LCD_GRP_WIN3_ALPHA(win_flat_alpha, alpha_value);
		dp_lcd_grp_set_ctrl_onoff(info, GRP_CTRL_WIN3_FLAT_ALPHA, on_off);
	}

	__raw_writel(win_flat_alpha, info->dp_lcd_regs + DP_LCD_GRP1_WIN_FLAT_ALPHA);

}

/**
 * @brief graphic alpha select a pixel alpha or flat alpha
 * @fn static void dp_lcd_set_grp_scaler(struct stfb_info *info, enum egrp_scale scale)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param scale[in] select original, double
 * @return
 *
 * @note
 */
static void dp_lcd_set_grp_scaler(struct stfb_info *info, enum egrp_scale scale)
{
	unsigned int grp_scaler_set = 0;

	if (scale == SCL_X2) {
		D4_DP_LCD_DMA_ZOOM_V(grp_scaler_set, DP_ON);
		D4_DP_LCD_DMA_ZOOM_H(grp_scaler_set, DP_ON);
	} else {
		D4_DP_LCD_DMA_ZOOM_V(grp_scaler_set, DP_OFF);
		D4_DP_LCD_DMA_ZOOM_H(grp_scaler_set, DP_OFF);
	}

	__raw_writel(grp_scaler_set, info->dp_lcd_regs + DP_LCD_GRP1_DMA_SCALER);
}

/**
 * @brief zebrra pattern on/off
 * @fn static void dp_lcd_zebra_pattern_onoff(struct stfb_info *info, struct stzebra_set *zebra_set)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *zebra_set[in] struct stzebra_set reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_lcd_zebra_pattern_onoff(struct stfb_info *info,
		struct stzebra_set *zebra_set)
{
	unsigned int zebra_info_set = 0;
	unsigned int zebra_pattern_color = 0;

	if (zebra_set->on_off == DP_OFF)
		dp_lcd_set_path_onoff(info, LCD_CTRL_ZEBRA_ON, DP_OFF);
	else {
		/* Upper/Lower threshold*/
		D4_DP_LCD_ZEBRA_THSLD_LOW(zebra_info_set, zebra_set->Lower_Threshold);
		D4_DP_LCD_ZEBRA_THSLD_UP(zebra_info_set, zebra_set->Upper_Threshold);

		/* Angle/Speed/Width*/
		D4_DP_LCD_ZEBRA_ANGLE(zebra_info_set, zebra_set->Angle);
		D4_DP_LCD_ZEBRA_SPEED(zebra_info_set, zebra_set->Speed);
		D4_DP_LCD_ZEBRA_WIDTH(zebra_info_set, zebra_set->Width);

		/* Alpha Value */
		D4_DP_LCD_ZEBRA_ALPHA(zebra_info_set, zebra_set->Pattern_Color.DP_A);

		/* Zebra Pattern Color */
		D4_DP_LCD_ZEBRA_R_PATTERN(zebra_pattern_color,
				zebra_set->Pattern_Color.DP_R);
		D4_DP_LCD_ZEBRA_G_PATTERN(zebra_pattern_color,
				zebra_set->Pattern_Color.DP_G);
		D4_DP_LCD_ZEBRA_B_PATTERN(zebra_pattern_color,
				zebra_set->Pattern_Color.DP_B);

		__raw_writel(zebra_info_set, info->dp_lcd_regs + DP_LCD_ZEBRA_CTRL0);
		__raw_writel(zebra_pattern_color, info->dp_lcd_regs
				+ DP_LCD_ZEBRA_CTRL1);
		dp_lcd_set_path_onoff(info, LCD_CTRL_ZEBRA_ON, DP_ON);
	}

}

/**
 * @brief tcc on/off
 * @fn static void dp_lcd_tcc_onoff(struct stfb_info *info, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param onoff[in] tcc on/off
 * @return
 *
 * @note
 */
static void dp_lcd_tcc_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_PATH_CTRL);

	D4_DP_LCD_PATH_TCC_ON(reg, onoff);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_PATH_CTRL);
}


/**
 * @brief tcc on/off
 * @fn static void dp_lcd_tcc_onoff(struct stfb_info *info, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param onoff[in] tcc on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_tcc_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_PATH_CTRL);

	D4_DP_LCD_PATH_TCC_ON(reg, onoff);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_PATH_CTRL);
}

/**
 * @brief video path tcc on/off
 * @fn static void dp_lcd_tcc_vid_onoff(struct stfb_info *info, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param onoff[in] tcc on/off
 * @return
 *
 * @note
 */
static void dp_lcd_tcc_vid_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_PATH_CTRL);

	D4_DP_LCD_PATH_TCC_VID_ON(reg, onoff);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_PATH_CTRL);
}

/**
 * @brief video path tcc on/off
 * @fn static void dp_sublcd_tcc_vid_onoff(struct stfb_info *info, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param onoff[in] tcc on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_tcc_vid_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_PATH_CTRL);

	D4_DP_LCD_PATH_TCC_VID_ON(reg, onoff);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_PATH_CTRL);
}


/**
 * @brief dp lcd output type set, depend on lcd pannel type
 * @fn static void dp_lcd_out_type(struct stfb_info *info, enum elcd_out_type type)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param type[in] lcd pannel type
 * @return
 *
 * @note
 */
static void dp_lcd_out_type(struct stfb_info *info, enum elcd_out_type type)
{
	unsigned int reg0 = 0, reg1 = 0;
	reg0 = __raw_readl(info->dp_lcd_regs + DP_LCD_OUT_CTRL);
	reg1 = __raw_readl(info->dp_lcd_regs + DP_LCD_OUT_DISP_SEQ);

	D4_DP_LCD_OUT_MULTI_CLK_TRANS(reg1, DP_OFF);
	D4_DP_LCD_OUT_8BIT_BYPASS(reg0, DP_OFF);

	switch (type) {
	case LCD_OUT_DELTA_8BIT:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, DATA_8);
		break;

	case LCD_OUT_STRIPE_8BIT_MSB:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_8BIT_DATA(reg0, DP_ON);
		break;

	case LCD_OUT_STRIPE_8BIT_LSB:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_8BIT_DATA(reg0, DP_OFF);
		D4_DP_LCD_OUT_MULTI_CLK_TRANS(reg1, DP_ON);
		break;

	case LCD_OUT_STRIPE_24BIT:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, DATA_24);
		break;

	case LCD_OUT_RGB_DUMMY:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x2);
		break;

	case LCD_OUT_RGB656:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x2);
		break;

	case LCD_OUT_YCBCR_8BIT_CBYCRY:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 0);
		break;

	case LCD_OUT_YCBCR_8BIT_CRYCBY:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 1);
		break;

	case LCD_OUT_YCBCR_8BIT_YCBYCR:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 0x2);
		break;

	case LCD_OUT_YCBCR_8BIT_YCRYCB:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 0x3);
		break;

	case LCD_OUT_YCBCR_16BIT_CBCR:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x2);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 0);
		break;

	case LCD_OUT_YCBCR_16BIT_CRCB:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x2);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 1);
		break;

	case LCD_OUT_CCIR656_CBCR:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_ON);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, DATA_24);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 0);
		break;

	case LCD_OUT_CCIR656_CRCB:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_ON);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, DATA_24);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 1);
		break;
	}

	__raw_writel(reg0, info->dp_lcd_regs + DP_LCD_OUT_CTRL);
	__raw_writel(reg1, info->dp_lcd_regs + DP_LCD_OUT_DISP_SEQ);
}

/**
 * @brief dp lcd pannel initialize
 * @fn void dp_lcd_panel_init(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_lcd_panel_init(struct stfb_info *info)
{
	/* LCD Cotroller Main Setting */
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_lcd_regs + DP_LCD_OUT_CTRL);

	D4_DP_LCD_OUT_8BIT_BYPASS(reg, DP_OFF);
	D4_DP_LCD_OUT_PATTERN_GEN(reg, DP_OFF);
	D4_DP_LCD_OUT_CTRL_ON(reg, DP_ON);
	D4_DP_LCD_OUT_BUS_WIDTH(reg, lcd_pannel.lcd_data_width);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_OUT_CTRL);

	/* LCD Display Sequence Setting ( Even/Odd ) : Dot Array */
	reg = 0;

	D4_DP_LCD_OUT_DISPLAY_SEQ_ODD(reg, lcd_pannel.odd_seq);
	D4_DP_LCD_OUT_DISPLAY_SEQ_EVEN(reg, lcd_pannel.even_seq);

	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_OUT_DISP_SEQ);

	/* LCD Size Setting */
	reg = 0;
	D4_DP_LCD_PATH_SIZE_H(reg, lcd_pannel.h_size);
	D4_DP_LCD_PATH_SIZE_V(reg, lcd_pannel.v_size);
	__raw_writel(reg, info->dp_lcd_regs + DP_LCD_SIZE);

	if ((lcd_pannel.lcd_data_width == DATA_8) && (lcd_pannel.type == STRIPE)) {
		/** 3 Times Transmission  */
		dp_lcd_out_type(info, LCD_OUT_STRIPE_8BIT_LSB);
	} else if ((lcd_pannel.lcd_data_width == DATA_24) && (lcd_pannel.type
			== STRIPE)) {

		/** Normal Stripe Type   */
		dp_lcd_out_type(info, LCD_OUT_STRIPE_24BIT);
	} else {
		/** Normal Delta Type   */
		dp_lcd_out_type(info, LCD_OUT_DELTA_8BIT);
	}

	mdelay(33);
}

/**
 * @brief dp lcd video initialize
 * @fn int d4_lcd_video_init_display(void)
 * @param void
 * @return
 *
 * @note
 */
int d4_lcd_video_init_display(void)
{
	struct stdp_display_area display;

	display = lcd_video.display;

	/* video stride set */
	dp_lcd_set_stride(&lcd_info, DP_VIDEO, lcd_video.vid_stride);

	/* video scantype set */
	dp_lcd_set_scantype(&lcd_info);

	/* video format set : ycbcr 420 */
	dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_YCBCR422, DP_OFF);

	/* video image address set  */
	dp_lcd_set_vid_image_address(&lcd_info, &lcd_video);

	/* video display area set */
	dp_lcd_set_vid_display_area(&lcd_info, DP_WIN0, _8bit, &display);

	/* lcd video window on */
	dp_lcd_vid_window_onoff(&lcd_info, DP_WIN0, DP_ON);

	return 0;
}

/**
 * @brief dp interrupt check
 * @fn int dp_interrupt_check(void)
 * @param void
 * @return
 *
 * @note
 */
int dp_interrupt_check(void)
{
	if (dp_get_interrupt_status(&lcd_info, INT_MLCD)) {
		clear_dp_interrupt(&lcd_info, INT_MLCD);

		printk("\n\nLCD ISR OK, lcd stop\n\n");
		dp_lcd_stop(&lcd_info);
		return 0;
	} else if (dp_get_interrupt_status(&lcd_info, INT_SLCD)) {
		clear_dp_interrupt(&lcd_info, INT_SLCD);

		printk("\n\nLCD ISR OK, sub lcd stop\n\n");
		dp_sublcd_stop(&lcd_info);
		return 0;
	} else
		return -1;
}

/**
 * @brief video stride set
 * @fn int d4_dp_lcd_video_stride(unsigned int stride)
 * @param stride[in]  stride value, it must be setted by the multiple of 8
 * @return
 *
 * @note
 */
int d4_dp_lcd_video_stride(unsigned int stride)
{
	return dp_lcd_set_stride(&lcd_info, DP_VIDEO, stride);
}

/**
 * @brief video background set
 * @fn void d4_dp_lcd_video_background(struct stdp_ycbcr *videobackground)
 * @param *videobackground[in] struct stdp_ycbcr  reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_video_background(struct stdp_ycbcr *videobackground)
{
	dp_lcd_set_vid_bkg_color(&lcd_info, videobackground);
}

/**
 * @brief video display area set
 * @fn void d4_dp_lcd_video_display_area(enum edp_window win,  enum edp_video_bit vid_bit, struct stdp_display_area display)
 * @param win[in] display window set
 * @param vid_bit[in] video bit 8/10 bit
 * @param display[in] video display area set,
 * @return
 *
 * @note
 */
void d4_dp_lcd_video_display_area(enum edp_window win,
		enum edp_video_bit vid_bit, struct stdp_display_area display)
{
	struct stdp_display_area display_set = display;

	dp_lcd_set_vid_display_area(&lcd_info, win, vid_bit, &display_set);
}

/**
 * @brief video display window on/off
 * @fn void d4_dp_lcd_video_window_onoff(enum edp_window window, enum edp_onoff on_off)
 * @param win[in] display window set
 * @param on_off[in] on/off
 * @return
 *
 * @note
 */
void d4_dp_lcd_video_window_onoff(enum edp_window window, enum edp_onoff on_off)
{
	dp_lcd_vid_window_onoff(&lcd_info, window, on_off);
}

/**
 * @brief video display window,display area, address, format, stride set, viedeo with relation all part set
 * @fn void d4_dp_lcd_video_set(struct stvideodisplay video)
 * @param video[in] struct stvideodisplay reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_video_set(struct stvideodisplay video)
{
	struct stdp_display_area display;
	struct stfb_video_info vid;
	display = video.display;
	vid.address = video.address;
	vid.bit = video.bit;

	vid.display = display;
	vid.format = video.format;
	vid.image.image_height = video.img_height;
	vid.image.image_width = video.img_width;
	vid.vid_stride = video.stride;
	vid.win = video.win;

	dp_lcd_set_vid_display_area(&lcd_info, video.win, video.bit, &display);
	dp_lcd_set_vid_image_address(&lcd_info, &vid);

	if (video.format == Ycbcr_420)
		dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_YCBCR422, DP_OFF);
	else
		dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_YCBCR422, DP_ON);

	if (video.bit == _8bit) {
		dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_VID_10BIT, DP_OFF);
	} else {
		dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_VID_10BIT, DP_ON);
	}
	dp_lcd_set_stride(&lcd_info, DP_VIDEO, vid.vid_stride);
}


void d4_dp_lcd_video_address_set(struct stvideo_address video)
{
	struct stfb_video_info vid;
	vid.address = video.address;
	vid.win = video.win;
	dp_lcd_set_vid_image_address(&lcd_info, &vid);
}

/**
 * @brief graphic stride set
 * @fn int d4_dp_lcd_graphic_stride(unsigned int stride)
 * @param stride[in] set stride  it must be setted by the multiple of 8
 * @return
 *
 * @note
 */
int d4_dp_lcd_graphic_stride(unsigned int stride)
{
	return dp_lcd_set_stride(&lcd_info, DP_GRP, stride);
}

/**
 * @brief graphic display area set
 * @fn void d4_dp_lcd_graphic_display_area(struct stgraphic_display_area *area)
 * @param *area[in] struct stgraphic_display_area reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_graphic_display_area(struct stgraphic_display_area *area)
{
	if (dp_lcd_set_grp_display_area(&lcd_info, area) < 0)
		printk(
				"[Warnig] graphic display H position the multiple of 2:[%s:%d]:%s\n",
				__FUNCTION__, __LINE__, __FILE__);
}

/**
 * @brief graphic background set
 * @fn void d4_dp_lcd_graphic_background(struct stdp_argb *argb)
 * @param *argb[in] struct stdp_argb reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_graphic_background(struct stdp_argb *argb)
{
	dp_lcd_grp_set_bkg_color(&lcd_info, argb);
}

/**
 * @brief graphic display window,display area, address, format, stride set, viedeo with relation all part set
 * @fn void d4_dp_lcd_graphic_set(struct stgrpdisplay graphic)
 * @param graphic[in] struct stgrpdisplay reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_graphic_set(struct stgrpdisplay graphic)
{
	struct stgraphic_display_area area;
	unsigned int address;

	if (graphic.win_onoff == DP_OFF)
		return;

	area.win = graphic.win;
	area.display = graphic.display;
	address = graphic.address;

	if (dp_lcd_set_grp_display_area(&lcd_info, &area) < 0)
		printk(
				"[Warnig] graphic display H position the multiple of 2:[%s:%d]:%s\n",
				__FUNCTION__, __LINE__, __FILE__);
	dp_lcd_set_grp_image_address(&lcd_info, graphic.win, address);
	dp_lcd_set_stride(&lcd_info, DP_GRP, graphic.stride);
	dp_lcd_grp_window_onoff(&lcd_info, graphic.win, graphic.win_onoff);
}


void d4_dp_lcd_graphic_address_set(struct stgrp_address graphic)
{
	dp_lcd_set_grp_image_address(&lcd_info, graphic.win, graphic.address);
}

/**
 * @brief graphic display window on/off
 * @fn void d4_dp_lcd_graphic_window_onoff(enum edp_window window,	enum edp_onoff on_off)
 * @param window[in] select graphic window
 * @param on_off[in] window on/off
 * @return
 *
 * @note
 */
void d4_dp_lcd_graphic_window_onoff(enum edp_window window,
		enum edp_onoff on_off)
{
	dp_lcd_grp_window_onoff(&lcd_info, window, on_off);
}

/**
 * @brief graphic scale set
 * @fn void d4_dp_lcd_graphic_scale(enum egrp_scale scale)
 * @param scale[in] graphic scale, original/double
 * @return
 *
 * @note
 */
void d4_dp_lcd_graphic_scale(enum egrp_scale scale)
{
	dp_lcd_set_grp_scaler(&lcd_info, scale);
}

/**
 * @brief graphic window display prority set
 * @fn void d4_dp_lcd_graphic_window_priority(struct stdp_grp_prority *priority)
 * @param *priority[in] struct stdp_grp_prority reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_graphic_window_priority(struct stdp_grp_prority *priority)
{
	dp_lcd_set_grp_priority(&lcd_info, priority);
}

/**
 * @brief dp lcd interrupt on/off
 * @fn void d4_dp_lcd_interrupt_onoff(enum edp_onoff onoff)
 * @param onoff[in] interrupt on/off
 * @return
 *
 * @note
 */
void d4_dp_lcd_interrupt_onoff(enum edp_onoff onoff)
{
	DpPRINTK("%s(),%d,onoff =%d\n", __FUNCTION__, __LINE__, onoff);
	dp_interrupt_on_off(&lcd_info, INT_MLCD, onoff);
}

/**
 * @brief dp lcd video nlc control
 * @fn void d4_dp_lcd_video_nlc(struct sttnlc_video nlcSet, enum edpnlc onoff)
 * @param nlcSet[in] struct sttnlc_video reference linux/include/vide/drime4/d4_dp_type.h
 * @param onoff[in] video nlc on/off
 * @return
 *
 * @note
 */
void d4_dp_lcd_video_nlc(struct sttnlc_video nlcSet, enum edpnlc onoff)
{
	if (onoff == NLC_VID_ON) {
		dp_nlc_vid_init(&lcd_info);
		dp_nlc_vid_set(&lcd_info, nlcSet);
		dp_nlc_control_set(&lcd_info, DP_MLCD, onoff);
	} else {
		dp_nlc_control_set(&lcd_info, DP_MLCD, onoff);
	}
}

/**
 * @brief dp lcd graphic nlc control
 * @fn void d4_dp_lcd_graphic_nlc(struct stnlc_graphic nlcset, enum edpnlc onoff)
 * @param nlcSet[in] struct stnlc_graphic reference linux/include/vide/drime4/d4_dp_type.h
 * @param onoff[in] graphic nlc on/off
 * @return
 *
 * @note
 */
void d4_dp_lcd_graphic_nlc(struct stnlc_graphic nlcset, enum edpnlc onoff)
{
	if (onoff == NLC_GRP_ON) {
		dp_nlc_grp_init(&lcd_info);

		dp_nlc_grp_set(&lcd_info, nlcset);

		dp_nlc_control_set(&lcd_info, DP_MLCD, onoff);
	} else
		dp_nlc_control_set(&lcd_info, DP_MLCD, onoff);
}

/**
 * @brief the dp lcd boundary box colortable select and the table set a color
 * @fn void d4_dp_lcd_boundarybox_table_color(enum edp_bb_color_table table, 	struct stdp_rgb *rgb_info)
 * @param table[in] boundary box color table select
 * @param *rgb_info[in] table color set
 * @return
 *
 * @note
 */
void d4_dp_lcd_boundarybox_table_color(enum edp_bb_color_table table,
		struct stdp_rgb *rgb_info)
{
	dp_lcd_vid_set_bb_color_table(&lcd_info, table, rgb_info);
}

/**
 * @brief boundary box set
 * @fn void d4_dp_lcd_bouddarybox_info_set(struct stbb_info *bb_info)
 * @param *bb_info[in] struct stbb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_bouddarybox_info_set(struct stbb_info *bb_info)
{
	struct stdp_rgb bb_out_color;
	bb_out_color.DP_R = 0;
	bb_out_color.DP_G = 0;
	bb_out_color.DP_B = 0;

	dp_lcd_set_bb_width(&lcd_info, 4, 4);
	dp_lcd_set_bb_alpha(&lcd_info, 0xf0);
	dp_lcd_set_bb_outline_color(&lcd_info, &bb_out_color);

	dp_lcd_set_bb_mode(&lcd_info, bb_info->bb_win, bb_info->style);
	dp_lcd_set_bb_color(&lcd_info, bb_info->bb_win, bb_info->table);
	dp_lcd_set_bb_outline_onoff(&lcd_info, bb_info->bb_win, DP_ON);
}

/**
 * @brief boundary box diplay area set, on/off
 * @fn void d4_dp_lcd_boudarybox_onoff(struct stbb_onoff *bb_onoff)
 * @param *bb_onoff[in]struct stbb_onoff reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_boudarybox_onoff(struct stbb_onoff *bb_onoff)
{
/*	unsigned int i;
	if (bb_onoff->bb_win_all_clear == DP_ON) {
		for (i = 0 ; i < 16; i++)
			dp_lcd_vid_bb_onoff(&lcd_info, i, DP_OFF);
	} else {*/
		dp_lcd_set_bb_display_area(&lcd_info, bb_onoff->bb_win, bb_onoff->area);
		dp_lcd_vid_bb_onoff(&lcd_info, bb_onoff->bb_win, bb_onoff->onoff);
/*	}*/
}

/**
 * @brief lcd filter set
 * @fn void d4_dp_lcd_filter_onoff(struct stlcdfilter *filter_ctrl)
 * @param *filter_ctrl[in]struct stlcdfilter reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_filter_onoff(struct stlcdfilter *filter_ctrl)
{
	dp_lcd_hrz_filter_onoff(&lcd_info, filter_ctrl->type,
			filter_ctrl->filer_value, filter_ctrl->on_off);
}

/**
 * @brief lcd filter set
 * @fn void d4_dp_lcd_filter_onoff(struct stlcdfilter *filter_ctrl)
 * @param *filter_ctrl[in]struct stlcdfilter reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_zebra_control(struct stzebra_set *zebra_set)
{
	dp_lcd_zebra_pattern_onoff(&lcd_info, zebra_set);
}

/**
 * @brief lcd GM set
 * @fn void d4_dp_lcd_gm_onoff(struct stlcd_gm_lcd val)
 * @param val[in]struct stlcd_gm_lcd reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_gm_onoff(struct stlcd_gm_lcd val)
{
	dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_OLED_GM_ON, val.gm_fict);

	dp_lcd_set_gm_fict_sw_type(&lcd_info, &val);

	if (val.gm_fict) {
		dp_lcd_set_gm_mode(&lcd_info, GM_FICT_SW, DP_ON);

		if (val.chroma_expand) {
			dp_lcd_set_gm_mode(&lcd_info, GM_BLOACK_C, DP_ON);
			dp_lcd_set_gm_mode(&lcd_info, GM_GAMMA_C, DP_ON);
			dp_lcd_set_gm_mode(&lcd_info, GM_CHROMA_C, DP_ON);

			dp_lcd_set_gm_cg_gb_rb_gr(&lcd_info, GM_CG_GB, val.gm_cg_gb);
			dp_lcd_set_gm_cg_gb_rb_gr(&lcd_info, GM_CG_RB, val.gm_cg_rb);
			dp_lcd_set_gm_cg_gb_rb_gr(&lcd_info, GM_CG_GR, val.gm_cg_gr);

			dp_lcd_set_gm_hth_gb_rb_gr(&lcd_info, GM_HTH_GB, val.gm_hth_gb);
			dp_lcd_set_gm_hth_gb_rb_gr(&lcd_info, GM_HTH_RB, val.gm_hth_rb);
			dp_lcd_set_gm_hth_gb_rb_gr(&lcd_info, GM_HTH_GR, val.gm_hth_gr);
		}

	} else {
		dp_lcd_set_gm_mode(&lcd_info, GM_BLOACK_C, DP_ON);
		dp_lcd_set_gm_mode(&lcd_info, GM_GAMMA_C, DP_ON);
		dp_lcd_set_gm_mode(&lcd_info, GM_CHROMA_C, DP_ON);
		dp_lcd_set_gm_mode(&lcd_info, GM_FICT_SW, DP_OFF);
	}

	dp_lcd_set_gm_fine(&lcd_info, GM_BO_FINE, 0x40);
	dp_lcd_set_gm_fine(&lcd_info, GM_CCG_FINE, 0x40);
	dp_lcd_set_gm_fine(&lcd_info, GM_GAME_FINE, 0x40);

	dp_lcd_set_gm_lux(&lcd_info, 0x3c);

}

/**
 * @brief lcd display mirroring
 * @fn void d4_dp_lcd_flip(enum edp_layer layer, enum eswap direct)
 * @param layer[in] select video/graphic
 * @param direct[in] horizontal/vertical direte
 * @return
 *
 * @note
 */
void d4_dp_lcd_flip(enum edp_layer layer, enum eswap direct)
{
	if (layer == DP_VIDEO)
		dp_lcd_set_vid_h_v_swap(&lcd_info, direct);
	else
		dp_lcd_set_grp_h_v_swap(&lcd_info, direct);
}

/**
 * @brief lcd TCC Lookup table set
 * @fn void d4_dp_lcd_tcc_set(struct stlcd_tcc tcc)
 * @param tcc[in] struct stlcd_tcc reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_tcc_set(struct stlcd_tcc tcc)
{
	unsigned int tcc_set_info = 0, index_num = 256, i = 0;

	dp_lcd_tcc_onoff(&lcd_info, tcc.onoff);

	if (tcc.onoff) {
		dp_lcd_tcc_vid_onoff(&lcd_info, tcc.video_only);

		for (i = 0; i < index_num; i++) {
			D4_DP_LCD_TCC_R_LUT(tcc_set_info, ((tcc.TCC_TABLE[i]>>16) & 0xFF));
			D4_DP_LCD_TCC_G_LUT(tcc_set_info, ((tcc.TCC_TABLE[i]>>8) & 0xFF));
			D4_DP_LCD_TCC_B_LUT(tcc_set_info, (tcc.TCC_TABLE[i] & 0xFF));
			D4_DP_LCD_TCC_LUT_INDEX(tcc_set_info, ((tcc.TCC_TABLE[i]>>24) & 0xFF));
			printk("tcc value =0x%x \n", tcc_set_info);
			__raw_writel(tcc_set_info, lcd_info.dp_lcd_regs + DP_TCC_LUT);
		}
	}
}


/**
 * @brief lcd limit set
 * @fn void d4_dp_lcd_limit_set(struct stdp_rgb_range range)
 * @param range[in] range set
 * @return
 *
 * @note
 */
void d4_dp_lcd_limit_set(struct stdp_rgb_range range)
{
	dp_lcd_set_grp_limtval(&lcd_info, range);
}

/**
 * @brief dp layer swap
 * @fn void d4_dp_lcd_overlayer(struct stdp_layer_change layer)
 * @param layer[in] struct stdp_layer_changereference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_overlayer(enum edp_layer over_layer)
{
	if (over_layer == DP_VIDEO)
		dp_lcd_set_channel_swap(&lcd_info, DP_ON);
	else
		dp_lcd_set_channel_swap(&lcd_info, DP_OFF);
}

/**
 * @brief dp graphic flat alpha set
 * @fn void d4_dp_lcd_graphic_window_alpha(struct stlcd_graphic_alpha alpha)
 * @param alpha[in] struct stlcd_graphic_alpha reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_lcd_graphic_window_alpha(struct stlcd_graphic_alpha alpha)
{
	dp_lcd_set_grp_flat_alpha_onoff(&lcd_info, alpha.win, alpha.alpha_value,
			alpha.on_off);
}

/**
 * @brief dp sublcd timming generating , it's related to a  panel
 * @fn static void dp_sublcd_set_timming_sig(struct stfb_info *info, struct stdp_lcd_tg tg)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param tg[in] struct stdp_lcd_tg  reference linux/drivers/video/drime4
 * @return
 *
 * @note
 */
static void dp_sublcd_set_timming_sig(struct stfb_info *info,
		struct stdp_lcd_tg tg)
{

	unsigned int reg = 0;
	D4_DP_LCD_TOTAL_PIXEL_NUM(reg, tg.total_h_size);
	D4_DP_LCD_TOTAL_LINE_NUM(reg, tg.total_v_size);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_SIZE);

	reg = 0;
	D4_DP_LCD_H_SYNC_BLK(reg, tg.h_sync_fall);
	D4_DP_LCD_H_SYNC_ACT(reg, tg.h_sync_rise);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_HSYNC);

	reg = 0;
	D4_DP_LCD_V_SYNC_BLK0(reg, tg.v_sync_fall);
	D4_DP_LCD_V_SYNC_ACT0(reg, tg.v_sync_rise);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_F0_VSYNC);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_F1_VSYNC);

	reg = 0;
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_FSYNC0);

	reg = 0;
	D4_DP_LCD_FRAME_INIT_LINE(reg, 0x0002);
	D4_DP_LCD_FRAME_FINAL_LINE(reg, tg.total_v_size);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_INFO0);

	reg = 0;
	D4_DP_LCD_FIELD_INIT_LINE(reg, 0x0002);
	D4_DP_LCD_BUF_READ_START_PIXEL(reg, tg.buf_read_h_start);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_INFO1);

	reg = 0;
	D4_DP_LCD_PRE_PIXEL_START(reg, 0x000A);
	D4_DP_LCD_PRE_PIXEL_END(reg, tg.total_h_size);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_INFO2);

	reg = 0;
	D4_DP_LCD_ACTIVE_PIXEL_START(reg, tg.enable_h_start);
	D4_DP_LCD_ACTIVE_PIXEL_END(reg, tg.enable_h_end);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_ACTIVE_PIXEL);

	reg = 0;
	D4_DP_LCD_ACTIVE_LINE_START(reg, tg.enable_v_start);
	D4_DP_LCD_ACTIVE_LINE_END(reg, tg.enable_v_end);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_ACTIVE_LINE0);

	reg = 0;
	D4_DP_LCD_ACTIVE_LINE_START(reg, tg.enable_v_start);
	D4_DP_LCD_ACTIVE_LINE_END(reg, tg.enable_v_end);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_TG_ACTIVE_LINE1);

	reg = 0;

	D4_DP_LCD_INV_DOT_CLK(reg, tg.inv_dot_clk);
	D4_DP_LCD_INV_ENABLE(reg, tg.inv_enable_clk);
	D4_DP_LCD_INV_H_SYNC(reg, tg.inv_h_sync);
	D4_DP_LCD_INV_V_SYNC(reg, tg.inv_v_sync);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_OUT_INV);

}

/**
 * @brief dp sublcd video/graphic stride set
 * @fn static int dp_sublcd_set_stride(struct stfb_info *info, enum edp_layer layer, unsigned int stride)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param tg[in] struct stdp_lcd_tg  reference linux/drivers/video/drime4
 * @param layer[in] select video/graphic
 * @param stride[in] the stride value must be the multiple of 8
 * @return
 *
 * @note
 */
static int dp_sublcd_set_stride(struct stfb_info *info, enum edp_layer layer,
		unsigned int stride)
{
	unsigned int stride_value, reg = 0;

	if (layer == DP_VIDEO)
	stride_value = stride;
	else
	stride_value = stride;

	if (stride_value == 0) {
		printk("[dp lcd] stride value = 0\n");
		return -1;
	}

	if (stride_value > 16384) {
		printk("[dp lcd] stride limits 16383\n");
		return -1;
	}
	stride_value = stride_value / 8;

	if (layer == DP_VIDEO)
	D4_DP_LCD_PATH_VID_STRIDE(reg, stride_value);
	else
	D4_DP_LCD_PATH_GRP_STRIDE(reg, stride_value);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_STRIDE);

	return 0;
}

/**
 * @brief internal function, LCD Scantype
 * @fn static void dp_sublcd_set_scantype(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_sublcd_set_scantype(struct stfb_info *info)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_PATH_CTRL);
	D4_DP_LCD_PATH_VID_SCAN_ON(reg, 1);
	D4_DP_LCD_PATH_GRP_SCAN_ON(reg, 1);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_PATH_CTRL);

}

/**
 * @brief video background color set function
 * @fn static void dp_sublcd_set_vid_bkg_color(struct stfb_info *info,	struct stdp_ycbcr *color)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *color[in]  struct stdp_ycbcr reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_sublcd_set_vid_bkg_color(struct stfb_info *info,
		struct stdp_ycbcr *color)
{
	unsigned int reg = 0;

	D4_DP_LCD_PATH_BKG_Y(reg, color->DP_Y);
	D4_DP_LCD_PATH_BKG_Cb(reg, color->DP_Cb);
	D4_DP_LCD_PATH_BKG_Cr(reg, color->DP_Cr);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_DMA_MODE2);
}

/**
 * @brief video display position set
 * @fn void dp_sublcd_set_vid_display_area(struct stfb_info *info, enum edp_window win, enum edp_video_bit vid_bit, struct stdp_display_area *display)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param win[in]  select display window
 * @param vid_bit[in]  video bit select 8/10bit
 * @param *display[in]  struct stdp_display_area reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note the horizontal start/end position must be the multiple of 2 in video 8bit <br>
 *	the horizontal start/end position must be the multiple of 16 in video 10bit
 */
void dp_sublcd_set_vid_display_area(struct stfb_info *info,
		enum edp_window win, enum edp_video_bit vid_bit,
		struct stdp_display_area *display)
{
	unsigned int base_offset = 0, reg = 0;
	unsigned int h_start = display->H_Start;
	unsigned int h_end = display->H_Size + display->H_Start;
	unsigned int v_start = display->V_Start;
	unsigned int v_end = display->V_Size + display->V_Start;

	if (win == DP_WIN0)
	base_offset = DP_LCD_WIN0_H_POS;
	else if (win == DP_WIN1)
	base_offset = DP_LCD_WIN1_H_POS;
	else if (win == DP_WIN2)
	base_offset = DP_LCD_WIN2_H_POS;
	else
	base_offset = DP_LCD_WIN3_H_POS;

	if (vid_bit == _8bit) {
		if (h_start % 2 != 0)
		h_start--;
		if (h_end % 2 != 0)
		h_end--;
	} else {
		h_start = (h_start / 16) * 16;
		h_end = (h_end / 16) * 16;
	}

	D4_DP_LCD_POS_H_START(reg, h_start);
	D4_DP_LCD_POS_H_END(reg, h_end);
	__raw_writel(reg, info->dp_sublcd_regs + base_offset);

	reg = 0;
	D4_DP_LCD_POS_V_START(reg, v_start);
	D4_DP_LCD_POS_V_END(reg, v_end);
	__raw_writel(reg, info->dp_sublcd_regs + base_offset + 0x04);
}

/**
 * @brief video display image address set
 * @fn static void dp_sublcd_set_vid_image_address(struct stfb_info *info,	struct stfb_video_info *video)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *video[in]  struct stfb_video_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_sublcd_set_vid_image_address(struct stfb_info *info,
		struct stfb_video_info *video)
{
	unsigned int base_offset = 0;
	unsigned int y0_addr = 0, y1_addr = 0, c0_addr = 0, c1_addr = 0;

	if (video->win == DP_WIN0)
	base_offset = DP_LCD_WIN0_F0_Y_ADDR;
	else if (video->win == DP_WIN1)
	base_offset = DP_LCD_WIN1_F0_Y_ADDR;
	else if (video->win == DP_WIN2)
	base_offset = DP_LCD_WIN2_F0_Y_ADDR;
	else
	base_offset = DP_LCD_WIN3_F0_Y_ADDR;

	if (video->address.y0_address % 2 != 0)
	video->address.y0_address++;

	if (video->address.c0_address % 2 != 0)
	video->address.c0_address++;

	if (video->address.y1_address % 2 != 0)
	video->address.y1_address++;

	if (video->address.c1_address % 2 != 0)
	video->address.c1_address++;

	/* y0 address */
	y0_addr = (unsigned int) video->address.y0_address;
	__raw_writel(y0_addr, info->dp_sublcd_regs + base_offset);

	/* y1 address */
	y1_addr = (unsigned int) video->address.y1_address;
	__raw_writel(y1_addr, info->dp_sublcd_regs + base_offset + 0x08);

	/* c0 address */
	c0_addr = video->address.c0_address;
	__raw_writel(c0_addr, info->dp_sublcd_regs + base_offset + 0x04);

	/* c1 address */
	c1_addr = video->address.c1_address;
	__raw_writel(c1_addr, info->dp_sublcd_regs + base_offset + 0x0c);
}

/**
 * @brief lcd path control
 * @fn static void dp_sublcd_set_path_onoff(struct stfb_info *info, enum elcd_path_onoff selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] lcd path
 * @param on_off[in] lcd path on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_set_path_onoff(struct stfb_info *info,
		enum elcd_path_onoff selection, enum edp_onoff on_off)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_PATH_CTRL);

	if (selection == LCD_CTRL_LCD_DMAC_ON)
	D4_DP_LCD_PATH_DMA_ON(reg, on_off);

	else if (selection == LCD_CTRL_VID_HRZFLT_ON)
	D4_DP_LCD_PATH_HRZ_FILTER_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_MIX_ON)
	D4_DP_LCD_PATH_GRP_MIX_ON(reg, on_off);

	else if (selection == LCD_CTRL_RGB2YCBCR_ON)
	D4_DP_LCD_PATH_GRP_RGB2YCBCR_ON(reg, on_off);

	else if (selection == LCD_CTRL_RGB2YCBCR_ON)
	D4_DP_LCD_PATH_GRP_RGB2YCBCR_ON(reg, on_off);

	else if (selection == LCD_CTRL_ZEBRA_ON)
	D4_DP_LCD_PATH_GRP_ZEBRA_ON(reg, on_off);

	else if (selection == LCD_CTRL_DITHER_ON)
	D4_DP_LCD_PATH_GRP_DITHER_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_DMAC_ON)
	D4_DP_LCD_PATH_GRP_DMA_CTRL_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_HRZFLT_ON)
	D4_DP_LCD_PATH_GRP_HRZ_FILTER_ON(reg, on_off);

	else if (selection == LCD_CTRL_OLED_GM_ON)
	D4_DP_LCD_PATH_OLED_GM_ON(reg, on_off);

	else if (selection == LCD_CTRL_TCC_ON)
	D4_DP_LCD_PATH_TCC_ON(reg, on_off);

	else if (selection == LCD_CTRL_3D_VID)
	D4_DP_LCD_PATH_3D_ON(reg, on_off);

	else if (selection == LCD_CTRL_SD_ON)
	D4_DP_LCD_PATH_SD_ON(reg, on_off);

	else if (selection == LCD_CTRL_YCBCR422)
	D4_DP_LCD_PATH_YCBCR422_ON(reg, on_off);

	else if (selection == LCD_CTRL_ROTATE_LR)
	D4_DP_LCD_PATH_ROTATE_ON(reg, on_off);

	else if (selection == LCD_CTRL_VID_10BIT)
	D4_DP_LCD_PATH_VID_10BIT_ON(reg, on_off);

	else if (selection == LCD_CTRL_TCC_VID)
	D4_DP_LCD_PATH_TCC_VID_ON(reg, on_off);

	else if (selection == LCD_CTRL_VID_SCAN)
	D4_DP_LCD_PATH_VID_SCAN_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_SCAN)
	D4_DP_LCD_PATH_GRP_SCAN_ON(reg, on_off);

	else if (selection == LCD_CTRL_BBOX_2X)
	D4_DP_LCD_PATH_BBOX_2X_ON(reg, on_off);

	else if (selection == LCD_CTRL_LINE_INV)
	D4_DP_LCD_PATH_LINE_INV_ON(reg, on_off);

	else if (selection == LCD_CTRL_VID_HSWAP)
	D4_DP_LCD_PATH_LINE_VID_H_SWAP_ON(reg, on_off);

	else if (selection == LCD_CTRL_VID_VSWAP)
	D4_DP_LCD_PATH_LINE_VID_V_SWAP_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_HSWAP)
	D4_DP_LCD_PATH_LINE_GRP_H_SWAP_ON(reg, on_off);

	else if (selection == LCD_CTRL_GRP_VSWAP)
	D4_DP_LCD_PATH_LINE_GRP_V_SWAP_ON(reg, on_off);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_PATH_CTRL);
}

/**
 * @brief lcd video window on/off set
 * @fn static void dp_sublcd_vid_window_onoff(struct stfb_info *info, enum edp_window window, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param window[in] select video window
 * @param on_off[in] lcd path on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_vid_window_onoff(struct stfb_info *info,
		enum edp_window window, enum edp_onoff on_off)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_DMA_MODE0);

	if (window == DP_WIN0)
	D4_DP_LCD_DMA_WIN0_ON(reg, on_off);
	else if (window == DP_WIN1)
	D4_DP_LCD_DMA_WIN1_ON(reg, on_off);
	else if (window == DP_WIN2)
	D4_DP_LCD_DMA_WIN2_ON(reg, on_off);
	else
	D4_DP_LCD_DMA_WIN3_ON(reg, on_off);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_DMA_MODE0);
}

/**
 * @brief graphic Display area set
 * @fn int dp_sublcd_set_grp_display_area(struct stfb_info *info,	struct stgraphic_display_area *area)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *area[in] struct stgraphic_display_area reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note Horizontal Start/End position be setted to the multiple of 2.
 */
int dp_sublcd_set_grp_display_area(struct stfb_info *info,
		struct stgraphic_display_area *area)
{
	unsigned int reg = 0, base_offset = 0;
	unsigned int h_start = area->display.H_Start;
	unsigned int h_end = area->display.H_Size + area->display.H_Start;
	unsigned int v_start = area->display.V_Start;
	unsigned int v_end = area->display.V_Size + area->display.V_Start;

	if (area->win == DP_WIN0)
	base_offset = DP_LCD_GRP1_WIN0_H_POS;
	else if (area->win == DP_WIN1)
	base_offset = DP_LCD_GRP1_WIN1_H_POS;
	else if (area->win == DP_WIN2)
	base_offset = DP_LCD_GRP1_WIN2_H_POS;
	else
	base_offset = DP_LCD_GRP1_WIN3_H_POS;

	if (h_start % 2 != 0 || h_end % 2 != 0) {
		printk("Horizontal Start/End must be the multiple of 2\n");
		return -1;
	}

	D4_DP_LCD_POS_H_START(reg, h_start);
	D4_DP_LCD_POS_H_END(reg, h_end);
	__raw_writel(reg, info->dp_sublcd_regs + base_offset);

	reg = 0;
	D4_DP_LCD_POS_V_START(reg, v_start);
	D4_DP_LCD_POS_V_END(reg, v_end);
	__raw_writel(reg, info->dp_sublcd_regs + base_offset + 0x04);

	return 0;

}

/**
 * @brief dp sub lcd video initialize
 * @fn int d4_sublcd_video_init_display(void)
 * @param void
 * @return
 *
 * @note
 */
int d4_sublcd_video_init_display(void)
{
	struct stdp_display_area display;

	display = lcd_video.display;

	/* video stride set */
	dp_sublcd_set_stride(&lcd_info, DP_VIDEO, lcd_video.vid_stride);

	/* video scantype set */
	dp_sublcd_set_scantype(&lcd_info);

	/* video format set : ycbcr 420 */
	dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_YCBCR422, DP_OFF);

	/* video image address set  */
	dp_sublcd_set_vid_image_address(&lcd_info, &lcd_video);

	/* video display area set */
	dp_sublcd_set_vid_display_area(&lcd_info, DP_WIN0, _8bit, &display);

	/* lcd video window on */
	dp_sublcd_vid_window_onoff(&lcd_info, DP_WIN0, DP_OFF);

	return 0;
}

/**
 * @brief sub lcd graphic window on/off set
 * @fn void dp_sublcd_grp_window_onoff(struct stfb_info *info, enum edp_window window, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param window[in] select video window
 * @param on_off[in] lcd path on/off
 * @return
 *
 * @note
 */
void dp_sublcd_grp_window_onoff(struct stfb_info *info, enum edp_window window,
		enum edp_onoff on_off)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_GRP1_DMA_CTRL);

	if (window == DP_WIN0)
	D4_DP_LCD_DMA_WIN0_ON(reg, on_off);
	else if (window == DP_WIN1)
	D4_DP_LCD_DMA_WIN1_ON(reg, on_off);
	else if (window == DP_WIN2)
	D4_DP_LCD_DMA_WIN2_ON(reg, on_off);
	else
	D4_DP_LCD_DMA_WIN3_ON(reg, on_off);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_GRP1_DMA_CTRL);
}

/**
 * @brief graphic window prority set
 * @fn static void dp_sublcd_grp_win_prority(struct stfb_info *info, unsigned char win0, unsigned char win1, unsigned char win2, unsigned char win3)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param win0[in] graphic window0
 * @param win1[in] graphic window1
 * @param win2[in] graphic window2
 * @param win3[in] graphic window3
 * @return
 *
 * @note
 */
static void dp_sublcd_grp_win_prority(struct stfb_info *info,
		unsigned char win0, unsigned char win1, unsigned char win2,
		unsigned char win3)
{
	unsigned int reg = 0;
	D4_DP_LCD_WIN0_PRORITY(reg, win0);
	D4_DP_LCD_WIN1_PRORITY(reg, win1);
	D4_DP_LCD_WIN2_PRORITY(reg, win2);
	D4_DP_LCD_WIN3_PRORITY(reg, win3);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_GRP1_WIN_PRIORITY);
}

/**
 * @brief graphic window prority set
 * @fnstatic void dp_sublcd_set_grp_priority(struct stfb_info *info, struct stdp_grp_prority *priority)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *priority[in] prority winow set
 * @return
 *
 * @note
 */
static void dp_sublcd_set_grp_priority(struct stfb_info *info,
		struct stdp_grp_prority *priority)
{

	unsigned char win_num, Sequence[4], Max = 3;

	for (win_num = 0; win_num < 4; win_num++) {
		if (priority->First_Priority == win_num)
		Sequence[win_num] = Max;
		if (priority->Second_Priority == win_num)
		Sequence[win_num] = Max - 1;
		if (priority->Third_Priotrity == win_num)
		Sequence[win_num] = Max - 2;
		if (priority->Fourth_Priority == win_num)
		Sequence[win_num] = Max - 3;
	}

	dp_sublcd_grp_win_prority(info, Sequence[0], Sequence[1], Sequence[2],
			Sequence[3]);
}

/**
 * @brief graphic alpha select a pixel alpha or flat alpha
 * @fn static void dp_sublcd_set_grp_scaler(struct stfb_info *info, enum egrp_scale scale)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param scale[in] select original, double
 * @return
 *
 * @note
 */
static void dp_sublcd_set_grp_scaler(struct stfb_info *info,
		enum egrp_scale scale)
{
	unsigned int grp_scaler_set = 0;

	if (scale == SCL_X2) {
		D4_DP_LCD_DMA_ZOOM_V(grp_scaler_set, DP_ON);
		D4_DP_LCD_DMA_ZOOM_H(grp_scaler_set, DP_ON);
	} else {
		D4_DP_LCD_DMA_ZOOM_V(grp_scaler_set, DP_OFF);
		D4_DP_LCD_DMA_ZOOM_H(grp_scaler_set, DP_OFF);
	}
	__raw_writel(grp_scaler_set, info->dp_sublcd_regs + DP_LCD_GRP1_DMA_SCALER);
}

/**
 * @brief zebrra pattern on/off
 * @fn static void dp_sublcd_zebra_pattern_onoff(struct stfb_info *info, struct stzebra_set *zebra_set)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *zebra_set[in] struct stzebra_set reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_sublcd_zebra_pattern_onoff(struct stfb_info *info,
		struct stzebra_set *zebra_set)
{
	unsigned int zebra_info_set = 0;
	unsigned int zebra_pattern_color = 0;

	if (zebra_set->on_off == DP_OFF)
	dp_sublcd_set_path_onoff(info, LCD_CTRL_ZEBRA_ON, DP_OFF);
	else {
		/* Upper/Lower threshold*/
		D4_DP_LCD_ZEBRA_THSLD_LOW(zebra_info_set, zebra_set->Lower_Threshold);
		D4_DP_LCD_ZEBRA_THSLD_UP(zebra_info_set, zebra_set->Upper_Threshold);

		/* Angle/Speed/Width*/
		D4_DP_LCD_ZEBRA_ANGLE(zebra_info_set, zebra_set->Angle);
		D4_DP_LCD_ZEBRA_SPEED(zebra_info_set, zebra_set->Speed);
		D4_DP_LCD_ZEBRA_WIDTH(zebra_info_set, zebra_set->Width);

		/* Alpha Value */
		D4_DP_LCD_ZEBRA_ALPHA(zebra_info_set, zebra_set->Pattern_Color.DP_A);

		/* Zebra Pattern Color */
		D4_DP_LCD_ZEBRA_R_PATTERN(zebra_pattern_color, zebra_set->Pattern_Color.DP_R);
		D4_DP_LCD_ZEBRA_G_PATTERN(zebra_pattern_color, zebra_set->Pattern_Color.DP_G);
		D4_DP_LCD_ZEBRA_B_PATTERN(zebra_pattern_color, zebra_set->Pattern_Color.DP_B);

		__raw_writel(zebra_info_set, info->dp_sublcd_regs + DP_LCD_ZEBRA_CTRL0);
		__raw_writel(zebra_pattern_color, info->dp_sublcd_regs
				+ DP_LCD_ZEBRA_CTRL1);
		dp_sublcd_set_path_onoff(info, LCD_CTRL_ZEBRA_ON, DP_ON);
	}

}

/**
 * @brief graphic Image addres set
 * @fn static void dp_sublcd_set_grp_image_address(struct stfb_info *info,	enum edp_window window, unsigned int address)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param window[in] select graphic window
 * @param address[in] graphic address
 * @return
 *
 * @note
 */
static void dp_sublcd_set_grp_image_address(struct stfb_info *info,
		enum edp_window window, unsigned int address)
{
	unsigned int grp_f0_addr = 0;
	unsigned int grp_f1_addr = 0;
	unsigned int base_offset = 0;

	unsigned int reg = 0;
	unsigned int stride = 0;

	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_STRIDE);
	reg = (reg >> 16) & 0x7ff;
	stride = reg * 8;

	if (window == DP_WIN0)
	base_offset = DP_LCD_GRP1_WIN0_F0_ADDR;
	else if (window == DP_WIN1)
	base_offset = DP_LCD_GRP1_WIN1_F0_ADDR;
	else if (window == DP_WIN2)
	base_offset = DP_LCD_GRP1_WIN2_F0_ADDR;
	else
	base_offset = DP_LCD_GRP1_WIN3_F0_ADDR;

	grp_f0_addr = address;
	grp_f1_addr = grp_f0_addr + stride;

	__raw_writel(grp_f0_addr, info->dp_sublcd_regs + base_offset);
	__raw_writel(grp_f1_addr, info->dp_sublcd_regs + base_offset + 0x4);
}

/**
 * @brief graphic mix alpha offset
 * @fn static void dp_sublcd_set_grp_alpha_offset_val(struct stfb_info *info, unsigned char alpha_offset_val)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param alpha_offset_val[in] graphic alpha offset value
 * @return
 *
 * @note
 */
static void dp_sublcd_set_grp_alpha_offset_val(struct stfb_info *info,
		unsigned char alpha_offset_val)
{
	unsigned int alpha_off_set_val = 0;

	alpha_off_set_val = __raw_readl(info->dp_sublcd_regs
			+ DP_LCD_MIX_ALPHA_CTRL);

	if (alpha_off_set_val & 0x04)
	D4_DP_LCD_PATH_ALP_OFS_4BIT(alpha_off_set_val, alpha_offset_val);
	else
	D4_DP_LCD_PATH_ALP_OFS_4BIT(alpha_off_set_val, 0);

	__raw_writel(alpha_off_set_val, info->dp_sublcd_regs
			+ DP_LCD_MIX_ALPHA_CTRL);
}

/**
 * @brief display video mirroring
 * @fn static void dp_sublcd_set_vid_h_v_swap(struct stfb_info *info, enum eswap direct)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param direct[in] h/v display swap set
 * @return
 *
 * @note
 */
static void dp_sublcd_set_vid_h_v_swap(struct stfb_info *info,
		enum eswap direct)
{
	unsigned int lcd_path_ctrl = 0;
	lcd_path_ctrl = __raw_readl(info->dp_sublcd_regs + DP_LCD_PATH_CTRL);

	if (direct == DP_V_SWAP)
	D4_DP_LCD_PATH_LINE_VID_V_SWAP_ON(lcd_path_ctrl, 1);
	else if (direct == DP_H_SWAP)
	D4_DP_LCD_PATH_LINE_VID_H_SWAP_ON(lcd_path_ctrl, 1);
	else {
		D4_DP_LCD_PATH_LINE_VID_V_SWAP_ON(lcd_path_ctrl, 0);
		D4_DP_LCD_PATH_LINE_VID_H_SWAP_ON(lcd_path_ctrl, 0);
	}

	__raw_writel(lcd_path_ctrl, info->dp_sublcd_regs + DP_LCD_PATH_CTRL);
}

/**
 * @brief display graphic mirroring
 * @fn static void dp_sublcd_set_grp_h_v_swap(struct stfb_info *info, enum eswap direct)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param direct[in] h/v display swap set
 * @return
 *
 * @note
 */
static void dp_sublcd_set_grp_h_v_swap(struct stfb_info *info,
		enum eswap direct)
{
	unsigned int lcd_path_ctrl = 0;

	lcd_path_ctrl = __raw_readl(info->dp_sublcd_regs + DP_LCD_PATH_CTRL);

	if (direct == DP_V_SWAP)
	D4_DP_LCD_PATH_LINE_VID_V_SWAP_ON(lcd_path_ctrl, 1);
	else if (direct == DP_H_SWAP)
	D4_DP_LCD_PATH_LINE_VID_H_SWAP_ON(lcd_path_ctrl, 1);
	else {
		D4_DP_LCD_PATH_LINE_VID_V_SWAP_ON(lcd_path_ctrl, 0);
		D4_DP_LCD_PATH_LINE_VID_H_SWAP_ON(lcd_path_ctrl, 0);
	}

	__raw_writel(lcd_path_ctrl, info->dp_sublcd_regs + DP_LCD_PATH_CTRL);
}

/**
 * @brief graphic background color
 * @fn static void dp_sublcd_grp_set_bkg_color(struct stfb_info *info,	struct stdp_argb *argb)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *argb[in] set graphic alpah/R/G/B
 * @return
 *
 * @note
 */
static void dp_sublcd_grp_set_bkg_color(struct stfb_info *info,
		struct stdp_argb *argb)
{
	unsigned int reg = 0;

	D4_DP_LCD_DMA_BKG_R(reg, argb->DP_R);
	D4_DP_LCD_DMA_BKG_G(reg, argb->DP_G);
	D4_DP_LCD_DMA_BKG_B(reg, argb->DP_B);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_GRP1_BKG);
}

/**
 * @brief sublcd video/graphic layer swap
 * @fn static void dp_sublcd_set_channel_swap(struct stfb_info *info,	enum elcd_layer_swap type)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param type[in] swap type
 * @return
 *
 * @note
 */
static void dp_sublcd_set_channel_swap(struct stfb_info *info,
		enum edp_onoff type)
{
	unsigned int grp_mix_on_off = 0;
	grp_mix_on_off = __raw_readl(info->dp_sublcd_regs + DP_LCD_GRP_MIX_ON);

	if (type ==  0) /* Graphic/video layer, default*/
		D4_DP_LCD_CH_SWAP(grp_mix_on_off, 0);
	else
		D4_DP_LCD_CH_SWAP(grp_mix_on_off, 1);/* video layer/Graphic layer change*/

	__raw_writel(grp_mix_on_off, info->dp_sublcd_regs + DP_LCD_GRP_MIX_ON);
}

/**
 * @brief boundary box color table set
 * @fn static void dp_sublcd_vid_set_bb_color_table(struct stfb_info *info,	enum edp_bb_color_table table, struct stdp_rgb *rgb_info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param table[in] select boudary box color table
 * @param *rgb_info[in] boundary box color table  set color
 * @return
 *
 * @note
 */
static void dp_sublcd_vid_set_bb_color_table(struct stfb_info *info,
		enum edp_bb_color_table table, struct stdp_rgb *rgb_info)
{
	unsigned int reg = 0;

	D4_DP_LCD_BB_B_COLOR(reg, rgb_info->DP_B);
	D4_DP_LCD_BB_G_COLOR(reg, rgb_info->DP_G);
	D4_DP_LCD_BB_R_COLOR(reg, rgb_info->DP_R);

	if (table == DP_BB_COLOR_TABLE_0)
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_COLOR_0);
	else if (table == DP_BB_COLOR_TABLE_1)
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_COLOR_1);
	else if (table == DP_BB_COLOR_TABLE_2)
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_COLOR_2);
	else if (table == DP_BB_COLOR_TABLE_3)
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_COLOR_3);
}

/**
 * @brief boundary box boundary horizontal, vertical gap size set
 * @fn static void dp_sublcd_set_bb_width(struct stfb_info *info, unsigned char h_width, unsigned char v_width)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param h_width[in] boudary box boudary horizontal gap size
 * @param v_width[in] boudary box boudary vertical gap size
 * @return
 *
 * @note
 */
static void dp_sublcd_set_bb_width(struct stfb_info *info,
		unsigned char h_width, unsigned char v_width)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_BB_WIDTH_ALPHA);
	D4_DP_LCD_BB_H_WIDTH(reg, h_width);
	D4_DP_LCD_BB_V_WIDTH(reg, v_width);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_WIDTH_ALPHA);
}

/**
 * @brief boundary box boundary transparent set
 * @fn static void dp_sublcd_set_bb_alpha(struct stfb_info *info, unsigned char alpha)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param alpha[in] transparent value
 * @return
 *
 * @note
 */
static void dp_sublcd_set_bb_alpha(struct stfb_info *info, unsigned char alpha)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_BB_WIDTH_ALPHA);
	D4_DP_LCD_BB_ALPHA(reg, alpha);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_WIDTH_ALPHA);
}

/**
 * @brief boundary box outline color
 * @fn static void dp_sublcd_set_bb_outline_color(struct stfb_info *info, struct stdp_rgb *bb_out_color)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param *bb_out_color[in] struct stdp_rgb reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
static void dp_sublcd_set_bb_outline_color(struct stfb_info *info,
		struct stdp_rgb *bb_out_color)
{
	unsigned int reg = 0;
	D4_DP_LCD_BB_OUTLINE_B(reg, bb_out_color->DP_B);
	D4_DP_LCD_BB_OUTLINE_G(reg, bb_out_color->DP_G);
	D4_DP_LCD_BB_OUTLINE_R(reg, bb_out_color->DP_R);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_OUTLINE_COLOR);
}

/**
 * @brief boundary box outline on/off
 * @fn static void dp_sublcd_set_bb_outline_onoff(struct stfb_info *info, enum edp_bb_box selection, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] boundary box select
 * @param onoff[in] boundary box outline on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_set_bb_outline_onoff(struct stfb_info *info,
		enum edp_bb_box selection, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_BB_WIDTH_ALPHA);

	if (selection == DP_BB_00)
		D4_DP_LCD_BB_OUTLINE_00(reg, onoff);
	else if (selection == DP_BB_01)
		D4_DP_LCD_BB_OUTLINE_01(reg, onoff);
	else if (selection == DP_BB_02)
		D4_DP_LCD_BB_OUTLINE_02(reg, onoff);
	else if (selection == DP_BB_03)
		D4_DP_LCD_BB_OUTLINE_03(reg, onoff);
	else if (selection == DP_BB_04)
		D4_DP_LCD_BB_OUTLINE_04(reg, onoff);
	else if (selection == DP_BB_05)
		D4_DP_LCD_BB_OUTLINE_05(reg, onoff);
	else if (selection == DP_BB_06)
		D4_DP_LCD_BB_OUTLINE_06(reg, onoff);
	else if (selection == DP_BB_07)
		D4_DP_LCD_BB_OUTLINE_07(reg, onoff);
	else if (selection == DP_BB_08)
		D4_DP_LCD_BB_OUTLINE_08(reg, onoff);
	else if (selection == DP_BB_09)
		D4_DP_LCD_BB_OUTLINE_09(reg, onoff);
	else if (selection == DP_BB_10)
		D4_DP_LCD_BB_OUTLINE_10(reg, onoff);
	else if (selection == DP_BB_11)
		D4_DP_LCD_BB_OUTLINE_11(reg, onoff);
	else if (selection == DP_BB_12)
		D4_DP_LCD_BB_OUTLINE_12(reg, onoff);
	else if (selection == DP_BB_13)
		D4_DP_LCD_BB_OUTLINE_13(reg, onoff);
	else if (selection == DP_BB_14)
		D4_DP_LCD_BB_OUTLINE_14(reg, onoff);
	else
		D4_DP_LCD_BB_OUTLINE_15(reg, onoff);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_WIDTH_ALPHA);
}

/**
 * @brief boundary box rectangle style select
 * @fn static void dp_sublcd_set_bb_mode(struct stfb_info *info,	enum edp_bb_box selection, enum edp_bb_style_set style)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] boundary box select
 * @param onoff[in] boundary box outline on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_set_bb_mode(struct stfb_info *info,
		enum edp_bb_box selection, enum edp_bb_style_set style)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_BB_MODE_ON);

	if (selection == DP_BB_00)
		D4_DP_LCD_BB00_MODE_ON(reg, style);
	else if (selection == DP_BB_01)
		D4_DP_LCD_BB01_MODE_ON(reg, style);
	else if (selection == DP_BB_02)
		D4_DP_LCD_BB02_MODE_ON(reg, style);
	else if (selection == DP_BB_03)
		D4_DP_LCD_BB03_MODE_ON(reg, style);
	else if (selection == DP_BB_04)
		D4_DP_LCD_BB04_MODE_ON(reg, style);
	else if (selection == DP_BB_05)
		D4_DP_LCD_BB05_MODE_ON(reg, style);
	else if (selection == DP_BB_06)
		D4_DP_LCD_BB06_MODE_ON(reg, style);
	else if (selection == DP_BB_07)
		D4_DP_LCD_BB07_MODE_ON(reg, style);
	else if (selection == DP_BB_08)
		D4_DP_LCD_BB08_MODE_ON(reg, style);
	else if (selection == DP_BB_09)
		D4_DP_LCD_BB09_MODE_ON(reg, style);
	else if (selection == DP_BB_10)
		D4_DP_LCD_BB10_MODE_ON(reg, style);
	else if (selection == DP_BB_11)
		D4_DP_LCD_BB11_MODE_ON(reg, style);
	else if (selection == DP_BB_12)
		D4_DP_LCD_BB12_MODE_ON(reg, style);
	else if (selection == DP_BB_13)
		D4_DP_LCD_BB13_MODE_ON(reg, style);
	else if (selection == DP_BB_14)
		D4_DP_LCD_BB14_MODE_ON(reg, style);
	else
		D4_DP_LCD_BB15_MODE_ON(reg, style);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_MODE_ON);
}

/**
 * @brief boundary box select color table select
 * @fn static void dp_sublcd_set_bb_color(struct stfb_info *info, enum edp_bb_box selection, enum edp_bb_color_table color)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] boundary box select
 * @param color[in] color table select
 * @return
 *
 * @note
 */
static void dp_sublcd_set_bb_color(struct stfb_info *info,
		enum edp_bb_box selection, enum edp_bb_color_table color)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_BB_COLOR_SELECT);

	if (selection == DP_BB_00)
	D4_DP_LCD_BB_COLOR_SELECT_00(reg, 1);
	else if (selection == DP_BB_01)
	D4_DP_LCD_BB_COLOR_SELECT_01(reg, 1);
	else if (selection == DP_BB_02)
	D4_DP_LCD_BB_COLOR_SELECT_02(reg, 1);
	else if (selection == DP_BB_03)
	D4_DP_LCD_BB_COLOR_SELECT_03(reg, 1);
	else if (selection == DP_BB_04)
	D4_DP_LCD_BB_COLOR_SELECT_04(reg, 1);
	else if (selection == DP_BB_05)
	D4_DP_LCD_BB_COLOR_SELECT_05(reg, 1);
	else if (selection == DP_BB_06)
	D4_DP_LCD_BB_COLOR_SELECT_06(reg, 1);
	else if (selection == DP_BB_07)
	D4_DP_LCD_BB_COLOR_SELECT_07(reg, 1);
	else if (selection == DP_BB_08)
	D4_DP_LCD_BB_COLOR_SELECT_08(reg, 1);
	else if (selection == DP_BB_09)
	D4_DP_LCD_BB_COLOR_SELECT_09(reg, 1);
	else if (selection == DP_BB_10)
	D4_DP_LCD_BB_COLOR_SELECT_10(reg, 1);
	else if (selection == DP_BB_11)
	D4_DP_LCD_BB_COLOR_SELECT_11(reg, 1);
	else if (selection == DP_BB_12)
	D4_DP_LCD_BB_COLOR_SELECT_12(reg, 1);
	else if (selection == DP_BB_13)
	D4_DP_LCD_BB_COLOR_SELECT_13(reg, 1);
	else if (selection == DP_BB_14)
	D4_DP_LCD_BB_COLOR_SELECT_14(reg, 1);
	else
	D4_DP_LCD_BB_COLOR_SELECT_15(reg, 1);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_COLOR_SELECT);
}

/**
 * @brief boundary box dislpay area set
 * @fn static void dp_sublcd_set_bb_display_area(struct stfb_info *info, enum edp_bb_box selection, struct stdp_display_area area)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] boundary box select
 * @param area[in] display area set
 * @return
 *
 * @note
 */
static void dp_sublcd_set_bb_display_area(struct stfb_info *info,
		enum edp_bb_box selection, struct stdp_display_area area)
{
	unsigned int h_position = 0, v_position = 0;

	D4_DP_LCD_BB_H_START(h_position, area.H_Start);
	D4_DP_LCD_BB_H_END(h_position, area.H_Size+area.H_Start);

	D4_DP_LCD_BB_V_START(v_position, area.V_Start);
	D4_DP_LCD_BB_V_END(v_position, area.V_Size+area.V_Start);

	switch (selection) {
	case DP_BB_00:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB00_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB00_V_POS);
		break;
	case DP_BB_01:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB01_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB01_V_POS);
		break;
	case DP_BB_02:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB02_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB02_V_POS);
		break;
	case DP_BB_03:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB03_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB03_V_POS);
		break;
	case DP_BB_04:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB04_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB04_V_POS);
		break;
	case DP_BB_05:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB05_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB05_V_POS);
		break;
	case DP_BB_06:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB06_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB06_V_POS);
		break;
	case DP_BB_07:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB07_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB07_V_POS);
		break;
	case DP_BB_08:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB08_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB08_V_POS);
		break;
	case DP_BB_09:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB09_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB09_V_POS);
		break;
	case DP_BB_10:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB10_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB10_V_POS);
		break;
	case DP_BB_11:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB11_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB11_V_POS);
		break;
	case DP_BB_12:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB12_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB12_V_POS);
		break;
	case DP_BB_13:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB13_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB13_V_POS);
		break;
	case DP_BB_14:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB14_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB14_V_POS);
		break;
	case DP_BB_15:
		__raw_writel(h_position, info->dp_sublcd_regs + DP_LCD_BB15_H_POS);
		__raw_writel(v_position, info->dp_sublcd_regs + DP_LCD_BB15_V_POS);
		break;
	}
}

/**
 * @brief boundary box on/off
 * @fn static void dp_sublcd_vid_bb_onoff(struct stfb_info *info,	enum edp_bb_box selection, enum edp_onoff onoff)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] boundary box select
 * @param onoff[in] on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_vid_bb_onoff(struct stfb_info *info,
		enum edp_bb_box selection, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_BB_MODE_ON);

	if (selection == DP_BB_00)
		D4_DP_LCD_BB00_ON(reg, onoff);
	else if (selection == DP_BB_01)
		D4_DP_LCD_BB01_ON(reg, onoff);
	else if (selection == DP_BB_02)
		D4_DP_LCD_BB02_ON(reg, onoff);
	else if (selection == DP_BB_03)
		D4_DP_LCD_BB03_ON(reg, onoff);
	else if (selection == DP_BB_04)
		D4_DP_LCD_BB04_ON(reg, onoff);
	else if (selection == DP_BB_05)
		D4_DP_LCD_BB05_ON(reg, onoff);
	else if (selection == DP_BB_06)
		D4_DP_LCD_BB06_ON(reg, onoff);
	else if (selection == DP_BB_07)
		D4_DP_LCD_BB07_ON(reg, onoff);
	else if (selection == DP_BB_08)
		D4_DP_LCD_BB08_ON(reg, onoff);
	else if (selection == DP_BB_09)
		D4_DP_LCD_BB09_ON(reg, onoff);
	else if (selection == DP_BB_10)
		D4_DP_LCD_BB10_ON(reg, onoff);
	else if (selection == DP_BB_11)
		D4_DP_LCD_BB11_ON(reg, onoff);
	else if (selection == DP_BB_12)
		D4_DP_LCD_BB12_ON(reg, onoff);
	else if (selection == DP_BB_13)
		D4_DP_LCD_BB13_ON(reg, onoff);
	else if (selection == DP_BB_14)
		D4_DP_LCD_BB14_ON(reg, onoff);
	else
		D4_DP_LCD_BB15_ON(reg, onoff);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_BB_MODE_ON);
}

/**
 * @brief graphic color limit set
 * @fn static void dp_sublcd_set_grp_limtval(struct stfb_info *info, struct stdp_rgb_range limit_range)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param limit_range[in] limit range set
 * @return
 *
 * @note
 */
static void dp_sublcd_set_grp_limtval(struct stfb_info *info,
		struct stdp_rgb_range limit_range)
{
	unsigned int r_range = 0, g_range = 0, b_range = 0;

	D4_DP_LCD_GRP_MIX_RANGE_LOW(r_range, limit_range.R_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(r_range, limit_range.R_Range.Upper_Range);

	D4_DP_LCD_GRP_MIX_RANGE_LOW(g_range, limit_range.G_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(g_range, limit_range.G_Range.Upper_Range);

	D4_DP_LCD_GRP_MIX_RANGE_LOW(b_range, limit_range.B_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(b_range, limit_range.B_Range.Upper_Range);

	__raw_writel(r_range, info->dp_sublcd_regs + DP_LCD_GRP_MIX_LIMIT_R);
	__raw_writel(g_range, info->dp_sublcd_regs + DP_LCD_GRP_MIX_LIMIT_G);
	__raw_writel(b_range, info->dp_sublcd_regs + DP_LCD_GRP_MIX_LIMIT_B);
}

/**
 * @brief graphic range set
 * @fn static void dp_sublcd_set_rangedetection_range(struct stfb_info *info, struct stdp_rgb_range range)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param range[in] graphic mix range set
 * @return
 *
 * @note
 */
static void dp_sublcd_set_rangedetection_range(struct stfb_info *info,
		struct stdp_rgb_range range)
{
	unsigned int r_range = 0, g_range = 0, b_range = 0;

	D4_DP_LCD_GRP_MIX_RANGE_LOW(r_range, range.R_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(r_range, range.R_Range.Upper_Range);

	D4_DP_LCD_GRP_MIX_RANGE_LOW(g_range, range.G_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(g_range, range.G_Range.Upper_Range);

	D4_DP_LCD_GRP_MIX_RANGE_LOW(b_range, range.B_Range.Lower_Range);
	D4_DP_LCD_GRP_MIX_RANGE_UP(b_range, range.B_Range.Upper_Range);

	__raw_writel(r_range, info->dp_sublcd_regs + DP_LCD_R_RANGE_CH0);
	__raw_writel(g_range, info->dp_sublcd_regs + DP_LCD_R_RANGE_CH0 + 0x4);
	__raw_writel(b_range, info->dp_sublcd_regs + DP_LCD_R_RANGE_CH0 + 0x8);
}

/**
 * @brief sub lcd color sapce matrix set
 * @fn static void dp_sublcd_set_csc_matrix(struct stfb_info *info, enum edp_layer layer,	enum elcd_csc_type type)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param layer[in] video/graphic
 * @param type[in] color space type
 * @return
 *
 * @note
 */
static void dp_sublcd_set_csc_matrix(struct stfb_info *info,
		enum edp_layer layer, enum elcd_csc_type type)
{
	unsigned int CSC_Matrix_Y_Cb_offset = 0;
	unsigned int CSC_Matrix_Cr_offset = 0;
	unsigned int CSC_Matrix_00_01 = 0;

	unsigned int CSC_Matrix_02_10 = 0;
	unsigned int CSC_Matrix_11_12 = 0;
	unsigned int CSC_Matrix_20_21 = 0;
	unsigned int CSC_Matrix_22 = 0;

	unsigned int CSC_Matrix_range = 0;
	unsigned int CSC_Matrix_range_y = 0;
	unsigned int CSC_Matrix_range_c = 0;

	if (DP_VIDEO == layer) {
		switch (type) {
		case LCD_CSC_SD_STANDARD:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x200);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x2BE);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x200);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x165);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x0AC);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x200);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x377);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x000);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x200);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x200);

			D4_DP_LCD_YCC2RGB_UNDER_RGB(CSC_Matrix_range, 0x58);
			D4_DP_LCD_YCC2RGB_OVER_RGB(CSC_Matrix_range, 0x3ac);
			break;

		case LCD_CSC_SD_FULL:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x331);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x1A0);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x0C9);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x409);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x040);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x200);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x200);

			D4_DP_LCD_YCC2RGB_UNDER_RGB(CSC_Matrix_range, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_RGB(CSC_Matrix_range, 0x3ff);
			break;

		case LCD_CSC_HD_STANDARD:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x200);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x314);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x000);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x200);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x0EB);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x05E);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x200);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x3A2);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x000);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x200);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x200);

			D4_DP_LCD_YCC2RGB_UNDER_RGB(CSC_Matrix_range, 0x40);
			D4_DP_LCD_YCC2RGB_OVER_RGB(CSC_Matrix_range, 0x3ac);
			break;

		case LCD_CSC_HD_FULL:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x396);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x111);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x06D);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x254);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x000);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x43B);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x040);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x200);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x200);

			D4_DP_LCD_YCC2RGB_UNDER_RGB(CSC_Matrix_range, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_RGB(CSC_Matrix_range, 0x3ff);
			break;
		}

		__raw_writel(CSC_Matrix_00_01, info->dp_sublcd_regs + DP_LCD_CSC0);
		__raw_writel(CSC_Matrix_02_10, info->dp_sublcd_regs + DP_LCD_CSC1);
		__raw_writel(CSC_Matrix_11_12, info->dp_sublcd_regs + DP_LCD_CSC2);
		__raw_writel(CSC_Matrix_20_21, info->dp_sublcd_regs + DP_LCD_CSC3);
		__raw_writel(CSC_Matrix_22, info->dp_sublcd_regs + DP_LCD_CSC4);
		__raw_writel(CSC_Matrix_Y_Cb_offset, info->dp_sublcd_regs + DP_LCD_CSC5);
		__raw_writel(CSC_Matrix_Cr_offset, info->dp_sublcd_regs + DP_LCD_CSC6);
		__raw_writel(CSC_Matrix_range, info->dp_sublcd_regs + DP_LCD_CSC7);

	} else {
		dp_lcd_set_path_onoff(info, LCD_CTRL_RGB2YCBCR_ON, DP_ON);
		switch (type) {
		case LCD_CSC_SD_STANDARD:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x06d);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x16e);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x024);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x03b);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x0c9);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x105);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x105);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x0ed);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x018);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x000);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x080);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x080);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_y, 0x00);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_y, 0xff);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_c, 0x00);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_c, 0xff);
			break;

		case LCD_CSC_SD_FULL:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x083);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x102);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x04B);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x032);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x094);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x0E0);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x0E0);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x0DC);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x024);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x010);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x080);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x080);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_y, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_y, 0xff);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_c, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_c, 0xff);
			break;

		case LCD_CSC_HD_STANDARD:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x06D);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x16E);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x03B);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x024);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x0C9);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x105);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x105);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x0ED);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x018);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x000);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x080);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x080);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_y, 0x00);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_y, 0xff);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_c, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_c, 0xff);
			break;

		case LCD_CSC_HD_FULL:
			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_00_01, 0x05D);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_00_01, 0x13A);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_02_10, 0x033);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_02_10, 0x01F);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_11_12, 0x0AD);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_11_12, 0x0E0);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_20_21, 0x0E0);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_20_21, 0x0CC);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_22, 0x014);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Y_Cb_offset, 0x010);
			D4_DP_LCD_YCC2RGB_MTRX1(CSC_Matrix_Y_Cb_offset, 0x080);

			D4_DP_LCD_YCC2RGB_MTRX0(CSC_Matrix_Cr_offset, 0x080);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_y, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_y, 0xff);

			D4_DP_LCD_YCC2RGB_UNDER_Y(CSC_Matrix_range_c, 0x0);
			D4_DP_LCD_YCC2RGB_OVER_Y(CSC_Matrix_range_c, 0xff);
			break;

		}
		__raw_writel(CSC_Matrix_00_01, info->dp_sublcd_regs + DP_GRP_CSC0);
		__raw_writel(CSC_Matrix_02_10, info->dp_sublcd_regs + DP_GRP_CSC1);
		__raw_writel(CSC_Matrix_11_12, info->dp_sublcd_regs + DP_GRP_CSC2);
		__raw_writel(CSC_Matrix_20_21, info->dp_sublcd_regs + DP_GRP_CSC3);
		__raw_writel(CSC_Matrix_22, info->dp_sublcd_regs + DP_GRP_CSC4);
		__raw_writel(CSC_Matrix_Y_Cb_offset, info->dp_sublcd_regs + DP_GRP_CSC5);
		__raw_writel(CSC_Matrix_Cr_offset, info->dp_sublcd_regs + DP_GRP_CSC6);
		__raw_writel(CSC_Matrix_range_y, info->dp_sublcd_regs + DP_GRP_CSC7);
		__raw_writel(CSC_Matrix_range_c, info->dp_sublcd_regs + DP_GRP_CSC8);
	}
}

/**
 * @brief lcd video/graphic horizontal filter set
 * @fn static void dp_sublcd_hrz_filter_onoff(struct stfb_info *info, enum ehrz_filter_type type, enum edp_filter filer_value, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param type[in] filter type
 * @param filer_value[in] value
 * @param on_off[in] on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_hrz_filter_onoff(struct stfb_info *info,
		enum ehrz_filter_type type, enum edp_filter filer_value,
		enum edp_onoff on_off)
{

	unsigned int reg = 0;

	if (on_off == DP_OFF)
	filer_value = DP_BYPASS;

	if (hrz_filter_set[filer_value].Tap0_Coef < 0) {
		D4_DP_LCD_PATH_T0_SIGN(reg, DP_ON);
		D4_DP_LCD_PATH_T0_COEF(reg, -1 * hrz_filter_set[filer_value].Tap0_Coef);
	} else {
		D4_DP_LCD_PATH_T0_SIGN(reg, DP_OFF);
		D4_DP_LCD_PATH_T0_COEF(reg, hrz_filter_set[filer_value].Tap0_Coef);
	}

	if (hrz_filter_set[filer_value].Tap1_Coef < 0) {
		D4_DP_LCD_PATH_T1_SIGN(reg, DP_ON);
		D4_DP_LCD_PATH_T1_COEF(reg, -1 * hrz_filter_set[filer_value].Tap1_Coef);
	} else {
		D4_DP_LCD_PATH_T1_SIGN(reg, DP_OFF);
		D4_DP_LCD_PATH_T1_COEF(reg, hrz_filter_set[filer_value].Tap1_Coef);
	}
	D4_DP_LCD_PATH_T2_COEF(reg, hrz_filter_set[filer_value].Tap2_Coef);

	if (hrz_filter_set[filer_value].Tap3_Coef < 0) {
		D4_DP_LCD_PATH_T3_SIGN(reg, DP_ON);
		D4_DP_LCD_PATH_T3_COEF(reg, -1 * hrz_filter_set[filer_value].Tap3_Coef);
	} else {
		D4_DP_LCD_PATH_T3_SIGN(reg, DP_OFF);
		D4_DP_LCD_PATH_T3_COEF(reg, hrz_filter_set[filer_value].Tap3_Coef);
	}

	if (hrz_filter_set[filer_value].Tap4_Coef < 0) {
		D4_DP_LCD_PATH_T4_SIGN(reg, DP_ON);
		D4_DP_LCD_PATH_T4_COEF(reg, -1 * hrz_filter_set[filer_value].Tap4_Coef);
	} else {
		D4_DP_LCD_PATH_T4_SIGN(reg, DP_OFF);
		D4_DP_LCD_PATH_T4_COEF(reg, hrz_filter_set[filer_value].Tap4_Coef);
	}
	D4_DP_LCD_PATH_POST_COEF(reg, hrz_filter_set[filer_value].Post_Coef);

	if (type == HRZ_FLT_TYPE_VIDEO) {
		__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_HRZ_FLT);
		dp_sublcd_set_path_onoff(info, LCD_CTRL_VID_HRZFLT_ON, on_off);
	} else {
		__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_GRP1_HRZ_FLT);
		dp_sublcd_set_path_onoff(info, LCD_CTRL_GRP_HRZFLT_ON, on_off);
	}
}

/**
 * @brief graphic mix control
 * @fn static void dp_sublcd_set_grp_mix_channel_onoff(struct stfb_info *info,	enum edp_layer layer, enum egrp_mix_channel_onoff selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param layer[in] select video/graphic
 * @param selection[in] selection graphic mix channel
 * @param on_off[in] graphic mix channel on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_set_grp_mix_channel_onoff(struct stfb_info *info,
		enum edp_layer layer, enum egrp_mix_channel_onoff selection,
		enum edp_onoff on_off)
{
	unsigned int reg = 0;
	if (DP_VIDEO == layer)
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_GRP_MIX_CH0);
	else
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_GRP_MIX_CH1);

	if (selection == GRP_MIX_CH_R_ON)
	D4_DP_LCD_GRP_MIX_R_ON(reg, on_off);
	else if (selection == GRP_MIX_CH_G_ON)
	D4_DP_LCD_GRP_MIX_G_ON(reg, on_off);
	else if (selection == GRP_MIX_CH_B_ON)
	D4_DP_LCD_GRP_MIX_B_ON(reg, on_off);
	else if (selection == GRP_MIX_CH_R_INV_ON)
	D4_DP_LCD_GRP_MIX_R_INV(reg, on_off);
	else if (selection == GRP_MIX_CH_G_INV_ON)
	D4_DP_LCD_GRP_MIX_G_INV(reg, on_off);
	else if (selection == GRP_MIX_CH_B_INV_ON)
	D4_DP_LCD_GRP_MIX_B_INV(reg, on_off);
	else if (selection == GRP_MIX_CH_R_OFS_ON)
	D4_DP_LCD_GRP_MIX_R_OFFSET(reg, on_off);
	else if (selection == GRP_MIX_CH_G_OFS_ON)
	D4_DP_LCD_GRP_MIX_G_OFFSET(reg, on_off);
	else
	D4_DP_LCD_GRP_MIX_B_OFFSET(reg, on_off);

	if (DP_VIDEO == layer)
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_GRP_MIX_CH0);
	else
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_GRP_MIX_CH1);

}

/**
 * @brief graphic mix on/off
 * @fn static void dp_sublcd_set_grp_mix_onoff(struct stfb_info *info,	enum egrp_mix_onoff selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] select graphic mix control
 * @param on_off[in] graphic mix channel on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_set_grp_mix_onoff(struct stfb_info *info,
		enum egrp_mix_onoff selection, enum edp_onoff on_off)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_GRP_MIX_ON);

	if (selection == GRP_MIX_ALP_BLD_ON)
	D4_DP_LCD_ALPHA_ON(reg, on_off);
	else if (selection == GRP_MIX_LIMIT_ON)
	D4_DP_LCD_LIMIT_ON(reg, on_off);
	else if (selection == GRP_MIX_FLAG_R_ON)
	D4_DP_LCD_FLAG_R_ON(reg, on_off);
	else if (selection == GRP_MIX_FLAG_G_ON)
	D4_DP_LCD_FLAG_G_ON(reg, on_off);
	else if (selection == GRP_MIX_FLAG_B_ON)
	D4_DP_LCD_FLAG_B_ON(reg, on_off);
	else if (selection == GRP_MIX_VID_RANGE_DET)
	D4_DP_LCD_RANGE_DET_VID(reg, on_off);
	else if (selection == GRP_MIX_GRP_RANGE_DET)
	D4_DP_LCD_RANGE_DET_GRP(reg, on_off);
	else if (selection == GRP_MIX_RANGE_TARGET)
	D4_DP_LCD_RANGE_TARGET(reg, on_off);
	else if (selection == GRP_MIX_LAYER_SWAP)
	D4_DP_LCD_CH_SWAP(reg, on_off);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_GRP_MIX_ON);

	DpPRINTK(" write grp mix reg =0x%x\n", reg);
}

/**
 * @brief sub lcd dma control
 * @fn static void dp_sublcd_set_mdma_control(struct stfb_info *info,	enum emdma_select select, enum edp_onoff burst_onoff, unsigned char req_num)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param select[in] select mdma
 * @param burst_onoff[in] on/off
 * @param req_num[in] set value
 * @return
 *
 * @note
 */
static void dp_sublcd_set_mdma_control(struct stfb_info *info,
		enum emdma_select select, enum edp_onoff burst_onoff,
		unsigned char req_num)
{
	unsigned int mdma_control = 0;
	mdma_control = __raw_readl(info->dp_sublcd_regs + DP_LCD_DMA_MODE1);

	if (select == MDMA_Y)
	D4_DP_LCD_Y_8BURST(mdma_control, burst_onoff);
	else if (select == MDMA_C)
	D4_DP_LCD_C_8BURST(mdma_control, burst_onoff);
	else if (select == MDMA_G)
	D4_DP_LCD_G_8BURST(mdma_control, burst_onoff);

	__raw_writel(mdma_control, info->dp_sublcd_regs + DP_LCD_DMA_MODE1);
}

/**
 * @brief graphic alpha control
 * @fn static void dp_sublcd_set_grp_alpha_ctrl_onoff(struct stfb_info *info, enum egrp_alpha_ctrl selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] set graphic alpah/R/G/B
 * @return
 *
 * @note
 */
static void dp_sublcd_set_grp_alpha_ctrl_onoff(struct stfb_info *info,
		enum egrp_alpha_ctrl selection, enum edp_onoff on_off)
{
	unsigned int alpha_ctrl_onoff = 0;

	alpha_ctrl_onoff
	= __raw_readl(info->dp_sublcd_regs + DP_LCD_MIX_ALPHA_CTRL);

	if (selection == ALP_CTRL_ON)
	D4_DP_LCD_GRP_ALP_ON(alpha_ctrl_onoff, on_off);
	else if (selection == ALP_CTRL_INV)
	D4_DP_LCD_GRP_ALP_INV_ON(alpha_ctrl_onoff, on_off);
	else if (selection == ALP_CTRL_OFS)
	D4_DP_LCD_GRP_ALP_OFS(alpha_ctrl_onoff, on_off);

	__raw_writel(alpha_ctrl_onoff, info->dp_sublcd_regs + DP_LCD_MIX_ALPHA_CTRL);
}

#if 0 /*sub lcd path iomux set */
#include <mach/iomux-d4.h>

/**
 * @brief sublcd path set
 * @fn static void dp_sublcd_set_grp_alpha_ctrl_onoff(struct stfb_info *info, enum egrp_alpha_ctrl selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] set graphic alpah/R/G/B
 * @return
 *
 * @note
 */
void d4_dp_sublcd_path_set(enum edp_onoff onoff)
{
	int ret;
	int gpio;

	if (onoff) {
		d4_iomux_set_func(FUNC_4, SLCD_RESET_CON);
		d4_iomux_set_func(FUNC_4, SLCD_EXT_CLK_CON);
		d4_iomux_set_func(FUNC_1, SLCD_CLK_CON);
		d4_iomux_set_func(FUNC_1, SLCD_EN_CON);
		d4_iomux_set_func(FUNC_1, SLCD_VSYNC_CON);
		d4_iomux_set_func(FUNC_1, SLCD_HSYNC_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA0_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA1_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA2_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA3_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA4_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA5_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA6_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA7_CON);

		d4_iomux_set_func(FUNC_1, SLCD_DATA8_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA9_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA10_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA11_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA12_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA13_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA14_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA15_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA16_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA17_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA18_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA19_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA20_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA21_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA22_CON);
		d4_iomux_set_func(FUNC_1, SLCD_DATA23_CON);

		gpio = DRIME4_GPIO5(2);
		ret = gpio_request(gpio, "GPIO5_2");
		if (ret < 0) {
			pr_err("cannot request gpio %d\n", gpio);
			return;
		}

		gpio_direction_output(DRIME4_GPIO5(2), 1);

		gpio = DRIME4_GPIO5(3);
		ret = gpio_request(gpio, "GPIO5_3");
		if (ret < 0) {
			pr_err("cannot request gpio %d\n", gpio);
			return;
		}

		gpio_direction_output(DRIME4_GPIO5(3), 1);
	} else {
		d4_iomux_set_func(FUNC_4, SLCD_RESET_CON);
		d4_iomux_set_func(FUNC_4, SLCD_EXT_CLK_CON);
		d4_iomux_set_func(FUNC_4, SLCD_CLK_CON);
		d4_iomux_set_func(FUNC_4, SLCD_EN_CON);
		d4_iomux_set_func(FUNC_4, SLCD_VSYNC_CON);
		d4_iomux_set_func(FUNC_4, SLCD_HSYNC_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA0_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA1_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA2_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA3_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA4_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA5_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA6_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA7_CON);

		d4_iomux_set_func(FUNC_4, SLCD_DATA8_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA9_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA10_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA11_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA12_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA13_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA14_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA15_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA16_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA17_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA18_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA19_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA20_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA21_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA22_CON);
		d4_iomux_set_func(FUNC_4, SLCD_DATA23_CON);
	}

}
#endif

/**
 * @brief graphic control
 * @fn static void dp_sublcd_grp_set_ctrl_onoff(struct stfb_info *info,	enum egrp_ctrl_onoff selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] select graphic control Item
 * @param on_off[in] control on/off
 * @return
 *
 * @note
 */
static void dp_sublcd_grp_set_ctrl_onoff(struct stfb_info *info,
		enum egrp_ctrl_onoff selection, enum edp_onoff on_off)
{
	unsigned int lcd_grp_ctrl = 0;

	lcd_grp_ctrl = __raw_readl(info->dp_sublcd_regs + DP_LCD_GRP1_DMA_CTRL);

	if (selection == GRP_CTRL_ARGB_ORDER)
	D4_DP_LCD_DMA_ARGB_ORDER(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN0_FLAT_ALPHA)
	D4_DP_LCD_DMA_WIN0_FLAT_ALPHA(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN1_FLAT_ALPHA)
	D4_DP_LCD_DMA_WIN1_FLAT_ALPHA(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN2_FLAT_ALPHA)
	D4_DP_LCD_DMA_WIN2_FLAT_ALPHA(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN3_FLAT_ALPHA)
	D4_DP_LCD_DMA_WIN3_FLAT_ALPHA(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN0_ADDR_SWAP)
	D4_DP_LCD_DMA_WIN0_SWAP(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN1_ADDR_SWAP)
	D4_DP_LCD_DMA_WIN0_SWAP(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN2_ADDR_SWAP)
	D4_DP_LCD_DMA_WIN0_SWAP(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN3_ADDR_SWAP)
	D4_DP_LCD_DMA_WIN0_SWAP(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN0_ON)
	D4_DP_LCD_DMA_WIN0_ON(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN1_ON)
	D4_DP_LCD_DMA_WIN0_ON(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN2_ON)
	D4_DP_LCD_DMA_WIN0_ON(lcd_grp_ctrl, on_off);
	else if (selection == GRP_CTRL_WIN3_ON)
	D4_DP_LCD_DMA_WIN0_ON(lcd_grp_ctrl, on_off);

	__raw_writel(lcd_grp_ctrl, info->dp_sublcd_regs + DP_LCD_GRP1_DMA_CTRL);

}

/**
 * @brief graphic alpha select a pixel alpha or flat alpha
 * @fn static void dp_sublcd_set_grp_flat_alpha_onoff(struct stfb_info *info, enum edp_window win, unsigned char alpha_value, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param win[in] select graphic window
 * @param alpha_value[in] alpha value set
 * @param on_off[in] on= flat alpha, off =pixel alpha
 * @return
 *
 * @note
 */
static void dp_sublcd_set_grp_flat_alpha_onoff(struct stfb_info *info,
		enum edp_window win, unsigned char alpha_value, enum edp_onoff on_off)
{

	unsigned int win_flat_alpha = 0;
	win_flat_alpha = __raw_readl(info->dp_sublcd_regs
			+ DP_LCD_GRP1_WIN_FLAT_ALPHA);

	if (win == DP_WIN0) {
		D4_DP_LCD_GRP_WIN0_ALPHA(win_flat_alpha, alpha_value);
		dp_lcd_grp_set_ctrl_onoff(info, GRP_CTRL_WIN0_FLAT_ALPHA, on_off);
	} else if (win == DP_WIN1) {
		D4_DP_LCD_GRP_WIN1_ALPHA(win_flat_alpha, alpha_value);
		dp_lcd_grp_set_ctrl_onoff(info, GRP_CTRL_WIN1_FLAT_ALPHA, on_off);
	} else if (win == DP_WIN2) {
		D4_DP_LCD_GRP_WIN2_ALPHA(win_flat_alpha, alpha_value);
		dp_lcd_grp_set_ctrl_onoff(info, GRP_CTRL_WIN2_FLAT_ALPHA, on_off);
	} else {
		D4_DP_LCD_GRP_WIN3_ALPHA(win_flat_alpha, alpha_value);
		dp_lcd_grp_set_ctrl_onoff(info, GRP_CTRL_WIN3_FLAT_ALPHA, on_off);
	}

	__raw_writel(win_flat_alpha, info->dp_sublcd_regs
			+ DP_LCD_GRP1_WIN_FLAT_ALPHA);
}

/**
 * @brief dp sublcd output type set, depend on lcd pannel type
 * @fn static void dp_sublcd_out_type(struct stfb_info *info, enum elcd_out_type type)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param type[in] lcd pannel type
 * @return
 *
 * @note
 */
static void dp_sublcd_out_type(struct stfb_info *info, enum elcd_out_type type)
{
	unsigned int reg0 = 0, reg1 = 0;
	reg0 = __raw_readl(info->dp_sublcd_regs + DP_LCD_OUT_CTRL);
	reg1 = __raw_readl(info->dp_sublcd_regs + DP_LCD_OUT_DISP_SEQ);
	D4_DP_LCD_OUT_MULTI_CLK_TRANS(reg1, DP_OFF);
	D4_DP_LCD_OUT_8BIT_BYPASS(reg0, DP_OFF);

	switch (type) {
	case LCD_OUT_DELTA_8BIT:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, DATA_8);
		break;

	case LCD_OUT_STRIPE_8BIT_MSB:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_8BIT_DATA(reg0, DP_ON);
		break;

	case LCD_OUT_STRIPE_8BIT_LSB:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_8BIT_DATA(reg0, DP_OFF);
		D4_DP_LCD_OUT_MULTI_CLK_TRANS(reg1, DP_ON);
		break;

	case LCD_OUT_STRIPE_24BIT:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, DATA_24);
		break;

	case LCD_OUT_RGB_DUMMY:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x2);
		break;

	case LCD_OUT_RGB656:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x2);
		break;

	case LCD_OUT_YCBCR_8BIT_CBYCRY:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 0);
		break;

	case LCD_OUT_YCBCR_8BIT_CRYCBY:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 1);
		break;

	case LCD_OUT_YCBCR_8BIT_YCBYCR:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 0x2);
		break;

	case LCD_OUT_YCBCR_8BIT_YCRYCB:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x3);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 0x3);
		break;

	case LCD_OUT_YCBCR_16BIT_CBCR:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x2);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 0);
		break;

	case LCD_OUT_YCBCR_16BIT_CRCB:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_OFF);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, 0x2);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 1);
		break;

	case LCD_OUT_CCIR656_CBCR:
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_ON);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, DATA_24);
		D4_DP_LCD_OUT_YCC_SEQ(reg0, 0);
		break;

	case LCD_OUT_CCIR656_CRCB:
		reg0 = 0;
		D4_DP_LCD_OUT_BT656_ON(reg0, DP_ON);
		D4_DP_LCD_OUT_CTRL_YCBCR(reg0, DP_ON);
		D4_DP_LCD_OUT_BUS_WIDTH(reg0, DATA_24);
		D4_DP_LCD_OUT_CTRL_ON(reg0, DP_ON);
		break;
	}

	__raw_writel(reg0, info->dp_sublcd_regs + DP_LCD_OUT_CTRL);
	__raw_writel(reg1, info->dp_sublcd_regs + DP_LCD_OUT_DISP_SEQ);
}

/**
 * @brief dp sub lcd pannel initialize
 * @fn void dp_sublcd_panel_init(struct stfb_info *info)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_sublcd_panel_init(struct stfb_info *info)
{
	/* LCD Cotroller Main Setting */
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_sublcd_regs + DP_LCD_OUT_CTRL);

	D4_DP_LCD_OUT_8BIT_BYPASS(reg, DP_OFF);
	D4_DP_LCD_OUT_PATTERN_GEN(reg, DP_OFF);
	D4_DP_LCD_OUT_CTRL_ON(reg, DP_ON);
	D4_DP_LCD_OUT_BUS_WIDTH(reg, sublcd_pannel.lcd_data_width);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_OUT_CTRL);

	/* LCD Display Sequence Setting ( Even/Odd ) : Dot Array */
	reg = 0;
	D4_DP_LCD_OUT_DISPLAY_SEQ_ODD(reg, sublcd_pannel.odd_seq);
	D4_DP_LCD_OUT_DISPLAY_SEQ_EVEN(reg, sublcd_pannel.even_seq);

	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_OUT_DISP_SEQ);

	/* LCD Size Setting */
	reg = 0;
	D4_DP_LCD_PATH_SIZE_H(reg, sublcd_pannel.h_size);
	D4_DP_LCD_PATH_SIZE_V(reg, sublcd_pannel.v_size);
	__raw_writel(reg, info->dp_sublcd_regs + DP_LCD_SIZE);

	if ((sublcd_pannel.lcd_data_width == DATA_8) && (sublcd_pannel.type
					== STRIPE)) {
		/** 3 Times Transmission  */
		dp_sublcd_out_type(info, LCD_OUT_STRIPE_8BIT_LSB);
	} else if ((sublcd_pannel.lcd_data_width == DATA_24) && (sublcd_pannel.type
					== STRIPE)) {

		/** Normal Stripe Type   */
		dp_sublcd_out_type(info, LCD_OUT_STRIPE_24BIT);
	} else {
		/** Normal Delta Type   */
		dp_sublcd_out_type(info, LCD_OUT_DELTA_8BIT);
	}

	mdelay(33);
}

/**
 * @brief dp sub lcd pannel set
 * @fn void dp_sublcd_pannel_set(void)
 * @param void
 * @return
 *
 * @note
 */
void dp_sublcd_pannel_set(void)
{
	dp_sublcd_panel_init(&lcd_info);
	dp_sublcd_set_timming_sig(&lcd_info, sublcd_pannel.timing);
}

/**
 * @brief video display window,display area, address, format, stride set, viedeo with relation all part set
 * @fn void d4_dp_sublcd_video_set(struct stvideodisplay video)
 * @param video[in] struct stvideodisplay reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_video_set(struct stvideodisplay video)
{
	struct stdp_display_area display;
	struct stfb_video_info vid;
	display = video.display;
	vid.address = video.address;
	vid.bit = video.bit;

	vid.display = display;
	vid.format = video.format;
	vid.image.image_height = video.img_height;
	vid.image.image_width = video.img_width;
	vid.vid_stride = video.stride;
	vid.win = video.win;

	dp_sublcd_set_vid_display_area(&lcd_info, video.win, video.bit, &display);
	dp_sublcd_set_vid_image_address(&lcd_info, &vid);

	if (video.format == Ycbcr_420)
	dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_YCBCR422, DP_OFF);
	else
	dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_YCBCR422, DP_ON);

	if (video.bit == _8bit) {
		dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_VID_10BIT, DP_OFF);
	} else {
		dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_VID_10BIT, DP_ON);
	}
	dp_sublcd_set_stride(&lcd_info, DP_VIDEO, vid.vid_stride);
}

/**
 * @brief video display window on/off
 * @fn void d4_dp_sublcd_video_window_onoff(enum edp_window window, enum edp_onoff on_off)
 * @param win[in] display window set
 * @param on_off[in] on/off
 * @return
 *
 * @note
 */
void d4_dp_sublcd_video_window_onoff(enum edp_window window, enum edp_onoff on_off)
{
	dp_sublcd_vid_window_onoff(&lcd_info, window, on_off);
}


/**
 * @brief video stride set
 * @fn int d4_dp_sublcd_video_stride(unsigned int stride)
 * @param stride[in]  stride value, it must be setted by the multiple of 8
 * @return
 *
 * @note
 */
int d4_dp_sublcd_video_stride(unsigned int stride)
{
	return dp_sublcd_set_stride(&lcd_info, DP_VIDEO, stride);
}

/**
 * @brief video background set
 * @fn void d4_dp_sublcd_video_background(struct stdp_ycbcr *videobackground)
 * @param *videobackground[in] struct stdp_ycbcr  reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_video_background(struct stdp_ycbcr *videobackground)
{
	dp_sublcd_set_vid_bkg_color(&lcd_info, videobackground);
}

/**
 * @brief video display area set
 * @fn void d4_dp_sublcd_video_display_area(enum edp_window win,  enum edp_video_bit vid_bit, struct stdp_display_area display)
 * @param win[in] display window set
 * @param vid_bit[in] video bit 8/10 bit
 * @param display[in] video display area set,
 * @return
 *
 * @note
 */
void d4_dp_sublcd_video_display_area(enum edp_window win,
		enum edp_video_bit vid_bit, struct stdp_display_area display)
{
	struct stdp_display_area display_set = display;

	dp_sublcd_set_vid_display_area(&lcd_info, win, vid_bit, &display_set);
}

void d4_dp_sublcd_video_address_set(struct stvideo_address video)
{
	struct stfb_video_info vid;
	vid.address = video.address;
	vid.win = video.win;
	dp_sublcd_set_vid_image_address(&lcd_info, &vid);
}

/**
 * @brief graphic stride set
 * @fn int d4_dp_lcd_graphic_stride(unsigned int stride)
 * @param stride[in] set stride  it must be setted by the multiple of 8
 * @return
 *
 * @note
 */
int d4_dp_sublcd_graphic_stride(unsigned int stride)
{
	return dp_sublcd_set_stride(&lcd_info, DP_GRP, stride);
}

/**
 * @brief graphic background set
 * @fn void d4_dp_sublcd_graphic_background(struct stdp_argb *argb)
 * @param *argb[in] struct stdp_argb reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_graphic_background(struct stdp_argb *argb)
{
	dp_sublcd_grp_set_bkg_color(&lcd_info, argb);
}

/**
 * @brief graphic display area set
 * @fn void d4_dp_sublcd_graphic_display_area(struct stgraphic_display_area *area)
 * @param *area[in] struct stgraphic_display_area reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_graphic_display_area(struct stgraphic_display_area *area)
{
	if (dp_sublcd_set_grp_display_area(&lcd_info, area) < 0)
		printk(
				"[Warnig] graphic display H position the multiple of 2:[%s:%d]:%s\n",
				__FUNCTION__, __LINE__, __FILE__);
}

/**
 * @brief graphic display window on/off
 * @fn void d4_dp_sublcd_graphic_window_onoff(enum edp_window window,	enum edp_onoff on_off)
 * @param window[in] select graphic window
 * @param on_off[in] window on/off
 * @return
 *
 * @note
 */
void d4_dp_sublcd_graphic_window_onoff(enum edp_window window,
		enum edp_onoff on_off)
{
	dp_sublcd_grp_window_onoff(&lcd_info, window, on_off);
}

/**
 * @brief graphic display window,display area, address, format, stride set, viedeo with relation all part set
 * @fn void d4_dp_sublcd_graphic_set(struct stgrpdisplay graphic)
 * @param graphic[in] struct stgrpdisplay reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_graphic_set(struct stgrpdisplay graphic)
{
	struct stgraphic_display_area area;
	unsigned int address;

	if (graphic.win_onoff == DP_OFF)
		return;

	area.win = graphic.win;
	area.display = graphic.display;
	address = graphic.address;

	if (dp_sublcd_set_grp_display_area(&lcd_info, &area) < 0)
		printk(
				"[Warnig] graphic display H position the multiple of 2:[%s:%d]:%s\n",
				__FUNCTION__, __LINE__, __FILE__);
	dp_sublcd_set_grp_image_address(&lcd_info, graphic.win, address);
	dp_sublcd_set_stride(&lcd_info, DP_GRP, graphic.stride);
	dp_sublcd_grp_window_onoff(&lcd_info, graphic.win, graphic.win_onoff);
}

void d4_dp_sublcd_graphic_address_set(struct stgrp_address graphic)
{
	dp_sublcd_set_grp_image_address(&lcd_info, graphic.win, graphic.address);
}

/**
 * @brief graphic scale set
 * @fn void d4_dp_lcd_graphic_scale(enum egrp_scale scale)
 * @param scale[in] graphic scale, original/double
 * @return
 *
 * @note
 */
void d4_dp_sublcd_graphic_scale(enum egrp_scale scale)
{
	dp_sublcd_set_grp_scaler(&lcd_info, scale);
}

/**
 * @brief graphic window display prority set
 * @fn void d4_dp_sublcd_graphic_window_priority(struct stdp_grp_prority *priority)
 * @param *priority[in] struct stdp_grp_prority reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_graphic_window_priority(struct stdp_grp_prority *priority)
{
	dp_sublcd_set_grp_priority(&lcd_info, priority);
}

/**
 * @brief the dp lcd boundary box colortable select and the table set a color
 * @fn void d4_dp_sublcd_boundarybox_table_color(enum edp_bb_color_table table, 	struct stdp_rgb *rgb_info)
 * @param table[in] boundary box color table select
 * @param *rgb_info[in] table color set
 * @return
 *
 * @note
 */
void d4_dp_sublcd_boundarybox_table_color(enum edp_bb_color_table table,
		struct stdp_rgb *rgb_info)
{
	dp_sublcd_vid_set_bb_color_table(&lcd_info, table, rgb_info);
}

/**
 * @brief boundary box set
 * @fn void d4_dp_lcd_bouddarybox_info_set(struct stbb_info *bb_info)
 * @param *bb_info[in] struct stbb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_bouddarybox_info_set(struct stbb_info *bb_info)
{
	struct stdp_rgb bb_out_color;
	bb_out_color.DP_R = 0;
	bb_out_color.DP_G = 0;
	bb_out_color.DP_B = 0;

	dp_sublcd_set_bb_width(&lcd_info, 4, 4);
	dp_sublcd_set_bb_alpha(&lcd_info, 0xf0);
	dp_sublcd_set_bb_outline_color(&lcd_info, &bb_out_color);

	dp_sublcd_set_bb_mode(&lcd_info, bb_info->bb_win, bb_info->style);
	dp_sublcd_set_bb_color(&lcd_info, bb_info->bb_win, bb_info->table);
	dp_sublcd_set_bb_outline_onoff(&lcd_info, bb_info->bb_win, DP_ON);
}

/**
 * @brief boundary box diplay area set, on/off
 * @fn void d4_dp_sublcd_boudarybox_onoff(struct stbb_onoff *bb_onoff)
 * @param *bb_onoff[in]struct stbb_onoff reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_boudarybox_onoff(struct stbb_onoff *bb_onoff)
{
/*	unsigned int i;
	if (bb_onoff->bb_win_all_clear == DP_ON) {
		for (i = 0; i < 16; i++)
			dp_sublcd_vid_bb_onoff(&lcd_info, i, DP_OFF);
	} else {*/
		dp_sublcd_set_bb_display_area(&lcd_info, bb_onoff->bb_win, bb_onoff->area);
		dp_sublcd_vid_bb_onoff(&lcd_info, bb_onoff->bb_win, bb_onoff->onoff);
/*	}*/
}

/**
 * @brief lcd filter set
 * @fn void d4_dp_sublcd_filter_onoff(struct stlcdfilter *filter_ctrl)
 * @param *filter_ctrl[in]struct stlcdfilter reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_filter_onoff(struct stlcdfilter *filter_ctrl)
{
	dp_sublcd_hrz_filter_onoff(&lcd_info, filter_ctrl->type,
			filter_ctrl->filer_value, filter_ctrl->on_off);
}

/**
 * @brief lcd filter set
 * @fn void d4_dp_sublcd_filter_onoff(struct stlcdfilter *filter_ctrl)
 * @param *filter_ctrl[in]struct stlcdfilter reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_zebra_control(struct stzebra_set *zebra_set)
{
	dp_sublcd_zebra_pattern_onoff(&lcd_info, zebra_set);
}

/**
 * @brief dp graphic flat alpha set
 * @fn void d4_dp_lcd_graphic_window_alpha(struct stlcd_graphic_alpha alpha)
 * @param alpha[in] struct stlcd_graphic_alpha reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_graphic_window_alpha(struct stlcd_graphic_alpha alpha)
{
	dp_sublcd_set_grp_flat_alpha_onoff(&lcd_info, alpha.win, alpha.alpha_value,
			alpha.on_off);
}

/**
 * @brief lcd display mirroring
 * @fn void d4_dp_sublcd_flip(enum edp_layer layer, enum eswap direct)
 * @param layer[in] select video/graphic
 * @param direct[in] horizontal/vertical direte
 * @return
 *
 * @note
 */
void d4_dp_sublcd_flip(enum edp_layer layer, enum eswap direct)
{
	if (layer == DP_VIDEO)
		dp_sublcd_set_vid_h_v_swap(&lcd_info, direct);
	else
		dp_sublcd_set_grp_h_v_swap(&lcd_info, direct);
}

/**
 * @brief lcd TCC Lookup table set
 * @fn void d4_dp_sublcd_tcc_set(struct stlcd_tcc tcc)
 * @param tcc[in] struct stlcd_tcc reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_tcc_set(struct stlcd_tcc tcc)
{
	unsigned int tcc_set_info = 0, index_num = 256, i = 0;

	dp_sublcd_tcc_onoff(&lcd_info, tcc.onoff);

	if (tcc.onoff) {
		dp_sublcd_tcc_vid_onoff(&lcd_info, tcc.video_only);

		for (i = 0; i < index_num; i++) {
			D4_DP_LCD_TCC_R_LUT(tcc_set_info, ((tcc.TCC_TABLE[i]>>16) & 0xFF));
			D4_DP_LCD_TCC_G_LUT(tcc_set_info, ((tcc.TCC_TABLE[i]>>8) & 0xFF));
			D4_DP_LCD_TCC_B_LUT(tcc_set_info, (tcc.TCC_TABLE[i] & 0xFF));
			D4_DP_LCD_TCC_LUT_INDEX(tcc_set_info, ((tcc.TCC_TABLE[i]>>24) & 0xFF));
			printk("tcc value =0x%x \n", tcc_set_info);
			__raw_writel(tcc_set_info, lcd_info.dp_sublcd_regs + DP_TCC_LUT);
		}
	}
}

/**
 * @brief lcd limit set
 * @fn void d4_dp_sublcd_limit_set(struct stdp_rgb_range range)
 * @param range[in] range set
 * @return
 *
 * @note
 */
void d4_dp_sublcd_limit_set(struct stdp_rgb_range range)
{
	dp_sublcd_set_grp_limtval(&lcd_info, range);
}

/**
 * @brief dp layer swap
 * @fn void d4_dp_sublcd_overlayer(struct stdp_layer_change layer)
 * @param layer[in] struct stdp_layer_changereference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_sublcd_overlayer(enum edp_layer over_layer)
{
	if (over_layer == DP_VIDEO)
		dp_sublcd_set_channel_swap(&lcd_info, DP_ON);
	else
		dp_sublcd_set_channel_swap(&lcd_info, DP_OFF);
}

void d4_dp_bt656_onoff(enum edp_onoff onoff)
{
	unsigned int reg;
	if (onoff) {
		__raw_writel(0x020C06b3, lcd_info.dp_sublcd_regs + DP_LCD_TG_SIZE);
		__raw_writel(0x00000114, lcd_info.dp_sublcd_regs + DP_LCD_TG_HSYNC);
		__raw_writel(0x00000014, lcd_info.dp_sublcd_regs + DP_LCD_TG_F0_VSYNC);
		__raw_writel(0x0106011B, lcd_info.dp_sublcd_regs + DP_LCD_TG_F1_VSYNC);
		__raw_writel(0x00000003, lcd_info.dp_sublcd_regs + DP_LCD_TG_FSYNC0);
		__raw_writel(0x01090000, lcd_info.dp_sublcd_regs + DP_LCD_TG_FVSYNC);
		__raw_writel(0x020C0010, lcd_info.dp_sublcd_regs + DP_LCD_TG_INFO0);
		__raw_writel(0x010c0116, lcd_info.dp_sublcd_regs + DP_LCD_TG_INFO1);
		__raw_writel(0x06B30014, lcd_info.dp_sublcd_regs + DP_LCD_TG_INFO2);
		__raw_writel(0x00000114, lcd_info.dp_sublcd_regs + DP_LCD_TG_ACTIVE_PIXEL);
		__raw_writel(0x01050016, lcd_info.dp_sublcd_regs + DP_LCD_TG_ACTIVE_LINE0);
		__raw_writel(0x00000000, lcd_info.dp_sublcd_regs + DP_LCD_OUT_INV);

		dp_sublcd_set_csc_matrix(&lcd_info, DP_GRP, LCD_CSC_SD_STANDARD);
		dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_LCD_DMAC_ON, DP_ON);
		dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_GRP_MIX_ON, DP_ON);
		dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_RGB2YCBCR_ON, DP_ON);
		dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_VID_SCAN, DP_OFF);
		dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_GRP_SCAN, DP_OFF);
		dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_SD_ON, DP_ON);
		dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_GRP_DMAC_ON, DP_OFF);
		dp_sublcd_out_type(&lcd_info, LCD_OUT_CCIR656_CRCB);

		reg = 0;
		D4_DP_LCD_PATH_SIZE_H(reg, 0x480);
		D4_DP_LCD_PATH_SIZE_V(reg, 0x1e0);
		__raw_writel(reg, lcd_info.dp_sublcd_regs + DP_LCD_SIZE);
	}

	#if 0 /*sub lcd path iomux set */
	d4_dp_sublcd_path_set(onoff);
	#endif
}


/**
 * @brief dp lcd pannel set
 * @fn void dp_lcd_pannel_set(void)
 * @param void
 * @return
 *
 * @note
 */
void dp_lcd_pannel_set(void)
{
	dp_lcd_panel_init(&lcd_info);
	dp_lcd_set_timming_sig(&lcd_info, lcd_pannel.timing);
}

/**
 * @brief dp lcd set framebuffer information
 * @fn void dp_lcd_set_info(struct stfb_get_info *getinfo)
 * @param *getinfo[in] struct stfb_get_info reference linux/include/vide/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void dp_lcd_set_info(struct stfb_get_info *getinfo)
{
	lcd_info.dp_global_regs = getinfo->dp_global;
	lcd_info.dp_tv_regs = getinfo->dp_tv;
	lcd_info.dp_lcd_regs = getinfo->dp_lcd;
	lcd_info.dp_sublcd_regs = lcd_info.dp_lcd_regs + 0x500;

	lcd_video = getinfo->video;
	lcd_graphic = getinfo->graphic;
	lcd_pannel = getinfo->pannel;
	sublcd_pannel = getinfo->sub_pannel;
}

/**
 * @brief dp lcd diplay initialize
 * @fn void drime4_lcd_display_init(void)
 * @param void
 * @return
 *
 * @note
 */
void drime4_lcd_display_init(void)
{
	struct stdp_ycbcr bkg_color;

	/* clock enable  & dp mlcd reset */
	dp_lcd_clock_on_off(&lcd_info, MLCD_SYS_CLK_EN, DP_ON);
	dp_lcd_clock_on_off(&lcd_info, MLCD_CLK_EN, DP_ON);
	dp_lcd_clock_on_off(&lcd_info, MLCD_CLK_INV, DP_ON);
	dp_reg_reset(&lcd_info, MLCD_Reset);

	/* NLC clock enable*/
	dp_lcd_clock_on_off(&lcd_info, NLC_VID_CLK_EN, DP_ON);
	dp_lcd_clock_on_off(&lcd_info, NLC_GRP_CLK_EN, DP_ON);

	/* csc set */
	dp_lcd_set_csc_matrix(&lcd_info, DP_VIDEO, LCD_CSC_HD_STANDARD);

	/* dma default set  */
	dp_lcd_set_mdma_control(&lcd_info, MDMA_Y, DP_OFF, 0x7);
	dp_lcd_set_mdma_control(&lcd_info, MDMA_C, DP_OFF, 0x7);
	dp_lcd_set_mdma_control(&lcd_info, MDMA_G, DP_OFF, 0x7);
	dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_LCD_DMAC_ON, DP_ON);

	/* horizontal filter bypass set */
	dp_lcd_hrz_filter_onoff(&lcd_info, HRZ_FLT_TYPE_VIDEO, DP_BYPASS, DP_OFF);
	dp_lcd_hrz_filter_onoff(&lcd_info, HRZ_FLT_TYPE_GRP, DP_BYPASS, DP_OFF);
	dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_VID_HRZFLT_ON, DP_OFF);
	dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_GRP_HRZFLT_ON, DP_OFF);

	/* background color set : black */
	bkg_color.DP_Y = 0x00;
	bkg_color.DP_Cb = 0x80;
	bkg_color.DP_Cr = 0x80;
	dp_lcd_set_vid_bkg_color(&lcd_info, &bkg_color);

	/* dp lcd start */
	dp_lcd_start(&lcd_info);

	/* lcd graphic initialize */
	dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_GRP_DMAC_ON, DP_ON);

	/* graphic mix on */
	dp_lcd_set_path_onoff(&lcd_info, LCD_CTRL_GRP_MIX_ON, DP_ON);

	/* graphic mix channel r/g/b on */
	dp_lcd_set_grp_mix_channel_onoff(&lcd_info, DP_VIDEO, GRP_MIX_CH_R_ON,
			DP_ON);
	dp_lcd_set_grp_mix_channel_onoff(&lcd_info, DP_VIDEO, GRP_MIX_CH_G_ON,
			DP_ON);
	dp_lcd_set_grp_mix_channel_onoff(&lcd_info, DP_VIDEO, GRP_MIX_CH_B_ON,
			DP_ON);

	dp_lcd_set_grp_mix_channel_onoff(&lcd_info, DP_GRP, GRP_MIX_CH_R_ON, DP_ON);
	dp_lcd_set_grp_mix_channel_onoff(&lcd_info, DP_GRP, GRP_MIX_CH_G_ON, DP_ON);
	dp_lcd_set_grp_mix_channel_onoff(&lcd_info, DP_GRP, GRP_MIX_CH_B_ON, DP_ON);

	dp_lcd_set_grp_mix_onoff(&lcd_info, GRP_MIX_ALP_BLD_ON, DP_ON);
	dp_lcd_set_grp_alpha_ctrl_onoff(&lcd_info, ALP_CTRL_ON, DP_ON);

	/* graphic argb sequence : A/B/G/R  */
	dp_lcd_grp_set_ctrl_onoff(&lcd_info, GRP_CTRL_ARGB_ORDER, DP_OFF);

	/*flat alpha on */
	dp_lcd_set_grp_flat_alpha_onoff(&lcd_info, DP_WIN0, 0xff, 1);
	dp_lcd_set_grp_flat_alpha_onoff(&lcd_info, DP_WIN1, 0xff, 1);
	dp_lcd_set_grp_flat_alpha_onoff(&lcd_info, DP_WIN2, 0xff, 1);
	dp_lcd_set_grp_flat_alpha_onoff(&lcd_info, DP_WIN3, 0xff, 1);

	/*interrupt frame 0=frame, 1= field*/
	dp_set_interrupt_interval(&lcd_info, DP_MLCD, 0);

	/*interrupt disable */
	dp_interrupt_on_off(&lcd_info, INT_MLCD, DP_OFF);
}

void drime4_sublcd_display_init(void)
{
	struct stdp_ycbcr bkg_color;

	dp_lcd_clock_on_off(&lcd_info, SLCD_SYS_CLK_EN, DP_ON);
	dp_lcd_clock_on_off(&lcd_info, SLCD_CLK_EN, DP_ON);
	dp_lcd_clock_on_off(&lcd_info, SLCD_CLK_INV, DP_OFF);
	dp_reg_reset(&lcd_info, SLCD_Reset);

	/* background color set : black */
	bkg_color.DP_Y = 0x00;
	bkg_color.DP_Cb = 0x80;
	bkg_color.DP_Cr = 0x80;

	dp_sublcd_set_csc_matrix(&lcd_info, DP_VIDEO, LCD_CSC_HD_STANDARD);

	dp_sublcd_set_mdma_control(&lcd_info, MDMA_Y, DP_ON, 0x7);
	dp_sublcd_set_mdma_control(&lcd_info, MDMA_C, DP_ON, 0x7);
	dp_sublcd_set_mdma_control(&lcd_info, MDMA_G, DP_ON, 0x7);
	dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_LCD_DMAC_ON, DP_ON);

	dp_sublcd_hrz_filter_onoff(&lcd_info, HRZ_FLT_TYPE_VIDEO, DP_BYPASS, DP_OFF);
	dp_sublcd_hrz_filter_onoff(&lcd_info, HRZ_FLT_TYPE_GRP, DP_BYPASS, DP_OFF);
	dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_VID_HRZFLT_ON, DP_OFF);
	dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_GRP_HRZFLT_ON, DP_OFF);

	dp_sublcd_set_vid_bkg_color(&lcd_info, &bkg_color);

	/* dp sublcd start */
	dp_sublcd_start(&lcd_info);
	dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_GRP_DMAC_ON, DP_ON);
	dp_sublcd_set_path_onoff(&lcd_info, LCD_CTRL_GRP_MIX_ON, DP_ON);

	dp_sublcd_set_grp_mix_channel_onoff(&lcd_info, DP_VIDEO, GRP_MIX_CH_R_ON,
			DP_ON);
	dp_sublcd_set_grp_mix_channel_onoff(&lcd_info, DP_VIDEO, GRP_MIX_CH_G_ON,
			DP_ON);
	dp_sublcd_set_grp_mix_channel_onoff(&lcd_info, DP_VIDEO, GRP_MIX_CH_B_ON,
			DP_ON);

	dp_sublcd_set_grp_mix_channel_onoff(&lcd_info, DP_GRP, GRP_MIX_CH_R_ON,
			DP_ON);
	dp_sublcd_set_grp_mix_channel_onoff(&lcd_info, DP_GRP, GRP_MIX_CH_G_ON,
			DP_ON);
	dp_sublcd_set_grp_mix_channel_onoff(&lcd_info, DP_GRP, GRP_MIX_CH_B_ON,
			DP_ON);
	dp_sublcd_set_grp_mix_onoff(&lcd_info, GRP_MIX_ALP_BLD_ON, DP_ON);
	dp_sublcd_set_grp_alpha_ctrl_onoff(&lcd_info, ALP_CTRL_ON, DP_ON);
	dp_sublcd_grp_set_ctrl_onoff(&lcd_info, GRP_CTRL_ARGB_ORDER, DP_OFF);

	dp_sublcd_set_grp_flat_alpha_onoff(&lcd_info, DP_WIN0, 0xff, 1);
	dp_sublcd_set_grp_flat_alpha_onoff(&lcd_info, DP_WIN1, 0xff, 1);
	dp_sublcd_set_grp_flat_alpha_onoff(&lcd_info, DP_WIN2, 0xff, 1);
	dp_sublcd_set_grp_flat_alpha_onoff(&lcd_info, DP_WIN3, 0xff, 1);

	dp_set_interrupt_interval(&lcd_info, DP_SLCD, 0);
	dp_interrupt_on_off(&lcd_info, INT_SLCD, DP_OFF);
}

/**
 * @brief dp lcd off
 * @fn void d4_dp_lcd_off(void)
 * @param void
 * @return
 *
 * @note
 */
void d4_dp_lcd_off(void)
{
	dp_lcd_vid_window_onoff(&lcd_info, DP_WIN0, DP_OFF);
	dp_lcd_vid_window_onoff(&lcd_info, DP_WIN1, DP_OFF);
	dp_lcd_vid_window_onoff(&lcd_info, DP_WIN2, DP_OFF);
	dp_lcd_vid_window_onoff(&lcd_info, DP_WIN3, DP_OFF);

	dp_lcd_grp_window_onoff(&lcd_info, DP_WIN0, DP_OFF);
	dp_lcd_grp_window_onoff(&lcd_info, DP_WIN1, DP_OFF);
	dp_lcd_grp_window_onoff(&lcd_info, DP_WIN2, DP_OFF);
	dp_lcd_grp_window_onoff(&lcd_info, DP_WIN3, DP_OFF);
	dp_interrupt_on_off(&lcd_info, INT_MLCD, DP_ON);
}

void d4_dp_sublcd_off(void)
{
	dp_sublcd_vid_window_onoff(&lcd_info, DP_WIN0, DP_OFF);
	dp_sublcd_vid_window_onoff(&lcd_info, DP_WIN1, DP_OFF);
	dp_sublcd_vid_window_onoff(&lcd_info, DP_WIN2, DP_OFF);
	dp_sublcd_vid_window_onoff(&lcd_info, DP_WIN3, DP_OFF);

	dp_sublcd_grp_window_onoff(&lcd_info, DP_WIN0, DP_OFF);
	dp_sublcd_grp_window_onoff(&lcd_info, DP_WIN1, DP_OFF);
	dp_sublcd_grp_window_onoff(&lcd_info, DP_WIN2, DP_OFF);
	dp_sublcd_grp_window_onoff(&lcd_info, DP_WIN3, DP_OFF);
	dp_interrupt_on_off(&lcd_info, INT_SLCD, DP_ON);
}

void dp_lcd_on(void)
{
	drime4_lcd_display_init();
	dp_lcd_pannel_set();
}

void dp_sublcd_on(void)
{
	drime4_sublcd_display_init();
	dp_sublcd_pannel_set();
}

/**
 * @brief dp lcd pannel manage timming pannel
 * @fn void d4_lcd_pannel_manager_set(struct d4_lcd pannel)
 * @param pannel timming
 * @return
 *
 * @note
 */
void d4_lcd_pannel_manager_set(struct stfb_lcd_pannel pannel)
{
	lcd_pannel = pannel;
	dp_lcd_pannel_set();
}

/**
 * @brief dp sublcd pannel manage timming  pannel
 * @fn void d4_lcd_pannel_manager_set(struct d4_lcd pannel)
 * @param pannel timming
 * @return
 *
 * @note
 */
void d4_sublcd_pannel_manager_set(struct stfb_lcd_pannel pannel)
{
	sublcd_pannel = pannel;
	dp_sublcd_pannel_set();
}

void d4_dp_lcd_graphic_order_change(enum edp_onoff onoff)
{
	/* graphic argb sequence : A/B/G/R  */
	dp_lcd_grp_set_ctrl_onoff(&lcd_info, GRP_CTRL_ARGB_ORDER, onoff);
}

#if 0
#if defined(CONFIG_LCD_CH2330)
extern void ch2330_pannel_onff(void);
extern int ch2330_lcd_init(void);

void d4_dp_lcd_pannel_ctrl(enum edp_onoff onoff)
{
	if (onoff)
	 ch2330_lcd_init();
	else
	 ch2330_pannel_onff();
}

#else
extern void a030vvn01_3_pannel_onff(void);
extern int a030vvn01_3_lcd_init(void);

void d4_dp_lcd_pannel_ctrl(enum edp_onoff onoff)
{
	if (onoff)
	 a030vvn01_3_lcd_init();
	else
	 a030vvn01_3_pannel_onff();
}
#endif
#else
void d4_dp_lcd_pannel_ctrl(enum edp_onoff onoff)
{
}
#endif
