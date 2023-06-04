#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct ans {
	char qname[100];
	double answer;
	struct ans *next;
	struct ans *prev;
}Ans;

#define MAXPATHLEN 1024
#define true 1
#define false 0

int score_blank (char* fname1, char* fname2);
char* ans_tok (int fd, char* answer);
int main(int argc, char* argv[])
{
	
	int check = score_blank (argv[1], argv[2]);
	if (check)
		printf("OOOOO\n");
	else
		printf("XXXXX\n");

	exit(0);
}


int score_blank (char* fname1, char* fname2)
{
	int fd1 = open(fname1, O_RDONLY);
	int fd2 = open(fname2, O_RDONLY);
	char answer[100];
	char stu [100];

	if (fd1 < 0) {
		fprintf(stderr, "open error for %s\n", fname1);
		exit(1);
	}

	if (fd2 < 0) {
		fprintf(stderr, "open error for %s\n", fname2);
		exit(1);
	}
	
	char* stu2 = ans_tok (fd2, stu);
	if (stu2 == NULL)
		return false;
	
	char* ans2 = ans_tok (fd1, answer);
	while(strcmp(answer, "")) {
		if (!strcmp(stu, answer)) {
			close(fd1);
			close(fd2);
			return true;
		}

		ans2 = ans_tok (fd1, answer);
	}
	
	close(fd1);
	close(fd2);
	return false;


}
char* ans_tok (int fd, char* answer)
{
	char ch;
	char* ptr = answer;
	memset(answer, 0, sizeof(answer));	

	while (read(fd, &ch, 1) > 0)
	{
		if (ch == -1 || ch == ':')
			break;
		
		if (ch != ' ')
			*ptr++ = ch;
	}
	
	*ptr = '\0';
	if (!strcmp(answer, ""))
		return NULL;
	if (answer[strlen(answer) -1] == '\n')
		answer[strlen(answer) -1] = '\0';

	return answer;
}

