#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#define BUFSIZE	1024*16

void do_fp(FILE *f, int opt);
void pt(unsigned char *md);

int main(int argc, char **argv)
{
	int i;
	FILE *IN;

	if (argc == 1)
	{
		do_fp(stdin, 2);
	}
	else
	{
		IN=fopen(argv[1],"r");
		if (IN == NULL)
		{
			perror(argv[1]);
			exit(1);
		}
		do_fp(IN, atoi(argv[2]));
		fclose(IN);
	}
	exit(0);
}

void do_fp(FILE *f, int opt)							//option 0 :md5, 1: sha1
{
SHA_CTX hash_buf;
unsigned char md[SHA_DIGEST_LENGTH];
int fd;
int i;
unsigned char buf[BUFSIZE];

fd=fileno(f);
opt==0 ? MD5_Init(&hash_buf) : SHA1_Init(&hash_buf);	// 필수! 처음 해싱할 때 생성해줘야함. (삼항연산 이용해서 사용.)
for (;;)
	{
	i=read(fd,buf,BUFSIZE);
	if (i <= 0) break;
	// 읽은 개수만큼 buf의 값을 해싱해서 c에 저장해줌. (반복적으로 호출가능)
	opt==0 ? MD5_Update(&hash_buf, buf,(unsigned long)i) : SHA1_Update(&hash_buf, buf,(unsigned long)i);		
	}
	// 받아온 해시 구조체를 -> md문자열에 바로저장.
opt==0 ? MD5_Final(md,&hash_buf) : SHA1_Final(md,&hash_buf);								
pt(md);
}


// 해시 출력함수
void pt(unsigned char *md)
{
int i;
// printf("test = %s\n",md); 						// 다음과 같이 사용불가. (16진수라서 잘못들어가는거 같음)
char string_buf[41] = {0,};
char onc_code[3];
for (i=0; i<SHA_DIGEST_LENGTH; i++)					// 부호없는 16진수로 표기.
{
	sprintf(onc_code, "%02x", md[i]);
	strcat(string_buf,onc_code);
}
printf("%s\n", string_buf);
printf("hash size is %ld\n", strlen(string_buf));
}
/**
 * junhyeong@DESKTOP-UPFPK8Q:~/go2$ ./hash_example diff.c 1			// 1: sha1
	83eba35f13c8f33a7bd40e6f3194bab14091a461
	hash size is 40

	junhyeong@DESKTOP-UPFPK8Q:~/go2$ ./hash_example diff.c 0		// 0 :md5, 
	29d6c16dacf3a7617f97204978cfce2c00000000
	hash size is 40
*/
