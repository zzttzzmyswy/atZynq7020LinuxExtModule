/*
 * @Description: 
 * @Version: 2.0
 * @Autor: ZZT
 * @Date: 2022-01-29 19:16:27
 * @LastEditors: ZZT
 * @LastEditTime: 2022-01-30 19:09:53
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/dma/xilinx_dma.h>
#include <linux/pwm.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_dma.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <video/of_videomode.h>
#include <video/videomode.h>

#include "../drivers/media/platform/xilinx/xilinx-vtc.h"

/* 正点原子LCD屏硬件ID */
#define ATK4342 0 // 4.3寸480*272
#define ATK4384 4 // 4.3寸800*480
#define ATK7084 1 // 7寸800*480
#define ATK7016 2 // 7寸1024*600 自制屏幕
#define ATK1018 5 // 10寸1280*800

/* 自定义结构体用于描述LCD设备 驱动私有结构体*/
struct xilinx_vdmafb_dev {
	struct fb_info *info; // FrameBuffer设备信息
	struct platform_device *pdev; // platform平台设备
	struct clk *pclk; // LCD像素时钟
	struct xvtc_device *vtc; // 时序控制器
	struct dma_chan *vdma; // VDMA通道
	int bl_gpio; // LCD背光引脚
};

static int vdmafb_setcolreg(unsigned regno, unsigned red,
			unsigned green, unsigned blue,
			unsigned transp, struct fb_info *fb_info)
{
	u32 tmp;

	if (regno >= 16)
		return 1;

	red >>= 8; green >>= 8; blue >>= 8;
	tmp = (red << 16) | (green << 8) | blue;
	((u32*)(fb_info->pseudo_palette))[regno] = tmp;

	return 0;
}

static int vdmafb_check_var(struct fb_var_screeninfo *var,
			struct fb_info *fb_info)
{
	struct fb_var_screeninfo *fb_var = &fb_info->var;
	memcpy(var, fb_var, sizeof(struct fb_var_screeninfo));
	return 0;
}


/* Frame Buffer操作函数集 */
static struct fb_ops vdmafb_ops = {
	.owner = THIS_MODULE,
	/* set color register 设置颜色寄存器 */
	.fb_setcolreg	= vdmafb_setcolreg,
	 /* 用于校验可变参数 fb_var_screeninfo */
	.fb_check_var	= vdmafb_check_var, 
	/* 矩形绘制 区域拷贝 图像绘制 使用fb子系统预先设定函数 
	`Generic' versions of the frame buffer device operations
	drivers/video/fbdev/core/cfbfillrect.c
	*/
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

/* 解析设备树获取LCD时序参数 */
static int vdmafb_init_fbinfo_dt(struct xilinx_vdmafb_dev *fbdev,
				 struct videomode *vmode)
{
	struct device *dev = &fbdev->pdev->dev;
	int display_timing;
	int gpios[3];
	int lcd_id = 0;
	int ret;
	int i;

	/* 读取LCD屏ID */
	for (i = 0; i < 3; i++) {
		gpios[i] = of_get_named_gpio(dev->of_node, "lcdID-gpio", i);
		if (!gpio_is_valid(gpios[i])) {
			dev_err(dev, "Failed to get lcd id gpio\n");
			/* 设置为默认LCD屏 */
			lcd_id = ATK7016;
			break;
		}

		ret = devm_gpio_request_one(dev, gpios[i], GPIOF_IN,
					    "lcd hardware ID");
		if (ret < 0) {
			dev_err(dev, "Failed to request lcd id gpio\n");
			/* 设置为默认LCD屏 */
			lcd_id = ATK7016;
			break;
		}

		/* 读取GPIO数值 */
		lcd_id |= (gpio_get_value_cansleep(gpios[i]) << i);
	}

	/* 打印ID值 */
	dev_info(dev, "LCD ID: %d\n", lcd_id);

	/* 再将ID引脚设置为输出模式 */
	msleep(5); // 延时5ms
	for (i = 0; i < 3; i++)
		gpio_direction_output(gpios[i], 0);

	/* 根据LCD ID匹配对应的时序参数 */
	switch (lcd_id) {
	case ATK4342:
		display_timing = 0;
		break;
	case ATK4384:
		display_timing = 1;
		break;
	case ATK7084:
		display_timing = 2;
		break;
	case ATK7016:
		display_timing = 3;
		break;
	case ATK1018:
		display_timing = 4;
		break;
	default:
		/* 默认使用1024x600像素的显示屏 */
		display_timing = 3;
		dev_info(dev,
			 "LCD ID Match failed, using default configuration\n");
		break;
	}

