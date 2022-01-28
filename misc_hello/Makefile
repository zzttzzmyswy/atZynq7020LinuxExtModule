#Makefile文件
PROFILE=hello

obj-m := $(PROFILE).o 
PWD := $(shell pwd) 

modules: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	@cp ${PROFILE}.ko ../output
	

# 外部模块，不需要modules_install
# modules_install: 
# 	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install 

clean:
	rm -f *.o
	rm -f *.symvers
	rm -f *.order
	rm -f *.ko
	rm -f *.mod.c
	rm -f *.cmd
	rm -f *.mod
	rm -f .*.cmd
