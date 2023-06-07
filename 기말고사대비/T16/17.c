#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define NAMESIZE 50
#define MAXTRIES 5
struct employee {
	char name[NAMESIZE];
	int salary;
	int pid;
};

int main(int argc, char* argv[])
{
	int fd, try=0, sum=0;
	struct flock lock;
	struct employee record;
	
	sleep(10);
	if ((fd = open(argv[1], O_RDWR)) == -1) {
		perror(argv[1]);
		exit(1);
	}

	lock.l_type = F_RDLCK;
	lock.l_whence = 0;
	lock.l_start = 0L;
	lock.l_len = 0L;

	while (fcntl(fd, F_SETLK, &lock) == -1) { 
		if (errno == EACCES) {
			if (try++ > MAXTRIES) {
				sleep(1);
				continue;
			}
			fprintf(stderr, "%s busy -- try later\n", argv[1]);
			exit(2);
		}
		perror(argv[1]);
		exit(3);
	}

	sum = 0;
	while(read(fd, (char*)&record, sizeof(record)) > 0)
	{
		printf("Employee: %s, Salary: %d\n", record.name, record.salary);
		sum += record.salary;
	}
	
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
	printf("\nTotal Salary: %d\n", sum);
	exit(0);
}
