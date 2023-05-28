#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_CALL 100
#define MAX_BUF 20000

void serror (char *str)
{
	fprintf(stderr, "%s\n", str);
	exit(1);
}

int main(void)
{
	int nread, nwrite, val, i = 0;
	char *ptr;
	char buf[MAX_BUF];
	int call[MAX_CALL];

	nread = read(STDIN_FILENO, buf, sizeof(buf));
	fprintf(stderr, "read %d bytes\n", nread);
	
	if((val = fcntl(STDOUT_FILENO, F_GETFL, 0)) < 0) 
		serror("fcntl FD_GETFL error");
		
	val |= O_NONBLOCK;
	
	if(fcntl(STDOUT_FILENO,F_SETFL, val) < 0) 
		serror("fcntl FD_SETFL error");

	for (ptr = buf ; nread > 0 ; i++) {
		errno = 0;
		nwrite = write(STDOUT_FILENO, ptr, nread);
		
		if (i < MAX_CALL)
			call[i] = nwrite;

		fprintf(stderr, "nwrite = %d, errno = %d\n", nwrite, errno);
		if (nwrite > 0) {
			ptr += nwrite;
			nread -= nread;
		}
	}

	if ((val = fcntl(STDOUT_FILENO, F_GETFL, 0)) < 0) 
		serror("fcntl FD_GETFL error");

	val &= ~O_NONBLOCK;
	
	if (fcntl(STDOUT_FILENO, F_SETFL, val) < 0)
		serror("fcntl FD_SETFL error");

	for (i = 0 ; i < MAX_CALL ; i++)
		fprintf(stderr, "call[%d] = %d\n", i, call[i]);

	exit(0);
}
