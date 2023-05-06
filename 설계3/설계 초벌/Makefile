CC=gcc
TARGET=ssu_monitor daemon

all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $<

clean:
	rm -rf $(TARGET)