#ifndef MAIN_H_
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


// 04.29 상대경로-->절대경로화 해주는 링크드리스트 (토큰배열받아와서 돌릴예정.)
typedef struct pn {

    char path[MAXPATHLEN];
    struct pn *prev;
    struct pn *next;
}pathNode;


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
void get_qname_number(char *qname, int *num1, int *num2); //, . 으로 문제 구분하는 함수
int do_nOption (char* pathname); 
int realpathS(char* pathname, size_t size); //초기 상대경로 절대경로화 함수 (이제안씀 링크드리스트기반으로 변경)
int csv_check (char* pathname);  //csv 파일인지 체크용 (n옵션전용)
int mkdirs(char* pathname);							//디렉토리 생성함수.
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
char** path_arr(char* str); // str을 / 기준으로 토큰배열화
int realpathS2 (char *str); // 링크드리스트기반 경로찾기 함수

#endif
