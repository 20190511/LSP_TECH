#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

#define TIMEOUT 5
void redirect (char* cmd, int new, int old);
pid_t inbackground (char *name);
void time_out (char* name);

int main(int argc, char* argv[])
{
	time_out(argv[1]);
	exit(0);
}

void time_out (char* name)
{
	clock_t start, end;
	pid_t pid;
	start = time(NULL);

	while((pid = inbackground(name)) > 0)
	{
		end = time(NULL);
		if (difftime(end, start) >= 5) {
			printf("kill process : %d\n", pid);
			kill(pid, SIGKILL);
			break;
		}
		sleep(1);
	}
	return;
}

void redirect (char* cmd, int new, int old)
{
	int saved;
	int error;
	
	error = dup(STDERR_FILENO);
	dup2(new, STDERR_FILENO);
	saved = dup(old);
	dup2(new, old);

	system(cmd);

	dup2(saved, old);
	dup2(error, STDERR_FILENO);
}
pid_t inbackground (char *name)
{
	int fd;
	pid_t pid;
	char* tmp = "background.txt";
	char cmd [100];
	char file [100];

	if ((fd = open (tmp, O_RDWR | O_CREAT | O_TRUNC, 0644)) < 0) {
		fprintf(stderr, "open error for %s\n", tmp);
		exit(1);
	}

	sprintf(cmd, "ps | grep %s", name);
	redirect(cmd, fd, STDOUT_FILENO);

	lseek(fd, 0, SEEK_SET);
	if (read(fd, file, sizeof(file)) < 0) {
		close(fd);
		unlink(tmp);
		return -1;
	}
	
	pid = atoi(strtok(file, " "));
	close(fd);
	unlink(tmp);
	return pid;
}


