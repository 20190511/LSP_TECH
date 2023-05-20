#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_NUM 5

void* ssu_printhello(void* arg);

int main(void)
{
	pthread_t tid[THREAD_NUM];
	int i;

	for (i = 0 ; i < THREAD_NUM ; i++) {
		printf("In main: creating thread %d\n", i);
		
		//ssu_printhello 함수에 int i 를 void* 로 캐스팅해서 전달하고 있음
		if (pthread_create(&tid[i], NULL, ssu_printhello, (void*)&i) != 0) { 
			fprintf(stderr, "pthread_create error\n");
			exit(1);
		}
	}

	pthread_exit(NULL); //thread 를 종료시킴
	exit(0);
}

void* ssu_printhello(void* arg) {
	int thread_index;

	thread_index = *((int *)arg); // arg 가 void* 형태로 왔으니까 (int*) 르 캐스팅한 후 indirecting해줄 것.
	printf("Hello World! It's me, thread #%d!\n", thread_index);
	return NULL;
}


