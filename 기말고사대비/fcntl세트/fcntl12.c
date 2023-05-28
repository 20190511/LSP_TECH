#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define NAME_SIZE 50
struct employee{
	char name[NAME_SIZE];
	int pid;
	int salary;
};


int main(int argc, char* argv[])
{
	struct flock lock;
	struct employee record;
	int fd, recnum, pid;
	long position;
	char ans[5];

	//1. open
	if ((fd = open(argv[1], O_RDWR)) == -1) {
		perror(argv[1]);
		exit(1);
	}
	
	//2. pid 받아오기
	pid = getpid();
	for(;;) {
		//3. Rec num 받아와서 음수면 종료
		printf("\nEnter record number : ");
		scanf("%d", &recnum);
		if (recnum < 0)
			break;
		//4. position 수정
		position = recnum * sizeof(record);
		//5. lock F_RDLCK로 수정
		lock.l_type = F_RDLCK;
		lock.l_whence = 0;
		lock.l_start = position;
		lock.l_len = sizeof(record);
		//6.F_WETLKW 로 수정
		if (fcntl(fd, F_SETLKW, &lock) == -1) {
			perror (argv[1]);
			exit(2);
		}
		//7. lseek 수정
		lseek(fd, position, 0);
		//8. 데이터 읽어오기 --> 0이면 F_UNLCK 설정하고 continue
		if (read(fd, (char*)&record, sizeof(record)) == 0) {
			fprintf(stderr, "record %d not found\n", recnum);
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			continue;
		}
		//8. 데이터 불러오고 ans에 y n받아오기
		printf("Employee: %s, salary: %d\n", record.name, record.salary);
		printf("Do you want to update salary (y or n)? ");
		scanf("%s", ans);
		//9. ans가 y 가 아니면 데이터 UNLCK 풀고 continue
		if (ans[0] != 'y') {
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			continue;
		}
		
		//10. 데이터를 써야하므로 WRLCK로 변경 --> 적용
		lock.l_type = F_WRLCK;
		if (fcntl(fd, F_SETLK, &lock) == -1) {
			perror(argv[1]);
			exit(2);
		}
		
		//11. (실수주의) pid 넣어주기
		record.pid = pid;
		//12. 갱신월급 받아와서 write하기 *실수주의!! lseek 다시 땡겨줘야함
		printf("\nEnter new Salary : ");
		scanf("%d", &record.salary); 
		lseek(fd, position, 0); 
		write(fd, (char*)&record, sizeof(record));
		
		//13. 끝났으므로 F_SETLK 해주고 종료
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}
	//14. 파일 디스크립터 종료.  
	close(fd);
	exit(0);
}
