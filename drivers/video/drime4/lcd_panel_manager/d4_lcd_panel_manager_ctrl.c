/**
 * @file d4_lcd_panel_manager_ctrl.c
 * @brief DRIMe4 LCD Panel Manager Control Function File
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>
#include <linux/hs_spi.h>
#include <linux/pinctrl/consumer.h>
#include <asm/cacheflush.h>

#include "d4_lcd_panel_manager_ctrl.h"
#include "../lcd/d4_dp_lcd_dd.h"

/* private variable */
static struct lcd_panel_manager_info *mlcd_1_panel_info;
static struct lcd_panel_manager_info *mlcd_2_panel_info;

static struct lcd_panel_manager_info *main_lcd_panel_info;
static struct lcd_panel_manager_info *sub_lcd_panel_info;

static struct d4_lcd_panel_manager_data *lcd_panel_init_info;

static struct hs_spi_data *mlcd_1_spi_info;
static struct hs_spi_data *mlcd_2_spi_info;
static struct hs_spi_data *slcd_spi_info;

/* private function */

/**< control function */
static void lcd_panel_reset(enum lpm_cotrol_lcd_select select);
static void lcd_panel_power_on(enum lpm_cotrol_lcd_select select);
static void lcd_panel_power_off(enum lpm_cotrol_lcd_select select);
static void lcd_panel_init(enum lpm_cotrol_lcd_select select);
static void lcd_panel_light_on(enum lpm_cotrol_lcd_select select);
static void lcd_panel_light_off(enum lpm_cotrol_lcd_select select);
static void lcd_panel_set_brightness(enum lpm_cotrol_lcd_select select, enum lpm_lcd_brightness_level level);
static unsigned short lcd_panel_get_h_size(enum lpm_cotrol_lcd_select select);
static unsigned short lcd_panel_get_v_size(enum lpm_cotrol_lcd_select select);

static void lcd_panel_standby_on(enum lpm_cotrol_lcd_select select);
static void lcd_panel_standby_off(enum lpm_cotrol_lcd_select select);
static void lcd_panel_h_flip_on(enum lpm_cotrol_lcd_select select);
static void lcd_panel_h_flip_off(enum lpm_cotrol_lcd_select select);
static void lcd_panel_v_flip_on(enum lpm_cotrol_lcd_select select);
static void lcd_panel_v_flip_off(enum lpm_cotrol_lcd_select select);
static void lcd_panel_control_rotation(enum lpm_cotrol_lcd_select select, enum lpm_lcd_rotation rotation);
static void lcd_panel_control_brightness_gamma(enum lpm_cotrol_lcd_select select, enum lpm_lcd_brightness_level level);
static void lcd_panel_control_csc(enum lpm_cotrol_lcd_select select, void *control);
static void lcd_panel_control_tcc(enum lpm_cotrol_lcd_select select, void *control);
static void lcd_panel_control_extension(enum lpm_cotrol_lcd_select select, void *ext_control);
static enum lpm_lcd_ratio lcd_panel_get_display_ratio(enum lpm_cotrol_lcd_select select);

static struct lcd_panel_manager_info *lcd_panel_selection_check(enum lpm_cotrol_lcd_select select);
static void lcd_panel_manager_spi_init(enum lpm_lcd_select lcd_select);
static void lcd_panel_spi_init(void);
static void lcd_panel_gpio_init(void);
static void lcd_panel_gpio_close_all(void);
static int lcd_panel_set_info(enum lpm_lcd_select select, struct lcd_panel_manager_info *panel_info);

static int lcd_panel_spi_write(enum lpm_lcd_select select, struct lcd_panel_spi_write_info *info);
static int lcd_panel_spi_read(enum lpm_lcd_select select, struct lcd_panel_spi_rw_info *info);
static int lcd_panel_spi_rw(enum lpm_lcd_select select, struct lcd_panel_spi_rw_info *info);

static void lcd_panel_set_gpio(enum lpm_lcd_select lcd_select, enum lpm_gpio_select select, enum lpm_onoff set_value);

/**< 임시 코드임 */
extern void d4_lcd_pannel_manager_set(struct stfb_lcd_pannel *pannel);
extern void d4_sublcd_pannel_manager_set(struct stfb_lcd_pannel *pannel);
extern void d4_dp_mlcd_clock_ctrl(unsigned int mlcdclock);
extern void d4_dp_slcd_clock_ctrl(unsigned int slcdclock);

/******************************************************************************/
/*                      Public Function Implementation	                      */
/******************************************************************************/

/******************************************************************************/
/** 				LCD Panel Common Control Function 						***/
/******************************************************************************/

/**
 * @brief Main LCD 1 panel 의 정보를 입력하기 위한 함수
 * @fn int mlcd_1_panel_set_info(struct lcd_panel_manage_info *panel_info)
 * @param  panel_info[in] LCD panel information
 *
 * @return 0/-1, 만약 panel 정보가 NULL 인 경우 -1.
 *
 * @note
 */
int mlcd_1_panel_set_info(struct lcd_panel_manager_info *panel_info)
{
	return lcd_panel_set_info(LPM_MLCD_1, panel_info);
}

/**
 * @brief Main LCD 2 panel 의 정보를 입력하기 위한 함수
 * @fn int mlcd_2_panel_set_info(struct lcd_panel_manager_info *panel_info)
 * @param  panel_info[in] LCD panel information
 *
 * @return 0/-1, 만약 panel 정보가 NULL 인 경우 -1.
 *
 * @note
 */
int mlcd_2_panel_set_info(struct lcd_panel_manager_info *panel_info)
{
	return lcd_panel_set_info(LPM_MLCD_2, panel_info);
}

