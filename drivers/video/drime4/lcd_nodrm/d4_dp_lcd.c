#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/completion.h>
#include <linux/fb.h>
#include <linux/pinctrl/pinmux.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <mach/d4_cma.h>

#include <mach/dp/d4_dp.h>
#include "d4_dp_lcd_dd.h"

extern struct stfb_info lcd_info;
static struct drime4_fb global_fb;
static struct drime4_fb_window global_win[4];
extern struct fb_ops drime4_fb_ops;
struct stfb_get_info getinfo;

int d4_set_lcd_pannel_info_set(struct d4_lcd *pannel)
{
	int ret = 0;
	if (pannel == NULL)
		ret = -1;
	else
		global_fb.lcd = pannel;

	return ret;
}

int d4_set_sublcd_pannel_info_set(struct d4_lcd *pannel)
{
	int ret = 0;
	if (pannel == NULL) {
		ret = -1;
	} else
		global_fb.sublcd = pannel;

	return ret;
}
struct drime4_platform_fb *to_fb_plat_lcd(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	return (struct drime4_platform_fb *) pdev->dev.platform_data;
}

static int drime4_init_fbinfo(struct platform_device *pdev,
		struct drime4_fb *drime4, int id, char sel_grp)
{
	struct fb_info *fb = drime4->fb[id];
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct fb_var_screeninfo *var = &fb->var;
	struct drime4_fb_window *win = fb->par;
	struct d4_lcd *lcd = drime4->lcd;
	struct lcd_tg *timing = &lcd->timing;
	struct drime4_platform_fb *pdata = to_fb_plat_lcd(&pdev->dev);
	int ret = -1;

	platform_set_drvdata(to_platform_device(&pdev->dev), fb);
	strcpy(fix->id, "drime4_fb");

	/* specific setting. */
	win->id = id;

	/* framebuffer info. */
	fb->fbops = &drime4_fb_ops;

	fb->flags = FBINFO_FLAG_DEFAULT;
	fb->pseudo_palette = &win->pseudo_pal;

	fix->xpanstep = 0;
	fix->ypanstep = 1;
	fix->type = FB_TYPE_PACKED_PIXELS;
	fix->accel = FB_ACCEL_NONE;
	fix->visual = FB_VISUAL_TRUECOLOR;

	var->xres = global_fb.lcd->h_size;
	var->yres = global_fb.lcd->v_size;
	var->xoffset = 0;
	var->yoffset = (pdata->buffer_cnt)
			* pdata->layer.vid_win[id].vid_image.img_height;

	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres;

	var->bits_per_pixel = 32;
	var->transp.length = 0;
	var->red.offset = 16;
	var->red.length = 8;
	var->green.offset = 8;
	var->green.length = 8;
	var->blue.offset = 0;
	var->blue.length = 8;
	var->transp.offset = 24;
	var->transp.length = 0;

	if (sel_grp)
		fix->line_length = pdata->layer.graphic_stride;
	else
		fix->line_length = pdata->layer.video_stride;

	/* fix memory set */
	if (sel_grp == 1)
		fix->smem_len = fix->line_length * pdata->layer.grp_win[id
				- pdata->vid_wins].vid_image.img_height * (pdata->buffer_cnt
				+ 1);
	else
		fix->smem_len = fix->line_length
				* pdata->layer.vid_win[id].vid_image.img_height
				* (pdata->buffer_cnt + 1);

	var->nonstd = 0;
	var->activate = FB_ACTIVATE_NOW;
	var->vmode = FB_VMODE_NONINTERLACED;

	var->hsync_len = timing->h_sync_fall;
	var->vsync_len = timing->v_sync_fall;

	/* left margin =active pixel h start - h sync , not HBP*/
	var->left_margin = timing->enable_h_start - timing->h_sync_fall;
	var->right_margin = timing->total_h_size - timing->enable_h_end;

