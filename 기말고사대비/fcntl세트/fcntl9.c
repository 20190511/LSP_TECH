#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define NAMESIZE 50
#define MAXTRIES 5 //최대시도?

struct employee {
	char name [NAMESIZE];
	int salary;
	int pid;
};

int main(int argc, char* argv[])
{
	// arg 처리 안함
	struct flock lock; //flock 구조체
	struct employee record;
	int fd, sum = 0, try = 0;

	sleep(10); // 10초 재움
	
	if ((fd=open(argv[1], O_RDONLY)) < 0) {
		perror(argv[1]);
		exit(1);
	}

	lock.l_type = F_RDLCK; //읽기 락 걸기
	lock.l_whence = 0; //어디서부터? SEEK_SET
	lock.l_start = 0L; //length
	lock.l_len = 0L; //

	// fcntl 에러처리 부분
	while (fcntl(fd, F_SETLK, &lock) == -1) {
		if (errno == EACCES) { //Error Access ERror일 때 5번 시도까지 허용
			if (try++ < MAXTRIES) {
				sleep(1);
				continue;
			}
			printf("%s busy -- try later\n", argv[1]);
			exit(2);
		} 
		perror(argv[1]);
		exit(3); //그 밖의 에러는 3 EXIT
	}

	sum = 0;
	// 결론 --> ssu_employee 파일에 레코드를 읽어와서 월급 총합 구하는 파일
	while (read(fd, (char*)&record, sizeof(record)) > 0) {
		printf("Employee: %s, Salary: %d\n", record.name, record.salary);
		sum += record.salary;
	}
	printf("\nTotal salary: %d\n", sum);

	lock.l_type = F_UNLCK; //LOCK풀기 
	fcntl(fd, F_SETLK, &lock);
	close(fd);

}

