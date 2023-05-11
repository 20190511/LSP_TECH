#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

#ifndef PROMPTLEN
#define PROMPTLEN 1024
#endif

#ifndef PIDSIZE
#define PIDSIZE 10
#endif

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


//** 디몬 프로세스꺼
int make_time (time_t* t, char* buf); // 시간 출력
LogPath* newL (char* path);
LogpathList* newList (); // list 생성자
/* path (디렉토리만 지원) 경로의 파일 모두 재귀호출하여 Regular File에 대해서 삽입*/
LogpathList* scandir_list (LogpathList* list_arg, char* path, int depth); // list_arg 리스트 NULL 이면 새로 생성해서 return해줌
LogPath* appendL (LogpathList *list, char* path);   // fl 
LogPath* pop_path (LogpathList *list, char* path);  // list에서 path에 해당하는 노드 삭제
LogPath* pop_node (LogpathList *list, LogPath* node); // list에서 node 즉시 삭제
int compare_list (LogpathList *list, char* path);   // 연결리스트 list 와 path를 비교하며 순회
int compare_node (LogpathList *list, char* path); //list 의 있는 모든 노드와 path를 비교해주는 함수.
int check_listvisit (LogpathList *list);    // 리스트를 순회하며 1인지 0인지 체킹
int print_logchar (LogPath* node, int option, char* print_buf); // [시간][모드][경로] 를 print_buf에 만들어 주는 함수
int final_module (LogpathList *list, char* path, unsigned int timer); // daemon 프로세스에서 실행되는 loop 모듈
int printfile (int fd, char* print_buf); //fd에 파일 출력 함수
void printL (LogpathList *list); // list의 모든 요소를 출력해주는 함수.
int exclusive_swpfile (char* path); //.swp 파일은 monitoring에서 제외하는 함수.
int log_fd; //log file descriptor 전역변수
unsigned int time_sleep = 1; // -t <TIME> 옵션

int daemon_init(void);
void handle_sigusr1(int signo);
char daemon_path [MAXPATHLEN];
char monitor_path [MAXPATHLEN];
char target_path [MAXPATHLEN];
char log_path[MAXPATHLEN+10]; //log 경로
char daemon_pid[PIDSIZE]; 


// 일반적으로 ./경로 <호출한 디몬 pid> <모니터링할 경로: target_path> [<-t> <TIME>] <ssu_monitoring 의 log.txt 경로> 를 인자로 받는다.
//#define DEBUG
int main(int argc, char* argv[])
{
#ifdef DEBUG
    printf("DEBUG : curr_process : %d\n", getpid());
#endif
    pid_t pid;
    int i = 0;
    sigset_t set;
    int fd, maxfd;
    char pid_info [MAXPATHLEN+10];
    
    strcpy(daemon_path, argv[0]);
    pid = atoi(argv[1]);  // 해당 프로세스를 호출한 디몬 프로세스 pid 받아옴
    strcpy(target_path, argv[2]);
    strcpy(monitor_path, argv[argc-1]);
    
    if (argc == 6)
        time_sleep = (unsigned int)atoi(argv[4]);
    
    //printf("parent process : %d\n", pid);
    //printf("daemon process initialization\n");
    
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR){
        fprintf (stderr, "SIGUSR1 can not catch\n");
        exit(1);
    }

    if ((fd = open(monitor_path, O_CREAT | O_WRONLY | O_APPEND, 0644)) < 0) {
        fprintf(stderr, "open error for %s\n", monitor_path);
        return false;
    }

    sprintf(pid_info, "%s %d\n",target_path , pid);
    
    if (write(fd, pid_info, strlen(pid_info)) != strlen(pid_info)) {
        fprintf(stderr, "write error for\n");
        return false;
    }
    
    close(fd);
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);
    
    umask(0);
    
    printf("monitoring started (%s)\n", target_path);
    int cnt = 1;

    sprintf(log_path, "%s/log.txt", target_path);

    if ((log_fd = open(log_path, O_CREAT | O_WRONLY | O_APPEND, 0644)) < 0) 
    {
        fprintf(stderr, "open error for %s\n", log_path);
        exit(1);
    }

    while(1) 
    {
        LogpathList* list = scandir_list(NULL, target_path, 0);
        if (!final_module(list, target_path, time_sleep))
        {
            fprintf(stderr, "final module error for %s\n", target_path);
            continue;
        }
    }

    exit(0);
}

void handle_sigusr1(int signo)
{
    fprintf(stderr, "monitoring ended (%s)\n", target_path);
    close(log_fd);
    exit(0);
}

int printfile (int fd, char* print_buf)
{
    strcat(print_buf, "\n");
    if (write(fd, print_buf, strlen(print_buf)) != strlen(print_buf))
    {
        fprintf (stderr, "write error\n");
        return false;
    } 
    return true;
}

int final_module (LogpathList *list, char* path, unsigned int timer) // daemon 프로세스에서 실행되는 loop 모듈
{
    sleep(timer);
    if (!compare_list(list, path))
    {
        fprintf(stderr, "compare list [create] error\n");
        return false;
    }
    if (!check_listvisit(list))
    {
        fprintf(stderr, "check list [modify, delete] error\n");
        return false;
    }

    return true;
}


