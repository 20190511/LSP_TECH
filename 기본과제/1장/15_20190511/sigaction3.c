#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

static void ssu_signal_handler1(int signo);
static void ssu_signal_handler2(int signo);

int main(void)
{
	struct sigaction act_int, act_quit;

	act_int.sa_handler = ssu_signal_handler1;
	sigemptyset(&act_int.sa_mask); //sig_int 시그널은 모두 비워버리고 SIGQUIT만 추가 
	//시그널 핸들러가 실행되는 도중 SIGQUIT 시그널은 받지 않겠다는 의미
	sigaddset(&act_int.sa_mask, SIGQUIT); //SIGQUIT 시그널 비트마스크 설정
	act_quit.sa_flags = 0;

	if (sigaction(SIGINT, &act_int, NULL) < 0) { //SIGINT 핸들러 조정
		fprintf(stderr, "sigaction(SIGINT) error\n");
		exit(1);
	}

	act_quit.sa_handler = ssu_signal_handler2;
	sigemptyset(&act_quit.sa_mask); //sig_quit에는 SIGINT mask를 블록하는중
	sigaddset(&act_quit.sa_mask, SIGINT);
	act_quit.sa_flags = 0;


	if (sigaction(SIGQUIT, &act_quit, NULL) < 0) { //SIGQUIT handler 조정
		fprintf(stderr, "sigaction(SIGQUIT) error\n");
		exit(1);
	}

	pause(); //프로세스 멈춤
	exit(0);
}


static void ssu_signal_handler1(int signo) {
	printf("Signal handler of SIGINT : %d\n", signo);
	printf("SIGQUIT signal is blocked : %d\n", signo);
	printf("sleeping 3 sec\n");
	sleep(3); //3초 안에 signal(CTRL+C) 이 다시 오면 다시 핸들러 호출
	printf("Signal handler of SIGINT ended\n");
}

static void ssu_signal_handler2(int signo) { 
	printf("Signal handler of SIGQUIT : %d\n", signo);
	printf("SIGINT signal is blocked : %d\n", signo);
	printf("sleeping 3 sec\n");
	sleep(3); //3초안에 다시 signal(Ctrl+\) 오면 다시 핸들러 호출
	printf("Signal handler of SIGQUIT ended\n");
}



