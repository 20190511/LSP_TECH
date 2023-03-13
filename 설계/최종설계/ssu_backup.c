#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

//add . -d
#define PROMPT  printf("20190511> ");
#define PR(_CMD, _OPT)      printf("%s %d printed\n", _CMD,_OPT);
#define MAXPATHLEN      256
#define MAXPROMPTLEN    1024
void main_help();
char** prompt_token(char* char_prompt, int* prompt_argc, char* hash);
int access_check(char* file_name);
int file_size_check (char file_names[]);

int main(int argc, char* argv[]){
    int check_response = 1;
    /*
    if (getuid() != 0)                                  // 03.11 sudo 권한으로 실행시켜야 root 권한으로 setuid 가능.
    {
        printf("If you didn't execute with 'sudo', you can not backup your file about root file\n"
               "Usage: sudo %s\n"
               "Do you want quit? (y/n)\n"
               "If you picked 'n', the program execute with not root permission\n", argv[0]);
        
        while (check_response)
        {
            printf(">> ");
            int response = fgetc(stdin);
            switch(response)
            {
                case 'y':
                case 'Y':
                    exit(1);
                    break;
                case 'n':
                case 'N':
                    check_response = 0;
                    break;
                default:
                    printf("Wrong Response! : Please type again\n");
                    break;
            }
        }
    }
            */                                           // [MAXPATHLEN] 으로 할당해서 쓸 것.,
    char char_prompt [MAXPROMPTLEN];
    char hash[5] = {0,};
    int status;
    //printf("your uid is %d\n", getuid());
    if (argc != 2 || ((strcmp(argv[1],"md5") != 0 && strcmp(argv[1],"sha1") != 0)))
    {
        printf("Usage: %s <md5 | sha1>\n",argv[0]);
        exit(1);
    } 
    else 
    {
        int max_ = 0;
        strcpy(hash, argv[1]);
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
                    strcmp(prompt_cmd, "vim") == 0 || strcmp(prompt_cmd, "help") == 0 || strcmp(prompt_cmd, "exit") == 0)        
            {
                if (strcmp(prompt_cmd, "exit") == 0)
                {
                    for (int i = 0 ; i < prompt_argc ; i++)
                        free(prompt_argv[i]);
                    free(prompt_argv);
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

                if (strcmp(prompt_cmd, "help") == 0)
                {
                    main_help();
                    for (int i = 0 ; i < prompt_argc ; i++)
                        free(prompt_argv[i]);
                    free(prompt_argv);
                    continue;
                }

                if (pid == 0)
                {
                    if (execv (prompt_argv[0], prompt_argv) == -1)
                    {
                        printf("execve error\n");
                        continue;
                    }
                }

            }
            else
            {
                main_help();
                for (int i = 0 ; i < prompt_argc ; i++)
                    free(prompt_argv[i]);
                free(prompt_argv);
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
            "  > add [FILENAME] [OPTION]\n"
            "    -d : add directory recursive\n"
            "  > recover [FILENAME] [OPTION]\n"
            "    -d : recover directory recursive\n"
            "    -n [NEWNAME] : recover file with new name\n"
            "  > remove [FILENAME] [OPTION]\n"
            "    -c : recover directory recursive\n"
            "    -a [NEWNAME] : recover file with new name\n"
            "  > ls\n"
            "  > vi\n"
            "  > vim\n"
            "  > help\n"
            "  > exit\n");
}


// char *tmp = realpath 형태 사용불가.
int file_size_check (char file_names[])
{
    char file_name [4097] = {0,};    
    strcpy(file_name, file_names);
    if (file_name[0] != '/')
        realpath(file_name, file_name);
    if (strstr(file_name, "/home") == NULL)
    {
        printf("%s is over from /home directory\n", file_name);
        return 0;
    }

    if (strlen(file_name) >= MAXPATHLEN)
    {
        printf("%s size is %ld. It is over MAX file legnth(%d)\n", file_name,strlen(file_name), MAXPATHLEN);
        return 0;
    }

    strcpy(file_names, file_name);
    return 1;
}
