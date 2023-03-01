/**
 * 일단 프린트 폴더 목록들을 다 저장을 해두고
 * 
 *  그리고 하위 인덱스 파일에 싹 출력
 *  시작~마지막 까지 파일은 ├──
 * 
 * 
 *  scandir 로 일단 구하면 쉬을듯 하고. (할당까지 다 해줌.)
 *  계층 깊이마다 출력 -> 앞에짠 리스트대로 리스트형태로 짜면 쉬울듯함
 * 
 *  새롭게 짜는 리스트는 [filename] 을 받으면 해당 filename까지 하위디렉토리 출력 목표.
 *  / home/ junhyeong/ go2/ ~~~
 *  1-> 2  ->  3     ->4
 *  
 * 
 *  File 초기화시 자동으로 root 폴더 생성 (중요)
*/
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>                  // 정수판별
#define NUM(_NUMS) #_NUMS

#define MAXNAMLENS  512
#define MAXPATHLEN  1024
#define TREECMPCNT  5

typedef struct File {
    int count_list;                 // 하위디렉토리 개수 scandir() return 값으로 사용.
    int parent_file_cnt;            // 부모 디렉토리 파일 개수 (사용안할지도)
    int depth_level;                // 디렉토리 깊이 수준 (/ 부터 1씩증가) -> tree 출력할 때 이만큼 공백처리
    struct dirent** child_dir;      // 고민 필요있는가?
    char path [MAXPATHLEN];         // 해당 파일명에 대한 경로
    char name [MAXNAMLENS];         // 해당 파일명
    struct File *next;              // 뒷 파일 연결

    struct File *next2[TREECMPCNT];    // File 구조체가 두 개 이상 들어왔을 때 사용 -> 트리비교시 사용 (TREECMPCNT개수만큼 탐색.)
}File;

typedef struct File_Header
{
    File *root;
    char filename [TREECMPCNT][MAXNAMLENS];          //파일명들을 저장하는 구조체 : 최종 파일 명등을 저장할 때 사용.
    int depth;
}File_Header;



File* file ();
File_Header* fileheader();
File_Header* fileheaders (char* paths);
void print_struct (File_Header* ex);
void print_tree (File_Header* ex, int depth_cnt, int auto_type);
void scandirs (File_Header* this);
void recur_print(File* root, int depth);
void recur_print2(File* root, int depth);
int scandir(const char *dirp, struct dirent *** namelist,
            int(*filter)(const struct dirent *),
            int(*compar)(const struct dirent**, const struct dirent **));
/** 문자열 교체 함수.*/
char* replace (char* original, char* rep_before, char* rep_after, int cnt);
int kmp (char* origin, char* target);

/** 트리 비교함수.*/
File_Header** compare_tree (int argc, char* argv[]);
void free_file (File** del);
/** 트리 2개 비교*/

int main(int argc, char* argv[])
{
    int check = 0;
    for (int i = 1 ; i < argc ; i++)
    {
        if (strlen(argv[i]) == 1 && isdigit(argv[i][0]) != 0)
        {
            check = 1;
            break;
        }
    }
    if (check)
    {
        File_Header* ex = fileheaders(argv[1]);
        scandirs(ex);
        if (argc == 2)
        {
            print_tree(ex,0, 1);
        }
        else
        {
            print_tree(ex,atoi(argv[2]), 0);
        }
    }
    else
    {
        if (argc > 6)
        {
            fprintf(stderr, "Your unit counts are %d, Can Print Maxmum %d units\n",argc-1, TREECMPCNT);
            exit(1);
        }
        File_Header** ex = compare_tree(argc,argv);
        print_tree(ex[0],0, 1);
        for (int dels = 0 ; dels < argc-1 ; dels++)
            free(ex[dels]);

    }
    return 0;
}


