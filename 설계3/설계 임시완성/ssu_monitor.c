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
#include <signal.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>

#define PROMPT() printf("20190511> ");

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

#ifndef PROMPTLEN
#define PROMPTLEN 1024
#endif

#ifndef PIDSIZE
#define PIDSIZE 10
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


typedef struct mt {
    char path [MAXPATHLEN];
    char pid [PIDSIZE];
    struct mt* next;
    struct mt* prev;
}Mnode;

typedef struct mtl{
    Mnode *head;
    Mnode *tail;
    int cnt;
}Mlist;



//** ssu_monitor 프로세스꺼
char** path_arr(char* str); // str을 / 기준으로 토큰배열화
int realpathS (char *str); // 링크드리스트기반 경로찾기 함수
char** prom_args (char* str); // 프롬포트 토큰 배열 생성
char* tokinzer (char* str, char* del); // del tokenizer
int path_setting ();   // 현재경로, 디몬경로, log.txt 파일 경로를 알려주는 함수
int print_tree (char* path, int depth);
Mnode* newM (char* path, char* pid);
Mlist* newMlist ();
Mnode* appendM (Mlist* list, char* path, char* pid);
int delNode (Mlist* list, char* path);  // return 은 해당 pid
int free_mlist (Mlist* list); //list를 요소를 모두 free 하는 함수.
void printM (Mlist *list); // list의 모든 요소를 출력해주는 함수.
Mlist* read_monitorfile(); //리스트의 있는 데이터를 moitor_list.txt 로부터 입력
int isExistMlist (Mlist* list, char* path); // 해당 리스트에 path가 있는지 확인
int canAddMlist (Mlist* list, char* path); // 해당 리스트에 path가 같거나 포함되거나, 포함하거나 하면 오류처리
int write_monitorfile(Mlist* list); //리스트의 있는 데이터를 moitor_list.txt 에 출력
int open_monitor_read();    //monitor_list.txt read 오픈 전용
int open_monitor_write();   //monitor_list.txt write 오픈 전용
void print_help(); //help 출력
int directory_check (char* path);

int do_add(int argc, char* args[]);
int do_delete(int argc, char* args[]);
int do_help(int argc, char* args[]);
int do_tree(int argc, char* args[]);

int do_module(int option, int argc, char* args[]);

int monitor_fd;
FILE* monitor_fp;


char current_path [MAXPATHLEN];
char demon_path [MAXPATHLEN];
char log_path [MAXPATHLEN];
char monitor_path [MAXPATHLEN];
char* command_str [5] ={
    "add",
    "delete",
    "tree",
    "help",
    "exit",
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
        int prompt_argc = 0;
        if (fgets(prompt_str, PROMPTLEN, stdin) == NULL) 
            continue;

        i = 0;
        while(prompt_str[i] != '\n')
            i++;
        prompt_str[i] = '\0';

        args = prom_args(prompt_str);
        for (prompt_argc = 0 ; args[prompt_argc] != NULL ; prompt_argc++);

        /**
        if (!strcmp(args[1], "search"))
        {
            char pathB [MAXPATHLEN];
            strcpy(pathB, args[2]);
            
            if(!realpathS(pathB)){
                fprintf(stderr, "path making error for %s\n", args[2]);
                continue;
            }

            LogpathList* list = scandir_list(NULL, pathB, 0);
            printf("Finish\n");
            if ((log_fd = open("/home/junhyeong/now/log.txt", O_CREAT | O_WRONLY | O_APPEND, 0644)) < 0) 
            {
                fprintf(stderr, "open error for %s\n", "/home/junhyeong/now/log.txt");
                continue;
            }
            if (!final_module(list, pathB, (unsigned int)atoi(args[3])))
            {
                fprintf(stderr, "final module error for %s\n", pathB);
                continue;
            }

            continue;
            //printL(list);
        }   */


        int cmd_idx = 0;
        int result = -1;
        for (cmd_idx = 0 ; cmd_idx < sizeof(command_str) / sizeof(command_str[0]) ; cmd_idx++)
        {
            if (!strcmp(args[1], command_str[cmd_idx]))
            {
                result = do_module(cmd_idx, prompt_argc, args);
                break;
            }

        }

        if (result == 10)
            break;
        else if (result == 1 || result == 0)
            continue;
        else
            print_help();
        continue;
    }
    
    exit(0);
}

