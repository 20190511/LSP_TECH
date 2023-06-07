#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "ssu_employee.h"

#define DUMMY 0
int main(int argc, char* argv[])
{
	struct ssu_employee record;
	int fd, flag, length, pid;
	if (argc < 2) {
		fprintf(stderr, "usage: %s file\n", argv[0]);
		exit(1);
	}

	if ((fd = open(argv[1], O_RDWR)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}
	
	if ((flag = fcntl(fd, F_GETFL, DUMMY)) < 0) {
		fprintf(stderr, "fcntl F_GETFL error\n");
		exit(1);
	}
	
	flag |= O_APPEND;

	if (fcntl(fd, F_SETFL, flag) < 0) {
		fprintf(stderr, "fcntl F_SETFL error\n");
		exit(1);
	}
	
	pid = getpid();
	while(1) {
		printf("Enter employee name : ");
		scanf("%s", record.name);

		if (record.name[0] == '.')
			break;
		
		printf("Enter employee salary : ");
		scanf("%d", &record.salary);
	
		record.pid = pid;
		length = sizeof(record);
		if (write (fd, (char*)&record, length) != length) {
			fprintf(stderr, "write record error\n");
			exit(1);
		}
	}

	close(fd);
	exit(0);
}