/**
 *  :연결리스트의 Node 역할을 하는 file 구조체를 초기화시켜주는 함수
 *  File_Header 내부적으로 사용하기 때문에 외부에서 사용하지 말것
 * 
*/
File* file ()
{
    File *newfile = (File*)calloc(1, sizeof(File));
    newfile->count_list = 0;
    newfile->parent_file_cnt = 0;
    newfile->depth_level = 0;
    newfile->child_dir = NULL;
    newfile->next = NULL;
    
    for(int i = 0 ; i < TREECMPCNT ; i++)
        newfile->next2[i] = NULL;

    return newfile;
}

/**
 *  File_Header 구조체를 가장 처음에 생성할 때 호출하는 생성자 함수
 *  일단 모든 파일은 기본적으로 / 에서 시작하기 때문에 / 부터 생성
 * 
 *  구현 상 fileheaders() 에서 사용하기 때문에 사용하지 말것
 *          -> fileheaders() 를 사용해서 초기화할 것
 *  File_Heaeder *news = fileheaders("경로"); 처럼 사용할 것 
*/
File_Header* fileheader()
{
    File_Header* newfileheader = (File_Header*)malloc(sizeof(File_Header));
    newfileheader->root = file();
    newfileheader->root->name[0] = '/';
    newfileheader->root->path[0] = '/';
    for (int idx = 0 ; idx < TREECMPCNT ; idx++)
        memset(newfileheader->filename[idx], '\0', MAXNAMLENS);
    return newfileheader;
}


/// @Overloading
/** 
 * : File_Header 연결리스트 구조체를 절대경로만큼 자동생성해주는 함수
 *  절대경로가 아닌경우 getcwd 등을 이용해서 넣을 것.
 *  
 *  File_Heaeder *news = fileheaders("경로"); 처럼 사용할 것 
 * 
 *  ex) /home/junhyeong/go2 의 경우
 *  / -> home -> junhyeong -> go2
 * 구조체 연결리스트가 생성된다.
 *  
*/
File_Header* fileheaders (char* paths)
{
    int dir_cnt = 1;
    char path[MAXPATHLEN];
    char file_name[MAXNAMLENS];

    strcpy(path, paths);                            //복사문제 해결. (path가 할당된게 아니라 스트링 포인터를 박은 경우 일 수 있기 때문)

    if (strstr(paths, "/") == NULL)                 // 상대경로로 받은 경우 출력. 
    {
        memset(path, '\0', MAXPATHLEN);
        sprintf(path, "%s/%s",getcwd(NULL, MAXPATHLEN) , paths);    
    }

    if (strstr(paths,"./") != NULL)
    {
        char *temp_ptr = replace(path, ".", getcwd(NULL, MAXPATHLEN), 1);       // ./~~~ ==> /home/junhyeong/go2/~~~ 로 전환.
        strcpy(path, temp_ptr);
        free(temp_ptr);                                                         // replace 에서 받은 문자열 할당해제.
    }

    /** 
     * //아래 코드는 문자열 할당 후 삭제가 안되어 후처리가 힘들 수 있음.
     * strcpy(path, replace(path, ".", getcwd(NULL, MAXPATHLEN), 1));    
     * printf("%s\n",path);
     */

    for (int i = 0 ; i < strlen(path) ; i++)            // (폴더 깊이 개수 탐색.)
    {
        if (path[i] == '/')
            dir_cnt++;
    }
    File_Header* newheader = fileheader();

    File *orignal = newheader->root;

    /* 원본 파일 명 추출.*/
    char* only_file = strrchr(path, (int)'/');
    strcpy(file_name, only_file+1);
    strcpy(newheader->filename[0], file_name);
    //printf("file name is %s\n", file_name);
    if (strcmp(path, "/")==0)
    {
        strcpy(newheader->filename[0], "/");
        return newheader;
    }
    else
    {   
        struct stat statbuf;
        File *parent;
        char path_name [MAXPATHLEN];
        char *token = strtok(path, "/");
        strcpy(path_name, "/");
        int level = 1;
        while (token != NULL)
        {  
            strcat(path_name, token);
            if(stat(path_name,&statbuf) < 0)
            {
                fprintf(stderr, "making File struct Error! : %s \n", path_name);
                exit(1);
            }
            if (S_ISREG(statbuf.st_mode))               //만약에 (일반)파일이면 그냥 넘기기 -> 디렉토리파일이면 탐색
            {
                //아래 4줄 : 만약 regular파일이 경로 마지막에 온다면 부모 노드의 child_dir 을 자신만 있도록 설정.
                parent->child_dir = (struct dirent**)malloc(sizeof(struct dirent*));
                parent->child_dir[0] = (struct dirent*)malloc(sizeof(struct dirent));
                strcpy(parent->child_dir[0]->d_name, file_name);
                parent->count_list = 1;                 // 부모노드 자식리스트 1개로 설정

                token = strtok(NULL, "/");
                continue;
            }
            File *newFile = file();
            strcpy(newFile->path, path_name);           //경로추가
            strcpy(newFile->name, token);               //이름추가
            newFile->depth_level = level;               //디렉토리 깊이 수준 
            newheader->root->next = newFile;            //부모 파일구조체와 연결
            newheader->root->next2[0] = newFile;        // 트리 비교시 사용. (B-트리?)
            newheader->root = newFile;                  //root 변경 (계속 이어줘야하기때문)
            token = strtok(NULL, "/");
            strcat(path_name, "/");
            level++;
            parent = newFile;                           // 부모 기록.
        }
        newheader->root = orignal;
        newheader->depth = level;                       //총 디렉토리 깊이 저장.
        return newheader;
    }

}

