#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ssu_exec.h"

extern char* optarg;

int main(int argc, char* argv[])
{
    int c;
    int flag = 0;
    char filename[MAXPATHLEN] = {0,};
    char hash[5] = {0,};

    if (argc < 2 || argc > 6)
    {
        main_help();
        exit(1);
    }

    strcpy(filename, argv[1]);
    strcpy(hash, argv[argc-1]);
    
    if (strcmp(hash, "md5") != 0 && strcmp(hash, "sha1") != 0)
    {
        printf("usage hash is <md5 | sha1>\n");
        printf("argc = %d\n", argc);
        printf("argv[1] = %s\n", argv[argc-1]);
        exit(1);
    }

    for (int i = 0 ; i < argc ; i++)
    {
        if (argv[i][0] == '-')
        {
            flag = 1;
            break;
        }
    }
    if (!flag)
    {
        if (argc == 3)
        {
            printf("no option remove\n");
            if(file_size_check(filename))
                printf("filename is %s\n", filename);
        }
        else
        {
            main_help();
        }
        return 0;
    }

    int c_flag = 0;
    int a_flag = 0;
    while((c = getopt(argc, argv, "ca")) != -1)
    {
        switch (c)
        {
            case 'c':
                c_flag = 1;
                break;
            case 'a':
                a_flag = 1;
                break;
            case '?':
                main_help();
                break;
        }
    }

    if (a_flag && c_flag)
    {
        main_help();
        exit(1);
    }
    else if (a_flag)
    {
        if (argc != 4)      //remove 경로 -a 의 경우에는 항상 argc가 4(+1(해시) 5임.)
        {
            main_help();
            exit(1);
        }

        if(file_size_check(filename))
            printf("filename is %s\n", filename);            
        if (access(filename, F_OK) != 0)
        {
            printf("%s can't be existed\n", filename);
            exit(1);
        }
        //printf("option a\n");
        printf("please wait .....\n");
    }
    else if (c_flag)
    {
        if (argc != 3)
        {
            main_help();
            exit(1);
        }
        //printf("option c: all clear\n");
        //printf("hash is %s\n", hash);
        printf("please wait .....\n");
        ssu_remove_all();
        exit(0);
    }

    ssu_remove(filename, a_flag);
    exit(0);
}
