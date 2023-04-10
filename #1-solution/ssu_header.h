#define OPENSSL_API_COMPAT 0x10100000L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <wait.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#define true 1
#define false 0

#define HASH_MD5  33
#define HASH_SHA1 41

#define NAMEMAX 255
#define PATHMAX 4096
#define STRMAX 4096

#define CMD_ADD			0b0000001
#define CMD_REM			0b0000010
#define CMD_REC			0b0000100
#define CMD_SYS 		0b0001000
#define CMD_HELP		0b0010000
#define CMD_EXIT		0b0100000
#define NOT_CMD			0b0000000

#define OPT_A		0b00001
#define OPT_C		0b00010
#define OPT_D		0b00100
#define OPT_N		0b01000
#define NOT_OPT 0b00000

char exeNAME[PATHMAX];
char exePATH[PATHMAX];
char homePATH[PATHMAX];
char backupPATH[PATHMAX];
int hash;

char *commanddata[10]={
    "add",
    "remove",
    "recover",
    "ls",
    "vi",
    "vim",
    "help",
    "exit"
  };

typedef struct command_parameter {
  char *command;
	char *filename;
	char *tmpname;
	int commandopt;
  char *argv[10];
} command_parameter;

typedef struct backupNode {
  char backupPath[PATHMAX];
  char newPath[PATHMAX];
  struct stat statbuf;

  struct backupNode *next;
} backupNode;

typedef struct fileNode {
  char path[PATHMAX];
  struct stat statbuf;

  backupNode *head;

  struct fileNode *next;
} fileNode;

typedef struct dirNode {
  char path[PATHMAX];
  char backupPath[PATHMAX];
  char newPath[PATHMAX];

  fileNode *head;

  struct dirNode *next;
} dirNode;

typedef struct dirList {
  struct dirNode *head;
  struct dirNode *tail;
} dirList;

dirList *mainDirList;


typedef struct pathList_ {
  struct pathList_ *next;
  struct pathList_ *prev;
  char path[NAMEMAX];

} pathList;

void help();

