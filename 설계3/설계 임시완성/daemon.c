#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#include <fcntl.h>


// 일반적으로 ./경로 <호출한 디몬 pid> <모니터링할 경로: target_path> [<-t> <TIME>] <ssu_monitoring 의 log.txt 경로> 를 인자로 받는다.

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

#ifndef PROMPTLEN
#define PROMPTLEN 1024
#endif

#ifndef PIDSIZE
#define PIDSIZE 10
#endif


int daemon_init();
void handle_sigusr1(int signo);
void monitor_setting();
char daemon_pid [PIDSIZE];
char child_path [MAXPATHLEN];
char* args[7] = {NULL, };
pid_t child_pid = -1;


int main(int argc, char* argv[])
{
    pid_t pid;
    char* ptr;
    int i = 0;
    
    for (int i = 0 ; i < argc ; i++)
        args[i] = argv[i];
    

    pid = getpid();
    getcwd(child_path, MAXPATHLEN); //일단 자기 경로를 받아와서 수정 예정.
    ptr = child_path + strlen(child_path);
    *ptr++ = '/';
    strcpy(ptr, "daemon_monitor");
    args[0] = child_path;
    
    if(daemon_init() < 0) {
        fprintf(stderr, "daemon failed\n");
        exit(1);
    }
    
    exit(0);
}

void handle_sigusr1(int signo)
{
    kill(child_pid, SIGUSR1); // 자식 프로세스 (모니터링 프로세스한테 종료 시그널 보냄)
    exit(0);
}


int daemon_init() {
    pid_t pid;
    sigset_t set;
    int status;
    int fd_cnt;

    
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR){
        fprintf (stderr, "SIGUSR1 can not catch\n");
        exit(1);
    }

    // 1. 자식 프로세스 독립 (백그라운드 실행)
    if((pid=fork()) < 0) {
        fprintf(stderr, "fork error\n");
        return false;
    }
    else if (pid != 0)
        exit(0);
    
    sprintf(daemon_pid, "%d", getpid());
    //printf("process pid = %s\n", daemon_pid);
    args[1] = daemon_pid;
    
    // 7. 파일 디스크립터 모두 닫기 --> /dev/null
/*
    fd_cnt = getdtablesize();
    for (int i = 0 ; i < fd_cnt ; i++)
        close(i);
    
    // 2. 터미널 모두 날려버리고 표준입/출/에러 다 /dev/null로 만들기.
    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);
*/
    // 3. 자식을 프로세스 init 프로세스를 부모로 삼도록 함.
    setsid();
    
    // 4. 터미널 관련 시그널을 모두 무시하도록함.
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    // 5. umask를 0으로 만들어버리기 (파일비트 마스크 해제)
    umask(0);
    // 6. chdir("/") 은 일단 안함


    // 디몬 모니터링 프로세스 실행
    child_pid = fork();
    if (child_pid == 0)
    {
        execv(args[0], args);
    }
    else
        while(1);
    exit(0);
}
