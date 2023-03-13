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
        if (argc == 1)
        {
            printf("Usage : remove <FILENAME> [OPTION]\n");
            exit(1);
        }
        main_help();
        exit(1);
    }

    strcpy(filename, argv[1]);

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
        if (argc == 2)
        {
            if(file_size_check(filename))
                printf("filename is %s\n", filename);            
            ssu_remove(filename, flag);
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
        if (argc != 3)      //remove 경로 -a 의 경우에는 항상 argc가 4(+1(해시) 5임.)
        {
            main_help();
            exit(1);
        }

        if(file_size_check(filename))
            printf("filename is %s\n", filename);            
        //printf("option a\n");
        printf("please wait .....\n");
    }
    else if (c_flag)
    {
        if (argc != 2)
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