	/* uper margin =active pixel v start - v sync , not VBP */
	var->upper_margin = timing->enable_v_start - timing->v_sync_fall;
	var->lower_margin = timing->total_v_size - timing->enable_v_end;

	var->pixclock = lcd->freq * (var->left_margin + var->right_margin
			+ var->hsync_len + var->xres) * (var->upper_margin
			+ var->lower_margin + var->vsync_len + var->yres);

	ret = fb_alloc_cmap(&fb->cmap, 1, 1);

	if (ret == 0)
		fb_set_cmap(&fb->cmap, fb);
	else {
		dev_err(fb->dev, "failed to allocate fb cmap.\n");
		return ret;
	}

	return 0;
}


static int drime4_alloc_framebuffer_free(struct drime4_fb *drime4)
{
	unsigned short i;
	struct fb_info *fb;

	for (i = 0; i < 8; i++) {
		fb = drime4->fb[i];
		kfree(fb);
	}
	kfree(drime4->fb);
	return 0;
}

static int drime4_alloc_framebuffer(struct platform_device *pdev,
		struct drime4_fb *drime4)
{
	struct drime4_platform_fb *pdata = to_fb_plat_lcd(&pdev->dev);
	int ret = -1, i, j, common_win_id = 0;
	struct drime4_fb_window *win = NULL;
	struct drime4_fb_video_window *vid_win = NULL;
	struct drime4_fb_graphic_window *grp_win = NULL;
	struct drime4_fb_layer *fb_layer = NULL;

	/* allocate memory regions for framebuffers. */
	drime4->fb = kmalloc(drime4->max_wins * sizeof(struct fb_info *),
			GFP_KERNEL);

	if (!drime4->fb) {
		dev_err(&pdev->dev, "not enough memory.\n");
		ret = -ENOMEM;
		goto err_alloc;
	};

	/* allocate framebuffer for video layer. */
	for (j = 0; j < pdata->vid_wins; j++) {
		drime4->fb[common_win_id] = framebuffer_alloc(
				sizeof(struct drime4_fb_window), &pdev->dev);

		if (!drime4->fb[common_win_id]) {
			dev_err(&pdev->dev, "not enough memory.\n");
			ret = -ENOMEM;
			goto err_alloc_fb;
		}

		win = (struct drime4_fb_window *) drime4->fb[common_win_id]->par;
		WARN_ON(win == NULL);

		vid_win = &pdata->layer.vid_win[j];
		fb_layer = &pdata->layer;
		WARN_ON(vid_win == NULL);

		global_win[common_win_id].id = common_win_id;

		/* get yuv format for video layer. */
		global_win[common_win_id].yuv_format = win->yuv_format
				= vid_win->format;

		/* get stride size. */
		global_win[common_win_id].video_stride = win->video_stride
				= fb_layer->video_stride;

		/* get real image size  */
		global_win[common_win_id].vid_image.img_height
				= win->vid_image.img_height = vid_win->vid_image.img_height;

		global_win[common_win_id].vid_image.img_width
				= win->vid_image.img_width = vid_win->vid_image.img_width;

		/*get image area */
		global_win[common_win_id].vid_display.Display_H_Start
				= win->vid_display.Display_H_Start
						= vid_win->vid_display.Display_H_Start;

		global_win[common_win_id].vid_display.Display_H_Size
				= win->vid_display.Display_H_Size
						= vid_win->vid_display.Display_H_Size;

		global_win[common_win_id].vid_display.Display_V_Start
				= win->vid_display.Display_V_Start
						= vid_win->vid_display.Display_V_Start;

		global_win[common_win_id].vid_display.Display_V_Size
				= win->vid_display.Display_V_Size
						= vid_win->vid_display.Display_V_Size;

		/* video bit 8bit */
		global_win[common_win_id].vid_bit = vid8bit;

		ret = drime4_init_fbinfo(pdev, drime4, common_win_id, 0);
		if (ret) {
			dev_err(&pdev->dev, "failed to allocate memory \n");
			ret = -ENOMEM;
			goto err_alloc_fb;
		}

		common_win_id++;
	}

