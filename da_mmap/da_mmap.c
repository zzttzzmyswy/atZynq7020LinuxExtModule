#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <math.h>

#define PAGE_SIZE (1000 * 1000 * 1)
#define PI (3.1415926535)

int main(int argc, char *argv[])
{
	int fd;
	int i;
	unsigned char *p_map;
	long long ik;

	//打开设备
	fd = open("/dev/da_zzt", O_RDWR);
	if (fd < 0) {
		printf("open fail\n");
		exit(1);
	}

	//内存映射
	p_map = (unsigned char *)mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE,
				      MAP_SHARED, fd, 0);
	if (p_map == MAP_FAILED) {
		printf("mmap fail\n");
		goto here;
	}
	printf("p_map:%x\n", (unsigned char *)p_map);
	for (i = 1; i <= 8; i++) {
		for (ik = 0; ik < PAGE_SIZE; ik++) {
			//p_map[ik] = (ik > PAGE_SIZE / 2) ? 0XEE : 0X11;
			p_map[ik] = (unsigned char)(((sin((ik * 1.0 * i / PAGE_SIZE) *
							  (2.0 * PI))) +
						     1.0) *
						    0x80);
		}
		sleep(2);
	}

here:
	munmap(p_map, PAGE_SIZE);
	return 0;
}
