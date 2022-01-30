SOURCES := $(filter-out $(wildcard *.mod.c),$(wildcard *.c))
TARGET := lcd_fb.o
PWD := $(shell pwd) 
obj-m := ${TARGET}

.PHONY: modules clean

modules: ${TARGET}

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

${TARGET} : $(SOURCES)
	$(info the target is $(obj-m))
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	cp *.ko ../output
