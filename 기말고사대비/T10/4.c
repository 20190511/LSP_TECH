#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

#define MAX_CALL 100
#define MAX_BUF 20000
void serror (char* str)
{
	fprintf(stderr, "%s\n", str);
	exit(1);
}

int main(void)
{
	int fd, var, nread, nwrite;
	int i = 0;
	char* ptr;
	char buf[MAX_BUF];
	int call[MAX_CALL];

	nread = read(STDIN_FILENO, buf, sizeof(buf));
	fprintf(stderr, "read %d bytes\n", nread);
	
	if ((var = fcntl(STDOUT_FILENO, F_GETFL, 0)) < 0) 
		serror("fcntl F_GETFL error");
	
	var |= O_NONBLOCK;

	if (fcntl(STDOUT_FILENO, F_SETFL, var) < 0)
		serror("fcntl F_SETFL error");

	for (ptr = buf ; nread > 0 ; i++) {
		errno = 0;
		nwrite = write (STDOUT_FILENO, buf, nread);
		fprintf(stderr, "nwrite = %d, errno = %d\n", nwrite, errno);
	
		if (i < MAX_CALL) 
			call[i] = nwrite;

		if (nwrite > 0) {
			nread -= nwrite;
			ptr += nwrite;
		}
	}
	
	if ((var = fcntl(STDOUT_FILENO, F_GETFL, 0)) < 0) 
		serror("fcntl F_GETFL error");
	
	var &= ~O_NONBLOCK;

	if (fcntl(STDOUT_FILENO, F_SETFL, var) < 0)
		serror("fcntl F_SETFL error");

	for (i = 0 ; i < MAX_CALL ; i++)
		printf("call[%d] = %d\n", i, call[i]);

	exit(0);
	
}
