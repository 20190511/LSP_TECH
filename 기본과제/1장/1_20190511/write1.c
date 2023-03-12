#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE	 1024
int main (void)
{
	char buf[BUFFER_SIZE];
	int length;

	length = read(0, buf, BUFFER_SIZE);
	write(1, buf, length);
	exit(0);
}
