#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void ssu_check_pending (int signo, char* signame);
void ssu_signal_handler (int signo);

int main(void)
{
	struct sigaction sigact;
	sigset_t sigset;

	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = ssu_signal_handler;
	
	if (sigaction(SIGUSR1, &sigact, NULL) != 0) {
		fprintf(stderr, "sigaction() error\n");
		exit(1);
	}
	else {
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGUSR1);
		
		if (sigprocmask(SIG_SETMASK, &sigset, NULL) != 0) {
			fprintf(stderr, "sigprocmask() error\n");
			exit(1);
		}
		else {
			printf("SIGUSR1 signals are now blocked\n");
			kill(getpid(), SIGUSR1);
			printf("after kill()\n");

			ssu_check_pending(SIGUSR1, "SIGUSR1");
			
			sigemptyset(&sigset);
			sigprocmask(SIG_SETMASK, &sigset, NULL);
			printf("SIGUSR1 are no longer blocked\n");
			ssu_check_pending(SIGUSR1, "SIGUSR1");
		}
	}

	exit(0);

}

void ssu_check_pending (int signo, char* signame)
{
	sigset_t pending;
	if (sigpending(&pending) < 0) 
		printf("sigpending() error\n");
	else if (sigismember(&pending, signo))
		printf("a %s signal is pending\n", signame);
	else
		printf("%s signals are not pending\n", signame);
}
void ssu_signal_handler (int signo)
{
	printf("in ssu_signal_handler function\n");
}
