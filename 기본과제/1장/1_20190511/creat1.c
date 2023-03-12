#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
	char *fname = "ssu_text.txt";
	int fd;
	if ((fd = creat(fname, 0666) < 0) )
	{
		fprintf(stderr, "creat error for %s\n", fname);
		exit(1);
	}
	else
	{
		printf("Suceess!\nFilename : %s\nDescriptor : %d\n", fname, fd);
		close(fd);
	}

	exit(0);
}
