#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PROMPT() printf("20190511> ");

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

#ifndef PROMPTLEN
#define PROMPTLEN 1024
#endif

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

typedef struct pn {

    char path[MAXPATHLEN];
    struct pn *prev;
    struct pn *next;
}pathNode;


char** path_arr(char* str); // str을 / 기준으로 토큰배열화
int realpathS (char *str); // 링크드리스트기반 경로찾기 함수
char** prom_args (char* str); // 프롬포트 토큰 배열 생성
char* tokinzer (char* str, char* del); // del tokenizer

int main(void)
{
    char prompt_str [PROMPTLEN];
    int i = 0;
    char** args;

    while(1)
    {
        PROMPT();
        if (fgets(prompt_str, PROMPTLEN, stdin) == NULL) 
            continue;

        i = 0;
        while(prompt_str[i] != '\n')
            i++;
        prompt_str[i] = '\0';

        if (!strcmp(prompt_str, "exit"))
            break;

        args = prom_args(prompt_str);
        for (i = 0 ; args[i] != NULL ; i++)
            printf("%s\n", args[i]);

    }

    exit(0);

}


// Arguement 인자를 개행 ' '단위로 토큰을 분리해서 배열에 넣어주는 함수
char** prom_args (char* str)
{
    int i,index;
    int arr_cnt = 0;
    char** token_arr;

    if (str == NULL)
        return NULL;

    i = 0;
    while (str[arr_cnt] != '\0' && str[arr_cnt] != '\n')
    {
        if (str[arr_cnt] == ' ')
            i++;
        arr_cnt++;
    }
    
    token_arr = (char**)malloc(sizeof(char*) * (i+1));
    
    char* tok = tokinzer(str, " ");
    index = 0;
    while(tok != NULL)
    {
        token_arr[index++] = tok;
        tok = tokinzer(NULL, " ");
    }

    token_arr[index] = NULL;
    return token_arr;
}


// str 을 del 뭉치 단위로 짤라주는 함수.
char* tokinzer (char* str, char* del)
{
    static char* ptr1;
    char* ptr2;
    int i = 0, j = 0;
    int del_size;

    if (del == NULL)
        return NULL;
    if (str == NULL && ptr1 == NULL)
        return NULL;
    else if (str != NULL && ptr1 == NULL)
        ptr1 = str;
    else if (str != NULL && ptr1 != NULL)
        ptr1 = str;
    
    i = 0;
    del_size = strlen(del);
    if (*ptr1 == '\n' || *ptr1 == '\0')
        return NULL;

    while(*ptr1 != '\n' && *ptr1 != '\0')
    {
        for (i = 0 ; i < del_size ; i++)
        {
            if (*ptr1 == del[i])
            {
                ptr1++;
                break;
            }
        }

        if (i >= del_size)
            break;
    }
    
    ptr2 = ptr1;
    if (*ptr1 == '\n' || *ptr1 == '\0')
        return NULL;

    i = 0;
    while(*ptr1 != '\n' && *ptr1 != '\0')
    {
        for (i = 0 ; i < del_size ; i++)
        {
            if (*ptr1 == del[i])
            {
                *ptr1 = '\0';
                break;
            }
        }
        ptr1++;
        if (i < del_size)
            break;
    }

    if (*ptr2 == '\n' || *ptr2 == '\0')
        return NULL;

    char* ret_str = (char*)malloc(strlen(ptr2) + 1);
    strcpy(ret_str, ptr2);
    
    if (*ptr1 == '\0' || *ptr1 == '\n')
        ptr1 = NULL;
    return ret_str;
}


int realpathS (char *str)
{
    if (str == NULL)
        return false;
    int idx = 0;
    char* home_path = getenv("HOME");
    char cur_path[MAXPATHLEN];
    getcwd(cur_path, MAXPATHLEN);
    char *tmp_path = (char*)malloc(MAXPATHLEN*2);
    char *path = (char*)malloc(MAXPATHLEN*2);
     
    strcpy(path, str);
    if (path[0] == '~') {
        sprintf(path, "%s/%s", home_path, str);
    }
    else if (path[0] != '/'){
        sprintf(path, "%s/%s", cur_path, str);
    }

    char **lex = path_arr(path);
    if (lex == NULL)
        return false;

    pathNode* head = (pathNode*)malloc(sizeof(pathNode));       //더미노드 생성.
    pathNode* cur = head;

    //설계과제1 응용: 링크드리스트로 경로 받아서 . 면 for문 passing, .. 이면 pop 형태로 제작.
    for (idx = 0 ; lex[idx] != NULL ; idx++)
    {
        if (!strcmp(lex[idx], "."))
            continue;
        else if (!strcmp(lex[idx], ".."))
        {
            if (cur->prev == NULL)
            {
                printf("this path %s is wrong! (out of root path)\n", str);
                return false;
            }
            cur = cur->prev;
            cur->next = NULL;
            continue;
        }

        pathNode* newNode = (pathNode*)malloc(sizeof(pathNode));
        strcpy(newNode->path, lex[idx]);
        cur->next = newNode;
        newNode->prev = cur;
        cur = newNode;
    }
    
    cur = head->next; //더미 노드이니까 하나 삭제.
    strcpy(tmp_path, "/");
    while(cur != NULL)
    {
        strcat(tmp_path, cur->path);
        strcat(tmp_path, "/");
        cur = cur->next;
    }

    if (strlen(tmp_path) > 1)
        tmp_path[strlen(tmp_path)-1] = '\0';

	if (strlen(tmp_path) >= MAXPATHLEN) 
	{
		 printf("this path %s is wrong! (path upperbound is = %d)\n", str, MAXPATHLEN);
		 printf("Your path length is %ld\n", strlen(tmp_path)+1);
		 return 0;
	}

    strcpy(str, tmp_path);
    return 1;
}

// 경로 /home/junhyeong/go2/a.c 를 home,junhyeong,go2,a.c 형태로 분할함.
char** path_arr(char* str)
{
    if (str == NULL)
        return NULL;
    
    char* ptr = str;
    char tok_path[MAXPATHLEN];
    int tk_cnt = 0;
    int i;
    while (*ptr != '\0')
    {        
        if (*ptr == '/')
            tk_cnt++;
        ptr++;
    }
    if (str[0] == '/')
        tk_cnt--;

    if (tk_cnt == 0)
        return NULL;
    
    // / 기준으로 배열을 만듦. 배열의 마지막엔 NULL이 들어감.
    char **lexeme_path = (char**)malloc(sizeof(char*) * (tk_cnt+1));
    strcpy(tok_path, str);
    
    ptr = strtok(tok_path, "/");
    for (i = 0 ; ptr != NULL ; i++)
    {
        if (ptr == NULL)
            break;
        
		size_t tok_size = strlen(ptr);
		if (tok_size >= 255) // 파일 이름의 최대길이 255 초과시
		{
			fprintf(stderr, "Max file length size is 255 Bytes\n");
			fprintf(stderr, "Your path length is %ld\n", strlen(ptr)+1);
			exit(1);
		}

        lexeme_path[i] = (char*)malloc(strlen(ptr) + 1);
        strcpy(lexeme_path[i], ptr);
        ptr = strtok(NULL, "/");
    }
    lexeme_path[i] = NULL;
    return lexeme_path;
}
