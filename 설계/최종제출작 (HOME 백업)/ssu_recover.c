#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ssu_exec.h"

#define ARG_ERR(_CNT)\
{\
    if (argc != _CNT)\
    {\
        main_help_recover();\
        exit(1);\
    }\
}

extern char* optarg;

int main(int argc, char* argv[])
{
    //ssu_add("/home/junhyeong/go", 1, 0);
    int c;
    int flag = 0;
    char filename[MAXPATHLEN] = {0,};
    char newname [MAXPATHLEN] = {0,};
    char hash[5] = {0,};

    if (argc == 2)
    {
        main_help_recover();
        exit(1);
    }
    if (argc < 2 || argc > 6)
    {
        main_help_recover();
        exit(1);
    }

    strcpy(filename, argv[1]);
    strcpy(hash, argv[argc-1]);
    if (strcmp(hash, "md5") != 0 && strcmp(hash, "sha1") != 0)
    {
        printf("usage hash is <md5 | sha1>\n");
        main_help_recover();
        exit(1);
    }

    int flag_d = 0;
    int flag_n = 0;
    while((c = getopt(argc, argv, "dn:")) != -1)
    {
        switch (c)
        {
            case 'd':
                flag_d = 1;
                break;
            case 'n':
                flag_n = 1;
                strcpy(newname, optarg);
                if(!file_size_check(newname) || strstr(newname, BACKUP_PATH) != NULL)
                {
                    printf("%s can't be backuped\n", newname);
                    exit(1);
                }

                break;
            case '?':
                main_help_recover();
                exit(1);
                break;
        }
    }
    
    int hash_num = (strcmp("md5", hash) == 0) ? 0 : 1;
    if (flag_d && flag_n)
    {
        ARG_ERR(6);
        /*
        printf("option -d and -n \n");
        printf("filename is %s\n", filename);
        printf("newname is %s\n", newname);
        printf("hash is %s\n", hash);
        */
        printf("please wait .....\n");
    }
    else if (flag_d)
    {
        ARG_ERR(4);
        printf("please wait .....\n");   
        /*
        printf("option d\n", filename);
        printf("filename is %s\n", filename);
        printf("hash is %s\n", hash);
        */
    }
    else if (flag_n)
    {
        ARG_ERR(5);
        /*
        printf("option n\n");
        printf("filename is %s\n", filename);
        printf("newname is %s\n", newname);
        printf("hash is %s\n", hash);
        */
    }
    else
    {
        ARG_ERR(3);
        /*
        printf("no option\n");
        printf("filename is %s\n", filename);
        printf("newname is %s\n", newname);
        printf("hash is %s\n", hash);
        */
    }
    if (strlen(BACKUP_PATH) == 0)
        get_backuppath();
    
    if (access(BACKUP_PATH, R_OK) != 0)
        mkdir (BACKUP_PATH, 0777);
    ssu_recover(filename, flag_d, flag_n, newname, hash_num);
    exit(0);
}