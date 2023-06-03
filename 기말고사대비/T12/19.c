#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main (int argc, char* argv[])
{
	char cmd[100];
	int fd;
	struct flock lock;
	
	if ((fd = open (argv[1], O_RDWR)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	lock.l_type = F_WRLCK;
	lock.l_whence = 0;
	lock.l_start = 0l;
	lock.l_len = 0l;

	if (fcntl(fd, F_SETLK, &lock) == -1) {
		if (errno == EACCES) {
			fprintf(stderr, "%s busy -- try later\n", argv[1]);
			exit(2);
		}
		perror(argv[1]);
		exit(3);
	}

	sprintf(cmd, "vim %s\n", argv[1]);
	system(cmd);
	
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	exit(0);
}

