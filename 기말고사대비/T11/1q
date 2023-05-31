#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define N 4
int matrix1 [N][N] = {{1,2,3,4}, {2,3,4,5}, {3,4,5,6}, {4,5,6,7}};
int matrix2 [N][N] = {{7,6,5,4}, {6,5,4,3}, {5,4,3,2}, {4,3,2,1}};

struct arg {
	int x;
	int y;
};

void *ssu_thread (void* arg);

int main(void)
{
	int i,j;
	pthread_t tid[i][j];
	struct arg args[N][N];
	int result [N][N] = {0,};
	
	for (i = 0 ; i < N ; i++) {
		for (j = 0 ; j < N ; j++) {
			args[i][j].x = i;
			args[i][j].y = j;

			if (pthread_create(&tid[i][j], NULL, ssu_thread, (void*)&args[i][j]) < 0) {
				fprintf(stderr, "pthread_create error\n");
				exit(1);
			}
		}
	}

	for (i = 0 ;i < N ;i++) {
		for (j = 0 ; j < N ; j++) {
			if (pthread_join(tid[i][j], (void**)&result[i][j]) < 0) {
				fprintf(stderr, "pthread_join error\n");
				exit(1);
			}
		}
	}

	for (i = 0 ; i < N ; i++) {
		for (j = 0 ; j < N ; j++) {
			printf("%d ", result[i][j]);
		}
		printf("\n");
	}

	exit(0);
}

void *ssu_thread (void* arg)
{
	int result;
	int i;
	struct arg index = *((struct arg*) arg);

	result = 0;
	for (i = 0 ; i < N ; i++) {
		result += matrix1[index.x][i] * matrix2[i][index.y];
	}

	return (void*)result;
}

