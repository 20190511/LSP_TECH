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
#define MAXPATHLEN  4097		/// 내 저으이 상수




struct ssu_scoreTable{
	char qname[FILELEN];
	double score;
};


typedef struct sclist {
    char qname[FILELEN];        // 문제번호
    double cur_score;              // 현 점수
    double score;                  // 원래배점
    struct sclist* file_next;        // 다음에 연결된 문제
}Sclist;

typedef struct stdnode {
    char id_name[10];           //학번
    double sums;                //총합

    int file_cnt;              //연결된 문제개수
    struct stdnode* next;              // 다음 학번
    Sclist* list_head;        // head -> .. -> .. -> tail ->NULL 형태로 연결
    Sclist* list_tail;
}Snode;


typedef struct slist {
    int id_cnt;
    Snode* head;            //header -> ... -> ... -> tail
    Snode* tail;  
}StdList;


/**
 *  04.04 
 * 		[score_table.csv 관련]
 * 		1. 점수를 고치려면 점수테이블 score_table.csv 값을 수정해야한다.
 * 				** score_table 전역변수는 read_scoreTable() 를 호출하면 세팅된다.
 * 
 * 				-->read_scoreTable() 에서 볼 수 있는데 %[^,],%s\n 를 잘 이용해서 $[^,] 동일한 이름의 점수를 수정할 수 있다.
 * 				--> 점수 테이블을 만드는 함수는 set_scoreTable( ansDir) 이다. 
 * 		2. get_create_type() 를 호출하면 점수테이블 1,2 중에 선택받게 한다. 
 * 				1: 빈칸/프로그램 문제 점수 일괄처리,
 * 				2: 개별 문제리스트를 출력해주는데 해당 점수 입력
 * 
 *  	3. write_first_row() 를 호출하면 score_table의 제목열부분을 만들 수 있다.
 * 
*/
void ssu_score(int argc, char *argv[]);
int check_option(int argc, char *argv[]);
void print_usage();   // -h 옵션
/**
 * score.csv 제작함수 (csv 파일 생성 함수)
 * 전체 score.csv에 전체학생 점수를 추가해서 저장하는듯함.
 * id_table 인덱스 개수만큼 파일에 데이터입력
 * 
*/
void score_students(); 

/**
 * 학생 답 채점함수
 * fd := csv 파일디스크립터, id := 학번 (학생학번)
 * score_table 배열 안의 점수만큼 채점을 한다.
 * 학생 한 명에대해서 점수총점을 구해서 csv file에 추가.
 * 
*/
double score_student(int fd, char *id, Snode* std_node);

/**
 *  $(PWD)/score.csv 제목 데이터 (score_table 을 이용하여 테이블 개수를 구하고)
 *  첫 열은 빈칸처리 (,) : 학번이 들어갈꺼니까
 * 	두 번째 열 ~ n-1 번 째 열까지 score_table 의 qname을 출력한다.
 *  n번째(마지막열) 에는 총합 sum을 만든다.
 * 
*/
void write_first_row(int fd);

/**
 * fd 디스크립터 파일의 답(내용) 을 가져와서 result 출력.
*/
char *get_answer(int fd, char *result);

/**
 * 빈 칸 문제 채점
*/
int score_blank(char *id, char *filename);
/**
 * compile_program() 호출, execute_program() 호출을 통해 프로그램 워닝 개수, 컴파일 성공여부를 바탕으로 점수추출
*/
double score_program(char *id, char *filename);             
double compile_program(char *id, char *filename);			// 프로그램 컴파일
int execute_program(char *id, char *filname);				// 프로그램문제 실행, 성공시 1, 실패시 0

/**
 * background.txt (임시파일) 을 만들어서 
 * 프로그램 문제의 실행파일을 system(ps | grep 실행파일.stdexe) 를 받아와서 
 *  indirect() 호출을 통해 해당 프로그램을 실행시키고, (해당 프로그램 표준출력--> background.txt 파일로 전환)
 * 	background.txt 를 처음부터 읽는다. (lseek(fd,0,SEEK_SET)
 * 	background.txt 파일이 비어져 있는 경우 0 return
 *  프로세스가 생성되어 파일이 채워져 있는 경우 해당 파일에는 processID가 있을 것이고 해당 pid return
 * 
*/
pid_t inBackground(char *name);

/** 
 *  컴파일 결과를 출력해놓은 filename을 읽어서
 * 		컴파일오류시 0 리턴, 워닝 당 0.1 점씩 감점
 * 
 * filename은 컴파일 결과를 넣어둔 파일이고
 * 해당 파일에서 Error 존재 시 Error(0) return
 * 해당 파일에서 Warning 존재 시 warning 만큼 0.1 점씩 점수를깎아서 return
 *  */
double check_error_warning(char *filename);
/** 
 * 실행결과가 같은지 비교
*/
int compare_resultfile(char *file1, char *file2);

void do_iOption(char (*ids)[FILELEN]);
void do_mOption();

/**
 *  학번 (target) 이 src에 존재하면 True, 없으면 False
*/
int is_exist(char (*src)[FILELEN], char *target);
/** 
 * 스레드 문제개수
*/
int is_thread(char *qname);

/**
 * new를 old 디스크립터로 출력(system) 하고 다시 복귀, old:표준입출력 
 * 	즉 command 를 newfd --> oldfd 로 파일시스템을 이용
*/
void redirection(char *command, int newfd, int oldfd);
/**
 * 파일 타입 가져오기
 * .txt파일이면 : TEXTFILE(3)
 * .c 파일이면  : CFILE(4)
 * 실패하면     : -1
*/
int get_file_type(char *filename);                           /// 파일 타입 가져오기
/** path 경로 내부 디렉토리 all clear*/
void rmdirs(const char *path);
/** 대문자-->소문자*/
void to_lower_case(char *c);

void set_scoreTable(char *ansDir);
/**
 * ssu_scoreTable 구조체 : {문제, 점수} 를 정의한 타입
 * csvfile이 존재하면 
 * 고급 테크닉 
 * 		%[,] : 쉼표포함해서 쉼표앞까지 읽어라
 * 		%[^,] : 쉽표 앞까지 읽어라.
 * 		쉼표까지읽어서, 문제번호(qname), 점수 (score)를 받아와서 문제테이블 score_table[] 에 저장
*/
void read_scoreTable(char *path);

/**
 * score_table.csv 파일 생성
 * 		ansDir 내부 파일들을 모두 순회하면서 score_table 구조체 배열에 문제이름 할당.
*/
void make_scoreTable(char *ansDir);

/**
 * 
 * score_table 구조체로부터 filename으로 점수표 생성가능.
*/
void write_scoreTable(char *filename);

/**
 * 학번 테이블 생성 
 * STUDIR 안의 학번을 모두 순회해서 학번디렉토리가 존재할 시
 * id_table에 학번 저장.
*/
void set_idTable(char *stuDir);

