#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char* argv[])
{
	pid_t pid;
	char character, first, last;
	long i;

	if((pid=fork()) > 0) { //부모 프로세스면
		first='A';
		last='Z';
	}
	else if (pid == 0) { //자식 프로세스면
		first ='a';
		last ='z';
	}
	else {
		fprintf (stderr, "%s\n", argv[0]);
		exit(1);
	}


	for (character=first ; character < last ; character++) { //자식프로세스와 부모프로세스의 race 상태 유발
		for (i = 0 ; i <= 100000 ; i++);
		write(1, &character, 1);
	}

	printf("\n");
	exit(0);
}

