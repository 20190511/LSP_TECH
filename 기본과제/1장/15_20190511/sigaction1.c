#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void ssu_signal_handler (int signo) {
	printf("ssu_signal_handler control\n");
}


int main(void) {
	struct sigaction sig_act;
	sigset_t sig_set;

	sigemptyset(&sig_act.sa_mask); //sig_act 핸들러의 sa_mask 를 모두 0으로 설정
	sig_act.sa_handler = ssu_signal_handler; //핸들러 조정
	sig_act.sa_flags = 0;
	sigaction(SIGUSR1, &sig_act, NULL);
	printf("before first kill()\n");
	kill(getpid(), SIGUSR1); //자기 자신한테 SIGUSR1 시그널 보냄
	sigemptyset(&sig_set);
	sigaddset(&sig_set, SIGUSR1);
	sigprocmask(SIG_SETMASK, &sig_set, NULL); //sig_set에 SIGUSR1 만 블락
	printf("before second kill()\n");
	kill(getpid(), SIGUSR1); //블락되어서 해당 시그널 pending만 되어있음 --> 출력X
	printf("after second kill()\n");
	exit(0);
}
	

