#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char* argv[])
{
	struct flock lock;
	int fd;
	char command[100];

	if((fd = open(argv[1], O_RDWR)) == -1) {
		perror(argv[1]); //perror 사용
		exit(1);
	}

	lock.l_type = F_WRLCK; //Write Lock 설정
	lock.l_whence = 0;
	lock.l_start = 0l;
	lock.l_len = 0l;

	while (fcntl(fd, F_SETLK, &lock) == -1) {
		if (errno == EACCES) {
			printf("%s busy -- try later\n", argv[0]);
			exit(2);
		}
		perror(argv[1]);
		exit(3);
	}

	sprintf(command, "vim %s\n", argv[1]);
	system (command);
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
	
	return 0;
}

