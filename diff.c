/*
DIFF 함수는 결국 줄단위 문자열 LCS 로 생각되어질 수 있다.
LCS는 전형적인 2차원 DP 테이블로 한 번 구현해보자
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRMAX  512
void example1 ();
void string_LCS(char a[][STRMAX], char b[][STRMAX], int sizex, int sizey);
char* analysis (char **item, int sizex, int sizey);
void diffs (char a[][STRMAX], char b[][STRMAX], char* rule);

int main(void)
{
    example1();

    return 0;
}


/**
 *  s가 나올떄까지 rule을 선형적으로 진행시킨다 
 *  다음의 규칙성을 따른다.
 * 
 *  s : same, (r:교체, a:추가, d:삭제)
 *  1. s를 만나면 앞에 기록된 문자열들을 모두 출력한다.
 *  2. s를 만나기 전까지 r이 하나라도 끼어있으면 해당 문자열들은 모두 교체시킨다.
 *  3. s를 만나기 전까지 r이 하나도 없으면 해당 문자열들은 a,d 중 하나로 처리된다.
 * 
 *  해당 문자열은 그대로 a,b를 사용하되 인덱스만 잘 조절하여 사용한다.
 */ 
void diffs (char a[][STRMAX], char b[][STRMAX], char* rule)
{
    
    int len_rule = strlen(rule);
    // a,b 인덱스 범위 조절 (해당 인덱스[0] ~ [1] 까지 라인 출력)
    int idx_a[2] = {0, 0};
    int idx_b[2] = {0, 0};
    int r_check = 0;
    for (int i = 0 ; i < len_rule ; i++)
    {
        
        //시작점을 s다음으로 설정하고 끝나는지점을 시작지점으로 설정한다.
        if (rule[i] == 's') 
        {
            idx_a[0]++;
            idx_b[0]++; 
            idx_a[1] = idx_a[0];
            idx_b[1] = idx_b[0];
            continue;
        }
        if (rule[i] == 'r')
        {
            r_check = 1;  //r이 하나라도 나왔는지 체크
            idx_a[1]++;   //현재의 다음 인덱스를 늘려줌.
            idx_b[1]++;
        }
        if (rule[i] == 'a') //b를 a에 추가해줘야되니까 b증가
        {
            idx_b[1]++;
        }
        if (rule[i] == 'd') //a를 삭제 해줘야되니까 a증가
        {
            idx_a[1]++;
        }
        

        if((i+1 == len_rule) || rule[i+1] == 's')
        {
            char a_line[20];
            char b_line[20];
            // 라인 출력 함수
            if (idx_a[1] - idx_a[0] <= 1)
                sprintf(a_line, "%d", idx_a[0]+1);
            else
                sprintf(a_line, "%d,%d", idx_a[0]+1, idx_a[1]);

            if (idx_b[1] - idx_b[0] <= 1)
                sprintf(b_line, "%d", idx_b[0]+1);
            else
                sprintf(b_line, "%d,%d", idx_b[0]+1, idx_b[1]);

            if (!r_check)
            {
                if (rule[i] == 'a')
                {
                    printf("%da%s :\n", a_line[0]+1, b_line);
                    for(int o = idx_b[0] ; o < idx_b[1] ; o++)
                    {
                        printf("> %s\n", b[o]);
                    }
                    idx_b[0] = idx_b[1]; //출력했으니까 시작점 현재위치 다음으로 이동.
                }

                if (rule[i] == 'd')
                {
                    printf("%sd%d :\n", a_line, idx_b[0]+1);
                    for(int o = idx_a[0] ; o < idx_a[1] ; o++)
                    {
                        printf("< %s\n", a[o]);
                    }
                    idx_a[0] = idx_a[1];
                }
            }
            else
            {
                printf("%sc%s :\n", a_line, b_line);
                for(int o = idx_a[0] ; o < idx_a[1] ; o++)
                {
                    printf("< %s\n", a[o]);
                }  
                printf(" --- \n");
                for(int o = idx_b[0] ; o < idx_b[1] ; o++)
                {
                    printf("> %s\n", b[o]);
                }              
                idx_a[0] = idx_a[1];
                idx_b[0] = idx_b[1];
            }
            r_check = 0;
        }


    }
}


