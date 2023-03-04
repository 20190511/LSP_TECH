#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <openssl/sha.h>            /*현재 home에 설치되어있음 */
#include <openssl/md5.h>
#include <string.h>
#define MAXPATHLEN          4097
#define MAXFILELEN          256
#define MAXPROMPTLEN        1024

//아래부터 추가된 것들.
#include <time.h>
#define BACKUP_PATH         "/home/junhyeong/backup"    //디버그용으로 백업폴더는 /home/junhyeong/backup 으로 설정     -> 잘돌아가면 root 권한으로 /home/backup 으로 변경.
#define ACTUAL_PATH         "/home/junhyeong"
#define TIME_TYPE           20
#define BUFSIZE	            1024*16
#define HASH_LEN            41
#define START_FLIST_IDX     40                    //일단 파일 IDX는 40개로 시작
#define MAX_FILE_SIZE       100000000

char CWD [MAXPATHLEN];                              // 현재 위치 getcwd() 사용.



/**
 * 2023-3-4 :구현
 *  - 해시 비교함수                                         (완료)
 *  - 파일 경로 A->B 파일 복사.                             (완료)
 *  - 디렉토리 전부탐사         -> flist                    (완료)
 *  - 파일명 같은 파일 경로찾아서 연결리스트 구현. ->rlist    (필요X)
 *  - ls,vi 제작.
 * 
 *  - add 할 때 필요한 original 경로 와 backup 파일 해시값 비교 함수 제작
 *  - add 옵션에 따른 비교 선택.
 * 
*/


// 해시형태의 링크드리스트를 생각해보았으나 print_tree 같은 계층도를 그릴 필요 없는 경우는 그냥 단순연결리스트로 구현
typedef struct filenode {
    //초기화할 때 설정해줘야하는 값
    char path_name [MAXPATHLEN];        // 파일 경로 저장 
    char file_name [MAXFILELEN];        // 파일 이름 저장
    char actual_path [MAXPATHLEN];        // 백업파일/원래경로를 제외한 실질적 경로.
    char inverse_path [MAXPATHLEN];     // original파일이면 -> 백업파일 경로 저장 (ex) /home/junhyeong/diff.c -> 백업경로/diff.c
    char hash[HASH_LEN];                // <- 해시값 저장 (해시길이40:). (저장할 때 해시값저장.)
    struct stat file_stat;               // stat 저장 (바이트 크기 등에 사용.) -> st_size
    char back_up_time [TIME_TYPE];      // 백업 타이밍 설정
    
    //나중에 설정할 값들.
    struct filenode* next;              // Reg 파일의 경우 연결리스트 사용
}Filenode;

/** back-up 파일 연결리스트
 *  디렉토리 파일은 그냥 테이블로 구현    (그냥 테이블.)
 *  디렉토리 이외의 파일은 Reg파일로 구현 (해시함수와같은 연결리스트 형태)
 * 
 * 
 *  <file_array 구조형태는 아래와 같은형태로 사용)
 *  A_123 -> A_345 -> A_467  (<-file_rear_table[0])
 *  B_145 -> B_234   (<-file_rear_table[1])
 *  D               
 *  F_175 -> F_293 -> F_239293     
*/
typedef struct flist{
    Filenode** dir_array;

    Filenode** file_array;
    Filenode** file_rear_table;          // 연결리스트를 바로 연결하기 위한 구조. O(1)
    int* file_cnt_table;                     // 해당 각 동일명의 파일 개수
    
    int file_cnt;                       // 파일 총 개수
    int dir_cnt;                        // 디렉토리 총 개수

    int max_file_cnt;                   // file_array 최대할당량
    int max_dir_cnt;                    // dir_array 최대할당량
    //★file_rear_table, file_cnt_table 는 max_file_cnt를 따라감.
}Flist;


/**
 * : rlist : 단순 연결리스트 헤더
 * 
 *  :꼬리부분과 헤더부분 존재.
 *  (꼬리부분 :rear (헤더)) + (헤더부분 : header(새로 추가될 때마다 갱신되는 부분))
 * 생각해보니, flist에 rlist 구조체를 넣어서 만들었으면 되었음.. (아쉬운점)
 * 
*/
typedef struct rlist{
    Filenode* header;
    Filenode* rear;
    
    int file_cnt;
}Rlist;

#define PRINT_ERR(_MSG) \
{fprintf(stderr, "%S Error !\n", _MSG);}

int add_backup(char* backup_path, char* file_name);
int cmd_add();

//해싱함수 : opt:0->md5, opt:1->SHA1
char* do_hashing(FILE *f, int opt);							//option 0 :md5, 1: sha1
char* hash_to_string(unsigned char *md);

// 해시 비교함수
int hash_compare (Filenode* a_node, Filenode* b_node);

// 구조체 초기화 함수들.
Filenode* new_filenode ();                                           // 기본 초기화
Filenode* new_filenodes (char* filename, int opt, int f_opt);        // 파일 초기화, option 0: original, 1: backup
void flist_sizeup (Flist* flst);                                     // Flist 인덱스 사이즈업.
Flist* new_flist ();
Rlist* new_Rlist();
void print_node (Filenode* node);                                    // Filenode Unit 상태 출력
void print_rlist (Rlist* rlist);                                     // rlist 모든 요소 출력
void print_flist (Flist* flist);                                     // flist 모든 요소 출력
void append (Flist* flist, char* file_name, int opt, int f_opt);     // flist 파일 array 대해 추가. option 0: orignal, 1: Backup
void delete (Flist* flist, char* del_path, int f_opt);               // flist 해당 경로 찾아서 삭제 (미구현)
void rappend (Rlist* rlist, char* file_name, int opt, int f_opt);    // Rlist 에 file_name 경로 데이터 단순 연결
Filenode* rpopleft (Rlist* rlist);                                   // Rlist 큐 popleft

