CC=gcc
TARGET=exit _exit getpid fork1 fork2 fork3

all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $<


clean:
	rm -rf $(TARGET)
