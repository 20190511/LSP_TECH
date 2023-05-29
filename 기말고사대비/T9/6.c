#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

#define MAXSIZE 50
#define MAXTRIES 5

struct employee {
	char name[MAXSIZE];
	int salary;
	int pid;
};

int main(int argc, char* argv[])
{
	int fd, length, try = 0, sums = 0;
	struct employee record;	
	struct flock lock;
	
	sleep(10); //실수 주의
	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}
	// sleep(10);		
	lock.l_type = F_RDLCK;
	lock.l_whence = 0;
	lock.l_start = 0L;
	lock.l_len = 0L;

	while (fcntl(fd, F_SETLK, &lock) == -1) {
		if (errno == EACCES) {
			if (try++ < MAXTRIES) {
				sleep(1);
				continue;

			}
			fprintf(stderr, "%s busy -- try later\n", argv[1]);
			exit(2);
		}
		perror(argv[1]);
		exit(3);
	}
	
	sums = 0;
	while (read(fd, (char*)&record, sizeof(record)) > 0) {
		printf("Employee: %s, Salary: %d\n", record.name, record.salary);
		sums += record.salary;
	}
	printf("\nTotal Salary: %d\n", sums);
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
	exit(0);
}
