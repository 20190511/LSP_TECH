#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
	char *fname = "ssu_line.txt";
	char *frname = "ssu_blank.txt";
	int fd_write, fd_read;
	int linecnt = 0;
	int wordcnt = 0;
	int charcnt = 0;
	char buf[200];
	int i = 0;

	if (access(fname, F_OK) == 0)
		remove (fname);
	
	if ((fd_read = open(frname, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", frname);
		exit(1);
	}

	//실수1. 생성할 때는 O_CREAT 써줄것.
	if ((fd_write = open(fname, O_WRONLY | O_CREAT, 0644)) < 0) {
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}

	int size = lseek(fd_read, 0, SEEK_END);
	lseek(fd_read, 0, 0);

	read(fd_read, buf, sizeof(buf));
	close(fd_read);

	//실수 2. 파일 오픈을 2개하면 4번에 복사하는게 아니라 5번에 복사함 이거 주의!
	dup2(1,5);
	dup2(fd_write, 1);

	char* ptr = buf;
	setbuf(stdout, NULL);

	for (i = 0 ; i < size ; i++) {
		charcnt++;
		if (buf[i] == ' ' || buf[i] == '\n'){
			if (buf[i] == '\n')
				linecnt++;
			wordcnt++;
			buf[i] = '\0';
			printf("%s", ptr);
			ptr = buf + i + 1;
			if (i != size-1)
				printf("\n");
		}
	}
	
	charcnt--; //마지막 \0 는 제외
	close(fd_write);

	dup2(5,1);
	printf("linecount = %d\n", linecnt);
	printf("wordcount = %d\n", wordcnt);
	printf("charcount = %d\n", charcnt);
	exit(0);
}
	