	/* graphic layer frame allcation */
	for (j = 0; j < pdata->grp_wins; j++) {

		drime4->fb[common_win_id] = framebuffer_alloc(
				sizeof(struct drime4_fb_window), &pdev->dev);

		if (!drime4->fb[common_win_id]) {
			dev_err(&pdev->dev, "not enough memory.\n");
			ret = -ENOMEM;
			goto err_alloc_fb;
		}

		win = (struct drime4_fb_window *) drime4->fb[common_win_id]->par;
		WARN_ON(win == NULL);

		grp_win = &pdata->layer.grp_win[j];
		fb_layer = &pdata->layer;
		WARN_ON(grp_win == NULL);

		/* get stride size. */
		global_win[common_win_id - pdata->vid_wins].graphic_stride
				= win->graphic_stride = fb_layer->graphic_stride;

		/* get graphic scale */
		global_win[common_win_id - pdata->vid_wins].graphic_scale
				= win->graphic_scale = fb_layer->graphic_scale;

		/* get real image size  */
		global_win[common_win_id - pdata->vid_wins].grp_image.img_height
				= win->grp_image.img_height = grp_win->vid_image.img_height;
		global_win[common_win_id - pdata->vid_wins].grp_image.img_width
				= win->grp_image.img_width = grp_win->vid_image.img_width;

		/*get image area */
		global_win[common_win_id - pdata->vid_wins].grp_display.Display_H_Start
				= win->grp_display.Display_H_Start
						= grp_win->vid_display.Display_H_Start;
		global_win[common_win_id - pdata->vid_wins].grp_display.Display_H_Size
				= win->grp_display.Display_H_Size
						= grp_win->vid_display.Display_H_Size;
		global_win[common_win_id - pdata->vid_wins].grp_display.Display_V_Start
				= win->grp_display.Display_V_Start
						= grp_win->vid_display.Display_V_Start;
		global_win[common_win_id - pdata->vid_wins].grp_display.Display_V_Size
				= win->grp_display.Display_V_Size
						= grp_win->vid_display.Display_V_Size;

		ret = drime4_init_fbinfo(pdev, drime4, common_win_id, 1);

		if (ret) {
			dev_err(&pdev->dev, "failed to allocate memory\n");
			ret = -ENOMEM;
			goto err_alloc_fb;
		}

		common_win_id++;
	}

	return 0;

	err_alloc_fb: for (i = 0; i < drime4->max_wins; i++)
		if (drime4->fb[i])
			framebuffer_release(drime4->fb[i]);

	kfree(drime4->fb);

	err_alloc: return ret;
}

static int drime4_lcd_fb_map_memory(struct drime4_fb *drime4, int id)
{
	struct fb_info *fb = drime4->fb[id];
	struct fb_fix_screeninfo *fix = &fb->fix;

	fix->smem_start = d4_cma_alloc(global_fb.dev, fix->smem_len);
	fb->screen_base = phys_to_virt(fix->smem_start);

	return fb->screen_base ? 0 : -ENOMEM;

}

static void drime4_lcd_fb_map_memory_free(struct platform_device *pdev,
		struct drime4_fb *drime4, int id)
{
	struct fb_info *fb = drime4->fb[id];
	struct fb_fix_screeninfo *fix = &fb->fix;

	d4_cma_free(global_fb.dev, fix->smem_start);
}

static irqreturn_t drime4_lcd_irq_frame(int irq, void *dev_id)
{
	struct drime4_fb *drime4 = (struct drime4_fb *) dev_id;

	disable_irq_nosync(irq);
	wake_up(&drime4->wq);

	/* lcd frame interrupt */
	if (dp_interrupt_check() < 0)
		printk("isr not work\n");

	enable_irq(irq);
	return IRQ_HANDLED;
}

