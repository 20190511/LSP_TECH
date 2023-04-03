#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include "blank.h"

#define MAIN_H_

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef STDOUT
	#define STDOUT 1
#endif
#ifndef STDERR
	#define STDERR 2
#endif
#ifndef TEXTFILE
	#define TEXTFILE 3
#endif
#ifndef CFILE
	#define CFILE 4
#endif
#ifndef OVER
	#define OVER 5
#endif
#ifndef WARNING
	#define WARNING -0.1
#endif
#ifndef ERROR
	#define ERROR 0
#endif

#define FILELEN 128
#define BUFLEN 1024
#define SNUM 100
#define QNUM 100
#define ARGNUM 5

struct ssu_scoreTable{
	char qname[FILELEN];
	double score;
};

void ssu_score(int argc, char *argv[]);
int check_option(int argc, char *argv[]);
void print_usage();

void score_students();
double score_student(int fd, char *id);
void write_first_row(int fd);

char *get_answer(int fd, char *result);
int score_blank(char *id, char *filename);
double score_program(char *id, char *filename);
double compile_program(char *id, char *filename);
int execute_program(char *id, char *filname);
pid_t inBackground(char *name);
double check_error_warning(char *filename);
int compare_resultfile(char *file1, char *file2);

void do_iOption(char (*ids)[FILELEN]);
void do_mOption();
int is_exist(char (*src)[FILELEN], char *target);

int is_thread(char *qname);
void redirection(char *command, int newfd, int oldfd);
int get_file_type(char *filename);
void rmdirs(const char *path);
void to_lower_case(char *c);

void set_scoreTable(char *ansDir);
void read_scoreTable(char *path);
void make_scoreTable(char *ansDir);
void write_scoreTable(char *filename);
void set_idTable(char *stuDir);
int get_create_type();

void sort_idTable(int size);
void sort_scoreTable(int size);
void get_qname_number(char *qname, int *num1, int *num2);


extern struct ssu_scoreTable score_table[QNUM];
extern char id_table[SNUM][10];

struct ssu_scoreTable score_table[QNUM];
char id_table[SNUM][10];

char stuDir[BUFLEN];
char ansDir[BUFLEN];
char errorDir[BUFLEN];
char threadFiles[ARGNUM][FILELEN];
char iIDs[ARGNUM][FILELEN];

int eOption = false;
int tOption = false;
int mOption = false;
int iOption = false;

void ssu_score(int argc, char *argv[])
{
	char saved_path[BUFLEN];
	int i;

	for(i = 0; i < argc; i++){
		if(!strcmp(argv[i], "-h")){
			print_usage();
			return;
		}
	}

	memset(saved_path, 0, BUFLEN);
	if(argc >= 3 && strcmp(argv[1], "-i") != 0){
		strcpy(stuDir, argv[1]);
		strcpy(ansDir, argv[2]);
	}

	if(!check_option(argc, argv))
		exit(1);

	if(!mOption && !eOption && !tOption && iOption 
			&& !strcmp(stuDir, "") && !strcmp(ansDir, "")){
		do_iOption(iIDs);
		return;
	}

	getcwd(saved_path, BUFLEN);

	if(chdir(stuDir) < 0){
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}
	getcwd(stuDir, BUFLEN);

	chdir(saved_path);
	if(chdir(ansDir) < 0){
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN);

	chdir(saved_path);

	set_scoreTable(ansDir);
	set_idTable(stuDir);

	if(mOption)
		do_mOption();

	printf("grading student's test papers..\n");
	score_students();

	if(iOption)
		do_iOption(iIDs);

	return;
}

