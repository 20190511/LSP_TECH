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
	char fname1[NAME_SIZE], fname2 [NAME_SIZE], output[NAME_SIZE];
	char buf[BUFFER_SIZE]; 

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

	printf("Enter your destination file name: ");
	if (fgets(output , NAME_SIZE, stdin) < 0) {
		fprintf(stderr, "fgets error\n");
		exit(1);
	}
	output[strlen(output)-1]= '\0';

	if ((fp1 = fopen(fname1, "r")) == NULL) {
		fprintf(stderr, "fopen error\n");
		exit(1);
	}
	if ((fp2 = fopen(fname2, "r")) == NULL) {
		fprintf(stderr, "fopen error\n");
		exit(1);
	}
	if ((fp3 = fopen(output, "w")) == NULL) {
		fprintf(stderr, "fopen error\n");
		exit(1);
	}

	int length;	
	while(!feof(fp1)) {
		if (fgets(buf, BUFFER_SIZE , fp1) == NULL)
			break;
		fputs(buf, fp3);
	}
	while(!feof(fp2)) {
		if (fgets(buf, BUFFER_SIZE, fp2) == NULL)
			break;
		fputs(buf, fp3);
	}
	
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	exit(0);
}
