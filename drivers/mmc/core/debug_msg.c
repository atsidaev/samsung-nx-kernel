#include <linux/kernel.h>
#include <stdarg.h>
#include <linux/mmc/debug_msg.h>


void MMC_COLOR_DEBUG(int color, const char* format, ...)
{
#if (MMC_DEBUG_MSG_ENABLE==0)
	return;
#endif

	char buf[512] = {0, };
	va_list args;
	int l;
	
	va_start(args, format);

	switch (color)
	{
	case RED: //core.c
		l = snprintf(buf, sizeof(buf), "\033[0;31m");
		break;
	case GREEN://dw_mmc.c
		l = snprintf(buf, sizeof(buf), "\033[0;32m");
		break;
	case BROWN://block.c
		l = snprintf(buf, sizeof(buf), "\033[0;33m");
		break;
	case BLUE: //Queue.c
		l = snprintf(buf, sizeof(buf), "\033[0;34m");
		break;
	case PURPLE: //Host.c
		l = snprintf(buf, sizeof(buf), "\033[0;35m");
		break;
	case CYAN: //mmc_ops.c & sd_ops.c
		l = snprintf(buf, sizeof(buf), "\033[0;36m");
		break;
	case GRAY: //sd.c
		l = snprintf(buf, sizeof(buf), "\033[1;30m");
		break;
	default:
		l = snprintf(buf, sizeof(buf), "\033[0m");
		break;
	}

	l += vsnprintf(buf + l, sizeof(buf), format, args);
	l += snprintf(buf + l, sizeof(buf), "\033[0m");
	va_end(args);
	buf[l++] = '\0';
	
	printk(KERN_EMERG"%s\n", buf);
	return;
	
}
