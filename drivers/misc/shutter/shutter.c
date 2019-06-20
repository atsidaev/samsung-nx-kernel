#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/err.h>
//#include <linux/sched.h>
//#include <linux/uaccess.h>
//#include <linux/proc_fs.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/d4_ht_pwm.h>
#include <linux/shutter.h>
#include <linux/shutter_ioctl.h>
#include <asm/uaccess.h>

#include <mach/gpio.h>

#define assert(s) do{if (!(s)) panic(#s);} while(0);
#define SHUTTER_DIRECTION_INPUT 			0
#define SHUTTER_DIRECTION_OUTPUT 			1

typedef struct t_irq_param {
	struct list_head		list;
	int						i_waiting_signal;
	int						irq_descriptor;
	unsigned 				ui_gpio_num;
	wait_queue_head_t*		pt_waiting_process;
}shutter_irq_cb;

typedef struct _shutter_pwm_control_block {
	struct list_head			list;
	struct ht_pwm_device*		pwm_dev;
	shutter_pwm_info*			pwm_info;
	wait_queue_head_t*			pt_waiting_process;
}shutter_pwm_cb;

typedef struct _shutter_timer_control_block {
	struct list_head		list;
	struct hrtimer 			timer;
	int						channel;
	bool					bl_is_waiting;
	wait_queue_head_t*		pt_waiting_process;
}shutter_timer_cb;


static LIST_HEAD(shutter_pwm_list);
static LIST_HEAD(shutter_timer_list);
static LIST_HEAD(shutter_irq_list);

static unsigned* g_shutter_gpio = NULL;

static struct miscdevice shutter_miscdev;
static irqreturn_t gpio_isr(int irq, void *handle);
static void pwm_isr(int pwm_id);
static enum hrtimer_restart shutter_timer_handler(struct hrtimer *timer_param);
static shutter_pwm_cb* find_pwm_cb(unsigned gpio_num);


