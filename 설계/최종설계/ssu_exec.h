#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <openssl/macros.h>
#include <openssl/sha.h>            /*현재 home에 설치되어있음 */
#include <openssl/md5.h>
#include <openssl/evp.h>

#include <string.h>
#include <errno.h>
#include <time.h>


#define MAXPATHLEN          4097                  //디버그용 (잘돌아가는거 확인)
#define MAXFILELEN          256
#define MAXPROMPTLEN        1024

//#define BACKUP_PATH         "/home/junhyeong/backup"  // 아래 BACKUP_PATH 설정.
//#define BACKUP_PATH         "/home/backup"            //<- 되는거 확인
//#define ACTUAL_PATH         "/home/junhyeong"         // 아래 ACTUAL_PATH 전역변수 사용.
//아래부터 추가된 것들.
#define TIME_TYPE           20
#define BUFSIZE	            1024*16
#define HASH_LEN            41
#define START_FLIST_IDX     40                    //일단 파일 IDX는 40개로 시작
#define MAX_FILE_SIZE       100000000
#define DIRECTRY_ACCESS     0777
#define LEGTH_ERR(_STR, _RETURN_TYPE)     if (strlen(_STR) >= MAXPATHLEN)\
{\
    printf("%s string length is %ld, It is over max limit length(%d)\n", _STR,strlen(_STR) ,MAXPATHLEN);\
    return _RETURN_TYPE;\
}

char ACTUAL_PATH [MAXPATHLEN]; // 현재 위치 getcwd() 사용.
char BACKUP_PATH [MAXPATHLEN]; // /home/사용자이름

/**
 * 2023-3-4 :구현
 *  - 해시 비교함수                                         (완료)
 *  - 파일 경로 A->B 파일 복사.                             (완료)
 *  - 디렉토리 전부탐사         -> flist                    (완료)
 *  - 파일명 같은 파일 경로찾아서 연결리스트 구현. ->rlist    (필요X)
 *  - ls,vi 제작.
 * 
 *  - add 할 때 필요한 original 경로 와 backup 파일 해시값 비교 함수 제작   (완료)
 *  - add 옵션에 따른 비교 선택.                            (완료)
 * 
 * 
 ** 2023-3-5 구현
 *  - recover
 *      (+ 같은 타임테이블 비교.)
 *      (+ md5 : 32 비트 ,SHA-1 40 비트여서 비트조정을 해줌. (new_filenodes() 비트조정.))
 *  - remove                    : #include <stdio.h> , int remove( const char *path );
 *  - ls/vim 제작
 *  - 해당 바이너리에 setuid 비트를 설정해서 실행하면 가능할 것 같습니다. (참고)
 *  
 *  ** file_rear_table 같은경우는 이중할당을 하지않음, 다응부터는 구조체에 명시해놓을것
 *      -> free에서 double free error 가 발생하면 두 가진를 생각해볼것
 *       1. double free 
 *         1-2. 내가 이차원 포인터를 2차까지 할당해주었는가?
 *              (만약 그냥 포인터를 담는 변수라면 구조체에 명시하자)
 * 
 ** 2023-3-5 구현     
 *  - 백업 경로 재설정
 *  - 파일 file_cpy() 함수 내에 make_dir() 함수가 미리 설정되지 않아 fd2==-1 나오던 오류 에러 디버깅
 *  - filenode == NULL 체크 안해준 부분 재설정.
 *  - 파일 경로 제한 설정.
 * 
 * 
 * 
 ** 2023-3-6~8 구현
 *  - 각종 디버깅 : add 에서 -d 옵션 사용시 regular 파일이 오는경우 regular 파일처럼 처리
 *                 add 단일파일 _23123123(시간) 부분이 지워지지 않아 인식하지 못하던 오류 디버깅
 * 
 *                 getopt 사용해서 모듈화 성공
 *                 ssu_main -> ssu_add, ssu_remove, ssu_recover fork() 호출 후 실행 방식으로 최종 구현
 * 
 * 
 *  ++ add : 연결리스트 next 디버깅 오류 수정 상태가 별로 안좋음. (백업은 되나, 파일이 )
 *              
 *  2023-3-10~12 디버깅 및 보고서화
 *           : openssl 중 openssl/anv.h 에 포함된 init 함수들을 사용하면 더 편함
 *           : ssu_recover.c 디버깅
 *           : 보고서 작성중...
 * 
 * 
 *  2023-3-13 recover no -d 옵션 처리 디버깅
 *          - 백업파일에만 존재하는 파일에대해 동작할 때 check 함수 내부에서 d옵션을 걸러주지 못하던 오류 수정
 *          - remove 경로에만 존재하는 파일 삭제.
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


/**
 * r,f 리스트를 동시에 저장하는 구조체.
*/
typedef struct frlist{
    Rlist* rlist;
    Flist* flist;
}FRlist;


#define PRINT_ERR(_MSG) \
{fprintf(stderr, "%S Error !\n", _MSG);}

// 사용하지 않는 함수계열
int add_backup(char* backup_path, char* file_name);
int cmd_add();
/** 추가도구*/
char* replace (char* original, char* rep_before, char* rep_after, int cnt);                 // 문자열 교체함수 (중요한 점은 char* a = replace() 형태로 쓸것.)
int kmp (char* origin, char* target);                                                       // 문자열 교체함수에서 KMP 알고리즘 이용.

//해싱함수 : opt:0->md5, opt:1->SHA1
char* do_hashing(FILE *f, int opt);							//option 0 :md5, 1: sha1
char* hash_to_string(unsigned char *md);

// 해시 비교함수
int hash_compare (Filenode* a_node, Filenode* b_node);
int hash_compare_one (Filenode* a_node, char* path_name, int opt, int f_opt);

// 구조체 초기화 함수들.
Filenode* new_filenode ();                                           // 기본 초기화
Filenode* new_filenodes (char* filename, int opt, int f_opt);        // 파일 초기화, option 0: original, 1: backup
void flist_sizeup (Flist* flst);                                     // Flist 인덱스 사이즈업.
Flist* new_flist ();
Rlist* new_Rlist();
FRlist* new_FRlist(Flist* flist, Rlist* rlist);
void print_node (Filenode* node);                                    // Filenode Unit 상태 출력
void print_rlist (Rlist* rlist);                                     // rlist 모든 요소 출력
void print_flist (Flist* flist);                                     // flist 모든 요소 출력
void append (Flist* flist, char* file_name, int opt, int f_opt);     // flist 파일 array 대해 추가. option 0: orignal, 1: Backup
void rappend (Rlist* rlist, char* file_name, int opt, int f_opt);    // Rlist 에 file_name 경로 데이터 단순 연결
Filenode* rpopleft (Rlist* rlist);                                   // Rlist 큐 popleft

void free_rlist(Rlist* rlist);                                       // rlist 모든 요소 동적할당 해제
void free_flist(Flist* flist);                                       // flist 모든 요소 동적할당 해제
void free_frlist(FRlist* frlist);                                    // frlist 모든 요소 동적할당 해제

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
            int(*compar)(const struct dirent**, const struct dirent **));       // scandir, alphasort 명시
int alphasort(const struct dirent **d1, const struct dirent **d2);


// 1. add 계열함수
int ssu_add (char* file_name, int flag, int f_opt);


// 2. remove 계열 함수.
void ssu_remove (char* file_name, int a_flag);                                           // ssu_recover_default() 재탕
void ssu_remove_all();                                       // scandir 사용.
void scandir_makefile_for_remove(Flist* flist, char *parent_folder);
Flist* check_backup_file_for_remove(char* file_name, int flag_a);

