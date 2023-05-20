#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *ssu_thread1 (void* arg);
void *ssu_thread2 (void* arg);

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

int count = 0;
int input = 0;
int t1 = 0 , t2 = 0;

int main(void)
{
	pthread_t tid1, tid2;
	int status;

	if (pthread_create(&tid1, NULL, ssu_thread1, NULL) != 0) {
		fprintf(stderr, "phtread_create error\n");
		exit(1);
	}

	if (pthread_create(&tid2, NULL, ssu_thread2, NULL) != 0) {
		fprintf(stderr, "phtread_create error\n");
		exit(1);
	}

	while (1) {
		printf("2 개 이상의 개수 입력 : ");
		scanf("%d", &input);
		
		if (input >= 2) {
			pthread_cond_signal(&cond1);
			break;
		}
	}

	pthread_join(tid1, (void*)&status); //thread 종료할 때 까지 기다림
	pthread_join(tid2, (void*)&status); //thread 종료할 때 까지 기다림 

	printf("complete \n");
	exit(0);
}


void *ssu_thread1 (void* arg)
{
	while(1) {
		pthread_mutex_lock(&mutex1);
		
		if (input < 2) 
			pthread_cond_wait(&cond1, &mutex1);

		if (input == count) { //count가 끝났으므로 break;
			pthread_cond_signal(&cond2); //끝나기 전에 cond2 깨우기
			break;
		}

		if (count == 0) { //count==0이기 때문에 t2 그냥 증가 
			t2++;
			count++;
			printf("Thread 1 : %d\n", t1);
		}
		else if (count % 2 == 0) {
			t1 += t2;
			count++;
			printf("Thread 1 : %d\n", t1);
		}
		
		pthread_cond_signal(&cond2); //cond2 를 깨우기
		pthread_cond_wait(&cond1, &mutex1);	//cond1 대기
		pthread_mutex_unlock(&mutex1); //lock해제
	}
}

void *ssu_thread2 (void* arg)
{
	while(1) {
		pthread_mutex_lock(&mutex2);
		
		if (input < 2)  //input 값이 이상하므로 일단 대기
			pthread_cond_wait(&cond2, &mutex2);

		if (input == count) { //count가 끝났으므로 break;
			pthread_cond_signal(&cond1); //끝나기 전에 cond1 깨우기
			break;
		}
		
		if (count == 1) { //피보니치 수열의 a1 이므로 count++ 용도로 사용
			count++; 
			printf("Thread 2 : %d\n", t2);
		}
		else if (count % 2 == 1) {
			t2 += t1;
			count++;
			printf("Thread 2 : %d\n", t2);
		}
		pthread_cond_signal(&cond1);  //cond1 를 깨움(Thread1 깨우러감)
		pthread_cond_wait(&cond2, &mutex2);	 //cond2는 wait
		pthread_mutex_unlock(&mutex2);
	}
}