int check_option(int argc, char *argv[])
{
	int i, j, k;
	int c;
	int exist = 0;

	while((c = getopt(argc, argv, "e:thmi")) != -1)
	{
		switch(c){
			case 'e':
				eOption = true;
				strcpy(errorDir, optarg);

				if(access(errorDir, F_OK) < 0)
					mkdir(errorDir, 0755);
				else{
					rmdirs(errorDir);
					mkdir(errorDir, 0755);
				}
				break;
			case 't':
				tOption = true;
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){

					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else{
						strcpy(threadFiles[j], argv[i]);
					}
					i++; 
					j++;
				}
				break;
			case 'm':
				mOption = true;
				break;

			case 'i':
				iOption = true;
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){
					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded. :: %s\n", argv[i]);
					else
						strcpy(iIDs[j], argv[i]);
					i++;
					j++;
				}
				break;

			case '?':
				printf("Unkown option %c\n", optopt);
				return false;
		}
	}

	return true;
}

void do_iOption(char (*ids)[FILELEN])
{
	FILE *fp;
	char tmp[BUFLEN];
	char qname[QNUM][FILELEN];
	char *p, *id;
	int i, j;
	char first, exist;

	if((fp = fopen("./score.csv", "r")) == NULL){
		fprintf(stderr, "score.csv file doesn't exist\n");
		return;
	}

	// get qnames
	i = 0;
	fscanf(fp, "%s\n", tmp);
	strcpy(qname[i++], strtok(tmp, ","));
	
	while((p = strtok(NULL, ",")) != NULL)
		strcpy(qname[i++], p);

	// print result
	i = 0;
	while(i++ <= ARGNUM - 1)
	{
		exist = 0;
		fseek(fp, 0, SEEK_SET);
		fscanf(fp, "%s\n", tmp);

		while(fscanf(fp, "%s\n", tmp) != EOF){
			id = strtok(tmp, ",");

			if(!strcmp(ids[i - 1], id)){
				exist = 1;
				j = 0;
				first = 0;
				while((p = strtok(NULL, ",")) != NULL){
					if(atof(p) == 0){
						if(!first){
							printf("%s's wrong answer :\n", id);
							first = 1;
						}
						if(strcmp(qname[j], "sum"))
							printf("%s    ", qname[j]);
					}
					j++;
				}
				printf("\n");
			}
		}

		if(!exist)
			printf("%s doesn't exist!\n", ids[i - 1]);
	}

	fclose(fp);
}

void do_mOption(char *ansDir)
{
	double newScore;
	char modiName[FILELEN];
	char filename[FILELEN];
	char *ptr;
	int i;

	ptr = malloc(sizeof(char) * FILELEN);

	while(1){

		printf("Input question's number to modify >> ");
		scanf("%s", modiName);

		if(strcmp(modiName, "no") == 0)
			break;

		for(i=0; i < sizeof(score_table) / sizeof(score_table[0]); i++){
			strcpy(ptr, score_table[i].qname);
			ptr = strtok(ptr, ".");
			if(!strcmp(ptr, modiName)){
				printf("Current score : %.2f\n", score_table[i].score);
				printf("New score : ");
				scanf("%lf", &newScore);
				getchar();
				score_table[i].score = newScore;
				break;
			}
		}
	}

	sprintf(filename, "./%s", "score_table.csv");
	write_scoreTable(filename);
	free(ptr);

}

char* get_header_char(char *header, int idx) // 인덱스 기준 파일명 파싱(temp: X.c | X-X.txt)
{
	char *temp = (char *)calloc(BUFLEN, sizeof(char));
	int i = 0;
	while(header[idx] != ',')
		temp[i++] = header[idx++];
	return temp;
}

int is_exist(char (*src)[FILELEN], char *target) // 학번이 IDS에 존재하는지 확인(유:1, 무:0)
{
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM)
			return false;
		else if(!strcmp(src[i], ""))
			return false;
		else if(!strcmp(src[i++], target))
			return true;
	}
	return false;
}

