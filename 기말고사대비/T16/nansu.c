#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int ch;
	FILE* fp = fopen(argv[1], "w");

	for (int i = 0 ; i < 500000 ; i++) {
		ch = i % 10;
		fprintf(fp, "%d", ch);
	}
	exit(0);
}
