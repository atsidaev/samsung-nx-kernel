#ifndef __SHUTTER_IOCTL_H__
#define __SHUTTER_IOCTL_H__

typedef struct _t_shutter_timer_param {
	int 				i_channel;
	unsigned long		ul_time;
}t_shutter_timer_param;

typedef struct _t_shutter_gpio_param {
	unsigned int		ui_gpio_num;
	int					i_value;
}t_shutter_gpio_param;

typedef struct _t_shutter_pwm_param {
	int 				i_channel;
	unsigned long		ul_time;
}t_shutter_pwm_param;


#define SHUTTER_MAGIC	's'


#define SHUTTER_IOCTL_GPIO_SET				_IOW(SHUTTER_MAGIC, 0, t_shutter_gpio_param)
#define SHUTTER_IOCTL_GPIO_GET				_IOWR(SHUTTER_MAGIC, 1, t_shutter_gpio_param)
//#define SHUTTER_IOCTL_GPIO_SET_DIR			_IOW(SHUTTER_MAGIC, 2, t_shutter_timer_param)
#define SHUTTER_IOCTL_INTERRUPT_SET_ACT		_IOW(SHUTTER_MAGIC, 3, t_shutter_gpio_param)
#define SHUTTER_IOCTL_INTERRUPT_WAIT		_IOW(SHUTTER_MAGIC, 4, t_shutter_gpio_param)
#define SHUTTER_IOCTL_INTERRUPT_SET_LEVEL	_IOW(SHUTTER_MAGIC, 5, t_shutter_gpio_param)
#define SHUTTER_IOCTL_PWM_START				_IOW(SHUTTER_MAGIC, 6, t_shutter_pwm_param)
#define SHUTTER_IOCTL_PWM_WAIT				_IOW(SHUTTER_MAGIC, 7, t_shutter_pwm_param)
#define SHUTTER_IOCTL_PWM_STOP				_IOW(SHUTTER_MAGIC, 8, t_shutter_pwm_param)
#define SHUTTER_IOCTL_GPIO_TO_PWM			_IOW(SHUTTER_MAGIC, 9, t_shutter_gpio_param)
#define SHUTTER_IOCTL_PWM_TO_GPIO			_IOW(SHUTTER_MAGIC, 10, t_shutter_gpio_param)
#define SHUTTER_IOCTL_TIMER_START			_IOW(SHUTTER_MAGIC, 11, t_shutter_timer_param)
#define SHUTTER_IOCTL_TIMER_WAIT			_IOW(SHUTTER_MAGIC, 12, t_shutter_timer_param)
#define SHUTTER_IOCTL_TIMER_STOP			_IOW(SHUTTER_MAGIC, 13, t_shutter_timer_param)
#define SHUTTER_IOCTL_INITIALIZE			_IO(SHUTTER_MAGIC, 14)

#define		SHUTTER_INT_RISING_EDGE				(1)
#define		SHUTTER_INT_FALLING_EDGE 			(0)
#define		SHUTTER_INT_NONE_EDGE 				(-1)

#define		SHUTTER_PIO_POWER 	 	(0)						
#define		SHUTTER_PIO_MG1			(1)					
#define		SHUTTER_PIO_MG2			(2)				
#define		SHUTTER_PIO_MOT1		(3)					
#define		SHUTTER_PIO_MOT2		(4)					
#define		SHUTTER_PIO_ACT1		(5)					
#define		SHUTTER_PIO_ACT2		(6)					
#define		SHUTTER_PIO_PI1			(7)
	//SHUTTER_PIO_PI2,		









#endif
