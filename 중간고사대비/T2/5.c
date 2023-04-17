#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#define BUFFER_SIZE 1024

void timechar (time_t t, char* buf);

int main (int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <file1> <file2> .. <fileN>\n", argv[0]);
		exit(1);
	}
	
	struct stat statbuf;
	int i, j = 0;
	char buf [BUFFER_SIZE];

	printf("Mode	Blocks Links   UID  GID        Access			Change		  Modify       Path\n");
	for (i = 1 ; i < argc ; i++) {
		if (lstat(argv[i], &statbuf) < 0) {
			fprintf(stderr, "stat error for %s\n", argv[i]);
			continue;
		}

		memset(buf, '\0', BUFFER_SIZE);
		char *ptr = buf;
		*ptr++ = S_ISDIR(statbuf.st_mode) ? 'd' : '-';
		char otherbuf[100];
		//실수 1. sprintf를 strcat 로 헷갈렸음
		//★실수 2 (S_ISUSR & Statbuf.st_mode) 이렇게 해줘야함!!!! 함수 매크로아님!!!
		*ptr++ = (statbuf.st_mode) & S_IRUSR ? 'r' : '-';
		*ptr++ = (statbuf.st_mode) & S_IWUSR ? 'w' : '-';
		*ptr++ = (statbuf.st_mode) & S_IXUSR ? 'x' : '-';
		*ptr++ = (statbuf.st_mode) & S_IRGRP ? 'r' : '-';
		*ptr++ = (statbuf.st_mode) & S_IWGRP ? 'w' : '-';
		*ptr++ = (statbuf.st_mode) & S_IXGRP ? 'x' : '-';
		*ptr++ = (statbuf.st_mode) & S_IROTH ? 'r' : '-';
		*ptr++ = (statbuf.st_mode) & S_IWOTH ? 'w' : '-';
		*ptr++ = (statbuf.st_mode) & S_IXOTH ? 'x' : '-';
		//실수7. -3lu 이런식으로 -(형식)lu 이런식으로 써야함. 잘 알아둘것.
		//주의8. stat 구조체 멤버들 정리해둘것, 출력형식이나 종류...
		sprintf(otherbuf, "    %-3lu %-3lu %5u %5u", statbuf.st_blocks, statbuf.st_nlink, statbuf.st_gid, statbuf.st_uid);
		strcat(buf, otherbuf);
		memset(otherbuf, '\0', 100);	
		timechar(statbuf.st_atime, otherbuf);
		strcat(buf, otherbuf);	
		memset(otherbuf, '\0', 100);
		timechar(statbuf.st_ctime, otherbuf);
		strcat(buf, otherbuf);	
		memset(otherbuf, '\0', 100);
		timechar(statbuf.st_mtime, otherbuf);
		strcat(buf, otherbuf);	
		strcat(buf, "     ");
		strcat(buf, argv[i]);
		printf("%s\n", buf);

	}
	exit(0);
}


void timechar (time_t t, char* buf) {
	// 실수 3. tm 구조체는 struct tm* 형태로 사용해야함.
	// 실수 4 localtime 에는 들어가는것도 포인터 , 나오는것도 포인터임.
	struct tm* t1;
	char timebuf [20];

	t1 = localtime(&t);
	
	//실수 5. t1을 포인터로 받았는데 . 으로 넣는 실수
	//실수 6. %02d 로 해줘야함.
	sprintf(timebuf, "    %02d-%02d-%02d %02d:%02d", t1->tm_year%100, t1->tm_mon+1, t1->tm_mday, t1->tm_hour, t1->tm_min);
	strcpy(buf, timebuf);
}

	
	


		
