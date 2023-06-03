#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


int main(void)
{
	int fd1, fd2;
	int flags;
	char* filename = "ssu_test.txt";
	
	if ((fd1 = open (filename, O_RDWR | O_APPEND)) < 0) {
		fprintf(stderr, "open error for %s\n", filename);
		exit(1);
	}

	if (fcntl(fd1, F_SETFD, FD_CLOEXEC) == -1) {
		fprintf(stderr, "fcntl F_SETFD error\n");
		exit(1);
	}
	
	if ((flags = fcntl(fd1, F_GETFL, 0)) == -1) {
		fprintf(stderr, "fcntl F_GETFL error\n");
		exit(1);
	}

	if (flags & O_APPEND)
		printf("fd1 : O_APEPND flag is set.\n");
	else
		printf("fd1 : O_APPEND flag is NOT set.\n");

	if ((flags = fcntl(fd1, F_GETFD, 0)) == -1) {
		fprintf(stderr, "fcntl F_GETFD error\n");
		exit(1);
	}
	
	if (flags & FD_CLOEXEC)
		printf("fd1 : FD_CLOEXEC flag is set.\n");
	else
		printf("fd1 : FD_CLOEXCE flag is NOT set.\n");

	if ((fd2 = fcntl(fd1, F_DUPFD, 0)) == -1) {
		fprintf(stderr, "fcntl F_DUPFD error\n");
		exit(1);
	}

	if ((flags = fcntl(fd2, F_GETFL, 0)) == -1) {
		fprintf(stderr, "fcntl F_GETFL error\n");
		exit(1);
	}

	if (flags & O_APPEND)
		printf("fd2 : O_APEPND flag is set.\n");
	else
		printf("fd2 : O_APPEND flag is NOT set.\n");

	if ((flags = fcntl(fd2, F_GETFD, 0)) == -1) {
		fprintf(stderr, "fcntl F_GETFD error\n");
		exit(1);
	}
	
	if (flags & FD_CLOEXEC)
		printf("fd2 : FD_CLOEXEC flag is set.\n");
	else
		printf("fd2 : FD_CLOEXCE flag is NOT set.\n");

	close(fd1);
	close(fd2);
	exit(0);

}
