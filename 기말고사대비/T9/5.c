#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include "ssu_employee.h"

#define DUMMY 0

int main (int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s file\n", argv[0]);
		exit(1);
	}
	int fd, var, pid;
	struct ssu_employee record;

	if ((fd = open(argv[1], O_RDWR)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	if ((var = fcntl(fd, F_GETFL, DUMMY)) < 0) {
		fprintf(stderr, "fcntl F_GETFL error\n");
		exit(1);
	}

	var |= O_APPEND;

	if (fcntl(fd, F_SETFL, var) < 0) {
		fprintf(stderr, "fcntl F_SETFL error\n");
		exit(1);
	}
	
	pid = getpid();
	while (1)
	{
		printf("Enter employee name : ");
		scanf("%s", record.name);
		if (record.name[0] == '.')
			break;

		printf("Enter employee salary : ");
		scanf("%d", &record.salary);
		record.pid = pid; //pid 안함 실수­D­E
		if (write(fd, (char*)&record, sizeof(record)) < 0) {
			fprintf(stderr, "write error\n");
			exit(1);
		}
	}
	close (fd);
	exit(0);
}