/** : 임시 출력함수
 *  print_struct(ex(절대경로로 File_Header 연결리스트가 완성되어있어야함.))
*/
void print_struct(File_Header* ex)
{
    File* original = ex->root;
    for(File* head = ex->root ; head != NULL ; head = head->next )
    {
        printf("[%s]: %s level is %d\n", head->name, head->path, head->depth_level);
        for (int k = 0 ; k < head->count_list ; k++)
        {
            printf("--> %s\n", head->child_dir[k]->d_name);
        }
    }
    printf("total depth is %d\n", ex->depth);
    ex->root = original;
}


/**
 * : File_Header this 구조체의 (dirent**) dir_child 를 채워주는 함수 (즉 알맹이를 채워주는 함수)
 *  ++ count_list도 채워준다.
 * 
 *  scandir(this(절대경로로 File_Header 연결리스트가 완성되어있어야함.))
*/
void scandirs (File_Header* this)
{
    File *original = this->root;
    for (int i = 0 ; i < this->depth ; i++)
    {
        struct dirent** dir;
        struct stat statbuf;
        int dir_cnt = 0;
        if (access(this->root->path, R_OK) != 0)        //읽어지는지 판단.
        {
            fprintf(stderr, "Read Error! : %s\n", this->root->name);
            exit(1);
        }

        // 신규 추가 : /home/junhyeong/go2/KMP.c 의 경우 /home/junhyeong/go2 의 go2부분이 이미 할당되어있음 (count_list==1)
        if (this->root->next == NULL && this->root->count_list == 1)
            break;

        if ((dir_cnt = scandir(this->root->path, &dir, NULL, alphasort)) == -1)
        {
            fprintf(stderr, "scandir error! : %s\n", strerror(errno));
            exit(1);
        }
        else
        {
            this->root->child_dir = dir;
            this->root->count_list = dir_cnt;
        }

        this->root = this->root->next;
    }

    this->root = original;
}

/** 
 * :트리 형태로 해당 경로를 출력해주는 함수
 * print_tree(ex(절대경로로 File_Header 연결리스트가 완성되어있어야함.), 탐색깊이, 자동트리 옵션기능);
 *  -> auto == 1 일 경우 자동으로 Tree 끝 부터 출력.
 *
 *  recur_print () 함수를 통해 tree 출력
 * count 옵션 추가해서 depth 범위를 지정가능*/
