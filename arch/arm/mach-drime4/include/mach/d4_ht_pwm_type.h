/**
 * @file d4_ht_pwm_type.h
 * @brief DRIMe4 Hardware Trigger PWM Type
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_HT_PWM_TYPE_H
#define _D4_HT_PWM_TYPE_H


enum ht_pwm_op_mode {
	ONETYPE_PULSE,     /**< Just using one pulse type */
	MIX_PULSE,         /**< Just using mix pulse type */
	CONTINUE_PULSE     /**< Just using continuous pulse type */
};

enum ht_pwm_trigger_type {
	SW_TRIGGER,        /**< Start Type for PWM Trigger is Enable bit setting */
	HW_TRIGGER						/**< Start Type for PWM Trigger is External or VD Input */
};

enum ht_pwm_trigger_sor {
	VD_TRIGGER,       /**< HW_TRIGGER VD Sync Input source */
	EXT_TRIGGER       /**< HW_TRIGGER External Input source */
};

enum ht_pwm_start_edge {
	RISING_EDGE,			/**< Rising edge Trigger */
	FALLING_EDGE    	/**< Falling edge Trigger */
};

enum ht_pwm_end_type {
	LOW_END,			/**< Low 상태에서 종료 */
	HIGH_END			/**< High 상태에서 종료 */
};

enum ht_pwm_invert_type {
	INVERT_OFF,			/**< Output Signal Invert Off */
	INVERT_ON				/**< Output Signal Invert On */
};

enum ht_pwm_invert_time {
	INVERT_CONFIG,			/**< Invert enable time is config set */
	INVERT_START			/**< Invert enable time is pwm enable time */
};

enum ht_pwm_int_enable_type {
	ALL_PERIOD,							/**< all period interrupt enable */
	MIX_FIRST_PERIOD,				/**< just first signal period interrupt enable */
	MIX_SECOND_PERIOD,		  /**< just second signal period interrupt enable */
	MIX_LAST_END,						/**< just only last time interrupt enable */
	NOT_USED
};


enum ht_pwm_extint {
	EXTINPUT_0,        /**< EXTINPUT Resource 0  */
	EXTINPUT_1,        /**< EXTINPUT Resource 1  */
	EXTINPUT_2,        /**< EXTINPUT Resource 2  */
	EXTINPUT_3,        /**< EXTINPUT Resource 3  */
	EXTINPUT_4,        /**< EXTINPUT Resource 4  */
	EXTINPUT_5,        /**< EXTINPUT Resource 5  */
	EXTINPUT_6,        /**< EXTINPUT Resource 6  */
	EXTINPUT_7,        /**< EXTINPUT Resource 7  */
	EXTINPUT_8         /**< EXTINPUT Resource 8  */
};




struct ht_pwm_cyc_info {
	unsigned long long	period;	/**<  PWM Period value set*/
	unsigned long long	duty;			/**<  PWM Duty value set*/
	unsigned char count;					/**<  PWM number of plus set*/
};



struct ht_pwm_conf_info {
	enum ht_pwm_op_mode					opmode;					/**<  PWM의 생성 파형의 형태를 선택*/
	enum ht_pwm_trigger_type	trigtype;    			/**<  PWM 동작의 시작의 type을 선택*/
	struct ht_pwm_cyc_info			freq1;        		/**<  PWM의 동작모드 중 Continue 동작을 실행시키기 위해 설정하거나 mix tpye의 처음 동작 파형을 구현하기 위해 설정 */
	struct ht_pwm_cyc_info			freq2;        		/**<  PWM의 동작모드 중 on plus를 생성하기 위해 설정하거나 mix tpye의 두 번째 동작 파형을 구현하기 위해 설정 */
	enum ht_pwm_trigger_sor		resource;					/**<  PWM Hardware Trigger의 Type을 선택함*/
	enum ht_pwm_start_edge			starttype;					/**<  PWM Hardware Trigger가 반응하는 입력 상태의 Edge Type 설정 */
	enum ht_pwm_end_type				endtype;						/**<  끝나는  Signal을 high 또는 low로 설정*/
	enum ht_pwm_invert_type		invert;						/**<  만들어진 파형 전체를 뒤집을 경우 설정*/
	enum ht_pwm_invert_time		ivt_time;						/**<  INVERT를 설정하는 시기 설정*/
};


struct ht_pwm_mconf_info {
    struct ht_pwm_conf_info mch_config;
    struct ht_pwm_conf_info sch_config;
};

struct ht_pwm_phys_reg_info {
  unsigned int reg_ch_addr;
  unsigned int reg_ch_size;
  unsigned int schannel;
  int swap_check;
};


#endif /* _D4_HT_PWM_TYPE_H */
