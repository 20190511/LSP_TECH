#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>


#ifdef PATH_MAX
static int pathmax = PATH_MAX;
#else
static int pathmax = 0;
#endif


#define MAX_PATH_GUESSED 1024

#ifndef LINE_MAX
#define LINE_MAX 2048
#endif

char *pathname;
char command[LINE_MAX], grep_cmd[LINE_MAX];

int ssu_do_grep(void) {
	struct dirent* dirp;
	struct stat statbuf;
	char *ptr;
	DIR* dp;

	if (lstat(pathname, &statbuf) < 0) { //directory information searching
		fprintf(stderr, "lstat error for %s\n", pathname);
		return 0;
	}

	if (S_ISDIR(statbuf.st_mode) == 0) { //directory file is not --> do command
		sprintf(command, "%s %s", grep_cmd, pathname);
		printf("%s : \n", command);
		system(command);
		return 0;
	}

	ptr = pathname + strlen(pathname);
	*ptr++ = '/';
	*ptr='\0';

	if ((dp = opendir(pathname)) == NULL) { // dirent := opendir(path)
		fprintf(stderr, "opendir error for %s\n", pathname);
		return 0;
	}
	while ((dirp=readdir(dp)) != NULL) //ptr 에 계속 경로를 변경해가며 추가하며 재귀호출
		if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..")) {
			strcpy(ptr, dirp->d_name);

			if (ssu_do_grep() < 0)
				break;
		}
	
	ptr[-1] = 0; //출력이 모두 종료되었으므로 ptr[-1] = 0 으로 원본 복구
	closedir(dp);
	return 0;
}


void ssu_make_grep (int argc, char* argv[]) {
	int i;
	strcpy(grep_cmd, "grep");

	for (i = 1 ;i < argc-1;i++) {//argv 받은 인자를 한 줄로 바꾸는 과정
		strcat(grep_cmd, " ");
		strcat(grep_cmd, argv[i]);
	}
}	


int main(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <-CVbchilnsvwx> <-num> <-Anum> <-Bnum> <-f file> \n"
				        "<-e> expr <directory>\n", argv[0]);
		exit(1);
	}

	if (pathmax == 0) {
		if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0)  //pathconf == root 경로에 있는 _PC_PATH_MAX value 값을 얻어옴
			pathmax = MAX_PATH_GUESSED;
		else
			pathmax++;
	}

	if ((pathname = (char*)malloc(pathmax+1)) == NULL) { //전역변수 동적할당.
		fprintf(stderr, "malloc error\n");
		exit(1);
	}

	strcpy(pathname, argv[argc-1]);
	ssu_make_grep(argc, argv);
	ssu_do_grep();
	exit(0);
}