static int shutter_pad_set(unsigned gpio_num, char* pch_name, int i_direction)
{
	int 					err = 0;
	err = pinmux_request_gpio(gpio_num);
	err = gpio_request(gpio_num, pch_name);
	if (err < 0)
	{
		printk("gpio_request: %d %s\n", gpio_num, pch_name);
		gpio_free(gpio_num);
	}
	else
	{
		if(i_direction == SHUTTER_DIRECTION_INPUT)
		{
			gpio_direction_input(gpio_num);
		}
		else if(i_direction == SHUTTER_DIRECTION_OUTPUT)
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

static bool shutter_gpio_set(unsigned gpio_num, int iValue)
{
	gpio_set_value(gpio_num, iValue);
	return true;
}
static int shutter_gpio_get(unsigned gpio_num)
{
	int iRetVal = 0;
	iRetVal = gpio_get_value(gpio_num);
	return iRetVal;
}
static int shutter_gpio_to_pwm(int gpio_num)
{	
	int 				ret 		= 0;
	shutter_pwm_cb* 	p_pwm_cb 	= NULL;
	struct pinmux*		pinmux		= NULL;
	pinmux_free_gpio(gpio_num);
	p_pwm_cb = find_pwm_cb(gpio_num);
	if(p_pwm_cb)
	{
		pinmux = pinmux_get(NULL, p_pwm_cb->pwm_info->pch_pcon_func_name);
		if (!IS_ERR(pinmux))
		{
			pinmux_enable(pinmux);
		}
	}
	else
	{
		ret = -1;
	}
	
	return ret;
}
static int shutter_pwm_to_gpio(int gpio_num)
{
	pinmux_request_gpio(gpio_num);
	return 0;
}
static shutter_irq_cb* find_irq_cb(unsigned gpio_num)
{
	shutter_irq_cb*			p_irq_cb = NULL;
	struct list_head*		list;
	list_for_each(list, &shutter_irq_list)
	{
		p_irq_cb	= list_entry(list, shutter_irq_cb, list);
		if (p_irq_cb->ui_gpio_num == gpio_num)
		{
			break;
		}
	}
	return p_irq_cb;
}
static shutter_pwm_cb* find_pwm_cb(unsigned gpio_num)
{
	shutter_pwm_cb* 	p_pwm_cb = NULL;
	struct list_head	*list = NULL;
	list_for_each(list, &shutter_pwm_list)
	{
		p_pwm_cb	= list_entry(list, shutter_pwm_cb, list);
		if (p_pwm_cb->pwm_info->shared_gpio_num == gpio_num)
		{
			break;
		}
	}
	return p_pwm_cb;
}

static int shutter_interrupt_wait(unsigned gpio_num)
{
	shutter_irq_cb*			p_irq_cb = NULL;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		return -1;
	}
	interruptible_sleep_on(p_irq_cb->pt_waiting_process);
	return 0;
}

static int shutter_interrupt_enable(unsigned gpio_num)
{
	shutter_irq_cb*			p_irq_cb = NULL;
	int 					err = 0;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		return -1;
	}
	p_irq_cb->irq_descriptor = gpio_to_irq(p_irq_cb->ui_gpio_num);
	if(p_irq_cb->i_waiting_signal == SHUTTER_INT_NONE_EDGE)
	{
		dev_err(shutter_miscdev.this_device, "Not defined Edge\n");
		return -1;
	}
	err = request_irq(p_irq_cb->irq_descriptor, gpio_isr, IRQF_DISABLED, "SHUTTER IRQ", (void*)p_irq_cb);
	if (err) {
		dev_err(shutter_miscdev.this_device, "request IRQ failed %d\n", err);
		return -1;
	}
	enable_irq(p_irq_cb->irq_descriptor);
	return 0;
}
static int shutter_interrupt_disable(unsigned gpio_num)
{
	shutter_irq_cb* 		p_irq_cb = NULL;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		return -1;
	}
	p_irq_cb->i_waiting_signal = SHUTTER_INT_NONE_EDGE;
	irq_set_irq_type(p_irq_cb->irq_descriptor, IRQ_TYPE_NONE);
	disable_irq_nosync(p_irq_cb->irq_descriptor);
	free_irq(p_irq_cb->irq_descriptor, p_irq_cb);
	wake_up_interruptible(p_irq_cb->pt_waiting_process);
	return 0;
}
static int shutter_interrupt_set_level(unsigned gpio_num, int i_level)
{
	shutter_irq_cb* 		p_irq_cb = NULL;	
	unsigned int			irq_level = IRQ_TYPE_NONE;
	int						err = 0;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		err = -1;
	}
	else
	{
		p_irq_cb->i_waiting_signal = i_level;			
		switch(p_irq_cb->i_waiting_signal)
		{
			case SHUTTER_INT_FALLING_EDGE:
				irq_level = IRQ_TYPE_EDGE_FALLING;
				break;
			case SHUTTER_INT_RISING_EDGE:
				irq_level = IRQ_TYPE_EDGE_RISING;
				break;
			default:
				break;
		}
		err = irq_set_irq_type(gpio_to_irq(gpio_num), irq_level);		
	}
	return err;
}