/**
 * 문제 점수 어떻게 넣을건지 선택받게 하기 1,2
*/
int get_create_type();

/**
 *	 학번 테이블을 오름차순으로 정렬, (오름차순으로 정렬했으니까, 반대로 내림치차순이면 이걸 건들이면된다.)
 *   정렬알고리즘은 단순 NN 정렬
 */
void sort_idTable(int size);

/**
 * 1-2 > 1-3 > 2-1 순으로 문제번호 정렬식으로 score_table를 정렬함.
*/
void sort_scoreTable(int size);

/**
 * 
 * qname 문자열을 받아서 문제가 1-3.txt 라면 num1(1), num2(3) 이 들어감,
 * 		 만약, 1.txt 라면 num1(1), num2(0) 이다.
 * strtok(dup, "-.") 를 넣어서 dup을 - 와 . 단위로 토큰을 끊어냄
 * 해당 방식으로 1-3.txt 를 1, 3, txt 로 분리시킬 수 있음.
*/
void get_qname_number(char *qname, int *num1, int *num2);


extern struct ssu_scoreTable score_table[QNUM];
extern char id_table[SNUM][10];

struct ssu_scoreTable score_table[QNUM];
char id_table[SNUM][10];

char stuDir[BUFLEN];
char ansDir[BUFLEN];
char errorDir[BUFLEN];					/// -e 옵션 에러 디렉토리.
char threadFiles[ARGNUM][FILELEN];
char iIDs[ARGNUM][FILELEN];
int iIDs_check = false;
int iIDs_cnt = 0;

int eOption = false;
int tOption = false;
int mOption = false;



/// 20190511, 배준형 |  새로만든 설정 변수/함수 들




int hOption = false;
int cOption = false;
int pOption = false;
int sOption = false;
int nOption = false;
int do_nOption (char* pathname);
int realpathS(char* pathname, size_t size);
int csv_check (char* pathname);
int mkdirs(char* pathname);							//디렉토리 생성함수.
char csvDir[BUFLEN];
int sortType[2];


Sclist* new_sclists (char* qname, double curS); //sclist 초기화함수
Sclist* new_sclistss (char* qname, double curS, double s); //문제 배점 붙은 Sclist 초기화함수
Snode* new_stdnode (char* id); //id 로 Snode 초기화함수
StdList* new_stdlist(); //stdlist 초기화함수
Snode* append_list (StdList* list, char* id);  // list 에 해당 id로 Snode 생성, return으로 append한 구조체 반환
void print_list (StdList* list);
void print_score (Snode* node); // Sclist 문제 요소 출력 (맞은문제, 문제배점)
int append_score(Snode* node, char* qname, double cur_score, double score);   // Snode 에 문제 구조체 Sclist append.
double find_score (char* qname);  // qname 과 일치하는 score 원래의 배점을 score_table 로부터 찾기
int swap_list (StdList* list, Snode* p, Snode* a, Snode* b);  // 버블 정렬에 필요한 a <-> b 링크드리스트 구조 변경
void sort_descentS(StdList* list); //list 의 sums(총함) 내림차순으로 정렬(버블sort)     
void sort_aescendS(StdList* list); //list 의 sums(총함) 오름차순으로 정렬(버블sort)
void sort_descentI(StdList* list); //list 의 id (학번) 내림차순으로 정렬(버블sort)
void sort_aescendI(StdList* list); //list 의 id (학번) 오름차순으로 정렬(버블sort)
void sort_manager(StdList* list, int type, int updown);     //sorting 관리 구조체
void print_wrongL (Snode* node); // node 안의 틀린문제들 요소 출력
int write_sort (StdList* list, int fd);  // 정렬된 리스트를 출력


void ssu_score(int argc, char *argv[])
{
	char saved_path[BUFLEN];			/// 프로그램이 실행된 디렉토리 저장.
	int i;

	/*
	for(i = 0; i < argc; i++){
		if(!strcmp(argv[i], "-h")){
			print_usage();
			return;
		}
	}
	*/


	memset(saved_path, 0, BUFLEN);					// 프로그램이 시작된 위치 saved_path 기록.

	if(argc >= 3){
		strcpy(stuDir, argv[1]);
		realpathS(stuDir, BUFLEN);
		strcpy(ansDir, argv[2]);
		realpathS(ansDir, BUFLEN);
	}
	

	if(!check_option(argc, argv))
		return;

	if (hOption && (nOption || mOption || cOption || pOption || sOption || tOption || eOption)) 		/// hOption 과 다른 옵션 같이오는 경우 예외처리
		return;
	
	if (pOption && sOption) ///p옵션과 s 옵션이 같이오는 경우 
		return;

	if (hOption)
	{
		print_usage();
		exit(1);
	}

	/*
	if(!mOption && !eOption && !tOption && iOption 
			&& !strcmp(stuDir, "") && !strcmp(ansDir, "")){
		do_iOption(iIDs);
		return;
	}
	*/


	getcwd(saved_path, BUFLEN);

	if(chdir(stuDir) < 0){
		fprintf(stderr, "%s doesn't exist\n", stuDir);			
		return;
	}
	getcwd(stuDir, BUFLEN);								/// stuDir <-- <STD_DIR> 복사

	chdir(saved_path);
	if(chdir(ansDir) < 0){			
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN);    							/// ans_dir <-- <ANS_DIR> 복사
	


	/// 1. n 옵션 (ans_dir 변경)
	if (nOption)
	{
		chdir(saved_path);			/// 현재 자신 경로로 상대경로를 받기 때문
		if(!do_nOption(csvDir))		//csvdir 에 -n 옵션 디렉토리 저장.
			return;
	}
	else
	{
		sprintf(csvDir, "%s/%s", ansDir, "score.csv"); /// 복사 디렉토리 생성
	}

	/// 수정 saved_path-> ansDir
	chdir(ansDir);
	set_scoreTable(ansDir);				/// score Table은 이미 존재하지 않으면 에러처리
	
	if(mOption)
	{
		char filename[BUFLEN] = {0,};
		sprintf(filename, "./%s", "score_table.csv");
		if (access(filename, R_OK) == -1)		//파일 접근 불가 시 에러처리
			return;
		do_mOption();			
	}
		

	set_idTable(stuDir);

	/// -p, -c 옵션 테이블 체크
	if (pOption || cOption)
	{
		for (int idx ; idx < iIDs_cnt ; idx++)
		{
			int pc_check = 0;
			for (int idx_y = 0 ; idx_y < sizeof(id_table) / sizeof(id_table[0]) ; idx_y++)
			{
				if (!strcmp(iIDs[idx], id_table[idx_y]))
				{
					pc_check = 1;
					continue;
				}
			}
			
			if (pc_check == 0)		// 인자로 받은 학번이 없는 경우 에러처리
			{
				return;
			}
			pc_check = 0;
		}
	}

	printf("grading student's test papers..\n");
	setbuf(stdout, NULL);
	score_students();		/// 실제 -p 옵션으로 보임


	///내가 임의로 생성.
	chdir(saved_path);	

	/*
	if(iOption)
		do_iOption(iIDs);
	*/

	return;
}

