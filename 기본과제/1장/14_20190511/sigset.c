#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(void)
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, SIGINT);

	switch (sigismember(&set, SIGINT))
	{
		case 1:
			printf("SIGINT is includeed\n");
			break;
		case 0:
			printf("SIGINT is not includeed\n");
			break;
		default:
			printf("failed to call sigismember()\n");
	}

	switch (sigismember(&set, SIGSYS)) 
	{
		case 1:
			printf("SIGSYS is includeed\n");
			break;
		case 0:
			printf("SIGSYS is not includeed\n");
			break;
		default:
			printf("failed to call sigismember()\n");
	}

	exit(0);
}
		


