#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>

static jmp_buf glob_buf;

void ssu_alarm (int signo);
unsigned int my_sleep (unsigned int timer);

int main(void)
{
	printf("Alarm Setting\n");
	signal(SIGALRM, ssu_alarm);

	while(1) {
		printf("done\n");
		my_sleep(2);
	}
	exit(0);
}

void ssu_alarm (int signo)
{
	printf("alarm..!!!\n");
	longjmp(glob_buf, 1);
}
unsigned int my_sleep (unsigned int timer)
{
	if (setjmp(glob_buf) == 0) {
		alarm(timer);
		pause();
	}

	sigset_t set;
	sigemptyset(&set);
	sigprocmask(SIG_SETMASK, &set, NULL);

	return alarm(0);
}


