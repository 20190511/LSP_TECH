#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void ssu_alarm (int signo);

int main(void)
{
	printf("Alarm Setting\n");
	signal(SIGALRM, ssu_alarm); //SIGALRM cautch 함수를 ssu_alarm으로 세팅
	alarm(2);

	while(1) {
		printf("done\n"); 
		pause();
		alarm(2); // 마치 sleep(2) 와 비슷하게 돌아감 (setjmp)를 사용해야한다고 하였음.
	}

	exit(0);
}

void ssu_alarm (int signo) {
	printf("alarm..!!!\n");
}

