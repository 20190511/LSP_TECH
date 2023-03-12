#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
	char *fname = "ssu_test.txt";
	int fd;

	if ((fd = creat(fname, 0666)) < 0)
	{
		printf("creat error for %s\n", fname);
		exit(1);
	}

	printf("first printf is on the screen.\n");
	dup2(fd, 1);
	printf("second printf is in this file\n");
	exit(0);
}

