#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define NAME_SIZE 256
#define BUFFER_SIZE 1024

int main(void)
{
	FILE *fp1, *fp2, *fp3;
	char fname1[NAME_SIZE], fname2 [NAME_SIZE], temp[] = "temp.txt";
	char buf1[BUFFER_SIZE]; 
	char buf2[BUFFER_SIZE];

	printf("Enter your first file name : ");
	if (fgets(fname1 , NAME_SIZE, stdin) < 0) {
		fprintf(stderr, "fgets error\n");
		exit(1);
	}
	fname1[strlen(fname1)-1]= '\0';

	printf("Enter your second file name : ");
	if (fgets(fname2 , NAME_SIZE, stdin) < 0) {
		fprintf(stderr, "fgets error\n");
		exit(1);
	}
	fname2[strlen(fname2)-1]= '\0';

	if ((fp1 = fopen(fname1, "r")) == NULL) {
		fprintf(stderr, "fopen error\n");
		exit(1);
	}
	if ((fp2 = fopen(fname2, "r")) == NULL) {
		fprintf(stderr, "fopen error\n");
		exit(1);
	}

	if ((fp3 = fopen(temp, "w")) == NULL) {
		fprintf(stderr, "fopen error\n");
		exit(1);
	}

	int chk1 = 0, chk2 = 0;
	int check = 1;
	while(!feof(fp1) && !feof(fp2)) {

		if (fgets(buf1, BUFFER_SIZE, fp1) > 0)
		{		
			fputs(buf1, fp3);
		}
		if (fgets(buf2, BUFFER_SIZE, fp2) > 0)
		{
			fputs(buf2, fp3);
		}
	}

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);

	unlink(fname1);
	unlink(fname2);
	rename(temp, fname1);

	exit(0);
}
