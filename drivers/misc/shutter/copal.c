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
#include <linux/copal_ioctl.h>
#include <asm/uaccess.h>
#include <linux/module.h>

#include <mach/gpio.h>
#include <linux/device.h>
#include <mach/version_information.h>


#define assert(s) do{if (!(s)) panic(#s);} while(0);
#define COPAL_DIRECTION_INPUT 			0
#define COPAL_DIRECTION_OUTPUT 			1

typedef struct t_irq_param {
	struct list_head		list;
	int						i_waiting_signal;
	int						irq_descriptor;
	unsigned 				ui_pi_io_num;
	unsigned 				ui_brk_io_num1;
	unsigned 				ui_brk_io_num2;
	bool					bl_brk_on;
	wait_queue_head_t*		pt_waiting_process;
}copal_irq_cb;

static LIST_HEAD(copal_irq_list);

static struct miscdevice copal_miscdev;
static unsigned          g_ui_copal_pi_gpio_num = 0;

static int copal_pad_set(unsigned gpio_num, char* pch_name, int i_direction)
{
	int err = 0;
	pinctrl_free_gpio(gpio_num);
	err = pinctrl_request_gpio(gpio_num);
	gpio_free(gpio_num);
	err = gpio_request(gpio_num, pch_name);
	if (err < 0)
	{
		dev_err(copal_miscdev.this_device, "PI request Error: %d %s\n", err, pch_name);
		gpio_free(gpio_num);
	}
	else
	{
		if(i_direction == COPAL_DIRECTION_INPUT)
		{
			gpio_direction_input(gpio_num);
		}
		else if(i_direction == COPAL_DIRECTION_OUTPUT)
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

static bool copal_gpio_set(unsigned gpio_num, int iValue)
{
	gpio_set_value(gpio_num, iValue);
	return true;
}

static int copal_gpio_get(unsigned gpio_num)
{
	int iRetVal = 0;
	iRetVal = gpio_get_value(gpio_num);
	return iRetVal;
}

static copal_irq_cb* find_irq_cb(unsigned gpio_num)
{
	copal_irq_cb*			p_irq_cb = NULL;
	struct list_head*		list;
	list_for_each(list, &copal_irq_list)
	{
		p_irq_cb	= list_entry(list, copal_irq_cb, list);
		if (p_irq_cb->ui_pi_io_num == gpio_num)
		{
			break;
		}
	}
	return p_irq_cb;
}

static int copal_interrupt_wait(unsigned gpio_num)
{
	copal_irq_cb*			p_irq_cb = NULL;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		return -1;
	}
	interruptible_sleep_on(p_irq_cb->pt_waiting_process);
	return 0;
}

static int copal_interrupt_enable(unsigned gpio_num)
{
	copal_irq_cb*			p_irq_cb = NULL;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		return -1;
	}
	if(p_irq_cb->i_waiting_signal == COPAL_INT_NONE_EDGE)
	{
		dev_err(copal_miscdev.this_device, "Not defined Edge\n");
		return -1;
	}
	enable_irq(p_irq_cb->irq_descriptor);
	return 0;
}
static int copal_interrupt_disable(unsigned gpio_num)
{
	copal_irq_cb* 		p_irq_cb = NULL;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		return -1;
	}
	p_irq_cb->i_waiting_signal = COPAL_INT_NONE_EDGE;
	irq_set_irq_type(p_irq_cb->irq_descriptor, IRQ_TYPE_NONE);
	disable_irq_nosync(p_irq_cb->irq_descriptor);
	return 0;
}
static int copal_interrupt_set_level(unsigned gpio_num, int i_level)
{
	copal_irq_cb* 		p_irq_cb = NULL;	
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
			case COPAL_INT_FALLING_EDGE:
				irq_level = IRQ_TYPE_EDGE_FALLING;
				break;
			case COPAL_INT_RISING_EDGE:
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
static int copal_interrupt_set(unsigned gpio_num, int i_level, bool bl_brk_on)
{
	copal_irq_cb* 		p_irq_cb = NULL;	
	int						err = 0;
	p_irq_cb = find_irq_cb(gpio_num);
	if(!p_irq_cb)
	{
		err = -1;
	}
	else
	{
		if(copal_interrupt_set_level(p_irq_cb->ui_pi_io_num, i_level) < 0)
		{
			err = -2;
		}
		p_irq_cb->bl_brk_on = bl_brk_on;
	}
	return err;
}
static irqreturn_t copal_isr(int irq, void *handle)
{
	copal_irq_cb* p_irq_cb = (copal_irq_cb*)handle;
	int i_current_pi_signal = 0;
	int i = 0;
	if(p_irq_cb == NULL)
	{
		assert(0);
		return IRQ_NONE;
	}
	else
	{
		i_current_pi_signal = copal_gpio_get(p_irq_cb->ui_pi_io_num);
		//Ripple check
		if(p_irq_cb->i_waiting_signal != i_current_pi_signal)
		{//unexpected situation. have a little delay time.
			for(i = 0; i < 40; i++)
			{
				i_current_pi_signal = copal_gpio_get(p_irq_cb->ui_pi_io_num);	
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
		//Break Motor
		if(p_irq_cb->bl_brk_on)
		{
			copal_gpio_set(p_irq_cb->ui_brk_io_num1, 1);
			copal_gpio_set(p_irq_cb->ui_brk_io_num2, 1);
		}
		wake_up_interruptible(p_irq_cb->pt_waiting_process);
			
		
		
	}
	return IRQ_HANDLED;
}

int copal_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int copal_release(struct inode *inode, struct file *filp)
{
	return 0;
}

long copal_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg)
{
	int  	size = 0;
	int  	err = 0;
	t_copal_param		t_command 	= {0, };
	
	if (_IOC_TYPE(cmd) != COPAL_MAGIC)
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
		case COPAL_IOCTL_GPIO_GET:
			t_command.i_level = copal_gpio_get(g_ui_copal_pi_gpio_num);
			if (copy_to_user((void *)arg, (const void *)&t_command, size))
			{
				err = -ENOMEM;
			}			
			break;
		case COPAL_IOCTL_INTERRUPT_SET:
			if (copy_from_user((void *)&t_command, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(copal_interrupt_set(g_ui_copal_pi_gpio_num, t_command.i_level, t_command.bl_is_break_on) < 0)
				{
					err = -EFAULT;
				}
			}
			break;
		case COPAL_IOCTL_INTERRUPT_ENABLE:
			if (copy_from_user((void *)&t_command, (const void *)arg, size))
			{
				err = -ENOMEM;
			}
			else
			{
				if(t_command.i_level)
				{//turn on pi interrupt
					if(copal_interrupt_enable(g_ui_copal_pi_gpio_num) < 0)
					{
						err = -EFAULT;
					}
				}
				else
				{//turn off pi interrupt
					if(copal_interrupt_disable(g_ui_copal_pi_gpio_num) < 0)
					{
						err = -EFAULT;
					}
				}
			}
			break;
		case COPAL_IOCTL_INTERRUPT_WAIT:		
			if(copal_interrupt_wait(g_ui_copal_pi_gpio_num) < 0)
			{
				assert(0);
			}			
			break;
		case COPAL_IOCTL_INITIALIZE:	
			if(copal_pad_set(g_ui_copal_pi_gpio_num, "sh_pi", COPAL_DIRECTION_INPUT) < 0)
			{
				err = -EFAULT;
				dev_err(copal_miscdev.this_device, "pad set failed %d\n", err);
			}	
			break;		
		
		default:
			err = -EINVAL;
			break;
	}
	return err;
}

const struct file_operations copal_fops = 
{
	.owner			= THIS_MODULE,
	.open			= copal_open,
	.release		= copal_release,
	.unlocked_ioctl = copal_ioctl,
};


static struct miscdevice copal_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "copal",
	.fops = &copal_fops,
};

static int __init copal_init(void)
{
	int ret;
	copal_irq_cb*			p_irq_cb;
	struct list_head*		list;	
	ret = 0;
	p_irq_cb = NULL;
	list = NULL;

	//allocate irq control block struct
	p_irq_cb = kzalloc(sizeof(copal_irq_cb), GFP_KERNEL);
	if (!p_irq_cb)
	{
		ret		= -ENOMEM;
		goto err_irq_alloc;			
	}
	else
	{
		g_ui_copal_pi_gpio_num = GPIO_SH_PI_OUT;
		p_irq_cb->ui_pi_io_num = g_ui_copal_pi_gpio_num; 
		p_irq_cb->ui_brk_io_num1 = GPIO_SH_MOT_IN1; 
		p_irq_cb->ui_brk_io_num2 = GPIO_SH_MOT_IN2; 
		p_irq_cb->irq_descriptor = gpio_to_irq(p_irq_cb->ui_pi_io_num);
		ret = request_irq(p_irq_cb->irq_descriptor, copal_isr, IRQF_DISABLED, "COPAL IRQ", (void*)p_irq_cb);
		if (ret)
		{
			dev_err(copal_miscdev.this_device, "request IRQ failed %d\n", ret);
			ret = -EFAULT;
			goto err_irq_alloc;
		}
		p_irq_cb->pt_waiting_process = kzalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
		if (!p_irq_cb->pt_waiting_process) 
		{				
			ret		= -ENOMEM;
			goto err_irq_alloc;
		}
		else
		{			
			init_waitqueue_head(p_irq_cb->pt_waiting_process);
			list_add_tail(&p_irq_cb->list, &copal_irq_list);
		}	
	}
	
	if(ret < 0)
	{
		//error case
		printk("Error Copal\n");
	}
	else
	{
		ret = misc_register(&copal_miscdev);
		if (ret < 0) {
			dev_err(copal_miscdev.this_device, "Failed to register misc driver.\n");
		}
		else
		{	
			dev_info(copal_miscdev.this_device, "device registered as /dev/%s\n", copal_miscdev.name);
		}
	}
		
	return ret;
err_irq_alloc:
	while(!list_empty(&copal_irq_list)) 
	{
		p_irq_cb	= list_entry(list, copal_irq_cb, list);
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

static void __exit copal_exit(void)
{
	struct list_head*		list = NULL;
	copal_irq_cb*			p_irq_cb = NULL;

	while(!list_empty(&copal_irq_list)) 
	{
		p_irq_cb	= list_entry(list, copal_irq_cb, list);
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
	misc_deregister(&copal_miscdev);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(copal_init);
#else
fast_dev_initcall(copal_init);
#endif

module_exit(copal_exit);

MODULE_AUTHOR("choi yong jin");
MODULE_DESCRIPTION("copal driver");
MODULE_LICENSE("GPL");