// 3. recover 계열 함수.
/**
 *  -d : 기존경로가 인 디렉토리의 모든 백업파일과 서브 디렉토리 내부의 파일들 <FILENAME> 까지 재귀적으로 복구함
 *  -n : 기존경로가 <FILENAME> 과 일치하는 백업파일을 <NEWFILE> 경로로 바꾸어 복구를 진행하고
 *       <NEWFILE> 의 파일 디렉토리가 없다면 생성 후 덮어씀 
 *        , 만약, -d와 같이 쓰는 경우 <FILENAME> 디렉토리의 모든 백업파일을 <NEWNAME> 경로로 복구를 진행
 * 
 * 
 * 
 *  설계: ssu_recover_flag_d 함수에서 backup -> original path 경로로 전환시켜줄 데이터들을 Rlist 로 return
 *       -> -n 옵션은 는 ssu_recover_flag_d 백업할 리스트를 받아와서 modify_inversepath 를 통해 경로변경.
 * 
 *      마지막에 한꺼번에 데이터변경.
 * 
 *       : 중간고찰 -> d 옵션이 오면 무조건 directory 형태로 받는다.
*/
int ssu_recover (char* file_name, int flag_d, int flag_n, char* new_name, int f_opt);       // replace이용. (캡슐함수)
int modify_inversepath (Filenode* file_name, char* new_name, int flag_d);                               // file_name => new_name으로 inverse_path 변경.
FRlist* ssu_recover_default (char* file_name, int d_flag, int f_opt);                           // d_flag 설정 여부 확인
void print_time_and_byte (Filenode* node);                                                  // 같은 디렉토리에 대한 230227172413 15bytes 등 출력.
void append_samefile (Flist* flist, char* original_file_name, int f_opt);                              // origin path 에 있는 같은 이름의 파일 백업 파일 긁어오기.
int check_backup_file(char* file_name, int flag_d);                                              //상대적인 경로에 파일이 있는 경우 만들어줌
void make_file_name(char* file_name);
void scandir_makefile(char *parent_folder, char* original_path);

//★ 시작은 무조건 ACTUAL_PATH, BACKUP_PATH부터 구할 것.
void get_actualpath();
void get_backuppath();          //get_backuppath() 를 구하면 자동으로 get_actaulpath()구해줌

// 파일 사이즈 체크함수.
int file_size_check (char file_names[]);
void main_help();

#ifdef DEBUG
int main(void)
{

    ssu_remove("/home/junhyeong/test1",1);
    //int check = check_backup_file("/home/junhyeong/ses/go.cpp");
    //ssu_recover("/home/junhyeong/go2", 1, 1, "good", 0);
    //get_actualpath();
    //ssu_recover("/home/junhyeong/test",1,1, "good",1);
    //ssu_remove("ssu_add.c", 0);   // 백업 부분 삭제함수
    //ssu_remove_all();             //전체 삭제함수
	exit(0);
}
#endif
void main_help()
{
    printf("Usage:\n"
            "  > add [FILENAME] [OPTION]\n"
            "    -d : add directory recursive\n"
            "  > recover [FILENAME] [OPTION]\n"
            "    -d : recover directory recursive\n"
            "    -n [NEWNAME] : recover file with new name\n"
            "  > remove [FILENAME] [OPTION]\n"
            "    -c : remove all sub directory and files from backup directory\n"
            "    -a [NEWNAME] : remove directory reculsive from backup directory  \n"
            "  > ls\n"
            "  > vi\n"
            "  > vim\n"
            "  > help\n"
            "  > exit\n");
}


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



void get_backuppath()
{
    if (strlen(ACTUAL_PATH) == 0)
        get_actualpath();
    
    strcpy(BACKUP_PATH, ACTUAL_PATH);
    char* plus_ptr = BACKUP_PATH + strlen(BACKUP_PATH);

    *plus_ptr++ = '/';
    
    strcpy(plus_ptr, "backup");
}



void make_file_name(char* file_name)
{
    int fd;
    char read_buf[BUFSIZE] = {0,};                      //BUFSIZE   =    1024*16*
    strcpy(read_buf, file_name);
    make_directory(file_name);
    fd = open (file_name, O_WRONLY | O_CREAT | O_TRUNC , S_IRWXU | S_IRWXG | S_IRWXO);

    if (fd < 0)
    {
        fprintf(stderr, "Fopen Error : %s\n", file_name);
        return;
    }

    int read_cnt;
    int write_cnt = write(fd, read_buf, read_cnt);

    close(fd);
    return;
}


/**
 *  재귀적으로 돌아가면서 백업파일 다 생성.
*/
void scandir_makefile(char *parent_folder, char* original_path)
{
    char originals[MAXFILELEN] = {0,};
    char backups[MAXFILELEN] = {0,};

    sprintf(originals, "%s/", original_path);
    sprintf(backups, "%s/", parent_folder);
    
    char* origin_tok = originals+ strlen(originals);
    char* backups_tok = backups+ strlen(backups);

    struct dirent** sub_dir;
    struct stat statbuf;
    int file_cnt;

    if ((file_cnt = scandir(parent_folder, &sub_dir, NULL, alphasort)) < 0)
    {
        return;
    }

    for (int i = 0 ; i < file_cnt ; i++)
    {
        if(strcmp(sub_dir[i]->d_name, ".")==0 || strcmp(sub_dir[i]->d_name, "..")==0)
        {
            free(sub_dir[i]);
            continue;
        }    
        
        strcpy(origin_tok, sub_dir[i]->d_name);
        strcpy(backups_tok, sub_dir[i]->d_name);

        stat(backups, &statbuf);

        if (S_ISDIR(statbuf.st_mode))
        {
            scandir_makefile(backups, originals);
        }

        if(S_ISREG(statbuf.st_mode))
        {
            char* time_tok = strrchr(originals, '_');
            if (time_tok != NULL)
                *time_tok = '\0';
            make_directory(originals);
            make_file_name(originals);
        }
        
        free(sub_dir[i]);
    }
    free(sub_dir);
}


/**
 * : 체크해보고 원래 경로에는 없지만 백업 파일에 있는 경우 원래경로에 파일 추가. 
 * 
*/
int check_backup_file(char* file_name, int flag_d)
{
    // 상대적인 백업폴더경로.
    // 파일이름.
    if (strlen(BACKUP_PATH) == 0)
        get_backuppath();
 
    char original_path [MAXPATHLEN*2] = {0,};                       
    char backup_path [MAXPATHLEN] = {0, };
    char only_file [MAXPATHLEN] = {0, };
    char parent_folder [MAXPATHLEN] = {0, };
    char pwd[MAXPATHLEN] = {0,};
    strcpy(original_path, file_name);
    getcwd(pwd, MAXPATHLEN);

    
    if(original_path[0] != '/')
        sprintf(original_path, "%s/%s", pwd, file_name);
    
    if(strstr(original_path, ACTUAL_PATH) == NULL)
    {
        return 0;
    }
    else
    {
        char* back_token = original_path + strlen(ACTUAL_PATH);
        sprintf(backup_path,"%s%s", BACKUP_PATH, back_token);
        
        strcpy(parent_folder, backup_path);
        char* file_name_ptr = strrchr(parent_folder, '/');
        if(file_name_ptr != NULL)
            strcpy(only_file, file_name_ptr+1);
        *file_name_ptr = '\0';
    }
    
    LEGTH_ERR(original_path, 0);                                       //03.10 original path 파일 길이 제한

    int checks = 0;
    int directory = 0;
    int regular = 0;


    struct dirent** sub_dir;
    struct stat statbuf;
    int file_cnt;
    char append_file [MAXPATHLEN*2] = {0,};
    if ((file_cnt = scandir(parent_folder, &sub_dir, NULL, alphasort)) < 0)
    {
        return 0;
    }
    for (int i = 0 ; i < file_cnt ; i++)
    {
        if (strstr(sub_dir[i]->d_name, only_file) != NULL)
        {
            checks = 1;
            
            sprintf(append_file, "%s/%s", parent_folder, sub_dir[i]->d_name);
            LEGTH_ERR(append_file, 0);                                              //03.10 append_file 파일 길이 제한
            stat(append_file, &statbuf);

            
            if (S_ISDIR(statbuf.st_mode))
            {
                if (!flag_d)
                {
                    printf("\'%s\' is a directory file\n", append_file);
                    free(sub_dir[i]);
                    continue;
                }
                directory=1;
                make_directory(original_path);
                mkdir(original_path, DIRECTRY_ACCESS);
            }

            if(S_ISREG(statbuf.st_mode))
            {
                regular=1;
                make_directory(original_path);
                make_file_name(original_path);
            }
        }
        free(sub_dir[i]);
    }
    free(sub_dir);


    if (directory)  //재귀적으로 다 살려내기.
    {
        scandir_makefile(backup_path, original_path);
        //****
    }
    return checks;
}


