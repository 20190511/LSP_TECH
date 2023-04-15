#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

void search_directory (char *dir) {
	struct dirent *dentry;
	struct stat statbuf;
	DIR *dirp;
	char cwd[1024];
	
	// chdir(dir); //예제에는 없지만 chdir (dir) 부터 해주고 getcwd 를 하는게 좋다.
	getcwd(cwd, 1024);

	if (lstat(dir, &statbuf) < 0) {
		fprintf(stderr, "lstat error\n");
		exit(1);
	}

	if (!S_ISDIR(statbuf.st_mode)) {
		printf("%s/%s\n", cwd, dentry->d_name); //cwd 를 기준으로 chdir 해준 상태로 간다..
		return;
	}


	if ((dirp=opendir(dir)) == NULL) {
		fprintf(stderr, "opendir error\n");
		exit(1);
	}

	chdir(dir);
	char *ptr = cwd + strlen(cwd);
	*ptr++ ='/';
	*ptr='\0';

	while ((dentry=readdir(dirp)) != NULL) {
		if (strcmp(dentry->d_name, ".") && strcmp(dentry->d_name,"..")) {
			strcpy(ptr, dentry->d_name);
			lstat(cwd, &statbuf);
			
			if (S_ISDIR(statbuf.st_mode)) {
				chdir(cwd);
				search_directory(cwd);
			}
			
			printf("%s\n", cwd);
		}
	}
}


int main(int argc, char* argv[]) 
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s filename\n",argv[0]);
		exit(1);
	}

	chdir(argv[1]);
	search_directory(argv[1]);
}

