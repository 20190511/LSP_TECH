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
#include <sys/time.h>
#include <time.h>
#include <dirent.h>

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



//디몬 프로세스 구조체 (이중연결리스트 형태)
typedef struct lp {
    char path [MAXPATHLEN];
    
    time_t mtime; // 최종 수정 시간
    int visit;    // 생성 시 1, 디몬에서 monitoring 할 때 0으로 변경-->  다시확인할 때 0인지 확인!
    
    struct lp* prev;
    struct lp* next;
}LogPath;

typedef struct lpl {
    LogPath *head;
    LogPath *tail;
    int cnt;
}LogpathList;


char** path_arr(char* str); // str을 / 기준으로 토큰배열화
int realpathS (char *str); // 링크드리스트기반 경로찾기 함수
char** prom_args (char* str); // 프롬포트 토큰 배열 생성
char* tokinzer (char* str, char* del); // del tokenizer
int path_setting ();   // 현재경로, 디몬경로, log.txt 파일 경로를 알려주는 함수


//** 디몬 프로세스꺼
int make_time (time_t* t, char* buf); // 시간 출력
LogPath* newL (char* path);
LogpathList* newList (); // list 생성자
/* path (디렉토리만 지원) 경로의 파일 모두 재귀호출하여 Regular File에 대해서 삽입*/
LogpathList* scandir_list (LogpathList* list_arg, char* path, int depth); // list_arg 리스트 NULL 이면 새로 생성해서 return해줌
LogPath* appendL (LogpathList *list, char* path);   // fl 
LogPath* pop_path (LogpathList *list, char* path);  // list에서 path에 해당하는 노드 삭제
LogPath* pop_node (LogpathList *list, LogPath* node); // list에서 node 즉시 삭제
void printL (LogpathList *list);


char current_path [MAXPATHLEN];
char demon_path [MAXPATHLEN];
char log_path [MAXPATHLEN];
char* command_str [5] ={
    "add",
    "remove",
    "help",
    "exit",
    "tree"
};


int main(void)
{
    char prompt_str [PROMPTLEN];
    int i = 0;
    char** args;
    if (!path_setting()) {
        fprintf(stderr, "path setting error\n");
        exit(1);
    }
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

        if (!strcmp(args[1], "search"))
        {
            char pathB [MAXPATHLEN];
            strcpy(pathB, args[2]);
            
            if(!realpathS(pathB)){
                fprintf(stderr, "path making error for %s\n", args[2]);
                continue;
            }

            LogpathList* list = scandir_list(NULL, pathB, 0);
            printL(list);
        }

    }
    
    exit(0);

}




LogpathList* scandir_list (LogpathList* list_arg, char* path, int depth)
{
    DIR* dp;
    struct dirent* dnt;
    struct stat statbuf;
    char tmp_path [MAXPATHLEN];
    LogpathList* list;
    int dir_cnt = 0;
    int i;

    setbuf(stdout, NULL);   
    if (stat(path, &statbuf) < 0 ) {
        fprintf(stderr, "path error for %s\n", path);
        return NULL;
    }

    if (!S_ISDIR(statbuf.st_mode)) {
        return NULL;
    }

    if (depth == 0 && list_arg == NULL)
        list = newList();
    else
        list = list_arg;


    strcpy(tmp_path, path);
    char* ptr = tmp_path + strlen(tmp_path);
    *ptr++ = '/';
    *ptr = '\0';

    if ((dp = opendir(tmp_path)) == NULL)
    {
        fprintf(stderr, "opendir error for %s\n", path);
        return NULL;
    }
    
    while((dnt = readdir(dp)) != NULL) {
        if (strcmp(dnt->d_name, ".") && strcmp(dnt->d_name, ".."))
        {
            strcpy(ptr, dnt->d_name);

            if (stat(tmp_path, &statbuf) < 0) 
            {
                fprintf(stderr, "stat error for %s\n", tmp_path);
                return NULL;
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                scandir_list(list, tmp_path, depth+1);
            }
            else if (S_ISREG(statbuf.st_mode))
            {
                appendL(list, tmp_path);
            }
       }
    }
    return list;
}




