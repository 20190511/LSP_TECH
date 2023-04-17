CC=gcc
TARGET= 5 6 7 9

all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $<

clean:
	rm -rf $(TARGET)
