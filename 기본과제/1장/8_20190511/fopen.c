#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	char *fname = "ssu_test.txt";
	char *mode = "r";

	if (fopen(fname, mode) == NULL) { //fname 으로 open
		fprintf(stderr, "fopen error for %s\n", fname);
		exit(1);
	}
	else 
		printf("Success!\nFilename: <%s>, mode: <%s>\n", fname, mode);

	exit(0);
}