static int shutter_pwm_start(int i_channel, unsigned long ul_time )
{
	char 					name[10] = {0, };
	shutter_pwm_cb* 		p_pwm_cb = NULL;
	struct list_head	*list;
	list_for_each(list, &shutter_pwm_list)
	{
		p_pwm_cb	= list_entry(list, shutter_pwm_cb, list);
		if (p_pwm_cb->pwm_info->id == i_channel)
			break;
	}
	if(!p_pwm_cb)
	{
		return -1;
	}
	sprintf(name, "pwm%d", p_pwm_cb->pwm_info->id);
	p_pwm_cb->pwm_dev = d4_ht_pwm_request(p_pwm_cb->pwm_info->id, name);
	if(p_pwm_cb->pwm_dev == ERR_PTR(-EBUSY) || p_pwm_cb->pwm_dev == ERR_PTR(-ENOENT))
	{
		printk("failed to request pwm (%s%d) !!!\n", "pwm", p_pwm_cb->pwm_info->id);
		return -1;
	}

	d4_ht_pwm_int_func_set(p_pwm_cb->pwm_dev, pwm_isr);
	d4_ht_pwm_extinput_set(p_pwm_cb->pwm_dev, p_pwm_cb->pwm_info->ext_in);
	d4_ht_pwm_int_enalbe(p_pwm_cb->pwm_dev, p_pwm_cb->pwm_info->int_type);
	p_pwm_cb->pwm_info->info.freq2.period	= (ul_time & 0xffffffff);
	d4_ht_pwm_config(p_pwm_cb->pwm_dev, &(p_pwm_cb->pwm_info->info));
	d4_ht_pwm_enable(p_pwm_cb->pwm_dev);
	printk("s-pwm\n");
	interruptible_sleep_on(p_pwm_cb->pt_waiting_process);
	printk("i-pwm\n");
	return 0;
}
static int shutter_pwm_stop(int i_channel)
{
	shutter_pwm_cb* 		p_pwm_cb = NULL;
	struct list_head	*list;
	list_for_each(list, &shutter_pwm_list)
	{
		p_pwm_cb	= list_entry(list, shutter_pwm_cb, list);
		if (p_pwm_cb->pwm_info->id == i_channel)
		{
			break;
		}
	}
	if(!p_pwm_cb)
	{
		return -1;
	}
	d4_ht_pwm_free(p_pwm_cb->pwm_dev);
	printk("e-pwm\n");
	wake_up_interruptible(p_pwm_cb->pt_waiting_process);
	printk("w-pwm\n");
	return 0;
}
static int shutter_timer_start(int i_channel, unsigned long ul_time)
{	
	shutter_timer_cb* p_timer_cb = NULL;
	struct list_head	*list;
	list_for_each(list, &shutter_timer_list)
	{
		p_timer_cb	= list_entry(list, shutter_timer_cb, list);
		if (p_timer_cb->channel == i_channel)
			break;
	}
	if(!p_timer_cb)
	{
		return -1;
	}
	hrtimer_init(&p_timer_cb->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	p_timer_cb->timer.function 		= shutter_timer_handler;
	if(hrtimer_start(&p_timer_cb->timer, ktime_set(0, ul_time), HRTIMER_MODE_REL) < 0)
	{
		return -1;
	}
	p_timer_cb->bl_is_waiting = true;
	//printk("S: %d[%x]\n", p_timer_cb->channel, p_timer_cb->pt_waiting_process);
	interruptible_sleep_on(p_timer_cb->pt_waiting_process);
	//printk("S%d-I\n", p_timer_cb->channel);
	return 0;
}
static int shutter_timer_stop(int i_channel)
{
	shutter_timer_cb* p_timer_cb = NULL;
	struct list_head	*list;
	list_for_each(list, &shutter_timer_list)
	{
		p_timer_cb	= list_entry(list, shutter_timer_cb, list);
		if (p_timer_cb->channel == i_channel)
		{
			break;
		}
	}
	if(!p_timer_cb)
	{
		return -1;
	}
	hrtimer_cancel(&(p_timer_cb->timer));
	if(p_timer_cb->bl_is_waiting)
	{
		//printk("E: %d\n", p_timer_cb->channel);
		wake_up_interruptible(p_timer_cb->pt_waiting_process);
		p_timer_cb->bl_is_waiting = false;
		//printk("E-W\n");
	}
	return 0;
}

static irqreturn_t gpio_isr(int irq, void *handle)
{
	shutter_irq_cb* p_irq_cb = (shutter_irq_cb*)handle;
	int i_current_pi_signal = 0;
	int i = 0;
	if(p_irq_cb == NULL)
	{
		assert(0);
		return IRQ_NONE;
	}
	else
	{
		i_current_pi_signal = shutter_gpio_get(p_irq_cb->ui_gpio_num);

		//Ripple check
		if(p_irq_cb->i_waiting_signal != i_current_pi_signal)
		{//unexpected situation. have a little delay time.
			for(i = 0; i < 40; i++)
			{
				i_current_pi_signal = shutter_gpio_get(p_irq_cb->ui_gpio_num);	
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
		
		disable_irq_nosync(irq);
		wake_up_interruptible(p_irq_cb->pt_waiting_process);
	}
	return IRQ_HANDLED;
}

static void pwm_isr(int pwm_id)
{
	shutter_pwm_cb* 	pwm_data = NULL;
	struct list_head	*list;
	list_for_each(list, &shutter_pwm_list)
	{
		pwm_data	= list_entry(list, shutter_pwm_cb, list);
		if (pwm_data->pwm_info->id == pwm_id)
			break;
	}
	printk("s-\n");
	wake_up_interruptible(pwm_data->pt_waiting_process);	
	printk("w-\n");
}

static enum hrtimer_restart shutter_timer_handler(struct hrtimer *timer_param)
{
	shutter_timer_cb* tdata = container_of(timer_param, shutter_timer_cb, timer);
//	printk("H: %d[0x%x]\n", tdata->channel, tdata->pt_waiting_process);
	if(tdata->bl_is_waiting)
	{
		//printk("H: %d\n", tdata->channel);
		wake_up_interruptible(tdata->pt_waiting_process);
		tdata->bl_is_waiting = false;
	//	printk("H-W\n");
	}

	return HRTIMER_NORESTART;
}


int shutter_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int shutter_release(struct inode *inode, struct file *filp)
{
	return 0;
}

long shutter_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg)
{
	int  	size = 0;
	int  	err = 0;
	t_shutter_gpio_param	t_gpio_param 	= {0, };
	t_shutter_pwm_param 	t_pwm_param 	= {0, };
	t_shutter_timer_param	t_timer_param  	= {0, };

	
	if (_IOC_TYPE(cmd) != SHUTTER_MAGIC)
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
		case SHUTTER_IOCTL_GPIO_SET:
			if (copy_from_user((void *)&t_gpio_param, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				shutter_gpio_set(g_shutter_gpio[t_gpio_param.ui_gpio_num], t_gpio_param.i_value);
			}
			break;
		case SHUTTER_IOCTL_GPIO_GET:
			if (copy_from_user((void *)&t_gpio_param, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				t_gpio_param.i_value = shutter_gpio_get(g_shutter_gpio[t_gpio_param.ui_gpio_num]);
				if (copy_to_user((void *)arg, (const void *)&t_gpio_param, size))
				{
					err = -ENOMEM;
				}
			}
			break;
		case SHUTTER_IOCTL_INTERRUPT_SET_ACT:
			if (copy_from_user((void *)&t_gpio_param, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(t_gpio_param.i_value)
				{//turn on pi interrupt
					if(shutter_interrupt_enable(g_shutter_gpio[t_gpio_param.ui_gpio_num]) < 0)
					{
						assert(0);
					}
					if(shutter_interrupt_wait(g_shutter_gpio[t_gpio_param.ui_gpio_num]) < 0)
					{
						assert(0);
					}
				}
				else
				{//turn off pi interrupt
					if(shutter_interrupt_disable(g_shutter_gpio[t_gpio_param.ui_gpio_num]) < 0)
					{
						assert(0);
					}
				}
			}
			break;
		case SHUTTER_IOCTL_INTERRUPT_WAIT:
			break;
		case SHUTTER_IOCTL_INTERRUPT_SET_LEVEL:
			if (copy_from_user((void *)&t_gpio_param, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(shutter_interrupt_set_level(g_shutter_gpio[t_gpio_param.ui_gpio_num], t_gpio_param.i_value) < 0)
				{
					dev_err(shutter_miscdev.this_device, "set IRQ type failed\n");	
					assert(0);
				}		
			}
			break;
		case SHUTTER_IOCTL_PWM_START:
			if (copy_from_user((void *)&t_pwm_param, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(shutter_pwm_start(t_pwm_param.i_channel, t_pwm_param.ul_time) < 0)
				{
					assert(0);
				}
			}
			break;
		case SHUTTER_IOCTL_PWM_WAIT:
			break;
		case SHUTTER_IOCTL_PWM_STOP:	
			if (copy_from_user((void *)&t_pwm_param, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(shutter_pwm_stop(t_pwm_param.i_channel) < 0)
				{
					assert(0);
				}
			}
			break;
		case SHUTTER_IOCTL_GPIO_TO_PWM:
			if (copy_from_user((void *)&t_gpio_param, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(shutter_gpio_to_pwm(g_shutter_gpio[t_gpio_param.ui_gpio_num]) < 0)
				{
					err = -EFAULT;
				}
			}
			break;
		case SHUTTER_IOCTL_PWM_TO_GPIO:
			if (copy_from_user((void *)&t_gpio_param, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(shutter_pwm_to_gpio(g_shutter_gpio[t_gpio_param.ui_gpio_num]) < 0)
				{
					err = -EFAULT;
				}
			}
			break;
		case SHUTTER_IOCTL_TIMER_START:	
			if (copy_from_user((void *)&t_timer_param, (const void *)arg, size))
			{
				err = -ENOMEM;
			}	
			else
			{
				if(shutter_timer_start(t_timer_param.i_channel, t_timer_param.ul_time) < 0)
				{
					assert(0);
				}
			}
			break;
		case SHUTTER_IOCTL_TIMER_WAIT:
			break;
		case SHUTTER_IOCTL_TIMER_STOP:	
			if (copy_from_user((void *)&t_timer_param, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(shutter_timer_stop(t_timer_param.i_channel) < 0)
				{
					assert(0);
				}
			}
			break;
		case SHUTTER_IOCTL_INITIALIZE:	
			if(shutter_pad_set(g_shutter_gpio[SHUTTER_PIO_POWER], "sh_pwr", SHUTTER_DIRECTION_OUTPUT) < 0)
			{
				return -EFAULT;
			}
			if(shutter_pad_set(g_shutter_gpio[SHUTTER_PIO_MG1], "sh_mg1", SHUTTER_DIRECTION_OUTPUT) < 0)
			{
				return -EFAULT;
			}
			if(shutter_pad_set(g_shutter_gpio[SHUTTER_PIO_MG2], "sh_mg2", SHUTTER_DIRECTION_OUTPUT) < 0)
			{
				return -EFAULT;
			}
			if(shutter_pad_set(g_shutter_gpio[SHUTTER_PIO_MOT1], "sh_mot1", SHUTTER_DIRECTION_OUTPUT) < 0)
			{
				return -EFAULT;
			}
			if(shutter_pad_set(g_shutter_gpio[SHUTTER_PIO_MOT2], "sh_mot2", SHUTTER_DIRECTION_OUTPUT) < 0)
			{
				return -EFAULT;
			}
			if(shutter_pad_set(g_shutter_gpio[SHUTTER_PIO_ACT1], "sh_act1", SHUTTER_DIRECTION_OUTPUT) < 0)
			{
				return -EFAULT;
			}
			if(shutter_pad_set(g_shutter_gpio[SHUTTER_PIO_ACT2], "sh_act2", SHUTTER_DIRECTION_OUTPUT) < 0)
			{
				return -EFAULT;
			}
			if(shutter_pad_set(g_shutter_gpio[SHUTTER_PIO_PI1], "sh_pi1", SHUTTER_DIRECTION_INPUT) < 0)
			{
				return -EFAULT;
			}		
			break;		
		
		default:
			break;
	}
	return err;
}

const struct file_operations shutter_fops = {
	.owner = THIS_MODULE,
	.open = shutter_open,
	.unlocked_ioctl = shutter_ioctl,
	.release = shutter_release,
};

static struct miscdevice shutter_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "shutter",
	.fops = &shutter_fops,
};

static int __devinit shutter_probe(struct platform_device *pdev)
{
	int ret = 0;
	int i = 0;
	shutter_platform_data*	pdata = pdev->dev.platform_data;
	shutter_pwm_cb*			p_pwm_cb = NULL;
	shutter_timer_cb*		p_timer_cb = NULL;
	shutter_irq_cb*			p_irq_cb = NULL;
	struct list_head*		list;
	//allocate pwm control block struct
	for (i = 0; i < pdata->n_pwm_infos; i++) 
	{
		p_pwm_cb	= kzalloc(sizeof(shutter_pwm_cb), GFP_KERNEL);
		if (!p_pwm_cb) 
		{
			ret		= -ENOMEM;
			goto err_pwm_alloc;
		}
		else
		{
			p_pwm_cb->pwm_info = &(pdata->pwm_info[i]);
			p_pwm_cb->pt_waiting_process = kzalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
			if (!p_pwm_cb->pt_waiting_process) 
			{				
				ret		= -ENOMEM;
				goto err_pwm_alloc;
			}
			else
			{			
				init_waitqueue_head(p_pwm_cb->pt_waiting_process);
				list_add_tail(&p_pwm_cb->list, &shutter_pwm_list);
			}
		}
	}
	//allocate timer control block struct
	for(i = 0; i < pdata->n_timer; i++)
	{
		p_timer_cb = kzalloc(sizeof(shutter_timer_cb), GFP_KERNEL);
		if (!p_timer_cb)
		{
			ret		= -ENOMEM;
			goto err_timer_alloc;			
		}
		else
		{
			p_timer_cb->channel 				= i;
			p_timer_cb->bl_is_waiting			= false;
			p_timer_cb->pt_waiting_process 		= kzalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
			if (!p_timer_cb->pt_waiting_process) 
			{				
				ret		= -ENOMEM;
				goto err_timer_alloc;
			}
			else
			{			
				init_waitqueue_head(p_timer_cb->pt_waiting_process);
				list_add_tail(&p_timer_cb->list, &shutter_timer_list);
			}	
		}
	}		
	//allocate irq control block struct
	for(i = 0; i < pdata->n_pi; i++)
	{
		p_irq_cb = kzalloc(sizeof(shutter_irq_cb), GFP_KERNEL);
		if (!p_irq_cb)
		{
			ret		= -ENOMEM;
			goto err_irq_alloc;			
		}
		else
		{
			p_irq_cb->ui_gpio_num = 96; //PI1 (we have only 1 PI interrupt now)
			p_irq_cb->pt_waiting_process = kzalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
			if (!p_irq_cb->pt_waiting_process) 
			{				
				ret		= -ENOMEM;
				goto err_irq_alloc;
			}
			else
			{			
				init_waitqueue_head(p_irq_cb->pt_waiting_process);
				list_add_tail(&p_irq_cb->list, &shutter_irq_list);
			}	
		}
	}
	//allocate gpio index info
	g_shutter_gpio = kzalloc(sizeof(unsigned) * pdata->n_gpio_infos, GFP_KERNEL);
	if (!g_shutter_gpio)
	{
		ret		= -ENOMEM;
		goto err_gpio_alloc;			
	}
	for(i = 0; i < pdata->n_gpio_infos; i++)
	{
		g_shutter_gpio[i] = pdata->gpio_info[i].gpio_num;
	}
	if(ret < 0)
	{
		//error case
	}
	else
	{
		ret = misc_register(&shutter_miscdev);
		if (ret < 0) {
			dev_err(&pdev->dev, "Failed to register misc driver.\n");
		}
		else
		{	
			dev_info(&pdev->dev, "device registered as /dev/%s\n", pdev->name);
		}
	}
	return ret;
err_irq_alloc:
	while(!list_empty(&shutter_irq_list)) 
	{
		p_irq_cb	= list_entry(list, shutter_irq_cb, list);
		if(p_irq_cb)
		{
			if(p_irq_cb->pt_waiting_process)
			{
				kfree(p_irq_cb->pt_waiting_process);
			}
			list_del(&p_irq_cb->list);
			kfree(p_irq_cb);
		}
	}
err_timer_alloc:
	while(!list_empty(&shutter_timer_list)) 
	{
		p_timer_cb = list_entry(list, shutter_timer_cb, list);
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
err_pwm_alloc:	
	while(!list_empty(&shutter_pwm_list)) 
	{
		p_pwm_cb = list_entry(list, shutter_pwm_cb, list);
		if(p_pwm_cb)
		{
			if(p_pwm_cb->pt_waiting_process)
			{
				kfree(p_pwm_cb->pt_waiting_process);
			}
			list_del(&p_pwm_cb->list);
			kfree(p_pwm_cb);
		}
	}	
err_gpio_alloc:
	if(g_shutter_gpio)
	{
		kfree(g_shutter_gpio);
	}

	return ret;
	
}

static int __devexit shutter_remove(struct platform_device* pdev)
{
	struct list_head*		list = NULL;
	shutter_pwm_cb*			pwm_data = NULL;
	shutter_timer_cb*		p_timer_cb = NULL;
	shutter_irq_cb*			p_irq_cb = NULL;

	while(!list_empty(&shutter_pwm_list)) 
	{
		pwm_data	= list_entry(list, shutter_pwm_cb, list);
		if(pwm_data)
		{
			if(pwm_data->pt_waiting_process)
			{
				kfree(pwm_data->pt_waiting_process);
			}
			list_del(&pwm_data->list);
			kfree(pwm_data);
		}
	}

	while(!list_empty(&shutter_timer_list)) 
	{
		p_timer_cb	= list_entry(list, shutter_timer_cb, list);
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
	while(!list_empty(&shutter_irq_list)) 
	{
		p_irq_cb	= list_entry(list, shutter_irq_cb, list);
		if(p_irq_cb)
		{
			if(p_irq_cb->pt_waiting_process)
			{
				kfree(p_irq_cb->pt_waiting_process);
			}
			list_del(&p_irq_cb->list);
			kfree(p_irq_cb);
		}
	}
	if(g_shutter_gpio)
	{
		kfree(g_shutter_gpio);
	}
	
	return 0;
}

static int shutter_suspend(struct platform_device* pdev, pm_message_t state)
{
	return 0;
}

static int shutter_resume(struct platform_device* pdev)
{
	return 0;
}



static struct platform_driver shutter_driver = {
	.probe 		= shutter_probe,
	.suspend  	= shutter_suspend,
	.resume 	= shutter_resume,
	.remove 	= __devexit_p(shutter_remove),
	.driver 	= 
	{
		.name = "shutter",
		.owner = THIS_MODULE,
	},
};

static int __init shutter_init(void)
{
	return platform_driver_register(&shutter_driver);
}

static void __exit shutter_exit(void)
{
	platform_driver_unregister(&shutter_driver);
}

module_init(shutter_init);
module_exit(shutter_exit);

MODULE_AUTHOR("choi yong jin");
MODULE_DESCRIPTION("shutter driver");
MODULE_LICENSE("GPL");
