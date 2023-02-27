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
#define NUM(_NUMS) #_NUMS

#define MAXNAMLENS  512
#define MAXPATHLEN  1024
typedef struct File {
    int count_list;                 // 하위디렉토리 개수 scandir() return 값으로 사용.
    int parent_file_cnt;            // 부모 디렉토리 파일 개수 (사용안할지도)
    int depth_level;                // 디렉토리 깊이 수준 (/ 부터 1씩증가) -> tree 출력할 때 이만큼 공백처리
    struct dirent** child_dir;      // 고민 필요있는가?
    char path [MAXPATHLEN];         // 해당 파일명에 대한 경로
    char name [MAXNAMLENS];         // 해당 파일명
    struct File *next;              // 뒷 파일 연결
}File;

typedef struct File_Header
{
    File *root;
    int depth;
}File_Header;

File* file ();
File_Header* fileheader();
File_Header* fileheaders (char* paths);
void print_struct (File_Header* ex);
void print_tree (File_Header* ex, int depth_cnt);
void scandirs (File_Header* this);
void recur_print(File* root, int depth);
int scandir(const char *dirp, struct dirent *** namelist,
            int(*filter)(const struct dirent *),
            int(*compar)(const struct dirent**, const struct dirent **));

int main(int argc, char* argv[])
{
    File_Header* ex = fileheaders(argv[1]);
    scandirs(ex);
    print_tree(ex,atoi(argv[2]));
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
    File_Header* newfileheader = (File_Header*)calloc(1, sizeof(File_Header));
    newfileheader->root = file();
    newfileheader->root->name[0] = '/';
    newfileheader->root->path[0] = '/';
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
    strcpy(path, paths);            //복사문제 해결. (path가 할당된게 아니라 스트링 포인터를 박은 경우 일 수 있기 때문)
    for (int i = 0 ; i < strlen(path) ; i++)
    {
        if (path[i] == '/')
            dir_cnt++;
    }
    File_Header* newheader = fileheader();

    File *orignal = newheader->root;
    if (strcmp(path, "/")==0)
        return newheader;
    else
    {   
        struct stat statbuf;
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
                token = strtok(NULL, "/");
                continue;
            }
            File *newFile = file();
            strcpy(newFile->path, path_name);           //경로추가
            strcpy(newFile->name, token);               //이름추가
            newFile->depth_level = level;               //디렉토리 깊이 수준 
            newheader->root->next = newFile;            //부모 파일구조체와 연결
            newheader->root = newFile;                  //root 변경 (계속 이어줘야하기때문)
            token = strtok(NULL, "/");
            strcat(path_name, "/");
            level++;
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
        int dir_cnt = 0;
        if (access(this->root->path, R_OK) != 0)        //읽어지는지 판단.
        {
            fprintf(stderr, "Read Error! : %s\n", this->root->name);
            exit(1);
        }
        
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
 * print_tree(ex(절대경로로 File_Header 연결리스트가 완성되어있어야함.), 탐색깊이);
 * 
 *  recur_print () 함수를 통해 tree 출력
 * count 옵션 추가해서 depth 범위를 지정가능*/
void print_tree (File_Header* ex, int depth_cnt)
{
    int del_scope = ex->depth - depth_cnt + 1; //몇 번 삭제할 것인지 판단.
    if (del_scope < 0 || del_scope > ex->depth)
    {
        printf("cur max depth scope is %d, it is over than max_scope\n", ex->depth+1);
        return;
    }
    File* original = ex->root;
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
    //root 출력
    printf("%s\n",  ex->root->path);
   
    recur_print(ex->root, 1);
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
