#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>

void set_flags (int fd, int flags);
void clr_flags (int fd, int flags);
char buf[500000];

int main(void)
{
	int length, nwrite, ntowrite;
	char* ptr;

	ntowrite = read(STDIN_FILENO, buf, 500000);
	fprintf(stderr, "reading %d bytes\n", ntowrite);
	set_flags(STDOUT_FILENO, O_NONBLOCK);
	
	ptr = buf;	
	while (ntowrite > 0) {
		errno = 0;
		nwrite = write(STDOUT_FILENO, buf, ntowrite);
		fprintf(stderr, "nwrite = %d, errno = %d\n", nwrite, errno);

		if (nwrite > 0) {
			ptr += nwrite;
			ntowrite -= nwrite;
		}
	}
	clr_flags(STDOUT_FILENO, O_NONBLOCK);
	exit(0);
}

void set_flags (int fd, int flags)
{
	int var;
	if ((var = fcntl(fd, F_GETFL, 0)) == -1) {
		fprintf(stderr, "fcntl F_GETFL failed\n");
		exit(1);
	}

	var |= flags;
	
	if (fcntl(fd, F_SETFL, var) == -1) {
		fprintf(stderr, "fcntl F_SETFL failed\n");
		exit(1);
	}
}
void clr_flags (int fd, int flags)
{
	int var;
	if ((var = fcntl(fd, F_GETFL, 0)) == -1) {
		fprintf(stderr, "fcntl F_GETFL failed\n");
		exit(1);
	}

	var &= ~flags;
	
	if (fcntl(fd, F_SETFL, var) == -1) {
		fprintf(stderr, "fcntl F_SETFL failed\n");
		exit(1);
	}

}
