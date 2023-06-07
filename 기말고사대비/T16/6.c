#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NAME_SIZE 256

int main (int argc, char* argv[])
{
	FILE* fp1, *fp2, *fp3;
	char fname1[NAME_SIZE], fname2[NAME_SIZE], output[NAME_SIZE];
	
	printf("Enter your first file name: ");
	fgets(fname1, NAME_SIZE, stdin);
	fname1[strlen(fname1)-1] = '\0';
	
	printf("Enter your second file name: ");
	fgets(fname2, NAME_SIZE, stdin);
	fname2[strlen(fname2)-1] = '\0';

	printf("Enter your destination file name: ");
	fgets(output, NAME_SIZE, stdin);
	output[strlen(output)-1] = '\0';

	if ((fp1 = fopen(fname1, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname1);
		exit(1);
	}
	if ((fp2 = fopen(fname2, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname2);
		exit(1);
	}

	if ((fp3 = fopen(output, "w")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", output);
		exit(1);
	}

	while(!feof(fp1)){
		int ch = fgetc(fp1);
		if (ch != EOF)
			fputc(ch, fp3);
	}

	while(!feof(fp2)){
		int ch = fgetc(fp2);
		if (ch != EOF)
			fputc(ch, fp3);
	}

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	exit(0);
}