//디렉토리인지 체크 // 존재하는지 체크
int directory_check (char* path)
{
    struct stat statbuf;
    if (access(path, F_OK) != 0)
    {
        fprintf(stderr, "access error for %s\n", path);
        return false;
    }

    if (stat(path, &statbuf) < 0)
    {
        fprintf(stderr, "stat error for %s\n", path);
        return false;
    }

    if (!S_ISDIR(statbuf.st_mode))
    {
        fprintf(stderr, "%s is not directory!\n", path);
        return false;
    }

    return true;
}


int do_module(int option, int argc, char* args[])
{
    int result = -1;
    switch (option)
    {
    case 0:
        result = do_add (argc, args);
        break;
    
    case 1:
        result = do_delete (argc, args);
        break;

    case 2:
        result = do_tree (argc, args);
        break;
    
    case 3:
        result = do_help (argc, args);
        break;

    case 4:
        result = 10; //브레이크 신호
        break;
    }
    return result;
}

//add 수행
int do_add(int argc, char* args[])
{
    if (argc != 4 && argc != 6)
    {
        printf("Arguement Error!!\n");
        print_help();
        return false;
    }

    if (argc == 6)
    {
        if (strcmp(args[3], "-t"))
        {
            fprintf(stderr, "add option error\n");
            return false;
        }

        int dg = 0;
        while(args[4][dg] != '\0')
        {
            if (!isdigit(args[4][dg]))
            {
                fprintf(stderr, "<TIME> can be only INTGERS , your value is %s\n", args[4]);
                return false;
            }
            dg++;
        }
    }

    char make_path[MAXPATHLEN];
    strcpy(make_path, args[2]);
    if (!realpathS(make_path))
    {
        fprintf(stderr, "make path error : %s\n", args[2]);
        return false;
    }
    else
        args[2] = make_path;

    if (!directory_check(args[2]))
    {
        return false;
    }

    Mlist* list = read_monitorfile();
    if (!canAddMlist(list, make_path))
    {
        fprintf(stderr, "%s has already been existed in list\n",make_path);
        free_mlist(list);
        return false;
    }

    pid_t pid;
    int status;
    if((pid = fork()) < 0)
    {
        fprintf(stderr, "fork error\n");
        free_mlist(list);
        return false;
    }
    else if (pid == 0)
    {
        /**
        printf("exec file : %s\n", args[0]);
        for (int i = 0 ; args[i] != NULL; i++)
            printf("%s\n", args[i]);
        execl("/usr/bin/echo", "yaho", NULL);
        */
        execv(args[0], args);
    }
    else
    {
        wait(&status);
        sleep(1);
    }
    return 1;
}
//tree 수행
int do_tree(int argc, char* args[])
{
    if (argc != 4)
    {
        printf("Arguement Error!!\n");
        print_help();
        return false;
    }
    char make_path[MAXPATHLEN];
    strcpy(make_path, args[2]);
    if (!realpathS(make_path))
    {
        fprintf(stderr, "make path error : %s\n", args[2]);
        return false;
    }
    Mlist* list = read_monitorfile();
    if(isExistMlist(list, make_path))
        print_tree(make_path, 0);
    else
    {
        fprintf(stderr, "[%s] is not existed in list\n", make_path);
        free_mlist(list);
        return false;
    }
    free_mlist(list);
    return true;
}
//help 수행
int do_help(int argc, char* args[])
{
    print_help();
    return true;
}


//delete 수행
int do_delete(int argc, char* args[])
{
    if (argc != 4)
    {
        printf("Arguement Error!!\n");
        print_help();
        return false;
    }
    Mlist* list = read_monitorfile();
    if(list == NULL)
    {
        printf("this list is NULL\n");
        return false;
    }
    int get_pid = delNode(list, args[2]);
    if (get_pid < 0)
    {
        fprintf(stderr, "wrong pid(%s) Error\n", args[2]);
        free_mlist(list);
        return false;
    }
    //printf("return pid is %d\n", get_pid);
    write_monitorfile(list);
    free_mlist(list);
    kill(get_pid, SIGUSR1);
    sleep(1);
    return true;
}



void print_help()
{
   printf("usage : ./ssu_monitor\n");
   printf(" add     <DIRPATH>\n");
   printf(" add     <DIRPATH> -t <TIME>\n");
   printf(" delete  <DAEMON_PID>\n");
   printf(" tree    <DIRPATH>\n");
   printf(" help\n");
   printf(" exit\n");
}


