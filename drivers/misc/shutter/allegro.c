#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/pinctrl/consumer.h>

#include <linux/sched.h>
#include <linux/slab.h>

#include <linux/wait.h>
#include <linux/allegro_ioctl.h>
#include <asm/uaccess.h>
#include <linux/module.h>

#include <mach/gpio.h>
#include <linux/device.h>
#include <linux/time.h>
#include <mach/version_information.h>

#define assert(s) do{if (!(s)) panic(#s);} while(0);
#define ALLEGRO_DIRECTION_INPUT 			0
#define ALLEGRO_DIRECTION_OUTPUT 			1
#define MS_TO_NS(TIME)	 ((TIME) * 1000000)
#define US_TO_NS(TIME)	 ((TIME) * 1000)

#define PI_INT_IGNORE_TIME (2000) //2ms

typedef struct t_allegro_irq_cb 
{
	struct list_head		list;
	int						i_waiting_signal;
	int						i_count;
	int						irq_descriptor;
	unsigned 				ui_pi_io_num;
	unsigned 				ui_brk_io_num1;
	unsigned 				ui_brk_io_num2;
	unsigned 				ui_mg_io_num1;
	unsigned 				ui_mg_io_num2;
	int						i_timer_channel;
	unsigned long			ul_timer_time;
	int						i_job_after_timer;
	wait_queue_head_t*		pt_waiting_process;
}allegro_irq_cb;

typedef struct _allegor_timer_control_block
{
	struct list_head		list;
	struct hrtimer 			timer;
	int						channel;
	unsigned 				ui_brk_io_num1;
	unsigned 				ui_brk_io_num2;
	unsigned 				ui_mg_io_num1;
	unsigned 				ui_mg_io_num2;
	int						i_job_after_timer;
	wait_queue_head_t*		pt_waiting_process;
}allegro_timer_cb;


static LIST_HEAD(allegro_irq_list);
static LIST_HEAD(allegro_timer_list);

static struct miscdevice allegro_miscdev;
static unsigned 		 g_ui_allegro_pi_gpio_num = 0;
static int				 g_i_timer_max				= 5;
static u64	 			 g_ull_prev_isr_time_us = 0;
static int allegro_timer_start(int i_channel, unsigned long ul_time, int i_job_after_timer);
static int allegro_timer_stop(int i_channel);

static int allegro_pad_set(unsigned gpio_num, char* pch_name, int i_direction)
{
	int err = 0;
	pinctrl_free_gpio(gpio_num);
	err = pinctrl_request_gpio(gpio_num);
	gpio_free(gpio_num);
	err = gpio_request(gpio_num, pch_name);
	if (err < 0)
	{
		dev_err(allegro_miscdev.this_device, "PI request Error: %d %s\n", err, pch_name);
		gpio_free(gpio_num);
	}
	else
	{
		if(i_direction == ALLEGRO_DIRECTION_INPUT)
		{
			gpio_direction_input(gpio_num);
		}
		else if(i_direction == ALLEGRO_DIRECTION_OUTPUT)
		{
			gpio_direction_output(gpio_num, 0);
		}
		else
		{
			err = -1;
		}
	}
	return err;
}

static bool allegro_gpio_set(unsigned gpio_num, int iValue)
{
	gpio_set_value(gpio_num, iValue);
	return true;
}

static int allegro_gpio_get(unsigned gpio_num)
{
	int iRetVal = 0;
	iRetVal = gpio_get_value(gpio_num);
	return iRetVal;
}
////////////////////////////////////////////////////////
//Interrupt Operation
////////////////////////////////////////////////////////
static allegro_irq_cb* find_irq_cb(unsigned gpio_num)
{
	allegro_irq_cb*			p_irq_cb = NULL;
	struct list_head*		list;
	list_for_each(list, &allegro_irq_list)
	{
		p_irq_cb	= list_entry(list, allegro_irq_cb, list);
		if (p_irq_cb->ui_pi_io_num == gpio_num)
		{
			break;
		}
	}
	return p_irq_cb;
}

