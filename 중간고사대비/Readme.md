중간고사 대비 교재 예제풀이

2023년 4월 15일 토요일
오전 10:59

	1. 2-3
	예제 	
	2. 2-4
	코드 	
		#include <stdio.h>
		#include <stdlib.h>
		#include <unistd.h>
		#include <fcntl.h>
		#include <sys/stat.h>
		#define WORD_MAX 100
		#define BUFFER_SIZE 1024
		int main(void)
		{
		    int fd;
		    int length = 0, offset = 0, count = 0;
		    char *fname = "ssu_test.txt";
		    char buf[WORD_MAX][BUFFER_SIZE];
		    int i;
		    if ((fd=open(fname, O_RDONLY)) < 0) {
		        fprintf(stderr, "fopen error for %s\n", fname);
		        exit(1);
		    }
		    while ((length=read(fd, buf[count], BUFFER_SIZE)) > 0) {
		        buf[count][length] = 0;
		        for (i = 0 ; i < BUFFER_SIZE ; i++) {
		            if (buf[count][i] == '\n') {
		                if (i == 0)
		                    break;
		                
		                offset = offset + i + 1;
		                lseek(fd, offset, SEEK_SET);
		                count++;
		            }
		        }
		    }
		    close (fd);
		    for (i = 0 ; i < count ; i++) {
		        printf("%s\n", buf[i]);
		    }
		    
		    printf("line number : %d\n", count);
		    exit(0);
		}
	
	
	3. 2-5
	코드 	
		#include <stdio.h>
		#include <stdlib.h>
		#include <unistd.h>
		#include <fcntl.h>
		#include <sys/stat.h>
		#define BUFFER_SIZE 128 
		int main(int argc, char* argv[])
		{
		    char buf[BUFFER_SIZE];
		    int fd1, fd2;
		    ssize_t size;
		    if (argc != 3) {
		        fprintf(stderr, "usage: %s <file1> <file2>\n", argv[0]);
		        exit(1);
		    }
		    if((fd1=open(argv[1], O_RDONLY)) < 0) {
		        fprintf(stderr, "open error for %s\n", argv[1]);
		        exit(1);
		    }
		    if((fd2=open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0 ) {
		        fprintf(stderr, "open error for %s\n", argv[2]);
		        exit(1);
		    }
		    
		    while ((size=read(fd1, buf, BUFFER_SIZE)) > 0) {
		        if(write(fd2, buf, size) != size) {
		            fprintf(stderr, "write error\n");
		            exit(1);
		        }
		    }
		    close(fd1);
		    close(fd2);
		    exit(0);
		}
	
	
	
	4. 3-3 (잘못만듦)
	5. 3-4 
	코드 	#include <stdio.h>
		#include <stdlib.h>
		#include <unistd.h>
		int main (int argc, char* argv[])
		{
		    if (argc != 3) {
		        fprintf(stderr, "usage : %s <oldfile><newfile>\n", argv[0]);
		        exit (1);
		    }
		    if (link(argv[1], argv[2]) < 0) {
		        fprintf(stderr, "link error\n");
		        exit(1);
		    }
		    if (unlink(argv[1]) < 0) {
		        fprintf(stderr, "unlink error\n");
		        unlink(argv[2]);
		        exit(1);
		    }
		    exit(1);
		}
	
	6. 3-5 : argv[1] 로 넘겨주는 디렉토리의 하위 디렉토리 모두 출력
	예제	 	
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
		
		    chdir(dir);      // dir이 디렉토리니까 일단 dir로 위치변경을 하고.
		    char *ptr = cwd + strlen(cwd);
		    *ptr++ ='/';
		    *ptr='\0';
		
		    while ((dentry=readdir(dirp)) != NULL) {
		        if (strcmp(dentry->d_name, ".") && strcmp(dentry->d_name,"..")) {
		            strcpy(ptr, dentry->d_name);
		            lstat(cwd, &statbuf);  //디렉토리인지 확인해서 디렉토리면 재귀호출..
		            
		            if (S_ISDIR(statbuf.st_mode)) {
		                chdir(cwd);  //cwd로 작업디렉토리르 변경해서 디렉토리 재귀호출
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
		
		      // chdir(argv[1]); //예제에는 없지만 chdir(argv[1]) 부터 해주고 getcwd 를 하는게 좋다.
		    search_directory(argv[1]);
		}
		
![image](https://user-images.githubusercontent.com/70988272/232180020-792cee77-dd91-41a3-a659-f797d4f90ef6.png)