int check_option(int argc, char *argv[])
{
	int i, j, k;
	int c;
	int exist = 0;

	while((c = getopt(argc, argv, ":pce:thmsn:")) != -1)
	{
		switch(c){
			case 'n':
				nOption = true;
				if (optarg == NULL)			// 인자가 없는 경우 예외처리
					return false;
				strcpy(csvDir, optarg);
				break;
			case 'c':
				cOption = true;
				i = optind;
				j = 0;
				while(i < argc && argv[i][0] != '-'){
					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded. :: %s\n", argv[i]);
					else
					{
						if (j == 0 && iIDs_check)		// 인자를 중복으로 받은 경우 예외처리
						{
							iIDs_check = -1;
							return false;
						}
						strcpy(iIDs[j], argv[i]);
						iIDs_check = true;
						iIDs_cnt = j+1;
					}
					i++;
					j++;
				}
				break;

			case 'p':
				pOption = true;
				i = optind;
				j = 0;
				while(i < argc && argv[i][0] != '-'){
					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded. :: %s\n", argv[i]);
					else
					{
						if (j == 0 && iIDs_check)		// 인자를 중복으로 받은 경우 예외처리
						{
							iIDs_check = -1;
							return false;
						}
						strcpy(iIDs[j], argv[i]);
						iIDs_check = true;
						iIDs_cnt = j+1;
					}
					i++;
					j++;
				}
				break;
			case 'h':
				hOption = 1;
				break;
			case 's':
				sOption = true;
				i = optind;
				if (i+2 < argc && argv[i][0] != '-')
					return false; 
				if (argc < i+2 || strlen(argv[i]) == 0 || strlen(argv[i+1]) == 0)	// 인자처리 매끄럽지 않으면 에러처리
					return false;

				if (!strcmp(argv[i], "stdid") | !strcmp(argv[i], "score"))
				{
					if (!strcmp(argv[i], "stdid"))
						sortType[0] = 0;
					if (!strcmp(argv[i], "score"))
						sortType[0] = 1;
				}
				else // 첫 인자가 stdid 나 score 가 아니면 예외처리
					return false;

				if (!strcmp(argv[i+1], "1") || !strcmp(argv[i+1], "-1"))
				{
					sortType[1] = atoi(argv[i+1]);
				}
				break;
			case 'e':
				eOption = true;
				strcpy(errorDir, optarg);
				realpathS(errorDir, BUFLEN); 			/// 상대경로 --> 절대경로화
				if(access(errorDir, F_OK) < 0)
				{
					if (mkdirs(errorDir) < 0)
						return false;
					mkdir(errorDir, 0755);
				}
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
			case ':':
				return false;
			case '?':
				if (optopt == '1' && sOption)
					return true;
				else
				{
					printf("Unkown option %c\n", optopt);
					return false;
				}
		}
	}

	return true;
}


// n옵션으로 score.csv 파일 위치 변동
int do_nOption (char* pathname)		
{
	char tmpP [MAXPATHLEN] = {0,};
	strcpy(tmpP, pathname);
	
	if (!realpathS(tmpP, MAXPATHLEN))			// 길이 제한 오류처리
		return false;
	
	if (!csv_check(tmpP))			// csv 파일 아닌 경우 삭제
		return false;

	strcpy(csvDir, tmpP);
	mkdirs(tmpP); 			// ansDir 부모디렉토리까지 디렉토리 생성.
	return true;
}

// pathname 부모디렉토리까지 재귀적으로 생성하는 함수
int mkdirs(char* pathname)		// 디렉토리 생성함수
{
    if (pathname[0] != '/')         // 절대경로만 받을 수 있음.
        return 0;
	char tokens[MAXPATHLEN] = {0,};
	char subS[MAXPATHLEN] = "/";
	strcpy(tokens, pathname);
	char* delParent = strrchr(tokens, '/');
    *delParent = '\0';


	char *tkn = strtok(tokens, "/");
	char *ptr = subS + strlen(subS);

    struct stat statbuf;
	while (tkn != NULL)
	{
		strcpy (ptr, tkn);
        if (access(subS, F_OK) == 0)
        {
            stat(subS, &statbuf);
            if (!S_ISDIR(statbuf.st_mode))      // 해당 경로에 파일이 있는 경우 덮어씌움
            {
                remove(subS);
                mkdir(subS, 0755);
            }
        }
        else
        {
		    mkdir(subS, 0755);
        }
		ptr = subS + strlen(subS);
		*ptr++ = '/';
		tkn = strtok(NULL, "/");
	}
}


//csv 파일인지 체크해주는 함수
int csv_check (char* pathname)				/// csv 파일 체크함수.
{
    char* csvtype = ".csv";
    char* ptr = strrchr(pathname, '.');

    if (ptr == NULL)
        return false;
    else if (strcmp(csvtype, ptr))
        return false;
    else
        return true;
}

// 상대경로 -> 절대경로 함수
int realpathS(char* pathname, size_t size)
{
    char tmp [MAXPATHLEN*2] = {0,};
    char home [MAXPATHLEN] = {0,};
	char buf [MAXPATHLEN*2] = {0,};
    strcpy(tmp, pathname); 
    if (strstr(pathname, "~/") != NULL || !strcmp(pathname, "~"))
    {
        char *home_path = getenv("HOME");
        if (home_path == NULL)
        {
            memset(pathname, 0, size);
            return false;
        }
        char *tok_ptr = strrchr(pathname, '~');
        sprintf(tmp, "%s%s", home_path, ++tok_ptr);
    }


    if (access(tmp, F_OK) != 0)
    {
        // 해당 경로에 파일이 존재하지 않으면 realpath 사용안됨.
        char pwds[MAXPATHLEN] = {0,};
        getcwd(pwds, MAXPATHLEN);
        strcpy(buf, tmp);
        char* ptr2 = tmp;
        if (pathname[0] != '/' && strncmp(pathname, "./", 2) && strncmp(pathname, "../",3))
        {
            /* //생각해보니 .. , . 들어가있어도 상관없음 (이거도 디렉토리명이잖어)
            if (!strncmp(pathname, "./", 2))
            {
                realpath("./", pwds);
                ptr2 += strlen("./");
            }
            if (!strncmp(pathname, "../",3))
            {
                realpath("../", pwds);
                ptr2 += strlen("../");
            }
            */
            sprintf(buf, "%s/%s", pwds, ptr2);       
        }
    }
    else
        realpath(tmp, buf);
	strcpy(tmp, buf);


    if (strlen(tmp) >= MAXPATHLEN || strlen(tmp) <= 0)
    {
        memset(pathname, 0, size);
        return false;
    }
    else
    {
        strcpy(pathname, tmp);
        return true;
    }
}

// 현재 사용 X 함수
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

