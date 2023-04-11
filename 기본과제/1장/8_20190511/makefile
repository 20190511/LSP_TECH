CC=gcc
CFLAGS=
LDFLAGS=

.SUFFIXES : .c .o
.c.o:
	$(CC) -c $(CFALGS) $<


ALL=fopen freopen fclose setbuf1 setbuf2 setvbuf
all: $(ALL) 


fopen: fopen.c
	$(CC) -o $@ $< $(LDFLAGS)

freopen: freopen.c
	$(CC) -o $@ $< $(LDFLAGS)
	
fclose: fclose.c
	$(CC) -o $@ $< $(LDFLAGS)

setbuf1: setbuf1.c
	$(CC) -o $@ $< $(LDFLAGS)

setbuf2: setbuf2.c
	$(CC) -o $@ $< $(LDFLAGS)
	
setvbuf: setvbuf.c
	$(CC) -o $@ $< $(LDFLAGS)



clean:
	rm -rf *.o $(ALL)

