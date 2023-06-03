#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NAME_SIZE 256

int main(int argc, char* argv[])
{
	FILE *fp1, *fp2, *fp3;
	char fname1[NAME_SIZE], fname2[NAME_SIZE];
	char buf1[NAME_SIZE], buf2[NAME_SIZE];
	
	printf("Enter your first file name: ");
	scanf("%s", fname1);
	printf("Enter your second file name: ");
	scanf("%s", fname2);

	if ((fp1 = fopen(fname1, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname1);
		exit(1);
	}

	if ((fp2 = fopen(fname2, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname2);
		exit(1);
	}

	int check = 1;
	while (!feof(fp1) && !feof(fp2)) {
		char* a = fgets(buf1, sizeof(buf1), fp1);
		char* b = fgets(buf2, sizeof(buf2), fp2);
		
		if (strcmp(buf1, buf2)) {
			printf("Given two files are not identical\n");
			check = 0;
			break;
		}
			
		if((!feof(fp1) && feof(fp2)) || (feof(fp1) && !feof(fp2))) {
			printf("Given two files are not identical\n");
			check = 0;
			break;
		}

	}
	if (check)
		printf("Given two file have identical contents\n");
	fclose(fp1);
	fclose(fp2);
	exit(0);
}

