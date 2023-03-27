#include "ssu_exec.h" // 관리 파일


typedef struct managenode {
    Filenode* filenode;
    //나중에 설정할 값들. 
    struct managenode* next;
    struct managenode* same_next;        // 같은 파일 , 다른 백업시간의 파일들을 연결하는 포인터. 
    struct managenode* same_next_tail;   // same_next에 연결된 노드 중 가장 마지막 노드, 삽입시간을 O(1) 로 하기위함. 없으면 NULL
    int same_cnt;                       // 자기 자신 노드를 제외하고 same_next에 연결된 파일 개수
                 
}Mnode;

typedef struct mlist{
    Mnode* dir_head;
    Mnode* file_head;
    Mnode* dir_tail;          // 연결리스트를 바로 연결하기 위한 구조. O(1)
    Mnode* file_tail;
    int file_cnt;                       // 파일 총 개수
    int dir_cnt;                        // 디렉토리 총 개수

    //★file_rear_table, file_cnt_table 는 max_file_cnt를 따라감.
}Mlist;


Mnode *new_mnodes(char* file_name, int opt, int f_opt);
Mlist* new_mlist();
void mappend (Mlist* mlist, char* file_name, int opt, int f_opt);

int main()
{
    Mnode* file = new_mnodes ("/home/junhyeong/backup/file2.cpp_230327142327", 1, 1); 

    mappend(mlist, "/home/junhyeong/backup/file2.cpp_230327142327", 1, 1);
    mappend(mlist, "/home/junhyeong/backup/file2.cpp_230327142349", 1, 1);
    mappend(mlist, "/home/junhyeong/backup/file2.cpp_230327142411", 1, 1);
    mappend(mlist, "/home/junhyeong/backup/ssu_add.c_230327142805", 1, 1);
    return 0;
}


Mnode *new_mnodes(char* file_name, int opt, int f_opt)
{
    Mnode* newnode = (Mnode*)malloc(sizeof(Mnode));
    newnode->filenode = new_filenodes(file_name, opt, f_opt);
    if (newnode->filenode == NULL)
        return NULL;
    newnode->next = NULL;
    newnode->same_next = NULL;
    newnode->same_next_tail = NULL;
    newnode->same_cnt = 0;
    return newnode;
}

Mlist* new_mlist()
{
    Mlist* mlist = (Mlist*)malloc(sizeof(Mlist));
    mlist->dir_head = NULL;
    mlist->file_head = NULL;
    mlist->dir_tail = NULL;          // 연결리스트를 바로 연결하기 위한 구조. O(1)
    mlist->file_tail = NULL;
    mlist->file_cnt = 0;                       // 파일 총 개수
    mlist->dir_cnt = 0;                        // 디렉토리 총 개수

    return mlist;
}



void mappend (Mlist* mlist, char* file_name, int opt, int f_opt)                  // 파일 array 대해 추가. option 0: orignal, 1: Backup
{
    Mnode* newfile = new_mnodes(file_name, opt, f_opt); // 동일하게 작동.

    int dir_check = 0;
    if (newfile == NULL)
        return;
    if(S_ISDIR(newfile->filenode->file_stat.st_mode))
    {
        if (mlist->dir_head == NULL)
        {
            mlist->dir_head = mlist->dir_tail = newfile;
        }
        else
        {
            mlist->dir_head->next = newfile;
            mlist->dir_head = newfile;
        }
        mlist->dir_cnt++;
        return;
    }
    else
    {
        if (mlist->file_head == NULL)
        {
            mlist->file_head = mlist->file_tail = newfile;
            mlist->file_cnt++;
            return;
        }
        if (opt == 0)                           //origianl 파일의 경로는 무조건 한개만 존재해야함
        {
            mlist->file_head->next = newfile;
            mlist->file_head = newfile;
            mlist->file_cnt++;
            return;
        }
        else
        {

            Mnode* start; 
            for(start = mlist->file_tail ; start != NULL ; start = start->next)
            {
                if (strcmp(start->filenode->inverse_path, newfile->filenode->inverse_path) == 0)
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
            
            mlist->file_head->next = newfile;
            mlist->file_head = newfile;
            mlist->file_cnt++;
            return;

        }
    }

}
