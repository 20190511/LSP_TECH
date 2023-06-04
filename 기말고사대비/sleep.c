#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

static jmp_buf buf;
static void sig_alarm(int signo) {
    printf("alarm..!!!\n");
    longjmp(buf, 1);
}

unsigned int my_sleep(unsigned int seconds) {

    if (setjmp(buf) == 0) 
    {
        alarm(seconds);
        pause();
    }
    
    sigset_t set;
    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);
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