// 문제 번호 수정 함수
void do_mOption(char *ansDir)
{
	double newScore;
	char modiName[FILELEN];
	char filename[FILELEN];
	char *ptr;
	int i;

	ptr = malloc(sizeof(char) * FILELEN);

	while(1){
		// 수정할 문제번호 입력받기
		printf("Input question's number to modify >> ");
		scanf("%s", modiName);

		if(strcmp(modiName, "no") == 0)
			break;

		for(i=0; i < sizeof(score_table) / sizeof(score_table[0]); i++){
			strcpy(ptr, score_table[i].qname);
			ptr = strtok(ptr, ".");
			if (ptr == NULL)		// 찾는 문제가 없는 경우 break;
			{
				fprintf(stderr, "your number is wrong\n");
				exit(1);	
			}
			if(!strcmp(ptr, modiName)){ //찾는 문제가 존재할 경우 현재 배점 띄워주고, 변화시킬 배점 받기
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

// 현재 사용 X?
char* get_header_char(char *header, int idx) 
{
	char *temp = (char *)calloc(BUFLEN, sizeof(char));
	int i = 0;
	while(header[idx] != ',')
		temp[i++] = header[idx++];
	return temp;
}

// 해당 옵션에 학번이 존재하는지 확인하는 함수
int is_exist(char (*src)[FILELEN], char *target) 
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

// ANS/score_table.csv 생성 함수
void set_scoreTable(char *ansDir) // score_table.csv 설정
{
	char filename[FILELEN]; 
	// chdir(ansDir);
	sprintf(filename, "%s", "score_table.csv");

	// 파일이 존재하면 score_table 전역변수 배열을 만들고, 없으면 새로 생성 
	if(access(filename, F_OK) == 0) 
		read_scoreTable(filename); 
	else{ 
		make_scoreTable(ansDir); 
		write_scoreTable(filename); 
	}
}

// score_table.csv 파일을 읽어서 각 문제 점수 배열을  score_table 배열에 할당
void read_scoreTable(char *path) 
{
	FILE *fp;
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){
		fprintf(stderr, "file open error for %s\n", path);
		return ;
	}

	// , 를 바탕으로 앞부분을 qname, score 로 설정
	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){ 
		strcpy(score_table[idx].qname, qname); 
		score_table[idx++].score = atof(score);
	}

	fclose(fp);
}

// score_table.csv 생성 함수
void make_scoreTable(char *ansDir) 
{
	int type, num; 
	double score, bscore, pscore;
	struct dirent *dirp; 
	DIR *dp; 
	char tmp[BUFLEN];
	int idx = 0;
	int i;

	num = get_create_type();			

	// 입력받은 문제 배점 방식 1번 인지 확인
	if(num == 1) 
	{
		// 빈칸문제점수와 프로그램 문제 점수 받기
		printf("Input value of blank question : ");
		scanf("%lf", &bscore);
		printf("Input value of program question : ");
		scanf("%lf", &pscore); 
	}

	// opendir->readdir 을 통해 정답 문제 파일 불러오기 + score_table 에 문제번호 저장
	if((dp = opendir(ansDir)) == NULL){ 
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}	

	while((dirp = readdir(dp)) != NULL) 
	{
		sprintf(tmp, "%s/%s", ansDir, dirp->d_name);
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) 
			continue;

		if((type = get_file_type(dirp->d_name)) < 0) // 문제 번호 타입이 맞는지 확인 (.txt, .c)
			continue;

		strcpy(score_table[idx++].qname, dirp->d_name); 

	}
	closedir(dp);
	sort_scoreTable(idx); // 문제 번호 정렬

	for(i = 0; i < idx; i++) 
	{
		//문제 번호로 
		type = get_file_type(score_table[i].qname);  // 문제 번호 타입이 맞는지 확인 (.txt, .c)

		if(num == 1) // 1번 방식으로 배점할 시 일괄처리
		{
			if(type == TEXTFILE) 
				score = bscore;
			else if(type == CFILE) 
				score = pscore;
		}
		else if(num == 2)  // 2번 방식으로 배점할 시 대화식처리
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score);
		}

		score_table[i].score = score; // score_table 갱신
	}
}

// score_table (문제 정답배점) csv 파일 생성함수
void write_scoreTable(char *filename) 
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]); 

	//  filename에 score_table 생성
	if((fd = creat(filename, 0666)) < 0){ 
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++)
	{
		// 해당 배점 중 0점이 존재하면 break.
		if(score_table[i].score == 0) 
			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score);
		write(fd, tmp, strlen(tmp));
	}

	close(fd);
}

// 학번 디렉토리 순회하면서 학번 테이블 id_table 생성
void set_idTable(char *stuDir) 
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

	//전형적인 opendir->readdir 로 해당 파일 모두 순회
	if((dp = opendir(stuDir)) == NULL){ 
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}

	while((dirp = readdir(dp)) != NULL){ 
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name); 
		stat(tmp, &statbuf); 

		// 만약 디렉토리면 학번이므로 id_table에 저장, 일반파일은 pass
		if(S_ISDIR(statbuf.st_mode)) 
			strcpy(id_table[num++], dirp->d_name); 
		else
			continue;
	}

	sort_idTable(num);
}

// 학번 버블 정렬 (aescend)
void sort_idTable(int size) 
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

// 문제번호로 문제번호 버블정렬함수
void sort_scoreTable(int size)
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for(i = 0; i < size - 1; i++) { 
		for(j = 0; j < size - 1 - i; j++) {
			get_qname_number(score_table[j].qname, &num1_1, &num1_2);
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2);

			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){ 
				memcpy(&tmp, &score_table[j], sizeof(score_table[0]));
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			} 
		}
	}
}

// -, . 으로 문제 번호 구분하는 함수
void get_qname_number(char *qname, int *num1, int *num2) 
{
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname)); 
	*num1 = atoi(strtok(dup, "-."));
	
	p = strtok(NULL, "-.");
	if(p == NULL)  
		*num2 = 0;
	else 
		*num2 = atoi(p); 
}

