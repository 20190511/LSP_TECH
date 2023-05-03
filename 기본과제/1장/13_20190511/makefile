CC=gcc
TARGET=signal1 signal2 kill1 kill2

all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $<


clean:
	rm -rf $(TARGET)