	/* 
	linux fb子系统中实现的获取显示屏时序设定的函数
	参数分别时 设备树节点 videomode结构体指针 时序参数索引
	int of_get_videomode(struct device_node *np, struct videomode *vm,
				int index)
	{
		struct display_timings *disp;
		int ret;

		disp = of_get_display_timings(np);
		if (!disp) {
			pr_err("%pOF: no timings specified\n", np);
			return -EINVAL;
		}

		if (index == OF_USE_NATIVE_MODE)
			index = disp->native_mode;

		ret = videomode_from_timings(disp, vm, index);

		display_timings_release(disp);

		return ret;
	}

	struct videomode {
	unsigned long pixelclock;	// pixelclock in Hz

	u32 hactive;
	u32 hfront_porch;
	u32 hback_porch;
	u32 hsync_len;

	u32 vactive;
	u32 vfront_porch;
	u32 vback_porch;
	u32 vsync_len;

	enum display_flags flags; // display flags 
	};
	*/
	ret = of_get_videomode(dev->of_node, vmode, display_timing);
	if (ret < 0) {
		dev_err(dev, "Failed to get videomode from DT\n");
		return ret;
	}

	return 0;
}

/* 初始化fb_info结构体 */
static int vdmafb_init_fbinfo(struct xilinx_vdmafb_dev *fbdev,
			      struct videomode *vmode)
{
	struct device *dev = &fbdev->pdev->dev;
	struct fb_info *fb_info = fbdev->info;
	struct fb_videomode mode = { 0 };
	dma_addr_t fb_phys; /* 显存物理地址 */
	void *fb_virt; /* 显存虚拟地址 */
	unsigned fb_size; /* 显存大小 */
	int ret;

	/* 解析设备树获取LCD时序参数 */
	ret = vdmafb_init_fbinfo_dt(fbdev, vmode);
	if (ret < 0)
		return ret;

	/* 申请LCD显存 大小为24位x像素
	这里使用dma_alloc_wc创建了dma内存，所以之后需要释放
	*/
	fb_size = vmode->hactive * vmode->vactive * 3;
	fb_virt = dma_alloc_wc(dev, PAGE_ALIGN(fb_size), &fb_phys, GFP_KERNEL);
	if (!fb_virt)
		return -ENOMEM;

	/* 显存清零 */
	memset(fb_virt, 0, fb_size);

	/* 初始化fb_info */
	fb_info->fbops = &vdmafb_ops;
	fb_info->flags = FBINFO_FLAG_DEFAULT;
	fb_info->screen_base = fb_virt;
	fb_info->screen_size = fb_size;

	/* 为`struct fb_fix_screeninfo`赋值 */
	strcpy(fb_info->fix.id, "xlnx");
	fb_info->fix.type = FB_TYPE_PACKED_PIXELS;
	fb_info->fix.visual = FB_VISUAL_TRUECOLOR,
	fb_info->fix.accel = FB_ACCEL_NONE;
	fb_info->fix.line_length = vmode->hactive * 3;
	fb_info->fix.smem_start = fb_phys;
	fb_info->fix.smem_len = fb_size;

	/* 为`struct fb_var_screeninfo var`赋值 */
	fb_info->var.grayscale = 0; /* 彩色 */
	fb_info->var.nonstd = 0; /* 标准像素格式 */
	fb_info->var.activate = FB_ACTIVATE_NOW;
	fb_info->var.accel_flags = FB_ACCEL_NONE;
	fb_info->var.bits_per_pixel = 24; /* 像素深度（bit位） */
	fb_info->var.width = 154; /* LCD屏的物理宽度（单位毫米）*/
	fb_info->var.height = 86; /* LCD屏的物理高度（单位毫米）*/

	fb_info->var.xres = fb_info->var.xres_virtual = vmode->hactive;
	fb_info->var.yres = fb_info->var.yres_virtual = vmode->vactive;
	fb_info->var.xoffset = fb_info->var.yoffset = 0;

