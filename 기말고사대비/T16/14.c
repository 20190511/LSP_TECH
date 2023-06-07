#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_CALL 100
#define MAX_BUF 200000
void serror (char* str) {
	fprintf(stderr, "%s\n", str);
	exit(1);
}

int main(void)
{
	int nread, nwrite, flag, i = 0;
	int call[MAX_CALL];
	char buf [MAX_BUF];
	char* ptr;

	nread = read(STDIN_FILENO, buf, sizeof(buf));
	fprintf(stderr, "read %d bytes\n", nread);
	
	if ((flag = fcntl(STDOUT_FILENO, F_GETFL, 0)) <0 ) 
		serror("fcntl F_GETFL error");

	flag |= O_NONBLOCK;
	
	if (fcntl(STDOUT_FILENO, F_SETFL, flag) < 0 ) 
		serror("fcntl F_SETFL error");



	for (ptr = buf ; nread > 0 ; i++) {
		errno = 0;
		nwrite = write(STDOUT_FILENO, buf, nread);

		if (i < MAX_CALL) 
			call[i] = nwrite;

		fprintf(stderr, "nwrite = %d, errno = %d\n", nwrite, errno);

		if (nwrite > 0) {
			nread -= nwrite;
			ptr += nwrite;
		}
	}


	if ((flag = fcntl(STDOUT_FILENO, F_GETFL, 0)) <0 ) 
		serror("fcntl F_GETFL error");

	flag &= ~O_NONBLOCK;
	
	if (fcntl(STDOUT_FILENO, F_SETFL, flag) < 0 ) 
		serror("fcntl F_SETFL error");

	for (i = 0 ; i < MAX_CALL ; i++) 
		fprintf(stdout, "call[%d] = %d\n", i, call[i]);
	exit(0);
}
