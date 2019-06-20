
#ifndef __LINUX_PWMDEV_H
#define __LINUX_PWMDEV_H


#include <linux/types.h>
#include <mach/d4_ht_pwm_type.h>

#define PWM_IOC_MAGIC  'p'

struct ptc_cnt_set {
	unsigned int cnt_val;
	unsigned int edge_type;
};

struct ptc_two_en {
	unsigned int sec_type;
	unsigned int sec_value;
};

#define PWM_SIOCTL_SET						_IOW(PWM_IOC_MAGIC, 1, struct ht_pwm_conf_info)
#define PWM_SIOCTL_ENABLE				_IO(PWM_IOC_MAGIC, 2)
#define PWM_SIOCTL_DISABLE			_IO(PWM_IOC_MAGIC, 3)
#define PWM_SIOCTL_INTEN				_IOW(PWM_IOC_MAGIC, 4, unsigned int)
#define PWM_SIOCTL_EXTEN				_IOW(PWM_IOC_MAGIC, 5, unsigned int)
#define PWM_SIOCTL_CLEAR				_IO(PWM_IOC_MAGIC, 6)

#define PWM_MIOCTL_SET						_IOW(PWM_IOC_MAGIC, 7, struct ht_pwm_mconf_info)
#define PWM_MIOCTL_ENABLE				_IO(PWM_IOC_MAGIC, 8)
#define PWM_MIOCTL_DISABLE			_IO(PWM_IOC_MAGIC, 9)
#define PWM_MIOCTL_INTEN				_IOW(PWM_IOC_MAGIC, 10, unsigned int)
#define PWM_MIOCTL_EXTEN				_IOW(PWM_IOC_MAGIC, 11, unsigned int)
#define PWM_MIOCTL_CLEAR				_IO(PWM_IOC_MAGIC, 12)

#define PWM_EXT_TRG						_IOW(PWM_IOC_MAGIC, 13, unsigned int)

#define PWM_PHY_INFO				_IOR(PWM_IOC_MAGIC, 14, struct ht_pwm_phys_reg_info)
#define PWM_INT_MFUNC_SET 				_IO(PWM_IOC_MAGIC, 15)
#define PWM_INT_SFUNC_SET 				_IO(PWM_IOC_MAGIC, 19)
#define PWM_PAD_CTRL 				_IOW(PWM_IOC_MAGIC, 20, unsigned int)


#endif
