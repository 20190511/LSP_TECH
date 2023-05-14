#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>

static void ssu_alarm(int signo);
static void ssu_func (int signo);
void ssu_mask (const char *str);

static volatile sig_atomic_t can_jump;
static sigjmp_buf jump_buf;

int main(void)
{
	if (signal(SIGUSR1, ssu_func) == SIG_ERR) { //SIGUSR2 에러처리
		fprintf(stderr, "SIGUSR1 error\n");
		exit(1);
	}

	if (signal(SIGALRM, ssu_alarm) == SIG_ERR) { //SIGALRM 에러처리
		fprintf(stderr, "SIGALRM error\n");
		exit(1);
	}
	
	ssu_mask("starting main: ");
	
	if (sigsetjmp(jump_buf, 1)) { //savesig가 0이 아니기 때문에 현재 bitmask도 jmup_buf에 저장
		ssu_mask("ending main: ");
		exit(0);
	}

	can_jump = 1;

	while(1)
		pause(); //시그널 올 때까지 pause

	exit(0);
}

void ssu_mask(const char* str) {
	sigset_t sig_set;
	int err_num;

	err_num = errno;

	if (sigprocmask(0, NULL, &sig_set) < 0 ){ //sigprocmask로 검사하는 예제
		printf("sigprocmask() error");
		exit(1);
	}

	printf("%s", str); 
	//해당 sigset 멤버가 SIGINT, SIGQUIT, SIGUSR1, SIGALRM 이 존재하는지 확인
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
static void ssu_func (int signo) {
	time_t start_time;

	if (can_jump == 0)
		return;

	ssu_mask("starting ssu_func: ");
	alarm(3);
	start_time = time(NULL); //현재시간을 받아옴
	
	while(1)
		if(time(NULL) > start_time + 5) //시간을 재출력해보니까 현재시간+5 보다 크면 break
			break;

	ssu_mask("ending ssu_func: "); //현재 masking되어있는 상태확인
	can_jump = 0;
	siglongjmp(jump_buf, 1);
}

static void ssu_alarm(int signo) {
	ssu_mask("in ssu_alarm: "); //mask handling
}

