	1. 이진수로 옵션 표현해서 | 사용해서 옵션 연결하였음.
	2. scandir 사용법
	
	struct dirent **namelist;
	if((cnt = scandir(filepath, &namelist, NULL, alphasort)) == -1) {
	    fprintf(stderr, "ERROR: scandir error for %s\n", filepath);
	    return 1;
	  }
	3. 시간 구하기
		a. >> time_t 구조체를 localtime 에 넣으면 tm_year, tm_mon, tm_mday
		b. 
 	
	
	
	>> time.h 를 정의해야함
	time_t 를 localtime 인자로 넣으면 tm_t 구조체를 뱉어냄
	        --> tm_sec, tm_min, tm_mon 이런 변수를 사용가능.
	                (대신 year 은 %100 해줘야하고, mon +1 해줘야함)
	
	1. 
	
	
	char *getDate() {
	
	  char *date = (char *)malloc(sizeof(char) * 14);
	  time_t timer;
	  struct tm *t;
	  timer = time(NULL); 
	  t = localtime(&timer);
	  sprintf(date, "%02d%02d%02d%02d%02d%02d",t->tm_year %100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	  
	  return date;
	}
	
	
	2. 해시 구하기 (문자열을 16진수 바꾸는 sprintf 테크닉 잘보기)
	
	#include <openssl/md5.h>
	#include <openssl/sha.h>
	
	int md5(char *target_path, char *hash_result)
	{
	  FILE *fp;
	  unsigned char hash[MD5_DIGEST_LENGTH];
	  unsigned char buffer[SHRT_MAX];
	  int bytes = 0;
	  MD5_CTX md5;    // ctx 넣어주고.
	  if ((fp = fopen(target_path, "rb")) == NULL){
	    printf("ERROR: fopen error for %s\n", target_path);
	    return 1;
	  }
	  MD5_Init(&md5);   // md5 를 초기화시켜주고
	  while ((bytes = fread(buffer, 1, SHRT_MAX, fp)) != 0)
	    MD5_Update(&md5, buffer, bytes);
	  
	  MD5_Final(hash, &md5);
	  for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
	    sprintf(hash_result + (i * 2), "%02x", hash[i]);
	  hash_result[HASH_MD5-1] = 0;
	  fclose(fp);
	  return 0;
	}
	
	int sha1(char *target_path, char *hash_result)
	{
	  FILE *fp;
	  unsigned char hash[SHA_DIGEST_LENGTH];
	  unsigned char buffer[SHRT_MAX];
	  int bytes = 0;
	  SHA_CTX sha1;
	  if ((fp = fopen(target_path, "rb")) == NULL){
	    printf("ERROR: fopen error for %s\n", target_path);
	    return 1;
	  }
	  SHA1_Init(&sha1);
	  while ((bytes = fread(buffer, 1, SHRT_MAX, fp)) != 0)
	    SHA1_Update(&sha1, buffer, bytes);
	  
	  SHA1_Final(hash, &sha1);
	  for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
	    sprintf(hash_result + (i * 2), "%02x", hash[i]);
	  hash_result[HASH_SHA1-1] = 0;
	  fclose(fp);
	  return 0;
	}
	int ConvertHash(char *target_path, char *hash_result) {
	  if(hash == HASH_MD5) {
	    md5(target_path, hash_result);
	  } else {
	    sha1(target_path, hash_result);
	  }
	}
	3. getopt 사용방법
	4. 3자리마다 , 처리
	
	char *cvtNumComma(int a) {
	  char *str = (char *)malloc(sizeof(char) * STRMAX);
	  char *ret = (char *)malloc(sizeof(char) * STRMAX);
	  int i;
	  for(i = 0; a > 0; i++) {
	    str[i] = a%10 + '0';
	    a /= 10;
	    if(i%4 == 2) {
	      i++;
	      str[i] = ',';
	    }
	  }
	  str[i] = '\0';
	  for(i = 0; i < strlen(str); i++) {
	    ret[i] = str[strlen(str)-i-1];
	  }
	  ret[i] = '\0';
	  
	  return ret;
	}
	
	
	5. 파일 이름을 16진수화 시키는 방법
	
	//파일 이름 16진수화
	char *strToHex(char* str) {
	  char *result = (char *)malloc(sizeof(char) * PATHMAX);
	  for(int i = 0; i < strlen(str); i++) {
	    sprintf(result+(i*2), "%02X", str[i]);
	  }
	  result[strlen(str)*2] = '\0';
	  
	  return result;
	}
	6. pid 사용방법 + execl 사용방법
	void SystemExec(char **arglist) {
	  pid_t pid;
	  char whichPath[PATHMAX];
	  sprintf(whichPath, "/usr/bin/%s", arglist[0]);
	  if((pid = fork()) < 0) {
	    fprintf(stderr, "ERROR: fork error\n");
	    exit(1);
	  } else if(pid == 0) {
	    execv(whichPath, arglist);
	    exit(0);
	  } else {
	    pid = wait(NULL);
	  }
	}
	
	
	#2 execv 를 쓸 때는 argv 마지막을 NULL 로 둔다!
	void CommandExec(command_parameter parameter) {
	  pid_t pid;
	  parameter.argv[0] = "command";
	  parameter.argv[1] = (char *)malloc(sizeof(char *) * 32);
	  sprintf(parameter.argv[1], "%d", hash);
	  parameter.argv[2] = parameter.command;
	  parameter.argv[3] = parameter.filename;
	  parameter.argv[4] = parameter.tmpname;
	  parameter.argv[5] = (char *)malloc(sizeof(char *) * 32);
	  sprintf(parameter.argv[5], "%d", parameter.commandopt);
	  parameter.argv[6] = (char *)0;
	  if((pid = fork()) < 0) {
	    fprintf(stderr, "ERROR: fork error\n");
	    exit(1);
	  } else if(pid == 0) {
	    execv(exeNAME, parameter.argv);
	    exit(0);
	  } else {
	    pid = wait(NULL);
	  }
	}
	
	
