#ifndef __COPAL_IOCTL_H__
#define __COPAL_IOCTL_H__


typedef struct _t_copal_param {
	int					i_level;
	bool				bl_is_break_on;
}t_copal_param;



#define COPAL_MAGIC	'm'


#define COPAL_IOCTL_GPIO_GET				_IOR(COPAL_MAGIC, 1, t_copal_param)
#define COPAL_IOCTL_INTERRUPT_SET			_IOW(COPAL_MAGIC, 3, t_copal_param)
#define COPAL_IOCTL_INTERRUPT_ENABLE		_IOW(COPAL_MAGIC, 4, t_copal_param)
#define COPAL_IOCTL_INTERRUPT_WAIT			_IO(COPAL_MAGIC, 5)
#define COPAL_IOCTL_INITIALIZE				_IO(COPAL_MAGIC, 14)
	
#define		COPAL_INT_RISING_EDGE				(1)
#define		COPAL_INT_FALLING_EDGE 			(0)
#define		COPAL_INT_NONE_EDGE 				(-1)
			

#endif