// 정답 테이블 점수 받을 방법 선택받기
int get_create_type() 
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
	double score2 = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]); // id_table 테이블 데이터 개수
	int print_check = 0;		// c, p 옵션 학생 수

	if((fd = creat(csvDir, 0666)) < 0){			// n 옵션부터 만들 것!
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	StdList* stdL = NULL;
	Snode* stdnode = NULL;
		// s 옵션, p 옵션 존재 시 리스트 생성
	if (sOption || pOption)
	{
		stdL = new_stdlist();
	}

	// 레코드 타입 생성
	if (!sOption)
		write_first_row(fd); 


	for(num = 0; num < size; num++)
	{
		// 학생 테이블이 없으면 break
		if(!strcmp(id_table[num], "")) 
			break;

		if (sOption || pOption)
		{
			stdnode = append_list(stdL, id_table[num]);
		}

		if (!sOption)
		{
			sprintf(tmp, "%s,", id_table[num]); 
			write(fd, tmp, strlen(tmp));  // mOption 없는 경우에는 바로 출력
		}

		if (cOption)
		{	
			for (int i = 0 ; i < sizeof(iIDs) / sizeof(iIDs[0]) ; i++)
			{
				if (!strcmp(id_table[num], iIDs[i]))
				{
					print_check += 1;
					continue;
				}
			}
		}

		int tmp_score = score_student(fd, id_table[num], stdnode); // 빈칸문제 계산
		if (cOption)
		{
			if (print_check == 0)
				score2 += tmp_score;
			else
			{
				for (int i = 0 ; i < sizeof(iIDs) / sizeof(iIDs[0]) ; i++)
				{
					if (!strcmp(id_table[num], iIDs[i]))
					{
						score2 += tmp_score;
						continue;
					}
				}
			}
		}	



		score += tmp_score;
	}

	// cOption 존재 시 아래에 정렬 함수 + for 문 출력함수 생성
	if (cOption)
	{
		if (print_check == 0)
			printf("Total average : %.2f\n", score / num);
		else
			printf("Total average : %.2f\n", score2 / print_check);
	}
	printf("result saved.. (%s)\n", csvDir);
	if (eOption)
		printf("error saved.. (%s)\n", errorDir);

	if (sOption)
	{
		sort_manager(stdL, sortType[0], sortType[1]);
		write_sort(stdL, fd);
	}
	close(fd);
}

// 학생 답안 채점 함수
double score_student(int fd, char *id, Snode* std_node) 
{
	int type;
	double result; 
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); // score_table 데이터 개수

	if ((pOption || sOption) && std_node == NULL)	/// 노드 존재X시 긴급종료
	{
		fprintf(stderr, "Your node is empty\n");
		exit(1);
	}

	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname);
		
		if(access(tmp, F_OK) < 0) 
			result = false;
		else
		{
			if((type = get_file_type(score_table[i].qname)) < 0) 			// 파일 타입 가져오기
				continue;
			
			// 빈칸문제인지 프로그램 문제인지 검사
			if(type == TEXTFILE) 
				result = score_blank(id, score_table[i].qname);
			else if(type == CFILE)
				result = score_program(id, score_table[i].qname);
		}

		if(result == false) // 매겨보니까 0점 (틀리면)이면?
		{
			if (!sOption)
				write(fd, "0,", 2); 
			
			if (sOption || pOption)	 // 틀렸으니까 당연리스트연결
			{
				append_score(std_node, score_table[i].qname, 0, score_table[i].score); 
			}
		}
		else{
			if(result == true){ // 맞은경우
				score += score_table[i].score; /// 총점에 추가
				sprintf(tmp, "%.2f,", score_table[i].score); 
				
				if (sOption)	// 맞은 경우는 sOption 에만 연결
				{
					append_score(std_node, score_table[i].qname, score_table[i].score, score_table[i].score); 
				}
			}
			else if(result < 0){ // Warning 으로 인하여 점수가 깎인경우
				double tmp_score = score_table[i].score + result; /// 워닝 감점 추가.
				if (score < 0)		// 감점된 점수가 0점 밑인 경우
					tmp_score = 0;
				score += tmp_score;
				sprintf(tmp, "%.2f,", tmp_score);

				if (sOption || pOption)	// 워닝 경우에는 sOption, pOption 모두 연결
				{
					append_score(std_node, score_table[i].qname, tmp_score, score_table[i].score); 
				}
			}
			if (!sOption)
				write(fd, tmp, strlen(tmp));
		}
	}

	if (!cOption && !pOption)
		printf("%s is finished..\n", id); // 결과 출력
	else if (cOption && !pOption)
	{
		if (iIDs_cnt == 0)
			printf("%s is finished.. score : %.2f\n", id, score); // 최종 결과 출력
		else
		{
			int print_check = 0;
			for (int i = 0 ; i < sizeof(iIDs) / sizeof(iIDs[0]) ; i++)
			{
				if (!strcmp(id, iIDs[i]))
				{
					// 출력함수 넣을 것.
					printf("%s is finished.. score : %.2f\n", id, score); // 결과 출력
					print_check = 1;
					continue;
				}
			}

			if (!print_check)
				printf("%s is finished..\n", id); // 결과 출력
		}
	}
	else if (!cOption && pOption)
	{
		if (iIDs_cnt == 0)
		{
			printf("%s is finished.. ", id); // 결과 출력
			print_wrongL(std_node);
		}
		else
		{
			int print_check = 0;
			for (int i = 0 ; i < sizeof(iIDs) / sizeof(iIDs[0]) ; i++)
			{
				if (!strcmp(id, iIDs[i]))
				{
					// 출력함수 넣을 것.
					printf("%s is finished.. ", id); // 결과 출력
					print_wrongL(std_node);
					print_check = 1;
					continue;
				}
			}

			if (!print_check)
				printf("%s is finished..\n", id); // 결과 출력
		}	
	}
	else if (cOption && pOption)
	{
		if (iIDs_cnt == 0)
		{
			printf("%s is finished.. score : %.2f, ", id, score);  
			print_wrongL(std_node);
		}
		else
		{
			int print_check = 0;
			for (int i = 0 ; i < sizeof(iIDs) / sizeof(iIDs[0]) ; i++)
			{
				if (!strcmp(id, iIDs[i]))
				{
					// 출력함수 넣을 것.
					printf("%s is finished.. score : %.2f, ", id, score); 
					print_wrongL(std_node);
					print_check = 1;
					continue;
				}
			}
			if (!print_check)
				printf("%s is finished..\n", id); // 결과 출력
		}
	}

	sprintf(tmp, "%.2f\n", score);
	if (!sOption)
	{
		write(fd, tmp, strlen(tmp)); // score.csv의 마지막 열에 데이터를 작성
	}
	if (sOption)
	{
		std_node->sums = score;
		append_score(std_node, "sum", score, score); 	// 총점 데이터 추가.
	}

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

