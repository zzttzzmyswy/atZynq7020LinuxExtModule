#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define ARGV_ADDR_POS 2
#define ARGV_DATA_POS 3
#define ARGV_WR_OFF_POS 3
#define ARGV_WR_DATA_POS 4
#define ARGV_CMD_POS 1

//显示使用方法
void printUsage()
{
	printf("Usage: \r\n Read address as Byte: mymm r [ADDR] [LEN] \r\n Read address as int: mymm i [ADDR] [LEN] \r\n");
	printf(" Write to address: mymm w [ADDR] [OFF] [DATA]\r\n");
}

//读取某个基地址addr+byte字节的数据并逐个字节显示。
//从DataSheet上看，页大小为0x400，因此这里的基地址addr必须是能够被0x400整除，否则Segment Error.
void readMMap(int dev_fd, unsigned int addr, unsigned int byte)
{
	int mmapByte = (byte / 4 * 4 + 4), iloop;
	int realByte = (byte / 4 * 4);
	//这里的mmapByte计算多此一举，因为这里只要进行映射都会按照页面大小向上取整，一般不用担心超出地址的问题
	unsigned char *map_base = (unsigned char *)mmap(
		NULL, mmapByte, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd,
		addr); //使用MMAP直接映射基地址的内存数据到map_base
	int i, j;
	if (map_base == (unsigned char *)-1) {
		printf("MMAP Error.\r\n");
		return;
	}
	printf("           | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");
	printf("------------------------------------------------------------\r\n");
	iloop = realByte / 16 + ((realByte % 16) ? 1 : 0);
	//printf("iloop is %d\r\n",iloop);
	for (i = 0; i < iloop; i++) {
		int loopbyte = (byte - i * 16 > 16) ? 16 : (byte - i * 16);
		//printf("loopbyte = %d\n",loopbyte);ARGV_WR_OFF_POS
		printf("0x%08X | ", addr + i * 16);
		for (j = 0; j < loopbyte; j++) {
			printf("%02X ",
			       *(volatile unsigned char *)(map_base + i * 16 +
							   j));
		}
		printf("\r\n");
	}
	munmap(map_base, mmapByte);
}

//读取某个基地址addr+byte字节的数据并按照逐个int显示。
//从DataSheet上看，页大小为0x400，因此这里的基地址addr必须是能够被0x400整除，否则Segment Error.
void readMMapByUINT(int dev_fd, unsigned int addr, unsigned int byte)
{
	int mmapByte = (byte / 4 * 4 + 4), iloop;
	int realByte = (byte / 4 * 4);
	unsigned char *map_base =
		(unsigned char *)mmap(NULL, mmapByte, PROT_READ | PROT_WRITE,
				      MAP_SHARED, dev_fd, addr);
	unsigned int *map_base_i = (unsigned int *)map_base;
	int i, j;
	if (map_base == (unsigned char *)-1) {
		printf("MMAP Error.\r\n");
		return;
	}
	printf("           | +0x00      +0x04      +0x08      +0x0C         \r\n");
	printf("------------------------------------------------------------\r\n");
	iloop = realByte / 16 + ((realByte % 16 > 0) ? 1 : 0);
	//printf("iloop = %d\r\n",iloop);
	for (i = 0; i < iloop; i++) {
		int loopcnt = (byte - i * 4 > 4) ? 4 : byte - i * 4;
		//printf("loopbyte = %d\n",loopbyte);
		printf("0x%08X | ", addr + i * 16);
		for (j = 0; j < loopcnt; j++) {
			printf("0x%08X ",
			       *(volatile unsigned int *)(map_base_i + i * 4 +
							  j));
		}
		printf("\r\n");
	}
	munmap(map_base, mmapByte);
}

//将数据data写入基地址addr+offset的位置。
void writeMMap(int dev_fd, unsigned int addr, unsigned int offset,
	       unsigned int data)
{
	unsigned int mmapByte = offset + 0x04;
	unsigned char *map_base =
		(unsigned char *)mmap(NULL, mmapByte, PROT_READ | PROT_WRITE,
				      MAP_SHARED, dev_fd, addr);
	if (map_base == (unsigned char *)-1) {
		printf("MMAP Error.\r\n");
		return;
	}
	printf("Before Modify : addr 0x%08X = 0x%08X\r\n", (addr + offset),
	       *(volatile unsigned int *)(map_base + offset));
	*(volatile unsigned int *)(map_base + offset) = data;
	printf("After Modify : addr 0x%08X = 0x%08X\r\n", (addr + offset),
	       *(volatile unsigned int *)(map_base + offset));
	munmap(map_base, mmapByte);
}

int main(int argc, char *argv[])
{
	int addr = 0, byte = 0, fd = 0, off = 0;
	if (argc < 4) {
		printf("ARG ERROR.\r\n");
		printUsage();
		return 0;
	}
	addr = strtoul(argv[ARGV_ADDR_POS], 0, 0);
	byte = strtoul(argv[ARGV_DATA_POS], 0, 0);

	if (addr == 0) {
		printf("Addr Err.\r\n");
		return;
	}
	fd = open("/dev/mem", O_RDWR | O_NDELAY);
	if (fd < 0) {
		printf("open(/dev/mem) failed.");
		return 0;
	}
	switch (argv[ARGV_CMD_POS][0]) {
	case 'r':
		if (byte == 0) {
			printf("Byte len Err.\r\n");
			close(fd);
			return 0;
		}
		printf("Now Read Memory at 0x%08X by %d\r\n", addr, byte);
		readMMap(fd, addr, byte);
		break;
	case 'i':
		if (byte == 0) {
			printf("Byte len Err.\r\n");
			close(fd);
			return 0;
		}
		printf("Now Read Memory By int at 0x%08X by %d\r\n", addr,
		       byte);
		readMMapByUINT(fd, addr, byte);
		break;
	case 'w':
		if (argc < 5) {
			printf("Write ARGC Error.");
			close(fd);
			return 0;
		}
		off = strtoul(argv[ARGV_WR_OFF_POS], 0, 0);
		byte = strtoul(argv[ARGV_WR_DATA_POS], 0, 0);
		printf("Now Write Memory By int at 0x%08X + 0x%08X by 0x%08X\r\n",
		       addr, off, byte);
		writeMMap(fd, addr, off, byte);
		break;
	default:
		printf("Error Cmd.\r\n");
		break;
	}
	close(fd);
	return 0;
}