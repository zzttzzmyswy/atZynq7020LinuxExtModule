export CROSS_COMPILE:= arm-none-linux-gnueabihf-
export ARCH:= arm
export CC:= $(CROSS_COMPILE)gcc
export LD:= $(CROSS_COMPILE)ld 
export KERNELDIR = /home/zzt/xilinx/project/petalinux/kernel_no1/src/linux-xlnx

.PHONY: all clean dclean

dirs := $(shell ls -d */)

all:
	+$(foreach N,$(dirs),make -C $(N) modules;) 
	@python3 .vscode/generate_compdb.py -O $(KERNELDIR) $(PWD)

clean:
	+$(foreach N,$(dirs),make -C $(N) clean;) 
	rm -f compile_commands.json

