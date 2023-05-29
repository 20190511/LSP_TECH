#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *ssu_thread1 (void* arg);
void *ssu_thread2 (void* arg);

int count = 0;
int input = 0;
int t1 = 0, t2 = 0;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

int main(void)
{
	pthread_t tid1, tid2;
	if (pthread_create(&tid1, NULL, ssu_thread1, NULL) != 0) {
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}

	if (pthread_create(&tid2, NULL, ssu_thread2, NULL) != 0) {
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}
	
	while(1)
	{
		printf("2개 이상의 개수 입력: ");
		scanf("%d", &input);

		if (input < 2)
			continue;
		pthread_cond_signal(&cond1);
		break;
	}
	
	pthread_join (tid1, NULL);
	pthread_join (tid2, NULL);
	printf("complete\n");
	exit(0);
}

void *ssu_thread1 (void* arg)
{
	while (1) {
		pthread_mutex_lock(&mutex1);
	
		if (input < 2) {
			pthread_cond_wait (&cond1, &mutex1);
		}

		if (count == input) {
			pthread_cond_signal(&cond2);
			break;
		}
		if (count == 0) {
			t2 += 1;
			printf("Thread1 : %d\n", t1);
			count++;
		}
		else if (count % 2 == 0) {
			t1 += t2;
			printf("Thread1 : %d\n", t1);
			count++;

		}
		pthread_cond_signal(&cond2);
		pthread_cond_wait(&cond1, &mutex1);
		pthread_mutex_unlock(&mutex1);
	}
	return NULL;
}
void *ssu_thread2 (void* arg)
{
	while(1) {
		pthread_mutex_lock(&mutex2);

		if (input < 2) {
			pthread_cond_wait (&cond2, &mutex2);
		}

		if (count == input) {
			pthread_cond_signal(&cond1);
			break;
		}

		if (count % 2 == 1) {
			t2 += t1;
			count++;
			printf("Thread2 : %d\n", t2);

		}
		pthread_cond_signal(&cond1);
		pthread_cond_wait(&cond2, &mutex2);
		pthread_mutex_unlock(&mutex2);
	}
	return NULL;
}
