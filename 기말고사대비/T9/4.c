#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int main(void)
{
	int fd1, fd2, var;
	char* filename = "ssu_test.txt"; //실수주의 

	if ((fd1 = open(filename, O_RDWR | O_APPEND, 0644)) < 0) { //실수주의2, O_RDWR | O_APPEND , 0644 임
		fprintf(stderr, "open error for %s\n", filename);
		exit(1);
	}

	/*
	if ((var = fcntl(fd1, F_GETFD, 0)) < 0) {
		fprintf(stderr, "fcntl F_GETFD error\n");
		exit(1);
	}
	
	var |= FD_CLOEXEC;
	
	if (fcntl(fd1, F_SETFD, var) < 0) {
		fprintf(stderr, "fcntl F_SETFD error\n");
		exit(1);
	}
	*/
	if (fcntl(fd1, F_SETFD, FD_CLOEXEC) < 0) {
		fprintf(stderr, "fcntl F_SETFD error\n");
		exit(1);
	}


	if ((var = fcntl(fd1, F_GETFL, 0)) < 0) {
		fprintf(stderr, "fcntl F_GETFL error\n");
		exit(1);
	}

	if (var & O_APPEND)
		printf("fd1 : O_APPEND flag is set.\n");
	else
		printf("fd1 : O_APPEND flag is NOT set.\n");

	if ((var = fcntl(fd1, F_GETFD, 0)) < 0) {
		fprintf(stderr, "fcntl F_GETFD error\n");
		exit(1);
	}

	if (var & FD_CLOEXEC)
		printf("fd1 : O_CLOEXEC flag is set.\n");
	else
		printf("fd1 : O_CLOEXEC flag is NOT set.\n");
	
	if ((fd2 = fcntl(fd1, F_DUPFD, 0)) < 0) { //F_DUPFD, 0 하면 됨
		fprintf(stderr, "fcntl F_DUPFD error\n");
		exit(1);
	}
	
	if ((var = fcntl(fd2, F_GETFL, 0)) < 0) {
		fprintf(stderr, "fcntl F_GETFL error\n");
		exit(1);
	}

	if (var & O_APPEND)
		printf("fd2 : O_APPEND flag is set.\n");
	else
		printf("fd2 : O_APPEND flag is NOT set.\n");

	if ((var = fcntl(fd2, F_GETFD, 0)) < 0) {
		fprintf(stderr, "fcntl F_GETFD error\n");
		exit(1);
	}

	if (var & FD_CLOEXEC)
		printf("fd2 : O_CLOEXEC flag is set.\n");
	else
		printf("fd2 : O_CLOEXEC flag is NOT set.\n");
	
	close(fd1);
	close(fd2);
	exit(0);
}