void print_tree (File_Header* ex, int depth_cnt, int auto_type)
{
    File* original = ex->root;
    if (!auto_type)
    {
        int del_scope = ex->depth - depth_cnt + 1; //몇 번 삭제할 것인지 판단.
        if (del_scope < 0 || del_scope > ex->depth)
        {
            printf("cur max depth scope is %d, it is over than max_scope\n", ex->depth+1);
            return;
        }
        File *prev;
        for (int i = 0 ; i < del_scope ; i++)
        { 
            prev = ex->root;                // depth = 1용
            ex->root = ex->root->next;
        }

        if(ex->root == NULL)                //root == depth_max 까지 간 경우
        {
            printf("%s\n", prev->path);
            ex->root = original;
            return;
        }
    }
    //root 출력
    printf("%s\n",  ex->root->path);
   
    recur_print2(ex->root, 1);
    ex->root = original;
}


/** 
 *  :재귀적으로 tree 호출함으로서 n 깊이 과정을 수행하는 중간에
 *  n+1 깊이의 node를 출력한 후 다시 n으로 돌아 올 수 있음.
 * 
 * recur_print(File root, 깊이)
*/
void recur_print(File* root, int depth)
{
    for (int i = 0 ; i < root->count_list ; i++)
    {   
        if (strcmp(root->child_dir[i]->d_name,".")==0 || strcmp(root->child_dir[i]->d_name,"..")==0)
            continue;
        for (int k = 0 ; k < depth ; k++)
            printf("│   ");
        printf("├── %s\n",root->child_dir[i]->d_name);
        if (root->next != NULL && strcmp(root->next->name, root->child_dir[i]->d_name)==0)
            recur_print(root->next,depth+1);
    }
}

/** 
 *  : recur_print1 에서 B-tree 형태로 재귀출력하는 함수(upgrade 버젼)
 *  recur_print2(File root, 깊이)
 * 
 *  <recur_print1 설명>
 *  :재귀적으로 tree 호출함으로서 n 깊이 과정을 수행하는 중간에
 *  n+1 깊이의 node를 출력한 후 다시 n으로 돌아 올 수 있음.
 * 
*/
void recur_print2(File* root, int depth)
{
    for (int i = 0 ; i < root->count_list ; i++)
    {   
        if (strcmp(root->child_dir[i]->d_name,".")==0 || strcmp(root->child_dir[i]->d_name,"..")==0)
            continue;
        for (int k = 0 ; k < depth ; k++)
            printf("│   ");
        printf("├── %s\n",root->child_dir[i]->d_name);
        for (int idx = 0 ; idx < TREECMPCNT ; idx++)
        {
            if (root->next2[idx] != NULL && strcmp(root->next2[idx]->name, root->child_dir[i]->d_name)==0)
                recur_print2(root->next2[idx],depth+1);
        }
    }
}

