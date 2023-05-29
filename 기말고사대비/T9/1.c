#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAXPATHLEN 1024
typedef struct pt {
	char* path;
	struct pt *next;
	struct pt *prev;
}Pt;


char** token (char* str, int* cnt);
char* convert (char* str, char* resolve);
int main(void)
{
	char str[MAXPATHLEN];
	char resolve [MAXPATHLEN];

	while(1)
	{
		printf("Your Path >> ");
		if (fgets(str, MAXPATHLEN, stdin) <= 0)
			continue;
		else if (!strcmp(str, "\n"))
			continue;

		str[strlen(str)-1] = '\0';
		if (!strcmp(str, "x"))
			break;
		convert (str, resolve);
		printf("%s\n", resolve);
	}
}

char** token (char* str, int* cnt)
{
	char tmp[MAXPATHLEN];
	char *tmp_list[100] = {NULL,};
	*cnt = 0;
	strcpy(tmp, str);
	char *tkn = strtok(tmp, "/");
	
	while (tkn != NULL) {
		tmp_list[*cnt] = tkn;
		*cnt += 1;
		tkn = strtok(NULL, "/");
	}

	char **list = (char**)malloc(sizeof(char*) * *cnt);
	for (int i = 0 ; i < *cnt ; i++) {
		list[i] = tmp_list[i];
	}

	return list;
}
char* convert (char* str, char* resolve)
{
	char str2[MAXPATHLEN * 2];
	
	if (str[0] == '~')
		sprintf(str2, "%s/%s", getenv("HOME"), str+1);
	else if (str[0] != '/')
		sprintf(str2, "%s/%s", getcwd(NULL, MAXPATHLEN), str);
	else
		strcpy(str2, str);
	
	int cnt;
	char** list = token(str2, &cnt);
	Pt* header = (Pt*)malloc(sizeof(Pt));
	Pt* ptr = header;
	for (int i = 0 ; i < cnt ; i++) {
		if (!strcmp(list[i], "."))
			continue;
		else if (!strcmp(list[i], "..")) {
			ptr = ptr->prev;
			ptr->next = NULL;
			continue;
		}
		
		Pt* newNode = (Pt*)malloc(sizeof(Pt));
		newNode->path = list[i];
		ptr->next = newNode;
		newNode->prev = ptr;
		ptr = newNode;
	}
	
	header = header->next;
	strcpy(str2, "/");
	while(header != NULL) {
		strcat(str2, header->path);
		strcat(str2, "/");
		header = header->next;
	}

	if (strcmp(str2, "/"))
		str2[strlen(str2)-1] = '\0';

	strcpy (resolve, str2);
	return resolve;
}
