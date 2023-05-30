#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

void set_flags (int fd, int flags);
void clr_flags (int fd, int flags);
char buf[500000];
int main(void)
{
	int ntowrite, nwrite;
	char* ptr;

	ntowrite = read(STDIN_FILENO, buf, sizeof(buf));
	fprintf(stderr, "reading %d bytes\n", ntowrite);
	
	set_flags(STDOUT_FILENO, O_NONBLOCK);

	ptr = buf;
	while(ntowrite > 0) {
		errno = 0;
		nwrite = write(STDOUT_FILENO, buf, ntowrite);
		fprintf(stderr, "nwrite = %d, errno = %d\n", nwrite, errno);	

		if (nwrite > 0) {
			ntowrite -= nwrite;
			ptr += nwrite;
		}

	}
	
	clr_flags(STDOUT_FILENO, O_NONBLOCK);
	exit(0); 
}

void set_flags (int fd, int flags)
{
	int var;
	
	if((var = fcntl(fd, F_GETFL, 0)) == -1) {
		fprintf(stderr, "fcntl F_GETFL failed\n");
		exit(1);
	}

	var |= flags;

	if(fcntl(fd, F_SETFL, var) == -1) {
		fprintf(stderr, "fcntl F_SETFL failed\n");
		exit(1);
	}

}
void clr_flags (int fd, int flags)
{
	int var;
	
	if((var = fcntl(fd, F_GETFL, 0)) == -1) {
		fprintf(stderr, "fcntl F_GETFL failed\n");
		exit(1);
	}

	var &= ~flags;

	if(fcntl(fd, F_SETFL, var) == -1) {
		fprintf(stderr, "fcntl F_SETFL failed\n");
		exit(1);
	}
}