void set_scoreTable(char *ansDir) // score_table.csv 설정
{
	char filename[FILELEN]; // $(PWD)/score_table.csv

	sprintf(filename, "%s", "score_table.csv");

	if(access(filename, F_OK) == 0) // score_table.csv 파일이 존재 할 경우
		read_scoreTable(filename); 
	else{ // score_table.csv 파일이 존재하지 않을 경우 
		make_scoreTable(ansDir); // score_table.csv 생성
		write_scoreTable(filename); // score_table.csv 작성
	}
}

void read_scoreTable(char *path) // score_table.csv 파일 읽기 및 구조체 데이터 할당
{
	FILE *fp;
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){ // $(PWD)/score_table.csv를 읽기 전용으로 열기
		fprintf(stderr, "file open error for %s\n", path);
		return ;
	}

	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){ // 쉼표 기준으로 qname, score 할당
		strcpy(score_table[idx].qname, qname); // score_table 구조체에 문제 할당
		score_table[idx++].score = atof(score); // score_table 구조체에 점수 할당
	}

	fclose(fp);
}

void make_scoreTable(char *ansDir) // score_table.csv 파일 생성
{
	int type, num; 
	double score, bscore, pscore;
	struct dirent *dirp; // $(PWD)/ANS_DIR 디렉토리 목록 구조체
	DIR *dp; // $(PWD)/ANS_DIR 디렉토리 구조체
	char tmp[BUFLEN];
	int idx = 0; // score_table 구조체 문제 항목 개수
	int i;

	num = get_create_type();

	if(num == 1) // 1번 옵션 선택 시, 한번에 점수 할당
	{
		printf("Input value of blank question : ");
		scanf("%lf", &bscore); // 빈칸 문제 점수
		printf("Input value of program question : ");
		scanf("%lf", &pscore); // 프로그램 문제 점수
	}

	if((dp = opendir(ansDir)) == NULL){ // $(PWD)/ANS_DIR 열기
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}	

	while((dirp = readdir(dp)) != NULL) // $(PWD)/ANS_DIR 디렉토리 목록 읽어오기
	{
		sprintf(tmp, "%s/%s", ansDir, dirp->d_name);
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) // 현재, 상위 디렉토리 파일 접근 생략
			continue;

		if((type = get_file_type(dirp->d_name)) < 0) // 디렉토리에 존재하는 파일의 타입 확인, 요구하지 않는 파일의 경우 생략
			continue;

		strcpy(score_table[idx++].qname, dirp->d_name); // X-X.txt | X.c

	}
	closedir(dp);
	sort_scoreTable(idx); // score_table.csv 테이블 항목 정렬

	for(i = 0; i < idx; i++) 
	{
		type = get_file_type(score_table[i].qname); // score_table 구조체에 존재하는 문제 항목 파일 타입 결정 

		if(num == 1) // 1번 옵션 선택 시 
		{
			if(type == TEXTFILE) // X-X.txt파일의 경우 
				score = bscore;
			else if(type == CFILE) // X.c파일의 경우
				score = pscore;
		}
		else if(num == 2)  // 2번 옵션 선택 시 각 항목 별 점수 할당
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score);
		}

		score_table[i].score = score; // 문제 할당 점수 갱신
	}
}

void write_scoreTable(char *filename) // score_table.csv 데이터 작성
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]); // score_table 구조체의 항목 개수

	if((fd = creat(filename, 0666)) < 0){ // $(PWD)/score_table.csv, 0666 생성
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++)
	{
		if(score_table[i].score == 0) // score_table 구조체에 존재하는 문제의 할당 점수가 0일 경우 
			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score); // tmp = X-X.txt,XX.XX | X.c,XX.XX
		write(fd, tmp, strlen(tmp));
	}

	close(fd);
}


void set_idTable(char *stuDir) // 학번 테이블 생성
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

	if((dp = opendir(stuDir)) == NULL){ // STD_DIR 열기
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}

	while((dirp = readdir(dp)) != NULL){ // 해당 디렉토리 파일들 순회
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name); // STD_DIR/2020XXXX
		stat(tmp, &statbuf); // STD_DIR/내의 내용물 정보 가져오기

		if(S_ISDIR(statbuf.st_mode)) // 내용물이 디렉토리 일 경우
			strcpy(id_table[num++], dirp->d_name); // 테이블에 디렉토리 이름(학번) 저장
		else
			continue;
	}

	sort_idTable(num);
}

