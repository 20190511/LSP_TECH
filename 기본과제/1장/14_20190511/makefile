CC=gcc
TARGET = alarm1 alarm2 pause sigset sigprocmask

all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $<


clean:
	rm -rf $(TARGET)