// 빈칸 텍스트 채점
int score_blank(char *id, char *filename)
{
	// make_tokens 함수로 받아올 토큰
	char tokens[TOKEN_CNT][MINLEN]; 
	node *std_root = NULL, *ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN]; 
	char qname[FILELEN] = { 0 }; 
	int fd_std, fd_ans; 
	int result = true; 
	int has_semicolon = false; // 다수 정답 존재여부 확인

	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); // .txt 부분 삭제

	// 빈칸 문제 채점
	sprintf(tmp, "%s/%s/%s", stuDir, id, filename); 
	fd_std = open(tmp, O_RDONLY); 
	strcpy(s_answer, get_answer(fd_std, s_answer)); //학생이 적은 답 가져오기

	// 빈칸이면 0점
	if(!strcmp(s_answer, "")) { 
		close(fd_std);
		return false;
	}

	// 괄호 짝이 안맞으면 0점
	if(!check_brackets(s_answer)){ 
		close(fd_std);
		return false;
	}

	//좌우 공백 삭제
	strcpy(s_answer, ltrim(rtrim(s_answer))); 

	// 끝에 ; 존재하면 ; 삭제
	if(s_answer[strlen(s_answer) - 1] == ';'){ 
		has_semicolon = true;
		s_answer[strlen(s_answer) - 1] = '\0'; 
	}

	// 학생 정답 lexeme 을 토큰 단위로 분리 해서 배열저장 오류 발생시 0 return
	if(!make_tokens(s_answer, tokens)){
		close(fd_std);
		return false;
	}

	
	idx = 0; 
	// 학생의 토큰으로 파스트리 생성
	std_root = make_tree(std_root, tokens, &idx, 0); 

	
	//정답 가져오기
	sprintf(tmp, "%s/%s", ansDir, filename); 
	fd_ans = open(tmp, O_RDONLY)

	while(1)
	{
		//정답 트리의 루트노드과 될 예정
		ans_root = NULL;
		result = true; 

		//토큰 초기화
		for(idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx]));

		//정답 가져오기
		strcpy(a_answer, get_answer(fd_ans, a_answer)); 

		//정답이 없으면 멈추기
		if(!strcmp(a_answer, "")) 
			break;

		// 좌우공백 삭제
		strcpy(a_answer, ltrim(rtrim(a_answer))); 

		// 끝에 ; 있으면 일단 넘어감
		if(has_semicolon == false){ 
			if(a_answer[strlen(a_answer) -1] == ';')  
				continue;
		}

		// 끝에 ; 존재하면 지우고 \0 으로 문자열화
		else if(has_semicolon == true) 
		{
			if(a_answer[strlen(a_answer) - 1] != ';') 
				continue;
			else
				a_answer[strlen(a_answer) - 1] = '\0'; 
		}

		//정답지 토큰 생성
		if(!make_tokens(a_answer, tokens))
			continue;

		idx = 0;
		// 정답 파스트리 생성
		ans_root = make_tree(ans_root, tokens, &idx, 0); 

		// 학생 답이랑, 정답지랑 비교
		compare_tree(std_root, ans_root, &result);

		// 학생트리와 정답트리가 동일할 경우 true return 하고 종료
		if(result == true){
			close(fd_std);
			close(fd_ans);

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

// 프로그램 문제 점수 매기는 함수 --> 컴파일해서 에러면 0점, 경고 시 경고 개수만큼 return
double score_program(char *id, char *filename) 
{
	double compile;
	int result;
	//컴파일
	compile = compile_program(id, filename); 

	// 에러면 0 점 리턴
	if(compile == ERROR || compile == false)
		return false;
	
	//프로그램 실행 해서 결과 담기
	result = execute_program(id, filename); 

	// 에러 존재 시 0 점
	if(!result)
		return false;

	//경고 시 경고 개수 return
	if(compile < 0) 
		return compile;

	return true;
}

// 스레드 문제인지 검사
int is_thread(char *qname)
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]); 

	//threadFiles 에 문제가 있으면 true
	for(i = 0; i < size; i++){
		if(!strcmp(threadFiles[i], qname)) 
			return true;
	}
	return false; 
}

// 프로그램 문제 컴파일 함수
double compile_program(char *id, char *filename)
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN] = { 0 };
	int isthread;
	off_t size;
	double result;
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));
	
	//스레드 사용 문제인지 확인
	isthread = is_thread(qname); 
	
	//실행파일과 c파일 받아옴.
	sprintf(tmp_f, "%s/%s", ansDir, filename); 
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname); 
	if(tOption && isthread) // -t 와 해당 문제가 스레드이용문제가 맞으면 -lpthread 사용
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else 
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_e, "%s/%s_error.txt", ansDir, qname);
	fd = creat(tmp_e, 0666);
	
	// 에러 발생 문구를 에러 error.txt 저장 (임시파일)
	redirection(command, fd, STDERR);
	size = lseek(fd, 0, SEEK_END); // 에러 파일 크기
	close(fd); 
	unlink(tmp_e); //저장한 에러파일 삭제

	//컴파일된 파일에 에러가 존재 시 종료
	if(size > 0) 
		return false;

	// 학생 파일 컴파일 진행 위와 동일 (아래 계속 동일)
	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename); 
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname); 

	if(tOption && isthread) 
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else 
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname); 
	fd = open(tmp_f, O_RDWR | O_CREAT | O_TRUNC, 0666); // 학생 에러파일 존재시 밀고 다시 작성

	redirection(command, fd, STDERR); //에러 파일 리다이렉션 (학생 디렉토리에 저장)
	size = lseek(fd, 0, SEEK_END); //에러파일 저장
	close(fd);

	//해당 에러파일에 에러 파일이 존재하는 경우! e옵션 있으면 에러 경로에 파일 저장.
	if(size > 0){ 
		if(eOption) 
		{
			sprintf(tmp_e, "%s/%s", errorDir, id); //에러 디렉토리에 학생 학번 저장 <errorDir>/id
			
			realpath(tmp_e, tmp_e); //상대경로->절대경로화 (realpathS 로 처리해주긴함.)
			
			if(access(tmp_e, F_OK) < 0) //디렉토리가 없으면 디렉토리 생성
				mkdir(tmp_e, 0755);

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname); //<errDir> 로 에러파일 이동
			rename(tmp_f, tmp_e); // 원래 <stdDIR>에 있던 에러파일을 errDir로 옮기기

			result = check_error_warning(tmp_e); // 에러면 0, 경고면 에러개수 저장
		}
		else{ 
			result = check_error_warning(tmp_f); // 에러면 0, 경고면 에러개수 저장
			unlink(tmp_f); // errorDir에 저장하니까 원래 위치 삭제
		}

		return result; // 에러면 0, 경고면 에러개수 리턴
	}

	unlink(tmp_f); // STD_DIR/2020XXXX/X_error.txt 삭제
	return true;
}

