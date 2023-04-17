#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct _person {
	char name[10];
	int age;
	double height;
} Person;


int main(void)
{
	FILE *fp;
	int i, res;
	Person ary[3] = {{"Hong GD", 500, 175.4}, {"Lee SS", 350, 180.0}, {"King SJ", 500, 178.6}};
	Person tmp;

	int size = sizeof(ary) / sizeof(ary[0]);
	if((fp = fopen("ssu_ftest.txt", "w")) == NULL) {
		fprintf(stderr, "fopen error for ssu_ftest.txt\n");
		exit(1);
	}

	for (i = 0 ; i < size ; i++) {
		fwrite(&ary[i], sizeof(Person), 1, fp);
	}

	fclose(fp);

	printf("[ First print]\n");
	
	if((fp = fopen("ssu_ftest.txt", "r")) == NULL) {
		fprintf(stderr, "fopen error for ssu_ftest.txt");
		exit(1);
	}

	for (i = 0 ; i < size ; i++) {
		fread(&tmp, sizeof(Person), 1, fp);
		printf("%s %d %.2f\n", tmp.name, tmp.age, tmp.height);
	}

	fseek(fp, 0, SEEK_SET);
	printf("[ Second print]\n");
	
	for (i = 0 ; i < size ; i++) {
		fread(&tmp, sizeof(Person), 1, fp);
		printf("%s %d %.2f\n", tmp.name, tmp.age, tmp.height);
	}
	fclose(fp);
	exit(0);

}
