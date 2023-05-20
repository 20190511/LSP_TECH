#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

void *ssu_thread(void *arg);

int main(void)
{
	pthread_t tid;
	pid_t pid;

	if (pthread_create(&tid, NULL, ssu_thread, NULL) != 0) {
		fprintf(stderr, "pthread create error\n");
		exit(1);
	}

	pid = getpid(); // pid 받아오기
	tid = pthread_self(); //main thread tid 받아오기
	printf("Main Thread: pid %u tid %u \n",
			(unsigned int)pid, (unsigned int)tid);
	sleep(1);

	exit(0);
}

void *ssu_thread(void *arg) {
	pthread_t tid;
	pid_t pid;

	pid = getpid();
	tid = pthread_self();
	printf("New Thread: pid %d tid %u\n", (int)pid, (unsigned int)tid);
	return NULL;
}

