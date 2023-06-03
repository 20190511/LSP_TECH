#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NAME_SIZE 256

int main(int argc, char* argv[])
{
	FILE *fp1, *fp2, *fp3;
	char fname1[NAME_SIZE], fname2[NAME_SIZE], output[NAME_SIZE];
	char buf1[NAME_SIZE], buf2[NAME_SIZE];
	
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
		
	while (!feof(fp1)) {
		if (fgets(buf1, sizeof(buf1), fp1) == NULL)
			break;
		fputs(buf1, fp3);
	}

	while(!feof(fp2)) {
		if (fgets(buf2, sizeof(buf2), fp2) == NULL)
			break;
		fputs(buf2, fp3);
	}
	
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	exit(0);
}

