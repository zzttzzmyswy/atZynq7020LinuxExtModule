SOURCES := $(wildcard *.c)
TARGET = input_touch_user_demo.o

.PHONY: demo clean

demo: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET) : $(SOURCES)
	$(info CC $@)
	$(hide) $(CC) $(SOURCES) -o ${TARGET}
	$(hide) chmod +x ${TARGET}
	$(hide) cp ${TARGET} ../output

