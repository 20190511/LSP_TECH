#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	if (chdir("/etc") < 0)  {	// /etc로 작업디렉토리 이동 실패 시 에러처리
		fprintf(stderr, "chdir error\n");
		exit(1);
	}

	printf("chdir to /etc succeeded.\n");
	exit(0);
}

