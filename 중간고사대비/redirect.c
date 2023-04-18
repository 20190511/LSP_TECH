#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char* argv[])
{

	int fd; 
	if (argc < 3) {
		fprintf(stderr, "usage: %s <process> <file>\n", argv[0]);
		exit(1);
	}

	struct stat statbuf;
	if (stat(argv[1], &statbuf) < 0) {
		fprintf(stderr, "stat error\n");
		exit(1);
	}

	
	if (!(statbuf.st_mode & S_IXUSR)) {
		fprintf(stderr, "this file is not process file\n");
		exit(1);
	}

	if((fd=open(argv[argc-1], O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[argc-1]);
		exit(1);
	}

	dup2(1, 4);
	dup2(fd, 1);

	pid_t pid;
	if ((pid = fork()) < 0) {
		fprintf(stderr, "pid error\n");
		exit(1);
	}
	else if (pid == 0) {
		char** agv = (char**)malloc(sizeof(char*) * argc-1);
		for (int i = 1 ; i < argc-1 ; i++) {
			agv[i-1] = argv[i];
		}
		agv[argc-1] = NULL; 
		execv(argv[1], agv);
	}
	else{
		pid = wait(NULL);
		dup2(4,1);
		printf("finished\n");
	}
	exit(0);
}

	
