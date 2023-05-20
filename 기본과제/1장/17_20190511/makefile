CC=gcc
TARGET=pthread_create_1 pthread_create_2 pthread_exit_1 pthread_exit_2

all:$(TARGET)

$(TARGET):%:%.c
	$(CC) -o $@ $< -pthread

clean:
	rm -rf $(TARGET)
