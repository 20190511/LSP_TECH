#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include "ssu_exec.h"

#define MAXPATHLEN      4097
int scandir(const char *dirp, struct dirent *** namelist,
            int(*filter)(const struct dirent *),
            int(*compar)(const struct dirent**, const struct dirent **));       // scandir, alphasort 명시
int alphasort(const struct dirent **d1, const struct dirent **d2);

/** return_value : 실패시 -1, 성공시 자식 파일 개수.
새로운 알고리즘 방식 제시*/
int scandir_filter (const struct dirent* sub_dir);
int append_samefile2 (Flist* flist, char* file_path, int opt, int f_opt);

/**
 * 
 * 
 * 
 * Typeing Your finding string in backup_path >> /home/junhyeong/backup/test.c
================ Start Flist Print ===============
-----------------Backup Dictinary-----------------
-----------------Backup File    -----------------
[1] /home/junhyeong/backup/test.c_230314095623
[2] /home/junhyeong/backup/test.c_230314095640
[3] /home/junhyeong/backup/test.c_230314095647
================ End Flist Print ===============
File count is 3



Typeing Your finding string in backup_path >> /home/junhyeong/backup/go2
================ Start Flist Print ===============
-----------------Backup Dictinary-----------------
/home/junhyeong/backup/go2
-----------------Backup File    -----------------
================ End Flist Print ===============
File count is 1
*/

char* scandir_filename;

#define DEBUG
#ifdef DEBUG
int main(void)
{
    char prompt [MAXPATHLEN] = {0,};
    printf("Typeing Your finding string in backup_path ");
    while (strlen(prompt) == 0 )
    {  
        printf(">> ");
        fgets(prompt, MAXPATHLEN, stdin);
    }
    strtok(prompt, "\n");
    Flist* t1 = new_flist();
    int cnt = append_samefile2(t1, prompt, 0, 0);
    print_flist(t1);
    
    printf("File count is %d\n", cnt);
    exit(0);
}
#endif



int scandir_filter (const struct dirent* entry)
{
    if (strlen(scandir_filename) == 0 || entry == NULL)
        return 0;
    if (strstr(entry->d_name, scandir_filename) != NULL)
        return 1;
    return 0;
}


// parent_path 는 절대경로만 받을 수 있음.
int append_samefile2 (Flist* flist, char* file_path, int opt, int f_opt)
{

    char parent_path[MAXPATHLEN] = {0,};
    char only_file_path [MAXFILELEN] = {0,};
    strcpy(parent_path, file_path);
    char *find_filename = strrchr(parent_path, '/');
    if (find_filename == NULL)
        return -1;
    strcpy(only_file_path, find_filename+1);
    *find_filename = '\0';

    // 파일 접근 권한 확인 (일단 해당 경로에 파일이 존재하는지 확인)
    if (access(parent_path, R_OK) != 0 )
        return -1;

    if (parent_path == NULL)
        return -1;

    scandir_filename = only_file_path;
    struct dirent** sub_dir;
    if (scandir(parent_path, &sub_dir, scandir_filter, alphasort) < 0)
        return -1;

    /*
    char temp_path [MAXPATHLEN] = {0, };
    strcpy(temp_path, parent_path);
    char* char_cpr = temp_path + strlen(parent_path);
    *char_cpr++ = '/';
    */

    *find_filename++ = '/';

    int scan_cnt = 0;
    for (scan_cnt = 0 ;; scan_cnt++)
    {
        if (sub_dir == NULL || sub_dir[scan_cnt] == NULL)
            break;
        strcpy(find_filename, sub_dir[scan_cnt]->d_name);
        append(flist, parent_path, opt, f_opt);
        free(sub_dir[scan_cnt]);
    }
    
    scandir_filename = NULL;
    return scan_cnt;
}
