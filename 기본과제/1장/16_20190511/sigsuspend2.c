#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

static void ssu_func(int signo);
void ssu_print_mask(const char* str);


int main(void)
{
	sigset_t new_mask, old_mask, wait_mask;

	ssu_print_mask("prgroam start:  ");

	if (signal(SIGINT, ssu_func) == SIG_ERR) { //SIGINT 핸들러를 ssu_func로 설정
		fprintf(stderr, "SIGINT(SIGINT) error\n");
		exit(1);
	}

	sigemptyset(&wait_mask); //wait_mask 비트맵에 SIGUSR1만 1로 설정 
	sigaddset(&wait_mask, SIGUSR1);
	sigemptyset(&new_mask); //new_mask 비트맵에 SIGINT만 1로 설정
	sigaddset(&new_mask, SIGINT);

	if (sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0) { //new_mask로 블락 --> 원래 bit맵은 old_mask에 저장
		fprintf(stderr, "SIG_BLOCK() error\n");
		exit(1);
	}

	ssu_print_mask("in critical region: ");

	if (sigsuspend(&wait_mask) != -1) { //에러처리 주의
		fprintf(stderr, "sigsuspend() error\n");
		exit(1);
	}

	ssu_print_mask("after return from sigsuspend: ");

	if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0) { //다시 원래 mask인 old_mask로 재설정
		fprintf(stderr, "SIG_SETMASK() error\n");
		exit(1);
	}

	ssu_print_mask("program exit: ");
	exit(0);

}

static void ssu_func(int signo) {
	ssu_print_mask("\nin ssu_func: "); //signal handler를 ssu_print_mask를 호출하여 signal 종류를 알아냄
}

void ssu_print_mask(const char* str) {
	sigset_t sig_set;
	int err_num;

	err_num = errno;

	if (sigprocmask(0, NULL, &sig_set) == -1) { //set부분에 NULL 이므로 검사 용도로 사용한 procmask
		fprintf(stderr, "sigprocmask error\n");
		exit(1);
	}

	printf("%s", str);

	// SIGINT, SIGQUIT, SIGUSR1, SIGALRM 이 멤버인지 확인 (IQUA)
	if (sigismember(&sig_set, SIGINT))
		printf("SIGINT ");

	if (sigismember(&sig_set, SIGQUIT))
		printf("SIGQUIT ");

	if (sigismember(&sig_set, SIGUSR1))
		printf("SIGUSR1 ");

	if (sigismember(&sig_set, SIGALRM))
		printf("SIGALRM ");

	printf("\n");
	errno = err_num;
}

