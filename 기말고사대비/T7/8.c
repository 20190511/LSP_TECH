#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>

int ssu_daemon_init(void);

int main(void)
{
	pid_t pid;
	pid = getpid();
	printf("parent process : %d\n", pid);
	printf("daemon process initalization\n");
	
	if (ssu_daemon_init() < 0) {
		fprintf(stderr, "ssu_daemon_init failed\n");
		exit(1);
	}

	while (1)
	{
		syslog(LOG_PID, "my pid is %d\n", getpid());
		closelog();
		sleep(5);
	}
	
	exit(0);
}

int ssu_daemon_init(void)
{
	pid_t pid;
	int fd, maxfd;
	
	if((pid = fork()) < 0) {
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if (pid != 0)
		exit(0);
	
	setsid();
	pid = getpid();
	printf("process %d running as daemon\n", pid);

	maxfd = getdtablesize();
	for (fd = 0 ; fd < maxfd ; fd++)
		close(fd);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
		
	umask(0);
	chdir("/");
	
	fd = open("/dev/null", O_RDWR);
	dup(0);
	dup(0);
	return 0;
}

