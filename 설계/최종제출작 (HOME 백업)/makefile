CC = gcc
CFLAGS =
LDFLAGS = -L/usr/local/openssl/lib64 -lcrypto

.SUFFIXES : .c .o
.c.o :
	$(CC) -c $(CFLAGS) $<

ALL=ssu_backup ssu_add ssu_recover ssu_remove ssu_help
all:$(ALL)
ssu_backup: ssu_backup.o
	$(CC) -o $@ $< 


ssu_recover: ssu_recover.o
	$(CC) -o $@ $< $(LDFLAGS)


ssu_remove: ssu_remove.o
	$(CC) -o $@ $< $(LDFLAGS)


ssu_add: ssu_add.o
	$(CC) -o $@ $< $(LDFLAGS)

ssu_help: ssu_help.o
	$(CC) -o $@ $< 

clean :
	rm -rf *.o $(ALL)
	
