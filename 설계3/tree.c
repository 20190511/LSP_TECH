#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif
#ifndef MAXPATHSIZE
#define MAXPATHSIZE 4096
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
int print_tree (char* path, int depth);

int main(int argc, char* argv[])
{
    char buf [BUFFER_SIZE];
    int i = 0;
    FILE* fp = fopen("log.txt", "r");
    if (fp == NULL) {
        fprintf (stderr, "fopen error for log.txt\n");
        exit(1);
    }


    int linecnt = 1;
    while (!feof(fp))
    {
        if (fgets(buf, BUFFER_SIZE, fp) == NULL)
            break;
        else
        {
            while (buf[i] != '\n' && buf[i] != '\0')
                i++;
            buf[i] = '\0';
            printf("[%d]: %s\n", linecnt++, buf);
        }
    }
    

    print_tree(argv[1], 0);
    exit(0);
}

int print_tree (char* path, int depth)
{
    struct dirent** dnt;
    struct stat statbuf;
    char tmp_path [MAXPATHSIZE];
    int dir_cnt = 0;
    int i;

    setbuf(stdout, NULL);   
    if (stat(path, &statbuf) < 0 ) {
        fprintf(stderr, "path error for %s\n", path);
        return false;
    }

    if (!S_ISDIR(statbuf.st_mode)) {
        return false;
    }

    strcpy(tmp_path, path);
    char* ptr = tmp_path + strlen(tmp_path);
    *ptr++ = '/';
    *ptr = '\0';

    if ((dir_cnt = scandir(path, &dnt, NULL, alphasort)) < 0)
    {
        fprintf(stderr, "scandir error for %s\n", path);
        return false;
    }

    if (depth == 0)
        printf("%s\n", path);
    
    for (i = 0 ; i < dir_cnt ; i++) {
        if (strcmp(dnt[i]->d_name, ".") && strcmp(dnt[i]->d_name, ".."))
        {
            strcpy(ptr, dnt[i]->d_name);

            for (int blank = 0 ; blank < depth ; blank++)
            {
                if (blank == 0)
                    printf("│  ");
                else
                    printf("   ");
            }

            if (i + 1 == dir_cnt)
                printf("└──  %s\n", dnt[i]->d_name);
            else
                printf("├──  %s\n", dnt[i]->d_name);


            if (stat(tmp_path, &statbuf) < 0) 
            {
                fprintf(stderr, "stat error for %s\n", tmp_path);
                return false;
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                print_tree(tmp_path, depth+1);
            }

       }
    }

    return true;
}
