#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#define NAME_SIZE 256
int main(void)
{
	FILE *fp;
	char fname[NAME_SIZE];
	char* type;
	struct stat statbuf;

	printf("Enter your file name : ");
	scanf("%s", fname);
	
	if ((fp = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname);
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	int f_size = ftell(fp);
	if (lstat(fname, &statbuf) < 0) {
		fprintf(stderr, "lstat error for %s\n", fname);
		exit(1);
	}
		
	printf("File Type : ");
	if (S_ISREG(statbuf.st_mode))
		type = "Regular file";
	else if (S_ISDIR(statbuf.st_mode))
		type = "Directory file";
	else if (S_ISLNK(statbuf.st_mode))
		type = "Symbolic link";

	printf("%s\n", type);

	printf("Owner Permission:\n");
	
	if (S_IRUSR & statbuf.st_mode)
		printf("Read Permission bit set\n");
	if (S_IWUSR & statbuf.st_mode)
		printf("Write Permission bit set\n");
	if (S_IXUSR & statbuf.st_mode)
		printf("Execute Permission bit set\n");

	//f_size = statbuf.st_size;
	printf("File size: %d bytes\n", f_size);
	printf("Last Modification: %s", ctime(&statbuf.st_mtime));
	fclose(fp);

	exit(0);
}



