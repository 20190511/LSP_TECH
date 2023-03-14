#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

char *filename; // 전역 변수로 filename 선언

int filter(const struct dirent *entry) {
    if (strstr(entry->d_name, filename) != NULL) {
        return 1;
    }
    return 0;
}

int main(void) {
    struct dirent **namelist;
    int n;

    filename = "example"; // filename에 검색할 부분 문자열 설정
    n = scandir(".", &namelist, filter, alphasort);
    if (n == -1) {
        perror("scandir");
        exit(EXIT_FAILURE);
    }
    while (n--) {
        printf("%s\n", namelist[n]->d_name);
        free(namelist[n]);
    }
    free(namelist);
    exit(EXIT_SUCCESS);
}
