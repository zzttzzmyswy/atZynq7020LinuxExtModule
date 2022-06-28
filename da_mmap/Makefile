SOURCES := $(wildcard *.c)
TARGET = da_mmap.o

.PHONY: demo clean

demo: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET) : $(SOURCES)
	$(info CC $@)
	$(hide) $(CC) $(SOURCES) -o ${TARGET} -lm 
	$(hide) chmod +x ${TARGET}
	$(hide) cp ${TARGET} ../output