/**
 * @brief Sub LCD panel 의 정보를 입력하기 위한 함수
 * @fn int slcd_panel_set_info(struct lcd_panel_manager_info *panel_info)
 * @param  panel_info[in] LCD panel information
 *
 * @return 0/-1, 만약 panel 정보가 NULL 인 경우 -1.
 *
 * @note
 */
int slcd_panel_set_info(struct lcd_panel_manager_info *panel_info)
{
	return lcd_panel_set_info(LPM_SLCD, panel_info);
}

/**
 * @brief Main LCD 1 panel 의 SPI init config 설정을 위한 함수
 * @fn int mlcd_1_panel_set_spi_init_config(struct d4_hs_spi_config *config)
 * @param  config[in] SPI config
 *
 * @return 0/-1, 만약 config 정보가 NULL 인 경우 -1.
 *
 * @note
 *        - mlcd_1_panel_set_info 함수 이전에 설정을 하여야 정상적으로 설정이 된다. (주의)
 *        - Board file 에서 설정한 config를 사용하기를 권장한다.
 *                     본 함수는 필요 시에만 사용하기 바란다.
 */
int mlcd_1_panel_set_spi_init_config(struct d4_hs_spi_config *config)
{
	if (config == NULL) {
		LPM_PRINTK("Invalid SPI config: Main LCD 1 \n");
		return -1;
	}
	lcd_panel_init_info->mlcd_1_hw_connect_info->spi_config = config;
	return 0;
}

/**
 * @brief Main LCD 2 panel 의 SPI init config 설정을 위한 함수
 * @fn int mlcd_2_panel_set_spi_init_config(struct d4_hs_spi_config *config)
 * @param  config[in] SPI config
 *
 * @return 0/-1, 만약 config 정보가 NULL 인 경우 -1.
 *
 * @note
 *        - mlcd_2_panel_set_info 함수 이전에 설정을 하여야 정상적으로 설정이 된다. (주의)
 *        - Board file 에서 설정한 config를 사용하기를 권장한다.
 *                     본 함수는 필요 시에만 사용하기 바란다.
 */
int mlcd_2_panel_set_spi_init_config(struct d4_hs_spi_config *config)
{
	if (config == NULL) {
		LPM_PRINTK("Invalid SPI config: Main LCD 2 \n");
		return -1;
	}
	lcd_panel_init_info->mlcd_2_hw_connect_info->spi_config = config;
	return 0;
}

/**
 * @brief Sub LCD panel 의 SPI init config 설정을 위한 함수
 * @fn int slcd_panel_set_spi_init_config(struct d4_hs_spi_config *config)
 * @param  config[in] SPI config
 *
 * @return 0/-1, 만약 config 정보가 NULL 인 경우 -1.
 *
 * @note
 *        - slcd_panel_set_info 함수 이전에 설정을 하여야 정상적으로 설정이 된다. (주의)
 *        - Board file 에서 설정한 config를 사용하기를 권장한다.
 *                     본 함수는 필요 시에만 사용하기 바란다.
 */
int slcd_panel_set_spi_init_config(struct d4_hs_spi_config *config)
{
	if (config == NULL) {
		LPM_PRINTK("Invalid SPI config: Sub LCD \n");
		return -1;
	}
	lcd_panel_init_info->slcd_hw_connect_info->spi_config = config;
	return 0;
}

/**
 * @brief 사용 하려고 하는 Main LCD 선택을 위한 함수
 * @fn int mlcd_panel_select(enum lpm_mlcd_select selection)
 * @param  selection[in] Main LCD 선택을 위한 인자
 *
 * @return 0/-1, 만약 잘못된 LCD 선택을 하는 경우 -1.
 *
 * @note 두 개의 Main LCD 는 동시에 사용을 할 수 없기 때문에,  <br>
 *       Main LCD 변경을 하기 위해서는 먼저 본 함수를 통해서,	 <br>
 *       Main LCD 를 선택을 하여야 한다.					 <br>
 *       기본적으로, main LCD 1 은 일반적인 main LCD 로		 <br>
 *       main LCD 2 는 EVF 로 사용하는 것을 전제로 한다.		 <br>
 */
int mlcd_panel_select(enum lpm_mlcd_select selection)
{
	switch (selection) {
	case LPM_MLCD_1_SELECT:
		if (mlcd_1_panel_info == NULL) {
			printk("[%s()], invalid selection: %d\n", __FUNCTION__, selection);
			return -1;
		}
		main_lcd_panel_info = mlcd_1_panel_info;
		break;
	case LPM_MLCD_2_SELECT:
		if (mlcd_2_panel_info == NULL) {
			printk("[%s()], invalid selection: %d\n", __FUNCTION__, selection);
			return -1;
		}
		main_lcd_panel_info = mlcd_2_panel_info;
		break;
	default:
		printk("[%s()], invalid selection: %d\n", __FUNCTION__, selection);
		return -1;
	}
	return 0;
}

void lcd_panel_suspend(void)
{
	if (mlcd_1_panel_info != NULL) {
		if (mlcd_1_panel_info->control_function->control_suspend == NULL) {
			LPM_PRINTK("No Operation \n");
		} else {
			mlcd_1_panel_info->control_function->control_suspend();
			LPM_PRINTK("Main 1 LCD Suspend \n");
		}
	}

	if (mlcd_2_panel_info != NULL) {
		if (mlcd_2_panel_info->control_function->control_suspend == NULL) {
			LPM_PRINTK("No Operation \n");
		} else {
			mlcd_2_panel_info->control_function->control_suspend();
			LPM_PRINTK("Main 2 LCD Suspend \n");
		}
	}

	if (sub_lcd_panel_info != NULL) {
		if (sub_lcd_panel_info->control_function->control_suspend == NULL) {
			LPM_PRINTK("No Operation \n");
		} else {
			sub_lcd_panel_info->control_function->control_suspend();
			LPM_PRINTK("Sub LCD Suspend \n");
		}
	}

	/**< Close GPIO */
	lcd_panel_gpio_close_all();
}