int isExistMlist (Mlist* list, char* path) // 해당 리스트에 path가 있는지 확인
{
    if (list == NULL)
        return false;
    
    Mnode* start = list->head;
    while (start != NULL)
    {
        if (!strcmp(start->path, path))
            return true;
        start = start->next;
    }
    return false;
}


int canAddMlist (Mlist* list, char* path) // 해당 리스트에 path가 같거나 포함되거나, 포함하거나 하면 오류처리
{
    if (list == NULL || path == NULL)
        return false;
    
    Mnode* start = list->head;
    while (start != NULL)
    {
        if (strstr(start->path, path) != NULL || strstr(path, start->path) != NULL)
            return false;
        start = start->next;
    }
    return true;
}


//리스트의 있는 데이터를 moitor_list.txt 에 출력
int write_monitorfile(Mlist* list)
{
    if (list == NULL)
        return false;

    if (!open_monitor_write())
        return false;

    Mnode* start = list->head;
    while (start != NULL)
    {
        if (fprintf(monitor_fp, "%s %s\n", start->path, start->pid) < 0)
        {
            fprintf(stderr, "write moniter_list error [%s]\n", start->path);
            exit(1);
        }
        start = start->next;
    }

    fclose(monitor_fp);
    monitor_fp = NULL;
    monitor_fd = -1;
    return true;
}

Mlist* read_monitorfile()
{

    char line_buf [MAXPATHLEN+50];
    int i = 0;
    Mlist* list = newMlist();

    if(!open_monitor_read())
        return list;

    while(!feof(monitor_fp))
    {
        if (fgets(line_buf, MAXPATHLEN+50, monitor_fp) == NULL)
            break;
        
        i = 0;
        char* read_pid = line_buf;
        char* read_path = line_buf;
        while(line_buf[i] != '\n' && line_buf[i] != '\0')
        {
            if (line_buf[i] == ' ')
            {
                line_buf[i] = '\0';
                read_pid += i+1;
            }
            i++;
        }
        line_buf[i] = '\0';

        if (read_pid == NULL || strlen(read_pid) == 0)
        {
            fprintf(stderr, "read_pid error for %s\n", line_buf);
            exit (1);
        }

        if (read_path == NULL || strlen(read_path) == 0)
        {
            fprintf(stderr, "read_path error for %s\n", line_buf);
            exit (1);
        }


        if (appendM(list, read_path, read_pid) == NULL)
            exit(1);
    }

    fclose(monitor_fp);
    monitor_fp = NULL;
    monitor_fd = -1;
    return list;
}



int open_monitor_read()
{
    struct stat statbuf;
    if((monitor_fp = fopen(monitor_path, "r")) == NULL)
        return false;
    return true;
}


int open_monitor_write()
{
    struct stat statbuf;
    if (access(monitor_path, F_OK) != 0) 
    {
        if((monitor_fp = fopen(monitor_path, "w")) == NULL)
        {
            fprintf(stderr, "fopen error for %s\n", monitor_path);
            exit(1);
        }
        else
            monitor_fd = fileno(monitor_fp); //파일 디스크립터 저장.
    }
    else
    {
        if (stat(monitor_path, &statbuf) < 0)
        {
            fprintf(stderr, "stat error for %s\n", monitor_path);
            exit(1);
        }
        
        if (!S_ISREG(statbuf.st_mode))
        {
            if(unlink(monitor_path) < 0) {
                fprintf(stderr, "unlink error for %s\n", monitor_path);
                exit(1);
            }
            if((monitor_fp = fopen(monitor_path, "w")) == NULL)
            {
                fprintf(stderr, "fopen error for %s\n", monitor_path);
                exit(1);
            }
            else
                monitor_fd = fileno(monitor_fp); //파일 디스크립터 저장.
        }
        else
        {
            if((monitor_fp = fopen(monitor_path, "w")) == NULL)
            {
                fprintf(stderr, "fopen error for %s\n", monitor_path);
                exit(1);
            }
            else
                monitor_fd = fileno(monitor_fp); //파일 디스크립터 저장.
        }
    } 
    return true;
}

