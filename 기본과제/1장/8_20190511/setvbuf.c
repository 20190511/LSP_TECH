#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void ssu_setbuf(FILE *fp, char *buf); //사용자에 따라 버퍼설정하는 함수

int main(void) 
{
	char buf[BUFFER_SIZE];
	char *fname = "/dev/pts/19"; //terminal 설정
	FILE *fp;

	if ((fp = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname);
		exit(1);
	}

	ssu_setbuf(fp, buf);   //buf 범위로 설정
	fprintf(fp, "Hello, ");
	sleep(1);
	fprintf(fp, "UNIX!!");
	sleep(1);
	fprintf(fp, "\n");
	sleep(1);
	ssu_setbuf(fp, NULL); // NULL 버퍼링 설정
	fprintf(fp, "HOW");
	sleep(1);
	fprintf(fp, " ARE");
	sleep(1);
	fprintf(fp, " YOU?");
	sleep(1);
	fprintf(fp, "\n");
	sleep(1);
	exit(0);
}


void ssu_setbuf(FILE *fp, char *buf) {
	size_t size;
	int fd;
	int mode;
	
	fd = fileno(fp); //File* 스트림 fp 로부터 fd 반환

	if (isatty(fd)) //tty가 맞으면 Line Buffering, 아니면 Full Buffering
		mode = _IOLBF;
	else
		mode = _IOFBF;

	if (buf == NULL) { //buf 가 NULL 이면 NBF 설정.
		mode = _IONBF;
		size = 0;
	}
	else
		size = BUFFER_SIZE; //buf 가 NULL 이면 size = 0 아니면 BUFFER_SIZE 설정

	setvbuf(fp, buf, mode, size);
}


