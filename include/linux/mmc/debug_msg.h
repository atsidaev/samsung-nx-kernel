

//#define MMC_DEBUG_MSG_ENABLE	1

enum PRINT_COLOR
{
	UNKNOW = 0,
	RED,
	GREEN,
	BROWN,
	BLUE,
	PURPLE,
	CYAN,
	GRAY
};

void MMC_COLOR_DEBUG(int color, const char* format, ...);
