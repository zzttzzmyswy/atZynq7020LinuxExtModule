#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>


static void display_demo_1 (unsigned char *frame, unsigned int width, unsigned int height, unsigned int stride)
{
	unsigned int xcoi, ycoi;
	unsigned char wRed, wBlue, wGreen;
	unsigned int iPixelAddr = 0;

	for(ycoi = 0; ycoi < height; ycoi++) {

		for(xcoi = 0; xcoi < (width * 3); xcoi += 3) {

			if (((xcoi / 4) & 0x20) ^ (ycoi & 0x20)) {
				wRed = 255;
				wGreen = 255;
				wBlue = 255;
			} else {
				wRed = 0;
				wGreen = 0;
				wBlue = 0;
			}

			frame[xcoi + iPixelAddr + 0] = wBlue;
			frame[xcoi + iPixelAddr + 1] = wGreen;
			frame[xcoi + iPixelAddr + 2] = wRed;
		}

		iPixelAddr += stride;
	}
}

static void display_demo_2 (unsigned char *frame, unsigned int width, unsigned int height, unsigned int stride)
{
	unsigned int xcoi, ycoi;
	unsigned int iPixelAddr = 0;
	unsigned char wRed, wBlue, wGreen;
	unsigned int xInt;

	xInt = width * 3 / 8;
	for(ycoi = 0; ycoi < height; ycoi++) {

		for(xcoi = 0; xcoi < (width*3); xcoi+=3) {

			if (xcoi < xInt) {                                   //White color
				wRed = 255;
				wGreen = 255;
				wBlue = 255;
			}
			else if ((xcoi >= xInt) && (xcoi < xInt*2)) {         //YELLOW color
				wRed = 255;
				wGreen = 255;
				wBlue = 0;
			}
			else if ((xcoi >= xInt * 2) && (xcoi < xInt * 3)) {        //CYAN color
				wRed = 0;
				wGreen = 255;
				wBlue = 255;
			}
			else if ((xcoi >= xInt * 3) && (xcoi < xInt * 4)) {        //GREEN color
				wRed = 0;
				wGreen = 255;
				wBlue = 0;
			}
			else if ((xcoi >= xInt * 4) && (xcoi < xInt * 5)) {        //MAGENTA color
				wRed = 255;
				wGreen = 0;
				wBlue = 255;
			}
			else if ((xcoi >= xInt * 5) && (xcoi < xInt * 6)) {        //RED color
				wRed = 255;
				wGreen = 0;
				wBlue = 0;
			}
			else if ((xcoi >= xInt * 6) && (xcoi < xInt * 7)) {        //BLUE color
				wRed = 0;
				wGreen = 0;
				wBlue = 255;
			}
			else {                                                //BLACK color
				wRed = 0;
				wGreen = 0;
				wBlue = 0;
			}

			frame[xcoi+iPixelAddr + 0] = wBlue;
			frame[xcoi+iPixelAddr + 1] = wGreen;
			frame[xcoi+iPixelAddr + 2] = wRed;
		}

		iPixelAddr += stride;
	}
}

static void display_demo_3 (unsigned char *frame, unsigned int width, unsigned int height, unsigned int stride)
{
	unsigned int xcoi, ycoi;
	unsigned int iPixelAddr = 0;

	for(ycoi = 0; ycoi < height; ycoi++) {

		for(xcoi = 0; xcoi < (width * 3); xcoi+=3) {

			frame[xcoi + iPixelAddr + 0] = xcoi/3;
			frame[xcoi + iPixelAddr + 1] = xcoi/3;
			frame[xcoi + iPixelAddr + 2] = xcoi/3;
		}

		iPixelAddr += stride;
	}
}

int main (int argc, char **argv)
{

	int fd;
	int error;
	unsigned char *base;
	unsigned int screensize;
	struct fb_var_screeninfo fb_var = {0};
	struct fb_fix_screeninfo fb_fix = {0};

	if (2 != argc) {
		printf("Usage: fb_test lcd|hdmi\n");
		return -1;
	}

	/* 打开framebuffer设备 */
	if (!strcmp(argv[1], "lcd")) fd = open("/dev/fb0", O_RDWR);
	else if (!strcmp(argv[1], "hdmi")) fd = open("/dev/fb1", O_RDWR);
	else {
		printf("Usage: fb_test lcd|hdmi\n");
		return -1;
	}

	if (0 > fd) {
		printf("Error: Failed to open %s device.\n", argv[1]);
		return fd;
	}

        /* 获取framebuffer设备的参数信息 */
	error = ioctl(fd, FBIOGET_VSCREENINFO, &fb_var);
	if (-1 == error) {
		close(fd);
		return error;
	}

	error = ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix);
	if (-1 == error) {
		close(fd);
		return error;
	}

	/* mmap映射 */
	screensize = fb_var.yres * fb_fix.line_length;
	base = (unsigned char *)mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if ((unsigned char *)-1 == base) {
		close(fd);
		return -1;
	}

	memset(base, 0x00, screensize);

	/* 循环显示不同颜色 */
	display_demo_2(base, fb_var.xres, fb_var.yres, fb_fix.line_length);
	for ( ; ; ) {

		display_demo_1(base, fb_var.xres, fb_var.yres, fb_fix.line_length);
		sleep(3);

		display_demo_2(base, fb_var.xres, fb_var.yres, fb_fix.line_length);
		sleep(3);

		display_demo_3(base, fb_var.xres, fb_var.yres, fb_fix.line_length);
		sleep(3);
	}

	/* 关闭设备 释放内存 */
	memset(base, 0x00, screensize);
	munmap(base, screensize);
	close(fd);
	return 0;
}

