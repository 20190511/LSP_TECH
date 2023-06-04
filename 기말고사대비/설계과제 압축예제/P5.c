#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define MAXARG 5
char *arg_set[MAXARG];
void opt_test (int argc, char* argv[]);
int main(int argc, char* argv[])
{
	opt_test(argc, argv);
	
	for (int i = 0 ; i < MAXARG ; i++) 
		printf("arg %d > %s\n", i+1, arg_set[i]);
	exit(0);
}



void opt_test (int argc, char* argv[])
{
	int ch;
	while((ch = getopt(argc, argv, "abcd")) != -1) {

		if (ch == 'a') {
			int i = optind;
			int cnt = 0;
			int overlap = 0;
			while (i < argc && argv[i][0] != '-') {
				if (cnt >= MAXARG) {
					if (!overlap)
						printf("argument Overlap! : ");
					overlap = 1;
					printf("%s ", argv[i]);
				}
				else	
					arg_set[cnt] = argv[i];
				i++;
				cnt++;
			}
			if (overlap)
				printf("\n");
		}

		else if (ch == 'b') {
			printf("opt b\n");
		}

		else if (ch == 'c') {
			printf("opt c\n");
		}

		else if (ch == 'd') {
			printf("opt d\n");
		}
	}
}