/* for gpu (bh1212.ahn) */
int gpu_get_dp_info(unsigned int cmd, enum edp_window window, unsigned long arg)
{
	struct fb_info *fb = global_fb.fb[window];
	struct stfb_lcd_pannel pannel = getinfo.pannel;
	void *argp = (void *) arg;

	int ret = 0;

	switch (cmd) {

	case 0: /* GET_FSCREENINFO */
		ret = memcpy(argp, &fb->fix, sizeof(fb->fix)) ? 0 : -EFAULT;
		break;

	case 1: /* GET_VSCREENINFO */
		ret = memcpy(argp, &fb->var, sizeof(fb->var)) ? 0 : -EFAULT;
		break;

	case 2: /* SET_GRP_WINDOW */{
		struct stgrpdisplay graphic;

		ret = memcpy(&graphic, argp, sizeof(graphic)) ? 0 : -EFAULT;

		printk("win %d, stride %d, addr 0x%x\n", graphic.win, graphic.stride,
				graphic.address);
		d4_dp_lcd_graphic_set(graphic);
		d4_dp_lcd_graphic_window_onoff(window, 1);
		break;
	}

	case 3: /* GET_PANNEL_INFO */
		ret = memcpy(argp, &pannel, sizeof(pannel)) ? 0 : -EFAULT;
		break;

	default:
		ret = -ENOIOCTLCMD;
		break;
	}

	return ret;
}
EXPORT_SYMBOL(gpu_get_dp_info);

