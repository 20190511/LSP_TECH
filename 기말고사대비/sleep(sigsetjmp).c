#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

static sigjmp_buf buf;
static void sig_alarm(int signo) {
    printf("alarm..!!!\n");
    siglongjmp(buf, 1);
}

unsigned int my_sleep(unsigned int seconds) {

    if (sigsetjmp(buf, 1) == 0) 
    {
        alarm(seconds);
        pause();
    }

    return alarm(0);
}

int main() {
    printf("Alarm Setting\n");
    signal(SIGALRM, sig_alarm);

    while (1)
    {
        printf("done\n");
        my_sleep(2);
    }
    return 0;
}
