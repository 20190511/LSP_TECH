CC=gcc
TARGET=1 2 3 4 5 6 7 8 9 10
THD = a1 a2 


all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $<

$(THD):%:%.c
	$(CC) -o $@ $< -lpthread


clean:
	rm -rf $(TARGET) $(THD)

