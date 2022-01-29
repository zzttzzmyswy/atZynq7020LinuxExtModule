SOURCES := $(wildcard *.c)
TARGET = input_key_user_demo.o

.PHONY: demo clean

demo: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET) : $(SOURCES)
	$(info CC $@)
	$(hide) $(CC) $(SOURCES) -o ${TARGET}
	cp ${TARGET} ../output