// 파일 복사 함수
int file_cpy (char* a_file, char* b_file);
int node_file_cpy (Filenode* a_node);
int make_directory (char* dest);

// 현재시간 _230227172231 (현재시간 생성 개체)
char* curr_time();

// 파일 탐색 함수.
Rlist* original_search(char* file_name, int f_opt, int all);           // 그냥 연결리스트 구현 (동작확인완료 . 3.04)
Flist* backup_search(char* file_name, int f_opt, int all);             // 해시 체이닝(연결리스트) 구현. (동작확인완료 . 3.04)
int scandir(const char *dirp, struct dirent *** namelist,
            int(*filter)(const struct dirent *),
            int(*compar)(const struct dirent**, const struct dirent **));


// 1. add 계열함수
int ssu_add (char* file_name, int flag, int f_opt);


int main(void)
{
    int check = ssu_add("/home/junhyeong/lect", 1, 0);
    //잘되는거 확인완료
    /*
    Rlist* original_sub_path = original_search("/home/junhyeong", 1, 1);
    printf("%s sub dir cnt is %d\n", original_sub_path->rear->file_name, original_sub_path->file_cnt);
    print_rlist(original_sub_path);
    

    Flist* flist_sub_path = backup_search("/home/junhyeong/backup", 1, 1);
    printf("%s sub dir cnt is %d+%d\n", flist_sub_path->dir_array[0]->file_name, flist_sub_path->file_cnt, flist_sub_path->dir_cnt);
    print_flist (flist_sub_path);
    */
    // 잘되는거 확인완료.
    /*
	Filenode* newfile = new_filenodes("diff.c_230227172302", 1,0);
	Filenode* newfile2 = new_filenodes("diff.c_230227172319", 1,0);
	Filenode* newfile4 = new_filenodes("a.c_230227172320", 1,0);
	Filenode* newfile3 = new_filenodes("diff.c", 0,0);
	Filenode* newfile5 = new_filenodes("/home/junhyeong/file2.cpp", 0,0);
	Filenode* newfile6 = new_filenodes("tt/a.c_230227172325", 1,0);

    print_node(newfile);
    print_node(newfile2);
    print_node(newfile3);
    print_node(newfile4);
    print_node(newfile5);
    Rlist* newflist = new_Rlist();
    rappend(newflist,"diff.c_230227172302",1,0);
    rappend(newflist,"diff.c_230227172319",1,0);
    rappend(newflist,"diff.c_230227172320",1,0);    
    rappend(newflist,"a.c_230227172320",1,0);   

    rpopleft(newflist);
    rpopleft(newflist);
    rpopleft(newflist);
    rpopleft(newflist);
    rpopleft(newflist);
    */
    //동작 확인완료
    /*make_directory("/home/junhyeong/backup/test1/test2/test3.c");*/

    //동작 확인완료 
    /*
    if( file_cpy("/home/junhyeong/go2/2023_assignment", "/home/junhyeong/backup/2023_assignment") <0)
    {
        fprintf(stderr, "File Cpy Error \n");
        exit(1);
    }
    */
    
    /* 
    //잘 돌아가는거 확인
    if(node_file_cpy(newfile3) < 0)
    {
        fprintf(stderr, "File Cpy Error \n");
        exit(1);
    }
    
    if(node_file_cpy(newfile6) < 0)
    {
        fprintf(stderr, "File Cpy Error \n");
        exit(1);
    }
    */
	exit(0);
}

