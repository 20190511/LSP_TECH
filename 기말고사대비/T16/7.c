#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NAME_SIZE 256

int main (int argc, char* argv[])
{
	FILE* fp1, *fp2, *fp3;
	char fname1[NAME_SIZE], fname2[NAME_SIZE], temp[] = "temp.txt";
	char buf1[NAME_SIZE], buf2[NAME_SIZE];
	
	printf("Enter your first file name: ");
	fgets(fname1, NAME_SIZE, stdin);
	fname1[strlen(fname1)-1] = '\0';
	
	printf("Enter your second file name: ");
	fgets(fname2, NAME_SIZE, stdin);
	fname2[strlen(fname2)-1] = '\0';

	if ((fp1 = fopen(fname1, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname1);
		exit(1);
	}
	if ((fp2 = fopen(fname2, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname2);
		exit(1);
	}

	if ((fp3 = fopen(temp, "w")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", temp);
		exit(1);
	}

	while(!feof(fp1) || !feof(fp2)) {
		fgets(buf1, NAME_SIZE, fp1);
		if (!feof(fp1)) 
			fputs(buf1, fp3);
		
		fgets(buf2, NAME_SIZE, fp2);
		if (!feof(fp2)) 
			fputs(buf2, fp3);

		memset(buf1, 0, NAME_SIZE);
		memset(buf2, 0, NAME_SIZE);
	}

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);

	remove(fname1);
	remove(fname2);
	rename(temp, fname1);
	exit(0);
}
