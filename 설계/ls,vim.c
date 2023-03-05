#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    char buf [500];
    int status;
    fgets(buf, 500, stdin);
    pid_t pd = fork();
    pid_t pd2 = fork();
    if (pd == 0)
    {
        if (execl("/usr/bin/ls", "ls", "-a", NULL) == -1)
        {
            printf("execve error\n");
            return 1;
        }
    }

    if (pd2 == 0)
    {
        if (execl("/usr/bin/vim", "vim", buf, NULL) == -1)
        {
            printf("execve error\n");
            return 1;
        }
    }
    
    wait(&status);
    wait(&status);
    exit(0);
}