void printL (LogpathList *list)
{
    LogPath* tmp = list->head;
    int cnt_idx;
    for (cnt_idx = 0 ; tmp != NULL ; tmp = tmp->next, cnt_idx++)
    {
        char buf[30];
        make_time(&tmp->mtime, buf);
        printf("[%-4d] : %-70s   --> (%s)\n", cnt_idx+1, tmp->path, buf);
    }
}


LogPath* newL (char* path)
{
    struct stat statbuf;
    LogPath* newNode = (LogPath*)malloc(sizeof(LogPath));
    strcpy(newNode->path, path);
    newNode->next = newNode->prev = NULL;
    newNode->visit = 1;

    if (stat(path, &statbuf) < 0) {
        fprintf (stderr, "stat error for %s\n", path);
        free(newNode);
        return NULL;
    }

    newNode->mtime = statbuf.st_mtime;
    return newNode;
}

LogpathList* newList ()
{
    LogpathList* newNode = (LogpathList*)malloc(sizeof(LogpathList));
    newNode->cnt = 0;
    newNode->head = newNode->tail = NULL;
    return newNode;
}


LogPath* appendL (LogpathList *list, char* path)
{
    if (list == NULL)
        return NULL;
    
    LogPath* newNode = newL (path);
    if (list->head == NULL)
    {
        list->head = list->tail = newNode;
        list->cnt++;
    }
    else
    {
        list->tail->next = newNode;
        newNode->prev = list->tail;
        list->tail = newNode;
        list->cnt++;
    }   

    return newNode;
}

LogPath* pop_path (LogpathList *list, char* path)
{
    if (list == NULL || list->cnt == 0)
        return NULL;

    LogPath* delNode = list->head;
    
    // 삭제 노드 찾기
    while (delNode != NULL)
    {
        if (!strcmp(delNode->path, path))
            break;
        delNode = delNode->next;
    }

    if (delNode == NULL) {
        printf("no path [%s] in list\n", path);
        return NULL;
    }

    if (list->cnt == 1)
    {
        list->head = list->tail = NULL;
    }
    else if (delNode->prev == NULL) // 가장 첫 노드
    {
        list->head = delNode->next;
        delNode->next->prev = NULL;
    }
    else if (delNode->next == NULL) // 가장 마지막 노드
    {
        list->tail = delNode->prev;
        delNode->prev->next = NULL;
    }
    else
    {
        delNode->prev->next = delNode->next;
        delNode->next->prev = delNode->prev;
    }

    list->cnt--;
    delNode->prev = delNode->next = NULL;
    return delNode;

}


LogPath* pop_node (LogpathList *list, LogPath* node)
{
    if (list == NULL || list->cnt == 0 || node == NULL)
        return NULL;

    LogPath* delNode = node;
    
    if (list->cnt == 1)
    {
        list->head = list->tail = NULL;
    }
    else if (delNode->prev == NULL) // 가장 첫 노드
    {
        list->head = delNode->next;
        delNode->next->prev = NULL;
    }
    else if (delNode->next == NULL) // 가장 마지막 노드
    {
        list->tail = delNode->prev;
        delNode->prev->next = NULL;
    }
    else
    {
        delNode->prev->next = delNode->next;
        delNode->next->prev = delNode->prev;
    }

    list->cnt--;
    delNode->prev = delNode->next = NULL;
    return delNode;
}


int make_time (time_t* t, char* buf)
{
    if (t <= 0)
        return false;

    struct tm* tm;
    if((tm = localtime(t)) == NULL) {
        fprintf(stderr, "tm error\n");
        return false;
    }

    sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
    tm->tm_hour, tm->tm_min, tm->tm_sec);

    return true;
}


int path_setting ()
{
    if (getcwd(current_path, MAXPATHLEN) < 0) {
        fprintf(stderr, "getcwd error\n");
        false;
    }

    snprintf(demon_path, MAXPATHLEN+10, "%s/daemon", current_path);
    snprintf(log_path, MAXPATHLEN+10, "%s/log.txt", current_path);
    return true;
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
    
    token_arr = (char**)malloc(sizeof(char*) * (i+3));
    token_arr[0] = demon_path;
    char* tok = tokinzer(str, " ");
    index = 1;
    while(tok != NULL)
    {
        token_arr[index++] = tok;
        tok = tokinzer(NULL, " ");
    }
    token_arr[index++] = log_path;
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
        sprintf(path, "%s%s", home_path, str+1);
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
