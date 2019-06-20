#ifndef _D4KEYS_H_
#define _D4KEYS_H_

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

	MASK_POWER	= 0x00010000,
	MASK_JOGPUSH 	= 0x00020000,
	MASK_MODE	 	= 0x00040000,

	
	MASK_ALL 		= 0x000FFFFF,
};

struct adc_info_key{
	int code;
	int adc_level;
	enum KEYMASK_ID mask_id;
};

enum KEYTYPE{
	PORT1_TYPE = 0,
	PORT2_TYPE,
	ADCKEY_TYPE,
	ADCMODE_TYPE,
};

struct d4_keys_button {
	/* Configuration parameters */
	enum KEYTYPE keytype;
	int code;		/* input event code (KEY_*, SW_*) */
	int code2nd;	/* one gpio needs 2nd code */
	int gpio;		/* set interrupt input */
	int gpio2nd;	/* no set interrupt input */
	int active_low;
	char *desc;
	int type;		/* input event type (EV_KEY, EV_SW) */
	int wakeup;		/* configure the button as a wake-up source */
	int debounce_interval;	/* debounce ticks interval in msecs */
	bool can_disable;
	enum KEYMASK_ID mask_id;
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
	char *desc;
	int type;		/* input event type (EV_KEY, EV_SW) */
	int debounce_interval;	/* debounce ticks interval in msecs */
	bool adc_input; /*adc type key ? true : false*/

	enum KEYTYPE keytype;
	unsigned int ref_mvolt;
	unsigned int time_margin;
	unsigned int data_diff;
	int adc_useint;
	int adc_ch;
	int adc_type; /*key[0], mode[1]*/
	struct adc_info_key *key_info;
	int key_cnt;
	
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


#endif
