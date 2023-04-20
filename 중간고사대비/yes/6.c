#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

void search_directory (char *dir) {
	struct dirent* dentry;
	DIR* dp;
	struct stat statbuf;
	char cwd[1024];

	chdir(dir);
	getcwd(cwd, 1024);

	if (lstat(dir, &statbuf) < 0) {
		fprintf(stderr, "stat error\n");
		exit(1);
	}

	if (!S_ISDIR(statbuf.st_mode)) {
		sprintf(cwd, "%s/%s", getcwd(NULL,0), dir);
		printf("%s\n", cwd);
		return;
	}


	if ((dp = opendir(cwd)) == NULL) {
		fprintf(stderr, "opendir error \n");
		exit(1);
	}
	
	char *ptr = cwd + strlen(cwd);
	*ptr++ = '/';
	*ptr = '\0';

	while ((dentry = readdir(dp)) != NULL) {
		if (strcmp(dentry->d_name, ".") && strcmp(dentry->d_name, "..")) {
			strcpy(ptr, dentry->d_name);
			
			if (stat(cwd, &statbuf) < 0) {
				fprintf(stderr, "stat error\n");
				exit(1);
			}

			if (!S_ISDIR(statbuf.st_mode)) {
				chdir (cwd);
				search_directory(cwd);
			}
			printf("%s\n", cwd);
		}
	}
	closedir(dp);
	return;
}


int main(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <file>\n", argv[0]);
		exit(1);
	}

	chdir(argv[1]);
	search_directory(argv[1]);
	exit(1);

}