	/* 赋值位偏移 */
	fb_info->var.red.offset = 0;
	fb_info->var.red.length = 8;
	fb_info->var.green.offset = 8;
	fb_info->var.green.length = 8;
	fb_info->var.blue.offset = 16;
	fb_info->var.blue.length = 8;
	fb_info->var.transp.offset = 0;
	fb_info->var.transp.length = 0;

	/* 将videomode结构体赋值于fb_videomode 
	此函数需要文件fbmon.c参与编译
	需要宏定义 CONFIG_FB_MODE_HELPERS
	Symbol: FB_MODE_HELPERS [=n]                                                                                         │  
	│ Type  : bool                                                                                                         │  
	│ Defined at drivers/video/fbdev/Kconfig:192                                                                           │  
	│   Prompt: Enable Video Mode Handling Helpers                                                                         │  
	│   Depends on: HAS_IOMEM [=y] && FB [=y]                                                                              │  
	│   Location:                                                                                                          │  
	│     -> Device Drivers                                                                                                │  
	│       -> Graphics support                                                                                            │  
	│         -> Frame buffer Devices                                                                                      │  
	│ (1)       -> Support for frame buffer devices (FB [=y])     
	│              Enable Video Mode Handling Helpers
	*/
	fb_videomode_from_videomode(vmode, &mode);

	/* 将fb_videomode结构体赋值于fb_info->var */
	/* 
	void fb_videomode_to_var(struct fb_var_screeninfo *var,
			 const struct fb_videomode *mode)
	{
		var->xres = mode->xres;
		var->yres = mode->yres;
		var->xres_virtual = mode->xres;
		var->yres_virtual = mode->yres;
		var->xoffset = 0;
		var->yoffset = 0;
		var->pixclock = mode->pixclock;
		var->left_margin = mode->left_margin;
		var->right_margin = mode->right_margin;
		var->upper_margin = mode->upper_margin;
		var->lower_margin = mode->lower_margin;
		var->hsync_len = mode->hsync_len;
		var->vsync_len = mode->vsync_len;
		var->sync = mode->sync;
		var->vmode = mode->vmode & FB_VMODE_MASK;
	}
	*/
	fb_videomode_to_var(&fb_info->var, &mode);

