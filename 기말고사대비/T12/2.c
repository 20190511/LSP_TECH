#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define MAXSIZE 50

struct employee {
	char name[MAXSIZE];
	int salary;
	int pid;
};

int main (int argc, char* argv[])
{
	int fd, pid, recnum;
	long position;
	struct flock lock;
	struct employee record;
	char ans[3];

	if ((fd = open(argv[1], O_RDWR)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	pid = getpid();
	while (1) {
		
		printf("\nEnter record number : ");
		scanf("%d", &recnum);

		if (recnum < 0) 
			break;
		
		position = recnum * sizeof(record);
		lock.l_type = F_RDLCK;
		lock.l_whence = 0;
		lock.l_start = position;
		lock.l_len = sizeof(record);

		if (fcntl(fd, F_SETLKW, &lock) < 0) {
			perror(argv[1]);
			exit(2);
		}
		
		lseek(fd, position, 0);
		if (read (fd, (char*)&record, sizeof(record)) == 0) {
			printf("%d records are not found\n", recnum);
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			continue;
		}

		printf("Employee: %s, Salary:%d\n", record.name, record.salary);
		printf("Do you want to update salary (y or n)? ");
		scanf("%s", ans);
		
		if (ans[0] != 'y') {
			lock.l_type = F_SETLK;
			fcntl(fd, F_SETLK, &lock);
			continue;
		}
		
		lock.l_type = F_WRLCK;
		if (fcntl(fd, F_SETLKW, &lock) < 0) {
			perror(argv[1]);
			exit(3);
		}
		
		lseek(fd, position, 0);
		record.pid = pid;
		printf("Enter new salary ");
		scanf("%d", &record.salary);

		if (write(fd, (char*)&record, sizeof(record)) != sizeof(record)) {
			fprintf(stderr, "record write error\n");
			exit(1);
		}
		
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}

	close(fd);
	exit(0);
}
