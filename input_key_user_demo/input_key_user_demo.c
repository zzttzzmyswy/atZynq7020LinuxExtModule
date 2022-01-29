/*
 * @Description: input_key_user_demo
 * @Version: 2.0
 * @Autor: ZZT
 * @Date: 2022-01-29 12:10:48
 * @LastEditors: ZZT
 * @LastEditTime: 2022-01-29 14:12:28
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <unistd.h>

#define ev_value(_value) (_value==1?"press":_value==2?"long press":"release")

int main(int argc, char *argv[])
{
	int fd, ret;
	struct input_event ev;
	if (2 != argc) {
		printf("使用实例: ./input_key_user_demo /dev/input/eventX\r\n");
		return -1;
	}

	/* 打开设备 */
	fd = open(argv[1], O_RDWR);
	if (0 > fd) {
		printf("Error: file %s open failed!\r\n", argv[1]);
		return -1;
	}

	/* 读取设备文件信息 */
	while (1) {
		ret = read(fd, &ev, sizeof(struct input_event));
		if (ret) {
			switch (ev.type) {
			case EV_KEY:
				switch (ev.code)
				{
				case KEY_UP:
					printf("key up is %s\r\n"
					,ev_value(ev.value));
					break;
				case KEY_DOWN:
					printf("key down is %s\r\n"
					,ev_value(ev.value));
					break;
				case KEY_ENTER:
					printf("key enter is %s\r\n"
					,ev_value(ev.value));
					break;
				case KEY_LEFT:
					printf("key left is %s\r\n"
					,ev_value(ev.value));
					break;
				case KEY_RIGHT:
					printf("key right is %s\r\n"
					,ev_value(ev.value));
					break;
				default:
					printf("can't know inf by key code\r\n");
					break;
				}
			default:
				break;
			}
		}
		else{
			printf("Error: file %s read failed!\r\n", argv[1]);
			goto out;
		}
	}
out:
	close(fd);
	return 0;
}
