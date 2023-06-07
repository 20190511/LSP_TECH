#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void ssu_signal(int signo) {
	printf("SIGUSR1 catched\n");
}

int main(void)
{
	sigset_t set;
	sigset_t pending_set;
	pid_t pid;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_SETMASK, &set, NULL);
	
	signal(SIGUSR1, ssu_signal);
	kill(getpid(), SIGUSR1);

	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if (pid == 0) {
		sigpending(&pending_set);

		if (sigismember(&pending_set, SIGUSR1))
			printf("child : SIGUSR1 pending\n");
	}
	else
	{
		wait(NULL);
		sigpending(&pending_set);

		if (sigismember(&pending_set, SIGUSR1))
			printf("parent : SIGUSR1 pending\n");
	}

	exit(0);
}

