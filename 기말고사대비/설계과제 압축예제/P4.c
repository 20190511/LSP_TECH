#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define SIZE_Q 100
typedef struct ans {
	char qname [SIZE_Q];
	double score;
	struct ans *prev;
	struct ans *next;
}Ans;

Ans* read_qname (char* filename);
int main(int argc, char* argv[])
{
	Ans* head = read_qname(argv[1]);
	for (int i = 0; head != NULL ; head = head->next, i++) 
		printf("qname[%d] = %s, Score = %.2f\n", i+1, head->qname, head->score);

	exit(0);
}

Ans* read_qname (char* filename)
{
	FILE *fp = fopen(filename, "r");
	char qname[SIZE_Q];
	char score[20];

	if (fp == NULL) {
		fprintf(stderr, "fopene error for %s\n", filename);
		exit(1);
	}
	
	Ans* head = (Ans*)malloc(sizeof(Ans));
	Ans* ptr = head;
	while (fscanf(fp, "%[^,],%s \n", qname, score) > 0) {
		Ans* newNode = (Ans*)malloc(sizeof(Ans));
		strcpy(newNode->qname, qname);
		newNode->score = atof(score);

		ptr->next = newNode;
		newNode->prev = ptr;
		ptr = newNode;
	}

	return head->next;
}



