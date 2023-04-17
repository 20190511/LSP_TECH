#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void ssu_charatatime(char *str);

int main(void)
{
	pid_t pid;

	if((pid=fork()) <0) {
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if (pid == 0)
		ssu_charatatime("output from child!\n");
	else
		ssu_charatatime("output from parent!\n");

	exit(0);
}

static void ssu_charatatime(char* str)
{
	char *ptr;
	int print_char;

	setbuf(stdout, NULL);	//버퍼 없애기
	
	for (ptr = str ; (print_char = *ptr++) != 0 ; ) { //print_char 에 str을 하나씩 출력하기
		putc(print_char, stdout);
		usleep(10);		//10마이크로초 sleep
	}
}	