char* analysis (char **item, int sizex, int sizey)
{

    // d : del, a : append, r : replace, s :same
    int str_max = sizex >= sizey ? sizex : sizey;
    str_max *= 2;
    char *result = (char*)malloc(sizeof(char) * str_max);
    int idx_x = sizex;
    int idx_y = sizey;
    int result_idx = 0;  // 경로의 최종 길이가 될 것.
    
    while (idx_x != 0 || idx_y != 0)
    {
        int curr = item[idx_x][idx_y];
        if (idx_x == 0 && idx_y == 0)
        {
            result[result_idx++] = item[idx_x+1][idx_y+1];
            break;
        }
        else if (idx_x == 0 && idx_y != 0)
        {
            result[result_idx++] = 'a';
            idx_y--;
            continue;
        }
        else if (idx_x != 0 && idx_y == 0)
        {
            result[result_idx++] = 'd';
            idx_x--;
            continue;
        }
        switch(curr)
        {
            case 's':
                result[result_idx++] = 's';
                idx_x--;
                idx_y--;
                break;
            case 'r':
                result[result_idx++] = 'r';
                idx_x--;
                idx_y--;
                break;
            case 'd':
                result[result_idx++] = 'd';
                idx_x--;
                break;
            case 'a':
                result[result_idx++] = 'a';
                idx_y--;
                break;
        }
    }
    

    //경로가 반대로 되어있으니 다시 reverse
    char* final_path = (char*)calloc(result_idx, sizeof(char));
    for (int i = 0 ; i < result_idx ; i++)
    {
        final_path[i] = result[result_idx-i-1];
    }
    free (result);
    
    //reverse 값 확인 완료
    //printf("%s\n", final_path);
    return final_path; //dp 경로 문자열 
}


void string_LCS(char a[][STRMAX], char b[][STRMAX], int sizex, int sizey)
{
    int** dp_list;
    char** char_list;
    dp_list = (int**)calloc(sizex+1,sizeof(int*));
    char_list = (char**)calloc(sizex+1, sizeof(char*));
    for (int i = 0 ; i < sizex+1 ; i++)
    {
        dp_list[i] = (int*)calloc(sizey+1, sizeof(int));
        char_list[i] = (char*)calloc(sizey+1, sizeof(char));
    }

    for (int i = 1 ; i < sizex+1 ; i++)
    {
        for (int j = 1 ; j < sizey+1 ; j++)
        {
            if (strcmp(a[i-1], b[j-1]) == 0)
            {
                dp_list[i][j] = dp_list[i-1][j-1] + 1;
                char_list[i][j] = 's';
            }
            else
            {
                int replace = dp_list[i-1][j-1];
                int del = dp_list[i-1][j];
                int append = dp_list[i][j-1];
                
                /*
                int max_num = replace < del ? del : replace;
                max_num = max_num < append ? append : max_num;
                dp_list[i][j] = max_num;
                */
               if (append >= del && append >= replace)
               {
                    dp_list[i][j] = append;
                    char_list[i][j] = 'a';
               }
               if (del >= append && del >= replace)
               {
                    dp_list[i][j] = del;              
                    char_list[i][j] = 'd';
               }
               if (replace >= del && replace >= append)
               {
                    dp_list[i][j] = replace;
                    char_list[i][j] = 'r';            
               }
            }
        }
    }

    char* string_rule = analysis(char_list, sizex, sizey);
    printf("%s length is %ld\n", string_rule, strlen(string_rule));
    diffs(a,b,string_rule);

    for (int i = 0 ; i < sizex+1 ; i++)
        free(dp_list[i]);
    free(dp_list);
}


void example1 ()
{
    char a[24][STRMAX] ={
        "This part of the",
        "document has stayed the",
        "same from version to",
        "version. It shouldn't",
        "be shown if it doesn't",
        "change. Otherwise, that",
        "would not be helping to",
        "compress the size of the",
        "changes",
        "",
        "This paragraph contains",
        "text that is outdated",
        "It will be deleted in the",
        "near future.",
        "",
        "It is important to spell",
        "check this dokument. On",
        "the other hand, a",
        "misspelled word isn't",
        "the end of the world.",
        "Nothing in the rest of",
        "this paragraph needs to",
        "be changed. Things can",
        "be added after it."
    };
    
    char b[25][STRMAX] =
    {
        "This is an important",
        "notice! It should be",
        "located at this document!",
        "",
        "This part of the",
        "document has stayed the",
        "same from version to",
        "version. It shouldn't",
        "be shown if it doesn't",
        "change. Otherwise, that",
        "would not be helping to",
        "compress anything",
        "",
        "It is important to spell",
        "check this document. On",
        "the other hand, a",
        "misspelled word isn't",
        "the end of the world.",
        "Nothing in the rest of",
        "this paragraph needs to",
        "be changed.",
        "",
        "This paragraph contains",
        "important additions",
        "to this document"
    };
    string_LCS(a,b,24,25);
}
/**
This part of the
document has stayed the
same from version to
version. It shouldn't
be shown if it doesn't
change. Otherwise, that
would not be helping to
compress the size of the
changes

This paragraph contains
text that is outdated
It will be deleted in the
near future.

It is important to spell
check this dokument. On
the other hand, a
nisspelled word isn't
the end of the world.
Nothing in the rest of
this paragraph needs to
be changed. Things can
be added after it.


------
This is an important
notice! It should be
located at this document!
This part of the
document has stayed the
same from version to
version.
It shouldn't
be shown if it doesn't
change. Otherwise, that
would not be helping to
compress anything

It is important to spell
check this document. On
the other hand, a
misspelled word isn't
the end of the world
Nothing in the rest of
this paragraph needs to
be changed.

This paragraph contains
important additions
this document


*/
