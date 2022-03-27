/*
 * @Description: input_key_user_demo
 * @Version: 2.0
 * @Autor: ZZT
 * @Date: 2022-01-29 12:10:48
 * @LastEditors: ZZT
 * @LastEditTime: 2022-03-26 22:44:22
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>

#define EVENT_TYPE EV_ABS
#define EVENT_CODE_X ABS_X
#define EVENT_CODE_Y ABS_Y

int main(int argc, char *argv[])
{
	struct input_event ev;
	int fd;
	char name[256] = "Unknown";
	if (2 != argc) {
		printf("使用实例: ./input_touch_user_demo /dev/input/eventX\r\n");
		return -1;
	}

	if ((getuid()) != 0) {
		fprintf(stderr, "You are not root! This may not work...\n");
		return EXIT_SUCCESS;
	}

	/* 打开设备 */
	fd = open(argv[1], O_RDONLY);
	if (0 > fd) {
		printf("Error: file %s open failed!\r\n", argv[1]);
		return EXIT_FAILURE;
	}

	/* Print Device Name */
	ioctl(fd, EVIOCGNAME(sizeof(name)), name);
	printf("Reading from:\n");
	printf("device file = %s\n", argv[1]);
	printf("device name = %s\n", name);

	for (;;) {
		const size_t ev_size = sizeof(struct input_event);
		ssize_t size;

		/* TODO: use select() */

		size = read(fd, &ev, ev_size);
		if (size < ev_size) {
			fprintf(stderr, "Error size when reading\n");
			goto err;
		}

		if (ev.type == EVENT_TYPE &&
		    (ev.code == EVENT_CODE_X || ev.code == EVENT_CODE_Y)) {
			printf("%s = %d\n", ev.code == EVENT_CODE_X ? "X" : "Y",
			       ev.value);
		}
	}

	return EXIT_SUCCESS;

err:
	close(fd);
	return EXIT_FAILURE;
}
