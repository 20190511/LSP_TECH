#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define NAMESIZE 50
struct employee {
	char name[NAMESIZE];
	int salary;
	int pid;
};

int main(int argc, char* argv[])
{
	int fd, pid, recnum;
	long position;
	struct flock lock;
	struct employee record;
	char ans[5];
	
	if ((fd = open(argv[1], O_RDWR)) == -1) {
		perror(argv[1]);
		exit(1);
	}

	pid = getpid();
	for(;;) {
		printf("\nEnter record number : ");
		scanf("%d", &recnum);

		if (recnum < 0)
			break;

		position = recnum * sizeof(record);
		lock.l_type = F_RDLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;

		if (fcntl(fd, F_SETLKW, &lock) < 0) {
			perror(argv[1]);
			exit(2);
		}

		lseek(fd, position, 0);
		if (read(fd, (char*)&record, sizeof(record)) <= 0) {
			printf("record %d is not found\n", recnum);
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			continue;
		}
		printf("Employee: %s, Salary: %d\n", record.name, record.salary);
		printf("Do you want to update salary (y or n)? ");
		scanf("%s", ans);

		if (ans[0] != 'y') {
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			continue;
		}

		printf("Enter new Salary : ");
		scanf("%d", &record.salary);
		lock.l_type = F_WRLCK;


		if (fcntl(fd, F_SETLKW, &lock) < 0) {
			perror(argv[1]);
			exit(2);
		}

		lseek(fd, position, 0);
		record.pid = pid;
		write(fd, (char*)&record, sizeof(record));

		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}
	close(fd);
	exit(0);
}