void get_actualpath()
{
    char tmp_path [MAXPATHLEN];
    strcpy(tmp_path, getcwd(NULL, MAXPATHLEN));
    char* tmp_ptr = tmp_path + strlen("/home/");
    for (int i = 0 ; i < MAXPATHLEN-strlen("/home")-1 ; i++)
    {
        if (*tmp_ptr == '/')
        {
            *tmp_ptr = '\0';
            break;
        }
        tmp_ptr++;
    }

    strcpy(ACTUAL_PATH, tmp_path);
}



/**
 *  : backup 전 디렉토리 삭제;
*/
void ssu_remove_all()
{
    if (strlen(BACKUP_PATH) == 0)
        get_backuppath();
    Flist* bs = backup_search (BACKUP_PATH, 0, 1);
    int sub_total = 0;
    int del_cnt = 0;
    if (bs->file_cnt == 0)                                  //백업 파일 0개이면 프롬포트 출력
    {  
        printf("no file(s) in the backup\n");
        free_flist(bs);
        return;
    }
    printf("backup directory cleared(%d regular files and %d subdirectories totally)\n",
                bs->file_cnt+sub_total, bs->dir_cnt);
    while(bs->dir_cnt != 1 && del_cnt < 100)                             //폴더가비워질 때 까지 재귀 삭제 : (하지만 100번 이상 폴더를 탐색하는 경우에는 break;)
    {

        for (int i = 0 ; i < bs->file_cnt ; i++)
        {
            sub_total += bs->file_cnt_table[i]-1;
            if (bs->file_cnt_table[i] == 1)
            {
                //파일 삭제.
                remove(bs->file_array[i]->path_name);
            }
            else
            {
                Filenode *node = bs->file_array[i];
                
                while(node != NULL)
                {
                    node = node->next;
                    //파일 삭제
                    remove(node->path_name);
                }
            }
        }

        for (int i = 0 ; i < bs->dir_cnt ; i++)
        {
            if(strcmp(bs->dir_array[i]->path_name, BACKUP_PATH) == 0)
                continue;
            // 딕셔너리 삭제
            if (rmdir(bs->dir_array[i]->path_name) < 0)
            {
                printf("%s can't be erased \n", bs->dir_array[i]->path_name);
                printf("err string is %s\n", strerror(errno));
            }
        }
        free_flist(bs);
        bs = backup_search (BACKUP_PATH, 0, 1);
        del_cnt++;
        printf("directory delete process : %d\n", del_cnt+1);
    }  
    free_flist(bs);
}


void ssu_remove (char* file_name, int a_flag)
{
    Filenode* newfile = new_filenodes(file_name, 0,0);
    int only_backup_path_flag = 0;
    Flist* only_backup_path;
    if (newfile == NULL)
    {
        only_backup_path = check_backup_file_for_remove(file_name, a_flag);
        if (only_backup_path == NULL)
        {
            printf("%s is not existed or can't be accessed\n", file_name);
            return;
        }
        else
        {
            only_backup_path_flag = 1;
        }
    }
    FRlist* remove_file;
    Flist* flist;
    if (a_flag)
    {
        if (only_backup_path_flag)
        {
            flist = only_backup_path;
        }
        else
        {
            remove_file = ssu_recover_default(newfile->path_name, 1, 0);
            flist = remove_file->flist;
        }
        
        
        for (int i = 0 ; i < flist->file_cnt ; i++)
        {
            if (flist->file_cnt_table[i] != 1)
            {
                Filenode* delnode = flist->file_array[i];
                while(delnode != NULL)
                {
                    printf("\"%s\" backup file removed\n", delnode->path_name);
                    //delnode 삭제
                    remove(delnode->path_name);
                    delnode = delnode->next;
                }
            }
            else
            {
                printf("\"%s\" backup file removed\n", flist->file_array[i]->path_name);
                //delnode 삭제 : flist->file_array[i]->path_name
                remove(flist->file_array[i]->path_name);

            }
        }
    }
    else
    {
        if (!only_backup_path_flag)
        {
            if (S_ISDIR(newfile->file_stat.st_mode))
            {
                printf("\"%s\" is a directory file\n", newfile->path_name);
            }
            remove_file = ssu_recover_default(newfile->path_name, 0, 0);

            if(remove_file == NULL)
            {
                printf("%s Open Error\n", file_name);
                free(newfile);
                return;
            }
            flist = remove_file->flist;
        }
        else
        {
            flist = only_backup_path;
        }
        if (flist->file_cnt_table[0] == 1)
        {
            printf("\"%s\" backup file removed\n", flist->file_array[0]->path_name);
            //delnode 삭제 : flist->file_array[0]->path_name
            remove(flist->file_array[0]->path_name);
            
        }
        else
        {
            printf("backup file list of \"%s\"\n", flist->file_array[0]->inverse_path);
            printf("0. exit\n");
            Filenode* tmp_node = flist->file_array[0];

            for (int ni = 0 ; ni < flist->file_cnt_table[0] ; ni++)
            {
                printf("%d. %-30s%ldbytes\n", 
                        ni+1, tmp_node->back_up_time, tmp_node->file_stat.st_size);
                tmp_node = tmp_node->next;
            }

            printf("Choose file to recover\n");
            int getnum = 5000000;
            while (getnum < 0 || getnum > flist->file_cnt_table[0])
            {
                printf(">> ");
                scanf("%d", &getnum);
                if (getnum < 0 || getnum > flist->file_cnt_table[0])
                    printf("Please choose 0 ~ %d nums\n",flist->file_cnt_table[0]);
            }
            getnum--;
            if(getnum == -1)                //exit() 들어가는 자리
            {
                if (only_backup_path_flag)
                {
                    free_flist(flist);
                }
                else
                {
                    free(newfile);
                    free_frlist(remove_file);
                }
                return;
            }
            else
            {
                tmp_node = flist->file_array[0];
                for (int s = 0 ; s < getnum ; s++)
                    tmp_node = tmp_node->next;
                printf("\"%s\" backup file removed\n", tmp_node->path_name);
                //백업파일 삭제 추가
                remove(tmp_node->path_name);
            }
        }
    }

    if (only_backup_path_flag)
    {
        free_flist(flist);
    }
    else
    {
        free(newfile);
        free_frlist(remove_file);
    }
}




void free_rlist(Rlist* rlist)                                       // rlist 모든 요소 동적할당 해제
{
    if (rlist == NULL)
        return;
    
    Filenode* delnode;
    while(rlist->rear != NULL)
    {
        delnode = rlist->rear;
        rlist->rear = rlist->rear->next;

        if (delnode != NULL)
        {
            free(delnode);
            delnode = NULL;
        }
    }
    if (rlist != NULL)
    {
        free(rlist); 
        rlist =NULL;
    }
    
}
void free_flist(Flist* flist)                                       // flist 모든 요소 동적할당 해제 (신중하게)
{
    if (flist == NULL)
        return;

    for (int i = 0 ; i < flist->dir_cnt ; i++)
    {
        if (flist->dir_array[i] != NULL)
        {
            free(flist->dir_array[i]); 
            flist->dir_array[i] = NULL;
        }
    }
    if (flist->dir_array != NULL)
    {
        free(flist->dir_array);
        flist->dir_array = NULL;
    }

    for (int i = 0 ; i < flist->file_cnt ; i++)
    {
        Filenode* delnode = flist->file_array[i];
        while(flist->file_array[i] != NULL)
        {
            delnode = flist->file_array[i];
            flist->file_array[i] = flist->file_array[i]->next;
            free(delnode); 
            delnode = NULL;
        }
        
    }
    
    if(flist->file_rear_table != NULL)
    {
        free(flist->file_rear_table); 
        flist->file_rear_table=NULL;
    }

    if (flist->file_array != NULL)
    {
        free(flist->file_array); 
        flist->file_array = NULL;
    }

    if (flist->file_cnt_table != NULL)
    {
        free(flist->file_cnt_table); 
        flist->file_cnt_table=NULL;
    }

    free(flist);
}

void free_frlist(FRlist* frlist)                                    // frlist 모든 요소 동적할당 해제
{
    if(frlist->flist != NULL)
        free_flist(frlist->flist);
    
    if(frlist->rlist != NULL)
        free_rlist(frlist->rlist);

    if (frlist != NULL)
        free(frlist);
}