void sort_idTable(int size) // 학번 테이블 정렬: 오름차순
{
	int i, j;
	char tmp[10];

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 -i; j++){
			if(strcmp(id_table[j], id_table[j+1]) > 0){ 
				strcpy(tmp, id_table[j]);
				strcpy(id_table[j], id_table[j+1]);
				strcpy(id_table[j+1], tmp);
			}
		}
	}
}

void sort_scoreTable(int size) // score_table.csv 데이터 정렬
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for(i = 0; i < size - 1; i++) { // size = 테이블에 존재하는 항목 개수
		for(j = 0; j < size - 1 - i; j++) {
			get_qname_number(score_table[j].qname, &num1_1, &num1_2);
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2);

			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){ // 사전적 정렬에서 정수적 정렬로 데이터 재정렬
				memcpy(&tmp, &score_table[j], sizeof(score_table[0]));
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			} 
		}
	}
}

void get_qname_number(char *qname, int *num1, int *num2) // 문제 파일명 구분
{
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname)); 
	*num1 = atoi(strtok(dup, "-.")); // -, .으로 문제 파일 이름 구분
	
	p = strtok(NULL, "-.");
	if(p == NULL)  
		*num2 = 0;
	else 
		*num2 = atoi(p); 
}

int get_create_type() // score_table.csv 데이터 할당 방법 선택, 선택 정수 반환
{
	int num;

	while(1)
	{
		printf("score_table.csv file doesn't exist!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num);

		if(num != 1 && num != 2)
			printf("not correct number!\n");
		else
			break;
	}

	return num;
}

void score_students() // score.csv 생성
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]); // id_table 테이블 데이터 개수

	if((fd = creat("score.csv", 0666)) < 0){
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	write_first_row(fd); // 테이블 제목 행 생성(문제 번호)

	for(num = 0; num < size; num++)
	{
		if(!strcmp(id_table[num], "")) // 학생 테이블 내용이 존재하지 않을 경우
			break;

		sprintf(tmp, "%s,", id_table[num]); // tmp = 2020XXXX
		write(fd, tmp, strlen(tmp));  // score.csv -> 2020xxxx,

		score += score_student(fd, id_table[num]); // 학생의 점수 계산
	}

	printf("Total average : %.2f\n", score / num);

	close(fd);
}

double score_student(int fd, char *id) // 학생들의 답안 채점
{
	int type;
	double result; // 채점 결과, true:정답, false:오답
	double score = 0; // 채점 총점
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); // score_table 데이터 개수

	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname); // STD_DIR/2020XXXX/X-X.txt | STD_DIR/2020XXXX/X.c
		
		if(access(tmp, F_OK) < 0) // 학생 답안 파일 정보 가져오기
			result = false;
		else
		{
			if((type = get_file_type(score_table[i].qname)) < 0) 
				continue;
			
			if(type == TEXTFILE) // 빈칸 문제(X-X.txt)
				result = score_blank(id, score_table[i].qname);
			else if(type == CFILE) // 프로그램 문제(X.c)
				result = score_program(id, score_table[i].qname);
		}

		if(result == false) // 채점 결과가 틀렸을 경우 0점 처리
			write(fd, "0,", 2); 
		else{
			if(result == true){ // 채점 결과가 맞았을 경우
				score += score_table[i].score; // 총점에 추가
				sprintf(tmp, "%.2f,", score_table[i].score); 
			}
			else if(result < 0){ // 채점 결과가 WARNING일 경우
				score = score + score_table[i].score + result; // -0.1점 감점
				sprintf(tmp, "%.2f,", score_table[i].score + result);
			}
			write(fd, tmp, strlen(tmp));
		}
	}

	printf("%s is finished.. score : %.2f\n", id, score); // 최종 결과 출력

	sprintf(tmp, "%.2f\n", score);
	write(fd, tmp, strlen(tmp)); // score.csv의 마지막 열에 데이터를 작성

	return score;
}

