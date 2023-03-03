#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <openssl/sha.h>            /*현재 home에 설치되어있음 */
#include <openssl/md5.h>
#include <string.h>
#define MAXPATHLEN      4097
#define MAXFILELEN      256
#define MAXPROMPTLEN    1024

//아래부터 추가된 것들.
#include <time.h>
#define BACKUP_PATH     "/home/junhyeong/backup"    //디버그용으로 백업폴더는 /home/junhyeong/backup 으로 설정     -> 잘돌아가면 root 권한으로 /home/backup 으로 변경.
#define ACTUAL_PATH     "/home/"
#define TIME_TYPE       20
#define BUFSIZE	        1024*16
#define HASH_LEN        41
#define START_FLIST_IDX       40                    //일단 파일 IDX는 40개로 시작

char CWD [MAXPATHLEN];                              // 현재 위치 getcwd() 사용.


// 해시형태의 링크드리스트를 생각해보았으나 print_tree 같은 계층도를 그릴 필요 없는 경우는 그냥 단순연결리스트로 구현
typedef struct filenode {
    //초기화할 때 설정해줘야하는 값
    char path_name [MAXPATHLEN];        // 파일 경로 저장 
    char file_name [MAXFILELEN];        // 파일 이름 저장
    char actual_path [MAXPATHLEN];        // 백업파일/원래경로를 제외한 실질적 경로.
    char inverse_path [MAXPATHLEN];     // original파일이면 -> 백업파일 경로 저장 (ex) /home/junhyeong/diff.c -> 백업경로/diff.c
    char hash[HASH_LEN];                // <- 해시값 저장 (해시길이40:). (저장할 때 해시값저장.)
    struct stat file_stat;               // stat 저장 (바이트 크기 등에 사용.) -> st_size
    
    //나중에 설정할 값들.
    char back_up_time [TIME_TYPE];      // 백업 타이밍 설정
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


#define PRINT_ERR(_MSG) \
{fprintf(stderr, "%S Error !\n", _MSG);}

int add_backup(char* backup_path, char* file_name);
int cmd_add();

//해싱함수 : opt:0->md5, opt:1->SHA1
char* do_hashing(FILE *f, int opt);							//option 0 :md5, 1: sha1
char* hash_to_string(unsigned char *md);

// 구조체 초기화 함수들.
Filenode* new_filenode ();                                // 기본 초기화
Filenode* new_filenodes (char* filename, int opt, int f_opt);        // 파일 초기화, option 0: original, 1: backup
Flist* new_flist ();
void print_node (Filenode* node);
void append (Flist* flist, char* file_name, int opt, int f_opt);                  // 파일 array 대해 추가. option 0: orignal, 1: Backup
void delete (Flist* flist, char* del_path, int f_opt);
void flist_sizeup (Flist* flst);                                //Flist 인덱스 사이즈업.

int main(void)
{
    // 잘되는거 확인완료.
	Filenode* newfile = new_filenodes("diff.c_230227172302", 1,0);
    Flist* newflist = new_flist();
    append(newflist,"diff.c_230227172302",1,0);
    append(newflist,"diff.c_230227172319",1,0);
    append(newflist,"diff.c_230227172320",1,0);    
    append(newflist,"a.c_230227172320",1,0);    
    print_node(newfile);
	exit(0);
}



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
        sprintf(newfile->inverse_path,"%s%s", ACTUAL_PATH, newfile->file_name);
    else
        sprintf(newfile->inverse_path,"%s/%s", BACKUP_PATH, newfile->file_name);

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
                if (strcmp(flist->file_array[i]->file_name, newfile->file_name) == 0)
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
}











int cmd_add(char* backup_path, char* file_name)
{
    if (access(BACKUP_PATH, F_OK) != 0) // 파일 존재하지 않을 경우 디렉토리 생성
    {
        if (mkdir(BACKUP_PATH, 777) < 0)
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
