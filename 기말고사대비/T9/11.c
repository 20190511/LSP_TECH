#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>

int ssu_daemon_init (void);
int main(void)
{
	pid_t pid;
	pid = getpid();
	printf("parent process : %d\n", pid);
	printf("daemon process initaliztion\n");

	if (ssu_daemon_init() < 0) {
		fprintf(stderr, "ssu_daemon_init failed\n");
		exit(1);
	}

	while(1) {
		syslog(LOG_INFO, "my pid is %d\n", getpid());
		closelog();
	}
	exit(0);
}

int ssu_daemon_init (void)
{
	pid_t pid;
	int fd, maxfd;	

	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if (pid != 0)
		exit(0);

	setsid();
	pid =getpid();
	printf("process %d running as daeomon\n", pid);

	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	umask(0);
	
	maxfd = getdtablesize();
	chdir("/");
	for (fd = 0 ; fd < maxfd ; fd++)
		close(fd);
	exit(0);
}


