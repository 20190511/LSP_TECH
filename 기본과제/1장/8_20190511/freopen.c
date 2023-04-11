#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	char *fname = "ssu_test.txt";
	int fd;

	printf("First printf : Hello, OSLAB!!\n");

	if ((fd = open(fname, O_RDONLY)) < 0) { // 첫번쨰는 그냥 오픈함
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}
	//freopen --> stdout(1) 디스크립터로 fname을 open해줌
	if (freopen(fname, "w", stdout) != NULL) // 두 번째는 freopen 을 통해서 stdout 을 쓰기 모드로 재오픈
		printf("Second printf : Hello, OSLAB!!\n");

	exit(0);
}