static int __devinit drime4_fb_lcd_probe(struct platform_device *pdev)
{
	struct drime4_platform_fb *pdata = NULL;
	struct resource *res;
	int ret = -1, i = 0;

	global_fb.dev = &pdev->dev;

	/**< DP Clock Enable */
	global_fb.dp_clk = clk_get(&pdev->dev, "dp");
	clk_enable(global_fb.dp_clk);

	global_fb.mlcd_clk = clk_get(NULL, "mlcd");
	global_fb.mlcd_out_clk = clk_get(NULL, "mlcd_out");
	global_fb.mlcd_sel = clk_get(NULL, "mlcd_sel");
	global_fb.mlcd_sel1 = clk_get(NULL, "mlcd_sel1");

	clk_set_parent(global_fb.mlcd_sel, global_fb.mlcd_sel1);
	clk_set_parent(global_fb.mlcd_clk, global_fb.mlcd_sel);
	clk_set_parent(global_fb.mlcd_out_clk, global_fb.mlcd_sel);

	clk_enable(global_fb.mlcd_clk);
	clk_enable(global_fb.mlcd_out_clk);

	clk_set_rate(global_fb.dp_clk, 133000000);
	clk_set_rate(global_fb.mlcd_sel1, 27000000);

	global_fb.slcd_clk = clk_get(NULL, "slcd");
	global_fb.slcd_out_clk = clk_get(NULL, "slcd_out");
	global_fb.slcd_sel = clk_get(NULL, "slcd_sel");
	global_fb.slcd_sel1 = clk_get(NULL, "slcd_sel1");

	clk_set_parent(global_fb.slcd_sel, global_fb.slcd_sel1);
	clk_set_parent(global_fb.slcd_clk, global_fb.slcd_sel);
	clk_set_parent(global_fb.slcd_out_clk, global_fb.slcd_sel);

	clk_enable(global_fb.slcd_clk);
	clk_enable(global_fb.slcd_out_clk);
	clk_set_rate(global_fb.slcd_sel1, 27000000);

	/* drime4_platform_fb : platform data get */
	global_fb.platform_data = to_fb_plat_lcd(&pdev->dev);
	pdata = global_fb.platform_data;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region.\n");
		ret = -EINVAL;
		goto err_io;
	}

	/* ioremap for register regions. */
	global_fb.regs_global = ioremap(res->start, res->end - res->start + 1);
	if (!global_fb.regs_global) {
		ret = -EINVAL;
		dev_err(&pdev->dev, "failed to remap io region.\n");
		goto err_io;
	}

	global_fb.regs_tv = global_fb.regs_global + DP_TV_BASE;
	global_fb.regs_lcd = global_fb.regs_global + DP_LCD_BASE;

	/* pannel information get */
	global_fb.irq = platform_get_irq(pdev, 0);

	ret = request_irq(global_fb.irq, drime4_lcd_irq_frame, IRQF_DISABLED,
			pdev->name, &global_fb);
	if (ret) {
		dev_err(&pdev->dev, "irq request failed.\n");
		goto err_irq;
	}

	/* init wait queue for vsync interrupt */
	init_waitqueue_head(&global_fb.wq);

	/* get window count. video window 4, graphic window 4*/
	global_fb.max_wins = pdata->vid_wins + pdata->grp_wins;

	/* allocate fb_info region for framebuffer. */
	ret = drime4_alloc_framebuffer(pdev, &global_fb);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to allocate fb_info regions.\n");
		goto err_irq;
	}

	/* Initialize video memory */
	for (i = 0; i < (pdata->vid_wins + pdata->grp_wins); i++) {
		ret = drime4_lcd_fb_map_memory(&global_fb, i);
		if (ret) {
			dev_err(&pdev->dev, "failed to allocate video RAM.\n");
			return -EINVAL;
		}
	}

	/* video framebuffer register set */
	for (i = 0; i < pdata->vid_wins; i++) {
		/* register windows to common framebuffer. */
		ret = register_framebuffer(global_fb.fb[i]);
		if (ret) {
			dev_err(&pdev->dev, "failed to register fb device.\n");
			return -EINVAL;
		}
	}

	/* graphic framebuffer register set */
	for (i = 0; i < pdata->grp_wins; i++) {
		/* register windows to common framebuffer. */
		ret = register_framebuffer(global_fb.fb[i + pdata->vid_wins]);
		if (ret) {
			dev_err(&pdev->dev, "failed to register fb device.\n");
			return -EINVAL;
		}
	}

	/* translate dp virtual address, video/graphic information,pannel */
	getinfo.dp_global = (unsigned int) global_fb.regs_global;
	getinfo.dp_tv = (unsigned int) global_fb.regs_tv;
	getinfo.dp_lcd = (unsigned int) global_fb.regs_lcd;

	getinfo.video.win = 0;
	getinfo.video.format = global_win[0].yuv_format;
	getinfo.video.bit = global_win[0].vid_bit;
	getinfo.video.vid_stride = global_win[0].video_stride;
	getinfo.video.image.image_width = global_win[0].vid_image.img_width;
	getinfo.video.image.image_height = global_win[0].vid_image.img_height;

	getinfo.video.display.H_Start = global_win[0].vid_display.Display_H_Start;
	getinfo.video.display.H_Size = global_fb.lcd->h_size;
	getinfo.video.display.V_Start = global_win[0].vid_display.Display_V_Start;
	getinfo.video.display.V_Size = global_fb.lcd->v_size;
	getinfo.video.address.y0_address = 0;
	getinfo.video.address.c0_address = 0;

	getinfo.graphic.win = 0;
	getinfo.graphic.grp_stride = global_win[0].graphic_stride;
	getinfo.graphic.image.image_width = global_win[0].grp_image.img_width;
	getinfo.graphic.image.image_height = global_win[0].grp_image.img_height;
	getinfo.graphic.display.H_Start = global_win[0].grp_display.Display_H_Start;
	getinfo.graphic.display.H_Size = global_win[0].grp_display.Display_H_Size;
	getinfo.graphic.display.V_Start = global_win[0].grp_display.Display_V_Start;
	getinfo.graphic.display.V_Size = global_win[0].grp_display.Display_V_Size;
	getinfo.graphic.address = global_win[0].grp_addr;

	getinfo.pannel.h_size = global_fb.lcd->h_size;
	getinfo.pannel.v_size = global_fb.lcd->v_size;
	getinfo.pannel.freq = global_fb.lcd->freq;
	getinfo.pannel.lcd_data_width = global_fb.lcd->lcd_data_width;
	getinfo.pannel.type = global_fb.lcd->type;
	getinfo.pannel.even_seq = global_fb.lcd->even_seq;
	getinfo.pannel.odd_seq = global_fb.lcd->odd_seq;
	getinfo.pannel.timing.total_h_size = global_fb.lcd->timing.total_h_size;
	getinfo.pannel.timing.total_v_size = global_fb.lcd->timing.total_v_size;
	getinfo.pannel.timing.h_sync_fall = global_fb.lcd->timing.h_sync_fall;
	getinfo.pannel.timing.h_sync_rise = global_fb.lcd->timing.h_sync_rise;
	getinfo.pannel.timing.v_sync_rise = global_fb.lcd->timing.v_sync_rise;
	getinfo.pannel.timing.v_sync_fall = global_fb.lcd->timing.v_sync_fall;
	getinfo.pannel.timing.buf_read_h_start
			= global_fb.lcd->timing.buf_read_h_start;
	getinfo.pannel.timing.enable_h_start = global_fb.lcd->timing.enable_h_start;
	getinfo.pannel.timing.enable_h_end = global_fb.lcd->timing.enable_h_end;
	getinfo.pannel.timing.enable_v_start = global_fb.lcd->timing.enable_v_start;
	getinfo.pannel.timing.enable_v_end = global_fb.lcd->timing.enable_v_end;
	getinfo.pannel.timing.inv_dot_clk = global_fb.lcd->timing.inv_dot_clk;
	getinfo.pannel.timing.inv_enable_clk = global_fb.lcd->timing.inv_enable_clk;
	getinfo.pannel.timing.inv_h_sync = global_fb.lcd->timing.inv_h_sync;
	getinfo.pannel.timing.inv_v_sync = global_fb.lcd->timing.inv_v_sync;

	if (global_fb.sublcd != NULL) {
		getinfo.sub_pannel.h_size = global_fb.sublcd->h_size;
		getinfo.sub_pannel.v_size = global_fb.sublcd->v_size;
		getinfo.sub_pannel.freq = global_fb.sublcd->freq;
		getinfo.sub_pannel.lcd_data_width = global_fb.sublcd->lcd_data_width;
		getinfo.sub_pannel.type = global_fb.sublcd->type;
		getinfo.sub_pannel.even_seq = global_fb.sublcd->even_seq;
		getinfo.sub_pannel.odd_seq = global_fb.sublcd->odd_seq;
		getinfo.sub_pannel.timing.total_h_size
		= global_fb.sublcd->timing.total_h_size;
		getinfo.sub_pannel.timing.total_v_size
		= global_fb.sublcd->timing.total_v_size;
		getinfo.sub_pannel.timing.h_sync_fall
		= global_fb.sublcd->timing.h_sync_fall;
		getinfo.sub_pannel.timing.h_sync_rise
		= global_fb.sublcd->timing.h_sync_rise;
		getinfo.sub_pannel.timing.v_sync_rise
		= global_fb.sublcd->timing.v_sync_rise;
		getinfo.sub_pannel.timing.v_sync_fall
		= global_fb.sublcd->timing.v_sync_fall;
		getinfo.sub_pannel.timing.buf_read_h_start
		= global_fb.sublcd->timing.buf_read_h_start;
		getinfo.sub_pannel.timing.enable_h_start
		= global_fb.sublcd->timing.enable_h_start;
		getinfo.sub_pannel.timing.enable_h_end
		= global_fb.sublcd->timing.enable_h_end;
		getinfo.sub_pannel.timing.enable_v_start
		= global_fb.sublcd->timing.enable_v_start;
		getinfo.sub_pannel.timing.enable_v_end
		= global_fb.sublcd->timing.enable_v_end;
		getinfo.sub_pannel.timing.inv_dot_clk
		= global_fb.sublcd->timing.inv_dot_clk;
		getinfo.sub_pannel.timing.inv_enable_clk
		= global_fb.sublcd->timing.inv_enable_clk;
		getinfo.sub_pannel.timing.inv_h_sync
		= global_fb.sublcd->timing.inv_h_sync;
		getinfo.sub_pannel.timing.inv_v_sync
		= global_fb.sublcd->timing.inv_v_sync;
	}

	dp_lcd_set_info(&getinfo);

	/* initialize dp lcd hardware. */
	drime4_lcd_display_init();
	dp_lcd_pannel_set();
	d4_lcd_video_init_display();

	if (global_fb.sublcd != NULL) {
		drime4_sublcd_display_init();
		dp_sublcd_pannel_set();
		d4_sublcd_video_init_display();
	}
