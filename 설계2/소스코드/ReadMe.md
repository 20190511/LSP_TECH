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
