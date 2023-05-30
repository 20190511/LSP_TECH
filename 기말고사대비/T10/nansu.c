#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_RANDOM_NUMBERS 500000

int main(int argc, char* argv[]) {
	int i;
	FILE *file;
	srand(time(NULL)); // 시간을 기반으로 시드(seed)를 초기화

	file = fopen(argv[1], "w");
	if (file == NULL) {
		printf("파일을 열 수 없습니다.");
		return 1;
	}

	for (i = 0; i < NUM_RANDOM_NUMBERS; i++) {
		int random_number = rand() % 10;
		fprintf(file, "%d", random_number);
	}

	fclose(file);
	printf("난수 생성이 완료되었습니다.\n");

	return 0;
}

