#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char* argv[])
{
	int fd;
	if((fd = open(argv[1], O_CREAT | O_RDWR, 0644)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	int i = 0;
	for (i = 0 ; i < 500000 ; i++) {
		char ch = i % 10 + '0'; 
		write(fd, (char*)&ch, 1);
	}
	close(fd);
	exit(0);

	
}
