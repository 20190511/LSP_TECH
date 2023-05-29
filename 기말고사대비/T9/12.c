#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void signal_handler (int signo)
{
	printf("Received signal: %d\n", signo);
}
int main(void)
{
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = signal_handler;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

	printf("Waiting for SIGINT...\n");
	sigset_t sigset;
	sigemptyset(&sigset);
	//sigaddset(&sigset, SIGINT);
	sigsuspend(&sigset);

	printf("Exiting...\n");
	exit(0);
}
