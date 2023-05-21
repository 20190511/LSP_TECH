#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void sigint_handler(int signo) {
    printf("\nSIGINT handler!!\n");
    exit(0);
}

int main(int argc, char* argv[])
{
    int i;
    signal(SIGINT, sigint_handler);

    while (1) {
        for (i = 0 ; i < argc ; i++)
            printf("ARGV[%d] : %s\n", i, argv[i]);
        sleep(1);
    }

    exit(0);
}