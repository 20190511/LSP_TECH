#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include <openssl/md5.h>
#include <fcntl.h>

#define STRMAX 100
#define PATHMAX 4096
#define HASHMAX 33
#define DUPMAX 20

// 교훈1. hash_func 를통해 해시 만드는과정
// 교훈2. 


int hash_func(char* filename, char* hash);
void print_path (char* dirpath, char* hash);
void delete_path (char* dirpath, char* hash);
void time_print (char* path);
void makePath (char* path);

int main(int argc, char* argv[])
{
	char hash[HASHMAX];
	char path1[PATHMAX];
	char path2[PATHMAX];
	// 실수2. argv 바로 편집하면안됨, 문자열을 복사하고 할 것.
	strcpy(path1, argv[1]);
	strcpy(path2, argv[2]);
	makePath(path1);
	makePath(path2);
	hash_func(path2, hash);
	printf("=== Duplicates ( %s ) ===\n", hash);
	print_path(path1, hash);
	printf("\n");
	delete_path(path1, hash);
	printf("\n=== Left Files ( %s ) ===\n", hash);
	print_path(path1, hash);
	
	exit(0);
}



void makePath (char* path)
{
	char cwd [1024];
	char tmp [PATHMAX];
	getcwd(cwd, sizeof(cwd));
	strcpy(tmp, path);
	if (path[0] != '/')
	{
		sprintf(tmp, "%s%s", cwd, path+1);
	}
	strcpy(path, tmp);
}

int hash_func(char* filename, char* hash)
{
	MD5_CTX md5;
	int fd;
	int length;
	char buf[1024*24];
	char tmp_hash [DUPMAX];

	if((fd=open(filename, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", filename);
		exit(1);
	}

	MD5_Init(&md5);
	while ((length=read(fd, buf, sizeof(buf))) > 0) {
		MD5_Update(&md5, buf,length);
	}
	MD5_Final(tmp_hash, &md5);

	for (int i = 0 ; i < DUPMAX ; i++){
		sprintf(hash+(i*2), "%02x", tmp_hash[i]);
	}
	
	//실수 3. 해시 받고나서 문자열 처리해줄 것.
	hash[HASHMAX-1] = '\0';
}
void print_path (char* dirpath, char* hash)
{
	char cp_hash[HASHMAX];
	char filename[PATHMAX];
	struct dirent* dnt;
	DIR* dp;

	if ((dp=opendir(dirpath)) == NULL) {
		fprintf(stderr, "opendir error for %s\n", dirpath);
		exit(1);
	}
	
	strcpy(filename, dirpath);
	char *ptr = filename + strlen(filename);
	*ptr++='/';
	*ptr= 0;

	int cnt = 1;
	while((dnt = readdir(dp)) != NULL) {
		if (strcmp(dnt->d_name, ".") && strcmp(dnt->d_name, "..")) {
			strcpy(ptr, dnt->d_name);
			hash_func(filename, cp_hash);
			if (!strcmp(hash, cp_hash)) {
				printf("[%d] %s ", cnt, filename);
				time_print(filename);
				cnt++;
			}
		}
	}		
}
void time_print (char* path)
{
	char time_buf[100];
	//실수 4. tm 구조체 포인터를 tm *t1, t2 이렇게 실수함.
	struct tm *t1,*t2;
	struct stat sb;
	
	if (lstat(path, &sb) < 0) {
		fprintf(stderr, "lstat error for %s\n", path);
		return;
	}
	
	t1 = localtime(&sb.st_mtime);
	t2 = localtime(&sb.st_atime);

	//실수 5. tm_year, tm_mon, tm_mday , tm_hour 등으로 작성할 것.
	sprintf(time_buf, "(mtime : %04d-%02d-%02d %02d:%02d) (atime : %04d-%02d-%02d %02d:%02d)\n", t1->tm_year+1900, t1->tm_mon+1, t1->tm_mday, t1->tm_hour,
			t1->tm_min, t2->tm_year+1900, t2->tm_mon+1, t2->tm_mday, t2->tm_hour, t2->tm_min);
	
	printf("%s", time_buf);	
}

void delete_path (char* dirpath, char* hash)
{

	char cp_hash[HASHMAX];
	char filename[PATHMAX];
	struct dirent* dnt;
	DIR* dp;

	if ((dp=opendir(dirpath)) == NULL) {
		fprintf(stderr, "opendir error for %s\n", dirpath);
		exit(1);
	}
	
	strcpy(filename, dirpath);
	char *ptr = filename + strlen(filename);
	*ptr++='/';
	*ptr= 0;

	while((dnt = readdir(dp)) != NULL) {
		if (strcmp(dnt->d_name, ".") && strcmp(dnt->d_name, "..")) {
			strcpy(ptr, dnt->d_name);
			hash_func(filename, cp_hash);
			if (!strcmp(hash, cp_hash)) {
				char answer = 0;
				while (answer != 'y' && answer != 'n') {
					printf(">> Delete %s? [y/n] ", filename);
					//주의 6.!!!!, fgetc 하고 while(getchar == '\n') 해주기!!!!!!
					answer = fgetc(stdin);	
					while(getchar() != '\n');
					//scanf("%s", &answer);
					if (answer != 'y' && answer != 'n')
						printf("Please enter y or n.\n");
					else if (answer == 'y')
						remove(filename);
							
				}
				
			}
		}
	}		
}
