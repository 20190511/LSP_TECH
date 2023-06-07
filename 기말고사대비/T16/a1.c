#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *ssu_thread1 (void* arg);
void *ssu_thread2 (void* arg);
int glob_var1 = 1, glob_var2 = 2;

int main(void)
{
	pthread_t tid1, tid2;
	pthread_create(&tid1, NULL, ssu_thread1, NULL);
	pthread_create(&tid2, NULL, ssu_thread2, NULL);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	exit(0);
}


void *ssu_thread1 (void* arg)
{
	sleep(1);
	glob_var1 = 2, glob_var2 = 1;
	if (glob_var1 > glob_var2) 
		pthread_cond_signal(&cond);
	printf("ssu_thread1 end\n");	
	return NULL;
}
void *ssu_thread2 (void* arg)
{
	struct timeval timeval;
	struct timespec timeout;
	
	pthread_mutex_lock (&mutex);
	gettimeofday(&timeval, NULL);
	timeout.tv_sec = timeval.tv_sec + 5;
	timeout.tv_nsec = timeval.tv_usec * 1000;
	
	if (glob_var1 < glob_var2) {
		printf("ssu_thread2 sleep\n");
		if (pthread_cond_timedwait(&cond, &mutex, &timeout) == ETIMEDOUT) 
			printf("timeout\n");
		else
			printf("glob_var1 = %d, glob_var2 = %d\n", glob_var1 ,glob_var2);
	}
	pthread_mutex_unlock (&mutex);
	printf("ssu_thread2 end\n");
	exit(1);

}