void lcd_panel_resume(void)
{
	/**< LCD SPI Setting */
	lcd_panel_spi_init();

	/**< LCD GPIO Setting */
	lcd_panel_gpio_init();


	if (mlcd_1_panel_info != NULL) {
		if (mlcd_1_panel_info->control_function->control_resume == NULL) {
			LPM_PRINTK("No Operation \n");
		} else {
			mlcd_1_panel_info->control_function->control_resume();
			LPM_PRINTK("Main 1 LCD Resume \n");
		}
	}

	if (mlcd_2_panel_info != NULL) {
		if (mlcd_2_panel_info->control_function->control_resume == NULL) {
			LPM_PRINTK("No Operation \n");
		} else {
			mlcd_2_panel_info->control_function->control_resume();
			LPM_PRINTK("Main 2 LCD Resume \n");
		}
	}

	if (sub_lcd_panel_info != NULL) {
		if (sub_lcd_panel_info->control_function->control_resume == NULL) {
			LPM_PRINTK("No Operation \n");
		} else {
			sub_lcd_panel_info->control_function->control_resume();
			LPM_PRINTK("Sub LCD Resume \n");
		}
	}
}

void lcd_panel_set_init_data(struct d4_lcd_panel_manager_data *init_data)
{
	mlcd_1_panel_info 	= NULL;
	mlcd_2_panel_info 	= NULL;

	main_lcd_panel_info = NULL;
	sub_lcd_panel_info  = NULL;

	lcd_panel_init_info = init_data;

	/**< LCD SPI Setting */
	lcd_panel_spi_init();

	/**< LCD GPIO Setting */
	lcd_panel_gpio_init();
}

/**
 *  일반적인 경우는 사용하지 않아도 된다.
 *  단, Main LCD1/ Main LCD2/ SubLCD 가
 *       중 같은 SPI channel 을 사용하면서,
 *       SPI 기본 설정값이 다른 LCD 는
 *       SPI 사용전에 이 함수를 실행시켜 주어야
 *       문제가 발생하지 않는다.
 */
void mlcd_1_panel_spi_init(void)
{
	/**< Main LCD 1 SPI Setting */
	lcd_panel_manager_spi_init(LPM_MLCD_1);
}

/**
 *  일반적인 경우는 사용하지 않아도 된다.
 *  단, Main LCD1/ Main LCD2/ SubLCD 가
 *       중 같은 SPI channel 을 사용하면서,
 *       SPI 기본 설정값이 다른 LCD 는
 *       SPI 사용전에 이 함수를 실행시켜 주어야
 *       문제가 발생하지 않는다.
 */
void mlcd_2_panel_spi_init(void)
{
	/**< Main LCD 2 SPI Setting */
	lcd_panel_manager_spi_init(LPM_MLCD_2);
}

/**
 *  일반적인 경우는 사용하지 않아도 된다.
 *  단, Main LCD1/ Main LCD2/ SubLCD 가
 *       중 같은 SPI channel 을 사용하면서,
 *       SPI 기본 설정값이 다른 LCD 는
 *       SPI 사용전에 이 함수를 실행시켜 주어야
 *       문제가 발생하지 않는다.
 */
void slcd_panel_spi_init(void)
{
	/**< Sub LCD SPI Setting */
	lcd_panel_manager_spi_init(LPM_SLCD);
}

int mlcd_1_panel_spi_write(struct lcd_panel_spi_write_info *set_info)
{
	return lcd_panel_spi_write(LPM_MLCD_1, set_info);
}

int mlcd_2_panel_spi_write(struct lcd_panel_spi_write_info *set_info)
{
	return lcd_panel_spi_write(LPM_MLCD_2, set_info);
}

int slcd_panel_spi_write(struct lcd_panel_spi_write_info *set_info)
{
	return lcd_panel_spi_write(LPM_SLCD, set_info);
}

int mlcd_1_panel_spi_read(struct lcd_panel_spi_rw_info *set_info)
{
	return lcd_panel_spi_read(LPM_MLCD_1, set_info);
}

int mlcd_2_panel_spi_read(struct lcd_panel_spi_rw_info *set_info)
{
	return lcd_panel_spi_read(LPM_MLCD_2, set_info);
}

int slcd_panel_spi_read(struct lcd_panel_spi_rw_info *set_info)
{
	return lcd_panel_spi_read(LPM_SLCD, set_info);
}

int mlcd_1_panel_spi_rw(struct lcd_panel_spi_rw_info *set_info)
{
	return lcd_panel_spi_rw(LPM_MLCD_1, set_info);
}

int mlcd_2_panel_spi_rw(struct lcd_panel_spi_rw_info *set_info)
{
	return lcd_panel_spi_rw(LPM_MLCD_2, set_info);
}

int slcd_panel_spi_rw(struct lcd_panel_spi_rw_info *set_info)
{
	return lcd_panel_spi_rw(LPM_SLCD, set_info);
}

void mlcd_1_panel_set_gpio(enum lpm_gpio_select select, enum lpm_onoff set_value)
{
	lcd_panel_set_gpio(LPM_MLCD_1, select, set_value);
}

void mlcd_2_panel_set_gpio(enum lpm_gpio_select select, enum lpm_onoff set_value)
{
	lcd_panel_set_gpio(LPM_MLCD_2, select, set_value);
}

void slcd_panel_set_gpio(enum lpm_gpio_select select, enum lpm_onoff set_value)
{
	lcd_panel_set_gpio(LPM_SLCD, select, set_value);
}

