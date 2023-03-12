#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
int main(void)
{
	char buf [BUFFER_SIZE];
	char *fname = "ssu_test.txt";
	int count;
	int fd1, fd2;
	
	if ((fd1 = open(fname, O_RDONLY, 0644)) < 0)
	{
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}

	fd2 = dup(fd1);
	count = read(fd1, buf, 12);
	buf[count] = 0;
	printf("fd1's printf : %s\n", buf);
	lseek(fd1, 1, SEEK_CUR);
	count = read(fd2, buf, 12);
	buf[count] = 0;
	printf("fd2's printf : %s\n", buf);
	exit(0);
}