#ifdef CONFIG_DRIME4_GPU
	extern int gpu_init_flag;
	extern int fb_init_flag;
	fb_init_flag = 1;
	if (gpu_init_flag == 1)	{
		extern int D4dpLcdBridgeInit(void);
		D4dpLcdBridgeInit();
	}
#endif
	return 0;

	err_irq: iounmap(global_fb.regs_global);
	err_io:
	return ret;
}
static int drime4_fb_remove(struct platform_device *pdev)
{
	unsigned char i = 0;

	d4_dp_lcd_off();

	for (i = 0; i < global_fb.max_wins; i++) {
		drime4_lcd_fb_map_memory_free(pdev, &global_fb, i);
	}

	drime4_alloc_framebuffer_free(&global_fb);

	/**< DP Clock Disable */
	clk_disable(global_fb.dp_clk);
	clk_put(global_fb.dp_clk);

	clk_disable(global_fb.mlcd_clk);
	clk_put(global_fb.mlcd_clk);

	clk_disable(global_fb.mlcd_out_clk);
	clk_put(global_fb.mlcd_out_clk);

	free_irq(global_fb.irq, NULL);
	return 0;
}

#ifdef CONFIG_PM

extern struct stfb_info lcd_info;
static int drime4_fb_lcd_suspend(struct platform_device *pdev, pm_message_t state)
{
	dp_lcd_save_reg(&lcd_info);
	return 0;
}

