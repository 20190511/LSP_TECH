#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define THREAD_NUM 8

struct thread_data {
	int thread_index;
	int sum;
	char *message;
};

void *ssu_printhello(void* arg); //thread에 넣을 함수

struct thread_data thread_data_array[THREAD_NUM];
char* messages[THREAD_NUM];

int main(void)
{
	pthread_t tid[THREAD_NUM];
	int sum;
	int i;

	sum = 0;
	messages[0] = "English: Hello World!";
	messages[1] = "French: Bonjour, le monde!";
	messages[2] = "Spanish: Hola al mundo";
	messages[3] = "Klingon: Nuq neH!";
	messages[4] = "German: Guten Tag, Welt!";
	messages[5] = "Russian: Zdravstvytye, mir!";
	messages[6] = "Japan: Sekai e konnichiwa!";
	messages[7] = "Latin: Orbis, te saluto!";

	for (i = 0 ; i < THREAD_NUM ; i++) {
		sum = sum + i;
		thread_data_array[i].thread_index = i;
		thread_data_array[i].sum = sum;
		thread_data_array[i].message = messages[i];
		printf("Creating thread %d\n", i);
		
		//ssu_printhello 에 thread_data_array 구조체를 call by reference 로 보내고 있음
		//thread_data_array[i] 는 pointor 가 아니므로 주솟값으로 전달해줄 것
		// 같은 이치로 tid 는 포인터지만, tid[i]는 포인터가 아니므로 &tid[i] 로 해줘야함
		if (pthread_create(&tid[i], NULL, ssu_printhello, (void*)&thread_data_array[i]) != 0) {
			fprintf(stderr, "pthread_create error\n");
			exit(1);
		}
	}

	sleep(5);
	exit(0);
}

void *ssu_printhello(void* arg) {
	struct thread_data *data;
	char *message;
	int thread_index;
	int sum;

	sleep(1);
	data = (struct thread_data *)arg; //void* 로 캐스팅해서 보내서 다시 구조체로 캐스팅
	thread_index = data->thread_index;
	sum = data->sum;
	message = data->message;
	printf("Thread %d: %s Sum=%d\n", thread_index, message, sum);
	return NULL;
}




