#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>

static jmp_buf glob_buf;

static void sig_alarm (int signo)
{
	printf("signo:%d\n", signo);
	longjmp(glob_buf, 1);
}

unsigned int ssu_sleep (unsigned int seconds)
{
	if (signal(SIGALRM, sig_alarm) == SIG_ERR) {
		fprintf(stderr, "signal(SIGALRM) error\n");
		exit(1);
	}

	if (setjmp(glob_buf) == 0) {
		int ret = alarm(seconds);
		printf("ret = %d, seconds=%d\n", ret, seconds);
		if (ret < seconds) {
			alarm(0);
			alarm(seconds);
		}	
		pause();
	}
	return alarm(0);
}

int main(int argc, char* argv[])
{
	unsigned int timer = 3;
	printf("3 seconds sleeping\n");

	timer = ssu_sleep(timer);

	if (argc == 2) {
		timer = atoi(argv[1]);	
		ssu_sleep(timer);
		printf("sleeping time is changed to %s\n", argv[1]);
	}
	
}
