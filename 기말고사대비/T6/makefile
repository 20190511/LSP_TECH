CC=gcc
THD = t1 t2 
TARGET=1 2 3 4 5 6

all:$(THD)

$(THD):%:%.c
	$(CC) -o $@ $< -pthread

claen:
	rf -rf $(THD)
