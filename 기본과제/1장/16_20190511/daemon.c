#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>

int ssu_daemon_init(void);

int main(void)
{
    pid_t pid;

    pid = getpid();
    printf("parent process : %d\n", pid);
    printf("daemon process initialization\n");

    if(ssu_daemon_init() < 0) { //daemon 프로세스 실행
        fprintf(stderr, "ssu_daemone_init failed\n");
        exit(1);
    }

    exit(0);
}

int ssu_daemon_init(void)
{
    pid_t pid;
    int fd, maxfd;

    if ((pid=fork()) < 0) { //1. 부모 프로세스로부터 독립한다.
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid != 0)
        exit(0);
    
    pid = getpid(); 
    printf("process %d running as daemon\n", pid);
    setsid(); //3. 부모가 init 프로세스가 되어 자신이 session 리더가 되게한다.
    signal(SIGTTIN, SIG_IGN); //4. 터미널 관련 시그널을 무시하도록 설정
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    maxfd = getdtablesize(); //7. 모든 파일 디스크립터를 닫는다.
    for (fd = 0 ; fd < maxfd ; fd++)
        close(fd);

    umask(0); //5. 파일 생성 비트마스크를 0으로 설정한다
    chdir("/"); //6.현재 작업 디렉토리를 루트로 변경 (옵션)
    fd = open("/dev/null", O_RDWR); //표준 입/출/에러 디스크립터를 /dev/null (사용안함) 으로 설정한다.
    dup(0);
    dup(0);
    return 0;
}
