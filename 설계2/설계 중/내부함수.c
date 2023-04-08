#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
// #define MAXPATHLEN 4097
#include "ssu_score.h"

typedef struct sclist {
    char qname[FILELEN];        // 문제번호
    double cur_score;              // 현 점수
    double score;                  // 원래배점
    struct sclist* file_next;        // 다음에 연결된 문제
}Sclist;

typedef struct stdnode {
    char id_name[10];           //학번
    double sums;                //총합

    int file_cnt;              //연결된 문제개수
    struct stdnode* next;              // 다음 학번
    Sclist* list_head;        // head -> .. -> .. -> tail ->NULL 형태로 연결
    Sclist* list_tail;
}Snode;


typedef struct slist {
    int id_cnt;
    Snode* head;            //header -> ... -> ... -> tail
    Snode* tail;  
}StdList;



Sclist* new_sclists (char* qname, double curS); //sclist 초기화함수
Sclist* new_sclistss (char* qname, double curS, double s); //문제 배점 붙은 Sclist 초기화함수
Snode* new_stdnode (char* id); //id 로 Snode 초기화함수
StdList* new_stdlist(); //stdlist 초기화함수
Snode* append_list (StdList* list, char* id);  // list 에 해당 id로 Snode 생성, return으로 append한 구조체 반환
void print_list (StdList* list);
void print_score (Snode* node); // Sclist 문제 요소 출력 (맞은문제, 문제배점)
int append_score(Snode* node, char* qname, double cur_score, double score);   // Snode 에 문제 구조체 Sclist append.
double find_score (char* qname);  // qname 과 일치하는 score 원래의 배점을 score_table 로부터 찾기
int swap_list (StdList* list, Snode* p, Snode* a, Snode* b);  // 버블 정렬에 필요한 a <-> b 링크드리스트 구조 변경
void sort_descentS(StdList* list); //list 의 sums(총함) 내림차순으로 정렬(버블sort)     
void sort_aescendS(StdList* list); //list 의 sums(총함) 오름차순으로 정렬(버블sort)
void sort_descentI(StdList* list); //list 의 id (학번) 내림차순으로 정렬(버블sort)
void sort_aescendI(StdList* list); //list 의 id (학번) 오름차순으로 정렬(버블sort)
void sort_manager(StdList* list, int type, int updown);     //sorting 관리 구조체
void print_wrongL (Snode* node); // node 안의 틀린문제들 요소 출력
int write_sort (StdList* list, int fd);  // 정렬된 리스트를 출력

int main(void)
{
    
    StdList* list = new_stdlist();
    read_scoreTable("/home/junhyeong/go2/ANS/score_table.csv");
    Snode* a1 = append_list(list, "20200001");
    Snode* a2 = append_list(list, "20200005");
    Snode* a3 = append_list(list, "20200007");
    Snode* a4 = append_list(list, "20200009");
    Snode* a5 = append_list(list, "20200011");
    Snode* a6 = append_list(list, "20200015");
    //swap_list(list, a2 , a3, a4);
    //swap_list(list, NULL, a1, a2);
    
    a1->sums = 105;
    a2->sums = 560;
    a3->sums = 320;    
    a4->sums = 900;
    a5->sums = 160;
    a6->sums = 500;

    append_score(list->head, "1-1.txt", 0.2, 0);
    append_score(list->head, "1-2.txt", 0.7, 0);
    append_score(list->head, "2.txt", 0.5, 10);
    append_score(list->head, "109.txt", 0.5, 15);
    
    append_score(a2, "1-1.txt", 0.2, 0);
    append_score(a2, "1-2.txt", 0.7, 0);
    append_score(a2, "2.txt", 0.5, 10);
    append_score(a2, "109.txt", 15, 15);


    sort_manager(list,0,-1);
    print_list(list);
    sort_manager(list,0,1);
    print_list(list);
    sort_manager(list,1,-1);
    print_list(list);
    sort_manager(list,1,1);
    print_list(list);

    print_wrongL(a1);
    print_wrongL(a2);
}

