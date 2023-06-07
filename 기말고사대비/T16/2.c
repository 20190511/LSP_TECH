#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


typedef struct dt {
	char qname[20];
	struct dt *next;
	struct dt *prev;
}Data;

Data record[100];
int record_cnt = 0;

void txt_ex (char* str, int* q1, int* q2);

void bubble()
{
	int i,j;
	Data* tmp;
	for (i = 0 ; i < record_cnt-1; i++) {
		for ( j = 0 ; j < record_cnt - i -1 ; j++) {
			int qa1,qa2, qb1,qb2;
			txt_ex(record[j].qname, &qa1, &qa2);
			txt_ex(record[j+1].qname, &qb1, &qb2);

			if (qa1 > qb1 || (qa1 == qb1 && qa2 > qb2)) {
				memcpy(tmp, &record[j], sizeof(record[j]));
				memcpy(&record[j], &record[j+1], sizeof(record[j+1]));
				memcpy(&record[j+1], tmp, sizeof(record[j]));
			}
		}
	}

}

void txt_ex (char* str, int* q1, int* q2) {
	char tmp [20];
	strcpy(tmp, str);
	char* tkn = strtok(tmp, "-.");
	if (tkn == NULL) {
		*q1 = 0;
		*q2 = 0;
		return;
	}
	
	*q1 = atoi(tkn);
	
	tkn = strtok(NULL, "-.");
	if (tkn == NULL) {
		*q2 = 0;
		return;
	}

	*q2 = atoi(tkn);
	return;
}

void read_data (char* csv)
{
	FILE* fp;
	char buf [1024];
	if ((fp = fopen(csv, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", csv);
		exit(1);
	}

	if (!feof(fp)) {
		fgets(buf, 1024, fp);
	}

	char* tok = strtok(buf, ",");
	bubble();
	while (tok != NULL) {
		strcpy(record[record_cnt++].qname, tok);
		tok = strtok(NULL, ",");
	}
}

int main(int argc, char* argv[])
{
	read_data(argv[1]);
	bubble();
	for (int i = 0 ; i < record_cnt ;i++)
		printf("Qname : %10s\n", record[i].qname);
	exit(0);
}