// Mlist 요소를 모두 free 하는 함수.
int free_mlist (Mlist* list)
{
    Mnode* start = list->head;
    while (start != NULL)
    {
        Mnode* delNode = start;
        start = start->next;

        delNode->next = delNode->prev = NULL;
        memset(delNode->path, '\0', MAXPATHLEN);
        free(delNode);
    }
    
    list->cnt = 0;
    list->head = list->tail = NULL;
    free(list);
    return true;
}



void printM (Mlist *list) // list의 모든 요소를 출력해주는 함수.
{
    Mnode* tmp = list->head;
    int cnt_idx;
    for (cnt_idx = 0 ; tmp != NULL ; tmp = tmp->next, cnt_idx++)
    {
        printf("[%-4d] : %-70s   [pid=%s]\n", cnt_idx+1, tmp->path, tmp->pid);
    }
}


Mnode* newM (char* path, char* pid)
{
    Mnode* newNode = (Mnode*)malloc(sizeof(Mnode));
    strcpy(newNode->path, path);
    strcpy(newNode->pid, pid);
    newNode->next = newNode->prev = NULL;
    return newNode;
}


Mlist* newMlist ()
{
    Mlist* newList = (Mlist*)malloc(sizeof(Mlist));
    newList->cnt = 0;
    newList->head  = newList->tail = NULL;
    return newList;
}

Mnode* appendM (Mlist* list, char* path, char* pid)
{
    if (list == NULL)
        return NULL;
    
    Mnode* newNode = newM (path, pid);
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


int delNode (Mlist* list, char* pid)  // return 은 해당 pid
{
    if (list == NULL || list->cnt == 0)
        return -1;

    Mnode* delNode = list->head;
    
    // 삭제 노드 찾기
    while (delNode != NULL)
    {
        if (!strcmp(delNode->pid, pid))
            break;
        delNode = delNode->next;
    }

    if (delNode == NULL) {
        printf("no path [%s] in list\n", pid);
        return -1;
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

    int return_pid = atoi(delNode->pid);
    free(delNode);
    return return_pid;
}


//*** ssu_monitor 프로세스꺼
int print_tree (char* path, int depth)
{
    struct dirent** dnt;
    struct stat statbuf;
    char tmp_path [MAXPATHLEN];
    int dir_cnt = 0;
    int i;

    setbuf(stdout, NULL);   
    if (stat(path, &statbuf) < 0 ) {
        fprintf(stderr, "path error for %s\n", path);
        return false;
    }

    if (!S_ISDIR(statbuf.st_mode)) {
        return false;
    }

    strcpy(tmp_path, path);
    char* ptr = tmp_path + strlen(tmp_path);
    *ptr++ = '/';
    *ptr = '\0';

    if ((dir_cnt = scandir(path, &dnt, NULL, alphasort)) < 0)
    {
        fprintf(stderr, "scandir error for %s\n", path);
        return false;
    }

    if (depth == 0)
        printf("%s\n", path);
    
    for (i = 0 ; i < dir_cnt ; i++) {
        if (strcmp(dnt[i]->d_name, ".") && strcmp(dnt[i]->d_name, ".."))
        {
            strcpy(ptr, dnt[i]->d_name);

            for (int blank = 0 ; blank < depth ; blank++)
            {
                if (blank == 0)
                    printf("│  ");
                else
                    printf("   ");
            }

            if (i + 1 == dir_cnt)
                printf("└──  %s\n", dnt[i]->d_name);
            else
                printf("├──  %s\n", dnt[i]->d_name);


            if (stat(tmp_path, &statbuf) < 0) 
            {
                fprintf(stderr, "stat error for %s\n", tmp_path);
                return false;
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                print_tree(tmp_path, depth+1);
            }

       }
    }

    return true;
}




int path_setting ()
{
    if (getcwd(current_path, MAXPATHLEN) < 0) {
        fprintf(stderr, "getcwd error\n");
        false;
    }
    
    snprintf(monitor_path, MAXPATHLEN+20, "%s/monitor_list.txt", current_path);
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
    
    token_arr = (char**)malloc(sizeof(char*) * (i+4));
    token_arr[0] = demon_path;
    char* tok = tokinzer(str, " ");
    index = 1;
    while(tok != NULL)
    {
        token_arr[index++] = tok;
        tok = tokinzer(NULL, " ");
    }

    token_arr[index++] = monitor_path;
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

//******