void write_first_row(int fd) // $(PWD)/score.csv의 제목행 데이터 삽입
{
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); // score_table 테이블 데이터 개수

	write(fd, ",", 1); // 첫번째 열은 빈칸

	for(i = 0; i < size; i++){
		if(score_table[i].score == 0) // 더이상 데이터가 존재하지 않을 경우
			break;
		
		sprintf(tmp, "%s,", score_table[i].qname);
		write(fd, tmp, strlen(tmp));
	}
	write(fd, "sum\n", 4); // 마지막 열은 총합 점수
}

char *get_answer(int fd, char *result) // X-X.txt에서 작성한 답안 반환
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN);
	while(read(fd, &c, 1) > 0) // X-X.txt에서 1바이트 씩 읽어들임
	{
		if(c == ':') 
			break;
		
		result[idx++] = c;
	}
	if(result[strlen(result) - 1] == '\n') 
		result[strlen(result) - 1] = '\0';

	return result;
}

int score_blank(char *id, char *filename) // 빈칸 문제 채점
{
	char tokens[TOKEN_CNT][MINLEN]; // 토큰 배열
	node *std_root = NULL, *ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN]; // 학생 답안 내용, 정답 답안 내용
	char qname[FILELEN] = { 0 }; // X | X-X
	int fd_std, fd_ans; // 학생 답안, 정답 답안 파일 디스크럽터
	int result = true; 
	int has_semicolon = false; // 세미콜론 유무 확인 변수

	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); // qname = X | X-X

	// 학생 빈칸 문제 답안
	sprintf(tmp, "%s/%s/%s", stuDir, id, filename); // STD_DIR/2020XXXX/X-X.txt
	fd_std = open(tmp, O_RDONLY); // $(PWD)/STD_DIR/X-X.txt 읽기 전용 열기
	strcpy(s_answer, get_answer(fd_std, s_answer)); // 학생 답안 내용 가져오기

	if(!strcmp(s_answer, "")) { // 학생 답안의 내용이 없을 경우
		close(fd_std);
		return false;
	}

	if(!check_brackets(s_answer)){ // 학생 답안의 괄호의 짝이 맞지 않을 경우
		close(fd_std);
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer))); // 학생 답안의 좌우 공백 지우기

	if(s_answer[strlen(s_answer) - 1] == ';'){ // 학생 답안의 끝에 ';'이 존재할 경우
		has_semicolon = true;
		s_answer[strlen(s_answer) - 1] = '\0'; // 세미콜론 삭제
	}

	if(!make_tokens(s_answer, tokens)){ // 학생 답안 토큰 생성, 정상:1, 오류:0
		close(fd_std);
		return false;
	}

	idx = 0; // 인덱스 초기화
	std_root = make_tree(std_root, tokens, &idx, 0); // std_root = 학생 답안의 토큰 트리의 파스노드, 학생 답안의 트리 생성

	// 정답 빈칸 문제
	sprintf(tmp, "%s/%s", ansDir, filename); // ANS_DIR/X-X.txt 
	fd_ans = open(tmp, O_RDONLY); // ANS_DIR/X-X.txt 읽기 전용 열기

	while(1)
	{
		ans_root = NULL; // ANS_DIR/X-X.txt의 토큰트리의 루트 노드
		result = true; 

		for(idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx]));

		strcpy(a_answer, get_answer(fd_ans, a_answer)); // 정답 답안 내용 가져오기

		if(!strcmp(a_answer, "")) // 정답 답안의 내용이 없을 경우
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer))); // 정답 답안의 좌우 공백 지우기

		if(has_semicolon == false){ // 학생 답안의 끝에 ';'이 존재하지 않을 경우
			if(a_answer[strlen(a_answer) -1] == ';')  // 정답 답안의 끝에 ';'이 존재하면 
				continue; // 넘어감
		}

		else if(has_semicolon == true) // 학생 답안에 ';'이 존재할 경우
		{
			if(a_answer[strlen(a_answer) - 1] != ';') // 정답 답안 내용의 끝이 ';'가 아닐 경우
				continue; // 넘어감
			else
				a_answer[strlen(a_answer) - 1] = '\0'; // 정답 답안 내용의 끝의 세미콜론을 지우고 NULL을 넣음
		}

		if(!make_tokens(a_answer, tokens)) // 정답 답안 토큰 생성, 정상:1, 오류:0
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0); // ans_root = 정답 답안의 파스트리의 루트노드, 정답 답안의 트리 생성

		compare_tree(std_root, ans_root, &result); // 학생 답안의 파스트리와 정답 답안의 파스트리 비교

		if(result == true){ // 정답 답안의 파스트리와 학생 답안의 파스트리가 같을 경우
			close(fd_std);
			close(fd_ans);

			// 파스트리 초기화
			if(std_root != NULL) 
				free_node(std_root);
			if(ans_root != NULL)
				free_node(ans_root);
			return true;

		}
	}
	
	close(fd_std);
	close(fd_ans);

	if(std_root != NULL)
		free_node(std_root);
	if(ans_root != NULL)
		free_node(ans_root);

	return false;
}

