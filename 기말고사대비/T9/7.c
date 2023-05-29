#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#define MAXSIZE 50
struct employee {
	char name[MAXSIZE];
	int salary;
	int pid;
};

int main(int argc, char* argv[])
{
	struct employee record;
	struct flock lock;
	char ans[3];
	int fd, recnum, pid;
	long position;
	
	if ((fd = open(argv[1], O_RDWR)) < 0) {
		perror(argv[1]);
		exit(1);
	}

	pid = getpid();

	while(1) {
		printf("\nEnter record number: ");
		scanf("%d", &recnum);
		if (recnum < 0)
			break;
		
		position = recnum * sizeof(record);
		lock.l_type = F_RDLCK;
		lock.l_whence = 0;
		lock.l_start = position;
		lock.l_len = sizeof(record);

		if (fcntl(fd, F_SETLKW, &lock) == -1) {
			perror(argv[1]);
			exit(2);
		}
		
		lseek(fd, position, 0);
		if (read (fd, (char*)&record, sizeof(record)) == 0) {
			printf("record %d not found\n", recnum); // 문구 암기!!
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			continue;
		}
		
		printf("Employee: %s, Salary: %d\n", record.name, record.salary);
		printf("Do you want to update salary (y or n) ? ");
		scanf("%s", ans);
		
		if (ans[0] != 'y') {
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			continue;
		}

		printf("Enter new salary : ");
		scanf("%d", &record.salary);
		lock.l_type = F_WRLCK;
		if (fcntl(fd, F_SETLKW, &lock) < 0){
			perror(argv[1]);
			exit(3);
		}
		
		//실수!!!!
		record.pid = pid;
		lseek(fd, position, 0);
		write (fd, (char*)&record, sizeof(record));
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);	

	}

	close(fd);
	exit(0);
}
