#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <wait.h>
#define BUFSIZ 1024*30


int main(void)
{
    int fd = open("/etc/passwd", O_RDONLY);
    char buf[BUFSIZ] = {0,};
    int pass_length;
    int check_id = 0;

    char BACKUP_PATH[1024] = {0,};

    while ((pass_length = read(fd, buf, BUFSIZ)) > 0 && check_id == 0)
    {
        char* user_name = strstr(buf, "junhyeong");
        if (user_name != NULL)
        {
            char* str = strstr(user_name, "/home");
            char* backup_tok = strstr(str, ":");
            *backup_tok = '\0';
            sprintf(BACKUP_PATH, "%s/backup", str);
            printf("%s\n", BACKUP_PATH);
            check_id = 1;
        }
    }



    return 0;
}