static int allegro_interrupt_wait(unsigned gpio_num)
{
	allegro_irq_cb*			p_irq_cb = NULL;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		return -1;
	}
	interruptible_sleep_on(p_irq_cb->pt_waiting_process);
	return 0;
}

static int allegro_interrupt_enable(unsigned gpio_num)
{
	allegro_irq_cb*			p_irq_cb = NULL;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		return -1;
	}	
	if(p_irq_cb->i_waiting_signal == ALLEGRO_INT_NONE_EDGE)
	{
		dev_err(allegro_miscdev.this_device, "Not defined Edge\n");
		return -1;
	}
	enable_irq(p_irq_cb->irq_descriptor);
	return 0;
}
static int allegro_interrupt_disable(unsigned gpio_num)
{
	allegro_irq_cb* 		p_irq_cb = NULL;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		return -1;
	}
	p_irq_cb->i_waiting_signal = ALLEGRO_INT_NONE_EDGE;
	irq_set_irq_type(p_irq_cb->irq_descriptor, IRQ_TYPE_NONE);
	//disable_irq_nosync(p_irq_cb->irq_descriptor);

	if(p_irq_cb->ul_timer_time)
	{
		p_irq_cb ->ul_timer_time = 0;
		if(allegro_timer_stop(2) < 0) //3th timer
		{
			assert(0);
			return -1;
		}
	}
	return 0;
}
static int allegro_interrupt_set_level(unsigned gpio_num, int i_level)
{
	allegro_irq_cb* 		p_irq_cb = NULL;	
	unsigned int			irq_level = IRQ_TYPE_NONE;
	int						err = 0;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		err = -1;
	}
	else
	{		
		switch(i_level)
		{
			case ALLEGRO_INT_FALLING_EDGE:
				irq_level = IRQ_TYPE_EDGE_FALLING;
				break;
			case ALLEGRO_INT_RISING_EDGE:
				irq_level = IRQ_TYPE_EDGE_RISING;
				break;
			default:
				err = -2;
				break;
		}
		if(err < 0)
		{
		}
		else
		{
			p_irq_cb->i_waiting_signal = i_level;	
			err = irq_set_irq_type(p_irq_cb->irq_descriptor, irq_level);		
		}
	}
	return err;
}
static int allegro_interrupt_set(unsigned gpio_num, int i_level, int i_count, unsigned long ul_time, int i_job_after_timer)
{
	allegro_irq_cb* 		p_irq_cb = NULL;	
	int						err = 0;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		err = -1;
	}
	else
	{
		if(allegro_interrupt_set_level(p_irq_cb->ui_pi_io_num, i_level) < 0)
		{
			err = -2;
		}
		p_irq_cb->i_count = i_count;
		p_irq_cb->i_job_after_timer = i_job_after_timer;
		p_irq_cb->i_timer_channel = 2; //3th timer
		p_irq_cb->ul_timer_time = ul_time;
	}
	return err;
}
static irqreturn_t allegro_isr(int irq, void *handle)
{
	allegro_irq_cb* p_irq_cb = (allegro_irq_cb*)handle;
	int 			i_current_pi_signal = 0;
	int 			i = 0;
	struct timeval 	st_curr_time;
	u64				u64_curr_time = 0;
	if(p_irq_cb == NULL)
	{
		printk("Allegro ISR No CB Error\n");
		assert(0);
		return IRQ_NONE;
	}
	else
	{
		i_current_pi_signal = allegro_gpio_get(p_irq_cb->ui_pi_io_num);
		//Ripple check
		if(p_irq_cb->i_waiting_signal != i_current_pi_signal)
		{//unexpected situation. have a little delay time.
			for(i = 0; i < 40; i++)
			{
				i_current_pi_signal = allegro_gpio_get(p_irq_cb->ui_pi_io_num);	
				if(p_irq_cb->i_waiting_signal == i_current_pi_signal)
				{
					break;
				}
			}
			if(p_irq_cb->i_waiting_signal != i_current_pi_signal)
			{
				return IRQ_NONE;
			}
		}
		
		//If interrupt happen again between 2ms, We ignore that interrupt.
		do_gettimeofday(&st_curr_time);
		u64_curr_time = (u64)(st_curr_time.tv_sec * 1000000) + (u64)(st_curr_time.tv_usec);	
		//printk("%llu %llu\n", u64_curr_time, g_ull_prev_isr_time_us);	
		if ((u64_curr_time - g_ull_prev_isr_time_us ) >= (u64)(PI_INT_IGNORE_TIME))
		{								
			g_ull_prev_isr_time_us = u64_curr_time;
		}
		else
		{
			return IRQ_NONE;
		}

		//real ISR routine
		if(p_irq_cb->i_count > 0)
		{
			p_irq_cb->i_count--;
			if(p_irq_cb->i_count == 0)
			{
				//Turn off Allegro Interrupt directly.
				disable_irq_nosync(irq);
				
				//Act relate to ISR
				if(p_irq_cb->ul_timer_time > 0)
				{
					if(allegro_timer_start(p_irq_cb->i_timer_channel, p_irq_cb->ul_timer_time, p_irq_cb->i_job_after_timer) < 0)
					{
						printk("Allegro ISR Timer Error\n");
						assert(0);
						return IRQ_NONE;
					}
				}
				else
				{
					//Break Motor
					if(p_irq_cb->i_job_after_timer == ALLEGRO_JOB_BRK_ON)
					{
						allegro_gpio_set(p_irq_cb->ui_brk_io_num1, 1);
						allegro_gpio_set(p_irq_cb->ui_brk_io_num2, 1);
					}
					else if(p_irq_cb->i_job_after_timer == ALLEGRO_JOB_MG1_ON)
					{
						allegro_gpio_set(p_irq_cb->ui_mg_io_num1, 1);
					}
					else if(p_irq_cb->i_job_after_timer == ALLEGRO_JOB_MG2_ON)
					{
						allegro_gpio_set(p_irq_cb->ui_mg_io_num2, 1);
					}
					else
					{
					}
					wake_up_interruptible(p_irq_cb->pt_waiting_process);
				}
			}
			else
			{				
				if(p_irq_cb->i_waiting_signal == ALLEGRO_INT_RISING_EDGE)
				{
					p_irq_cb->i_waiting_signal = ALLEGRO_INT_FALLING_EDGE;
				}
				else
				{
					p_irq_cb->i_waiting_signal = ALLEGRO_INT_RISING_EDGE;
				}
				allegro_interrupt_set_level(p_irq_cb->ui_pi_io_num, p_irq_cb->i_waiting_signal);
			}		
		}
		else
		{
			printk("Allegro ISR Error\n");
			assert(0);
			return IRQ_NONE;
		}		
	}
	return IRQ_HANDLED;
}
/////////////////////////////////////
//Timer Operation
/////////////////////////////////////
static allegro_timer_cb* find_timer_cb(int i_channel)
{
	allegro_timer_cb*			p_timer_cb = NULL;
	struct list_head*			list;
	list_for_each(list, &allegro_timer_list)
	{
		p_timer_cb	= list_entry(list, allegro_timer_cb, list);
		if (p_timer_cb->channel == i_channel)
		{
			break;
		}
	}
	return p_timer_cb;
}

