SOURCES := $(wildcard *.c)
TARGET = atcamtest.o

.PHONY: demo clean

demo: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET) : $(SOURCES)
	$(info CC $@)
	$(hide) $(CC) $(SOURCES) -o ${TARGET} -pthread
	$(hide) chmod +x ${TARGET}
	$(hide) cp ${TARGET} ../output