int ssu_add (char* file_name, int flag, int f_opt)
{
    Filenode *tmp_node = new_filenodes(file_name, 0, f_opt);
    if (tmp_node == NULL)
    {
        printf("Error!\n");
        return 0;
    }
    char original_path[MAXPATHLEN] = {0,};
    char backup_path[MAXPATHLEN] = {0,};

    strcpy(original_path, tmp_node->path_name);
    strcpy(backup_path, tmp_node->inverse_path);

    if (flag)       //해당경로로 모두 탐색.
    {
        if (S_ISREG(tmp_node->file_stat.st_mode))
        {
            printf("미구현 ㅎㅎ\n");
            return 1;
        }
        if (S_ISDIR(tmp_node->file_stat.st_mode))
        {
            char* token_ptr = strrchr(backup_path, '/');
            *token_ptr = '\0';
            Rlist* original_node = original_search(original_path, f_opt, 1);
            Flist* backup_node = backup_search(backup_path, f_opt, 1);

            Filenode* cpy_node = original_node->rear;
            for (int file_cnt = 0 ; file_cnt < original_node->file_cnt ; file_cnt++)
            {
                if (!S_ISDIR(cpy_node->file_stat.st_mode))
                {
                    int check = 1;
                    if (backup_node != NULL)
                    {
                        for (int cnt = 0 ; cnt < backup_node->file_cnt ; cnt++)
                        {
                            if (strcmp(backup_node->file_array[cnt]->file_name, cpy_node->file_name) == 0)
                            {
                                for (int i = 0 ; i < backup_node->file_cnt_table[cnt] ; i++)
                                {
                                    Filenode* node = backup_node->file_array[cnt];
                                    if (strcmp(node->hash, cpy_node->hash) == 0)
                                    {
                                        check = 0;
                                        printf("\"%s\" is already backuped\n", node->path_name);
                                        break;
                                    }
                                    node = node->next;
                                }
                                if (!check)
                                    break;
                            }
                        }
                    }
                    if (check)
                    {
                        printf("\"%s\" backuped\n", cpy_node->inverse_path);
                        node_file_cpy(cpy_node);
                    }
                }
                cpy_node = cpy_node->next;
            }
        }
        return 0;

    }
    else
    {
        if (S_ISDIR(tmp_node->file_stat.st_mode))
        {
            printf("\"%s\" is a directory file\n", tmp_node->path_name);
            return 0;
        }
        // 실제로 해보니 해당 경로에 있는 값들을 다 가져와서 비교해야됨
        char* token_ptr = strrchr(backup_path, '/');
        *token_ptr = '\0';
        Rlist* original_node = original_search (original_path, f_opt, 0);
        Flist* backup_node = backup_search (backup_path, f_opt, 0);

        int check = 1;
        if (backup_node != NULL)
        {
            for (int cnt = 0 ; cnt < backup_node->file_cnt ; cnt++)
            {
                if (strcmp(backup_node->file_array[cnt]->file_name, original_node->header->file_name) == 0)
                {
                    for (int i = 0 ; i < backup_node->file_cnt_table[cnt] ; i++)
                    {
                        Filenode* node = backup_node->file_array[cnt];
                        if (strcmp(node->hash, original_node->header->hash) == 0)
                        {
                            check = 0;
                            printf("\"%s\" is already backuped\n", node->path_name);
                            break;
                        }
                        node = node->next;
                    }
                    if (!check)
                        break;
                }
            }
        }

        if (check)      // 동일한 해시가 존재하기때문에 생성할 필요 없음.
        {
            printf("\"%s\" backuped\n", original_node->header->inverse_path);
            node_file_cpy(original_node->header);
        }
    }
    return 1;
}




void print_flist (Flist* flist)
{
    printf("================ Start Flist Print ===============\n");
    printf("-----------------Backup Dictinary-----------------\n");
    for (int i = 0 ; i < flist->dir_cnt ; i++)      //딕셔너리 출력
    {
        printf("%s\n", flist->dir_array[i]->path_name);
    }
    printf("-----------------Backup File    -----------------\n");
    for (int i = 0 ; i < flist->file_cnt ; i++)     //파일 출력
    {
        if (flist->file_cnt_table[i] > 1)
        {
            Filenode* original = flist->file_array[i];
            for (int x = 0 ; x < flist->file_cnt_table[i] ; x++)
            {
                printf("[%d] >> same file : %s\n",i+1, original->path_name);
                original = original->next;
            }
        }
        else
        {
            printf("[%d] %s\n",i+1, flist->file_array[i]->path_name);
        }
    }
    printf("================ End Flist Print ===============\n");

}

void print_rlist (Rlist* rlist)
{
    Filenode* origin = rlist->rear;
    for (int i = 0 ; i < rlist->file_cnt ; i++)
    {
        printf("%s\n",rlist->rear->path_name);
        rlist->rear = rlist->rear->next;
    }
    rlist->rear = origin;
}


/**
 *  original_search -> Orignal 폴더 경로 하위 폴더 및 해당 파일 리스트 반환
 *  : 일반적인 경로에 있는 폴더 탐색함수.
 *      
 *  
 *  all 0:해당 파일/디렉토리안 파일만. 1:하위 모든파일
 */
