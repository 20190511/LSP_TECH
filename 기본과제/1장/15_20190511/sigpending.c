#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
	sigset_t pendingset;
	sigset_t sig_set;

	int count = 0;

	sigfillset(&sig_set);
	sigprocmask(SIG_SETMASK, &sig_set, NULL); //모든 sig_set 을 블록 걸어버림

	while(1) {
		printf("count: %d\n", count++);
		sleep(1);

		if (sigpending(&pendingset) == 0) { //sigpending 이 error가 아니라면
			if (sigismember(&pendingset, SIGINT)) { // pending set중 SIGINT가 검출된다면
				printf("SIGINT가 블록되어 대기 중. 무한 루프를 종료.\n");
				break;
			}
		}
	}

	exit(0);
}

