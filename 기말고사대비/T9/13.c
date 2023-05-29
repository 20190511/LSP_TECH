#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

static jmp_buf glob_buf;

static void signal_handler (int signo) {
	longjmp(glob_buf,1);
}


int main(void)
{
	printf("before sleep\n");

	if (signal(SIGALRM, signal_handler) == SIG_ERR) {
		fprintf(stderr, "signal error\n");
		exit(1);
	}

	if (setjmp(glob_buf) == 0) {
		int ret = alarm(10);
		if (ret < 10) {
			alarm(0);
			alarm(10);
		}

		pause();
	}
	printf("after sleep\n");
	exit(0);

}