static enum hrtimer_restart allegro_timer_handler(struct hrtimer *timer_param)
{
	allegro_timer_cb* tdata = container_of(timer_param, allegro_timer_cb, timer);
	if(tdata->i_job_after_timer == ALLEGRO_JOB_BRK_ON)
	{
		allegro_gpio_set(tdata->ui_brk_io_num1, 1);
		allegro_gpio_set(tdata->ui_brk_io_num2, 1);
	}
	else if(tdata->i_job_after_timer == ALLEGRO_JOB_MG1_ON)
	{
		allegro_gpio_set(tdata->ui_mg_io_num1, 1);
	}
	else if(tdata->i_job_after_timer == ALLEGRO_JOB_MG2_ON)
	{
		allegro_gpio_set(tdata->ui_mg_io_num2, 1);
	}
	else
	{
	}
	wake_up_interruptible(tdata->pt_waiting_process);
	return HRTIMER_NORESTART;
}
static int allegro_timer_wait(int i_channel)
{
	int ret = 0;
	allegro_timer_cb* p_timer_cb = NULL;
	p_timer_cb = find_timer_cb(i_channel);
	if(!p_timer_cb)
	{	
		ret = -1;
	}
	else
	{
		interruptible_sleep_on(p_timer_cb->pt_waiting_process);
	}
	return ret;
}

