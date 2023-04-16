#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define DIRECTORY_SIZE MAXNAMLEN
#define isdigit(x) (x>='0' && x <='9') ? 1 : 0
#define MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH

int creat_dir (int depth, char* cur_dir);
void writefile (char *in_f, char* out_f);
void change_mod (const char* file_path);

char *fname = "ssu_test.txt";
int o_flag = 0, e_flag = 0;

int main(int argc, char* argv[])
{
	int opt;
	int depth;
	char cur_dir_name [DIRECTORY_SIZE] = {"\0",};
	int fd;

	while ((opt = getopt(argc, argv, "e:o:")) != -1)
	{
		switch(opt)
		{

			case 'e':
				e_flag = 1;
				break;
			
			case 'o':
				o_flag = 1;
				break;


			case '?':
				break;
		}
	}
	
	if (argc < 3) {
		// 주의!, 해당 부분에서 옵션 마지막 인자의 길이를 확인함
		char num = argv[argc-1][strlen(argv[argc-1])-1];
		if (isdigit(num))
			depth = num - 48;
		else {
			printf("%c\n", num);
			printf("dpeth error\n");
			exit(1);
		}
	}
	else
		fprintf(stderr, "too many argv\n");

	if((fd=creat(fname, 0600)) < 0) {
		fprintf(stderr, "creat error for %s\n", fname);
		exit(1);
	}
	else
		close(fd);


	if(mkdir("0", 0777) < 0)
	{
		fprintf(stderr, "mkdir error\n");
		exit(1);
	}
	
	//실수2. cur_dir_name에 0을 안넣는 경우가 있는데 넣을 것
	strcpy(cur_dir_name, "0");
	creat_dir(depth, cur_dir_name);
	exit(0);
}

int creat_dir (int depth, char* cur_dir)
{
	struct stat dir_st;
	int i = 0;
	char tmp_filename[MAXNAMLEN] = {'\0'};
	while(cur_dir[i] != '\0') i++;

	//주의 4. 주어진 cur_dir 을 바탕으로 depth 확장처리
	cur_dir[i++] = '/';
	cur_dir[i++] = depth + 48; //문자열 화
	cur_dir[i] = '\0';

	if (stat(cur_dir, &dir_st) < 0 ) {
		if (mkdir(cur_dir, 0777) < 0) {
			fprintf (stderr, "mkdir error\n");
			exit(1);
		}
	}

	//주의5. 상대경로로 tmp_filename 만들 것.
	if (o_flag)
	{
		if (depth % 2 == 0) { 
			sprintf(tmp_filename, "%s/%s", cur_dir, fname);
			writefile(fname, tmp_filename);
			change_mod(tmp_filename);
		}
			
	}
	else if (e_flag)
	{
		if (depth % 2 == 1) {
			sprintf(tmp_filename, "%s/%s", cur_dir, fname);
			writefile(fname, tmp_filename);
			change_mod(tmp_filename);
		}
	}
	else if (!o_flag && !e_flag)
	{
		sprintf(tmp_filename, "%s/%s", cur_dir, fname);
		writefile(fname, tmp_filename);
	}
	if (depth == 0)
		return 0;
	return creat_dir(depth-1, cur_dir);
}

//주의 6. writefile 할 때 open 사용할 것 (상대경로 허용하기 위함.)
//실수 7. 생성할 때 0600 으로 만들 것.
void writefile (char *in_f, char* out_f)
{
	int fd1, fd2;
	int length;
	char buf [1024];
	fd1 = open (in_f, O_RDONLY);
	fd2 = open (out_f, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	
	if (fd1 < 0 || fd2 < 0) {
		fprintf(stderr, "open error\n");
		exit(1);
	}

	while ((length=read(fd1, buf, 1024)) > 0) {
		write(fd2, buf, length);
	}
	close(fd1);
	close(fd2);
}

void change_mod (const char *file_path)
{
	if (file_path != NULL) {
		if (chmod(file_path, MODE) < 0) {
			fprintf(stderr, "chmod error for %s\n",file_path);
			exit(1);
		}
	}

}

