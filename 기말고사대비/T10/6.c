#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include "ssu_employee.h"

#define DUMMY 0
int main(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s file\n", argv[0]);
		exit(1);
	}	
	
	int fd, var, length, pid;
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
	while (1) {
		printf("Enter employee name : ");
		scanf("%s", record.name);
		
		if (record.name[0] == '.')
			break;
		record.pid = pid;
		printf("Enter employee salary : ");
		scanf("%d", &record.salary);
		length = sizeof(record);	
		if (write(fd, (char*)&record, length) != length) {
			fprintf(stderr, "record write error\n");
			exit(1);
		}

	}
	
	close(fd);
	exit(0);

}
