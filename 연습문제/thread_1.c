#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define N 4

int matrix1 [N][N] = {{1, 2, 3, 4}, {2, 3, 4, 5}, {3, 4, 5 ,6}, {4, 5, 6, 7}};
int matrix2 [N][N] = {{7, 6, 5, 4}, {6, 5, 4, 3}, {5, 4, 3, 2}, {4, 3, 2, 1}};

struct arg{
    int x;
    int y;
};

void *ssu_thread(void *arg) {
    struct arg str_arg = *(struct arg*)arg;
    int result;
    int i;

    result = 0;
    for (i = 0 ; i < N ; i++)
        result += matrix1[str_arg.x][i] * matrix2[i][str_arg.y];

    return (void*)result; // result를 주솟값 형태로 전달하고 있음 (50 -> 0x32) (기억)
}


int main(void)
{
    struct arg arg[N][N];
    pthread_t tid[N][N];
    int result [N][N];
    int  i, j;

    for (i = 0 ; i < N ; i++)
        for (j = 0 ; j < N ; j++) {
            arg[i][j].x = i;
            arg[i][j].y = j;


            if (pthread_create(&tid[i][j], NULL, ssu_thread, (void*)&arg[i][j]) != 0) {
                fprintf(stderr, "pthread_create error\n");
                exit(1);
            }
        }

// (void*)&result[i][j] 를 하면 result[i][j] 주솟값을 void* 바꾸는거고, (void**)&result[i][j] 를 하면 result[i][j] 공간을 (void*) 로 변환한 주솟공간에 (void*) 데이터가 들어감.
    for (i = 0 ; i < N ; i++)
        for(j = 0 ; j < N ; j++)
            pthread_join(tid[i][j], (void**)&result[i][j]); 

    for (i = 0 ; i < N ; i++) {
        for (j = 0 ; j < N ; j++)
            printf("%d ", result[i][j]); // 해당 데이터를 정숫값으로 나타내고있음.
        printf("\n");
    }
}