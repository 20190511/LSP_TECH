CC=gcc
TARGET = 1 2 3 4 5 6 7 8 9 10

all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $<

clean:
	rm -rf $(TARGET)
