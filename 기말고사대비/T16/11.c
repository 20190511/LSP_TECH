#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
	sigset_t sig_set;
	int count;

	sigemptyset(&sig_set);
	sigaddset(&sig_set,  SIGINT);
	sigprocmask(SIG_BLOCK, &sig_set, NULL);

	for (count = 3 ; count > 0 ; count--) {
		printf("count :%d\n", count);
		sleep(1);
	}
	
	printf("Ctrl-C 에 대한 블록을 해제\n");
	sigprocmask(SIG_UNBLOCK, &sig_set, NULL);
	printf("count 중 Ctrl-C 입력하면 이 문장은 출력되지 않음\n");
	
	while(1);
	exit(0);
}
