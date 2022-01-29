#Makefile文件
CFILE:=$(shell basename ${PWD})
obj-m:=$(CFILE).o 
PWD:=$(shell pwd) 

modules: 
	echo $(CFILES)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	cp ${CFILE}.ko ../output
	

# 外部模块，不需要modules_install
# modules_install: 
# 	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install 

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