/**
 *  : ssu_recover(파일명, d_flag 사용여부, n_flag 사용여부, 새로 백업할 경로 위치, f_opt);
 *      -> 해싱 비교과정 추가되어있음
 * 
 *  
 * 
*/
int ssu_recover (char* file_name, int flag_d, int flag_n, char* new_name, int f_opt)       // replace이용. (캡슐함수)
{
    FRlist* recover_list;
    Filenode* newnode = new_filenodes(file_name, 0, f_opt);
    int flg_d = flag_d;
    int flg_n = flag_n;
    
    if(newnode == NULL)
    {
        //체킹함수 제작
        int checks = check_backup_file(file_name, flag_d);
        if (checks)
            newnode = new_filenodes(file_name, 0, f_opt);
        if (newnode == NULL)
            return 0;
    }

    if (flg_d)
    {
        if (!S_ISDIR(newnode->file_stat.st_mode))
        {
            flg_d = 0;
        }
        else
        {   
            recover_list = ssu_recover_default(newnode->path_name, flg_d, f_opt);
            Rlist* rlist = recover_list->rlist;
            Flist* flist = recover_list->flist;
            char time_default[TIME_TYPE] = {0,};
            char** prev_inverse_path;
            if (flg_n)
            {
                prev_inverse_path = (char**)malloc(sizeof(char*) * flist->file_cnt);
                for(int idx = 0 ; idx < flist->file_cnt ; idx++)
                {
                    prev_inverse_path[idx] = (char*)malloc(sizeof(char) * sizeof(flist->file_array[idx]->inverse_path));
                    strcpy(prev_inverse_path[idx], flist->file_array[idx]->inverse_path);
                    Filenode* tmp_node = flist->file_array[idx];
                    for (int i = 0 ; i < flist->file_cnt_table[idx] ; i++)
                    {
                        modify_inversepath(tmp_node, new_name, flg_d);
                        tmp_node = tmp_node->next;
                    }
                }
            }


            for (int i = 0 ; i < flist->file_cnt ; i++)
            {
                if (flist->file_cnt_table[i] == 1)
                {
                    if (!hash_compare_one(flist->file_array[i], flist->file_array[i]->inverse_path, 0, f_opt)) // 복사.
                    {
                        printf("\"%s\" backup file recover to %s\n", flist->file_array[i]->path_name, flist->file_array[i]->inverse_path);
                        //백업복사해줘야함 (해시 비교 위의 두 문자 단우위로 비교하면될듯.)
                        file_cpy(flist->file_array[i]->path_name,flist->file_array[i]->inverse_path);
                    }
                    else
                    {
                        printf("\"%s\" and \"%s\" is same file, so Don't backup\n", flist->file_array[i]->path_name, flist->file_array[i]->inverse_path);
                    }

                }
                if (flist->file_cnt_table[i] > 1)
                {
                    int time_check = 1;
                    Filenode* tmp_node;
                    tmp_node = flist->file_array[i];

                    while(tmp_node != NULL)
                    {
                        if (strcmp(time_default, tmp_node->back_up_time) == 0)
                        {
                            if (!hash_compare_one(tmp_node, tmp_node->inverse_path, 0, f_opt)) // 복사.
                            {
                                printf("\"%s\" backup file recover to %s\n", tmp_node->path_name, tmp_node->inverse_path);
                                //백업복사해줘야함 (해시 비교 위의 두 문자 단위로 비교하면될듯.)
                                file_cpy(tmp_node->path_name,tmp_node->inverse_path);
                            }
                            else
                            {
                                printf("\"%s\" and \"%s\" is same file, so Don't backup\n", tmp_node->path_name, tmp_node->inverse_path);
                            }
                            time_check = 0;
                            break;
                        }
                        tmp_node = tmp_node->next;
                    }

                    
                    if (time_check)
                    {
                        printf("backup file list of \"%s\"\n", flg_n == 1 ? prev_inverse_path[i] : flist->file_array[i]->inverse_path);
                        if (flg_n == 1)
                        {
                            free(prev_inverse_path[i]);
                        }
                        printf("0. exit\n");
                        tmp_node = flist->file_array[i];

                        for (int ni = 0 ; ni < flist->file_cnt_table[i] ; ni++)
                        {
                            printf("%d. %-30s%ldbytes\n", 
                                    ni+1, tmp_node->back_up_time, tmp_node->file_stat.st_size);
                            tmp_node = tmp_node->next;
                        }

                        printf("Choose file to recover\n");
                        int getnum = 5000000;
                        while (getnum < 0 || getnum > flist->file_cnt_table[i])
                        {
                            printf(">> ");
                            scanf("%d", &getnum);
                            if (getnum < 0 || getnum > flist->file_cnt_table[i])
                                printf("Please choose 0 ~ %d nums\n",flist->file_cnt_table[i]);
                        }
                        getnum--;
                        if(getnum == -1) //<- exit() 들어가면됨 : return 이 exit임 어차피 여기서 exit는 다시 프롬포트띄워야함
                        {
                            free(newnode);
                            free_frlist(recover_list);
                            return 1;
                        }
                        else
                        {
                            tmp_node = flist->file_array[i];
                            for (int s = 0 ; s < getnum ; s++)
                                tmp_node = tmp_node->next;
                            strcpy(time_default, tmp_node->back_up_time);
                            if (!hash_compare_one(tmp_node, tmp_node->inverse_path, 0, f_opt)) // 복사.
                            {
                                printf("\"%s\" backup file recover to %s\n", tmp_node->path_name, tmp_node->inverse_path);
                                //백업복사해줘야함 (해시 비교 위의 두 문자 단우위로 비교하면될듯.)
                                file_cpy(tmp_node->path_name, tmp_node->inverse_path);
                            }
                            else
                            {
                                printf("\"%s\" and \"%s\" is same file, so Don't backup\n", tmp_node->path_name, tmp_node->inverse_path);
                            }
                        }
                    }

                }
            }
            if(flg_n)
            {
                free(prev_inverse_path);
            }
        }
    }

    if (!flg_d)
    {
        if (S_ISDIR(newnode->file_stat.st_mode))
        {
            free(newnode);
            printf("\"%s\" is a directory\n", newnode->path_name);
            return 0;
        }

        recover_list = ssu_recover_default(newnode->path_name, flg_d, f_opt);
        Rlist* rlist = recover_list->rlist;
        Flist* flist = recover_list->flist;
        char time_default[TIME_TYPE] = {0,};
        char* prev_inverse_path;
        if (flg_n)
        {
            prev_inverse_path = (char*)malloc(sizeof(char) * sizeof(flist->file_array[0]->inverse_path));
            strcpy(prev_inverse_path, flist->file_array[0]->inverse_path);
            Filenode* tmp_node = flist->file_array[0];
            for (int i = 0 ; i < flist->file_cnt_table[0] ; i++)
            {
                modify_inversepath(tmp_node, new_name, flg_d);
                tmp_node = tmp_node->next;
            }
        }


        if (flist->file_cnt_table[0] == 1)
        {
            if (!hash_compare_one(flist->file_array[0], flist->file_array[0]->inverse_path, 0, f_opt)) // 복사.
            {
                printf("\"%s\" backup file recover to \"%s\"\n", flist->file_array[0]->path_name, flist->file_array[0]->inverse_path);
                //백업복사해줘야함 (해시 비교 위의 두 문자 단우위로 비교하면될듯.)
                file_cpy(flist->file_array[0]->path_name, flist->file_array[0]->inverse_path);
            }
            else
            {
                printf("\"%s\" and \"%s\" is same file, so Don't backup\n", flist->file_array[0]->path_name, flist->file_array[0]->inverse_path);
            }

        }
        if (flist->file_cnt_table[0] > 1)
        {
            int time_check = 1;
            Filenode* tmp_node;
            tmp_node = flist->file_array[0];

            while(tmp_node != NULL)
            {
                if (strcmp(time_default, tmp_node->back_up_time) == 0)
                {
                    if (!hash_compare_one(tmp_node, tmp_node->inverse_path, 0, f_opt)) // 복사.
                    {
                        printf("\"%s\" backup file recover to \"%s\"\n", tmp_node->path_name, tmp_node->inverse_path);
                        //백업복사해줘야함 (해시 비교 위의 두 문자 단위로 비교하면될듯.)
                        file_cpy(tmp_node->path_name, tmp_node->inverse_path);
                    }
                    else
                    {
                        printf("\"%s\" and \"%s\" is same file, so Don't backup\n", tmp_node->path_name, tmp_node->inverse_path);
                    }
                    time_check = 0;
                    break;
                }
                tmp_node = tmp_node->next;
            }

            
            if (time_check)
            {
                printf("backup file list of \"%s\"\n", flg_n == 1 ? prev_inverse_path : flist->file_array[0]->inverse_path);
                if (flg_n == 1)
                {
                    free(prev_inverse_path);
                }
                printf("0. exit\n");
                tmp_node = flist->file_array[0];

                for (int ni = 0 ; ni < flist->file_cnt_table[0] ; ni++)
                {
                    printf("%d. %-30s%ldbytes\n", 
                            ni+1, tmp_node->back_up_time, tmp_node->file_stat.st_size);
                    tmp_node = tmp_node->next;
                }

                printf("Choose file to recover\n");
                int getnum = 5000000;
                while (getnum < 0 || getnum > flist->file_cnt_table[0])
                {
                    printf(">> ");
                    scanf("%d", &getnum);
                    if (getnum < 0 || getnum > flist->file_cnt_table[0])
                        printf("Please choose 0 ~ %d nums\n",flist->file_cnt_table[0]);
                }
                getnum--;
                if(getnum == -1)                //exit() 들어가는 자리
                {
                    free(newnode);
                    free_frlist(recover_list);
                    return 1;
                }
                else
                {
                    tmp_node = flist->file_array[0];
                    for (int s = 0 ; s < getnum ; s++)
                        tmp_node = tmp_node->next;
                    strcpy(time_default, tmp_node->back_up_time);
                    if (!hash_compare_one(tmp_node, tmp_node->inverse_path, 0, f_opt)) // 복사.
                    {
                        printf("\"%s\" backup file recover to \"%s\"\n", tmp_node->path_name, tmp_node->inverse_path);
                        //백업복사해줘야함 (해시 비교 위의 두 문자 단우위로 비교하면될듯.)
                        file_cpy(tmp_node->path_name, tmp_node->inverse_path);
                    }
                    else
                    {
                        printf("\"%s\" and \"%s\" is same file, so Don't backup\n", tmp_node->path_name, tmp_node->inverse_path);
                    }
                }
            }

        }
    }
    
    free(newnode);
    free_frlist(recover_list);
    return 1;
}



