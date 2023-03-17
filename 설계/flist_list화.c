#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssu_exec.h"

typedef struct filenode2 {
    //초기화할 때 설정해줘야하는 값
    char path_name [MAXPATHLEN];       
    char file_name [MAXFILELEN];        
    char actual_path [MAXPATHLEN];        
    char inverse_path [MAXPATHLEN];     
    char hash[HASH_LEN];                
    struct stat file_stat;               
    char back_up_time [TIME_TYPE];      
    
    //나중에 설정할 값들.
    struct filenode2* next;      
    struct filenode2* same_next; // 같은 파일 , 다른 백업시간의 파일들을 연결하는 포인터. 
    struct filenode2* same_next_tail;
    int same_cnt;               // 자기 자신 노드를 포함하여 same_next에 연결된 파일 개수
                 
}Filenode2;

typedef struct flist2{
    Filenode2* dir_head;
    Filenode2* file_head;
    Filenode2* dir_tail;          // 연결리스트를 바로 연결하기 위한 구조. O(1)
    Filenode2* file_tail;
    int file_cnt;                       // 파일 총 개수
    int dir_cnt;                        // 디렉토리 총 개수

    //★file_rear_table, file_cnt_table 는 max_file_cnt를 따라감.
}Flist2;

void append2 (Flist2* flist, char* file_name, int opt, int f_opt)                  // 파일 array 대해 추가. option 0: orignal, 1: Backup
{
    Filenode2* newfile = new_filenodes2(file_name, opt, f_opt); // 동일하게 작동.
    int dir_check = 0;
    if (newfile == NULL)
        return;
    if(S_ISDIR(newfile->file_stat.st_mode))
    {
        if (flist->dir_head == NULL)
        {
            flist->dir_head = flist->dir_tail = newfile;
        }
        else
        {
            flist->dir_head->next = newfile;
            flist->dir_head = newfile;
        }
        flist->dir_cnt++;
        return;
    }
    else
    {
        if (flist->file_head == NULL)
        {
            flist->file_head = flist->file_tail = newfile;
            flist->file_cnt++;
            return;
        }
        if (opt == 0)                           //origianl 파일의 경로는 무조건 한개만 존재해야함
        {
            flist->file_head->next = newfile;
            flist->file_head = newfile;
            flist->file_cnt++;
            return;
        }
        else
        {

            Filenode2* start; 
            for(start = flist->file_tail ; start != NULL ; start = start->next)
            {
                if (strcmp(start->inverse_path, newfile->inverse_path) == 0)
                {
                    if (start->same_next_tail == NULL)  
                    {
                        start->same_next = start->same_next_tail = newfile;
                    }
                    else
                    {
                        start->same_next_tail->same_next = newfile;
                        start->same_next_tail = newfile;
                    }
                    start->same_cnt++;          //파일명만 동일한 파일에 대해서는 file_cnt를 늘려주지 않도록 설계
                    return;
                }
            }
            
            flist->file_head->next = newfile;
            flist->file_head = newfile;
            flist->file_cnt++;
            return;

        }
    }

}
