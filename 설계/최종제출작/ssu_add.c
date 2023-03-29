#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ssu_exec.h"

//test
extern char* optarg;
int main(int argc, char* argv[])
{
    //printf("your uid is %d\n", (int)getuid());            //uid 확인완료
    //ssu_add("/home/junhyeong/go", 1, 0);
    int c;
    int flag = 0;
    char filename[MAXPATHLEN] = {0,};
    char hash[5] = {0,};

    if (argc == 2)
    {
        main_help_add();
        exit(1);
    }
    strcpy(filename, argv[1]);
    strcpy(hash, argv[argc-1]);
    if (strcmp(hash, "md5") != 0 && strcmp(hash, "sha1") != 0)
    {
        printf("usage hash is <md5 | sha1>\n");
        exit(1);
    }

    int flag_d = 0;
    while((c = getopt(argc, argv, "d")) != -1)
    {
        switch (c)
        {
            case 'd':
                flag_d = 1;
                break;
            case '?':
                main_help_add();
                exit(1);
                break;

            
        }
    }

    if (strlen(BACKUP_PATH) == 0)
        get_backuppath();
    
    if(strstr(filename, BACKUP_PATH) != NULL)
    {
        printf("%s can't be backuped\n", filename);
        main_help_add();
        exit(1);
    }

    if (access(filename, R_OK) != 0)
    {
        if (access(filename, F_OK) != 0)
        {
            printf("%s can't be backuped\n", filename);                //나중에 다 can't be beckuped 로 고칠 것.
            main_help_add();
            exit(1);
        }
        else
        {
            printf("%s is can not access\n", filename);             //나중에 다 can't be beckuped 로 고칠 것.
            main_help_add();
            exit(1);
        }
    }

    /** filename이 filename 기준으로 backup 디렉토리랑 겹치는 경우도 종료*/
    get_actualpath2(filename);
    char actual_backup [MAXPATHLEN+7] = {0,};
    sprintf(actual_backup, "%s/backup", ACTUAL_PATH);
    if (strstr(filename, actual_backup) != NULL)
    {
        printf("%s can't be backuped\n", filename);
        main_help_add();
        exit(1);
    }

    int hash_num = (strcmp("md5", hash) == 0) ? 0 : 1;
    if (flag_d)
    {
        if (argc != 4)
        {
            main_help_add();
            exit(1);
        }
        printf("please wait .....\n");
        /*
        printf("option d\n");
        printf("filename is %s\n", filename);
        printf("hash is %s\n", hash);
        */
    }
    else
    {
        if (argc != 3)
        {
            main_help_add();
            exit(1);
        }
        /*
        strcpy(filename,argv[1]);
        printf("no option\n");
        printf("filename is %s\n", filename);
        printf("hash is %s\n", hash);
        return 0;
        */
    }

    if (strlen(BACKUP_PATH) == 0)
        get_backuppath();
    
    if (access(BACKUP_PATH, R_OK) != 0)
        mkdir (BACKUP_PATH, 0777);
    ssu_add(filename, flag_d, hash_num);
    exit(0);
}