/**
 *  : 원래경로(file_name) 을 기준으로
 *      백업해야할 백업 폴더 (inverse_path) 로 가서
 *      recover 할 Flist, Rlist 들을 받아오는 함수.             
 * 
 *  -> 예외처리 추가 (3.6) : 백업파일에는 파일이 있는 인자면 체크 후 해당 백업파일경로에 파일 추가 후 진행. 
 * 
 *  d_flag -> 0:단일 파일경로 출력, 1: 하위 폴더 모두 긁어옴.
 * 
 */
FRlist* ssu_recover_default (char* file_name, int d_flag, int f_opt)
{
    int check = 1;
    Filenode* newfile = new_filenodes(file_name, 0, f_opt);
    Flist* flist = new_flist();
    FRlist* frlist;
    if (d_flag)
    {
        if (newfile == NULL)
        {
            return NULL;
        }

        if (S_ISDIR(newfile->file_stat.st_mode))        // -d 플래그 + file_name
        {
            check = 0;
            Rlist* rlist = original_search(newfile->path_name, f_opt, 1);
            Filenode* tmp_node = rlist->rear;
            
            while(tmp_node != NULL)
            {
                if(S_ISDIR(tmp_node->file_stat.st_mode))
                {
                    tmp_node = tmp_node->next;
                    continue;
                }
                else
                {
                    append_samefile(flist, tmp_node->path_name, f_opt);
                    tmp_node = tmp_node->next;
                }
            }
            
            free(newfile);
            frlist = new_FRlist(flist, rlist);
            return frlist;
        }
    }
    
    if(check)
    {
        // 일단 현 디렉토리 기준으로 받아야함.
        if (newfile == NULL)
            return NULL;
        
        if (S_ISDIR(newfile->file_stat.st_mode))
        {
            printf("\"%s\" is a directory file\n", newfile->path_name);
            free(newfile);
            free(flist);
            return NULL;
        }
        Rlist* rlist = new_Rlist();
        rappend(rlist, newfile->path_name, 0, f_opt);
        append_samefile(flist, newfile->path_name, f_opt);
        free(newfile);
        frlist = new_FRlist(flist, rlist);
        return frlist;
    }
}


/**
 *  : original 경로에 있는 original_file_name (상대경로시 현재경로 기준) 
 *      의 백업파일에 있는 같은 파일들을 모두 append 해줌.
 * 
*/
void append_samefile (Flist* flist, char* original_file_name, int f_opt)
{
    Filenode* filename = new_filenodes(original_file_name, 0 ,f_opt);
    if (filename == NULL)
    {
        return;
    }

    if (S_ISDIR(filename->file_stat.st_mode))
    {
        free(filename);
        return;
    }

    char inverse_path[MAXPATHLEN];
    char* token_a = strrchr(filename->inverse_path, '_');
    *token_a = '\0';
    strcpy(inverse_path, filename->inverse_path);
    
    char* last_ptr = strrchr(filename->inverse_path, '/');
    if (last_ptr != NULL)
    {
        *last_ptr = '\0';
    }

    //printf("%s\n", filename->inverse_path);
    ///추가 03.06
    struct stat tmp_stat;
    stat(filename->inverse_path, &tmp_stat);
    if (!S_ISDIR(tmp_stat.st_mode))
    {
        free(filename);
        return;
    }

    struct dirent** sub_dir;
    int file_cnt;
    if ((file_cnt = scandir(filename->inverse_path, &sub_dir, NULL, alphasort)) < 0)
    {
        printf("apple_samefile: Scan Error %s\n", filename->inverse_path);
        free(filename);
        return;
    }

    char* inverse_ptr;
    if (file_cnt > 2)
    {        
        inverse_ptr = filename->inverse_path + strlen(filename->inverse_path);
        *inverse_ptr = '/';
        inverse_ptr++;
    }
    for (int i = 0 ; i < file_cnt ; i++)
    {
        char dir_name[MAXFILELEN]; 
        strcpy(dir_name, sub_dir[i]->d_name);
        char* token_ptr = strrchr(dir_name, '_');
        if (token_ptr != NULL)
            *token_ptr = '\0';
        strcpy(inverse_ptr, dir_name);
        if (strcmp(inverse_path, filename->inverse_path) == 0)
        {
            strcpy(inverse_ptr, sub_dir[i]->d_name);
            append(flist, filename->inverse_path, 1, f_opt);
        }

        free(sub_dir[i]);
    }
    free(filename);
    free(sub_dir);
}


/**
 *  : inverse_path의 경로를 -> new_name으로 경로변경    ==> 이전의 inverse_path경로를 리턴해줌.
 * modify_inversepath()     --> 0(실패), 1(성공), 2(워닝 : 만들었지만 타입안맞는 경우)
 *  
 * flag_d -> new_path 를 0:단일파일, 1:폴더 로 인식
 * 
 * 
 * */
