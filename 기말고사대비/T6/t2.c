#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>

int glob_var1 = 1, glob_var2 = 2;

void *ssu_thread1 (void *arg);
void *ssu_thread2 (void *arg);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(void) 
{
	pthread_t tid1, tid2;

	pthread_create(&tid1, NULL, ssu_thread1, NULL);
	pthread_create(&tid2, NULL, ssu_thread2, NULL);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);

	exit(0);
}


void *ssu_thread1 (void *arg)
{
	sleep(7);
	glob_var1 = 2;
	glob_var2 = 1;

	if (glob_var1 > glob_var2) {
		pthread_cond_broadcast(&cond);
	}

	printf("ssu_thread1 end\n");
	return NULL;
}
void *ssu_thread2 (void *arg)
{
	struct timespec timeout;
	struct timeval now;
	gettimeofday(&now, NULL);
	pthread_mutex_lock (&lock);
	
	timeout.tv_sec = now.tv_sec + 5 ;
	timeout.tv_nsec = now.tv_usec * 1000;
	
	if (glob_var1 <= glob_var2) {
		printf("ssu_thread2 sleep\n");	
		if (pthread_cond_timedwait(&cond, &lock, &timeout) == ETIMEDOUT)
			printf("timeout\n");
		else
			printf("glob_var1 = %d, glob_var2 = %d\n", glob_var1, glob_var2);

	}
	pthread_mutex_unlock(&lock);
	printf("ssu_thread2 end\n");
	return NULL;
}