//컴파일 에러나 워닝 개수 함수
double check_error_warning(char *filename) 
{
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;

	if((fp = fopen(filename, "r")) == NULL){ 
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	//에러 발생시 바로 0 return
	while(fscanf(fp, "%s", tmp) > 0){ 
		if(!strcmp(tmp, "error:"))
			return ERROR;
		else if(!strcmp(tmp, "warning:")) //워닝 존재 시 개수 추합
			warning += WARNING;
	}

	return warning;
}

// 프로그램 실행 함수 
int execute_program(char *id, char *filename) 
{
	char std_fname[BUFLEN], ans_fname[BUFLEN];
	char tmp[BUFLEN];
	char qname[FILELEN] = { 0 }; 
	time_t start, end;	// 프로그램 시간 기록
	pid_t pid; 
	int fd; /

	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));

	// 정답 디렉토리에서 프로그램 일단 실행
	sprintf(ans_fname, "%s/%s.stdout", ansDir, qname); 
	fd = creat(ans_fname, 0666);

	//실행결과를 리다이렉션 후 복귀
	sprintf(tmp, "%s/%s.exe", ansDir, qname); 
	redirection(tmp, fd, STDOUT); 
	close(fd);

	// 학생 답안지 실행 <STD_DIR>/학번/문제번호.stdexe
	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname);
	fd = creat(std_fname, 0666);

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname);	// 백그라운드 실행

	start = time(NULL); // 시작 시간 기록
	redirection(tmp, fd, STDOUT);
	
	sprintf(tmp, "%s.stdexe", qname);
	while((pid = inBackground(tmp)) > 0){ 
		end = time(NULL);

			// 실행시간 5초 이상 지나면 스레드 죽임
		if(difftime(end, start) > OVER){ 
			kill(pid, SIGKILL); // pid 를 일단 죽임
			close(fd); 
			return false;
		}
	}

	close(fd);

	//실행결과 같으면 1 다르면 0
	return compare_resultfile(std_fname, ans_fname); 
}

// 백그라운드로 실행중인 파일 pid 받아오는 함수 (스레드 실행함수 찾는 함수인듯)
pid_t inBackground(char *name)
{
	pid_t pid; 
	char command[64]; 
	char tmp[64] = { 0 }; 
	int fd;
	off_t size;
	
	// 백그라운드 .txt 파일을 존재시 밀고 다시 쓰기
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); 

	// 현재 실행중인 프로세스를 백그라운드가 있는지 확인해서 표준출력하고 다시 복귀
	sprintf(command, "ps | grep %s", name);
	redirection(command, fd, STDOUT); // command 실행 후 결과(학생디렉토리 해당문제에 리다이렉션 후) background.txt파일에 저장

	// 백그라운드 파일을 맨 앞으로 되돌리기.
	lseek(fd, 0, SEEK_SET); 
	read(fd, tmp, sizeof(tmp));

	// 백그라운드 파일에 아무것도 없으면 return 0;
	if(!strcmp(tmp, "")){ 
		unlink("background.txt");
		close(fd);
		return 0;
	}

	// 실행중인 프로세스 pid로 받아오기
	pid = atoi(strtok(tmp, " ")); 
	close(fd);

	unlink("background.txt"); //다썼으니까 삭제 
	return pid;
}


// file1 , file2 실행결가 같으면 1 다르면 0
int compare_resultfile(char *file1, char *file2)
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY); 
	fd2 = open(file2, O_RDONLY); 

	// 한 글자씩 실행결과를 비교해서 같은지 틀린지 비교
	while(1)
	{
		while((len1 = read(fd1, &c1, 1)) > 0){ 
			if(c1 == ' ') 
				continue;
			else 
				break;
		}
		while((len2 = read(fd2, &c2, 1)) > 0){ /
			if(c2 == ' ') 
				continue;
			else 
				break;
		}
		
		if(len1 == 0 && len2 == 0) 
			break;

		to_lower_case(&c1); 
		to_lower_case(&c2);

		//두 실행결과가 다르면 false 리턴
		if(c1 != c2){ 
			close(fd1);
			close(fd2);
			return false; 
		}
	}
	close(fd1);
	close(fd2);
	return true; 
}

// new를 old 디스크립터로 출력(system) 하고 다시 복귀, old:표준입출력
void redirection(char *command, int new, int old) 
{
	int saved;

	saved = dup(old); /// old 를 복사한 디스크립터를 받아옴
	dup2(new, old); // new -> old 디스크럽터 복사

	system(command);

	dup2(saved, old); /// saved -> old 디스크럽터 복사 (다시 old 위치에 old가 가도록)
	close(saved);
}

// 파일이 .txt인지 .c 인지 구분
int get_file_type(char *filename)
{
	char *extension = strrchr(filename, '.');

	if(!strcmp(extension, ".txt"))
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))
		return CFILE;
	else
		return -1;
}

//path 까지 디렉토리 제거함수
void rmdirs(const char *path) 
{
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[BUFLEN] = { 0 };
	
	if((dp = opendir(path)) == NULL)
		return;

	// 디렉토리 재귀 제거
	while((dirp = readdir(dp)) != NULL)
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", path, dirp->d_name); 

		if(lstat(tmp, &statbuf) == -1) 
			continue;

		//디렉토리면 재귀적으로 제거
		if(S_ISDIR(statbuf.st_mode))
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
	printf(" -n <CSVFILENAME>\n");
	printf(" -m\n");
	printf(" -c [STUDENTIDS ...]\n");
	printf(" -p [STUDENTIDS ...]\n");
	printf(" -t [QNAMES ...]\n");
	printf(" -s <CATEGORY> <1|-1>\n");
	printf(" -e <DIRNAME>\n");
	printf(" -h\n");
}


// 틀린문제 출력함수.
void print_wrongL (Snode* node)
{
    if (node == NULL)
        return;
    
    printf("wrong problem : ");
    int comma = 0;
    for (Sclist* tmp_q = node->list_head ; tmp_q != NULL ; tmp_q = tmp_q->file_next)
    {
        if (tmp_q->cur_score != tmp_q->score) //문제 원배점이랑 맞지 않으면 틀린문제로 간주
        {
            if (comma != 0)
                printf(", ");
            printf("%s(%.2f)", tmp_q->qname, tmp_q->score);
            comma++;
        }
    }
    printf("\n");
}

// list 파일을 csv 파일로 출력해주는 함수
int write_sort (StdList* list, int fd)
{
	if (list == NULL)
		return false;
		
    char tmp[20];
    write_first_row(fd);

	//list를 순회하며 정렬순서에 맞게 리스트 출력
    for (Snode* id_tmp = list->head ; id_tmp != NULL ; id_tmp = id_tmp->next)
    {
        sprintf(tmp, "%s,", id_tmp->id_name);
        if (write(fd, tmp, strlen(tmp)) != strlen(tmp))
        {
            return false;
        }
        for (Sclist* q_tmp = id_tmp->list_head ; q_tmp != NULL ; q_tmp = q_tmp->file_next)
        {
            sprintf(tmp, "%.2f", q_tmp->cur_score);
			if (q_tmp->file_next != NULL)
				strcat(tmp, ",");
			else
				strcat(tmp, "\n");
            if (write(fd, tmp, strlen(tmp)) != strlen(tmp))
            {
                return false;
            }
        }
    }
    return true;
}


/**
 * type  0:학번, 1:총점수 sums
 * updown -1:내림차순, 1:오름차순
*/
void sort_manager(StdList* list, int type, int updown)
{
    if (list == NULL)
        return;

    if (type == 0 && updown == -1)
        sort_descentI(list);
    else if (type == 0 && updown == 1)
        sort_aescendI(list);
    else if (type == 1 && updown == -1)
        sort_descentS(list);
    else if (type == 1 && updown == 1)
        sort_aescendS(list);
}

