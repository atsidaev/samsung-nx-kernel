#ifndef __ALLEGRO_IOCTL_H__
#define __ALLEGRO_IOCTL_H__


typedef struct _t_allegro_param {
	int					i_level;
	int					i_count;
	unsigned long		ul_timer_time;
	int					i_job_after_timer;
}t_allegro_param;

typedef struct _t_allegro_timer_param {
	int					i_channel;
	int					ul_time;
	int					i_job_after_timer;
}t_allegro_timer_param;


#define ALLEGRO_MAGIC	's'


#define ALLEGRO_IOCTL_GPIO_GET				_IOR(ALLEGRO_MAGIC, 1, t_allegro_param)
#define ALLEGRO_IOCTL_INTERRUPT_SET			_IOW(ALLEGRO_MAGIC, 3, t_allegro_param)
#define ALLEGRO_IOCTL_INTERRUPT_ENABLE		_IOW(ALLEGRO_MAGIC, 4, t_allegro_param)
#define ALLEGRO_IOCTL_INTERRUPT_WAIT		_IO(ALLEGRO_MAGIC, 5)
#define ALLEGRO_IOCTL_TIMER_START			_IOW(ALLEGRO_MAGIC, 6, t_allegro_timer_param)
#define ALLEGRO_IOCTL_TIMER_STOP			_IOW(ALLEGRO_MAGIC, 7, t_allegro_timer_param)
#define ALLEGRO_IOCTL_TIMER_WAIT			_IOW(ALLEGRO_MAGIC, 8, t_allegro_timer_param)
#define ALLEGRO_IOCTL_INITIALIZE			_IO(ALLEGRO_MAGIC, 14)

#define		ALLEGRO_INT_RISING_EDGE				(1)
#define		ALLEGRO_INT_FALLING_EDGE 			(0)
#define		ALLEGRO_INT_NONE_EDGE 				(-1)

#define ALLEGRO_JOB_BRK_OFF						(0)
#define ALLEGRO_JOB_BRK_ON						(1)
#define ALLEGRO_JOB_MG1_ON						(2)
#define ALLEGRO_JOB_MG2_ON						(3)
#define ALLEGRO_JOB_MG1_OFF						(4)
#define ALLEGRO_JOB_MG2_OFF						(5)




			

#endif