/******************************************************************************/
/** 				Main LCD Panel Control Function 						***/
/******************************************************************************/

/**< Basic Control Function */

void mlcd_panel_reset(void)
{
	lcd_panel_reset(LPM_MLCD_CTRL);
}

void mlcd_panel_power_on(void)
{
	lcd_panel_power_on(LPM_MLCD_CTRL);
}

void mlcd_panel_power_off(void)
{
	lcd_panel_power_off(LPM_MLCD_CTRL);
}

void mlcd_panel_init(void)
{
	lcd_panel_init(LPM_MLCD_CTRL);
}

void mlcd_panel_light_on(void)
{
	lcd_panel_light_on(LPM_MLCD_CTRL);
}

void mlcd_panel_light_off(void)
{
	lcd_panel_light_off(LPM_MLCD_CTRL);
}

void mlcd_panel_set_brightness(enum lpm_lcd_brightness_level level)
{
	lcd_panel_set_brightness(LPM_MLCD_CTRL, level);
}

unsigned short mlcd_panel_get_h_size(void)
{
	return lcd_panel_get_h_size(LPM_MLCD_CTRL);
}

unsigned short mlcd_panel_get_v_size(void)
{
	return lcd_panel_get_v_size(LPM_MLCD_CTRL);
}

/**< Optional Control Function */

void mlcd_panel_standby_on(void)
{
	lcd_panel_standby_on(LPM_MLCD_CTRL);
}

void mlcd_panel_standby_off(void)
{
	lcd_panel_standby_off(LPM_MLCD_CTRL);
}

void mlcd_panel_h_flip_on(void)
{
	lcd_panel_h_flip_on(LPM_MLCD_CTRL);
}

void mlcd_panel_h_flip_off(void)
{
	lcd_panel_h_flip_off(LPM_MLCD_CTRL);
}

void mlcd_panel_v_flip_on(void)
{
	lcd_panel_v_flip_on(LPM_MLCD_CTRL);
}

void mlcd_panel_v_flip_off(void)
{
	lcd_panel_v_flip_off(LPM_MLCD_CTRL);
}

void mlcd_panel_control_rotation(enum lpm_lcd_rotation rotation)
{
	lcd_panel_control_rotation(LPM_MLCD_CTRL, rotation);
}

void mlcd_panel_control_brightness_gamma(enum lpm_lcd_brightness_level level)
{
	lcd_panel_control_brightness_gamma(LPM_MLCD_CTRL, level);
}

void mlcd_panel_control_csc(void *control)
{
	lcd_panel_control_csc(LPM_MLCD_CTRL, control);
}

void mlcd_panel_control_tcc(void *control)
{
	lcd_panel_control_tcc(LPM_MLCD_CTRL, control);
}

void mlcd_panel_control_extension(void *ext_control)
{
	lcd_panel_control_extension(LPM_MLCD_CTRL, ext_control);
}

enum lpm_lcd_ratio mlcd_panel_get_display_ratio(void)
{
	return lcd_panel_get_display_ratio(LPM_MLCD_CTRL);
}

/******************************************************************************/
/** 					Sub LCD Panel Control Function 						***/
/******************************************************************************/

/**< Basic Control Function */

void slcd_panel_reset(void)
{
	lcd_panel_reset(LPM_SLCD_CTRL);
}

void slcd_panel_power_on(void)
{
	lcd_panel_power_on(LPM_SLCD_CTRL);
}

void slcd_panel_power_off(void)
{
	lcd_panel_power_off(LPM_SLCD_CTRL);
}

void slcd_panel_init(void)
{
	lcd_panel_init(LPM_SLCD_CTRL);
}

void slcd_panel_light_on(void)
{
	lcd_panel_light_on(LPM_SLCD_CTRL);
}

void slcd_panel_light_off(void)
{
	lcd_panel_light_off(LPM_SLCD_CTRL);
}

void slcd_panel_set_brightness(enum lpm_lcd_brightness_level level)
{
	lcd_panel_set_brightness(LPM_SLCD_CTRL, level);
}

unsigned short slcd_panel_get_h_size(void)
{
	return lcd_panel_get_h_size(LPM_SLCD_CTRL);
}

unsigned short slcd_panel_get_v_size(void)
{
	return lcd_panel_get_v_size(LPM_SLCD_CTRL);
}

/**< Optional Control Function */

void slcd_panel_standby_on(void)
{
	lcd_panel_standby_on(LPM_SLCD_CTRL);
}

void slcd_panel_standby_off(void)
{
	lcd_panel_standby_off(LPM_SLCD_CTRL);
}

void slcd_panel_h_flip_on(void)
{
	lcd_panel_h_flip_on(LPM_SLCD_CTRL);
}

void slcd_panel_h_flip_off(void)
{
	lcd_panel_h_flip_off(LPM_SLCD_CTRL);
}

void slcd_panel_v_flip_on(void)
{
	lcd_panel_v_flip_on(LPM_SLCD_CTRL);
}

void slcd_panel_v_flip_off(void)
{
	lcd_panel_v_flip_off(LPM_SLCD_CTRL);
}

void slcd_panel_control_rotation(enum lpm_lcd_rotation rotation)
{
	lcd_panel_control_rotation(LPM_SLCD_CTRL, rotation);
}

void slcd_panel_control_brightness_gamma(enum lpm_lcd_brightness_level level)
{
	lcd_panel_control_brightness_gamma(LPM_SLCD_CTRL, level);
}

void slcd_panel_control_csc(void *control)
{
	lcd_panel_control_csc(LPM_SLCD_CTRL, control);
}

