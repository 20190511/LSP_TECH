#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
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

#ifndef CHILD_PROCESS
#define CHILD_PROCESS "daemon_io"
#endif

#define DEFAULT_OPT "O_CREAT | O_WRONLY | O_APPEND"

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
void print_dyingMessage();
void fprint_process(char* opt, char* path,  char* str); //process 이용 입출력
char daemon_path [MAXPATHLEN];
char monitor_path [MAXPATHLEN];
char target_path [MAXPATHLEN];
char log_path[MAXPATHLEN+10]; //log 경로
char daemon_pid [PIDSIZE]; 
char die_path [MAXPATHLEN]; //죽으면서 다잉메시지를 남길 파일.
char child_path [MAXPATHLEN]; //디몬이 호출하는 표준입출력 전용 프로세스 경로
char* args[5] = {NULL, }; //표준 입출력 에 넣을 인자 {"자식프로세스 경로", "출력 옵션", "경로", "출력 내용", NULL};

int main(int argc, char* argv[])
{
    pid_t pid;
    int i = 0;
    
    pid = getpid();
    strcpy(monitor_path, argv[argc-1]);
    strcpy(target_path, argv[2]);
    strcpy(daemon_path, argv[0]);
    strcpy(die_path, monitor_path);
    char *ptr = strrchr(die_path, '/');
    strcpy(ptr+1, "die.txt");

    if (argc == 6)
        time_sleep = (unsigned int)atoi(argv[4]);
    
    //printf("parent process : %d\n", pid);
    //printf("daemon process initialization\n");
    
    
    pid = getpid();
    getcwd(child_path, MAXPATHLEN); //일단 자기 경로를 받아와서 수정 예정.
    ptr = child_path + strlen(child_path);
    *ptr++ = '/';
    strcpy(ptr, CHILD_PROCESS); //자식 표준 입출력 해줄 프로세스 경로
    args[0] = child_path;
    
    if(daemon_init() < 0) {
        fprintf(stderr, "daemon failed\n");
        exit(1);
    }

    exit(0);
}


//입출력 작업 완료
void print_dyingMessage()
{
    char buf[MAXPATHLEN+100];
    sprintf(buf, "monitoring ended (%s)\n", target_path);   // 입출력 부분
    fprint_process("w", die_path, buf);
}

void handle_sigusr1(int signo)
{
    print_dyingMessage();
    close(log_fd);
    exit(0);
}

int daemon_init(void) {
    pid_t pid;
    sigset_t set;
    int fd, maxfd;
    int fd_cnt;
    char pid_info [MAXPATHLEN+10];

    
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR){
        fprintf (stderr, "SIGUSR1 can not catch\n");
        exit(1);
    }

    // 1. 자식 프로세스 독립 (백그라운드 실행)
    if((pid=fork()) < 0) {
        fprintf(stderr, "fork error\n");
        return false;
    }
    else if (pid != 0)
        exit(0);
    


    // 3. 자식을 프로세스 init 프로세스를 부모로 삼도록 함.
    setsid();
    
    pid = getpid();
    //printf("process %d running as daemon\n", pid);
    // 입출력 부분 2
    /***/
    sprintf(pid_info, "%s %d\n",target_path , pid);
    fprint_process("O_CREAT | O_WRONLY | O_APPEND", monitor_path, pid_info);
    /***/

    // 7. 파일 디스크립터 모두 닫기 --> /dev/null
    fd_cnt = getdtablesize();
    for (int i = 0 ; i < fd_cnt ; i++)
        close(i);
    
    // 2. 터미널 모두 날려버리고 표준입/출/에러 다 /dev/null로 만들기.
    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);

    // 4. 터미널 관련 시그널을 모두 무시하도록함.
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    // 5. umask를 0으로 만들어버리기 (파일비트 마스크 해제)
    umask(0);
    // 6. chdir("/") 은 일단 안함



    // 입출력 부분 3
    char start_buf[MAXPATHLEN+100];
    sprintf(start_buf, "monitoring started (%s)\n", target_path);
    fprint_process("w", die_path, start_buf);
    int cnt = 1;

    // 입출력 부분 4

    sprintf(log_path, "%s/log.txt", target_path);

    /**
    if ((log_fd = open(log_path, O_CREAT | O_WRONLY | O_APPEND, 0644)) < 0) 
    {
        fprintf(stderr, "open error for %s\n", log_path);
        exit(1);
    }
    */

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

void fprint_process(char* opt, char* path,  char* str) //process 이용 입출력
{
    pid_t pid;
    pid = fork();
    if (pid != 0)
        wait(NULL);
    else if (pid == 0)
    {
        args[1] = opt;
        args[2] = path;
        args[3] = str;
        args[4] = NULL;
        execv(args[0], args);
    }
    return;
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
    if (timer != 0)
        sleep(timer);
    else
        sleep(1);
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
                strcat(print_buf, "\n");
                fprint_process(DEFAULT_OPT, log_path, print_buf);
                //printfile(log_fd, print_buf);
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
            strcat(print_buf, "\n");
            fprint_process(DEFAULT_OPT, log_path, print_buf);
            //printfile(log_fd, print_buf);
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
                        strcat(print_buf, "\n");
                        fprint_process(DEFAULT_OPT, log_path, print_buf);
                        //printfile(log_fd, print_buf);  // 입출력 부분 5
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