double score_program(char *id, char *filename) // 프로그램 문제 채점, id:202000XX, filename:$(PWD)/DIR/filepath
{
	double compile;
	int result;
	compile = compile_program(id, filename); // 프로그램 컴파일

	if(compile == ERROR || compile == false) // ERROR:0, false:0
		return false;
	
	result = execute_program(id, filename); // 프로그램 실행

	if(!result) // 프로그램 컴파일 시 에러가 떴을 경우
		return false;

	if(compile < 0) // 프로그램 컴파일 시 경고가 떴을 경우
		return compile;

	return true;
}

int is_thread(char *qname)
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]); // threadFiles 구조체 개수(문제 개수)

	for(i = 0; i < size; i++){
		if(!strcmp(threadFiles[i], qname)) // 해당하는 문제가 존재할 경우
			return true;
	}
	return false; // 존재하지 않을 경우
}

double compile_program(char *id, char *filename) // 프로그램 문제 컴파일
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN] = { 0 };
	int isthread;
	off_t size;
	double result;
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); // qname = X
	
	isthread = is_thread(qname); // -t 옵션으로 받은 문제인지 확인

	// 정답 프로그램 컴파일
	sprintf(tmp_f, "%s/%s", ansDir, filename); // ANS_DIR/X.c
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname); // ANS_DIR/X.exe
	if(tOption && isthread) // -t 옵션이 문제에 주어졌을 경우
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else // 아닐 경우
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_e, "%s/%s_error.txt", ansDir, qname);
	fd = creat(tmp_e, 0666);
	
	redirection(command, fd, STDERR); // command 명령 실행 후, 에러시 ANS_DIR/X_error.txt에 내용 저장
	size = lseek(fd, 0, SEEK_END); // 파일 크기 저장
	close(fd); 
	unlink(tmp_e); // ANS_DIR/X.exe 파일 삭제

	if(size > 0) // 파일에 컴파일 에러 내용이 저장되어 있을 경우
		return false;

	// 학생 답안 프로그램 컴파일
	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename); // STD_DIR/2020XXXX/X.c
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname); // STD_DIR/2020XXXX/X.stdexe

	if(tOption && isthread) // -t 옵션이 문제에 주어졌을 경우
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else // 아닐 경우
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname); // STD_DIR/2020XXXX/X_error.txt
	fd = creat(tmp_f, 0666);

	redirection(command, fd, STDERR); // command 명령 실행 후, 에러 시 STD_DIR/2020XXXX/X_error.txt에 내용 저장
	size = lseek(fd, 0, SEEK_END); // 파일 크기 저장
	close(fd);

	if(size > 0){ // 파일에 컴파일 에러 내용이 저장되어 있을 경우
		if(eOption) // -e 옵션을 주었을 때 ERROR/2020XXXX/X_error.txt로 저장
		{
			sprintf(tmp_e, "%s/%s", errorDir, id); // ERROR/2020XXXX
			if(access(tmp_e, F_OK) < 0) // 디렉토리 접근이 가능하지 않을 경우(디렉토리가 존재하지 않을 경우)
				mkdir(tmp_e, 0755);

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname); // ERROR/2020XXXX/X_error.txt
			rename(tmp_f, tmp_e); // mv STD_DIR/2020XXXX/X_error.txt ERROR/2020XXXX/X_error.txt

			result = check_error_warning(tmp_e); // 에러인지 경고인지 파악
		}
		else{ 
			result = check_error_warning(tmp_f); // 에러인지 경고인지 파악
			unlink(tmp_f); // STD_DIR/2020XXXX/X_error.txt 삭제
		}

		return result;
	}

	unlink(tmp_f); // STD_DIR/2020XXXX/X_error.txt 삭제
	return true;
}

