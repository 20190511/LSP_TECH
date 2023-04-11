CC=gcc
CFALGS=
LDFLAGS=

.SUFFIXES: .c .o
.c.o:
	$(CC) -c $(CFLAGS) $<

ALL = directory_1 directory_2 chdir getcwd
all:$(ALL)

directory_1: directory_1.c
	$(CC) -o $@ $< $(LDFLAGS)


directory_2: directory_2.c
	$(CC) -o $@ $< $(LDFLAGS)


chdir: chdir.c
	$(CC) -o $@ $< $(LDFLAGS)

getcwd: getcwd.c
	$(CC) -o $@ $< $(LDFLAGS)



clean:
	rm -rf *.o $(ALL)
