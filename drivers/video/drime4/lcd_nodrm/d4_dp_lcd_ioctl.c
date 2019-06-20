#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/memory.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/completion.h>
#include <linux/fb.h>

#include <video/drime4/d4_dp_ioctl.h>
#include <mach/d4_mem.h>
#include "d4_dp_lcd_if.h"

static unsigned int stride;
static enum edp_layer over_layer;
static struct stdp_ycbcr background;
static struct stvideo_display_area vid_display;
static struct stdp_argb grp_background;
static struct stgraphic_display_area grp_display;
static struct stgrpdisplay graphic;
static enum egrp_scale scale;
static struct stdp_grp_prority prority;
static struct stbb_table bb_table;
static struct stdp_rgb rgb_info;
static struct stbb_info bb_info;
static struct stbb_onoff bb_onoff;
static struct stlcdfilter filter_ctrl;
static struct stzebra_set zebra_set;
static struct stdp_rgb_range range;
static struct stfbnlcvideo nlc_video;
static struct sttnlc_video nlc_vid_Set;
static struct stfbnlcgraphic nlc_grp;
static struct stnlc_graphic nlc_grp_set;
static struct stlcd_graphic_alpha alpha;
static struct stdp_window_onoff win_onoff;

int drime4_fb_release(struct fb_info *info, int user)
{

	return 0;
}

static int drime4_fb_lcd_check_var(struct fb_var_screeninfo *var,
		struct fb_info *fb)
{

	return 0;
}

static int drime4_fb_lcd_set_par(struct fb_info *fb)
{

	return 0;
}

static int drime4_fb_lcd_blank(int blank_mode, struct fb_info *fb)
{
	/* do nothing for now */

	return 0;
}

static int drime4_fb_lcd_setcolreg(unsigned int regno, unsigned int red,
		unsigned int green, unsigned int blue, unsigned int transp,
		struct fb_info *fb)
{
	return 0;
}

static int drime4_fb_lcd_pan_display(struct fb_var_screeninfo *var,
		struct fb_info *info)
{

