.PHONY: all clean dclean

dirs := $(shell ls -d */)

all:
	+$(foreach N,$(dirs),make -C $(N) modules;)
	python3 .vscode/generate_compdb.py -O $(KERNELDIR) $(PWD)

clean:
	+$(foreach N,$(dirs),make -C $(N) clean;)
#	rm -f compile_commands.json