double check_error_warning(char *filename) // 컴파일 에러 혹은 경고 체크
{
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;

	if((fp = fopen(filename, "r")) == NULL){ // X_error.txt파일 읽기 전용으로 파일 열기
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	while(fscanf(fp, "%s", tmp) > 0){ // 파일 내용 비교
		if(!strcmp(tmp, "error:")) // 내용에 에러가 존재할 경우
			return ERROR;
		else if(!strcmp(tmp, "warning:")) // 내용에 경고가 존재할 경우
			warning += WARNING;
	}

	return warning;
}

int execute_program(char *id, char *filename) // 프로그램 실행
{
	char std_fname[BUFLEN], ans_fname[BUFLEN];
	char tmp[BUFLEN];
	char qname[FILELEN] = { 0 }; // X | X-X
	time_t start, end; // 프로그램 시작, 종료 시간
	pid_t pid; // 프로그램 PID
	int fd; // X.exe | X.stdexe 파일 디스크럽터

	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));

	// 정답 프로그램 실행
	sprintf(ans_fname, "%s/%s.stdout", ansDir, qname); // ANS_DIR/X.stdout
	fd = creat(ans_fname, 0666); // ANS_DIR/X.stdout을 0666권한으로 생성

	sprintf(tmp, "%s/%s.exe", ansDir, qname); // ANS_DIR/X.exe
	redirection(tmp, fd, STDOUT); // tmp 실행 후 표준 출력 내용을 ANS_DIR/X.stdout에 저장
	close(fd);

	// 학생 답안 프로그램 실행 
	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname); // STD_DIR/2020XXXX/X.stdout
	fd = creat(std_fname, 0666); // STD_DIR/2020XXXX/X.stdout을 0666 권한으로 생성

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname); // STD_DIR/2020XXXX/X.stdexe &

	start = time(NULL); // 시작 시간 기록
	redirection(tmp, fd, STDOUT); // tmp 실행 후 표준 출력 내용을 STD_DIR/2020XXXX/X.stdout으로 저장
	
	sprintf(tmp, "%s.stdexe", qname); // X.stdexe
	while((pid = inBackground(tmp)) > 0){ // X.stdexe를 스레드 생성하여 실행, 정상:PID, 오류:-1
		end = time(NULL);

		if(difftime(end, start) > OVER){ // OVER:5, 실행 시간 초과
			kill(pid, SIGKILL); // 스레드 제거
			close(fd); 
			return false;
		}
	}

	close(fd);

	return compare_resultfile(std_fname, ans_fname); // 실행 결과 비교, 같음:1, 다름:0
}

