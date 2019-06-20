#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/linkage.h>
#include <asm/io.h>

#ifdef CONFIG_SND_SOC_D4_WM8994

int wm8994_delayed_resume(void);

asmlinkage int sys_sound_restart(void)
{
	return wm8994_delayed_resume();
}
#else

asmlinkage int sys_sound_restart(void) {
	return 0;
}

#endif



