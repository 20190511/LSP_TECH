#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "ssu_shared.h"

//add . -d
#define PROMPT  printf("20190511> ");
#define PR(_CMD, _OPT)      printf("%s %d printed\n", _CMD,_OPT);

void main_help();
char** prompt_token(char* char_prompt, int* prompt_argc, char* hash);
int access_check(char* file_name);
int file_size_check (char file_names[]);
void clear_func();

int main(int argc, char* argv[]){
    int check_response = 1;

    //printf("your uid is %d\n", getuid());
    if (argc != 2 || ((strcmp(argv[1],"md5") != 0 && strcmp(argv[1],"sha1") != 0)))
    {
        printf("Usage: %s <md5 | sha1>\n",argv[0]);
        exit(1);
    } 
    else 
    {
        if (exist_username())             // [MAXPATHLEN] 으로 할당해서 쓸 것.,
        {
            //아이디 생성
            pid_t pid;
            int status;
            if ((pid = fork()) < 0)
            {
                fprintf(stderr, "Fork Error\n");
                exit(1);
            }

            if (pid != 0)
            {
                sleep(1);
                wait(&status);
            }
            else
            {
                //아이디 생성
                if (execl ("/usr/sbin/useradd", "/usr/sbin/useradd", "-m", BACKUP_USERNAME, NULL) == -1)
                {
                    //존재하는지 확인.
                    find_anyuser();
                    printf("execve error : so backup path : %s\n", changed_username);
                }
    
            }
        }
        char char_prompt [MAXPROMPTLEN];
        char hash[5] = {0,};
        int status;
        int max_ = 0;
        strcpy(hash, argv[1]);


        // 백업 디렉토리 생성.
        get_backuppath();
        struct stat dirstat;
        if (access(BACKUP_PATH, F_OK) == 0)
        {
            if (stat(BACKUP_PATH, &dirstat) < 0)
            {
                fprintf(stderr, "Can't be accessed my backup directory : %s, exit program...\n", BACKUP_PATH);
                exit(1);
            }
            if (!S_ISDIR(dirstat.st_mode))
            {
                fprintf(stderr, "There are conflict name and no directory file about my backup directory : %s, exit program...\n", BACKUP_PATH);
                exit(1);
            }
        }
        else
            mkdir(BACKUP_PATH, 0777);

        while(1)
        {
            fflush(stdin);
            PROMPT;
            fgets(char_prompt, MAXPROMPTLEN, stdin);                    // 문자열 표준입력으로부터 입력받음.    -> fegts(char_prompt, MAXPROMPTLEN, stdin) 쓸 시 \n 도 딸려들어옴 
            if (strcmp(char_prompt, "\n") == 0 && check_response == 0)
            {
                check_response = 1;
                fgets(char_prompt, MAXPROMPTLEN, stdin);
            }
            int prompt_argc = 1;                                        // 인자길이

            
            char **prompt_argv = prompt_token(char_prompt, &prompt_argc, hash);
            if (prompt_argc <= 0 || prompt_argc > 6 || prompt_argv==NULL)
            {
                if (strcmp(char_prompt, "\n") != 0)
                    main_help();
                continue;
            }

            char prompt_cmd[MAXPROMPTLEN] = {0,};
            char file_name [MAXPATHLEN] = {0,};                        // 파일 (경로) 가 들어가는 변수
            strcpy(prompt_cmd, char_prompt);
            for (int i = 0 ; i < MAXPROMPTLEN ; i++)
            {
                if (prompt_cmd[i] == ' ' || prompt_cmd[i] == '\n')
                {
                    prompt_cmd[i] = '\0';
                    break;
                }
            }
            
            if (prompt_argc >= 2)
            {
                strcpy(file_name, prompt_argv[1]);                          // prompt_arv[1] : filename은 무조건 들어감.
            }

            // 인자 있는 명령어 | 없는 명령어 | OThers
            if (strcmp(prompt_cmd, "add") == 0 || strcmp(prompt_cmd, "remove") == 0 || strcmp(prompt_cmd, "recover") == 0 )
            {
                                         
                if (strcmp(prompt_cmd, "remove") != 0 && prompt_argc != 2)
                    file_size_check(prompt_argv[1]);
                                    
                //일단 만약 실행파일 ex> ssu_add 가 없으면 컴파일부터.. 해주자.. 03.11
                    // -> 실행파일 없으면 자동컴파일
                char compile_file [1028] = {0,};
                char compile_file_c [1030] = {0,};
                sprintf(compile_file, "ssu_%s", prompt_cmd);
                sprintf(compile_file_c, "%s.c", compile_file);
                file_size_check(compile_file);
                file_size_check(compile_file_c);

                if (access(compile_file, F_OK) != 0)
                {
                    pid_t pid;
                    int compile_status;
                    if ((pid = fork()) < 0)
                    {
                        fprintf(stderr, "Fork Error : %s\n", prompt_cmd);
                        continue;
                    }

                    if (pid != 0)
                    {
                        wait(&compile_status);
                    }
                    else
                    {

                        if (execl("/usr/bin/gcc", "gcc", "-g", compile_file_c, "-o", compile_file, "-lcrypto", NULL) == -1)
                        {
                            printf("execve error\n");
                            continue;
                        }
                    }
                }


                pid_t pid;
                if ((pid = fork()) < 0)
                {
                    fprintf(stderr, "Fork Error : %s\n", prompt_cmd);
                    continue;
                }

                if (pid != 0)
                {
                    sleep(1);
                    wait(&status);
                }
                else
                {

                    if (execv (prompt_argv[0], prompt_argv) == -1)
                    {
                        printf("execve error\n");
                        continue;
                    }

                }
                
            }

            else if (strcmp(prompt_cmd, "ls") == 0  || strcmp(prompt_cmd, "vi") == 0  || 
                    strcmp(prompt_cmd, "vim") == 0 || strcmp(prompt_cmd, "exit") == 0)        
            {
                if (strcmp(prompt_cmd, "exit") == 0)
                {
                    for (int i = 0 ; i < prompt_argc ; i++)
                        free(prompt_argv[i]);
                    free(prompt_argv);

                    //exit 하면 실행파일 싹 다 지워버리기 03.15 (새로운 컴파일을 위해 귀찮아서 제작.)
                    clear_func();
                    break;
                }

                pid_t pid;
                if ((pid = fork()) < 0)
                {
                    fprintf(stderr, "Fork Error : %s\n", prompt_cmd);
                    continue;
                }
                if (pid != 0)
                {
                    wait(&status);
                }

                if (pid == 0)
                {
                    if (prompt_argv[prompt_argc-1][0] != '-')
                        file_size_check(prompt_argv[prompt_argc-1]);
                    
                    if (execv (prompt_argv[0], prompt_argv) == -1)
                    {
                        printf("execve error\n");
                        continue;
                    }
                }

            }
            else
            {
                strcpy(prompt_argv[0], "ssu_help");
                realpath(prompt_argv[0],prompt_argv[0]);
                char help_c[MAXPATHLEN] = {0,};
                sprintf(help_c,"%s.c", prompt_argv[0]);
                pid_t pid1, pid2;
                if (access(prompt_argv[0], F_OK) != 0)
                {
                    pid_t pid1;
                    int compile_status;
                    if ((pid1 = fork()) < 0)
                    {
                        fprintf(stderr, "Fork Error : %s\n", prompt_cmd);
                        continue;
                    }

                    if (pid1 != 0)
                    {
                        wait(&compile_status);
                    }
                    else
                    {

                        if (execl("/usr/bin/gcc", "gcc", "-g", help_c, "-o", prompt_argv[0], NULL) == -1)
                        {
                            printf("execve error\n");
                            continue;
                        }
                    }
                }


                if ((pid2 = fork()) < 0)
                {
                    fprintf(stderr, "Fork Error : %s\n", prompt_cmd);
                    continue;
                }
                if (pid2 != 0)
                {
                    wait(&status);
                }
                if (pid2 == 0)
                {

                    if (execv (prompt_argv[0], prompt_argv) == -1)
                    {
                        printf("execve error\n");
                        continue;
                    }
                }
            }
            
        }
    }
    
    exit(0);
}

