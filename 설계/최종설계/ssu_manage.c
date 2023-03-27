#include "ssu_exec.h" // 관리 파일


typedef struct managenode {
    Filenode* filenode;
    //나중에 설정할 값들. 
    struct managenode* next;
    struct managenode* same_next;        // 같은 파일 , 다른 백업시간의 파일들을 연결하는 포인터. 
    struct managenode* same_next_tail;   // same_next에 연결된 노드 중 가장 마지막 노드, 삽입시간을 O(1) 로 하기위함. 없으면 NULL
    int same_cnt;                       // 자기 자신 노드를 포함하고 same_next에 연결된 파일 개수
                 
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


/** 모두 03.27 추가 : 파일 관리자 구조체*/
Mnode *new_mnodes(char* file_name, int opt, int f_opt);                                 // Mnode 생성자 블럭 (filenode->구조체이용)
Mlist* new_mlist();                                                                     // mlist 생성자 블럭
void mappend (Mlist* mlist, char* file_name, int opt, int f_opt);                       // mlist 에 딕셔너리/파일 등 연결리스트에 연결해주는 구조체 (dir,file 구분연결)
void print_mlist (Mlist* mlist);                                                        // 파일 관리된 mlist 출력
void update_mlist (Mlist* mlist, Flist* flist, Rlist* rlist, int opt, int f_opt);       // flist, rlist 를 기준으로 manage list 를 업데하트하기
void free_mlist(Mlist* mlist);                                                          // add,remove,recover 함수 호출 후 삭제.
void pop_mlist (Mlist* mlist, char* delete_string);                                     // 백업 경로에 있는 파일 삭제 시 관리
Mlist* manage_backup_path_file();                                                       // 백업 경로에 있는 모든 파일 mlist화


int main()
{
    /**
    Mnode* file = new_mnodes ("/home/junhyeong/backup/file2.cpp_230327142327", 1, 1); 
    Mlist* mlist = new_mlist();
    mappend(mlist, "/home/junhyeong/backup/file2.cpp_230327142327", 1, 1);
    mappend(mlist, "/home/junhyeong/backup/file2.cpp_230327142349", 1, 1);
    mappend(mlist, "/home/junhyeong/backup/file2.cpp_230327142411", 1, 1);
    mappend(mlist, "/home/junhyeong/backup/ssu_add.c_230327142805", 1, 1);
    print_mlist(mlist);

    FRlist* frlist = ssu_recover_default ("/home/junhyeong/go2", 1, 0);
    Mlist* mlist2 = new_mlist();

    update_mlist (mlist2, frlist->flist, frlist->rlist,1, 0);
    pop_mlist (mlist2, "/home/junhyeong/go2/test_a");
    print_mlist(mlist2);
    free_mlist(mlist2);
    */


    Mlist* mlist3 =  manage_backup_path_file(1);
    print_mlist(mlist3);
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
    newnode->same_cnt = 1;
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
                char text1[MAXPATHLEN] = {0,};
                char text2[MAXPATHLEN] = {0,};
                strcpy(text1, start->filenode->path_name);
                strcpy(text2, newfile->filenode->path_name);
                char* token1 = strrchr(text1, '_');
                char* token2 = strrchr(text2, '_');
                if (token1 != NULL)
                    *token1 = '\0';
                if (token2 != NULL)
                    *token2 = '\0';
                
                if (strcmp(text1, text2) == 0)
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

void print_mlist (Mlist* mlist)
{
    printf("================ Start Mange List Print ===============\n");
    printf("-----------------Backup Dictinary-----------------\n");
    Mnode* tail = mlist->dir_tail;
    for (int i = 0 ; i < mlist->dir_cnt ; i++)      //딕셔너리 출력
    {
        printf("%s\n", tail->filenode->path_name);
        tail = tail->next;
    }
    printf("-----------------Backup File    -----------------\n");
    tail = mlist->file_tail;
    for (int i = 0 ; i < mlist->file_cnt ; i++)     //파일 출력
    {
        if (tail->same_cnt > 1)
        {
            Mnode* original = tail;
            for (int x = 0 ; x < tail->same_cnt ; x++)
            {
                printf("[%d] >> same file : %s\n",i+1, original->filenode->path_name);
                original = original->same_next;
            }
            tail = tail->next;
        }
        else
        {
            printf("[%d] %s\n",i+1, tail->filenode->path_name);
            tail = tail->next;
        }
    }
    printf("================ End Mange List Print ===============\n");

}



void update_mlist (Mlist* mlist, Flist* flist, Rlist* rlist, int opt, int f_opt)
{
    if (flist != NULL)            //flist 로 업데이트
    {
        for (int i = 0 ; i < flist->dir_cnt ; i++)      //딕셔너리 업데이트
        {
            mappend(mlist, flist->dir_array[i]->path_name, opt, f_opt);
        }

        for (int i = 0 ; i < flist->file_cnt ; i++)     //파일 출력
        {
            Filenode* original = flist->file_array[i];
            for (int x = 0 ; x < flist->file_cnt_table[i] ; x++)
            {
                mappend(mlist, original->path_name, 1, f_opt);
                original = original->next;
            }
        }
    }

    if (rlist != NULL)           //rlist 로 업데이트
    {  
        Filenode* tmp = rlist->rear;
        for (int i = 0 ; i < rlist->file_cnt ; i++)
        {
            mappend(mlist, tmp->path_name, 0, f_opt);
            tmp = tmp->next;
        }
    }
}

void free_mlist(Mlist* mlist)
{
    if (mlist == NULL)
        return;

    Mnode* delnode;
    while (mlist->dir_tail != NULL)
    {
        delnode = mlist->dir_tail;
        mlist->dir_tail = mlist->dir_tail->next;

        if (delnode != NULL)
        {
            free(delnode->filenode);
            free(delnode);
        }
    }

    delnode = NULL;
    while(mlist->file_tail != NULL)
    {
        while(mlist->file_tail->same_next != NULL)
        {
            delnode = mlist->file_tail->same_next;
            mlist->file_tail->same_next = mlist->file_tail->same_next->same_next;
            
            if (delnode != NULL)
            {
                free(delnode->filenode);
                free(delnode);
            }
        }
        delnode = mlist->file_tail;
        mlist->file_tail = mlist->file_tail->next;

        if (delnode != NULL)
        {
            free(delnode->filenode);
            free(delnode);
        }
    }

    if (mlist != NULL)
        free(mlist);
}


Mlist* manage_backup_path_file(int f_opt) // 백업 경로 파일 모두 긁어옴.
{
    if (strlen(BACKUP_PATH) == 0)
        get_backuppath();
    Flist* tmp = backup_search(BACKUP_PATH,f_opt,1);
    if (tmp == NULL)
        return NULL;
    
    Mlist* mlist = new_mlist();
    update_mlist(mlist, tmp, NULL, 1, f_opt);
    free_flist(tmp);
    return mlist;
}

//delete 명령어 용 pop 함수 : 백업디렉토리 파일 삭제에 특화된 함수.
void pop_mlist (Mlist* mlist, char* delete_string)
{
    Mnode* delnode;
    Mnode* prevnode = NULL;
    struct stat statbuf;
    if (stat(delete_string, &statbuf) < 0)
        return;

    Mnode* move;
    if (S_ISDIR(statbuf.st_mode))
    {
        move = mlist->dir_tail;
        while (move != NULL)
        {
            if (strstr(move->filenode->path_name, "backup") == NULL) //무조건 backup 부터 들어가도록 설계함.
                return;
            if (strcmp(move->filenode->path_name, delete_string) == 0)
            {
                //printf("!!! Delete Node %s (dictionary)!!!\n", delete_string);
                delnode = move;
                if (prevnode == NULL)
                {
                    mlist->dir_tail = mlist->dir_tail;
                    mlist->dir_cnt--;
                }
                else
                {
                    prevnode->next = move->next;
                    mlist->dir_cnt--;
                }

                if (delnode != NULL)
                {
                    free(delnode->filenode);
                    free(delnode);
                }
                return;
            }
            prevnode = move;
            move = move->next;
        }
    }

    if (S_ISREG(statbuf.st_mode))
    {
        move = mlist->file_tail;
        while(move != NULL)
        {
            if (strstr(move->filenode->path_name, "backup") == NULL) //무조건 backup 부터 들어가도록 설계함.
                return;
            char text1[MAXPATHLEN] = {0,};
            char text2[MAXPATHLEN] = {0,};
            strcpy(text1, move->filenode->path_name);
            strcpy(text2, delete_string);
            char* token1 = strrchr(text1, '_');
            char* token2 = strrchr(text2, '_');
            if (token1 != NULL)
                *token1 = '\0';
            if (token2 != NULL)
                *token2 = '\0';
            if (strcmp(text1, text2) == 0)
            {
                if (strcmp(move->filenode->path_name, delete_string) == 0)
                {
                    delnode = move;

                    if (prevnode == NULL)                   //root node 인 경우
                    {
                        if (mlist->file_tail->same_next != NULL)
                        {
                            mlist->file_tail = mlist->file_tail->same_next;         //1번
                            mlist->file_tail->next = move->next;
                            mlist->file_tail->same_next_tail = move->same_next_tail;
                            mlist->file_tail->same_cnt = move->same_cnt;
                            mlist->file_tail->same_cnt--;
                        }
                        else
                        {
                            mlist->file_tail = mlist->file_tail->next;
                            mlist->file_cnt--;
                        }
                    }
                    else
                    {
                        if (prevnode->same_next != NULL)
                        {
                            prevnode->next = move->same_next;    //2번
                            
                            prevnode->next->next = move->next;
                            prevnode->next->same_next_tail = move->same_next_tail;
                            prevnode->next->same_cnt = move->same_cnt;
                            prevnode->next->same_cnt--;
                        }
                        else
                        {
                            prevnode->next = move->next;
                            mlist->file_tail = mlist->file_tail->next;
                            mlist->file_cnt--;
                        }
                    }

                    if (delnode != NULL)
                    {
                        free(delnode->filenode);
                        free(delnode);
                    }
                    return;
                }
                else
                {
                    Mnode* sub_move = move->same_next;
                    Mnode* prev_sub_move = NULL;
                    while(sub_move != NULL)
                    {
                        if (strcmp(sub_move->filenode->path_name, delete_string) == 0)
                        {
                            delnode = sub_move;
                            if (prev_sub_move == NULL)
                            {
                                move->same_next = sub_move->next;          
                                move->same_cnt--;
                            }
                            else
                            {
                                prev_sub_move->same_next = sub_move->next;
                                move->same_cnt--; 
                            }
                            if (delnode != NULL)
                            {
                                free(delnode->filenode);
                                free(delnode);
                            }
                            return;
                        }
                        prev_sub_move = sub_move;                //이전 노드 기억.
                        sub_move = sub_move->same_next;
                    }
                }
            }
            prevnode = move;
            move = move->next;
        }
    }
}
