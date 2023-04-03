#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
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
#define MAX_FILE_SIZE       1000000000
#define DIRECTRY_ACCESS     0777
char BACKUP_USERNAME[MAXPATHLEN];


// 파일 사이즈 체크함수.
int file_size_check (char file_names[]);
//★ 시작은 무조건 ACTUAL_PATH, BACKUP_PATH부터 구할 것.
void get_actualpath();
// 입력된 파일 기준 actual_path
void get_actualpath2(char* orignal_path);       
//get_backuppath() 를 구하면 자동으로 get_actaulpath()구해줌
void get_backuppath();          

//동일 유저명이 존재하는지 확인
int exist_username();
// /home 디렉토리를 가진 아무 User나 가져오는 함수.
char* find_anyuser();

char ACTUAL_PATH [MAXPATHLEN]; // 현재 위치 getcwd() 사용.
char BACKUP_PATH [MAXPATHLEN]; // /home/사용자이름


int file_size_check (char file_names[])
{
    char file_name [MAXPATHLEN*2] = {0,};    
    strcpy(file_name, file_names);
    
    if (file_name[0] == '~')
    {
        char* find_delimeter = strrchr(file_name, '~');
        if (strlen(BACKUP_PATH) == 0)
            get_backuppath();

        char tmp_pwd[MAXPATHLEN] = {0,};
        strcpy(tmp_pwd, BACKUP_PATH);
        // 만약 ../~ 이런 형태로 단독으로 온다면.. --> 03.29 ~경로는 새로만든 id를 기준으로 하고 .는 현재디렉토리 기준으로 하도록 제작
        char* del2 = strrchr(tmp_pwd, '/');
        *del2 = '\0';
        if (strcmp(find_delimeter, "~") != 0) 
        {
            strcat(tmp_pwd, find_delimeter+1);
        }
        strcpy(file_name,tmp_pwd);
    }

    if (file_name[0] != '/' || strstr(file_name,"/..") != NULL)
        realpath(file_name, file_name);
    get_actualpath();                 //actual_path 재설정
    if (strstr(file_name, ACTUAL_PATH) == NULL)      // 아예 /home 으로 들어온 경우 예외처리
    {
        //printf("%s is over from /home directory\n", file_name);
        return 0;
    }

    if (strlen(file_name) >= MAXPATHLEN)
    {
        printf("%s size is %ld. It is over MAX file legnth(%d)\n", file_name,strlen(file_name), MAXPATHLEN-1);
        return 0;
    }


    char *onlyfile = strrchr(file_name, '/');
    onlyfile++;
    if (strlen(onlyfile) >= MAXFILELEN)
    {
        printf("%s size is %ld. It is over MAX file legnth(%d)\n", onlyfile,strlen(file_name), MAXFILELEN-1);
        return 0;
    }

    strcpy(file_names, file_name);
    return 1;
}



void get_backuppath()
{
    // $HOME 을 기준으로 backup
    char* my_home = getenv("HOME");
    if (strcmp(my_home, "/root") == 0)
    {
        char tmp_path [MAXPATHLEN-10];
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
        sprintf(BACKUP_PATH, "%s/backup", tmp_path);
    }
    else
        sprintf(BACKUP_PATH, "%s/backup", my_home);
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


void get_actualpath2(char* orignal_path)
{

    if (strlen(BACKUP_PATH) == 0)
        get_backuppath();
    
    char or_pth [MAXPATHLEN] = {0,};                       
    strcpy(or_pth, orignal_path);

    if(orignal_path[0] != '/')
        realpath(orignal_path, or_pth);
    

    if(strstr(or_pth, "/home/") == NULL)
        return;
    char* home_tok = or_pth + strlen("/home/");
    char* only_home_tok = strrchr(home_tok, '/');
    if (only_home_tok == NULL)                      // /home/사용자아이디 형태
    {
        strcpy(ACTUAL_PATH, or_pth);
        //printf("%s\n", or_pth);
        return;
    }
    else
    {        
        char* max_num = home_tok + strlen(home_tok);
        for (char* tk = or_pth + strlen("/home/") ; tk < max_num ; tk++)
        {
            if (*tk == '/')
            {
                *tk = '\0';
                //printf("%s\n", or_pth);
                strcpy(ACTUAL_PATH, or_pth);
                return;
            }
        }
    }
}

/**
 * username이 존재하는지 확인하는 함수
 * 
 * 성공 : 1 (동일 유저 명 존재 X)
 * 실패 : -1 (open error)
 * 실패 : 0 (동일 유저 명 존재 O)
*/
int exist_username()
{
    char username [MAXPATHLEN] = {0,};
    strcpy(username, BACKUP_USERNAME);
    FILE* file = fopen("/etc/passwd", "r");
    char buf[1024];

    if (file == NULL) {
        return -1;
    }

    while (fgets(buf, 1024, file)) {
        char* name = strtok(buf, ":");
        if (strcmp(name, username) == 0) {
            //printf("%s user exists\n", username);
            fclose(file);
            return 0;
        }
    }

    return 1;
    fclose(file);

}


/**
 * 만약에, 아이디를 생성할 수 없다면 아무 아이디를 백업경로로 설정한다.
*/
char* find_anyuser()
{
    char line[1024];
    FILE* fp = fopen("/etc/passwd", "r");

    if (fp == NULL) {
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        char* usrname = strtok(line, ":");      //첫번쨰 토큰은 이름
        for (int i = 0 ; i < 4 ; i++)
            strtok(NULL, ":"); 
        char* home = strtok(NULL, ":");         //6번째 토큰이 home경로다.
        if (strstr(home, "/home") != NULL) {
            char* username = strtok(line, ":");
            fclose(fp);
            strcpy(BACKUP_USERNAME, usrname);
            return usrname;
        }
    }
    fclose(fp);
    return NULL;
}
