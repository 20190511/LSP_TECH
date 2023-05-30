#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#define MAXSIZE 50
#define MAXTRIES 5

struct employee {
	char name [MAXSIZE];
	int salary;
	int pid;
};

int main(int argc, char* argv[])
{
	int fd, length, recnum, pid;
	long position;
	char ans[3];
	struct flock lock;
	struct employee record;

	if ((fd = open(argv[1], O_RDWR)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}
	
	pid = getpid();
	for(;;)
	{
		//실수 포인트 1 \n 안해줌
		printf("\nEnter record number : ");
		scanf("%d", &recnum);

		if (recnum < 0)
			break;
	
		position = recnum * sizeof(record);
		lock.l_type = F_RDLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = position;
		lock.l_len = sizeof(position);

		if (fcntl(fd, F_SETLKW, &lock) < 0) {
			perror(argv[1]);
			exit(2); 
		} 
	
		lseek(fd, position, 0);
		
		if (read(fd, (char*)&record, sizeof(record)) == 0) {
			printf("record %d not found\n", recnum);
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
	
		record.pid = pid;
		lock.l_type = F_WRLCK;
		if (fcntl(fd, F_SETLKW, &lock) < 0)  {
			perror(argv[1]);
			exit(3);
		}
		printf("Enter your salary : ");
		scanf("%d", &record.salary);
		lseek(fd, position, SEEK_SET);
		write(fd, (char*)&record, sizeof(record));
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);	
	}
	
	close(fd);
	exit(0); 
}