Rlist* original_search(char* file_name, int f_opt, int all)           // 그냥 연결리스트 구현
{
    Filenode* rootnode = new_filenodes(file_name, 0, f_opt);
    Rlist* rlist = new_Rlist();
    if (rootnode == NULL)
    {
        free(rootnode);
        free(rlist);
        return NULL;
    }
    
    if (S_ISDIR (rootnode->file_stat.st_mode))
    {
        rappend(rlist, rootnode->path_name, 0, f_opt);
        if (all)
        {
            Filenode* original = rlist->rear;
            while(rlist->rear != NULL)
            {
                char path_name [MAXPATHLEN] = {0,};
                strcpy(path_name, rlist->rear->path_name);
                char *modify_ptr = path_name + strlen(path_name);                   //이름 수정할 포인터

                char* orignal_ptr = modify_ptr;
                if (S_ISDIR(rlist->rear->file_stat.st_mode))
                {
                    strcat(path_name, "/");
                    modify_ptr++;
                    orignal_ptr = modify_ptr;
                    if (access(rootnode->path_name, R_OK) != 0)
                    {
                        fprintf(stderr, "Access Error :%s \n", rootnode->path_name);
                        rlist->rear = rlist->rear->next;
                        continue;
                    }

                    struct dirent** sub_dir;
                    int file_cnt;
                    if ((file_cnt=scandir(rlist->rear->path_name, &sub_dir, NULL, alphasort)) < 0)
                    {
                        printf("open dir error :%s \n", rootnode->path_name);
                        rlist->rear = rlist->rear->next;
                        continue;
                    }

                    for (int i = 0 ; i < file_cnt ; i++)
                    {
                        char sub_file_name[MAXFILELEN];
                        strcpy(sub_file_name, sub_dir[i]->d_name);
                        if (strcmp(sub_file_name, ".") == 0 || strcmp(sub_file_name, "..") == 0)
                            continue;
                        strcpy(modify_ptr, sub_file_name);
                        rappend(rlist, path_name, 0, f_opt);       //만약에 오류나면 modify_ptr 이 char* 으로 전달되고있음을 생각해볼것
                        free(sub_dir[i]);   
                        modify_ptr = orignal_ptr;
                    }
                    free(sub_dir);
                    rlist->rear = rlist->rear->next;                // rear이 디렉토리인 경우 (탐색하고 다음 노드로)
                }
                else
                {
                    rlist->rear = rlist->rear->next;                // rear이 디렉토리가 아닌 경우 (탐색하지 않고 다음 노드로)
                }
            }
            rlist->rear = original; //원 노드로 복귀
            return rlist;
        }
        else        //자기 하위 파일만 탐색하는 경우.
        {
            //디렉토리는 제외하고 REG 파일만 백업
            rappend(rlist, rootnode->path_name, 0, f_opt);

            char path_name [MAXPATHLEN] = {0,};
            sprintf(path_name, "%s/", rootnode->path_name);                  //경로 저장.
            char *modify_ptr = path_name + strlen(path_name);                   //이름 수정할 포인터
            char* original_ptr = modify_ptr;
            int file_cnt;
            struct dirent** sub_dir;
            if ((file_cnt=scandir(rlist->rear->path_name, &sub_dir, NULL, alphasort)) < 0)
            {
                printf("open dir error :%s \n", rootnode->path_name);
                return NULL;
            }

            for (int i = 0 ; i < file_cnt ; i++)
            {
                char sub_file_name[MAXFILELEN];
                strcpy(sub_file_name, sub_dir[i]->d_name);
                if (strcmp(sub_file_name, ".") == 0 || strcmp(sub_file_name, "..") == 0)
                    continue;
                strcpy(modify_ptr, sub_file_name);
                rappend(rlist, path_name, 0, f_opt);       //만약에 오류나면 modify_ptr 이 char* 으로 전달되고있음을 생각해볼것
                free(sub_dir[i]);   
                modify_ptr = original_ptr;
            }
            free(sub_dir);
            
            return rlist;
        }
    }
    else
    {
        rappend(rlist, rootnode->path_name, 0, f_opt);
        return rlist;
    }

}

/**
 *  backup_search -> backup 폴더 경로 하위 폴더 및 해당 파일 리스트 반환
 * : all 0:해당 파일/디렉토리안 파일만. 1:하위 모든파일
 */
Flist* backup_search(char* file_name, int f_opt, int all)             // 해시 체이닝 구현.
{
    Filenode* rootnode = new_filenodes(file_name, 0, f_opt);
    Flist* flist = new_flist();
    if (rootnode == NULL)
    {
        free(rootnode);
        free(flist);
        return NULL;
    }
    
    if (S_ISDIR (rootnode->file_stat.st_mode))
    {
        append(flist, rootnode->path_name, 1, f_opt);
        if (all)
        {
            for (int dir_idx = 0 ; dir_idx < flist->dir_cnt ; dir_idx++)
            {
                char* dir_name = flist->dir_array[dir_idx]->path_name;
                char tmp_name[MAXPATHLEN] = {0,};
                sprintf(tmp_name, "%s/", dir_name);
                char* modify_ptr = tmp_name + strlen(tmp_name);
                char* original_ptr = modify_ptr;                //복구시키는 기능.

                if (access(dir_name, R_OK) != 0)
                    continue;
                
                struct dirent** sub_dir;
                int dir_unit_cnt;
                if ((dir_unit_cnt=scandir(dir_name, &sub_dir, NULL, alphasort)) < 0)
                {
                    printf("open dir error :%s \n", rootnode->path_name);
                    return NULL;
                }

                for(int i = 0 ; i < dir_unit_cnt ; i++)
                {
                    char sub_file_name[MAXFILELEN];
                    strcpy(sub_file_name, sub_dir[i]->d_name);
                    if (strcmp(sub_file_name, ".") == 0 || strcmp(sub_file_name, "..") == 0)
                        continue;
                    strcpy(modify_ptr, sub_file_name);
                    append(flist, tmp_name, 1, f_opt);       //만약에 오류나면 modify_ptr 이 char* 으로 전달되고있음을 생각해볼것
                    free(sub_dir[i]);   
                    modify_ptr = original_ptr;
                }
                free(sub_dir);
            }
            return flist;
        }
        else   // 디렉토리가 아니라 그냥 파일인경우.
        {
            char* dir_name = flist->dir_array[0]->path_name;
            char tmp_name[MAXPATHLEN] = {0,};
            sprintf(tmp_name, "%s/", dir_name);
            char* modify_ptr = tmp_name + strlen(tmp_name);
            char* original_ptr = modify_ptr;                //복구시키는 기능.

            if (access(dir_name, R_OK) != 0)
                return NULL;
            struct dirent** sub_dir;
            int dir_unit_cnt;
            if ((dir_unit_cnt=scandir(dir_name, &sub_dir, NULL, alphasort)) < 0)
            {
                printf("open dir error :%s \n", rootnode->path_name);
                return NULL;
            }

            for(int i = 0 ; i < dir_unit_cnt ; i++)
            {
                char sub_file_name[MAXFILELEN];
                strcpy(sub_file_name, sub_dir[i]->d_name);
                if (strcmp(sub_file_name, ".") == 0 || strcmp(sub_file_name, "..") == 0)
                    continue;
                strcpy(modify_ptr, sub_file_name);
                append(flist, tmp_name, 1, f_opt);       //만약에 오류나면 modify_ptr 이 char* 으로 전달되고있음을 생각해볼것
                free(sub_dir[i]);   
                modify_ptr = original_ptr;
            }
            free(sub_dir);

            return flist;
        }
    }
    else
    {
        append(flist, rootnode->file_name, 1, f_opt);
        return flist;
    }

}