/**
 * kmp 알고리즘 : 패턴문자열 인덱스 찾는 함수 : replace에서 이용
 *  
 * kmp(원본 문자열, 찾을 패턴); --> 첫 번째로 일치하는 패턴 인덱스
*/
int kmp (char* origin, char* target)
{   
    int tar_len = strlen(target);
    int origin_len = strlen(origin);
    if (tar_len <= 1)
    {
        for (int i = 0 ; i < origin_len ; i++)
        {
            if (origin[i] == target[0])
                return i;
        }
        return -1;
    }
    else
    {
        int* target_array = (int*)calloc(tar_len, sizeof(int));
        int tar_j = 0;
        for (int i = 1 ; i < tar_len ; i++)
        {
            if (target[tar_j] == target[i])
            {
                target_array[i] = tar_j+1;
                tar_j++;
            }
            else
                tar_j = 0;
        }

        tar_j = 0;
        int total_check = 0;
        for (int i = 0 ; i < origin_len ; i++)
        {

            while (tar_j > 0 && target[tar_j] != origin[i])
            {
                tar_j = target_array[tar_j-1];
                total_check = tar_j;
            }
            
            if (target[tar_j] == origin[i])
            {
                tar_j++;
                total_check++;
            }

            if (total_check == tar_len)
            {
                printf("Same String idx is %d\n", i-tar_len+1);
                return  i-tar_len+1;
            }
        }

        free(target_array);
        return -1;
    }

}
/** 
 *  original 문자열의 rep_before 을 rep_after를 cnt만큼 바꿔줌
 *  cnt == 0 이면 모든 문자열을 교체
 *
 * original 의 rep_before 을 rep_after로 교체 
 *  kmp로 일치 문자열 찾기
 *  ptr = 일치문자열 + 찾을 문자열길이 해서 뒷부분 복사
 *  원본 -> 바꿀 문자열 바꾸기
 *  붙여넣기
 * 
 *  replace (원본 문자열, 전 패턴문자열, 바꿀 패턴문자열, 몇개까지 바꿀 것인가?)
*/
char* replace (char* original, char* rep_before, char* rep_after, int cnt)
{
    char* temp_char = (char*)malloc(strlen(original));
    strcpy(temp_char,original);
    int rep_cnt = cnt == 0 ? strlen(original) : cnt; //삼항연산이용.
    for (int i = 0 ; i < rep_cnt ; i++)
    {
        char* ptr = temp_char;
        int original_len = strlen(temp_char);
        int replace_idx = kmp(temp_char, rep_before);
        if (replace_idx == -1)
            return temp_char;
        ptr += replace_idx;
        
        //후방 문자열 복사.
        int rear_string = replace_idx+strlen(rep_before);
        char *temp_rear;
        int rear_cnt;
        if (rear_string <= original_len)
        {
            rear_cnt = original_len - rear_string;
            temp_rear = (char*)malloc(rear_cnt+1);
            memcpy(temp_rear, ptr+strlen(rep_before), rear_cnt);
            temp_rear[rear_cnt] = '\0';
        }

        strcpy(ptr, rep_after);
        ptr += strlen(rep_after);
        *ptr = '\0';
        strcat(temp_char, temp_rear);
    }

    return temp_char;
}




File_Header** compare_tree (int argc, char* argv[])
{
    File_Header** nodes = (File_Header**)malloc(sizeof(File_Header*) * (argc-1));       //nodes 할당값.
    
    for (int ag = 1 ; ag < argc ; ag++)
    {
        nodes[ag-1] = fileheaders(argv[ag]);        //각각의 파일 헤더들을 생성. 그 중 가장 처음에 생성된 idx가 기준.
        scandirs(nodes[ag-1]);
    }
    
    File* original = nodes[0]->root;
    for (int idx = 1 ; idx < argc-1 ; idx++)
    {
        int same_path_check = 0;                    //같은 경로인지 체크해주는 변수 -> 같은 경로이면 아래 depth for문이 아무것도 안하고 탈출함.
        strcpy(nodes[0]->filename[idx], nodes[idx]->filename[0]);
        File* original2 = nodes[idx]->root;
        int depth_cnt = nodes[idx]->depth;
        int check_point = 0;                    //아래 (nodes[0]->root->next2[idx_j] == NULL) 가 참이면 변경. -> check==True ? break를 해주기위함.
        for (int depth = 0 ; depth < depth_cnt-1 ; depth++)
        {
            int idx_j;
            // 기준 File_Header next2[] 중 하나도 안 똑같은지 판단해야함
            for (idx_j = 0 ; idx_j <= idx ; idx_j++)
            {
                if (nodes[0]->root->next2[idx_j] == NULL)                   // 경로가 같은 파일이 하나도 없는 것이므로. 연결 + root depth 길이 조정. (긴 녀석으로 연결)
                {
                    nodes[0]->root->next = nodes[idx]->root->next;          // 호환성을 위해 next, next2 모두 연결
                    nodes[0]->root->next2[idx_j] = nodes[idx]->root->next;
                    nodes[0]->root = original;
                    nodes[0]->depth = nodes[idx]->depth;                    // root depth 길이 조정. (긴 녀석으로 연결)
                    check_point = 1;
                    same_path_check = 1;                                    // 같은 경로 아니므로 1으로 변경.
                    break;
                }

                if (strcmp(nodes[idx]->root->next->path, nodes[0]->root->next2[idx_j]->path) == 0)       // 경로가 같으면 노드 이동. (이동하면서 삭제도 가능..)
                {
                    break;
                }
            }

            if (check_point)
            {
                break;
            }
            else
            {
                if (nodes[0]->root->next2[idx_j] == NULL)
                {
                    nodes[0]->root->next2[idx_j] = nodes[idx]->root;
                    nodes[0]->root = original;
                    same_path_check = 1;                                //같은 경로 아니므로 1으로 변경
                    break;
                }
                else                                                    // 같은 경로 내에 존재하는 경우.
                {   
                    File* del = nodes[idx]->root;                       //중복되는 files는 삭제.
                    nodes[0]->root = nodes[0]->root->next2[idx_j];
                    nodes[idx]->root = nodes[idx]->root->next;
                    free_file(&del);                                           //중복되는 files는 삭제.
                }
            }
        }


        // 사실 same_path_check 없어도 같은 경로면 들어오지만, 확장성을 위해 사용
        if (same_path_check == 0)
        {
            int ccheck = 1;
            for (int x = 0 ; x < nodes[0]->root->count_list ; x++)
            {
                if (strcmp(nodes[0]->root->child_dir[x]->d_name, nodes[0]->filename[idx])==0)
                    ccheck = 0;   
            }

            if(ccheck)
            {
                File *tmp = nodes[0]->root;
                int cur_child_cnt = tmp->count_list;
                tmp->child_dir = (struct dirent**)realloc(tmp->child_dir, sizeof(cur_child_cnt)+1);
                tmp->child_dir[cur_child_cnt] = (struct dirent*)malloc(sizeof(struct dirent));
                strcpy(tmp->child_dir[cur_child_cnt]->d_name, nodes[0]->filename[idx]);
                tmp->count_list++;
            }
        }
        nodes[0]->root = original;
    }


    nodes[0]->root = original;
    // 디버그 돌리니까 최적화 오류가 발생하는 듯 함. ex> 함수 내에서 free를 해줘버리니까 메모리 오류가 발생함. (확인해보니 정설)
    /** 중요한 점 : File_Header 은 할당된 값이 없으므로 그냥 free(File_Header)를 해주면된다. 
    for (int dels = 1 ; dels < argc - 1 ; dels++)                        // 기준 nodes 제외하고 모두 삭제
        free_fileheader(&nodes[dels]);
    */
    return nodes;
}


