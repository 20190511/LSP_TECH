#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>


#define DIRECTORY_SIZE	MAXNAMLEN

int main(int argc, char* argv[])
{
	struct dirent* dentry;
	struct stat statbuf;
	char filename[DIRECTORY_SIZE];
	DIR *dirp;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <DIRECTORY>\n", argv[0]); //error 처리
		exit(1);
	}

	if ((dirp = opendir(argv[1]))== NULL || chdir(argv[1])==-1) { //디렉토리가 열리지 않거나, 해당 디렉토리로 이동할 수 없는 경우
		fprintf(stderr, "opendir, chdir error for %s\n", argv[1]);
		exit(1);
	}

	while((dentry = readdir(dirp)) != NULL) {	//재귀적으로 해당 디렉토리 탐색.
		if (dentry->d_ino == 0)
			continue;

		memcpy(filename, dentry->d_name, DIRECTORY_SIZE);

		if (stat(dentry->d_name, &statbuf) == -1) {
			fprintf(stderr, "stat error for %s\n", filename);
			exit(1);
		}

		if((statbuf.st_mode & S_IFMT) == S_IFREG)
			printf("%-14s %ld\n", filename, statbuf.st_size);
		else
			printf("%-14s\n", filename);
	}

	exit(0);
}
