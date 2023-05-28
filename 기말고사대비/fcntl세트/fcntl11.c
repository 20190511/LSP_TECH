#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXSIZE 50

struct employee {
	char name [MAXSIZE];
	int salary;
	int pid;
};

int main (int argc, char* argv[])
{
	struct flock lock;
	struct employee record;
	int fd, recnum, pid;
	long position;

	if ((fd = open(argv[1], O_RDWR)) == -1) {
		perror(argv[1]);
		exit(1);
	}

	pid = getpid();
	for(;;) {
		printf("\nEnter record number: ");
		scanf("%d", &recnum);
		if (recnum < 0)
			break;
	
		//lock 거는 과정
		position = recnum * sizeof(record); //record 위치 이동
		lock.l_type = F_WRLCK;
		lock.l_whence = 0; //lseek(fd, start, whence) 설정
		lock.l_start = position; 
		lock.l_len = sizeof(record); //가져올 데이터 길이

		if (fcntl(fd, F_SETLK, &lock) == -1) {
			perror(argv[1]);
			exit(2);
		}
		
		//데이터를 읽어오는 과정
		lseek(fd, position, 0);
		if (read(fd, (char*)&record, sizeof(record)) == 0) {
			printf("record %d not found\n", recnum); //데이터 읽기에 실패한 경우 UNLCK 처리하고 return
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			continue;
		}
		
		// 데이터를 보여주고 월급을 새로 갱신하는 부분
		printf("Employee: %s, salary: %d\n", record.name, record.salary);
		record.pid = pid;
		printf("Enter new salary: ");
		scanf("%d", &record.salary);
		lseek(fd, position, 0);
		write(fd, (char*)&record, sizeof(record));

		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}
	close(fd);
	exit(0);
}

