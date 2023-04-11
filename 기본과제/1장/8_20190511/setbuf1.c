#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(void)
{
	char buf[BUFFER_SIZE]; //setbuf 로 설정할 buffer

	setbuf(stdout, buf); //Buffer를 buf로 설정, 시스템이 할당한 BUFSIZ 로 버퍼사이즈 설정
	printf("Hello, ");
	sleep(1);
	printf("OSLAB!!");
	sleep(1);
	printf("\n");
	sleep(1);

	setbuf(stdout, NULL); //Buffer를 _IONBF 동일하게 설정
	printf("How");
	sleep(1);
	printf(" are");
	sleep(1);
	printf(" you?");
	sleep(1);
	printf("\n");
	exit(0);
}

			
