#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

void ssu_signal_handler(int signo);

jmp_buf jump_buffer;

int main(void)
{
	signal(SIGINT, ssu_signal_handler);

	while(1) {
		if (setjmp(jump_buffer) == 0) { //해당 위치로 jump했을 때 signal mask가 자동으로 설정되어서 일회용되버림..
			printf("Hit Ctrl-c at anytime ... \n");
			pause();
		}
	}

	exit(0);
}


void ssu_signal_handler(int signo) {
	char character;

	signal(signo, SIG_IGN); //해당 시그널을 무시하도록 마치 indirecting??
	printf("Did you hit Ctrl-c?\n" "Do you really want to quit? [y/n]\n");
	character = getchar();

	if (character == 'y' || character == 'Y') 
		exit(0);
	else {
		signal(SIGINT, ssu_signal_handler);
		longjmp(jump_buffer, 1);
	}
}

