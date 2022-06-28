#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <pthread.h>
#include <linux/input.h>

#define VIDEO_DEV			"/dev/video0"
#define FB_DEV				"/dev/fb1"
#define INPUT_DEV			"/dev/input/event0"
#define VIDEO_BUFFER_COUNT	3

FILE *File_out = NULL;

static void process_image(const void *p, int size)
{
        fwrite(p, size, 1, File_out);
 
        fflush(stderr);
        fprintf(stderr, ".");
        fflush(File_out);
}

struct buffer_info {
	void *start;
	unsigned int length;
};

static void *thread_function(void *msg)
{
	int fb_fd = -1;
	int video_fd = -1;
	struct fb_var_screeninfo fb_var = {0};
	struct fb_fix_screeninfo fb_fix = {0};
	unsigned char *base;
	unsigned int screensize;
	struct v4l2_format fmt = {0};
	struct v4l2_requestbuffers req_bufs = {0};
	static struct v4l2_buffer buf;
	int n_buf;
	struct buffer_info bufs_info[VIDEO_BUFFER_COUNT];
	enum v4l2_buf_type type;

	/* 打开framebuffer设备 */
	fb_fd = open(FB_DEV, O_RDWR);
	if (0 > fb_fd) {
		printf("ERROR: failed to open framebuffer device %s\n", FB_DEV);
		return (void *)-1;
	}

	/* 获取framebuffer设备信息 */
	if (0 > ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_var)) {
		printf("ERROR: failed to FBIOGET_VSCREENINFO\n");
		return (void *)-1;
	}

	if (0 > ioctl(fb_fd, FBIOGET_FSCREENINFO, &fb_fix)) {
		printf("ERROR: failed to FBIOGET_FSCREENINFO\n");
		return (void *)-1;
	}

	/* 内存映射 */
	screensize = fb_var.yres * fb_fix.line_length;
	printf("screensize:%d\n",screensize);
	base = (unsigned char *)mmap(NULL, screensize,
				PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if (MAP_FAILED == base) {
		printf("ERROR: failed to mmap framebuffer, size 0x%x\n", screensize);
		return (void *)-1;
	}

	File_out = fopen("out.yuv", "wb");

	/* 打开摄像头设备 */
	video_fd = open(VIDEO_DEV, O_RDWR);
	if (0 > video_fd) {
		printf("ERROR: failed to open video device %s\n", VIDEO_DEV);
		return (void *)-1;
	}

	/* 设置摄像头格式 */
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = fb_var.xres;
	fmt.fmt.pix.height = fb_var.yres;
	fmt.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	if (0 > ioctl(video_fd, VIDIOC_S_FMT, &fmt)) {
		printf("ERROR: failed to VIDIOC_S_FMT\n");
		return (void *)-1;
	}

	/* 请求buffer */
	req_bufs.count = VIDEO_BUFFER_COUNT;
	req_bufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req_bufs.memory = V4L2_MEMORY_MMAP;

	if (0 > ioctl(video_fd, VIDIOC_REQBUFS, &req_bufs)) {
		printf("ERROR: failed to VIDIOC_REQBUFS\n");
		return (void *)-1;
	}

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	for (n_buf = 0; n_buf < VIDEO_BUFFER_COUNT; n_buf++) {

		buf.index = n_buf;
		if (0 > ioctl(video_fd, VIDIOC_QUERYBUF, &buf)) {
			printf("ERROR: failed to VIDIOC_QUERYBUF\n");
			return (void *)-1;
		}

		bufs_info[n_buf].length = buf.length;
		bufs_info[n_buf].start = mmap(NULL, buf.length,
					PROT_READ | PROT_WRITE, MAP_SHARED,
					video_fd, buf.m.offset);
		if (MAP_FAILED == bufs_info[n_buf].start) {
			printf("ERROR: failed to mmap video buffer, size 0x%x\n", buf.length);
			return (void *)-1;
		}
	}

	/* buffer入队 */
	for (n_buf = 0; n_buf < VIDEO_BUFFER_COUNT; n_buf++) {

		buf.index = n_buf;
		if (0 > ioctl(video_fd, VIDIOC_QBUF, &buf)) {
			printf("ERROR: failed to VIDIOC_QBUF\n");
			return (void *)-1;
		}
	}

	/* 开始采集 */
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 > ioctl(video_fd, VIDIOC_STREAMON, &type)) {
		printf("ERROR: failed to VIDIOC_STREAMON\n");
		return (void *)-1;
	}
	printf("stream on, start copy image.\n");
	for ( ; ; ) {

		for(n_buf = 0; n_buf < VIDEO_BUFFER_COUNT; n_buf++) {

			buf.index = n_buf;
			fprintf(stdout,"VIDIOC_DQBUF start\n");
			if (0 > ioctl(video_fd, VIDIOC_DQBUF, &buf)) {
				printf("ERROR: failed to VIDIOC_DQBUF\n");
				return (void *)-1;
			}
			fprintf(stdout,"VIDIOC_DQBUF end\n");
			fprintf(stdout,"image buf start:0x%x len:%d\n",(int)((unsigned char *)bufs_info[n_buf].start),bufs_info->length);
			memcpy(base, (unsigned char *)bufs_info[n_buf].start, screensize);
			//process_image(bufs_info[n_buf].start, bufs_info[n_buf].length);

			if (0 > ioctl(video_fd, VIDIOC_QBUF, &buf)) {
				printf("ERROR: failed to VIDIOC_QBUF\n");
				return (void *)-1;
			}
		}
	}

	return (void *)0;;
}

int main(int argc, char **argv)
{
	pthread_t sub_thread = {0};
	int input_fd = -1;
	struct input_event input_ev;

	/* 创建并开启线程 */
	if (0 != pthread_create(&sub_thread, NULL,
				thread_function, "This is a thread!")) {
		printf("ERROR: failed to create thread\n");
		return -1;
	}

	/* 打开按键输入设备 */
	if ( 0 == access(INPUT_DEV, F_OK) )
		input_fd = open(INPUT_DEV, O_RDONLY);
	else
		input_fd = open(INPUT_DEV, O_RDONLY);

	if (0 > input_fd) {
		printf("ERROR: failed to open input device %s", INPUT_DEV);
		return -1;
	}

	/* 监视按键活动 */
	for ( ; ; ) {

		if (read(input_fd, &input_ev, sizeof(input_ev)) == sizeof(input_ev)) {
			if (input_ev.type == EV_KEY)			// 判断是不是按键类型
				if (input_ev.value == 0) {			// 判断是否松开
					if(input_ev.code == KEY_ENTER) {	// 判断具体按键
						//open(1, O_WRONLY);
                    				write(1, "show", 4);
						fflush(File_out);
						fclose(File_out);
                    				close(1);
						return 0;
					}
				}
		}
	}
}
