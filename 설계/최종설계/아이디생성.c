#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <wait.h>

int main(void)
{
    pid_t pid;
    int status;
    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "Fork Error\n");
        exit(1);
    }

    if (pid != 0)
    {
        sleep(1);
        wait(&status);
    }
    else
    {
        //아이디 생성
        if (execl ("/usr/sbin/useradd", "/usr/sbin/useradd", "-m", "junhyeong2", NULL) == -1)
        {
            printf("execve error\n");
            exit(1);
        }
    }
    exit(0);
}
