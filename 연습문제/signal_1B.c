#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(void)
{
    pid_t pid;

    if((pid = fork()) < 0)
    {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid == 0)
    {
        char* args[] = {"./signal_1A", "process", "running", NULL};
        execv(args[0], args);
        
        if (execv(args[0], args) != 0) {
            fprintf(stderr, "execv error\n");
            exit(1);
        }
    }
    else
        sleep(3);
    
    kill(pid, SIGINT);
    exit(0);
}