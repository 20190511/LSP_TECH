#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define NAME_SIZE 256

int main(int argc, char* argv[])
{
	FILE *fp;
	char fname[NAME_SIZE];
	struct stat statbuf;

	printf("Enter your file name: ");
	scanf("%s", fname);
	if ((fp = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname);
		exit(1);
	}

	if (lstat(fname, &statbuf) < 0) {
		fprintf(stderr, "lstat error for %s\n", fname);
		exit(1);
	}

	if(S_ISDIR(statbuf.st_mode))
		printf("Directory file");
	else if (S_ISREG(statbuf.st_mode))
		printf("Regular file");
	else if (S_ISLNK(statbuf.st_mode))
		printf("Symbolic link");
	printf("\n");

	printf("Owner Permission: \n");
	if (statbuf.st_mode & S_IRUSR)
		printf("Read Permission bit set\n");
	if (statbuf.st_mode & S_IWUSR)
		printf("Write Permission bit set\n");
	if (statbuf.st_mode & S_IXUSR)
		printf("Execute Permission bit set\n");

	printf("File Size: %ld bytes\n", statbuf.st_size);
	printf("Last Modification Time: %s", ctime(&statbuf.st_mtime));
	fclose(fp);

	exit(0);
}