pid_t inBackground(char *name) // 스레스 생성 및 실행, name = X.stdexe, 정상:PID, 오류:0
{
	pid_t pid; // PID
	char command[64]; // 
	char tmp[64] = { 0 }; // X.stdexe
	int fd;
	off_t size;
	
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); // $(PWD)/background.txt 파일을 0666권한으로 읽기 및 쓰기, 존재하지 않으면 생성, 존재하면 새로 쓰기

	sprintf(command, "ps | grep %s", name); // ps | grep X.stdexe
	redirection(command, fd, STDOUT); // command 실행 후 표준 출력 내용을 background.txt파일에 저장

	lseek(fd, 0, SEEK_SET); // background.txt의 오프셋을 시작으로 이동
	read(fd, tmp, sizeof(tmp));

	if(!strcmp(tmp, "")){ // background.txt 파일에 아무런 내용도 써져있지 않을 경우
		unlink("background.txt");
		close(fd);
		return 0;
	}

	pid = atoi(strtok(tmp, " ")); // 프로세스가 생성되어 내용이 작성되어있을 경우 앞의 PID를 변수에 할당
	close(fd);

	unlink("background.txt"); 
	return pid;
}

int compare_resultfile(char *file1, char *file2) // 실행 결과 비교, 같음:1, 다름:0
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY); // STD_DIR/2020XXXX/X.stdout
	fd2 = open(file2, O_RDONLY); // ANS_DIR/X.stdout

	while(1)
	{
		while((len1 = read(fd1, &c1, 1)) > 0){ // STD_DIR/2020XXXX/X.stdout을 한글자씩 읽어들임
			if(c1 == ' ') 
				continue;
			else 
				break;
		}
		while((len2 = read(fd2, &c2, 1)) > 0){ // ANS_DIR/X.stdout을 한글자씩 읽어들임
			if(c2 == ' ') 
				continue;
			else 
				break;
		}
		
		if(len1 == 0 && len2 == 0) // 둘다 파일의 끝까지 읽었을 경우
			break;

		to_lower_case(&c1); 
		to_lower_case(&c2);

		if(c1 != c2){ // 둘의 문자가 다를 경우
			close(fd1);
			close(fd2);
			return false; // 다름
		}
	}
	close(fd1);
	close(fd2);
	return true; // 같음
}

void redirection(char *command, int new, int old) // 디스크럽터 변경, old:표준입출력
{
	int saved;

	saved = dup(old); 
	dup2(new, old); // old -> new 디스크럽터 변경

	system(command);

	dup2(saved, old); // old -> saved 디스크럽터 변경
	close(saved);
}

int get_file_type(char *filename) // 파일 유형 리턴
{
	char *extension = strrchr(filename, '.');

	if(!strcmp(extension, ".txt"))
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))
		return CFILE;
	else
		return -1;
}

void rmdirs(const char *path) // 디렉토리 제거
{
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[BUFLEN] = { 0 };
	
	if((dp = opendir(path)) == NULL)
		return;

	// 해당 디렉토리 안의 내용물을 전부 제거
	while((dirp = readdir(dp)) != NULL)
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", path, dirp->d_name); // tmp = 디렉토리 내부 파일

		if(lstat(tmp, &statbuf) == -1) // 파일 상태 정보 가져오기
			continue;

		if(S_ISDIR(statbuf.st_mode)) // 디렉토리일 경우 재귀적으로 제거 ($rm -rf)
			rmdirs(tmp);
		else
			unlink(tmp);
	}

	closedir(dp);
	rmdir(path);
}

void to_lower_case(char *c) // 대문자를 소문자로 변환
{
	if(*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage() // -h 옵션
{
	printf("Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -m                modify question's score\n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file \n");
	printf(" -t <QNAMES>       compile QNAME.C with -lpthread option\n");
	printf(" -i <IDS>          print ID's wrong questions\n");
	printf(" -h                print usage\n");
}
