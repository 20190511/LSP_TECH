CC=gcc
TARGET=ssu_monitor daemon daemon_io

all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $<

clean:
	rm -rf $(TARGET)