	return 0;
}
static int drime4_fb_lcd_ioctl(struct fb_info *info, unsigned int cmd,
		unsigned long arg)
{
	unsigned int retval = 0;
	int ret;

	switch (cmd) {

	case DP_IOCTL_LCD_VID_STRIDE: {

		DpPRINTK(" fb ioctl video stride arg = %d\n", arg);
		ret = copy_from_user((void *) &stride, (const void *) arg,
				sizeof(stride));
		if (ret < 0)
			return -EFAULT;

		if (d4_dp_lcd_video_stride(stride) < 0)
			printk("stride ioctl set fail\n");
		break;
	}
	case DP_IOCTL_SLCD_VID_STRIDE: {
			DpPRINTK(" fb ioctl video stride arg = %d\n", arg);
			ret = copy_from_user((void *) &stride, (const void *) arg,
					sizeof(stride));
			if (ret < 0)
				return -EFAULT;
			if (d4_dp_sublcd_video_stride(stride) < 0)
				printk("stride ioctl set fail\n");
			break;
		}

	case DP_IOCTL_LCD_VID_BACKGROUND: {

		ret = copy_from_user((void *) &background, (const void *) arg,
				sizeof(background));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" back ground data.y =0x%x\n", background.DP_Y);
		DpPRINTK(" back ground data.cb =0x%x\n", background.DP_Cb);
		DpPRINTK(" back ground data.cr =0x%x\n", background.DP_Cr);

		d4_dp_lcd_video_background(&background);

		break;
	}

	case DP_IOCTL_SLCD_VID_BACKGROUND: {

		ret = copy_from_user((void *) &background, (const void *) arg,
				sizeof(background));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" back ground data.y =0x%x\n", background.DP_Y);
		DpPRINTK(" back ground data.cb =0x%x\n", background.DP_Cb);
		DpPRINTK(" back ground data.cr =0x%x\n", background.DP_Cr);

		d4_dp_sublcd_video_background(&background);

		break;
	}

	case DP_IOCTL_LCD_VID_DISPLAY: {

		ret = copy_from_user((void *) &vid_display, (const void *) arg,
				sizeof(vid_display));

		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" fb ioctl video display h star = %d\n", vid_display.display.H_Start);
		DpPRINTK(" fb ioctl video display h size = %d\n", vid_display.display.H_Size);
		DpPRINTK(" fb ioctl video display v star = %d\n", vid_display.display.V_Start);
		DpPRINTK(" fb ioctl video display v size = %d\n", vid_display.display.V_Size);

		d4_dp_lcd_video_display_area(vid_display.win, vid_display.bit,
				vid_display.display);

		break;
	}

	case DP_IOCTL_SLCD_VID_DISPLAY: {

		ret = copy_from_user((void *) &vid_display, (const void *) arg,
				sizeof(vid_display));

		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" fb ioctl video display h star = %d\n", vid_display.display.H_Start);
		DpPRINTK(" fb ioctl video display h size = %d\n", vid_display.display.H_Size);
		DpPRINTK(" fb ioctl video display v star = %d\n", vid_display.display.V_Start);
		DpPRINTK(" fb ioctl video display v size = %d\n", vid_display.display.V_Size);

		d4_dp_sublcd_video_display_area(vid_display.win, vid_display.bit,
				vid_display.display);

		break;
	}

	case DP_IOCTL_LCD_VID_WINDOW_ONOFF: {
		ret = copy_from_user((void *) &win_onoff, (const void *) arg,
				sizeof(win_onoff));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("video window%d  on/off =%d\n", win_onoff.win, win_onoff.onoff);

		d4_dp_lcd_video_window_onoff(win_onoff.win, win_onoff.onoff);
		break;
	}

	case DP_IOCTL_SLCD_VID_WINDOW_ONOFF: {
		ret = copy_from_user((void *) &win_onoff, (const void *) arg,
				sizeof(win_onoff));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("video window%d  on/off =%d\n", win_onoff.win, win_onoff.onoff);

		d4_dp_sublcd_video_window_onoff(win_onoff.win, win_onoff.onoff);
		break;
	}

	case DP_IOCTL_LCD_VID_SET: {

		struct stvideodisplay video;

		DpPRINTK(" Video Set I/O cntrl \n");

		ret
				= copy_from_user((void *) &video, (const void *) arg,
						sizeof(video));
		if (ret < 0)
			return -EFAULT;

		if (video.address_mode == DP_VIRTUAL_SET) {
			video.address.y0_address = d4_uservirt_to_phys(
					video.address.y0_address);
			video.address.c0_address = d4_uservirt_to_phys(
					video.address.c0_address);
			video.address.y1_address = d4_uservirt_to_phys(
					video.address.y1_address);
			video.address.c1_address = d4_uservirt_to_phys(
					video.address.c1_address);
		}

		DpPRINTK("video.address.y0_address =0x%x\n" , video.address.y0_address);
		DpPRINTK("video.address.y1_address =0x%x\n" , video.address.y1_address);
		DpPRINTK("video.address.c0_address =0x%x\n" , video.address.c0_address);
		DpPRINTK("video.address.c1_address =0x%x\n" , video.address.c1_address);

		d4_dp_lcd_video_set(video);
		d4_dp_lcd_video_window_onoff(video.win, video.win_onoff);
		break;
	}

	case DP_IOCTL_LCD_VID_ADDRESS: {

		struct stvideo_address vid;

		DpPRINTK(" Video address set I/O Ctrl \n");

		ret
				= copy_from_user((void *) &vid, (const void *) arg,
						sizeof(vid));
		if (ret < 0)
			return -EFAULT;

		if (vid.address_mode == DP_VIRTUAL_SET) {
			vid.address.y0_address = d4_uservirt_to_phys(
					vid.address.y0_address);
			vid.address.c0_address = d4_uservirt_to_phys(
					vid.address.c0_address);
			vid.address.y1_address = d4_uservirt_to_phys(
					vid.address.y1_address);
			vid.address.c1_address = d4_uservirt_to_phys(
					vid.address.c1_address);
		}
		d4_dp_lcd_video_address_set(vid);
		break;
	}

	case DP_IOCTL_SLCD_VID_ADDRESS: {

		struct stvideo_address vid;

		DpPRINTK(" Video address set I/O Ctrl \n");

		ret
				= copy_from_user((void *) &vid, (const void *) arg,
						sizeof(vid));
		if (ret < 0)
			return -EFAULT;

		if (vid.address_mode == DP_VIRTUAL_SET) {
			vid.address.y0_address = d4_uservirt_to_phys(
					vid.address.y0_address);
			vid.address.c0_address = d4_uservirt_to_phys(
					vid.address.c0_address);
			vid.address.y1_address = d4_uservirt_to_phys(
					vid.address.y1_address);
			vid.address.c1_address = d4_uservirt_to_phys(
					vid.address.c1_address);
		}
		d4_dp_sublcd_video_address_set(vid);
		break;
	}

	case DP_IOCTL_SLCD_VID_SET: {
		struct stvideodisplay video;

		DpPRINTK(" Video Set I/O cntrl \n");

		ret
				= copy_from_user((void *) &video, (const void *) arg,
						sizeof(video));
		if (ret < 0)
			return -EFAULT;

		if (video.address_mode == DP_VIRTUAL_SET) {
			video.address.y0_address = d4_uservirt_to_phys(
					video.address.y0_address);
			video.address.c0_address = d4_uservirt_to_phys(
					video.address.c0_address);
			video.address.y1_address = d4_uservirt_to_phys(
					video.address.y1_address);
			video.address.c1_address = d4_uservirt_to_phys(
					video.address.c1_address);
		}

		DpPRINTK("video.address.y0_address =0x%x\n" , video.address.y0_address);
		DpPRINTK("video.address.y1_address =0x%x\n" , video.address.y1_address);
		DpPRINTK("video.address.c0_address =0x%x\n" , video.address.c0_address);
		DpPRINTK("video.address.c1_address =0x%x\n" , video.address.c1_address);

		d4_dp_sublcd_video_set(video);
		d4_dp_sublcd_video_window_onoff(video.win, video.win_onoff);
		break;
	}

	case DP_IOCTL_LCD_GRP_STRIDE: {
		ret = copy_from_user((void *) &stride, (const void *) arg,
				sizeof(stride));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_graphic_stride(stride);
		break;
	}

	case DP_IOCTL_SLCD_GRP_STRIDE: {
		ret = copy_from_user((void *) &stride, (const void *) arg,
				sizeof(stride));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_graphic_stride(stride);
		break;
	}

	case DP_IOCTL_LCD_GRP_BACKGROUND: {
		DpPRINTK(" fb ioctl graphic background first arg = %d\n", arg);
		ret = copy_from_user((void *) &grp_background, (const void *) arg,
				sizeof(grp_background));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" back ground data.alpha =0x%x\n" , grp_background.DP_A);
		DpPRINTK(" back ground data.r =0x%x\n" , grp_background.DP_R);
		DpPRINTK(" back ground data.g =0x%x\n" , grp_background.DP_G);
		DpPRINTK(" back ground data.g =0x%x\n" , grp_background.DP_B);

		d4_dp_lcd_graphic_background(&grp_background);
		break;
	}

	case DP_IOCTL_SLCD_GRP_BACKGROUND: {
		DpPRINTK(" fb ioctl graphic background first arg = %d\n", arg);
		ret = copy_from_user((void *) &grp_background, (const void *) arg,
				sizeof(grp_background));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" back ground data.alpha =0x%x\n" , grp_background.DP_A);
		DpPRINTK(" back ground data.r =0x%x\n" , grp_background.DP_R);
		DpPRINTK(" back ground data.g =0x%x\n" , grp_background.DP_G);
		DpPRINTK(" back ground data.g =0x%x\n" , grp_background.DP_B);

		d4_dp_sublcd_graphic_background(&grp_background);
		break;
	}

	case DP_IOCTL_LCD_GRP_DISPLAY: {
		ret = copy_from_user((void *) &grp_display, (const void *) arg,
				sizeof(grp_display));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_graphic_display_area(&grp_display);
		break;
	}

	case DP_IOCTL_SLCD_GRP_DISPLAY: {
		ret = copy_from_user((void *) &grp_display, (const void *) arg,
				sizeof(grp_display));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_graphic_display_area(&grp_display);
		break;
	}

	case DP_IOCTL_LCD_GRP_SET: {
		unsigned int address;
		ret = copy_from_user((void *) &graphic, (const void *) arg,
				sizeof(graphic));
		if (ret < 0)
			return -EFAULT;

		if (graphic.address_mode == DP_VIRTUAL_SET) {
			address = d4_uservirt_to_phys(graphic.address);
			graphic.address = address;
		}

		d4_dp_lcd_graphic_set(graphic);
		break;
	}

	case DP_IOCTL_SLCD_GRP_SET: {
		unsigned int address;
		ret = copy_from_user((void *) &graphic, (const void *) arg,
				sizeof(graphic));
		if (ret < 0)
			return -EFAULT;

		if (graphic.address_mode == DP_VIRTUAL_SET) {
			address = d4_uservirt_to_phys(graphic.address);
			graphic.address = address;
		}

		d4_dp_sublcd_graphic_set(graphic);
		break;
	}

	case DP_IOCTL_LCD_GRP_ADDRESS: {
		struct stgrp_address grp;
		ret = copy_from_user((void *) &grp, (const void *) arg,
				sizeof(grp));
		if (ret < 0)
			return -EFAULT;

		if (grp.address_mode == DP_VIRTUAL_SET) {
			grp.address = d4_uservirt_to_phys(grp.address);
		}

		d4_dp_lcd_graphic_address_set(grp);
		break;
	}

	case DP_IOCTL_SLCD_GRP_ADDRESS: {
		struct stgrp_address grp;
		ret = copy_from_user((void *) &grp, (const void *) arg,
				sizeof(grp));
		if (ret < 0)
			return -EFAULT;

		if (grp.address_mode == DP_VIRTUAL_SET) {
			grp.address = d4_uservirt_to_phys(grp.address);
		}

		d4_dp_sublcd_graphic_address_set(grp);
		break;
	}

	case DP_IOCTL_LCD_GRP_WINDOW_ONOFF: {
		ret = copy_from_user((void *) &win_onoff, (const void *) arg,
				sizeof(win_onoff));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("graphic window%d  on/off =%d\n", win_onoff.win, win_onoff.onoff);
		d4_dp_lcd_graphic_window_onoff(win_onoff.win, win_onoff.onoff);
		break;
	}

	case DP_IOCTL_SLCD_GRP_WINDOW_ONOFF: {
		ret = copy_from_user((void *) &win_onoff, (const void *) arg,
				sizeof(win_onoff));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("graphic window%d  on/off =%d\n", win_onoff.win, win_onoff.onoff);
		d4_dp_sublcd_graphic_window_onoff(win_onoff.win, win_onoff.onoff);
		break;
	}

	case DP_IOCTL_LCD_GRP_SCALE: {
		ret
				= copy_from_user((void *) &scale, (const void *) arg,
						sizeof(scale));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("graphic scale =%d\n", scale);
		d4_dp_lcd_graphic_scale(scale);
		break;
	}

	case DP_IOCTL_SLCD_GRP_SCALE: {
	   ret
			   = copy_from_user((void *) &scale, (const void *) arg,
					   sizeof(scale));
	   if (ret < 0)
		   return -EFAULT;

	   DpPRINTK("graphic scale =%d\n", scale);
	   d4_dp_sublcd_graphic_scale(scale);
	   break;
   }

	case DP_IOCTL_LCD_GRP_PRORITY: {
		ret = copy_from_user((void *) &prority, (const void *) arg,
				sizeof(prority));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_graphic_window_priority(&prority);

		break;
	}

	case DP_IOCTL_SLCD_GRP_PRORITY: {
		ret = copy_from_user((void *) &prority, (const void *) arg,
				sizeof(prority));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_graphic_window_priority(&prority);

		break;
	}

	case DP_IOCTL_LCD_BB_TABLE: {
		ret = copy_from_user((void *) &bb_table, (const void *) arg,
				sizeof(bb_table));
		if (ret < 0)
			return -EFAULT;

		rgb_info = bb_table.rgb_info;
		d4_dp_lcd_boundarybox_table_color(bb_table.table, &rgb_info);
		break;
	}

	case DP_IOCTL_SLCD_BB_TABLE: {
		ret = copy_from_user((void *) &bb_table, (const void *) arg,
				sizeof(bb_table));
		if (ret < 0)
			return -EFAULT;

		rgb_info = bb_table.rgb_info;
		d4_dp_sublcd_boundarybox_table_color(bb_table.table, &rgb_info);
		break;
	}

	case DP_IOCTL_LCD_BB_INFO: {
		ret = copy_from_user((void *) &bb_info, (const void *) arg,
				sizeof(bb_info));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_bouddarybox_info_set(&bb_info);
		break;
	}

	case DP_IOCTL_SLCD_BB_INFO: {
		ret = copy_from_user((void *) &bb_info, (const void *) arg,
				sizeof(bb_info));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_bouddarybox_info_set(&bb_info);
		break;
	}

	case DP_IOCTL_LCD_BB_ONOFF: {
		ret = copy_from_user((void *) &bb_onoff, (const void *) arg,
				sizeof(bb_onoff));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_boudarybox_onoff(&bb_onoff);

		break;
	}

	case DP_IOCTL_SLCD_BB_ONOFF: {
		ret = copy_from_user((void *) &bb_onoff, (const void *) arg,
				sizeof(bb_onoff));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_boudarybox_onoff(&bb_onoff);

		break;
	}

	case DP_IOCTL_LCD_FILTER: {
		ret = copy_from_user((void *) &filter_ctrl, (const void *) arg,
				sizeof(filter_ctrl));
		if (ret < 0)
			return -EFAULT;

		if (filter_ctrl.filer_value > DP_BYPASS) {
			printk("ioctl fail: [%d], lcd filter value out of range\n", cmd);
			return -1;
		}
		d4_dp_lcd_filter_onoff(&filter_ctrl);
		break;
	}

	case DP_IOCTL_SLCD_FILTER: {
		ret = copy_from_user((void *) &filter_ctrl, (const void *) arg,
				sizeof(filter_ctrl));
		if (ret < 0)
			return -EFAULT;

		if (filter_ctrl.filer_value > DP_BYPASS) {
			printk("ioctl fail: [%d], lcd filter value out of range\n", cmd);
			return -1;
		}
		d4_dp_sublcd_filter_onoff(&filter_ctrl);
		break;
	}

	case DP_IOCTL_LCD_ZEBRA: {
		ret = copy_from_user((void *) &zebra_set, (const void *) arg,
				sizeof(zebra_set));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_zebra_control(&zebra_set);
		break;
	}

	case DP_IOCTL_SLCD_ZEBRA: {
		ret = copy_from_user((void *) &zebra_set, (const void *) arg,
				sizeof(zebra_set));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_zebra_control(&zebra_set);
		break;
	}

	case DP_IOCTL_LCD_GM: {
		struct stlcd_gm_lcd val;

		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_gm_onoff(val);
		break;
	}

	case DP_IOCTL_LCD_DISPLAY_SWAP: {
		struct stdp_display_swap swap;
		int h, v;
		
		ret = copy_from_user((void *) &swap, (const void *) arg, sizeof(swap));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_flip(swap.layer, swap.direct);
		break;
	}

	case DP_IOCTL_SLCD_DISPLAY_SWAP: {
		struct stdp_display_swap swap;

		ret = copy_from_user((void *) &swap, (const void *) arg, sizeof(swap));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_flip(swap.layer, swap.direct);
		break;
	}

