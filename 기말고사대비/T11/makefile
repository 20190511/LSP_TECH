CC=gcc
TARGET = 1 2 3 4 5 6 7 8 9 10
THD = a1 a2 a3


all:$(TARGET) $(THD)

$(TARGET):%:%.c
	$(CC) -o $@ $<

$(THD):%:%.c
	$(CC) -o $@ $< -pthread


clean:
	rm -rf $(TARGET) $(THD)
