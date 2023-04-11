#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PATH_MAX	1024

int main(void)
{
	char *pathname;


	if(chdir("/home/oslab") < 0) { //  /home/oslab 으로 작업디렉토리 변경
		fprintf(stderr, "chdir error\n");
		exit(1);
	}


	pathname = malloc(PATH_MAX);

	if (getcwd(pathname, PATH_MAX) == NULL) {
		fprintf(stderr, "getcwd error\n");
		exit(1);
	}

	printf("current directory = %s\n", pathname);
	exit(0);
}