/** 
 *  : 파일 복사할 때 서브 디렉토리가 존재하는지 여부 체크
 *  존재하지 않으면 새로생성.
 */
int make_directory (char* dest)
{
    char tmp_path [MAXPATHLEN] = {0,};
    
    char tmp_dest [MAXFILELEN] = {0,};
    strcpy(tmp_dest, dest);
    char* token_dest = strtok(tmp_dest, "/");
    while(token_dest != NULL)
    {
        strcat(tmp_path,"/");
        strcat(tmp_path, token_dest);
        token_dest = strtok(NULL, "/");  
        if (token_dest != NULL)             // strtok 값이 NULL 이라는건 마지막파일이라는 의미/
        {
            if (access(tmp_path, F_OK) != 0)
            {
                if (mkdir(tmp_path, 0777) < 0)
                {
                    printf("Making directory : %s Error !\n", tmp_path);
                }
                sleep(0.5);
            }
        }
    }
}


/**
 *  현재 시간 _230227172231 생성
 *  ★ _ 가 안 붙여서 나오기 때문에 _붙여서 사용할것 
 */
char* curr_time()
{
    time_t seconds = time(NULL);
    struct tm* now = localtime(&seconds);   // 시간 생성


    char* file_suffix = (char*)malloc(sizeof(char)*TIME_TYPE);            //_230227172231 (백업시간) 접미어 생성.
    memset(file_suffix, '\0', TIME_TYPE);
    sprintf(file_suffix,"%d%02d%02d%02d%02d%02d",                        
            now->tm_year-100, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

    return file_suffix;
}


/** 
 *  file_cpy()  --> 성공시 1, 실패시 0
 * : 단순 a경로->b경로 파일 복사.
 *  
 *  동작확인
 */
int file_cpy (char* a_file, char* b_file)
{
    if (access(a_file,R_OK) != 0)
        return 0;

    int fd1, fd2;
    char read_buf[BUFSIZE] = {0,};                      //BUFSIZE   =    1024*16*

    fd1 = open (a_file, O_RDONLY);
    fd2 = open (b_file, O_WRONLY | O_CREAT | O_TRUNC , S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd1 < 0)
    {
        fprintf(stderr, "Fopen Error : %s\n", a_file);
        return 0;
    }

    make_directory(b_file);
    if (fd2 < 0)
    {
        fprintf(stderr, "Fopen Error : %s\n", b_file);
        return 0;
    }

    int read_cnt;
    while((read_cnt =read(fd1, read_buf, BUFSIZE)) > 0)
    {
        int write_cnt = write(fd2, read_buf, read_cnt);
        //printf("%s\n", read_buf);
        if (write_cnt != read_cnt)
        {
            printf("write_error :%s\n", b_file);
            close(fd1);
            close(fd2);
            return 0;
        }
    }

    close(fd1);
    close(fd2);
    return 1;
}



/** 
 * node로 받는 파일복사함수 (A->B 복사)
 *      성공하면 1 실패하면 0
 * a_node 가 디렉토리라면 해당 경로에 디렉토리 생성.
 */
int node_file_cpy (Filenode* a_node)
{ 
    if (a_node == NULL)
        return 0;
    
    if (S_ISDIR(a_node->file_stat.st_mode))
    {
        if (access(a_node->inverse_path, F_OK) != 0)
        {
            if (mkdir(a_node->inverse_path, 0777) < 0)
            {
                printf("Make Directory Error! : %s\n", a_node->inverse_path);
            }
            sleep(0.5);
        }
        return 1;
    }


    // original->backup     의 경우에는 원래파일 + 백업시간을 붙여서 생성해야하고,
    // back_up-> original   의 경우에는 백업시간붙은파일 -> 분리시켜서 보내야한다.
    int check = file_cpy(a_node->path_name, a_node->inverse_path);

    if (check)          // 성공하면 1 실패하면 0이다.
        return 1;
    else
        return 0;
}

/** 해시 비교함수
 *  같으면 1 , 다르면 0
 * 
 */
int hash_compare (Filenode* a_node, Filenode* b_node)
{
    char* a_hash = a_node->hash;
    char* b_hash = b_node->hash;
    return strcmp(a_hash,b_hash) == 0;
}

/** Rlist 초기화함수 */
Rlist* new_Rlist()
{
    Rlist *newRlist = (Rlist*)malloc(sizeof(Rlist));
    newRlist->file_cnt = 0;
    newRlist->header = newRlist->rear = NULL;
    return newRlist;
}


/** Rlist에 단순 연결*/
void rappend (Rlist* rlist, char* file_name, int opt, int f_opt)
{
    Filenode* newnode = new_filenodes(file_name, opt, f_opt);
    if (newnode == NULL)
    {
        printf("this filename %s is can't be opened\n", file_name);
        return;
    }

    if (rlist->file_cnt == 0)
    {
        rlist->header = rlist->rear = newnode;
        rlist->file_cnt = 1;
    }
    else
    {
        rlist->header->next = newnode;
        rlist->header = newnode;
        rlist->file_cnt++;
    }
}


/** 
 *  rplopleft(rlist)    --> Filenode* 삭제될 노드
 * : 큐형태로 rear 부분을 지워주고 return해주는 함수
 *  
 * 
 * ★ free를 해주지 않으므로 사용하고 return 받은 node를 꼭 free 해줄 것.
*/
Filenode* rpopleft (Rlist* rlist)
{
    if( rlist->file_cnt == 0 )
    {
        printf("This list is NULL\n");
        return NULL;
    }

    Filenode* return_node;
    if (rlist->file_cnt == 1)
    {
        return_node = rlist->rear;
        rlist->header = rlist->rear = NULL;
        rlist->file_cnt--;
        return return_node;
    }
    else
    {
        return_node = rlist->rear;
        rlist->rear = rlist->rear->next;
        rlist->file_cnt--;
        return return_node; 
    }
}



/**
 * Filenode 생성자(기초) : new_filenodes 에서 사용하는 내부 함수 (쌩으로 쓰는 것 지양할것)
 * 
*/
Filenode* new_filenode ()                                // 기본 초기화
{
    Filenode* newfile = (Filenode*)malloc(sizeof(Filenode));
    memset(newfile->hash, '\0', HASH_LEN);
    memset(newfile->path_name, '\0', MAXPATHLEN);
    memset(newfile->file_name, '\0', MAXFILELEN);
    memset(newfile->actual_path, '\0', MAXPATHLEN);
    memset(newfile->inverse_path, '\0', MAXPATHLEN);
    memset(newfile->back_up_time, '\0', TIME_TYPE);
    newfile->next = NULL;
    return newfile;
}

/**
 *  : 해당 filename 을 opt 옵션으로 Filenode 생성 
 * 
 * opt      0: original, 1: backup,
 * f_opt    0: md5       1: SHA-1
 * 
 * *  <초기화 해주는 것들>
 *   char path_name [MAXPATHLEN];         // 파일 경로 저장 
 *   char file_name [MAXFILELEN];         // 파일 이름 저장
 *   char actual_path [MAXPATHLEN];       // 백업파일/원래경로를 제외한 실질적 경로.
 *   char hash[HASH_LEN];                 // <- 해시값 저장 (해시길이40:). (저장할 때 해시값저장.)
 *   struct stat file_stat;               // stat 저장 (바이트 크기 등에 사용.) -> st_size
 * 
 *  ★ 주의! a.txt_230227172331 의 경우 
 *  file_name = a.txt
 *  back_up_time = 230227172331 로 설정된다.
 *   
 */ 
Filenode* new_filenodes (char* filename, int opt, int f_opt)        
{
    char tmp_path[MAXPATHLEN] = {0,};
    Filenode* newfile = new_filenode();

    if(filename[0] != '/') // 절대경로가 아닌경우,
    {
        sprintf(newfile->path_name,"%s/%s", opt==1 ? BACKUP_PATH : getcwd(NULL, MAXPATHLEN), filename);    
    }
    else
    {
        if (strstr(filename, "/home") == NULL)                                                       // /home 을 벗어난 파일은 애초에 오류임.
        {
            free(newfile);
            return NULL;
        }

        strcpy(newfile->path_name, filename);
    }
    strcpy(tmp_path, newfile->path_name);
    char* tks = tmp_path + strlen(opt==1 ? BACKUP_PATH : ACTUAL_PATH);
    strcpy(newfile->actual_path, tks);

    char* tmp_ptr = strrchr(newfile->path_name, '/');
    tmp_ptr++;
    strcpy(newfile->file_name, tmp_ptr);

    if (opt == 1)
    {
        char origin_filename[MAXFILELEN] = {0,};
        strcpy(tmp_path, newfile->file_name);
        
        char* new_ptr = strrchr(newfile->file_name,'_');        //백업 시간 결정. (strtok를 쓰기엔 _가 파일명에 들어가 있을 수 있음.)
        char* name_ptr = newfile->file_name;
        if (new_ptr != NULL)                                    // a.txt_230227172331 이걸 -> a.txt 와 230227172331 로 구분.
        {
            new_ptr++;
            strcpy(newfile->back_up_time, new_ptr);
            new_ptr--;
            char* cpy_cnt = name_ptr + (new_ptr - name_ptr);
            memset(cpy_cnt, '\0', TIME_TYPE);
            *cpy_cnt = '\0';
        }

        
    }
    if (opt == 1)
    {
        sprintf(newfile->inverse_path,"%s%s", ACTUAL_PATH, newfile->actual_path);
        char* time_token = strrchr(newfile->inverse_path, '_');
        if (time_token != NULL)
            memset(time_token,'\0', TIME_TYPE);
    }
    else
    {
        char* cur_time_ptr = curr_time();
        strcpy(newfile->back_up_time, cur_time_ptr);
        sprintf(newfile->inverse_path,"%s%s_%s", BACKUP_PATH, newfile->actual_path,cur_time_ptr);
        
    }


    if (access(newfile->path_name, R_OK) != 0)          //없거나 접근 불가능할 때,
    {
        free(newfile);
        return NULL;
    }

    //접근가능하니까 해싱이랑 stat 넣어주기
    if (stat(newfile->path_name, &(newfile->file_stat)) < 0)
    {
        fprintf(stderr, "%s : stat error\n", newfile->path_name);
    }

    if (!S_ISDIR(newfile->file_stat.st_mode) && !S_ISREG(newfile->file_stat.st_mode))        //만들고보니까 파이프파일 같은 함정카드다? 없애기
    {
        free(newfile);
        return NULL;
    }
    
    //파일용량 (MAX_FILE_SIZE : 100000000) 로 제한 : 안 해주면 4기가짜리 파일 읽는데 시간 엄청걸림
    if (newfile->file_stat.st_size > MAX_FILE_SIZE)
    {
        printf("%s file size is %ld, pass\n", newfile->path_name, newfile->file_stat.st_size);
        free(newfile);
        return NULL;
    }    
    FILE *fp_hash = fopen(newfile->path_name, "r");
    char* hash_ptr = do_hashing(fp_hash, f_opt);
    if (hash_ptr == NULL)
    {
        fprintf(stderr, "hash_err : %s\n", newfile->path_name);
    }

    strcpy(newfile->hash, hash_ptr);
    free(hash_ptr);
    fclose(fp_hash);
    return newfile;
}


Flist* new_flist ()
{
    Flist *newflist = (Flist*)malloc(sizeof(Flist));
    newflist->dir_array = (Filenode**)malloc(sizeof(Filenode*)*START_FLIST_IDX);

    newflist->file_array = (Filenode**)malloc(sizeof(Filenode*)*START_FLIST_IDX);
    newflist->file_rear_table = (Filenode**)malloc(sizeof(Filenode*)*START_FLIST_IDX);
    newflist->file_cnt_table =  (int*)malloc(sizeof(int)*START_FLIST_IDX);
    for (int i = 0 ; i < START_FLIST_IDX ; i++)
    {
        newflist->file_array[i] = NULL;
        newflist->dir_array[i] = NULL;
        newflist->file_rear_table[i] = NULL;
        newflist->file_cnt_table[i] = 0;
    }
    newflist->file_cnt = 0;
    newflist->dir_cnt = 0;
    newflist->max_file_cnt = START_FLIST_IDX;
    newflist->max_dir_cnt = START_FLIST_IDX;

    return newflist;
}


/**
 *  : flist 에 file_name을 추가해주는 함수 (opt에 따라 연결리스트 여부 자동결정)
 * 
*/
void append (Flist* flist, char* file_name, int opt, int f_opt)                  // 파일 array 대해 추가. option 0: orignal, 1: Backup
{
    Filenode* newfile = new_filenodes(file_name, opt, f_opt);
    int dir_check = 0;
    if (newfile == NULL)
        return;
    if(S_ISDIR(newfile->file_stat.st_mode))
        dir_check = 1;
    if (S_ISDIR(newfile->file_stat.st_mode) && flist->dir_cnt == flist->max_dir_cnt)
        flist_sizeup(flist);
    
    if (S_ISREG(newfile->file_stat.st_mode) && flist->file_cnt == flist->max_file_cnt)
        flist_sizeup(flist);

    if (opt == 0)               //그냥 백업이므로 연결리스트필요없음.
    {
        if (dir_check)
        {
            int idx = flist->dir_cnt;
            flist->dir_array[idx++] = newfile;
            flist->dir_cnt = idx;
        }
        else
        {
            int idx = flist->file_cnt;
            flist->file_array[idx++] = newfile;
            flist->file_cnt = idx;
        }
    }
    else                        // path_name 일치하면 file_name을 strrchr 로 뒷값을 가져와서
    {
        if (dir_check)          // 디렉토리면 그냥 연결.
        {
            int idx = flist->dir_cnt;
            flist->dir_array[idx++] = newfile;
            flist->dir_cnt = idx;
        }
        else
        {
            int same = 0;
            for (int i = 0 ; i < flist->file_cnt ; i++)
            {
                if (strcmp(flist->file_array[i]->inverse_path, newfile->inverse_path) == 0)
                {
                    same = 1; //같은게 존재한다는건 노드가 있다는 뜻.
                    flist->file_rear_table[i]->next = newfile;          // 붙이기 전에 끝에 존재하던 노드에 연결
                    flist->file_rear_table[i] = newfile;                // rear를 newfile로 재설정
                    flist->file_cnt_table[i]++;                         // 추가된 리스트의 개수 추가
                }
            }
            if (same == 0)
            {
                flist->file_array[flist->file_cnt] = newfile;
                flist->file_rear_table[flist->file_cnt] = newfile;            // 시작점이니까 rear에 등록.
                flist->file_cnt_table[flist->file_cnt] = 1;                   // 추가된 리스트의 개수 추가. (1로 갱신)
                flist->file_cnt++;                                            // 파일 총 개수 증가.
            }
        }
    }
}
/**
 *  원본을 삭제하는 경우는 없음 무조건 백업파일을 삭제하는 함수임.
*/
void delete (Flist* flist, char* del_path, int f_opt)
{

}

void flist_sizeup (Flist* flst)
{
    if (flst->dir_cnt >= flst->max_dir_cnt)
    {
        flst->dir_array = (Filenode**)realloc(flst->dir_array, sizeof(sizeof(Filenode*)*flst->max_dir_cnt*2));
        for (int i = flst->max_dir_cnt ; i < flst->max_dir_cnt*2 ; i++)
            flst->dir_array[i] = NULL;
        flst->max_dir_cnt *= 2;
    }

    if (flst->file_cnt >= flst->max_file_cnt)
    {
        flst->file_array = (Filenode**)realloc(flst->file_array, sizeof(sizeof(Filenode*)*flst->max_file_cnt*2));
        flst->file_rear_table = (Filenode**)realloc(flst->file_rear_table, sizeof(sizeof(Filenode*)*flst->max_file_cnt*2));
        flst->file_cnt_table = (int*)realloc(flst->file_cnt_table, sizeof(sizeof(int)*flst->max_file_cnt*2));
        
        for (int i = flst->max_file_cnt ; i < flst->max_file_cnt*2 ; i++)
        {
            flst->file_array[i] = NULL;
            flst->file_rear_table[i] = NULL;
            flst->file_cnt_table[i] = 0;
        }
        flst->max_file_cnt *= 2;
    }
}

void print_node (Filenode* node)
{
    printf("path name is %s\n",node->path_name);
    printf("file name is %s\n",node->file_name);
    printf("actual path is %s\n",node->actual_path);
    printf("inverse path is %s\n",node->inverse_path);
    printf("hash is %s\n",node->hash);

    if (S_ISDIR(node->file_stat.st_mode))
        printf("%s is directory file\n", node->path_name);
    else
        printf("%s is Regular file\n", node->path_name);
    printf("\n\n");
}











int cmd_add(char* backup_path, char* file_name)
{
    if (access(BACKUP_PATH, F_OK) != 0) // 파일 존재하지 않을 경우 디렉토리 생성
    {
        if (mkdir(BACKUP_PATH, 0777) < 0)
        {
            fprintf(stderr, "Make Directory Error\n");
            return -1;
        }
        sleep(0.5);
    }
    add_backup(backup_path, file_name);
}


// 기본옵션 add
int add_backup(char* backup_path, char* file_name)
{
    struct stat statbuf;
    struct dirent** file_dir;
    time_t seconds = time(NULL);
    struct tm* now = localtime(&seconds);   // 시간 생성


    char file_suffix[TIME_TYPE];            //_230227172231 (백업시간) 접미어 생성.
    sprintf(file_suffix,"_%d%02d%02d%02d%02d%02d", 
            now->tm_year-100, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

    if (file_name[0] != '/');               //상대경로인 경우 절대경로화
        file_name = realpath(file_name,NULL);
    
    if (stat(file_name, &statbuf) < 0)
    {
        printf("%s can't be backuped\n", file_name);
        return -1;
    }

    if (S_ISDIR(statbuf.st_mode))
    {
        printf("%s is a directory file\n", file_name);
        return -1;
    }

}


/**
 * 해싱 함수 : 해당 파일 f 를 option 으로 해싱
 * return -> 해싱된 문자열.
*/
char* do_hashing(FILE *f, int opt)							//option 0 :md5, 1: sha1
{
    SHA_CTX hash_buf;
    unsigned char* md[SHA_DIGEST_LENGTH];
    int fd;
    int i;
    unsigned char buf[BUFSIZE];

    fd=fileno(f);
    opt==0 ? MD5_Init(&hash_buf) : SHA1_Init(&hash_buf);	    // 필수! 처음 해싱할 때 생성해줘야함. (삼항연산 이용해서 사용.)
    for (;;)
        {
        i=read(fd,buf,BUFSIZE);
        if (i <= 0) break;
        // 읽은 개수만큼 buf의 값을 해싱해서 c에 저장해줌. (반복적으로 호출가능)
        opt==0 ? MD5_Update(&hash_buf, buf,(unsigned long)i) : SHA1_Update(&hash_buf, buf,(unsigned long)i);		
        }
        // 받아온 해시 구조체를 -> md문자열에 바로저장.
    opt==0 ? MD5_Final(md, &hash_buf) : SHA1_Final(md,&hash_buf);								
    char* hash_str = hash_to_string(md);
    return hash_str;
}


// 해시 출력함수
char* hash_to_string(unsigned char *md)
{
    int i;
    // printf("test = %s\n",md); 						// 다음과 같이 사용불가. (16진수라서 잘못들어가는거 같음)
    char* string_buf = (char*)calloc(sizeof(char), HASH_LEN);
    char onc_code[3];
    for (i=0; i<SHA_DIGEST_LENGTH; i++)					// 부호없는 16진수로 표기.
    {
        sprintf(onc_code, "%02x", md[i]);
        strcat(string_buf,onc_code);
    }
    //printf("%s\n", string_buf);
    //printf("hash size is %ld\n", strlen(string_buf));
    return string_buf;
}
/**
 * junhyeong@DESKTOP-UPFPK8Q:~/go2$ ./hash_example diff.c 1			// 1: sha1
	83eba35f13c8f33a7bd40e6f3194bab14091a461
	hash size is 40

	junhyeong@DESKTOP-UPFPK8Q:~/go2$ ./hash_example diff.c 0		// 0 :md5, 
	29d6c16dacf3a7617f97204978cfce2c00000000
	hash size is 40
*/