int access_check(char* file_name)
{
    char *pwd = getcwd(NULL, MAXPATHLEN);
    char new_path[MAXPATHLEN];
    if (file_name[0] != '/')    //상대경로
    {
        sprintf(new_path, "%s/%s",pwd,file_name);
    }
    if (access(pwd, R_OK) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char** prompt_token(char* char_prompt, int* prompt_argc, char* hash)
{
    if (char_prompt[0] == '\n')
    {
        *prompt_argc = 0;
        return NULL;
    }

    char temp [MAXPROMPTLEN]; 
    strcpy(temp, char_prompt);
    char* token_ptr;
    
    int idx = 0;
    token_ptr = strtok(temp, " ");          // token 분리하면서 원본이 파괴되기 때문에 지역변수 할당.
    while(token_ptr != NULL)
    {
        idx++;
        token_ptr = strtok(NULL, " ");
    }
    idx++;                       //마지막 인자로는 hash값을 줄 예정.
    *prompt_argc = idx;                  
    strcpy(temp, char_prompt);
    strtok(temp, "\n");
    char** prompt_argvs = (char**)malloc(sizeof(char*) * idx);
    for (int i = 0 ; i < idx ; i++)
        prompt_argvs[i] = NULL;
    token_ptr = strtok(temp, " ");    
    idx = 0;
    int hash_plus_flag = 0;
    while(token_ptr != NULL)
    {
        prompt_argvs[idx] = (char*)malloc(sizeof(char) * MAXPATHLEN);
        if (strcmp(token_ptr, "add") == 0 || strcmp(token_ptr, "remove") == 0 || strcmp(token_ptr, "recover") == 0)
        {
            if (strcmp(token_ptr, "remove") != 0)
                hash_plus_flag = 1;
            char pwd[MAXPATHLEN] = {0,};
            getcwd(pwd, MAXPATHLEN);
            sprintf(prompt_argvs[idx++], "%s/ssu_%s", pwd, token_ptr);    //실행파일은 ssu_add.c, ssu_remove.c, ssu_recover.c 로함.
        }
        else if (strcmp(token_ptr, "ls") == 0 || strcmp(token_ptr, "vi") == 0 || strcmp(token_ptr, "vim") == 0)
        {
            char pwd[MAXPATHLEN] = {0,};
            strcpy(pwd, "/usr/bin");
            sprintf(prompt_argvs[idx++], "%s/%s", pwd, token_ptr);    //실행파일은 ssu_add.c, ssu_remove.c, ssu_recover.c 로함.
        }
        else
        {
            //파일 크기 체크
            if (strlen(token_ptr) >= MAXPATHLEN)
            {
                printf("MAX Path length over Error(%d)\n", MAXPATHLEN);
                return NULL;
            }
            strcpy(prompt_argvs[idx++], token_ptr);
        }
        token_ptr = strtok(NULL, " ");
    }
    if (hash_plus_flag)
    {
        prompt_argvs[idx] = (char*)calloc(5, sizeof(char));        //해시값 설정
        strcpy(prompt_argvs[idx++], hash);
        prompt_argvs[idx] = NULL;
    }
    else
    {
        free(prompt_argvs[idx]);
        prompt_argvs[idx] = NULL;
        *prompt_argc -= 1;
    }
    
    return prompt_argvs;
}

void main_help()
{
    printf("Usage:\n"
            "  > add <FILENAME> [OPTION]\n"
            "    -d : add directory recursive\n"
            "  > remove <FILENAME> [OPTION]\n"
            "    -c : remove all file(reculsive)\n"
            "    -a <NEWNAME> : clear backup directory\n"
            "  > recover <FILENAME> [OPTION]\n"
            "    -d : recover directory recursive\n"
            "    -n <NEWNAME> : recover file with new name\n"
            "  > ls\n"
            "  > vi\n"
            "  > vim\n"
            "  > help\n"
            "  > exit\n");
}




void clear_func()
{
    char clean_file [4][MAXPATHLEN] = {0,};
    strcpy(clean_file[0],"ssu_add");
    strcpy(clean_file[1],"ssu_recover");
    strcpy(clean_file[2],"ssu_remove");
    strcpy(clean_file[3],"ssu_help");  
 
    for (int i = 0 ; i < 4 ; i++)
    {
        realpath(clean_file[i], clean_file[i]);

        if (access(clean_file[i], F_OK) == 0)
        {
            remove(clean_file[i]);
        }
    }
}