void slcd_panel_control_tcc(void *control)
{
	lcd_panel_control_tcc(LPM_SLCD_CTRL, control);
}

void slcd_panel_control_extension(void *ext_control)
{
	lcd_panel_control_extension(LPM_SLCD_CTRL, ext_control);
}

enum lpm_lcd_ratio slcd_panel_get_display_ratio(void)
{
	return lcd_panel_get_display_ratio(LPM_SLCD_CTRL);
}

/******************************************************************************/
/*                        Private Function Implementation                     */
/******************************************************************************/

/**< Basic Control Function */

static void lcd_panel_reset(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->reset == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->reset();
		LPM_PRINTK("LCD[%d] Reset(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_power_on(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->power_on == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->power_on();
		LPM_PRINTK("LCD[%d] Power ON(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_power_off(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->power_off == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->power_off();
		LPM_PRINTK("LCD[%d] Power OFF(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_init(enum lpm_cotrol_lcd_select select)
{
	struct stfb_lcd_pannel dp_pannel_info;
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	/**< LCD Clock 설정 */
	switch (select) {
	case LPM_MLCD_CTRL:
		d4_dp_mlcd_clock_ctrl(lcd_panel_info->hw_info->lcd_clock);
		break;
	case LPM_SLCD_CTRL:
		d4_dp_slcd_clock_ctrl(lcd_panel_info->hw_info->lcd_clock);
		break;
	}

	/**< LCD timing signal control setting */
	dp_pannel_info.timing.total_h_size		= lcd_panel_info->timing_signal_info->total_h_size;
	dp_pannel_info.timing.total_v_size		= lcd_panel_info->timing_signal_info->total_v_size;

	dp_pannel_info.timing.h_sync_rise		= lcd_panel_info->timing_signal_info->hsync_rising;
	dp_pannel_info.timing.h_sync_fall		= lcd_panel_info->timing_signal_info->hsync_falling;

	dp_pannel_info.timing.v_sync_rise		= lcd_panel_info->timing_signal_info->vsync_rising;
	dp_pannel_info.timing.v_sync_fall		= lcd_panel_info->timing_signal_info->vsync_falling;

	dp_pannel_info.timing.buf_read_h_start	= lcd_panel_info->timing_signal_info->buf_read_h_start;

	dp_pannel_info.timing.enable_h_start	= lcd_panel_info->timing_signal_info->enable_h_start;
	dp_pannel_info.timing.enable_h_end 		= dp_pannel_info.timing.enable_h_start
											  + lcd_panel_info->timing_signal_info->enable_h_size;

	dp_pannel_info.timing.enable_v_start	= lcd_panel_info->timing_signal_info->enable_v_start;
	dp_pannel_info.timing.enable_v_end 		= dp_pannel_info.timing.enable_v_start
											  + lcd_panel_info->timing_signal_info->enable_v_size;

	dp_pannel_info.timing.inv_dot_clk		= lcd_panel_info->timing_signal_info->inv_dot_clk;
	dp_pannel_info.timing.inv_enable_clk	= lcd_panel_info->timing_signal_info->inv_enable_clk;
	dp_pannel_info.timing.inv_h_sync		= lcd_panel_info->timing_signal_info->inv_hsync;
	dp_pannel_info.timing.inv_v_sync		= lcd_panel_info->timing_signal_info->inv_vsync;

	/**< LCD H/W information setting for DP LCD */
	dp_pannel_info.h_size = lcd_panel_info->hw_info->lcd_width_size;
	dp_pannel_info.v_size = lcd_panel_info->hw_info->lcd_height_size;

	/**< LCD output format setting */
	if (lcd_panel_info->hw_info->lcd_out_format == LPM_RGB_OUT) {
		switch (lcd_panel_info->hw_info->lcd_out_bitwith) {
		case LPM_BIT_WIDTH_8:
		case LPM_BIT_WIDTH_24:
			dp_pannel_info.lcd_data_width
					= lcd_panel_info->hw_info->lcd_out_bitwith;
			break;
		case LPM_BIT_WIDTH_16:
			break;
		}
	} else if (lcd_panel_info->hw_info->lcd_out_format == LPM_YCC_OUT) {
		switch (lcd_panel_info->hw_info->lcd_out_bitwith) {
		case LPM_BIT_WIDTH_8:
			break;
		case LPM_BIT_WIDTH_16:
			break;
		case LPM_BIT_WIDTH_24:
			break;
		}
	} else if (lcd_panel_info->hw_info->lcd_out_format == LPM_CCIR656_OUT) {
		switch (lcd_panel_info->hw_info->lcd_out_bitwith) {
		case LPM_BIT_WIDTH_8:
			break;
		case LPM_BIT_WIDTH_16:
			break;
		case LPM_BIT_WIDTH_24:
			break;
		}
	}

	dp_pannel_info.type = lcd_panel_info->hw_info->lcd_dot_array_type;
	dp_pannel_info.even_seq
			= lcd_panel_info->hw_info->lcd_dot_even_sequence;
	dp_pannel_info.odd_seq = lcd_panel_info->hw_info->lcd_dot_odd_sequence;

	/**< DRIMe4 DP LCD setting */
	switch (select) {
	case LPM_MLCD_CTRL:
		d4_lcd_pannel_manager_set(&dp_pannel_info);
		break;
	case LPM_SLCD_CTRL:
		/**< Sub LCD initialization */
		drime4_sublcd_display_init();
		d4_sublcd_pannel_manager_set(&dp_pannel_info);
		break;
	}

	/**< execute LCD panel initialization control function */
	if (lcd_panel_info->control_function->init == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->init();
		LPM_PRINTK("LCD[%d] Initialization(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_light_on(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->light_on == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->light_on();
		LPM_PRINTK("LCD[%d] Light ON(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_light_off(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->light_off == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->light_off();
		LPM_PRINTK("LCD[%d] Light OFF(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_set_brightness(enum lpm_cotrol_lcd_select select, enum lpm_lcd_brightness_level level)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->set_brightness == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->set_brightness(level);
		LPM_PRINTK("LCD[%d] Brightness setting(0: Main, 1: Sub) \n", select);
	}
}

static unsigned short lcd_panel_get_h_size(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return 0;

	return lcd_panel_info->hw_info->lcd_width_size;
}

static unsigned short lcd_panel_get_v_size(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return 0;

	return lcd_panel_info->hw_info->lcd_height_size;
}

/**< Optional Control Function */

static void lcd_panel_standby_on(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_standby_on == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_standby_on();
		LPM_PRINTK("LCD[%d] Standby ON(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_standby_off(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_standby_off == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_standby_off();
		LPM_PRINTK("LCD[%d] Standby OFF(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_h_flip_on(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_h_flip_on == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_h_flip_on();
		LPM_PRINTK("LCD[%d] H Flip ON(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_h_flip_off(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_h_flip_off == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_h_flip_off();
		LPM_PRINTK("LCD[%d] H Flip OFF(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_v_flip_on(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_v_flip_on == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_v_flip_on();
		LPM_PRINTK("LCD[%d] V Flip ON(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_v_flip_off(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_v_flip_off == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_v_flip_off();
		LPM_PRINTK("LCD[%d] V Flip OFF(0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_control_rotation(enum lpm_cotrol_lcd_select select, enum lpm_lcd_rotation rotation)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_rotation == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_rotation(rotation);
		LPM_PRINTK("LCD[%d] control Rotation[%d] (0: Main, 1: Sub) \n", select, rotation);
	}
}

static void lcd_panel_control_brightness_gamma(enum lpm_cotrol_lcd_select select, enum lpm_lcd_brightness_level level)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_brightness_gamma == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_brightness_gamma(level);
		LPM_PRINTK("LCD[%d] control Brightness Level[%d] (0: Main, 1: Sub) \n", select, level);
	}
}

static void lcd_panel_control_csc(enum lpm_cotrol_lcd_select select, void *control)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_csc == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_csc(control);
		LPM_PRINTK("LCD[%d] control extension (0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_control_tcc(enum lpm_cotrol_lcd_select select, void *control)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_tcc == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_tcc(control);
		LPM_PRINTK("LCD[%d] control extension (0: Main, 1: Sub) \n", select);
	}
}

static void lcd_panel_control_extension(enum lpm_cotrol_lcd_select select, void *ext_control)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return;

	if (lcd_panel_info->control_function->control_extension == NULL) {
		LPM_PRINTK("No Operation \n");
	} else {
		lcd_panel_info->control_function->control_extension(ext_control);
		LPM_PRINTK("LCD[%d] control extension (0: Main, 1: Sub) \n", select);
	}
}

static enum lpm_lcd_ratio lcd_panel_get_display_ratio(enum lpm_cotrol_lcd_select select)
{
	struct lcd_panel_manager_info *lcd_panel_info = NULL;

	lcd_panel_info = lcd_panel_selection_check(select);
	if (lcd_panel_info == NULL)
		return LPM_RATIO_4_3;

	return lcd_panel_info->hw_info->lcd_ratio;
}

static int mlcd_selection_check(void)
{
	if (main_lcd_panel_info == NULL) {
		printk("[%s()], Main LCD: No Information\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

static int slcd_selection_check(void)
{
	if (sub_lcd_panel_info == NULL) {
		printk("[%s()], Sub LCD: No Information\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

static struct lcd_panel_manager_info *lcd_panel_selection_check(enum lpm_cotrol_lcd_select select)
{
	switch (select) {
	case LPM_MLCD_CTRL:
		if (mlcd_selection_check() < 0)
			return NULL;
		return main_lcd_panel_info;
	case LPM_SLCD_CTRL:
		if (slcd_selection_check() < 0)
			return NULL;
		return sub_lcd_panel_info;
	default:
		return NULL;
	}
	return NULL;
}

static int lcd_panel_set_info(enum lpm_lcd_select select, struct lcd_panel_manager_info *panel_info)
{
	if (panel_info == NULL) {
		printk("invalid argument[%s]", __FUNCTION__);
		return -1;
	}

	switch (select) {
	case LPM_MLCD_1:
		mlcd_1_panel_info = panel_info;
		break;
	case LPM_MLCD_2:
		mlcd_2_panel_info = panel_info;
		break;
	case LPM_SLCD:
		sub_lcd_panel_info = panel_info;
		break;
	default:
		printk("invalid argument[ %s, %d]\n", __FUNCTION__, __LINE__);
		return -1;
	}
    return 0;
}

static int lcd_panel_spi_write(enum lpm_lcd_select select, struct lcd_panel_spi_write_info *info)
{
	int ret = -1;
	struct spi_data_info spi_w_data;
	struct hs_spi_data *lcd_spi_info;

	switch (select) {
	case LPM_MLCD_1:
		lcd_spi_info = mlcd_1_spi_info;
		break;
	case LPM_MLCD_2:
		lcd_spi_info = mlcd_2_spi_info;
		break;
	case LPM_SLCD:
		lcd_spi_info = slcd_spi_info;
		break;
	default:
		printk("invalid argument[ %s, %d]\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (lcd_spi_info == NULL) {
		LPM_PRINTK("LCD [ %d ] SPI Control: Fail \n", select);
		return 0;
	}

    spi_w_data.data_len = info->data_cnt;
    spi_w_data.wbuffer = info->data_buf;
    spi_w_data.rbuffer = NULL;

    ret = hs_spi_polling_write(lcd_spi_info, &spi_w_data);

	if (ret < 0)
		return ret;

    return 0;
}

static int lcd_panel_spi_read(enum lpm_lcd_select select, struct lcd_panel_spi_rw_info *info)
{
	int ret = -1;
	struct spi_data_info spi_w_data;
	struct hs_spi_data *lcd_spi_info;

	switch (select) {
	case LPM_MLCD_1:
		lcd_spi_info = mlcd_1_spi_info;
		break;
	case LPM_MLCD_2:
		lcd_spi_info = mlcd_2_spi_info;
		break;
	case LPM_SLCD:
		lcd_spi_info = slcd_spi_info;
		break;
	default:
		printk("invalid argument[ %s, %d]\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (lcd_spi_info == NULL) {
		LPM_PRINTK("LCD [ %d ] SPI Control: Fail \n", select);
		return 0;
	}

    spi_w_data.data_len = info->data_cnt;
    spi_w_data.wbuffer = info->w_data_buf;
    spi_w_data.rbuffer = info->r_data_buf;;

    ret = hs_spi_polling_read(lcd_spi_info, &spi_w_data);

	if (ret < 0)
		return ret;

    return 0;
}

static int lcd_panel_spi_rw(enum lpm_lcd_select select, struct lcd_panel_spi_rw_info *info)
{
	int ret = -1;
	struct spi_data_info spi_w_data;
	struct hs_spi_data *lcd_spi_info;

	switch (select) {
	case LPM_MLCD_1:
		lcd_spi_info = mlcd_1_spi_info;
		break;
	case LPM_MLCD_2:
		lcd_spi_info = mlcd_2_spi_info;
		break;
	case LPM_SLCD:
		lcd_spi_info = slcd_spi_info;
		break;
	default:
		printk("invalid argument[ %s, %d]\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (lcd_spi_info == NULL) {
		LPM_PRINTK("LCD [ %d ] SPI Control: Fail \n", select);
		return 0;
	}

    spi_w_data.data_len = info->data_cnt;
    spi_w_data.wbuffer = info->w_data_buf;
    spi_w_data.rbuffer = info->r_data_buf;;

    ret = hs_spi_polling_rw(lcd_spi_info, &spi_w_data);

	if (ret < 0)
		return ret;

    return 0;
}

static void lcd_panel_manager_set_gpio(unsigned int select, enum lpm_onoff set_value)
{
	if (select == LPM_HW_CONNECT_NO_USE) {
		LPM_PRINTK("GPIO : NO using \n");
		return;
	}

	gpio_direction_output(select, (int)set_value);
}

static void lcd_panel_set_gpio(enum lpm_lcd_select lcd_select, enum lpm_gpio_select gpio_select, enum lpm_onoff set_value)
{
	unsigned int selected_gpio = 0x1000;
	struct d4_lcd_hw_connect_info *lcd_hw_connect_info;

	switch (lcd_select) {
	case LPM_MLCD_1:
		lcd_hw_connect_info = lcd_panel_init_info->mlcd_1_hw_connect_info;
		break;
	case LPM_MLCD_2:
		lcd_hw_connect_info = lcd_panel_init_info->mlcd_2_hw_connect_info;
		break;
	case LPM_SLCD:
		lcd_hw_connect_info = lcd_panel_init_info->slcd_hw_connect_info;
		break;
	default:
		printk("invalid argument[ %s, %d]\n", __FUNCTION__, __LINE__);
		return;
	}

	if (lcd_hw_connect_info == NULL) {
		LPM_PRINTK("LCD[ %d ] GPIO Control: Fail \n", lcd_select);
		return;
	}

	switch (gpio_select) {
	case LPM_GPIO_RESET:
		selected_gpio = lcd_hw_connect_info->gpio_reset;
		break;
	case LPM_GPIO_POWER_1:
		selected_gpio = lcd_hw_connect_info->gpio_power_1;
		break;
	case LPM_GPIO_POWER_2:
		selected_gpio = lcd_hw_connect_info->gpio_power_2;
		break;
	case LPM_GPIO_POWER_3:
		selected_gpio = lcd_hw_connect_info->gpio_power_3;
		break;
	}
	lcd_panel_manager_set_gpio(selected_gpio, set_value);
	LPM_PRINTK("GPIO Set: %d, %d \n", selected_gpio, set_value);
}

/**< SPI Setting 은 SPI Channel 설정이 LPM_HW_CONNECT_NO_USE 이거나,
 *   HW connect info 가 NULL 인 경우 하지 않고 넘어간다.
 */
static void lcd_panel_manager_spi_init(enum lpm_lcd_select lcd_select)
{
	struct d4_lcd_hw_connect_info *lcd_hw_connect_info;
	struct hs_spi_data *lcd_spi_info;

	switch (lcd_select) {
	case LPM_MLCD_1:
		lcd_hw_connect_info = lcd_panel_init_info->mlcd_1_hw_connect_info;
		lcd_spi_info		= mlcd_1_spi_info;
		break;
	case LPM_MLCD_2:
		lcd_hw_connect_info = lcd_panel_init_info->mlcd_2_hw_connect_info;
		lcd_spi_info		= mlcd_2_spi_info;
		break;
	case LPM_SLCD:
		lcd_hw_connect_info = lcd_panel_init_info->slcd_hw_connect_info;
		lcd_spi_info		= slcd_spi_info;
		break;
	default:
		printk("invalid argument[ %s, %d]\n", __FUNCTION__, __LINE__);
		return;
	}

	if (lcd_hw_connect_info == NULL) {
		LPM_PRINTK("LCD[ %d ] SPI Init: Skip \n", lcd_select);
	} else if (lcd_hw_connect_info->spi_ch == LPM_HW_CONNECT_NO_USE) {
		LPM_PRINTK("LCD[ %d ] SPI Init: Skip \n", lcd_select);
	} else {
		lcd_spi_info = hs_spi_request(lcd_hw_connect_info->spi_ch);
		if (lcd_spi_info == NULL) {
			printk("LCD[ %d ] SPI request: Fail \n", lcd_select);
			return;
		}
		hs_spi_config(lcd_spi_info, lcd_hw_connect_info->spi_config);

		switch (lcd_select) {
		case LPM_MLCD_1:
			mlcd_1_spi_info		= lcd_spi_info;
			break;
		case LPM_MLCD_2:
			mlcd_2_spi_info		= lcd_spi_info;
			break;
		case LPM_SLCD:
			slcd_spi_info		= lcd_spi_info;
			break;
		}
		LPM_PRINTK("LCD[ %d ] SPI Init \n", lcd_select);
	}
}


static void lcd_panel_spi_init(void)
{
	/**< Main LCD 1 SPI Setting */
	lcd_panel_manager_spi_init(LPM_MLCD_1);

	/**< Main LCD 2 SPI Setting */
	lcd_panel_manager_spi_init(LPM_MLCD_2);

	/**< Sub LCD SPI Setting */
	lcd_panel_manager_spi_init(LPM_SLCD);
}

static int lcd_panel_open_gpio(unsigned int select)
{
	int ret = -1;

	if (select == LPM_HW_CONNECT_NO_USE) {
		LPM_PRINTK("GPIO : NO using \n");
		return 0;
	}

	/**< GPIO Setting */
	ret = pinctrl_request_gpio(select);
	if (ret) {
		printk("pinmux request gpio fail: %d\n", select);
		return -EINVAL;
	}

	ret = gpio_request(select, "");
	if (ret < 0) {
		printk("request gpio fail: %d\n", select);
		return -EINVAL;
	}
	LPM_PRINTK("GPIO[%d] Init \n", select);
	return 0;
}

static void lcd_panel_close_gpio(unsigned int select)
{
	if (select == LPM_HW_CONNECT_NO_USE) {
		LPM_PRINTK("GPIO : NO using \n");
		return;
	}

	gpio_free(select);
	pinctrl_free_gpio(select);
}

static void lcd_panel_manager_gpio_init(enum lpm_lcd_select lcd_select)
{
	struct d4_lcd_hw_connect_info *lcd_hw_connect_info;

	switch (lcd_select) {
	case LPM_MLCD_1:
		lcd_hw_connect_info = lcd_panel_init_info->mlcd_1_hw_connect_info;
		break;
	case LPM_MLCD_2:
		lcd_hw_connect_info = lcd_panel_init_info->mlcd_2_hw_connect_info;
		break;
	case LPM_SLCD:
		lcd_hw_connect_info = lcd_panel_init_info->slcd_hw_connect_info;
		break;
	default:
		printk("invalid argument[ %s, %d]\n", __FUNCTION__, __LINE__);
		return;
	}

	if (lcd_hw_connect_info == NULL) {
		LPM_PRINTK("LCD[ %d ] GPIO Setting: Skip \n", lcd_select);
	} else {
		lcd_panel_open_gpio(lcd_hw_connect_info->gpio_reset);
		lcd_panel_open_gpio(lcd_hw_connect_info->gpio_power_1);
		lcd_panel_open_gpio(lcd_hw_connect_info->gpio_power_2);
		lcd_panel_open_gpio(lcd_hw_connect_info->gpio_power_3);
	}
}

static void lcd_panel_gpio_init(void)
{
	/**< Main LCD 1 GPIO Setting */
	lcd_panel_manager_gpio_init(LPM_MLCD_1);

	/**< Main LCD 2 GPIO Setting */
	lcd_panel_manager_gpio_init(LPM_MLCD_2);

	/**< Sub LCD GPIO Setting */
	lcd_panel_manager_gpio_init(LPM_SLCD);
}

static void lcd_panel_manager_gpio_close_all(enum lpm_lcd_select lcd_select)
{
	struct d4_lcd_hw_connect_info *lcd_hw_connect_info;

	switch (lcd_select) {
	case LPM_MLCD_1:
		lcd_hw_connect_info = lcd_panel_init_info->mlcd_1_hw_connect_info;
		break;
	case LPM_MLCD_2:
		lcd_hw_connect_info = lcd_panel_init_info->mlcd_2_hw_connect_info;
		break;
	case LPM_SLCD:
		lcd_hw_connect_info = lcd_panel_init_info->slcd_hw_connect_info;
		break;
	default:
		printk("invalid argument[ %s, %d]\n", __FUNCTION__, __LINE__);
		return;
	}

	if (lcd_hw_connect_info == NULL) {
		LPM_PRINTK("LCD[ %d ] GPIO Setting: Skip \n", lcd_select);
	} else {
		lcd_panel_close_gpio(lcd_hw_connect_info->gpio_reset);
		lcd_panel_close_gpio(lcd_hw_connect_info->gpio_power_1);
		lcd_panel_close_gpio(lcd_hw_connect_info->gpio_power_2);
		lcd_panel_close_gpio(lcd_hw_connect_info->gpio_power_3);
	}
}


static void lcd_panel_gpio_close_all(void)
{
	/**< Main LCD 1 GPIO Setting */
	lcd_panel_manager_gpio_close_all(LPM_MLCD_1);

	/**< Main LCD 2 GPIO Setting */
	lcd_panel_manager_gpio_close_all(LPM_MLCD_2);

	/**< Sub LCD GPIO Setting */
	lcd_panel_manager_gpio_close_all(LPM_SLCD);
}
