#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(void)
{
	char buf[BUFFER_SIZE];
	char* fname = "ssu_test.txt";
	int fd;
	int length;
	
	if((fd=open(fname, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}
	
	if( dup2(1,4) < 0) {
		fprintf(stderr, "dup2 error\n");
		exit(1);
	}

	while((length=read(fd,buf,BUFFER_SIZE)) > 0) {
		if (write(4, buf, length) != length) {
			fprintf(stderr, "write error\n");
			exit(1);
		}
	}

	close(fd);
	exit(0);
}


