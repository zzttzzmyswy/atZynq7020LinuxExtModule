.PHONY: all clean

dirs := $(shell ls -d */)

all:
	+$(foreach N,$(dirs),make -C $(N);)
	cp compile_commands.json compile_commands.json.bak
	python3 .vscode/generate_compdb.py -O $(KERNELDIR) $(PWD)

clean:
	+$(foreach N,$(dirs),make -C $(N) clean;)
#	rm -f compile_commands.json