static int allegro_timer_start(int i_channel, unsigned long ul_time, int i_job_after_timer)
{	
	int ret = 0;
	allegro_timer_cb* 	p_timer_cb = NULL;
	p_timer_cb = find_timer_cb(i_channel);
	if(p_timer_cb)
	{		
		p_timer_cb->i_job_after_timer = i_job_after_timer;
		if(hrtimer_start(&p_timer_cb->timer, ktime_set(0, US_TO_NS(ul_time)), HRTIMER_MODE_REL) < 0)
		{
			ret = -1;
		}
	}
	else
	{
		ret = -2;
	}
	return ret;
}
static int allegro_timer_stop(int i_channel)
{
	int ret = 0;
	allegro_timer_cb* p_timer_cb = NULL;
	p_timer_cb = find_timer_cb(i_channel);
	if(p_timer_cb)
	{		
		hrtimer_cancel(&(p_timer_cb->timer));
		p_timer_cb->i_job_after_timer = ALLEGRO_JOB_BRK_OFF;
	}
	else
	{
		ret = -1;
	}
	return ret;
}



int allegro_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int allegro_release(struct inode *inode, struct file *filp)
{
	return 0;
}

long allegro_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg)
{
	int  	size = 0;
	int  	err = 0;
	t_allegro_param				t_command 			= {0, };
	t_allegro_timer_param		t_timer_command 	= {0, };

	if (_IOC_TYPE(cmd) != ALLEGRO_MAGIC)
	{
		return -EINVAL;
	}

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
	{
		err = access_ok(VERIFY_WRITE, (void *) arg, size);
	}
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = access_ok(VERIFY_READ, (void *) arg, size);
	}
	else
	{
		err = 1;
	}

	if (!err)
	{
		return -err;
	}
	switch (cmd) 
	{
		case ALLEGRO_IOCTL_GPIO_GET:
			t_command.i_level = allegro_gpio_get(g_ui_allegro_pi_gpio_num);
			if (copy_to_user((void *)arg, (const void *)&t_command, size))
			{
				err = -ENOMEM;
			}			
			break;
		case ALLEGRO_IOCTL_INTERRUPT_SET:
			if (copy_from_user((void *)&t_command, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(allegro_interrupt_set(g_ui_allegro_pi_gpio_num, t_command.i_level, t_command.i_count, t_command.ul_timer_time, t_command.i_job_after_timer) < 0)
				{
					err = -EFAULT;
				}
			}
			break;
		case ALLEGRO_IOCTL_INTERRUPT_ENABLE:
			if (copy_from_user((void *)&t_command, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(t_command.i_level)
				{//turn on pi interrupt
					if(allegro_interrupt_enable(g_ui_allegro_pi_gpio_num) < 0)
					{
						err = -EFAULT;
					}
				}
				else
				{//turn off pi interrupt
					if(allegro_interrupt_disable(g_ui_allegro_pi_gpio_num) < 0)
					{
						err = -EFAULT;
					}
				}
			}
			break;
		case ALLEGRO_IOCTL_INTERRUPT_WAIT:		
			if(allegro_interrupt_wait(g_ui_allegro_pi_gpio_num) < 0)
			{
				err = -EFAULT;
			}			
			break;


		case ALLEGRO_IOCTL_TIMER_START:
			if (copy_from_user((void *)&t_timer_command, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(allegro_timer_start(t_timer_command.i_channel, t_timer_command.ul_time, t_timer_command.i_job_after_timer) < 0)
				{
					err = -EFAULT;
				}
			}
			break;
		case ALLEGRO_IOCTL_TIMER_STOP:
			if (copy_from_user((void *)&t_timer_command, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(allegro_timer_stop(t_timer_command.i_channel) < 0)
				{
					err = -EFAULT;
				}
			}
			break;
		case ALLEGRO_IOCTL_TIMER_WAIT:
			if (copy_from_user((void *)&t_timer_command, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(allegro_timer_wait(t_timer_command.i_channel) < 0)
				{
					err = -EFAULT;
				}
			}
			break;
			
		case ALLEGRO_IOCTL_INITIALIZE:	
			if(allegro_pad_set(g_ui_allegro_pi_gpio_num, "sh_pi2", ALLEGRO_DIRECTION_INPUT) < 0)
			{
				err = -EFAULT;
			}	
			break;		
		
		default:
			break;
	}
	return err;
}

const struct file_operations allegro_fops = 
{
	.owner			= THIS_MODULE,
	.open			= allegro_open,
	.release		= allegro_release,
	.unlocked_ioctl = allegro_ioctl,
};


static struct miscdevice allegro_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "allegro",
	.fops = &allegro_fops,
};

static int __init allegro_init(void)
{
	int ret, i;
	allegro_irq_cb*			p_irq_cb;
	allegro_timer_cb*		p_timer_cb;
	struct list_head*		list;	
	ret = 0;
	i = 0;
	p_irq_cb = NULL;
	p_timer_cb = NULL;
	list = NULL;

	//allocate irq control block struct
	p_irq_cb = kzalloc(sizeof(allegro_irq_cb), GFP_KERNEL);
	if (!p_irq_cb)
	{
		ret		= -ENOMEM;
		goto err_irq_alloc;			
	}
	else
	{
		g_ui_allegro_pi_gpio_num = SH_PI2_OUT;
		p_irq_cb->ui_pi_io_num = g_ui_allegro_pi_gpio_num; 
		p_irq_cb->ui_brk_io_num1 = GPIO_SH_MOT_IN1; 
		p_irq_cb->ui_brk_io_num2 = GPIO_SH_MOT_IN2;
		p_irq_cb->ui_mg_io_num1	 = GPIO_SHUTTER_MG1_ON;
		p_irq_cb->ui_mg_io_num2	 = GPIO_SHUTTER_MG2_ON;
		p_irq_cb->i_timer_channel = g_i_timer_max - 1; //last timer
		p_irq_cb->ul_timer_time = 0;
		p_irq_cb->irq_descriptor = gpio_to_irq(p_irq_cb->ui_pi_io_num);
		ret = request_irq(p_irq_cb->irq_descriptor, allegro_isr, IRQF_DISABLED, "ALLEGRO IRQ", (void*)p_irq_cb);
		if (ret)
		{
			dev_err(allegro_miscdev.this_device, "request IRQ failed %d\n", ret);
			ret = -EFAULT;
			goto err_irq_alloc;
		}
		disable_irq_nosync(p_irq_cb->irq_descriptor);
	
		p_irq_cb->pt_waiting_process = kzalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
		if (!p_irq_cb->pt_waiting_process) 
		{				
			ret		= -ENOMEM;
			goto err_irq_alloc;
		}
		else
		{			
			init_waitqueue_head(p_irq_cb->pt_waiting_process);
			list_add_tail(&p_irq_cb->list, &allegro_irq_list);
		}	
	}

	//allocate timer control block struct
	for(i = 0; i < g_i_timer_max; i++)
	{
		p_timer_cb = kzalloc(sizeof(allegro_timer_cb), GFP_KERNEL);
		if (!p_timer_cb)
		{
			ret		= -ENOMEM;
			goto err_timer_alloc;			
		}
		else
		{
			hrtimer_init(&p_timer_cb->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
			p_timer_cb->channel 				= i;
			p_timer_cb->ui_brk_io_num1 			= GPIO_SH_MOT_IN1; 
			p_timer_cb->ui_brk_io_num2 			= GPIO_SH_MOT_IN2;
			p_timer_cb->ui_mg_io_num1			= GPIO_SHUTTER_MG1_ON;
			p_timer_cb->ui_mg_io_num2			= GPIO_SHUTTER_MG2_ON;
			p_timer_cb->timer.function 			= allegro_timer_handler;
			p_timer_cb->pt_waiting_process 		= kzalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
			if (!p_timer_cb->pt_waiting_process) 
			{				
				ret		= -ENOMEM;
				goto err_timer_alloc;
			}
			else
			{			
				init_waitqueue_head(p_timer_cb->pt_waiting_process);
				list_add_tail(&p_timer_cb->list, &allegro_timer_list);
			}	
		}
	}	


	
	if(ret < 0)
	{
		//error case
	}
	else
	{
		ret = misc_register(&allegro_miscdev);
		if (ret < 0) {
			dev_err(allegro_miscdev.this_device, "Failed to register misc driver.\n");
		}
		else
		{	
			dev_info(allegro_miscdev.this_device, "device registered as /dev/%s\n", allegro_miscdev.name);
		}
	}

	return ret;
err_timer_alloc:
	while(!list_empty(&allegro_timer_list)) 
	{
		p_timer_cb = list_entry(list, allegro_timer_cb, list);
		if(p_timer_cb)
		{
			if(p_timer_cb->pt_waiting_process)
			{
				kfree(p_timer_cb->pt_waiting_process);
			}
			list_del(&p_timer_cb->list);
			kfree(p_timer_cb);
		}
	}
err_irq_alloc:
	while(!list_empty(&allegro_irq_list)) 
	{
		p_irq_cb	= list_entry(list, allegro_irq_cb, list);
		if(p_irq_cb)
		{
			if(p_irq_cb->irq_descriptor)
			{
				free_irq(p_irq_cb->irq_descriptor, p_irq_cb);
			}
			if(p_irq_cb->pt_waiting_process)
			{
				kfree(p_irq_cb->pt_waiting_process);
			}
			list_del(&p_irq_cb->list);
			kfree(p_irq_cb);
		}
	}
	return ret;
}

static void __exit allegro_exit(void)
{
	struct list_head*		list = NULL;
	allegro_irq_cb*			p_irq_cb = NULL;
	allegro_timer_cb*		p_timer_cb = NULL;
	while(!list_empty(&allegro_timer_list)) 
	{
		p_timer_cb = list_entry(list, allegro_timer_cb, list);
		if(p_timer_cb)
		{
			if(p_timer_cb->pt_waiting_process)
			{
				kfree(p_timer_cb->pt_waiting_process);
			}
			list_del(&p_timer_cb->list);
			kfree(p_timer_cb);
		}
	}
	
	while(!list_empty(&allegro_irq_list)) 
	{
		p_irq_cb	= list_entry(list, allegro_irq_cb, list);
		if(p_irq_cb)
		{			
			if(p_irq_cb->irq_descriptor)
			{
				free_irq(p_irq_cb->irq_descriptor, p_irq_cb);
			}
			if(p_irq_cb->pt_waiting_process)
			{
				kfree(p_irq_cb->pt_waiting_process);
			}
			list_del(&p_irq_cb->list);
			kfree(p_irq_cb);
		}
	}
	misc_deregister(&allegro_miscdev);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(allegro_init);
#else
fast_dev_initcall(allegro_init);
#endif

module_exit(allegro_exit);

MODULE_AUTHOR("choi yong jin");
MODULE_DESCRIPTION("allegro driver");
MODULE_LICENSE("GPL");

