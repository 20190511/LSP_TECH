CC=gcc
TARGET=ssu_monitor daemon daemon_monitor

all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $<

clean:
	rm -rf $(TARGET)
