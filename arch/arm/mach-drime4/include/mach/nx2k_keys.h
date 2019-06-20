#ifndef _D4KEYS_H_
#define _D4KEYS_H_

struct d4_keys_button {
	/* Configuration parameters */
	int code;		/* input event code (KEY_*, SW_*) */
	int code2nd;	/* one gpio needs 2nd code */
	int gpio;
	int active_low;
	char *desc;
	int type;		/* input event type (EV_KEY, EV_SW) */
	int wakeup;		/* configure the button as a wake-up source */
	int debounce_interval;	/* debounce ticks interval in msecs */
	bool can_disable;
	bool intrrupt; /* enable interrupt or not about gpio*/
	bool adc_input; /*adc type key ? true : false*/
	unsigned int ref_mvolt;
	unsigned int time_margin;
	unsigned int data_diff;
	int adc_useint;
	int adc_ch;
};

struct d4_keys_platform_data {
	struct d4_keys_button *buttons;
	int nbuttons;
	unsigned int poll_interval;	/* polling interval in msecs -
					for polling driver only */
	unsigned int rep:1;		/* enable input subsystem auto repeat */
	int (*enable)(struct device *dev);
	void (*disable)(struct device *dev);
};



struct d4_keys_button_polled {
	/* Configuration parameters */
	int code;		/* input event code (KEY_*, SW_*) */
	int gpio;
	char *desc;
	int type;		/* input event type (EV_KEY, EV_SW) */
	int debounce_interval;	/* debounce ticks interval in msecs */
	bool adc_input; /*adc type key ? true : false*/

	unsigned int ref_mvolt;
	unsigned int time_margin;
	unsigned int data_diff;
	int adc_useint;
	int adc_ch;
	int adc_type; /*key[0], mode[1]*/
};

struct d4_keys_platform_data_polled {
	struct d4_keys_button_polled *buttons;
	int nbuttons;
	unsigned int poll_interval;	/* polling interval in msecs -
					for polling driver only */
	unsigned int rep:1;		/* enable input subsystem auto repeat */
	int (*enable)(struct device *dev);
	void (*disable)(struct device *dev);
};

enum KEYMASK_ID{
	MASK_INVALID 	= 0x00000000,

	MASK_UP 		= 0x00000001,
	MASK_DOWN 		= 0x00000002,
	MASK_RIGHT 		= 0x00000004,
	MASK_LEFT 		= 0x00000008,

	MASK_OK 		= 0x00000010,
	MASK_EV 		= 0x00000020,
	MASK_FN 		= 0x00000040,
	MASK_MENU 		= 0x00000080,

	MASK_DELET 		= 0x00000100,
	MASK_PB 		= 0x00000200,
	MASK_REC 		= 0x00000400,
	MASK_SH1 		= 0x00000800,

	MASK_SH2 		= 0x00001000,
	MASK_JOG 		= 0x00002000,
	MASK_WIFI 		= 0x00004000,
	MASK_TOUCH 	= 0x00008000,

	MASK_JOGPUSH 	= 0x00010000,

	MASK_ALL 		= 0x0001FFFF,
};

struct mask_info_key{
	int state;
	int mask;
	enum KEYMASK_ID mask_id;
};

struct adc_info_key{
	int code;
	int adc_ch;
	int adc_level;
	struct mask_info_key mask_key;
};

struct gpio_info_key{
	int code;
	struct mask_info_key mask_key;
};

struct key_info_cnt{
	int adckey_cnt;
	int gpiokey_cnt;
};

#endif
