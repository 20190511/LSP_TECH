#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXPATHLEN 1024
#define MAXTABLE

typedef struct table{
	char qname[20];
	double score;
	struct table* next;
	struct table* prev;
}Data;

Data* read_file(char* csv);
int main (int argc, char* argv[])
{
	Data* node = read_file(argv[1]);
	while(node != NULL) {
		printf("Qname : %10s, Score : %f\n", node->qname, node->score);
		node = node->next;
	}
	exit(0);
}

Data* read_file(char* csv)
{
	FILE* fp;
	if ((fp = fopen(csv, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", csv);
		exit(1);
	}

	char qname[20];
	char score[20];
	Data* head = (Data*)malloc(sizeof(Data));
	Data* ptr = head;
	while (fscanf(fp, "%[^,],%s\n", qname, score) > 0) {
		Data* newNode = (Data*)malloc(sizeof(Data));
		strcpy(newNode->qname, qname);
		newNode->score = atof(score);
		
		ptr->next = newNode;
		newNode->prev = ptr;
		ptr = newNode;
	}

	return head->next;
}