#if 0
	case DP_IOCTL_LCD_INTERRUPT: {
		enum edp_onoff val;

		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;
		DpPRINTK("%s(),%d,val =%d\n", __FUNCTION__, __LINE__, val);
		/*interrupt disable */
		d4_dp_lcd_interrupt_onoff(val);
		break;
	}
#endif
	case DP_IOCTL_LCD_TCC: {
			struct stlcd_tcc tcc;

			ret = copy_from_user((void *) &tcc, (const void *) arg, sizeof(tcc));
			if (ret < 0)
			return -EFAULT;

			d4_dp_lcd_tcc_set(tcc);
			break;
		}

		case DP_IOCTL_SLCD_TCC: {
			struct stlcd_tcc tcc;

			ret = copy_from_user((void *) &tcc, (const void *) arg, sizeof(tcc));
			if (ret < 0)
			return -EFAULT;

			d4_dp_sublcd_tcc_set(tcc);
			break;
		}

		case DP_IOCTL_LCD_LIMIT: {
			ret
			= copy_from_user((void *) &range, (const void *) arg,
					sizeof(range));
			if (ret < 0)
			return -EFAULT;

			d4_dp_lcd_limit_set(range);
			break;
		}

		case DP_IOCTL_SLCD_LIMIT: {
			ret
			= copy_from_user((void *) &range, (const void *) arg,
					sizeof(range));
			if (ret < 0)
			return -EFAULT;

			d4_dp_sublcd_limit_set(range);
			break;
		}

	case DP_IOCTL_LCD_LAYER_CHANGE: {
		ret
				= copy_from_user((void *) &over_layer, (const void *) arg,
						sizeof(over_layer));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_overlayer(over_layer);
		break;
	}

	case DP_IOCTL_SLCD_LAYER_CHANGE: {
		ret
				= copy_from_user((void *) &over_layer, (const void *) arg,
						sizeof(over_layer));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_overlayer(over_layer);
		break;
	}

	case DP_IOCTL_NLC_VIDEO: {
		DpPRINTK("NLC Video Set I/O cntrl \n");
		ret = copy_from_user((void *) &nlc_video, (const void *) arg,
				sizeof(nlc_video));
		if (ret < 0)
			return -EFAULT;

		if (nlc_video.dp_path == DP_MLCD) {
			if (nlc_video.address_mode == DP_VIRTUAL_SET) {
				nlc_video.y_address = d4_uservirt_to_phys(nlc_video.y_address);
				nlc_video.c_address = d4_uservirt_to_phys(nlc_video.c_address);
			} else {
				nlc_vid_Set.address.Addr_Y0 = nlc_video.y_address;
				nlc_vid_Set.address.Addr_C0 = nlc_video.c_address;
			}

			nlc_vid_Set.path = DP_MLCD;
			nlc_vid_Set.format = nlc_video.format;
			nlc_vid_Set.inputImage_height = nlc_video.img_height;
			nlc_vid_Set.inputImage_width = nlc_video.img_width;

			nlc_vid_Set.display.H_Start = nlc_video.H_Start;
			nlc_vid_Set.display.H_Size = nlc_video.H_Size;
			nlc_vid_Set.display.V_Start = nlc_video.V_Start;
			nlc_vid_Set.display.V_Size = nlc_video.V_Size;

			if (nlc_video.nlc == NLC_VID_ON) {
				d4_dp_lcd_video_nlc(nlc_vid_Set, NLC_VID_ON);
			} else if (nlc_video.nlc == NLC_OFF)
				d4_dp_lcd_video_nlc(nlc_vid_Set, NLC_OFF);
		}

		break;
	}
	case DP_IOCTL_NLC_GRAPHIC: {
		struct stgraphic_display_area display;

		DpPRINTK("NLC Video Set I/O cntrl \n");

		ret = copy_from_user((void *) &nlc_grp, (const void *) arg,
				sizeof(nlc_grp));
		if (ret < 0)
			return -EFAULT;

		if (nlc_grp.dp_path == DP_MLCD) {
			nlc_grp_set = nlc_grp.nlcset;
			display.win = nlc_grp_set.display_win;
			display.display = nlc_grp_set.display;

			if (nlc_grp_set.image_width < nlc_grp_set.display.H_Size)
				nlc_grp_set.display.H_Size = nlc_grp_set.image_width;
			if (nlc_grp_set.image_height < nlc_grp_set.image_height)
				nlc_grp_set.display.V_Size = nlc_grp_set.image_height;

			if (nlc_grp.onoff == NLC_GRP_ON)
				d4_dp_lcd_graphic_nlc(nlc_grp_set, NLC_GRP_ON);
			else if (nlc_grp.onoff == NLC_OFF)
				d4_dp_lcd_graphic_nlc(nlc_grp_set, NLC_OFF);
		}

		break;
	}

	case DP_IOCTL_LCD_GRP_ALPHA: {
		ret
				= copy_from_user((void *) &alpha, (const void *) arg,
						sizeof(alpha));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_graphic_window_alpha(alpha);
		break;
	}

	case DP_IOCTL_SLCD_GRP_ALPHA: {
		ret
				= copy_from_user((void *) &alpha, (const void *) arg,
						sizeof(alpha));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_graphic_window_alpha(alpha);
		break;
	}

	case DP_IOCTL_BT656_SET: {
		enum edp_onoff val;

		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;
		d4_dp_bt656_onoff(val);
		break;
	}
	case DP_IOCTL_LCD_GRP_ORDER_SET: {
		enum edp_onoff val;

		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_graphic_order_change(val);
		break;
	}

#if 0
	case DP_IOCTL_PANNEL_CTRL: {
		enum edp_onoff val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;
		d4_dp_lcd_pannel_ctrl(val);
		 break;
	 }
#endif
	default:
		retval = -ENOIOCTLCMD;
		break;
	}

	return retval;
}

struct fb_ops drime4_fb_ops = { .owner = THIS_MODULE,
		.fb_release = drime4_fb_release,
		.fb_check_var = drime4_fb_lcd_check_var,
		.fb_set_par = drime4_fb_lcd_set_par, .fb_blank = drime4_fb_lcd_blank,
		.fb_setcolreg = drime4_fb_lcd_setcolreg,
		.fb_pan_display = drime4_fb_lcd_pan_display,
		.fb_ioctl = drime4_fb_lcd_ioctl, };

