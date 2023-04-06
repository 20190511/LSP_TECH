	- c 옵션 설계
	- 
	1. cOption = 1 설정
	2. 인자로 받은 [STUDENTS …] 를 -t 옵션처리한것처럼, 개별적으로, t checkOpt (StdList* list, char* id); 처리
		a. 실패시 즉시종료 (학번이 없는경우)
		b. 성공시 double score_student(int fd, char *id) 실행할 때, 해당 Node에 print_opts 가 있는지 확인 후 있으면 sums 출력
			i. 실제 설계시, cOption, pOption, sOption 이외에는 id_name 만 링킹되어있고, 
			ii. 자세히는 c,p 옵션에서 학번을 받는경우 5개의 학번만 StdNode가 모두 채워질 예정
![image](https://user-images.githubusercontent.com/70988272/230394851-a50ea54d-6700-4518-b45c-8ac6b79db163.png)
-s 옵션 설계 (score_table도 리스트로 만들어야되면, 손봐야함) --> 링크드리스트 기반
 --> 현재 정렬된 파일리스트를 전부 쪼개서 임시적으로 동적 테이블 (StdNode** remake_table);
에 담아서 정렬진행 후,  해당 테이블 첫 요소부터 append 해준다. 
그리고, 해당 노드들에서 score_table 과 각 노드의 q_name을 비교 하면서, score_table과 겹치는 값이 있으면 리스트의 값을 출력하고, score_table과 겹치지 않으면 score_table 값을 csv에 출력,

	1. 테이블 내부에서 옵션에 따라 정렬한다.
	2. 테이블 첫 번째요소부터 
	3. 링크드 리스트로 파일리스트를 전체를 뽑는다.
	4. score_student 내부에서 StdList 내부 StdNode 구조체의 데이터가 채워지도록 한다.
	5. 정렬함수 4개를 만들어서 해당 함수에서 내부적으로 정렬진행
	6. wrtie_stdlist() 호출



 [ 정렬함수 내부 : sorting_stdid_up (StdList* list);]
1. 학생수 만큼 동적배열을 만든다. (StdNode** remake_table);
2. 전체 학생리스트를 pop을 하며 임시적으로 테이블형태로 전환한다.
3. 동적배열을 이용해서 정렬을 한다.
4. 동적 테이블을 0번 테이블부터 ~ n-1번 테이블까지 append 해준다. + 동적테이블 Free
5. 동적테이블을 할당해제하고  함수를 종료한다.

[StdList 기반 파일 출력 함수 : write_stdlist(StdList* list, char* filename)]
1. 해당 filename 을 open한다.
2. write_first_row(fd); 
3. for 문을 돌리면서 파일리스트 내부 값 출력,
	1. node->list_header == NULL || node->wrong_cnt == 0 이면 score_table을 차례대로 출력 [(%.2f,) 형태로]
	2. [아니라면 ssu_scorelist* wrongQ := list->scorelist->list_tail;]
	3. wrongQ 가 NULL 이거나 wrongQ->qname 가 NULL 이면 
	4. score_table[i] 와 wrongQ->qname 이 일치하지 않으면 score_table[i] 출력, next 하지않고 for 문 진행
	5. score_table[i] 와 wrongQ->qname 이 일치하면 wrongQ->cur_score 출력, next하면서 for문 진행
	6. wrongQ->qname 가 NULL 이면 score_table[i] 출력, next 하지않고 for 문 진행

![image](https://user-images.githubusercontent.com/70988272/230394875-3944949f-9fd5-4099-b62f-2806780f11c3.png)

설계과제2 설계도

2023년 4월 5일 수요일
오후 5:34

	0. 기본옵션
		a. - 출제되는 시험 문제는 (1)빈칸 채우기 문제와 (2)프로그램 작성 문제 총 두 가지 종류 
			i. 학생들이 출제되는 시험문제에 대한 답을 작성하여 자신의 학번 디렉토리 안에 모든 문제에 대한 답안 파일을 넣어 제출함 
		b. 빈칸 채우기 문제는 여러 서브문제로 구성가능, 한 문제가 여러개의 하위문제로 이루어질 수 있음. (ex) 1-1.txt, 
		c. 정답 파일도 학생 답안과 동일하게 하나의 디렉토리 안에 문제번호 와 프로그램문제번호 디렉토리 문제번호 문제번호
		d. 출제되는 문제 수는 최대 문제 100 까지 허용 
		e. ssu_score은 링크드 리스트로 파일 리스트를 관리해야 함 --> 옵션 보고 파일리스트 설정. 
			i. (지금 당장생각나는 아이디어) : ssu_scoreTable 구조체를 
			ii. 리눅스 상에서 파일 경로의 최대 크기는 4,096 , 255 바이트이며 파일 이름의 최대 크기는 바이트임 - 
		f. 실행결과 및 에러 메시지를 파일에 출력 시 함수 system() 사용 금지 
		g. - popen(), fork(), exec() 계열 함수 사용 금지 
		h. - getopt() 라이브러리를 사용하여 옵션 처리 권장 
		i. - ssu_score foregroun 은 로만 수행되는 것으로 가정함 수행은 안되는 것으로 함 (& background X


	1. Usage : ssu_score <STD_DIR> <ANS_DIR> [OPTION]
		a. <STD_DIR> : 학생 답안디렉토리로 학번 서브디렉토리 포함
		b. <ANS_DIR> : 정답 디렉토리, 문제당 하나의 서브디렉토리 포함
		c. 실행결과
			i. 채점+ 출제 시험문제 프로그램 수행시간 출력 후 프로그램 종료
		d. 채점방법 생략..
			i. 텍스트 파일 : 파일을 비교할 때 다수의 답을 모두 정답으로 채점
				1)  정답 파일에 새로운 답을 추가 및 삭제를 통해 학생들의 답안 을 채점 (확인해볼것)
			ii. 프로그램 파일 : 생략
		e. 점수 테이블 생성 : ./ANS/score_table.csv 이름으로 생성해야함?
		f. 채점결과테이블 새성  면 이름으로 자 “ ./ANS/score.csv” , 
			i. --> 현재는 자기디렉토리에 생성되는듯함. (준) ==> chdir 사용해서 일단 수리

	손봐야되는 핵심 	1. resulte saved (~~~~) 출력하게 해줘야함
		        1. 
		                a. 
		        a. 
		2. open errror 이 계속 뜨는데 확인해봐야할듯.
		3. 현재 자동으로 c 옵션이 구현되어있는듯함, 이것도 손봐줘야함. 
		        a. (이거 옵션 분리부터 해줘야함!)
	설계방식	
	
	
	
옵션 구현
	1. -n <CSVFILENAME>
	: CSVFILENAME 으로 score.csv 생성, 
	주의 	1. 얘도 result saved.. (경로…) 가 있어야함 
		2. <CSVFILENAME> 에 .csv 파일이 아닌경우 에러처리 후 프로그램 종료
		        --> 여튼 .csv 파일 아니면 종료시키니까 NULL 와도 종료시키도록 설계할 예정(A)
		3. 이미 있는 파일인 경우 덮어씌움 (TRUNC 쓸 것)
		
		
		<개인 소견>
		1. 기본 score.csv를 생성하지 말고 <CSVFILENAME>으로 생성 (질문할것)
	설계 방식	
	2. -m
	:점수 테이블 파일 내의 특정 문제 번호의 배점 수정 (이미 구현되어있음)
	주의 	점수 테이블 파일이 존재하지 ㅇ낳을 경우 에러처리 후 프로그램 종료
	설계방식	
	3. -c [STUDENTDS …]
	: 인자로 입력받은 학생들 점수 출력 (현재 기본구현된 상태)
	주의 	 <인자 주의>
		        1. [STUDENTS …] 생략여부
		                a. 생략시    : 모든 학생에 대하여 결과 출력 (현재 옵션구현된 상태)
		                b. 생략X시  :
		        2. <STD_DIR> <ANS_DIR>  생략시
		                a. score.csv 파일을 확인해서 점수 출력 (-n 옵션 없으면)
		                b. score.csv 파일 없으면 에러처리 후 프로그램 종료
		                c. 만약 -n 옵션 올 시 <CSVFILENAME> 을 채점결과 파일로 사용
		                        i. --> (그러니까 -n 옵션이 먼저 수행되어야함 || 그냥 score.csv 위치를 알려주는 것일 뿐일지도..).
		
		        A. 현재 학생들 점수에 대한 평균을 "Total Average"  : <점수평균> 형태로 출력
		        B. 가변인자 개수를 5개로 제한 5개 넘을 시 
		                -->“Maximum Number of Argument Exceeded. :: [STUDENTIDS ...]” 출력 후 결과 반영 X
		        C. 첫 번째 인자로 받은 학번이 존재X || <STD_DIR> <ANS_DIR> 없이 프로그램 실행 시 채점 결과 파일에대한 학번 정보 X
		                --> 에러처리 후 프로그램 종료
		        D. <STD_DIR> <ANS_DIR> 있는데 채점파일 없으면 ( 프로세스가 종료되도록 설정)
		        E. <STD_DIR> <ANS_DIR> 두개 중 1개만 나오는 경우 에러처리
		        F. <STD_DIR> <ANS_DIR> 있는데 -n 옵션사용 --> 독립시행 (score.csv 사용 + 새 파일 생성)
		                a. 에러처리??? -- 명세에 없으니까 에러처리예정. 
		
		<개인 소견>
		        1. -n 옵션을 먼저 만들어야 제작 가능.
		        2. 파일리스트 구현에 신경써야할듯
		        3. 
		                a. -c 뒤에 오는 옵션들은 stduent 구조체로 append 한다.
		                b. 형태를 보아하니, id_table 을 이용해서 
		                c. 
		                d. 학번 리스트를 연결리스트로 뽑고, 입력받은 인자와 keymatching 되는지 확인 후 돌리면될 듯,
		                e. -c 뒤에 입력받는 인자 중 학번 하나라도 없을 시 프로세스 종료
		                f. student 구조체에 -p 옵션들어오면 student 구조체 멤버변수 중 ssu_scoreTable 
		
	4. -p [STUDENTDS …]
	:인자로 넘겨받은 학생들의 틀린문제(점수) 출력
	5. 
	주의 	 <인자 주의 : -c 옵션과 동일> 
		        1. [STUDENTS …] 생략여부
		                a. 생략시    : 모든 학생에 대하여 결과 출력 (현재 옵션구현된 상태)
		                b. 생략X시  : 5개까지 인자제한 (학생들 개각각 채점을 돌려야될 듯함)
		        2. <STD_DIR> <ANS_DIR>  생략시
		                a. score.csv 파일을 확인해서 점수 출력 (-n 옵션 없으면)
		                b. score.csv 파일 없으면 에러처리 후 프로그램 종료
		                c. 만약 -n 옵션 올 시 <CSVFILENAME> 을 채점결과 파일로 사용  
		                --> (그러니까 -n 옵션이 먼저 수행되어야함 || 그냥 score.csv 위치를 알려주는 것일 뿐일지도..).
		        
		        A. (기본적인 에러처리는 -c 와동일하나 같이 사용되는 경우 아래 서술)
		
		        A. -p -c 옵션 같이 나오면 점수 먼저 출력하고 틀린문제(점수) 출력
		        20190002 is finished.. score : 73.5 
		        20190002 is finished.. wrong problem : 1-3(3), 2-2(2), 2-3(2), 3-2(1.5), 3-3(3), 5-1(4), 8(6), 10(5) 
		        20190003 is finished.. score : 72.0 
		        20190003 is finished.. wrong problem : 1-3(3), 2-2(2), 2-3(2), 3-1(3), 3-3(3), 5-1(4), 8(6), 10(5)
		
		        B. -p, -c 옵션 뒤에 -p [STUDENTS] -c [STUDENTS] 이런식으로 오면안됨!! --> 에러처리 후 프로그램 종료
		                a. 단, -t <QNAMES> 기변인자 그룹은 별개취급
		        
		
	
	6. -t [QNAMES … ]
	: <QNAME>을 문제로 문제는 컴파일 시 -lpthread 옵션 추가 
	주의 	1. 기본적으로 -c 옵션과 함께 사용, 단독으로 -t 가 오는경우 역시 -c 옵션처럼 구현하거나 (예외처리) 하면될 듯
		2. <QNAME> 에 없는 인자가 올 경우
		        --> 에러처리 후 프로그램 종료
		3. <QNAME> 가변인자 5개 제한 그 이상 가변인자 입력 시
		        ---> Maximum Number of Argument Exceeded. :: [QNAMES ...]” 출력 후 수행에는 반영하지 않음
		        
		        
		        
		<소견>
		-c, -t , -p 는 동시에 설계해야함! 그래야 안 얽힐 듯,
		
		
	7. -s <CATEGGORY> <1|-1>
	:채점 결과 파일을 <CATEGORY> 상대로 정렬해서 저장
	주의 	 인자 : 1 (오름차순 (작은 순))
		          -1 (내림차순 (큰 순))
		        category는 stdid(학번), scroe(점수) 입력가능
		        
		        A. stdid 나 score 이외의 경우 예외처리
		        B. 1, -1 이외에는 예외처리
		        
		 <개인소견>
		 파일리스트를 관리하는게 아니라, 점수관리니까 교수님 학번 정렬 코드를 살짝 손보면 되지않을까..
		 
		
	8. -e <DIRNAME>
	: <DIRNAME>/학번/문제번호_error.txt 에 에러 메시지 출력
	주의 	<DIRNAME> 는 절대/상대경로 모두 가능
		
		
		<개인소견>
		        -e 옵션은 단독으로 오게해도될듯 (같이오면 예외처리해버려야지)
		                --> -c 옵션 일 때, e 옵션있는지 확인 필요..
		
	9. -h
	: 사용법 출력
	주의 	다른 옵션과 사용시 에러처리
	10. makefile ??

옵션 구현 설계
♠ 기본적으로 <STD_DIR> <ANS_DIR> 존재시,
	table 과 score.csv 파일을 만들고 종료해야한다.
	이들을 관리하는 연결리스트 구조체 제작예정

case1: <STD_DIR> <ANS_DIR> 존재
예외처리 	
중첩사용	 중첩사용 가능.
	case1: 함께 실행되어야하는 set
	
	 [0번그룹] 
	 -m : 가장 먼저 실행되어야함.
	
	 [1번그룹] 
	 -t : --> is_thread 이용해서 옵션받을 때 설정  (이미 처리해둠)
	 -e : --> 이미 구현되어있음… 
	        // 상대경로 --> 절대경로화 안시켜줌 ( getopt realpath()이용하면 될듯)
	 
	 -p, -c,
	
	double score_student(int fd, char *id) 
	void score_students()
	        두개를 손보면 위에 -p, -c, 한꺼번에 구현 가능할듯.
	
	 (채점 파일 (csv) 파일 생성옵션)
	        -n --> 해당 이름으로 chdir ..
	        -s --> sorting 프로그램 생각해둘것
	        
	
단독사용	-h
 다른 옵션같이오면 에러처리



case2: <STD_DIR> <ANS_DIR> 존재X : 이게 핵심이다!
예외처리 	 -e : 에러처리 --> 프롬포트 종료
	 -s : 에러처리 --> 프롬포트 종료 
	 -t : 에러처리 --> 프롬포트 종료
	 -m : 예외처리 --> 프롬포트종료
	        (위 3개는 없으면 아예 종료)
	
	-n 단독으로 오는 경우 에러처리
	        (-c, -p 같이와야 돌아감)
사용가능	-p
	-p -n
	
	-c
	-c -n
	
	-c -p -n
	(순서는 -c -> -p 출력)
	
	
	※ 세부예외처리
	 1. -n <FILE> 에서 File 존재 X (NULL) 시 종료
	 2. -c, -p 둘 중 하나만 인자존재해야함.
	 3. csv 파일 존재 X 시 예외처리 (질문 했는데 답변 못받음)
단독사용	-h (타옵션 같이 사용시 에러)

	- c 옵션 설계
	- 
	1. cOption = 1 설정
	2. 인자로 받은 [STUDENTS …] 를 -t 옵션처리한것처럼, 개별적으로, t checkOpt (StdList* list, char* id); 처리
		a. 실패시 즉시종료 (학번이 없는경우)
		b. 성공시 double score_student(int fd, char *id) 실행할 때, 해당 Node에 print_opts 가 있는지 확인 후 있으면 sums 출력
			i. 실제 설계시, cOption, pOption, sOption 이외에는 id_name 만 링킹되어있고, 
			ii. 자세히는 c,p 옵션에서 학번을 받는경우 5개의 학번만 StdNode가 모두 채워질 예정

-s 옵션 설계 (score_table도 리스트로 만들어야되면, 손봐야함) --> 링크드리스트 기반
 --> 현재 정렬된 파일리스트를 전부 쪼개서 임시적으로 동적 테이블 (StdNode** remake_table);
에 담아서 정렬진행 후,  해당 테이블 첫 요소부터 append 해준다. 
그리고, 해당 노드들에서 score_table 과 각 노드의 q_name을 비교 하면서, score_table과 겹치는 값이 있으면 리스트의 값을 출력하고, score_table과 겹치지 않으면 score_table 값을 csv에 출력,

	1. 테이블 내부에서 옵션에 따라 정렬한다.
	2. 테이블 첫 번째요소부터 
	3. 링크드 리스트로 파일리스트를 전체를 뽑는다.
	4. score_student 내부에서 StdList 내부 StdNode 구조체의 데이터가 채워지도록 한다.
	5. 정렬함수 4개를 만들어서 해당 함수에서 내부적으로 정렬진행
	6. wrtie_stdlist() 호출



 [ 정렬함수 내부 : sorting_stdid_up (StdList* list);]
1. 학생수 만큼 동적배열을 만든다. (StdNode** remake_table);
2. 전체 학생리스트를 pop을 하며 임시적으로 테이블형태로 전환한다.
3. 동적배열을 이용해서 정렬을 한다.
4. 동적 테이블을 0번 테이블부터 ~ n-1번 테이블까지 append 해준다. + 동적테이블 Free
5. 동적테이블을 할당해제하고  함수를 종료한다.

[StdList 기반 파일 출력 함수 : write_stdlist(StdList* list, char* filename)]
1. 해당 filename 을 open한다.
2. write_first_row(fd); 
3. for 문을 돌리면서 파일리스트 내부 값 출력,
	1. node->list_header == NULL || node->wrong_cnt == 0 이면 score_table을 차례대로 출력 [(%.2f,) 형태로]
	2. [아니라면 ssu_scorelist* wrongQ := list->scorelist->list_tail;]
	3. wrongQ 가 NULL 이거나 wrongQ->qname 가 NULL 이면 
	4. score_table[i] 와 wrongQ->qname 이 일치하지 않으면 score_table[i] 출력, next 하지않고 for 문 진행
	5. score_table[i] 와 wrongQ->qname 이 일치하면 wrongQ->cur_score 출력, next하면서 for문 진행
	6. wrongQ->qname 가 NULL 이면 score_table[i] 출력, next 하지않고 for 문 진행


전 학번 파일리스트를 아래와같이 추출하도록 설계? (질문중 -- 대답에 따라 리스트형태는 바뀔 수 있음.)
	- m옵션 사용시, 에러파일 생성시, pathname 에 해당하는 text파일 파일리스트 append.
	
	
 2. 옵션 연산 리스트구조체 (stduent1->student2-> … ->student n 이런식으로 연산 예정
// 만점 기준으로 조금이라도 점수가 깎이면 틀린 문제로 설정,
 [StdList 구조체]
	1. StdNode* head;   // head -> …. -> tail 형태로 제작하므로 꼬리부분
	2. StdNode* tail;     // 머리부분
	3. StdNode* id_cnt; // 학생 총 수
	4. int detailM         // 0: id_name 만 채움, 1: cOption, pOption, sOption 일 때 설정하고 StdNode 전 데이터를 채움

 [StdNode 구조체]
	1. char id_name [10];
	2. double sums    //학생 총점
	3. int print_opts      //  0:미지정, 1:지정. --> 지정되면 아래 wrong_cnt, 리스트 사용.
	4. int wrong_cnt   // 총 틀린개수.
	5. ssu_scorelist* list_header;   // header->..->..->tail->NULL;
	6. ssu_scorelist* list_tail;


[ssu_scorelist]
char qname[FILELEN];    // 문제번호
int cur_score;          // 현 점수
int score;              // 원래 배점
ssu_scorelist* next;    // 다음 노드




링크드 리스트 관리함수
int append (StdList* list, char* pathname);          // pathname으로 노드를 할당해서 연결
int append_node (StdList* list, StdNode* node);   // Node 를 할당하지 않고, 기존 노드 연결
StdNode* pop(StdList* list);   // 맨 앞 요소를 삭제하고, 해당 노드 반환 --> s옵션에서 리스트 재구성할 때 사용,
void free_pop(StdList* list);    // free 해주면서 pop
void print_node(StdList* list);

int checkOpt (StdList* list, char* id);     //list 링크드리스트에 일치하는 id 있으면 해당노드 print_opts=1; 처리,

■ 추가 생각.
	1. score_table 도 링크드리스트로 만들어야하는가? (제일중요)
	2. 학번정렬 알고리즘을 이용해서 id_table을 손을 볼 수 있을 수도 있을지도..
		void sort_idTable(int size) // 학번 테이블 정렬: 오름차순
		void set_idTable(char *stuDir) // 학번 테이블 생성
		void score_students() // score.csv 생성

 설계방향 : 유기적으로 -c옵션과 연결된 추가옵션들이 많은것 같다.
void ssu_score(int argc, char *argv[]) 를 기본적인 프레임 함수로 설정하고
	1. 기본적으로 각 옵션에 대히여, do_[]Option () 함수를 만들되,
	2. do_cOption() 의 경우



![image](https://user-images.githubusercontent.com/70988272/230395061-2dbb4cfd-fb8f-49be-8f8d-a193bac3b047.png)

