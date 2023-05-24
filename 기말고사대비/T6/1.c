#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>


#define NAME_SIZE 256
int main(void)
{
	FILE *fp;
	char fname[NAME_SIZE];
	struct stat statbuf;
	
	printf("Enter your file name : ");
	if (fgets(fname, NAME_SIZE, stdin) < 0) {
		fprintf(stderr, "fgets error\n");
		exit(1);
	}
	
	fname[strlen(fname)-1] = '\0';
	
	if (lstat(fname, &statbuf) < 0) {
		fprintf(stderr, "lstat error\n");
		exit(1);
	}

	char* type;
	if (S_ISREG(statbuf.st_mode))
		type = "Regular file";
	else if (S_ISDIR(statbuf.st_mode))
		type = "Directory file";
	else if (S_ISLNK(statbuf.st_mode))
		type = "Symbolic link";
	
	printf("File Type : %s\n", type);
	printf("Owner Permission:\n");
	
	if (S_IRUSR & statbuf.st_mode)
		printf("Read Permission bit set\n");
	if (S_IWUSR & statbuf.st_mode)
		printf("Write Permission bit set\n");
	if (S_IXUSR & statbuf.st_mode)
		printf("Execute Permission bit set\n");
	
	printf("File Size : %ld bytes\n", statbuf.st_size);
	printf("Last Modification Time : %s", ctime(&statbuf.st_mtime));


	exit(0);

}
