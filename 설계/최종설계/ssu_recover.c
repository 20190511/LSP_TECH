#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ssu_exec.h"

#define ARG_ERR(_CNT)\
{\
    if (argc != _CNT)\
    {\
        main_help();\
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
                if(!file_size_check(newname))
                    exit(1);

                break;
            case '?':
                main_help();
                break;
        }
    }
    
    int hash_num = (strcmp("md5", hash) == 0) ? 0 : 1;
    if (flag_d && flag_n)
    {
        ARG_ERR(6);
        ssu_recover(filename, flag_d, flag_n, newname, hash_num);
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
        /*
        ARG_ERR(3);
        printf("no option\n");
        printf("filename is %s\n", filename);
        printf("newname is %s\n", newname);
        printf("hash is %s\n", hash);
        */
    }
    ssu_recover(filename, flag_d, flag_n, newname, hash_num);
    exit(0);
}
