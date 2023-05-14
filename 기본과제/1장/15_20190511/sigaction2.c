#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void ssu_check_pending (int signo, char* signame);
void ssu_signal_handler (int signo);

int main (void)
{
	struct sigaction sig_act;
	sigset_t sig_set;

	sigemptyset(&sig_act.sa_mask); //sig_act 핸들러 초기설정
	sig_act.sa_flags = 0;
	sig_act.sa_handler = ssu_signal_handler;

	if(sigaction(SIGUSR1, &sig_act, NULL) != 0) { //SIGUSR1  에 대한 설정
		fprintf(stderr, "sigaction() error\n");
		exit(1);
	}
	else {
		sigemptyset(&sig_set);
		sigaddset(&sig_set, SIGUSR1); //SIGUSR1 블락

		if (sigprocmask(SIG_SETMASK, &sig_set, NULL) != 0) { //sig_set에 SIGUSR1 블락
			fprintf(stderr, "sigprocmask() error\n");
			exit(1);
		}
		else {
			printf("SIGUSR1 signals are now blocked\n");
			kill(getpid(), SIGUSR1);
			printf("after kill()\n");
			ssu_check_pending(SIGUSR1, "SIGUSR1"); // pending되어 있기 때문에 pending되어있음으로 출력
			sigemptyset(&sig_set);
			sigprocmask(SIG_SETMASK, &sig_set, NULL); //다시 sig_set을 모두 블락해제를 해버림 --> sig_handler 출력
			printf("SIGUSR1 signals are no longer blocked\n");
			ssu_check_pending(SIGUSR1, "SIGUSR1");
		}
	}

	exit(0);
}

void ssu_check_pending (int signo, char* signame)
{
	sigset_t sig_set;

	if (sigpending(&sig_set) != 0)  //block되어 pending 되어있는 시그널 가져옴
		printf("sigpending() error\n");
	else if (sigismember(&sig_set, signo))  //pending 되어있으면 (멤버이면) 
 		printf("a %s signal is pending\n", signame);
	else //pending되어 있지 않으면
		printf("%s signals are not pending\n", signame);

}

void ssu_signal_handler (int signo) {
	printf("in ssu_signal_handler function\n");
}

	