int modify_inversepath (Filenode* node, char* new_name, int flag_d)
{
    if (strlen(BACKUP_PATH) == 0)
        get_backuppath();
    char pwd[MAXPATHLEN] = {0,};
    char file_size_check_str[MAXPATHLEN*2] = {0,};
    getcwd(pwd, MAXPATHLEN);
    if(S_ISDIR(node->file_stat.st_mode))        //디렉토리는 그냥 넘김
    {
        return 0;
    }

    char newname [MAXPATHLEN] = {0,};
    if (new_name[0] != '/')
    {
        sprintf(file_size_check_str, "%s/%s", pwd, new_name);  
        LEGTH_ERR(file_size_check_str, 0);
        strcpy(newname, file_size_check_str);
    }
    else
        strcpy(newname, new_name);

    if (strstr(newname, ACTUAL_PATH) == NULL)
    {
        return 0;
    }

    char* inverse_path = (char*)malloc(sizeof(node->inverse_path));
    strcpy(inverse_path, node->inverse_path);
    if (flag_d)             //d 플래그 존재시 NEWNAME은 폴더로 인식.
    {
        // 03.06 디버깅 newname 하나의 폴더로 때려박는게 아니라 /새이름/ + actual_path 를 해줘야한다.
        if (strstr(newname, node->file_name) == NULL)       //혹시나 파일명으로 줬다면?
        {
            char* name_ptr = newname + strlen(newname);
            if (strstr(node->inverse_path, pwd) == NULL)   // 백업할 때 gwtcwd() 이후의 하위폴더 아닌, /home 하위폴더로 진행
            {
                char token_string[MAXPATHLEN] = {0,};
                strcpy(token_string, node->actual_path);
                char* token_time = strrchr(token_string, '_');
                *token_time = '\0';
                if (strlen(newname) + strlen(token_string) > MAXPATHLEN)
                {
                    printf("new_path_name is over MAXPATHLEN=%d\n", MAXPATHLEN);
                    return 0;
                }
                strcpy(name_ptr,token_string); 
            }
            else
            {
                char* token_string = node->inverse_path + strlen(pwd);
                // 만약 새로 만든 파일 경로가 MAXPATHLEN 넘어가면 오류
                if (strlen(newname) + strlen(token_string) > MAXPATHLEN)
                {
                    printf("new_path_name is over MAXPATHLEN=%d\n", MAXPATHLEN);
                    return 0;
                }
                strcpy(name_ptr,token_string); 
            }
        }
    }
    memset(node->inverse_path, '\0', MAXPATHLEN);
    strcpy(node->inverse_path, newname);

    char* file_type_ptr = strrchr(node->file_name,'.');
    char* new_name_ptr = strrchr(newname,'.');
    int check = 1;
    if ((file_type_ptr != NULL && new_name_ptr == NULL) || 
        (file_type_ptr == NULL && new_name_ptr != NULL) || 
        ((file_type_ptr != NULL && new_name_ptr != NULL) && strcmp(file_type_ptr, new_name_ptr) != 0))
    {
        printf("warining!, [%s] -> [%s]\n>> newname type is not matched: %s->%s\n\n",
                node->path_name, node->inverse_path ,file_type_ptr, new_name_ptr);
    }

    free(inverse_path);
    return check;
}


/**
 *  : 같은 파일 출력함수.
 *  230227172302        13bytes 처럼 출력해주는 함수.
 */