void sort_aescendI(StdList* list)
{

    Snode* swap_tmp = NULL;
    for (int x = 0 ; x < list->id_cnt-1 ;x++)
    {
        Snode* par = NULL;
        Snode* tmp = list->head;
        for (int y = 0 ; y < list->id_cnt-x-1 ; y++)
        {
            if (atoi(tmp->id_name) > atoi(tmp->next->id_name))
            {
                swap_tmp = tmp->next; //swaping 이 일어나면 next가 바뀌는 문제 해결 위함
                swap_list(list, par, tmp, tmp->next);
                tmp = swap_tmp;
            }
            par = tmp;
            tmp = tmp->next;
        }
    }
}

void sort_descentI(StdList* list)
{

    Snode* swap_tmp = NULL;
    for (int x = 0 ; x < list->id_cnt-1 ;x++)
    {
        Snode* par = NULL;
        Snode* tmp = list->head;
        for (int y = 0 ; y < list->id_cnt-x-1 ; y++)
        {
            if (atoi(tmp->id_name) < atoi(tmp->next->id_name))
            {
                swap_tmp = tmp->next; //swaping 이 일어나면 next가 바뀌는 문제 해결 위함
                swap_list(list, par, tmp, tmp->next);
                tmp = swap_tmp;
            }
            par = tmp;
            tmp = tmp->next;
        }
    }
}

void sort_aescendS(StdList* list)
{

    Snode* swap_tmp = NULL;
    for (int x = 0 ; x < list->id_cnt-1 ;x++)
    {
        Snode* par = NULL;
        Snode* tmp = list->head;
        for (int y = 0 ; y < list->id_cnt-x-1 ; y++)
        {
            if (tmp->sums > tmp->next->sums)
            {
                swap_tmp = tmp->next; //swaping 이 일어나면 next가 바뀌는 문제 해결 위함
                swap_list(list, par, tmp, tmp->next);
                tmp = swap_tmp;
            }
            par = tmp;
            tmp = tmp->next;
        }
    }
}


void sort_descentS(StdList* list)
{

    Snode* swap_tmp = NULL;
    for (int x = 0 ; x < list->id_cnt-1 ;x++)
    {
        Snode* par = NULL;
        Snode* tmp = list->head;
        for (int y = 0 ; y < list->id_cnt-x-1 ; y++)
        {
            if (tmp->sums < tmp->next->sums)
            {
                swap_tmp = tmp->next; //swaping 이 일어나면 next가 바뀌는 문제 해결 위함
                swap_list(list, par, tmp, tmp->next);
                tmp = swap_tmp;
            }
            par = tmp;
            tmp = tmp->next;
        }
    }
}


/**
 * 버블 정렬을 할 것이기 때문에, p->a->b->next 형태를 무조건 띄고 있음
 *  p->next = b
 *  a->next = b->next
 *  b->next = a
 *  를하면됨
*/
int swap_list (StdList* list, Snode* p, Snode* a, Snode* b)  /// 버블 정렬에 필요한 a <-> b 링크드리스트 구조 변경 (a, b 링크는 인접함.)
{
    if (a == NULL || b == NULL)
        return false;
    
    if (list->head == a && p == NULL) // 리스트의 head와 p 가 동일하면.. (+p == NULL)
    {
        a->next = b->next;
        list->head = b;
        b->next = a;
        return true;
    }
    else
    {
        a->next = b->next;
        p->next = b;
        b->next = a;
        return true;
    }
}

double find_score (char* qname)
{
    for (int i = 0 ; i < QNUM ; i++)
    {
        if (!strcmp(score_table[i].qname, qname))
        {
            return score_table[i].score;
        }
    }
    return -1;
}


void print_score (Snode* node)
{
    if (node == NULL)
        return;
    printf("------\n");    
    for (Sclist* tmp = node->list_head ; tmp != NULL ; tmp = tmp->file_next)
    {
        printf("[%s] score = %0.2f, cur_score = %0.2f\n", tmp->qname, tmp->score, tmp->cur_score);
    }
    printf(">>> total question count = %d\n", node->file_cnt);
    printf("------\n");
}

int append_score(Snode* node, char* qname, double cur_score, double score)
{
    if (node == NULL)
        return false;
    
    Sclist* newnode;
    
    if (score == 0) // 문제 찾아서 넣는 함수 제작 필요.
    {
        newnode  = new_sclists(qname, cur_score);
        newnode->score = find_score(qname);
    }
    else
        newnode = new_sclistss(qname, cur_score, score);

    
    if (newnode == NULL)
        return false;

    if (node->list_head == NULL)
    {
        node->list_head = node->list_tail = newnode;
        node->file_cnt++;
        return true;
    }
    else
    {
        node->list_tail->file_next = newnode;
        node->list_tail = newnode;
        node->file_cnt++;
        return true;
    }
}


void print_list (StdList* list)
{
    printf("--------------<start printing node>---------------\n");
    for (Snode* tmp = list->head ; tmp != NULL ; tmp = tmp->next)
    {
        printf("%s\n", tmp->id_name);        
        printf("sums : %f\n", tmp->sums);
    }
    printf(">>> total id_cnt = %d\n", list->id_cnt);
    printf("--------------<end printing node>---------------\n");

}

Snode* append_list (StdList* list, char* id)
{
    if (list == NULL)
        return NULL;

    Snode* newnode = new_stdnode(id);
    if (newnode == NULL)
        return NULL;
    
    if (list->head == NULL)                 /// 헤드가 NULL이면 초기연결
    {
        list->head = list->tail = newnode;
        list->id_cnt++;
        return newnode;
    }
    else                                    ///아닐 시, tail 추가연결
    {
        list->tail->next = newnode;
        list->tail = newnode;
        list->id_cnt++;
        return newnode;
    }

}


// sc 리스트 생성자
Sclist* new_sclists (char* qname, double curS)
{
    Sclist* node = (Sclist*)malloc(sizeof(Sclist));
    strcpy(node->qname, qname);
    node->cur_score = curS;
    node->score = 0;
    node->file_next = NULL;

    return node;
}


//sc list에 문제번호에 해당하는 원점수까지 할당
Sclist* new_sclistss (char* qname, double curS, double s)
{
    Sclist* node = (Sclist*)malloc(sizeof(Sclist));
    strcpy(node->qname, qname);
    node->cur_score = curS;
    node->score = s;
    node->file_next = NULL;

    return node;
}

//학생노드 생성자
Snode* new_stdnode (char* id)
{
    Snode* node = (Snode*)malloc(sizeof(Snode));
    strcpy(node->id_name, id);

    node->file_cnt = 0;
    node->sums = 0;
    node->next = NULL;
    node->list_head = node->list_tail = NULL;

    return node;
}

//학생 리스트 생성자
StdList* new_stdlist()
{
    StdList* list = (StdList*)malloc(sizeof(StdList));
    list->head = list->tail = NULL;
    list->id_cnt = 0;

    return list;
}
