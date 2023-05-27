#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXPATHLEN 4096

typedef struct pt {
	struct pt *next;
	struct pt *prev;
	char* path;
}Pt;

char** token(char* str, int* cnt);
char* convert (char* str, char* resolve_path);

int main(void)
{
	char str[MAXPATHLEN];
	char resolve [MAXPATHLEN];

	while(1)
	{
		if (fgets(str, MAXPATHLEN, stdin) < 0)
			break;
		
		str[strlen(str)-1] = '\0';
		
		if (!strcmp(str, "x"))
			break;

		convert(str, resolve);
		printf("%s\n", resolve);
	}
}


char** token(char* str, int* cnt)
{
	char tmp [MAXPATHLEN];
	char *tmp_list[100] = {NULL,};
	strcpy(tmp, str);
	*cnt = 0;
	char *tkn = strtok(tmp, "/");
	while (tkn != NULL) {
		tmp_list[*cnt] = tkn;
		*cnt += 1;
		tkn = strtok(NULL, "/");
	}
	
	char** list = (char**)malloc(sizeof(char*) * *cnt);
	for (int i = 0 ; i < *cnt ; i++)
		list[i] = tmp_list[i];

	return list;
}

char* convert (char* str, char* resolve_path)
{
	char tmp[MAXPATHLEN*2];
	
	if (str == NULL)
		return NULL;
	else if (str[0] == '~')
		sprintf(tmp, "%s/%s", getenv("HOME"), str+1);
	else if (str[0] != '/')
		sprintf(tmp, "%s/%s", getcwd(NULL, MAXPATHLEN), str);
	else
		strcpy(tmp, str);

	int cnt;
	char** list = token(tmp, &cnt);

	Pt* head = (Pt*)malloc(sizeof(Pt));
	Pt* ptr = head;
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
	
	head = head->next;
	strcpy(tmp, "/");
	
	while (head != NULL) {
		strcat(tmp, head->path);
		strcat(tmp, "/");
		head = head->next;
	}
	
	if (strcmp(tmp, "/"))
		tmp[strlen(tmp) - 1] = '\0';

	strcpy(resolve_path, tmp);
	return resolve_path;
}
