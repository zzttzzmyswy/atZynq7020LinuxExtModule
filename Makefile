.PHONY: all clean

sources := $(filter-out $(shell find ./ -name "*.mod.c"),$(shell find ./ -name "*.c"))
dirs := $(shell ls -d */)

all : $(sources)
	$(info source files are $(sources))
	+$(foreach N,$(dirs),make -C $(N);)
	cp compile_commands.json compile_commands.json.bak
	python3 .vscode/generate_compdb.py -O $(KERNELDIR) $(PWD)

clean:
	+$(foreach N,$(dirs),make -C $(N) clean;)
#	rm -f compile_commands.json