	return 0;
}

static struct dma_interleaved_template dma_template = {0};

/* 初始化VDMA */
static int vdmafb_init_vdma(struct xilinx_vdmafb_dev *fbdev)
{
	struct device *dev = &fbdev->pdev->dev;
	struct fb_info *info = fbdev->info;
	
	struct dma_async_tx_descriptor *tx_desc;
	struct xilinx_vdma_config vdma_config = {0};

	/* 申请VDMA通道 */
	fbdev->vdma = of_dma_request_slave_channel(dev->of_node, "lcd_vdma");
	if (IS_ERR(fbdev->vdma)) {
		dev_err(dev, "Failed to request vdma channel\n");
		//return PTR_ERR(fbdev->vdma);
		return -1;
	}

	/* 终止VDMA通道数据传输 */
	dmaengine_terminate_all(fbdev->vdma);

	/* 初始化VDMA通道 */
	dma_template.dir         = DMA_MEM_TO_DEV;
	dma_template.numf        = info->var.yres;
	dma_template.sgl[0].size = info->fix.line_length;
	dma_template.frame_size  = 1;
	dma_template.sgl[0].icg  = 0;
	dma_template.src_start   = info->fix.smem_start;	// 物理地址
	dma_template.src_sgl     = 1;
	dma_template.src_inc     = 1;
	dma_template.dst_inc     = 0;
	dma_template.dst_sgl     = 0;

	tx_desc = dmaengine_prep_interleaved_dma(fbdev->vdma, &dma_template,
			DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
	if (!tx_desc) {
		dev_err(dev, "Failed to prepare DMA descriptor\n");
		dma_release_channel(fbdev->vdma);
		return -1;
	}

	vdma_config.park = 1;
	xilinx_vdma_channel_set_config(fbdev->vdma, &vdma_config);

	/* 启动VDMA通道数据传输 */
	dmaengine_submit(tx_desc);
	dma_async_issue_pending(fbdev->vdma);

	return 0;
}

/* 初始化LCD时序控制器vtc */
static int vdmafb_init_vtc(struct xilinx_vdmafb_dev *fbdev,
			   struct videomode *vmode)
{
	struct device *dev = &fbdev->pdev->dev;
	struct xvtc_config config;

	/* 解析设备树得到vtc节点 
	新版本的vtc驱动能够自己解析节点
	*/
	// node = of_parse_phandle(dev->of_node, "vtc", 0);
	// if (!node) {
	// 	dev_err(dev, "Failed to parse VTC phandle\n");
	// 	return -ENODEV;
	// }

	/* 获取vtc */
	fbdev->vtc = xvtc_of_get(dev->of_node);
	if (IS_ERR(fbdev->vtc)) {
		dev_err(dev, "Failed to probe VTC\n");
		return PTR_ERR(fbdev->vtc);
	}
	
	/* 
	htotal = vm->hactive + vm->hfront_porch + vm->hsync_len +
		vm->hback_porch;
	vtotal = vm->vactive + vm->vfront_porch + vm->vsync_len +
		vm->vback_porch;

	hactive = vm->hactive;
	vactive = vm->vactive;

	hsync_start = vm->hactive + vm->hfront_porch;
	vsync_start = vm->vactive + vm->vfront_porch;

	hbackporch_start = hsync_start + vm->hsync_len;
	vbackporch_start = vsync_start + vm->vsync_len;
	*/
	config.hblank_start = vmode->hactive;
	config.hsync_start = vmode->hactive + vmode->hsync_len;
	config.hsync_end = vmode->hactive + vmode->hsync_len + 
		vmode->hfront_porch;
	config.hsize = vmode->hactive + vmode->hfront_porch + 
		vmode->hsync_len + vmode->hback_porch;
	config.vblank_start = vmode->vactive;
	config.vsync_start = vmode->vactive + vmode->vsync_len;
	config.vsync_end = vmode->vactive + vmode->vsync_len + 
		vmode->vfront_porch;
	config.vsize = vmode->vactive + vmode->vfront_porch + 
		vmode->vsync_len + vmode->vback_porch;
	config.fps = (u32)vmode->pixelclock / (config.hsize * config.vsize);
	//config.fps = 60;

	xvtc_generator_stop(fbdev->vtc);

	return xvtc_generator_start(fbdev->vtc,&config);
	//xilinx_vtc_reset(fbdev->vtc); /* 复位vtc */
	//xilinx_vtc_disable(fbdev->vtc); /* 禁止vtc */
	//xilinx_vtc_config_sig(fbdev->vtc, vmode); /* 配置vtc时序参数 */
	//xilinx_vtc_enable(fbdev->vtc); /* 使能vtc */

	//return 0;
}

static int vdmafb_probe(struct platform_device *pdev)
{
	struct xilinx_vdmafb_dev *ddev;
	struct fb_info *info;
	struct videomode vmode;
	struct pwm_device *pwm;
	int ret;

	/* 实例化一个fb_info结构体对象 */
	info = framebuffer_alloc(sizeof(struct xilinx_vdmafb_dev), &pdev->dev);
	if (!info) {
		dev_err(&pdev->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	/* 将设备私有结构体与平台驱动结构体和fb_info相关联 */
	ddev = info->par;
	ddev->info = info;
	ddev->pdev = pdev;
	platform_set_drvdata(pdev, ddev);

	/* 获取LCD所需的像素时钟 */
	ddev->pclk = devm_clk_get(&pdev->dev, "lcd_pclk");
	if (IS_ERR(ddev->pclk)) {
		dev_err(&pdev->dev, "Failed to get pixel clock\n");
		ret = PTR_ERR(ddev->pclk);
		goto out1;
	}

	/* 禁止时钟输出 */
	clk_disable_unprepare(ddev->pclk);

	/* 初始化info变量 */
	ret = vdmafb_init_fbinfo(ddev, &vmode);
	if (ret)
		goto out1;

	ret = fb_alloc_cmap(&info->cmap, 256, 0);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to allocate color map\n");
		goto out2;
	}

	info->pseudo_palette =
		devm_kzalloc(&pdev->dev, sizeof(u32) * 16, GFP_KERNEL);
	if (!info->pseudo_palette) {
		ret = -ENOMEM;
		goto out3;
	}

	/* 设置LCD像素时钟、使能时钟 */
	clk_set_rate(ddev->pclk, PICOS2KHZ(info->var.pixclock) * 1000);
	clk_prepare_enable(ddev->pclk);
	msleep(5);

	/* 初始化LCD时序控制器vtc */
	ret = vdmafb_init_vtc(ddev, &vmode);
	if (ret)
		goto out4;

	/* 初始化LCD VDMA */
	ret = vdmafb_init_vdma(ddev);
	if (ret)
		goto out5;

	/* 注册FrameBuffer设备 */
	ret = register_framebuffer(info);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register framebuffer device\n");
		goto out6;
	}

	/* 打开LCD背光 */
	/* 打开LCD背光 */
	pwm = devm_pwm_get(&pdev->dev, NULL);
	pwm_config(pwm, 5000000, 5000000);	// 配置PWM
	pwm_enable(pwm);		// 使能PWM打开背光
	pwm_free(pwm);
	/* 全部注释掉 TODO
	ddev->bl_gpio = of_get_named_gpio(pdev->dev.of_node, "bl-gpio", 0);
	if (!gpio_is_valid(ddev->bl_gpio)) {
		dev_err(&pdev->dev, "Failed to get lcd backlight gpio\n");
		ret = ddev->bl_gpio;
		goto out7;
	}

	ret = devm_gpio_request_one(&pdev->dev, ddev->bl_gpio,
				GPIOF_INIT_HIGH, "lcd backlight");
	if (ret < 0)
		goto out7;
	*/

	platform_set_drvdata(pdev, ddev);
	dev_dbg(&(pdev->dev),"vdmafb probe\r\n");
	return 0;

out7:
	/* 卸载FrameBuffer设备 */
	unregister_framebuffer(info);
out6:
	/* 终止VDMA通道所有数据传输 */
	dmaengine_terminate_all(ddev->vdma);
	/* 释放VDMA通道 */
	dma_release_channel(ddev->vdma);
out5:
	/* 禁止vtc时序控制器 */
	xvtc_generator_stop(ddev->vtc);	
out4:
	/* 禁止时钟输出 */
	clk_disable_unprepare(ddev->pclk);

out3:
	/* 销毁colormap */
	fb_dealloc_cmap(&info->cmap);
out2:
	/* 释放DMA内存 */
	dma_free_wc(&pdev->dev, info->screen_size, info->screen_base,
		    info->fix.smem_start);
out1:
	/* 释放fb_info对象 */
	framebuffer_release(info);
	dev_err(&(pdev->dev),"vdmafb probe err: %d\r\n",ret);
	return ret;
}

static int vdmafb_remove(struct platform_device *pdev)
{
	/* 从平台驱动结构体获取驱动私有数据 */
	struct xilinx_vdmafb_dev *ddev = platform_get_drvdata(pdev);
	struct fb_info *info = ddev->info;

	unregister_framebuffer(info);
	dmaengine_terminate_all(ddev->vdma);
	dma_release_channel(ddev->vdma);
	xvtc_generator_stop(ddev->vtc);
	clk_disable_unprepare(ddev->pclk);
	fb_dealloc_cmap(&info->cmap);
	dma_free_wc(&pdev->dev, info->screen_size,
				info->screen_base, info->fix.smem_start);
	framebuffer_release(info);
	dev_dbg(&(pdev->dev),"vdmafb remove\r\n");
	return 0;
}

static void vdmafb_shutdown(struct platform_device *pdev)
{
	struct xilinx_vdmafb_dev *ddev = platform_get_drvdata(pdev);
	xvtc_generator_stop(ddev->vtc);
	clk_disable_unprepare(ddev->pclk);
	dev_dbg(&(pdev->dev),"vdmafb shutdown\r\n");
}

static const struct of_device_id vdmafb_of_match_table[] = {
	{
		.compatible = "xilinx,vdmafb",
	},
	{ /* end of table */ },
};
MODULE_DEVICE_TABLE(of, vdmafb_of_match_table);

static struct platform_driver xilinx_vdmafb_driver = {
	.probe    = vdmafb_probe,
	.remove   = vdmafb_remove,
	.shutdown = vdmafb_shutdown,
	.driver = {
		.name           = "xilinx-vdmafb",
		.of_match_table = vdmafb_of_match_table,
	},
};

module_platform_driver(xilinx_vdmafb_driver);

MODULE_DESCRIPTION("Framebuffer driver based on Xilinx VDMA IP Core.");
MODULE_AUTHOR("Zhang Zetao <2775859312@qq.com>");
MODULE_LICENSE("GPL");
