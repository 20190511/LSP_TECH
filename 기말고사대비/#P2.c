#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#define false 0
#define true 1
#define MAX_NAME 256

void lowerToCase(char* ch);
int compare(char* file1, char* file2);
int redirection(char* file1, char* file2, int old, int new);

int main(int argc, char* argv[])
{
	char file1[MAX_NAME];
	char file2[MAX_NAME];
	char file3[MAX_NAME];
	strcpy(file1, argv[1]);
	strcpy(file2, argv[2]);
	strcpy(file3, argv[3]);
	
	int new;
	if ((new = open(file3, O_CREAT | O_RDWR | O_APPEND, 0644)) < 0) {
		fprintf(stderr, "open error for %s\n", file3);
		exit(1);
	}

	redirection(file1, file2, STDOUT_FILENO, new);
	
	exit(0);

}

int redirection(char* file1, char* file2, int old, int new)
{
	int saved;
	int error;
	error = dup(STDERR_FILENO);
	dup2(new, STDERR_FILENO);
	saved = dup(old);
	dup2(new, old);
	printf("hello\n");
	
	int check =	compare(file1, file2);
	if (check)
		printf("%s< %s> identical file\n", file1, file2);
	else
		printf("%s< %s> no identical file\n", file1, file2);
	
	
	dup2(error, STDERR_FILENO);
	dup2(saved, old);	
	return true;
}


int compare(char* file1, char* file2)
{
	int fd1, fd2;
	int len1, len2;
	char a,b;
	if((fd1 = open(file1, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", file1);
		exit(1);
	}

	if((fd2 = open(file2, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", file2);
		exit(1);
	}
	
	while (1)
	{
		while ((len1 = read(fd1, &a, 1)) > 0) {
			if (a == ' ')
				continue;
			else
				break;
		}

		while ((len2 = read(fd2, &b, 1)) > 0) {
			if (b == ' ')
				continue;
			else
				break;
		}
	
		if (len1 == 0 && len2 == 0)
			break;

		lowerToCase(&a);
		lowerToCase(&b);
		if (a != b) {
			close(fd1);
			close(fd2);
			return false;
		}
	}
	close(fd1);
	close(fd2);
	return true;
}
void lowerToCase(char* ch)
{
	if (*ch >= 'A' && *ch <= 'Z')
		*ch = *ch - 'A' + 'a';
}
