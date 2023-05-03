#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
	sigset_t sig_set;
	int count;

	sigemptyset(&sig_set);
	sigaddset(&sig_set, SIGINT); //sig_set 을 모두 0으로 만들고 SIG_INT만 1로 만듦
	sigprocmask(SIG_BLOCK, &sig_set, NULL); //NULL이라서 블락 시그널을 모두 0인것과 Union(합집합) 시켜버림.

	for (count = 3 ; 0 < count ; count--) {
		printf("count %d\n", count);
		sleep(1);
	}

	printf("Ctrl-C에 대한 블록을 해제\n");
	sigprocmask(SIG_UNBLOCK, &sig_set, NULL); //블락 시그널을 모두 0인것과 sig_set을 intersection(교집합) 시켜버림 
	printf("count 중 Ctrl-C입력하면 이 문장은 출력되지 않음.\n"); //Pending(대기) 된 signal이 풀려서 SIGINT가 default(프로세스종료)가 되버림
	
	while(1);

	exit(0);
}
