#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif


#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif


char io_option [100];
char io_path [MAXPATHLEN];
char io_str[MAXPATHLEN+50];
//{"자식프로세스 경로", "출력 옵션", "경로", "출력 내용", NULL};
char* token(char* str, char* del);
char* remove_whitespace (char* str); // 사용자로 입력받은 문자열을 return

int main(int argc, char* argv[])
{
    FILE* fp;
    int fd;

    strcpy(io_option, argv[1]);
    strcpy(io_path, argv[2]);
    strcpy(io_str, argv[3]);
    
    if (!strcmp(io_option, "a") || !strcmp(io_option, "w") || !strcmp(io_option, "r"))
    {
        fp = fopen(io_path, io_option);
        fprintf(fp, "%s", io_str);
        fclose(fp);
        exit(0);
    }
    else
    {
        char tmp_tok[10] ={0,};
        int oflag = 0;
        char* tok = token(io_option, "|");
        while (tok != NULL)
        {
            strcpy(tmp_tok, tok);
            char* tok_ptr = remove_whitespace(tmp_tok);
            if (!strcmp(tok_ptr, "O_CREAT"))
                oflag |= O_CREAT;
            else if (!strcmp(tok_ptr, "O_WRONLY"))
                oflag |= O_WRONLY;
            else if (!strcmp(tok_ptr, "O_APPEND"))
                oflag |= O_APPEND;
            else if (!strcmp(tok_ptr, "O_RDONLY"))
                oflag |= O_RDONLY;
            else if (!strcmp(tok_ptr, "O_RDWR"))
                oflag |= O_RDWR;
            else if (!strcmp(tok_ptr, "O_TRUNC"))
                oflag |= O_TRUNC;
            tok = token(NULL, "|");
            memset(tmp_tok, '\0', 10);
        }
        
        fd = open(io_path, oflag, 0644);
        write(fd, io_str, strlen(io_str));
        close(fd);
        exit(0);
    }

    exit(0);
}



/**
 * { "   bubble    " 와 같이 중간 문자열 앞 뒤의 white space를 없애주는 함수.}
 * char* remove_whitespace (char* str : 문자열)
 *  리턴값 : White Space 를 건너뛴 시작 문자열 포인터.
 *  동작원리:
 *      1. 중간 문자열의 앞부분의 공백만큼 포인터 조정
 *      2. 뒷 문자열의 공백이 나타날 때마다 \0 으로 처리. (공백이 아니면 break)
*/
char* remove_whitespace (char* str)
{
    if (str == NULL || *str == '\0')
        return NULL;

    char* start = str;
    char* end = str+strlen(str)-1;

    // "   Insert" 의 앞부분 공백만큼 건너띄는 while
    while (*start != '\0')
        if (*start == ' ')
            start++;
        else
            break;
    
    //*insert    "의 뒷 부분 공백을 \0 으로 만들어버리는 while
    while (end != start)
        if (*end == ' ')
        {
            *end = '\0';
            end--;
        }
        else
            break;
    
    return start;
}


char* token(char* str, char* del)
{
    if (del == NULL)
        return NULL;
    int del_size = strlen(del);
    int i = 0, j = 0;
    static char* ptr1;
    char* ptr2;

    if (str != NULL && ptr1 == NULL)
        ptr1 = str;
    else if (str == NULL && ptr1 == NULL)
        return NULL;

    i = 0;
    while(*ptr1 != '\0' && *ptr1 != '\n')
    {
        for (i = 0 ; i < del_size ; i++)
        {
            if (*ptr1 == del[i])
            {
                ptr1++;
                break;
            }
        }
        if (i >= del_size)
            break;
    }

    ptr2 = ptr1;
    i = 0;
    while(*ptr1 != '\0' && *ptr1 != '\n')
    {
        for (i = 0 ; i < del_size ; i++)
        {
            if (*ptr1 == del[i])
            {
                *ptr1 = '\0';
                break;
            }
        }
        ptr1++;

        if (i < del_size)
            break;
    }

    if (*ptr2 == '\n' || *ptr2 == '\0')
    {
        ptr1 = NULL;
        return NULL;
    }
    
    return ptr2;
}