#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main(void)
{
	char buf[BUFFER_SIZE];
	int a, b;
	int i;

	setbuf(stdin, buf); // 표준 입력에 buf 설정 
	scanf("%d %d", &a, &b);

	for (i = 0; buf[i] != '\n'; i++)
		putchar(buf[i]); // putchar 을 하지만 \n 전까지 하기 때문에 출력은 되지 않음

	putchar('\n'); //해당 부분에서 한 꺼번에출력 
	exit(0);
}
