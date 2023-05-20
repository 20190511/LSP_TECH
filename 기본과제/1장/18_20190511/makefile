CC=gcc
TARGET=pthread_cond_1 pthread_cond_2 pthread_cond_3

all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $< -lpthread


clean:
	rm -rf $(TARGET)