void print_wrongL (Snode* node)
{
    if (node == NULL)
        return;
    
    printf("wrong problem : ");
    int comma = 0;
    for (Sclist* tmp_q = node->list_head ; tmp_q != NULL ; tmp_q = tmp_q->file_next)
    {
        if (tmp_q->cur_score != tmp_q->score)
        {
            if (comma != 0)
                printf(", ");
            printf("%s(%.2f)", tmp_q->qname, tmp_q->score);
            comma++;
        }
    }
    printf("\n");
}


int write_sort (StdList* list, int fd)
{
    char tmp[20];
    write_first_row(fd);

    for (Snode* id_tmp = list->head ; id_tmp != NULL ; id_tmp = id_tmp->next)
    {
        sprintf(tmp, "%s,", id_tmp->id_name);
        if (write(fd, tmp, strlen(tmp)) != strlen(tmp))
        {
            return false;
        }
        for (Sclist* q_tmp = id_tmp->list_head ; q_tmp != NULL ; q_tmp = q_tmp->file_next)
        {
            sprintf(tmp, "%.2f,", q_tmp->cur_score);
            if (write(fd, tmp, strlen(tmp)) != strlen(tmp))
            {
                return false;
            }
        }
    }
    return true;
}


/**
 * type  0:학번, 1:총점수 sums
 * updown -1:내림차순, 1:오름차순
*/
void sort_manager(StdList* list, int type, int updown)
{
    if (list == NULL)
        return;

    if (type == 0 && updown == -1)
        sort_descentI(list);
    else if (type == 1 && updown == 1)
        sort_aescendI(list);
    else if (type == 0 && updown == -1)
        sort_descentS(list);
    else if (type == 1 && updown == 1)
        sort_aescendS(list);
}

void sort_aescendI(StdList* list)
{

    Snode* swap_tmp = NULL;
    for (int x = 0 ; x < list->id_cnt-1 ;x++)
    {
        Snode* par = NULL;
        Snode* tmp = list->head;
        for (int y = 0 ; y < list->id_cnt-x-1 ; y++)
        {
            if (atoi(tmp->id_name) > atoi(tmp->next->id_name))
            {
                swap_tmp = tmp->next; //swaping 이 일어나면 next가 바뀌는 문제 해결 위함
                swap_list(list, par, tmp, tmp->next);
                tmp = swap_tmp;
            }
            par = tmp;
            tmp = tmp->next;
        }
    }
}

void sort_descentI(StdList* list)
{

    Snode* swap_tmp = NULL;
    for (int x = 0 ; x < list->id_cnt-1 ;x++)
    {
        Snode* par = NULL;
        Snode* tmp = list->head;
        for (int y = 0 ; y < list->id_cnt-x-1 ; y++)
        {
            if (atoi(tmp->id_name) < atoi(tmp->next->id_name))
            {
                swap_tmp = tmp->next; //swaping 이 일어나면 next가 바뀌는 문제 해결 위함
                swap_list(list, par, tmp, tmp->next);
                tmp = swap_tmp;
            }
            par = tmp;
            tmp = tmp->next;
        }
    }
}

void sort_aescendS(StdList* list)
{

    Snode* swap_tmp = NULL;
    for (int x = 0 ; x < list->id_cnt-1 ;x++)
    {
        Snode* par = NULL;
        Snode* tmp = list->head;
        for (int y = 0 ; y < list->id_cnt-x-1 ; y++)
        {
            if (tmp->sums > tmp->next->sums)
            {
                swap_tmp = tmp->next; //swaping 이 일어나면 next가 바뀌는 문제 해결 위함
                swap_list(list, par, tmp, tmp->next);
                tmp = swap_tmp;
            }
            par = tmp;
            tmp = tmp->next;
        }
    }
}


void sort_descentS(StdList* list)
{

    Snode* swap_tmp = NULL;
    for (int x = 0 ; x < list->id_cnt-1 ;x++)
    {
        Snode* par = NULL;
        Snode* tmp = list->head;
        for (int y = 0 ; y < list->id_cnt-x-1 ; y++)
        {
            if (tmp->sums < tmp->next->sums)
            {
                swap_tmp = tmp->next; //swaping 이 일어나면 next가 바뀌는 문제 해결 위함
                swap_list(list, par, tmp, tmp->next);
                tmp = swap_tmp;
            }
            par = tmp;
            tmp = tmp->next;
        }
    }
}