int check_listvisit (LogpathList *list)    // 리스트를 순회하며 1인지 0인지 체킹
{
    if (list == NULL)
        return false;

    LogPath* start = list->head;
    struct stat statbuf;
    char print_buf [MAXPATHLEN+40];

    while (start != NULL)
    {
        if (start->visit == 0)
        {
            start->visit = 1;
            if (stat(start->path, &statbuf) < 0)
            {
                fprintf(stderr, "stat error for %s\n", start->path);
                return false;
            }

            time_t ti = statbuf.st_mtime;
            if (ti != start->mtime)
            {
                start->mtime = ti;
                print_logchar(start, 2, print_buf);
                printfile(log_fd, print_buf);
                //printf("%s\n", print_buf); // 출력 부분
            }

            start = start->next;
        }
        else if (start->visit == 1)
        {
            LogPath* delNode = start;
            start = start->next;
            pop_node(list, delNode);
            print_logchar(delNode, 1, print_buf);
            printfile(log_fd, print_buf);
            //printf("%s\n", print_buf); // 출력부분
        }
    }
    return true;
}


/** option : 0,1 (그냥 time), 2 : node의 mtime
*/
int print_logchar (LogPath* node, int option, char* print_buf)
{
    if (node == NULL && (option < 0 || option > 2))
        return false;
    
    time_t print_time;
    char time_buf [25];
    char* opt_buf [] = {
        "create",
        "delete",
        "modfiy"
    };

    if (option == 0 || option == 1) 
    {
        time(&print_time);
    }
    else if (option == 2)
    {
        print_time = node->mtime;
    }

    if(!make_time(&print_time, time_buf))
    {
        fprintf(stderr, "make time error\n");
        return false;
    }

    sprintf(print_buf, "[%s][%s][%s]", time_buf, opt_buf[option], node->path);
    return true;
}


/** list->head 부터 노드를 차례대로 비교하면서 동일한 노드가 존재하면 visit=0 으로 바꾸고 return true;
 *  만약 list->head 와 start가 동일하면 head를 다음 노드로 이동. (compare_list() 마지막에 head 다시 원복시켜줌)
 */
int compare_node (LogpathList *list, char* path)
{
    LogPath* start = list->head;
    while(start != NULL)
    {
        if (!strcmp(start->path, path))
        {
            start->visit = 0;

            if (start == list->head)  // 시작노드랑 헤드가 같으면 이동
                list->head = list->head->next;
            return true;
        }
        start = start->next;
    }
    
    return false;
    // 여기서 node가 헤드노드면 head 다음꺼로 변경.
}


int compare_list (LogpathList *list, char* path)   // 연결리스트 list 와 path를 비교하며 순회
{
    if (list == NULL)
        return false;

    struct stat statbuf;
    struct dirent* dnt;
    DIR* dp;
    char tmp_path [MAXPATHLEN];
    char print_buf [MAXPATHLEN+40];
    LogPath* saved_head = list->head;
    int i = 0, j = 0;

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

    if ((dp = opendir(tmp_path)) == NULL)
    {
        fprintf(stderr, "opendir error for %s\n", path);
        return false;
    }
    
    while((dnt = readdir(dp)) != NULL) {
        if (strcmp(dnt->d_name, ".") && strcmp(dnt->d_name, ".."))
        {
            strcpy(ptr, dnt->d_name);

            if (stat(tmp_path, &statbuf) < 0) 
            {
                fprintf(stderr, "stat error for %s\n", tmp_path);
                return false;
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                compare_list(list, tmp_path);
            }
            else if (S_ISREG(statbuf.st_mode))
            {
                if(!compare_node(list, tmp_path))
                {
                    if (strcmp(tmp_path, monitor_path) && strcmp(tmp_path, log_path) && exclusive_swpfile(tmp_path))
                    {

                        // 리스트에 추가
                        LogPath* newNode = appendL(list, tmp_path);
                        newNode->visit = 0; // 0으로 변경
                        // [현재시간] [create] [tmp_path] 출력
                        print_logchar(newNode, 0, print_buf);
                        printfile(log_fd, print_buf);
                        //printf("%s\n", print_buf);
                    }
                }
            }
       }
    }
    
    list->head = saved_head;
    closedir(dp);
    return true;
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
                if (strcmp(tmp_path, monitor_path) && strcmp(tmp_path, log_path) && exclusive_swpfile(tmp_path))
                    appendL(list, tmp_path);
            }
       }
    }
    
    closedir(dp);
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

//.swp 파일은 monitoring에서 제외하는 함수.
int exclusive_swpfile (char* path) 
{
    if (path == NULL)
        return false;

    char tmp_buf [MAXPATHLEN];
    strcpy(tmp_buf, path);

    char* type = strrchr(tmp_buf, '.');
    if (type == NULL)
        return true;

    if (!strcmp(type, ".swp"))
        return false;
    
    return true;
}
