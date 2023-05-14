#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	printf("abort terminate this program.\n");
	abort(); //abort 시켜버려서 아래 printf는 출력되지 않음(비정상 종료)
	printf("this line is never reached\n");
	exit(0);
}
