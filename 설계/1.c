#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <openssl/sha.h>            /*현재 home에 설치되어있음 */
#include <openssl/md5.h>

#define PROMPT  printf("20190511> ");
#define PR(_CMD, _OPT)      printf("%s %d printed\n", _CMD,_OPT);
#define MAXPATHLEN  256
#define MAXPROMPTLEN    1024
void main_help();
char** prompt_token(char* char_prompt, int* prompt_argc);
int access_check(char* file_name);


int main(int argc, char* argv[])
{
    
                                                       // [MAXPATHLEN] 으로 할당해서 쓸 것.,
    char char_prompt [MAXPROMPTLEN];

    if (argc < 2 || argc >= 4 || (strcmp(argv[1],"md5") != 0 && strcmp(argv[1],"md5") != 0))
    {
        main_help();
    } 
    else 
    {
        int max_ = 0;
        while(1)
        {
            PROMPT;
            fgets(char_prompt, MAXPROMPTLEN,stdin);                    // 문자열 표준입력으로부터 입력받음.    -> fegts(char_prompt, MAXPROMPTLEN, stdin) 쓸 시 \n 도 딸려들어옴 
            int prompt_argc = 1;                                // 인자길이
            char **prompt_argv = prompt_token(char_prompt, &prompt_argc);
            char prompt_cmd[MAXPROMPTLEN] = {0,};
            char file_name [MAXPATHLEN] = {0,};                        // 파일 (경로) 가 들어가는 변수
            strcpy(prompt_cmd, prompt_argv[0]);                 // prompt_arv[0] : 명령어가 들어가는위치.
            strcpy(file_name, prompt_argv[1]);
            if (!access_check(file_name))                       // file_name이 아닌경우
            {
                main_help();
                continue;
            }
            

            // 인자 있는 명령어 | 없는 명령어 | OThers
            if (strcmp(prompt_cmd, "add") == 0 || strcmp(prompt_cmd, "remove") == 0 || strcmp(prompt_cmd, "recover") == 0 )
            {
                
                // 일단 아무것도 안들어오는 경우 제작 X
                int cmd_idx;
                if (strcmp(prompt_cmd, "add") == 0)
                {                
                    //d 생략가능.
                    int err = 0;
                    int flag_d = 0;
                    for (cmd_idx = 2 ; cmd_idx < prompt_argc ; cmd_idx++)
                    {
                        if (prompt_argv[cmd_idx][0] == '-')
                        {
                            if (prompt_argv[cmd_idx][1] == 'd')
                                flag_d = 1;
                            else
                            {
                                err = 1;
                                break;
                            }
                        }
                    }
                    if (err)
                    {
                        main_help();
                        continue;
                    }
                    
                    if (flag_d)
                    {
                        printf("add, flag_d=%d\n",flag_d);
                    }
                    else
                    {

                    }
                }

                else if (strcmp(prompt_cmd, "remove") == 0)
                {
                    //a,c 모두 생략가능.
                    int err = 0;
                    int flag_a = 0;
                    int flag_c = 0;
                    for (cmd_idx = 2 ; cmd_idx < prompt_argc ; cmd_idx++)
                    {
                        if (prompt_argv[cmd_idx][0] == '-')
                        {
                            if (prompt_argv[cmd_idx][1] == 'a')
                                flag_a = 1;
                            if (prompt_argv[cmd_idx][1] == 'c')
                                flag_c = 1;
                            else
                            {
                                err = 1;
                                break;
                            }
                        }
                    }
                    if (err)
                    {
                        main_help();
                        continue;
                    }
                    

                    if (flag_a == 0 && flag_c == 0)
                    {
                        printf("remove, flag_a=%d, flag_c=%d\n",flag_a, flag_c);
                    }
                    if (flag_a)
                    {
                        printf("remove, flag_a=%d, flag_c=%d\n",flag_a, flag_c);
                    }

                    if (flag_c)
                    {
                        printf("remove, flag_a=%d, flag_c=%d\n",flag_a, flag_c);
                    }
                    
                }

                else if (strcmp(prompt_cmd, "recover") == 0)
                {
                    char new_file [MAXPATHLEN] = {0,};
                    int err = 0;
                    int flag_d = 0;
                    int flag_n = 0;
                    for (cmd_idx = 2 ; cmd_idx < prompt_argc ; cmd_idx++)
                    {
                        if (prompt_argv[cmd_idx][0] == '-')
                        {
                            if (prompt_argv[cmd_idx][1] == 'd')
                                flag_d = 1;
                            if (prompt_argv[cmd_idx][1] == 'n')
                            {
                                flag_n = 1;
                                if (cmd_idx+1 < prompt_argc)
                                    strcpy(new_file,prompt_argv[cmd_idx+1]);   
                            }
                            else
                            {
                                err = 1;
                                break;
                            }
                        }
                    }
                    if (err)
                    {
                        main_help();
                        continue;
                    }

                    if (flag_d == 0 && flag_n == 0)
                    {
                        printf("recover, flag_d=%d, flag_n=%d, %s\n",flag_d, flag_n,new_file);
                    }
                    if (flag_d)
                    {
                        printf("recover, flag_d=%d, flag_n=%d, %s\n",flag_d, flag_n,new_file);
                    }

                    if (flag_n)
                    {
                        printf("recover, flag_d=%d, flag_n=%d, %s\n",flag_d, flag_n,new_file);
                    }

                    
                }

            }
            else if (strcmp(prompt_cmd, "ls") == 0  || strcmp(prompt_cmd, "vi") == 0  || 
                    strcmp(prompt_cmd, "vim") == 0 || strcmp(prompt_cmd, "help") == 0 || strcmp(prompt_cmd, "exit") == 0)        
            {
                printf("ss\n");
                if (strcmp(prompt_cmd, "exit") == 0)
                {
                    break;
                }
            }
            else
            {
                main_help();
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

char** prompt_token(char* char_prompt, int* prompt_argc)
{
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
    *prompt_argc = idx;
    strcpy(temp, char_prompt);
    strtok(temp, "\n");
    char** prompt_argvs = (char**)malloc(sizeof(char*) * idx);
    token_ptr = strtok(temp, " ");    
    idx = 0;
    while(token_ptr != NULL)
    {
        prompt_argvs[idx] = (char*)malloc(sizeof(char) * strlen(token_ptr));
        strcpy(prompt_argvs[idx++], token_ptr);
        token_ptr = strtok(NULL, " ");
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
            "  > help\n"
            "  > exit\n");
}
