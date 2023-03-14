#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define MAXPATHLEN      4097
int scandir_filter (const struct dirent* sub_dir);
int scandir(const char *dirp, struct dirent *** namelist,
            int(*filter)(const struct dirent *),
            int(*compar)(const struct dirent**, const struct dirent **));       // scandir, alphasort 명시
int alphasort(const struct dirent **d1, const struct dirent **d2);
char* scandir_filename;

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
    scandir_filename = prompt;
    char* path = "/home/junhyeong/backup";
    struct dirent** sub_dir;
    int file_cnt = 0;
    if (file_cnt = scandir(path, &sub_dir, scandir_filter, alphasort) < 0)
    {
        exit(1);
    }
    char temp [MAXPATHLEN] = {0, };
    strcpy(temp, path);
    char* char_cpr = temp + strlen(path);
    int scan_cnt = 0;
    for (scan_cnt = 0 ; scan_cnt < 10000 ; scan_cnt++)
    {
        if (sub_dir == NULL || sub_dir[scan_cnt] == NULL)
            break;
        strcpy(temp, sub_dir[scan_cnt]->d_name);
        printf("%s\n", temp);
    }
 
    if (scan_cnt == 0)
        printf("%s is no filename\n", scandir_filename);
    exit(0);
}

int scandir_filter (const struct dirent* entry)
{
    if (strlen(scandir_filename) == 0)
        return 0;
    if (strstr(entry->d_name, scandir_filename) != NULL)
        return 1;
    return 0;
}
