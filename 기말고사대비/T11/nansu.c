#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char* argv[])
{
	FILE* fp;
	fp = fopen(argv[1], "w");

	for (int i = 0 ;i < 500000; i++) {
		fprintf(fp, "%d", (i*i)%10);
	}

	fclose(fp);
	exit(0);
}
