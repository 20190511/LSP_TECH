#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
	sigset_t sig_set;
	sigset_t old_set;
	
	sigemptyset(&sig_set); //sigset을 다 비우고 SIGINT만 BLOCK 시킴
	sigaddset(&sig_set, SIGINT);
	sigprocmask(SIG_BLOCK, &sig_set, &old_set);
	sigsuspend(&old_set); //SIGINT 블락을 해제하고 SIGINT가 올 때까지 Pause();

	exit(0);
}