static int drime4_fb_lcd_resume(struct platform_device *pdev)
{
	drime4_lcd_display_init();
	dp_lcd_pannel_set();
	d4_lcd_video_init_display();
	dp_lcd_restroe_reg(&lcd_info);

	return 0;
}
#else
#define drime4_fb_lcd_suspend NULL
#define drime4_fb_lcd_resume NULL
#endif

static struct platform_driver drime4_fb_driver = {
		.probe = drime4_fb_lcd_probe, .remove = drime4_fb_remove,
		.suspend = drime4_fb_lcd_suspend, .resume = drime4_fb_lcd_resume,
		.driver = {
			.name = FB_MODULE_NAME,
			.owner = THIS_MODULE,
		}, };

static int __init drime4_fb_init(void)
{
	return platform_driver_register(&drime4_fb_driver);
}

static void __exit drime4_fb_cleanup(void)
{
	platform_driver_unregister(&drime4_fb_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(drime4_fb_init);
#else
fast_dev_initcall(drime4_fb_init);
#endif
module_exit(drime4_fb_cleanup);

MODULE_AUTHOR("sejong <sejong55.oh@samsung.com>");
MODULE_LICENSE("GPL");

int drime4_peri_clk_check(struct clk *);

int drime4_check_dp_clk(void)
{
	return drime4_peri_clk_check(global_fb.dp_clk);
}
