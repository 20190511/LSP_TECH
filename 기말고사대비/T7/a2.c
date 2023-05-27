#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int length;
int buf[100];

void *ssu_thread_producer(void* arg);
void *ssu_thread_consumer (void* arg);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

int main(void)
{
	pthread_t tid1, tid2;

	pthread_create(&tid1, NULL, ssu_thread_producer, NULL);
	pthread_create(&tid2, NULL, ssu_thread_consumer, NULL);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond1);
	pthread_cond_destroy(&cond2);
	exit(0);
}

void *ssu_thread_producer(void* arg)
{
	int i;
	for (i = 0 ; i <= 300 ; i++)
	{
		pthread_mutex_lock(&mutex);
		
		buf[length++] = i;

		if (length == 100) {
			pthread_cond_signal(&cond2);
			pthread_cond_wait(&cond1, &mutex);
		}
		
		if (i == 300)
			pthread_cond_signal(&cond2);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}
void *ssu_thread_consumer (void* arg)
{
	int sums = 0;
	int i;
	for (i = 0 ;i <= 300 ; i++) {
		pthread_mutex_lock(&mutex);

		if (length == 0) {
			pthread_cond_signal(&cond1);
			pthread_cond_wait(&cond2, &mutex);
		}
		sums += buf[--length];
		pthread_mutex_unlock(&mutex);
	}
	printf("%d\n", sums);
	return NULL;
}