void print_time_and_byte (Filenode* node)
{
    printf("%s        %ldbytes\n",node->back_up_time, node->file_stat.st_size);
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
    int d_flag = flag;

    if (d_flag)       //해당경로로 모두 탐색.
    {
        if (S_ISREG(tmp_node->file_stat.st_mode))
        {
            d_flag = 0;
        }
        if (S_ISDIR(tmp_node->file_stat.st_mode))
        {
            Rlist* original_node = original_search(original_path, f_opt, 1);
            Flist* backup_node = backup_search(backup_path, f_opt, 1);              //03.08 얜 애초에 디렉토리라서 상관없음.

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
                                Filenode* node = backup_node->file_array[cnt];
                                while (node != NULL)
                                {
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
            free_rlist(original_node);
            free_flist(backup_node);
            free(tmp_node);
            return 0;
        }
    }
    
    if (!d_flag)
    {
        if (S_ISDIR(tmp_node->file_stat.st_mode))
        {
            printf("\"%s\" is a directory file\n", tmp_node->path_name);
            free(tmp_node);
            return 0;
        }
        // 실제로 해보니 해당 경로에 있는 값들을 다 가져와서 비교해야됨
        Rlist* original_node = original_search (original_path, f_opt, 0);
                /** 03.08 긴급 디버깅 추가*/
        char* ptr_tok = strrchr(backup_path, '_');        //파일인 경우에는 토큰을 분리시킨다.
        if (ptr_tok != NULL)
        {
            ptr_tok = strrchr(backup_path, '/');
            *ptr_tok ='\0';
        }
        Flist* backup_node = backup_search (backup_path, f_opt, 0);

        int check = 1;
        if (backup_node != NULL)
        {
            for (int cnt = 0 ; cnt < backup_node->file_cnt ; cnt++)
            {
                if (strcmp(backup_node->file_array[cnt]->file_name, original_node->header->file_name) == 0)
                {
                    Filenode* node = backup_node->file_array[cnt];
                    while(node != NULL)
                    {
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

        free_rlist(original_node);
        free_flist(backup_node);
    }
    free(tmp_node);
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
                
                /** ★긴급 디버깅!! : 백업 폴더 중첩 방지코드*/
                if (strstr(rlist->rear->path_name, BACKUP_PATH) != NULL)            // 03.05 : 긴급 디버깅 -> 백업폴더중첩 방지
                {
                    rlist->rear = rlist->rear->next;
                    continue;
                }

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
                        {
                            free(sub_dir[i]);   
                            continue;
                        }
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
            free(rootnode);
            return rlist;
        }
        else        //자기 하위 파일만 탐색하는 경우.
        {
            //디렉토리는 제외하고 REG 파일만 백업
            rappend(rlist, rootnode->path_name, 0, f_opt);
            
            char path_name [MAXPATHLEN*2] = {0,};                           //file_size_check_str 대용
            sprintf(path_name, "%s/", rootnode->path_name);                  //경로 저장.
            char *modify_ptr = path_name + strlen(path_name);                   //이름 수정할 포인터
            char* original_ptr = modify_ptr;
            int file_cnt;
            struct dirent** sub_dir;
            if ((file_cnt=scandir(rlist->rear->path_name, &sub_dir, NULL, alphasort)) < 0)
            {
                printf("open dir error :%s \n", rootnode->path_name);
                free(rootnode);
                free_rlist(rlist);
                return NULL;
            }

            for (int i = 0 ; i < file_cnt ; i++)
            {
                char sub_file_name[MAXFILELEN];
                strcpy(sub_file_name, sub_dir[i]->d_name);
                if (strcmp(sub_file_name, ".") == 0 || strcmp(sub_file_name, "..") == 0)
                {
                    free(sub_dir[i]);   
                    continue;
                }
                strcpy(modify_ptr, sub_file_name);
                LEGTH_ERR(path_name, NULL);                      //03.10 수정
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
        free(rootnode);
        return rlist;
    }

}

/**
 *  : backup_search -> backup 폴더 경로 하위 폴더 및 해당 파일 리스트 반환
 * : all 0:해당 파일/디렉토리안 파일만. 1:하위 모든파일
 */
Flist* backup_search(char* file_name, int f_opt, int all)             // 해시 체이닝 구현.
{
    
    Filenode* rootnode = new_filenodes(file_name, 1, f_opt);
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
                    free(rootnode);
                    free_flist(flist);
                    return NULL;
                }

                for(int i = 0 ; i < dir_unit_cnt ; i++)
                {
                    char sub_file_name[MAXFILELEN];
                    strcpy(sub_file_name, sub_dir[i]->d_name);
                    if (strcmp(sub_file_name, ".") == 0 || strcmp(sub_file_name, "..") == 0)
                    {
                        free(sub_dir[i]);
                        continue;
                    }
                    strcpy(modify_ptr, sub_file_name);
                    append(flist, tmp_name, 1, f_opt);       //만약에 오류나면 modify_ptr 이 char* 으로 전달되고있음을 생각해볼것
                    free(sub_dir[i]);   
                    modify_ptr = original_ptr;
                }
                free(sub_dir);
            }
            free(rootnode);
            return flist;
        }
        else   
        {
            char* dir_name = flist->dir_array[0]->path_name;
            char tmp_name[MAXPATHLEN] = {0,};
            sprintf(tmp_name, "%s/", dir_name);
            char* modify_ptr = tmp_name + strlen(tmp_name);
            char* original_ptr = modify_ptr;                //복구시키는 기능.

            if (access(dir_name, R_OK) != 0)
            {
                free(rootnode);
                free(flist);
                return NULL;
            }
            struct dirent** sub_dir;
            int dir_unit_cnt;
            if ((dir_unit_cnt=scandir(dir_name, &sub_dir, NULL, alphasort)) < 0)
            {
                printf("open dir error :%s \n", rootnode->path_name);
                free(rootnode);
                free(flist);
                return NULL;
            }

            for(int i = 0 ; i < dir_unit_cnt ; i++)
            {
                char sub_file_name[MAXFILELEN];
                strcpy(sub_file_name, sub_dir[i]->d_name);
                if (strcmp(sub_file_name, ".") == 0 || strcmp(sub_file_name, "..") == 0)
                {
                    free(sub_dir[i]); 
                    continue;
                }
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
        free(rootnode);
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
                if (mkdir(tmp_path, DIRECTRY_ACCESS) < 0)
                {
                    printf("Making directory : %s Error !\n", tmp_path);
                }
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

    make_directory(b_file);
    fd1 = open (a_file, O_RDONLY);
    fd2 = open (b_file, O_WRONLY | O_CREAT | O_TRUNC , S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd1 < 0)
    {
        fprintf(stderr, "Fopen Error : %s\n", a_file);
        return 0;
    }

    // No such file or directory error! -> 
    /**
     * "/home/junhyeong/backup/tests/ssu_add.c_23037100559" 
     * backup file recover to /home/junhyeong/go2/good/tests/ssu_add.c
    */
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
    
    make_directory(a_node->inverse_path);               //파일 제작 전 우선 디렉토리 제작.
    if (S_ISDIR(a_node->file_stat.st_mode))
    {
        if (access(a_node->inverse_path, F_OK) != 0)
        {
            if (mkdir(a_node->inverse_path, DIRECTRY_ACCESS) < 0)
            {
                printf("Make Directory Error! : %s\n", a_node->inverse_path);
            }
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

int hash_compare_one (Filenode* a_node, char* path_name, int opt, int f_opt)
{
    Filenode* b_node = new_filenodes(path_name, opt, f_opt);
    if (b_node == NULL)
    {
        return 0;
    }

    char a_hash[HASH_LEN] = {0,};  
    char b_hash[HASH_LEN] = {0,};
    strcpy(a_hash, a_node->hash); 
    strcpy(b_hash, b_node->hash);

    free(b_node);
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
    if (strlen(BACKUP_PATH) == 0)
        get_backuppath();
    char tmp_path[MAXPATHLEN] = {0,};
    Filenode* newfile = new_filenode();

    if(filename[0] != '/') // 절대경로가 아닌경우,
    {
        if (strcmp(filename,"") == 0)
        {
            strcpy(newfile->path_name, opt==1 ? BACKUP_PATH : getcwd(NULL, MAXPATHLEN));
        }
        else
        {
            sprintf(newfile->path_name,"%s/%s", opt==1 ? BACKUP_PATH : getcwd(NULL, MAXPATHLEN), filename);    
        }
    }
    else
    {
        if (strstr(filename, "/home") == NULL)                                                       // /home 을 벗어난 파일은 애초에 오류임.
        {
            printf("%s is over from /home directory\n", filename);
            free(newfile);
            return NULL;
        }

        strcpy(newfile->path_name, filename);
    }

    char file_size_check_str[MAXPATHLEN*2] = {0,};      //03.10 파일 사이즈 체크전용 문자열


    if (access(newfile->path_name, R_OK) != 0)          //없거나 접근 불가능할 때, 
    {
        if (access(newfile->path_name, F_OK) != 0)
        {
            printf("%s is not existed!\n", filename);
        }
        else
        {
            printf("%s can not access!\n", filename);
        }
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

    //home 경로인데 + opt도 같으면
    if (strcmp(filename, ACTUAL_PATH) == 0 && opt == 0)
    {
        strcpy(newfile->path_name, ACTUAL_PATH);
        char* cur_time_ptr = curr_time(); 
        strcpy(newfile->back_up_time, cur_time_ptr);
        sprintf(newfile->inverse_path, "%s", BACKUP_PATH); //일관성유지
        free(cur_time_ptr);
        return newfile;
    }

    // backup 경로인데 + opt까지 같으면
    if (strcmp(filename,BACKUP_PATH) == 0 && opt == 1)
    {
        strcpy(newfile->path_name, BACKUP_PATH);
        char* cur_time_ptr = curr_time(); 
        strcpy(newfile->back_up_time, cur_time_ptr);
        sprintf(newfile->inverse_path, "%s", ACTUAL_PATH);
        free(cur_time_ptr);
        return newfile;
    }

    //actual_path 만드는 과정
    strcpy(tmp_path, newfile->path_name);
    char* tks = tmp_path + strlen(opt==1 ? BACKUP_PATH : ACTUAL_PATH);
    strcpy(newfile->actual_path, tks);

    if (!S_ISDIR(newfile->file_stat.st_mode))                               //03.05 : file_name 은 Directory 파일은 생성하지 않도록 하였음.
    {

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
    }


    if (opt == 1)       //inverse_path 만드는 과정.
    {
        
        sprintf(file_size_check_str,"%s%s", ACTUAL_PATH, newfile->actual_path);
        LEGTH_ERR(file_size_check_str, NULL);                                   //에러체크

        strcpy(newfile->inverse_path, file_size_check_str);
        char* time_token = strrchr(newfile->inverse_path, '_');                     // inverse path에서 _230227172331 를 떼줘야함.
        if (time_token != NULL)
            memset(time_token,'\0', TIME_TYPE);
    }
    else
    {
        if (!S_ISDIR(newfile->file_stat.st_mode))
        {
            char* cur_time_ptr = curr_time();
            strcpy(newfile->back_up_time, cur_time_ptr);
            sprintf(file_size_check_str,"%s%s_%s", BACKUP_PATH, newfile->actual_path,cur_time_ptr);
            LEGTH_ERR(file_size_check_str, NULL);
            strcpy(newfile->inverse_path, file_size_check_str);
            free(cur_time_ptr);
        }
        else
        {
            
            sprintf(file_size_check_str,"%s%s", BACKUP_PATH, newfile->actual_path);
            LEGTH_ERR(file_size_check_str, NULL);
            strcpy(newfile->inverse_path, file_size_check_str);
        }
        
    }
   
    //파일용량 (MAX_FILE_SIZE : 100000000) 로 제한 : 안 해주면 4기가짜리 파일 읽는데 시간 엄청걸림 (해싱+복사 하는 과정에 시간 너무써서 버리는걸로..)
    if (newfile->file_stat.st_size > MAX_FILE_SIZE)
    {
        printf("%s file size is %ld, pass\n", newfile->path_name, newfile->file_stat.st_size);
        free(newfile);
        return NULL;
    }    

    if (S_ISDIR(newfile->file_stat.st_mode))                                //해당 파일이 디렉토리면 해시를 구하지않음.
        return newfile;

    FILE *fp_hash = fopen(newfile->path_name, "r");
    char* hash_ptr = do_hashing(fp_hash, f_opt);
    if (hash_ptr == NULL)
    {
        fprintf(stderr, "hash_err : %s\n", newfile->path_name);
    }

    strcpy(newfile->hash, hash_ptr);
    if (f_opt == 0)
    {
        char* hash_ptr_s = newfile->hash + 32;
        memset(hash_ptr_s, '\0', HASH_LEN - 32);
    }
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
 * 구조체 세트 반환
*/
FRlist* new_FRlist(Flist* flist, Rlist* rlist)
{
    FRlist* frlist = (FRlist*)malloc(sizeof(frlist));
    frlist->flist = flist;
    frlist->rlist = rlist;
    return frlist;
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

void flist_sizeup (Flist* flst)
{
    if (flst->dir_cnt >= flst->max_dir_cnt)
    {
        flst->dir_array = (Filenode**)realloc(flst->dir_array, sizeof(Filenode*) * flst->max_dir_cnt*2);
        for (int i = flst->max_dir_cnt ; i < flst->max_dir_cnt*2 ; i++)
            flst->dir_array[i] = NULL;
        flst->max_dir_cnt *= 2;
    }

    if (flst->file_cnt >= flst->max_file_cnt)
    {
        
        flst->file_array = (Filenode**)realloc(flst->file_array, sizeof(Filenode*) * flst->max_file_cnt*2);
        flst->file_rear_table = (Filenode**)realloc(flst->file_rear_table, sizeof(Filenode*) * flst->max_file_cnt*2);
        flst->file_cnt_table = (int*)realloc(flst->file_cnt_table, sizeof(int*) * flst->max_file_cnt*2);

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
        if (mkdir(BACKUP_PATH, DIRECTRY_ACCESS) < 0)
        {
            fprintf(stderr, "Make Directory Error\n");
            return -1;
        }
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

char* do_hashing(FILE *f, int opt)							//option 0 :md5, 1: sha1
{
    SHA_CTX sha1_buf;
    MD5_CTX md5_buf;
    unsigned char md[SHA_DIGEST_LENGTH];
    int fd;
    int i;
    unsigned char buf[BUFSIZE];

    fd=fileno(f);
    opt==0 ? MD5_Init(&md5_buf) : SHA1_Init(&sha1_buf);	    // 필수! 처음 해싱할 때 생성해줘야함. (삼항연산 이용해서 사용.)
    for (;;)
        {
        i=read(fd,buf,BUFSIZE);
        if (i <= 0) break;
        // 읽은 개수만큼 buf의 값을 해싱해서 c에 저장해줌. (반복적으로 호출가능)
        opt==0 ? MD5_Update(&md5_buf, buf,(unsigned long)i) : SHA1_Update(&sha1_buf, buf,(unsigned long)i);		
        }
        // 받아온 해시 구조체를 -> md문자열에 바로저장.
    opt==0 ? MD5_Final(&(md[0]), &md5_buf) : SHA1_Final(&(md[0]),&sha1_buf);								
    char* hash_str = hash_to_string(md);
    return hash_str;
}
*/

/** https://www.openssl.org/docs/man1.1.1/man3/EVP_MD_CTX_new.html :참고*/
char* do_hashing (FILE *f, int opt)							//option 0 :md5, 1: sha1
{
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned char buf[BUFSIZE];
    unsigned int md_len;
    size_t read_cnt;
    int fd=fileno(f);

    const EVP_MD *hash_struct = EVP_get_digestbyname( opt==0 ? "md5" : "sha1");
    if (hash_struct == NULL)
    {
        printf("Making Hashing Error!\n");
        return NULL;
    }

    EVP_MD_CTX *mdctx  = EVP_MD_CTX_new();
    if (hash_struct == NULL)
    {
        printf("Making Hashing Error!\n");
        return NULL;
    }

    if (EVP_DigestInit_ex(mdctx , hash_struct, NULL) == 0) {
        printf("Making Hashing Error!\n");
        return NULL;
    }
    for (;;)
    {
        read_cnt = read(fd,buf,BUFSIZE);
        if (read_cnt <= 0) break;
        // 읽은 개수만큼 buf의 값을 해싱해서 c에 저장해줌. (반복적으로 호출가능)
        EVP_DigestUpdate(mdctx, buf, read_cnt);	
    }

    EVP_DigestFinal_ex(mdctx, md, &md_len);
    EVP_MD_CTX_free(mdctx);

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
        int* target_array = (int*)malloc(tar_len* sizeof(int));
        memset(target_array,0, tar_len);
        int tar_j = 0;
        target_array[0] = 0;
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
                //printf("Same String idx is %d\n", i-tar_len+1);
                return  i-tar_len+1;
            }
        }

        free(target_array);
        return -1;
    }

}
/** 
 *  문자열 교체 함수: 마지막에 cnt에 몇 개까지 교체할 것인지 선택할 수 있음.
 *  ★ 사용법 :  strcpy(path, replace(path, "A", "BBB"));
 *        or    char* ptr = replace(path, "A", "BBB");
 *
 *  original 문자열의 rep_before 을 rep_after를 cnt만큼 바꿔줌
 *  cnt == 0 이면 모든 문자열을 교체
 *
 * original 의 rep_before 을 rep_after로 교체 
 *  kmp로 일치 문자열 찾기
 *  ptr = 일치문자열 + 찾을 문자열길이 해서 뒷부분 복사
 *  원본 -> 바꿀 문자열 바꾸기
 *  붙여넣기
 *  주의할 점은 : 할당된 문자열이 오기 때문에 사용 후 free를 해줄 것
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




Flist* check_backup_file_for_remove(char* file_name, int flag_a)
{
    // 상대적인 백업폴더경로.
    // 파일이름.
    if (strlen(BACKUP_PATH) == 0)
        get_backuppath();
    
    Flist* flist = new_flist();
    char original_path [MAXPATHLEN*2] = {0,};                       
    char backup_path [MAXPATHLEN] = {0, };
    char only_file [MAXPATHLEN] = {0, };
    char parent_folder [MAXPATHLEN] = {0, };
    char pwd[MAXPATHLEN] = {0,};
    strcpy(original_path, file_name);
    getcwd(pwd, MAXPATHLEN);

    
    if(original_path[0] != '/')
        sprintf(original_path, "%s/%s", pwd, file_name);
    
    if(strstr(original_path, ACTUAL_PATH) == NULL)
    {
        return NULL;
    }
    else
    {
        char* back_token = original_path + strlen(ACTUAL_PATH);
        sprintf(backup_path,"%s%s", BACKUP_PATH, back_token);
        
        strcpy(parent_folder, backup_path);
        char* file_name_ptr = strrchr(parent_folder, '/');
        if(file_name_ptr != NULL)
            strcpy(only_file, file_name_ptr+1);
        *file_name_ptr = '\0';
    }
    
    LEGTH_ERR(original_path, 0);                                       //03.10 original path 파일 길이 제한

    int checks = 0;
    int directory = 0;
    int regular = 0;


    struct dirent** sub_dir;
    struct stat statbuf;
    int file_cnt;
    char append_file [MAXPATHLEN*2] = {0,};
    if ((file_cnt = scandir(parent_folder, &sub_dir, NULL, alphasort)) < 0)
    {
        return NULL;
    }
    for (int i = 0 ; i < file_cnt ; i++)
    {
        if (strstr(sub_dir[i]->d_name, only_file) != NULL)
        {
            if (strcmp(sub_dir[i]->d_name, ".") == 0 || strcmp(sub_dir[i]->d_name, "..")== 0)
            {
                free(sub_dir[i]);
                continue;
            }
            checks = 1;
            
            sprintf(append_file, "%s/%s", parent_folder, sub_dir[i]->d_name);
            LEGTH_ERR(append_file, 0);                                              //03.10 append_file 파일 길이 제한
            stat(append_file, &statbuf);

            
            if (S_ISDIR(statbuf.st_mode))
            {
                if (!flag_a)
                {
                    printf("\'%s\' is a directory file\n", append_file);
                    free(sub_dir[i]);
                    continue;
                }
                directory=1;
                append(flist, append_file, 1, 0);
            }

            if(S_ISREG(statbuf.st_mode))
            {
                regular=1;
                append(flist, append_file, 1, 0);
            }
        }
        free(sub_dir[i]);
    }
    free(sub_dir);


    if (directory)  //재귀적으로 파일 삭제.
    {
        scandir_makefile_for_remove(flist, backup_path);
        //****
    }
    return flist;
}



void scandir_makefile_for_remove(Flist* flist, char *parent_folder)
{
    char backups[MAXFILELEN] = {0,};
    sprintf(backups, "%s/", parent_folder);
    
    char* backups_tok = backups+ strlen(backups);

    struct dirent** sub_dir;
    struct stat statbuf;
    int file_cnt;

    if ((file_cnt = scandir(parent_folder, &sub_dir, NULL, alphasort)) < 0)
    {
        return;
    }

    for (int i = 0 ; i < file_cnt ; i++)
    {
        if(strcmp(sub_dir[i]->d_name, ".")==0 || strcmp(sub_dir[i]->d_name, "..")==0)
        {
            free(sub_dir[i]);
            continue;
        }    
        
        strcpy(backups_tok, sub_dir[i]->d_name);

        stat(backups, &statbuf);

        if (S_ISDIR(statbuf.st_mode))
        {
            append(flist, backups, 1, 0);
            scandir_makefile_for_remove(flist, backups);
        }

        if(S_ISREG(statbuf.st_mode))
        {
            append(flist, backups, 1, 0);
        }
        
        free(sub_dir[i]);
    }
    free(sub_dir);
}
/**
 * junhyeong@DESKTOP-UPFPK8Q:~/go2$ ./hash_example diff.c 1			// 1: sha1
	83eba35f13c8f33a7bd40e6f3194bab14091a461
	hash size is 40

	junhyeong@DESKTOP-UPFPK8Q:~/go2$ ./hash_example diff.c 0		// 0 :md5, 
	29d6c16dacf3a7617f97204978cfce2c00000000
	hash size is 40
*/