/** 
 * : File* 구조체 모든 요소들(dirent** 포함) 을 할당해제 해주는 함수.
 * 
 *  ex>     free_File(&해당 File);
 * 
 */
void free_file (File** del)
{
    for (int i = 0 ; i <(*del)->count_list ; i++)
        free((*del)->child_dir[i]);
    free((*del)->child_dir);
    (*del)->next = NULL;
    free((*del));
}


/**
 * 
/home
└── junhyeong
    ├── a.out
    ├── go -> /mnt/c/LShared/
    ├── go2
    │   ├── Books
    │   │   ├── directory
    │   │   ├── directory.c
    │   │   ├── file1.txt
    │   │   └── file2.txt
    │   ├── StringLCS
    │   ├── StringLCS.c
    │   ├── moving
    │   ├── moving.c
    │   ├── print_text.txt
    │   ├── testir
    │   │   ├── directory
    │   │   ├── directory.c
    │   │   ├── helloWorld
    │   │   ├── helloWorld.c
    │   │   ├── newFoldersss
    │   │   ├── printTree
    │   │   ├── printTree.c
    │   │   ├── print_text.txt
    │   │   ├── rading
    │   │   ├── rading.c
    │   │   ├── test
    │   │   ├── test.c
    │   │   └── test.txt
    │   └── 설계과제실습
    │       ├── assignment1_19.c
    │       ├── assignment1_22.c
    │       ├── assignment1_22.h
    │       ├── error.txt
    │       ├── getopt
    │       ├── getopt.c
    │       ├── getopt2.c
    │       ├── hello
    │       ├── hello.c
    │       ├── pthread_5sec
    │       ├── pthread_5sec.c
    │       ├── strings
    │       ├── strings.c
    │       └── test.txt
    ├── test
    ├── test.c
    └── ~p
*/