/**
 * 버블 정렬을 할 것이기 때문에, p->a->b->next 형태를 무조건 띄고 있음
 *  p->next = b
 *  a->next = b->next
 *  b->next = a
 *  를하면됨
*/
int swap_list (StdList* list, Snode* p, Snode* a, Snode* b)  /// 버블 정렬에 필요한 a <-> b 링크드리스트 구조 변경 (a, b 링크는 인접함.)
{
    if (a == NULL || b == NULL)
        return false;
    
    if (list->head == a && p == NULL) // 리스트의 head와 p 가 동일하면.. (+p == NULL)
    {
        a->next = b->next;
        list->head = b;
        b->next = a;
        return true;
    }
    else
    {
        a->next = b->next;
        p->next = b;
        b->next = a;
        return true;
    }
}

double find_score (char* qname)
{
    for (int i = 0 ; i < QNUM ; i++)
    {
        if (!strcmp(score_table[i].qname, qname))
        {
            return score_table[i].score;
        }
    }
    return -1;
}


void print_score (Snode* node)
{
    if (node == NULL)
        return;
    printf("------\n");    
    for (Sclist* tmp = node->list_head ; tmp != NULL ; tmp = tmp->file_next)
    {
        printf("[%s] score = %0.2f, cur_score = %0.2f\n", tmp->qname, tmp->score, tmp->cur_score);
    }
    printf(">>> total question count = %d\n", node->file_cnt);
    printf("------\n");
}

int append_score(Snode* node, char* qname, double cur_score, double score)
{
    if (node == NULL)
        return false;
    
    Sclist* newnode;
    
    if (score == 0) // 문제 찾아서 넣는 함수 제작 필요.
    {
        newnode  = new_sclists(qname, cur_score);
        newnode->score = find_score(qname);
    }
    else
        newnode = new_sclistss(qname, cur_score, score);

    
    if (newnode == NULL)
        return false;

    if (node->list_head == NULL)
    {
        node->list_head = node->list_tail = newnode;
        node->file_cnt++;
        return true;
    }
    else
    {
        node->list_tail->file_next = newnode;
        node->list_tail = newnode;
        node->file_cnt++;
        return true;
    }
}


void print_list (StdList* list)
{
    printf("--------------<start printing node>---------------\n");
    for (Snode* tmp = list->head ; tmp != NULL ; tmp = tmp->next)
    {
        printf("%s\n", tmp->id_name);        
        printf("sums : %f\n", tmp->sums);
    }
    printf(">>> total id_cnt = %d\n", list->id_cnt);
    printf("--------------<end printing node>---------------\n");

}

Snode* append_list (StdList* list, char* id)
{
    if (list == NULL)
        return NULL;

    Snode* newnode = new_stdnode(id);
    if (newnode == NULL)
        return NULL;
    
    if (list->head == NULL)                 /// 헤드가 NULL이면 초기연결
    {
        list->head = list->tail = newnode;
        list->id_cnt++;
        return newnode;
    }
    else                                    ///아닐 시, tail 추가연결
    {
        list->tail->next = newnode;
        list->tail = newnode;
        list->id_cnt++;
        return newnode;
    }

}



Sclist* new_sclists (char* qname, double curS)
{
    Sclist* node = (Sclist*)malloc(sizeof(Sclist));
    strcpy(node->qname, qname);
    node->cur_score = curS;
    node->score = 0;
    node->file_next = NULL;

    return node;
}


Sclist* new_sclistss (char* qname, double curS, double s)
{
    Sclist* node = (Sclist*)malloc(sizeof(Sclist));
    strcpy(node->qname, qname);
    node->cur_score = curS;
    node->score = s;
    node->file_next = NULL;

    return node;
}

Snode* new_stdnode (char* id)
{
    Snode* node = (Snode*)malloc(sizeof(Snode));
    strcpy(node->id_name, id);

    node->file_cnt = 0;
    node->sums = 0;
    node->next = NULL;
    node->list_head = node->list_tail = NULL;

    return node;
}

StdList* new_stdlist()
{
    StdList* list = (StdList*)malloc(sizeof(StdList));
    list->head = list->tail = NULL;
    list->id_cnt = 0;

    return list;
}