int md5(char *target_path, char *hash_result)
{
	FILE *fp;
	unsigned char hash[MD5_DIGEST_LENGTH];
	unsigned char buffer[SHRT_MAX];
	int bytes = 0;
	MD5_CTX md5;

	if ((fp = fopen(target_path, "rb")) == NULL){
		printf("ERROR: fopen error for %s\n", target_path);
		return 1;
	}

	MD5_Init(&md5);

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

int cmpHash(char *path1, char *path2) {
  char *hash1 = (char *)malloc(sizeof(char) * hash);
  char *hash2 = (char *)malloc(sizeof(char) * hash);

  ConvertHash(path1, hash1);
  ConvertHash(path2, hash2);

  return strcmp(hash1, hash2);
}

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

char *GetFileName(char file_path[]) {
  char *file_name;
  
  while(*file_path) {
    if(*file_path == '/' && (file_path +1) != NULL) {
    file_name = file_path+1;
    }
    file_path++;
  }
  return file_name;
}

char *strToHex(char* str) {
  char *result = (char *)malloc(sizeof(char) * PATHMAX);
  for(int i = 0; i < strlen(str); i++) {
    sprintf(result+(i*2), "%02X", str[i]);
  }
  result[strlen(str)*2] = '\0';
  
  return result;
}

char *getDate() {
	char *date = (char *)malloc(sizeof(char) * 14);
	time_t timer;
	struct tm *t;

	timer = time(NULL);	
	t = localtime(&timer);

  sprintf(date, "%02d%02d%02d%02d%02d%02d",t->tm_year %100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
  
  return date;
}

char *strTodate(char *old_str, char **date) {
	char *result = (char *)calloc(sizeof(char), PATHMAX);
	*date = getDate();

  sprintf(result, "%s_%s", old_str, *date);
	
	return result;
}

char *strTodirPATH(char *dirpath, char *filepath) {
  char *fullpath = (char *)malloc(sizeof(char) * PATHMAX);
	strcpy(fullpath, dirpath);
	strcat(fullpath,"/");
	if(filepath != NULL)
		strcat(fullpath,filepath);
	return fullpath;
}

char *QuoteCheck(char **str, char del) {
  char *tmp = *str+1;
  int i = 0;

  while(*tmp != '\0' && *tmp != del) {
    tmp++;
    i++;
  }
  if(*tmp == '\0') {
    *str = tmp;
    return NULL;
  }
  if(*tmp == del) {
    for(char *c = *str; *c != '\0'; c++) {
      *c = *(c+1);
    }
    *str += i;
    for(char *c = *str; *c != '\0'; c++) {
      *c = *(c+1);
    }
  }
}

char *Tokenize(char *str, char *del) {
  int i = 0;
  int del_len = strlen(del);
  static char *tmp = NULL;
  char *tmp2 = NULL;

  if(str != NULL && tmp == NULL) {
    tmp = str;
  }

  if(str == NULL && tmp == NULL) {
    return NULL;
  }

  char *idx = tmp;

  while(i < del_len) {
    if(*idx == del[i]) {
      idx++;
      i = 0;
    } else {
      i++;
    }
  }
  if(*idx == '\0') {
    tmp = NULL;
    return tmp;
  }
  tmp = idx;

  while(*tmp != '\0') {
    if(*tmp == '\'' || *tmp == '\"') {
      QuoteCheck(&tmp, *tmp);
      continue;
    }
    for(i = 0; i < del_len; i++) {
      if(*tmp == del[i]) {
        *tmp = '\0';
        break;
      }
    }
    tmp++;
    if(i < del_len) {
      break;
    }
  }

  return idx;
}

char **GetSubstring(char *str, int *cnt, char *del) {
  *cnt = 0;
  int i = 0;
  char *token = NULL;
  char *templist[100] = {NULL, };
  token = Tokenize(str, del);
  if(token == NULL) {
    return NULL;
  }

  while(token != NULL) {
    templist[*cnt] = token;
    *cnt += 1;
    token = Tokenize(NULL, del);
  }
  
	char **temp = (char **)malloc(sizeof(char *) * (*cnt + 1));
	for (i = 0; i < *cnt; i++) {
		temp[i] = templist[i];
	}
	return temp;
}

int ConvertPath(char* origin, char* resolved) {
  int idx = 0;
  int i;
  char *path = (char *)malloc(sizeof(char *) * PATH_MAX);
  char *tmppath = (char *)malloc(sizeof(char *) * PATH_MAX);
  char **pathlist;
  int pathcnt;

  if(origin == NULL) {
    return -1;
  }

  if(origin[0] == '~') {
    sprintf(path, "%s%s", homePATH, origin+1);
  } else if(origin[0] != '/') {
    sprintf(path, "%s/%s", exePATH, origin);
  } else {
    sprintf(path, "%s", origin);
  }

  if(!strcmp(path, "/")) {
    resolved = "/";
    return 0;
  }

  if((pathlist = GetSubstring(path, &pathcnt, "/")) == NULL) {
    return -1;
  }

  pathList *headpath = (pathList *)malloc(sizeof(pathList));
  pathList *currpath = headpath;

  for(i = 0; i < pathcnt; i++) {
    if(!strcmp(pathlist[i], ".")) {
      continue;
    } else if(!strcmp(pathlist[i], "..")) {
      currpath = currpath->prev;
      currpath->next = NULL;
      continue;
    }

    pathList *newpath = (pathList *)malloc(sizeof(pathList));
    strcpy(newpath->path, pathlist[i]);
    currpath->next = newpath;
    newpath->prev = currpath;

    currpath = currpath->next;
  }

  currpath = headpath->next;

  strcpy(tmppath, "/");
  while(currpath != NULL) {
    strcat(tmppath, currpath->path);
    if(currpath->next != NULL) {
      strcat(tmppath, "/");
    }
    currpath = currpath->next;
  }

  strcpy(resolved, tmppath);

  return 0;
}

int cmpPath(char *path1, char *path2) {
  int i;
  int cnt1, cnt2;
  char tmp1[PATHMAX], tmp2[PATHMAX];
  strcpy(tmp1, path1);
  strcpy(tmp2, path2);
  char **pathlist1 = GetSubstring(tmp1, &cnt1, "/");
  char **pathlist2 = GetSubstring(tmp2, &cnt2, "/");

  if(cnt1==cnt2) {
    for(i = 0; i < cnt1; i++) {
      if(!strcmp(pathlist1[i], pathlist2[i])) continue;
      return -strcmp(pathlist1[i], pathlist2[i]);
    }
  } else {
    return cnt1 < cnt2;
  }
  return 1;
}