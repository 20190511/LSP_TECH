#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>

static void ssu_signal_handler1(int signo);
static void ssu_signal_handler2(int signo);

sigjmp_buf jmp_buf1;
sigjmp_buf jmp_buf2;

int main(void)
{
	struct sigaction act_sig1;
	struct sigaction act_sig2;
	int i, ret;

	printf("My Pid is %d\n", getpid());
	ret = sigsetjmp(jmp_buf1, 1); //현재 마스크 시그널 set 저장

	if (ret == 0) {
		act_sig1.sa_handler = ssu_signal_handler1; //sig_act1에 핸들러 저장
		sigaction(SIGINT, &act_sig1, NULL);
	}
	else if (ret == 3)
		printf("----------------\n");

	printf("Starting\n");
	sigsetjmp(jmp_buf2, 2); //현재 마스크 시그널 jmp_buf2에 저장
	act_sig2.sa_handler = ssu_signal_handler2; //sig_act2 에 핸들러 저장
	sigaction(SIGUSR1, &act_sig2, NULL);

	for (i = 0 ; i < 20 ; i++){ //20초가 될 때까지 SIGINT 나 SIGUSR1 중 하나는 시그널 오면 jump
		printf("i = %d\n", i);
		sleep(1);
	}

	exit(0);
}

static void ssu_signal_handler1(int signo) {
	fprintf(stderr, "\nInterrupted\n");
	siglongjmp(jmp_buf1, 3); //3번을 리턴하면서 1번으로 jump
}
static void ssu_signal_handler2(int signo) {
	fprintf(stderr, "\nSIGUSR1\n");
	siglongjmp(jmp_buf2, 2); //2번을 리턴하면서 2번으로 jump
}




