#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct data {
    char data [100];
    struct data* next;
    struct data* prev;
}Data;
Data record[100];
int idx  = 0;
#define BUFSIZE 1024

void bubble_sort ();
void tok_name (char* str, int* q1, int* q2);

int main(void)
{
    char fname [BUFSIZE];
    char buf[BUFSIZE];
    FILE* fp;
    printf("20190511> ");
    fgets(fname, sizeof(fname), stdin);
    fname[strlen(fname)-1] = '\0';

    if ((fp = fopen(fname, "r"))== NULL) {
        fprintf(stderr, "fopen error for %s\n", fname);
        exit(1);
    }


    fgets(buf, BUFSIZE, fp);
    char* tkn = strtok(buf, ",");
    while (tkn != NULL) {
        strcpy(record[idx++].data, tkn);
        tkn = strtok(NULL, ",");
    }
    //bubble_sort();
    for (int i = 0 ; i < idx ; i++)
        printf("data[%d] : %s\n", i, record[i].data);


    exit(0);
}


void bubble_sort ()
{
    int i, j;
    Data* tmp;
    for (i = 0 ;i < idx - 1 ; i++) {
        for (j = 0 ; j < idx - 1 - i ; j++) {
            int qa1, qa2, qb1, qb2;
            tok_name(record[j].data, &qa1, &qa2);
            tok_name(record[j+1].data, &qb1, &qb2);

            if (qa1 < qb1 || (qa1 == qb1 && qa2 < qb2)) {
                memcpy(tmp, &record[j], sizeof(record[j]));
                memcpy(&record[j], &record[j+1], sizeof(record[j+1]));
                memcpy(&record[j+1], tmp, sizeof(record[j]));
            }
        }
    }

}

void tok_name (char* str, int* q1, int* q2)
{
    char qes[10];
    strcpy(qes, str);
    char* n1 = strtok(qes, ".-");
    if (n1 == NULL) {
        *q1 = 0;
        *q2 = 0;
        return;
    }
    else
    *q1 = atoi(n1);
    char* n2 = strtok(qes, ".-");
    if (n2 == NULL) {
        *q2 = 0;
        return; 
    }
    *q2 = atoi(n2);
    return;
}
