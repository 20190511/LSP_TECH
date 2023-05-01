#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "blank.h"

/// 자료형을 정리해둔 배열 datatype
char datatype[DATATYPE_SIZE][MINLEN] = {"int", "char", "double", "float", "long"
			, "short", "ushort", "FILE", "DIR","pid"
			,"key_t", "ssize_t", "mode_t", "ino_t", "dev_t"
			, "nlink_t", "uid_t", "gid_t", "time_t", "blksize_t"
			, "blkcnt_t", "pid_t", "pthread_mutex_t", "pthread_cond_t", "pthread_t"
			, "void", "size_t", "unsigned", "sigset_t", "sigjmp_buf"
			, "rlim_t", "jmp_buf", "sig_atomic_t", "clock_t", "struct"};

/// 24 개의 연산자 우선순위
operator_precedence operators[OPERATOR_CNT] = {
	{"(", 0}, {")", 0}
	,{"->", 1}	
	,{"*", 4}	,{"/", 3}	,{"%", 2}	
	,{"+", 6}	,{"-", 5}	
	,{"<", 7}	,{"<=", 7}	,{">", 7}	,{">=", 7}
	,{"==", 8}	,{"!=", 8}
	,{"&", 9}
	,{"^", 10}
	,{"|", 11}
	,{"&&", 12}
	,{"||", 13}
	,{"=", 14}	,{"+=", 14}	,{"-=", 14}	,{"&=", 14}	,{"|=", 14}
};

// root1, root2 parse tree 비교 함수
void compare_tree(node *root1,  node *root2, int *result) // 같음:1, 다름:0
{
	node *tmp;
	int cnt1, cnt2;

	// 트리 존재 X 예외처리
	if(root1 == NULL || root2 == NULL){ 
		*result = false; 
		return;
	}

	//비교 연산을 하는 경우
	if(!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")) {
		if(strcmp(root1->name, root2->name) != 0){ 

			// 학생의 답을 root2 구조체에 담음.
			if(!strncmp(root2->name, "<", 1)) 
				strncpy(root2->name, ">", 1); 

			else if(!strncmp(root2->name, ">", 1)) 
				strncpy(root2->name, "<", 1);

			else if(!strncmp(root2->name, "<=", 2)) 
				strncpy(root2->name, ">=", 2); 

			else if(!strncmp(root2->name, ">=", 2)) 
				strncpy(root2->name, "<=", 2); 

			root2 = change_sibling(root2); /// root2의 다음 형제노드로 이동.
		}
	}

	// 학생답과 정답과 다를 경우 false return
	if(strcmp(root1->name, root2->name) != 0){ 
		*result = false; 
		return;
	}

	// root1 과 root2의 자식노드 상태가 다를 경우 false return
	if((root1->child_head != NULL && root2->child_head == NULL) 
		|| (root1->child_head == NULL && root2->child_head != NULL)){ 
		*result = false;
		return;
	}
	else if(root1->child_head != NULL){ // 학생 답에 자식노드가 있는 경우 
		// 답지와 학생의 자식노드 수가 일치하지 않는 경우 false
		if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){ 
			*result = false;
			return;
		}
 
		// 같거나 다른 연산인 경우
		if(!strcmp(root1->name, "==") || !strcmp(root1->name, "!=")) 
		{
			// 재귀적으로 트리를 비교해서 맞는지 검사 후 result에 저장
			compare_tree(root1->child_head, root2->child_head, result);

			// 일단 답이 일치하지 않네? --> 여러 정답 중 정답이 있는지 확인
			if(*result == false) 
			{
				*result = true;
				root2 = change_sibling(root2);	//정답답안의 형제노드를 가져와서 정답지 비교
				compare_tree(root1->child_head, root2->child_head, result);
			}
		}
		else if(!strcmp(root1->name, "+") || !strcmp(root1->name, "*")
				|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
				|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&")) // +, *, |, &, ||, && 연산이라면?
		{
			if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){  // 답지와 형제노드 개수가 같은지 확인
				*result = false;
				return;
			}
			
			// 학생 호출노드를 재귀적으로 호출
			tmp = root2->child_head;

			while(tmp->prev != NULL) // 정답의 이전 형제노드가 존재하는 경우,
				tmp = tmp->prev; 

			while(tmp != NULL) // 여러개의 정답트리에대해 학생트리를 다시 재귀적으로 호출하여 비교
			{
				compare_tree(root1->child_head, tmp, result);
			
				if(*result == true)
					break;
				else{
					if(tmp->next != NULL)
						*result = true;
					tmp = tmp->next;
				}
			}
		}
		else compare_tree(root1->child_head, root2->child_head, result);
	}	

	//답안지가 아직 next가 있는 경우 재귀호출
	if(root1->next != NULL){ 

		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)){ //root2 와  형제노드 개수가 맞지않으면 false return
			*result = false;
			return;
		}

		//재귀호출 결과가 맞는지 비교
		if(*result == true) 
		{
			tmp = get_operator(root1); // 학생 답안의 노드 부모 참조
	
			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*") // 부모노드가 연산자일 경우
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&"))
			{	
				tmp = root2;

				while(tmp->prev != NULL) 
					tmp = tmp->prev;
				
				//형제 노드가 아직 존재하면 재귀호출
				while(tmp != NULL) 
				{
					compare_tree(root1->next, tmp, result); // 형제들과 비교

					if(*result == true) 
						break;
					else{
						if(tmp->next != NULL)
							*result = true;
						tmp = tmp->next;
					}
				}
			}

			else
				compare_tree(root1->next, root2->next, result);
		}
	}
}

//str 을 토큰화 (pl시간에 배운 lex 함수와 유사)
int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN])
{
	char *start, *end;
	char tmp[BUFLEN];
	char str2[BUFLEN];
	char *op = "(),;><=!|&^/+-*\""; 
	int row = 0;
	int i;
 	int isPointer;
	int lcount, rcount;
	int p_str;
	
	// 토큰배열 초기화
	clear_tokens(tokens); 

	//start --> str 을 가리키는 포인터
	start = str; 
	
	//str 이 tokens 타입에 존재하지 않는 경우
	if(is_typeStatement(str) == 0) 
		return false;	


	while(1)
	{
		// strpbrk --> op의 문자들 중 하나라도 start에 존재한다면 해당 문자가 위치한 주소 리턴
		if((end = strpbrk(start, op)) == NULL) // start 에 op에 정의된 문자 중 하나라도 없으면 break; --> return
			break;

		if(start == end){ // 만약 해당 시작 문자가 가장 처음에 존재하는 경우
			// 1. 맨 앞 글자가 전위 ++, -- 로 시작하는 경우 (전위 증감연산자가 온 경우 분석)
			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)) { 
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4)) // 잘못된 연산자 사용 시 false
					return false;

				if(is_character(*ltrim(start + 2))){ // 공백 제거 후 비교
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])) 
						return false; 

					end = strpbrk(start + 2, op); // 증감 연산자 이후 연산자 비교
					if(end == NULL) // 토큰의 끝을 탐색하지 못했을 경우 |
						end = &str[strlen(str)];
					while(start < end) { 
						 // start 이전 문자가 빈칸 + 현재 토큰의 마지막 인자가 문자일 경우 --> 토큰 오류 return false
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
							return false;
						else if(*start != ' ') // start가 빈칸이 아니라면 토큰에다 start의 맨 앞부분을 연결함
							strncat(tokens[row], start, 1); 
						start++;	
					}
				}
				
				// 지금 토큰의 바로 앞 토큰의 마지막이 문자/정수 일 일 때 해당 문자열에 증감, 감소 연산이 있다면,
				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL) //해당 토큰에 ++, -- 가 존재하면 false
						return false;

					// 만약 앞 토큰에 증감 /감소 연산자가 없으면 앞 토큰에 현토큰의 앞2자리를 복사해줌.
					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp, start, 2); 
					strcat(tokens[row - 1], tmp); 
					start += 2; // start를 2칸 땡김
					row--; //현 토큰위치 감소
				}
				else{ // 그 앞 토큰에 증감/감소 연산이 없으면 tmp에 start의 앞 2자리를 복사해옴
					memset(tmp, 0, sizeof(tmp)); 
					strncpy(tmp, start, 2);
					strcat(tokens[row], tmp);
					start += 2;
				}
			}

			// 2. 일반적인 연산인 경우 (비교연산)
			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2) 
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){ 

					// 현 토큰에 2자리수 연산자를 생각해서 복사하고 start 2포인터 이동
				strncpy(tokens[row], start, 2); 
				start += 2; 
			}
			else if(!strncmp(start, "->", 2))  //3. 객체 인스턴스 접근 포인터의 경우
			{
				//end를 start 이후에 연산자가 나오는지 체크해서 연산자가 없으면 end를 str의 문장의 끝으로 설정,
				end = strpbrk(start + 2, op); 

				if(end == NULL) 
					end = &str[strlen(str)];
				
				/* end 이후 또다른 연산자가 있다면
				 앞 토큰에 start의 앞부분을 하나씩 이어 붙임. (빈칸 나올떄까지 붙인다는 뜻) */
				while(start < end){
					if(*start != ' ')
						strncat(tokens[row - 1], start, 1); // 앞 토큰에 이어붙임
					start++;
				}
				row--; // 현재 토큰 개수 감소
			}
			else if(*end == '&') // 3. 앰퍼센드 처리
			{
				// 해당 앰퍼센드 str이 토큰의 가장 첫 부분이거나, 앞 토큰에 연산자가 없는 경우 처리 진행
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){
					// end를 start 이후의 연산자가 있는지 확인해서 있으면 처리진행, 없으면 해당 str의 끝까지 모두 end 처리
					end = strpbrk(start + 1, op); 
					if(end == NULL) 
						end = &str[strlen(str)]; 
					
					strncat(tokens[row], start, 1); // 현재 토큰에 앰퍼센드(&) 붙임
					start++; // 다음 문자이동

					while(start < end){ //탐색 (다음 연산자가 올 때까지..)
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&') // start의 앞 문자가 빈칸인데 현 토큰의 마지막문자가 & 인 경우 에러처리
							return false;
						else if(*start != ' ') // start가 빈칸으로 나올 때 까지 현재 토큰에 이어붙임
							strncat(tokens[row], start, 1);
						start++; 
					}
				}
				
				else{
					strncpy(tokens[row], start, 1); // 1바이트 이어붙이기
					start += 1;
				}
				
			}
		  	else if(*end == '*') // 4. * (포인터? , 곱하기연산? 처리)
			{
				isPointer=0; // * 얘가 포인터인지 곱하기인지 모르니까 일단은 isPointer를 0으로 설정

				//앞부분에 토큰이 존재했는가? 여부 체크
				if(row > 0) 
				{
					
					for(i = 0; i < DATATYPE_SIZE; i++) { // 데이어타입 개수만큼
						if(strstr(tokens[row - 1], datatype[i]) != NULL){ // 앞 토큰에 데이터타입이 있으면
							strcat(tokens[row - 1], "*"); // 앞 토큰에 '*'삽입 (데이터 포인터)
							start += 1; // 다음 문자 이동	
							isPointer = 1; // 포인터로 체크
							break;
						}
					}
					if(isPointer == 1) // 포인터일 경우
						continue;
					if(*(start+1) !=0) // 다음 인자가 널문자가 아니면
						end = start + 1; // end를 한칸 이동

					// 토큰개수가 2개이상인데 ** 형태로 온 경우 앞 토큰에 중간 문자열을 갖다붙이고 토큰개수 감소 (더블포인터)
					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){ 
						strncat(tokens[row - 1], start, end - start); 
						row--; 
					}
					
					//앞 토큰의 마지막이 문자/정수라면 현 토큰에 다음연산자 이전까지 모두 복사
					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){  
						strncat(tokens[row], start, end - start); 
					}

					// 앞 토큰에 연산자가 존재한다면, 현재토큰에 다음연산자전까지 모두 복사
					else if(strpbrk(tokens[row - 1], op) != NULL){ 
						strncat(tokens[row] , start, end - start); 
							
					}
					else // 그외의 경우 현 토큰에 다음 연산자 이전까지 모두 복사
						strncat(tokens[row], start, end - start); 

					// 다음 연산자로 start 이동
					start += (end - start); 
				}
			 	else if(row == 0) //앞부분에 토큰이 없어 -> 무조건 포인터여야함.
				{
					//start 이후에 다음 연산자가 하나도 없으면 일단 현 토큰에 * 를 붙이고 시작
					if((end = strpbrk(start + 1, op)) == NULL){
						strncat(tokens[row], start, 1);
						start += 1; 
					}
					else{ // start 이후에 연산자가 있으면 자신 앞의 문자가 빈칸이고 토큰의 마지막 문자가 문자인 경우 에러처리 (곱하기만 달랑 온거니까..)
						while(start < end){ 
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
								return false;
							else if(*start != ' ')  // 현 토큰에 빈칸이 올 때까지 내용 이어붙임
								strncat(tokens[row], start, 1);
							start++; 
						}
						if(all_star(tokens[row])) // 현재 토큰이 모두 *로 이루어지면 토큰 감소
							row--;
						
					}
				}
			}
			else if(*end == '(') // 5.  여는 괄호의 경우
			{
				lcount = 0;
				rcount = 0;
				// 토큰이 1개이상이고 앞 토큰이 &, * 중 하나라면 탐색 개시
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){
					//만약 괄호 내부에 또 괄호가 있다면... lcount 증가 하고 start 포인터를 내부 ( 위치로 이동
					while(*(end + lcount + 1) == '(') 
						lcount++; 
					start += lcount; 

					end = strpbrk(start + 1, ")"); // 닫는 괄호가 나오기 전까지 탐색

					if(end == NULL) // 닫는 괄호가 없는 없으면 괄호처리 에러라서 false return
						return false;
					else{
						while(*(end + rcount +1) == ')') // ) 괄호 위치로 이동.
							rcount++;
						end += rcount;

						if(lcount != rcount) // 괄호의 짝이 안맞을 경우 false return
							return false;

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){  // 토큰이 2개 이상이고, 2개 앞 토큰의 마지막 인자가 정수/문자가 아니거나 지금 row가 1이라면
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1); // 앞 토큰에 ) 괄호 전까지 이어붙이기.
							row--; // 토큰 개수 감소
							start = end + 1; //가장 바깥쪽 닫는 괄호 다음으로 이동
						}
						else{ // 현재 토큰에 start 맨 앞 글자를 이어붙임
							strncat(tokens[row], start, 1);
							start += 1;
						}
					}
						
				}
				else{ // 그 외엔 ( 붙임
					strncat(tokens[row], start, 1);
					start += 1;
				}

			}
			else if(*end == '\"') // 6. ' (어퍼스트로피) 면 : ' 짝이 맞는지 확인
			{
				// 상대적인 ' 가 어딨는지 확인 없으면 false return
				end = strpbrk(start + 1, "\""); 
		
				if(end == NULL)
					return false;

				else{ // 현재 토큰에 'good' 사이의 good 같은 내용을 붙임
					strncat(tokens[row], start, end - start + 1);
					start = end + 1;
				}

			}

			else{ // 7. 그 외의 경우 에러처리 
				
				//토큰이 1개 이상인데 앞 토큰이 ++ 나 -- 가 오면 에러처리
				if(row > 0 && !strcmp(tokens[row - 1], "++")) 
					return false;

				
				if(row > 0 && !strcmp(tokens[row - 1], "--"))
					return false;

				//현재 토큰에 맨 앞 글자를 붙이고 start 포인터이동
				strncat(tokens[row], start, 1); 
				start += 1;
				
				//현재 토큰이 -- ++ - + 중 하나라면 첫 토큰이면 row를 감소
				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){

					if(row == 0) 
						row--; 

					
					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ // 앞 토큰의 마지막 인자가 문자가 아닐 경우
					
						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL) // 앞 토큰에 전위 증감 연산자가 없으면 row 감소
							row--;
					}
				}
			}
		}
		else{ // start와 end가 다른 경우 
			/* row가 포인터인지 검사한다.*/
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) 
				row--;				

			if(all_star(tokens[row - 1]) && row == 1) 
				row--;	

			 // 다음 토큰 문자가 오기 전까지 탐색 진행
			for(i = 0; i < end - start; i++){
				//구조체 인스턴스 접근이라면 현 토큰에 . 을 바로 붙인다. 그리고 다음 빈칸이 올 때까지 i를 늘린다.
				if(i > 0 && *(start + i) == '.'){ 
					strncat(tokens[row], start + i, 1); 

					while( *(start + i +1) == ' ' && i< end - start ) 
						i++; 
				}
				else if(start[i] == ' '){ // 공백처리
					while(start[i] == ' ')
						i++;
					break;
				}
				else //그 외의 다른 경우면 공백도 아니고, 인스턴스 도 아니면 그냥 일단 토큰에 붙임.
					strncat(tokens[row], start + i, 1);
			}

			if(start[0] == ' '){ // 공백처리
				start += i;
				continue;
			}
			start += i; // 다음 토큰으로 start 이동.
		}
			
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); // 현 토큰의 좌우 공백 제거

		// 토큰이 1개 이상인데 앞 토큰이 변수명이고, 현재 토큰의 마지막이 문자이거나, 바로앞 토큰의 마지막이 문자/정수 이거나 . 으로 끝났다면.
		 if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){

			if(row > 1 && strcmp(tokens[row - 2],"(") == 0) // 토큰이 2개이상인데, 2개 앞 토큰이 ( 인데 바로앞토큰이 struct 나 unsigned 가 안오면 에러처리
			{
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0) 
					return false; 
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { // 토큰이 한개이고, 현 토큰의 마지막값이 문자일때, 첫 토큰이 extern, unisgned, 데이터타입도 아니라면 에러처리
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)
					return false;
			} 
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){ // 토큰이 2개 이상 있고, 앞 토큰이 변수명인데 unisgned나 extern이 아니면 에러처리
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;
			}
			
		}
		
		//첫 토큰이gcc 라면 토큰을 모두 비우고 첫 토큰에 문장 전체를 넣고 return 1;
		if((row == 0 && !strcmp(tokens[row], "gcc")) ){
			clear_tokens(tokens); 
			strcpy(tokens[0], str);	
			return 1;
		} 

		row++;	//다음 열 진행..
	} // 반복 종료

	// 토큰 처리..
	// 토큰이 하나고, 바로앞 토큰이 스타라면 토큰개수줄임, || 토큰 개수가 1보다 크고 바로 앞 토큰이 스타이고 그전 토큰의 마지막 문자가 영문자/숫가자 아니면 토큰줄임
	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) 
		row--;				
	if(all_star(tokens[row - 1]) && row == 1)
		row--;	

	//남은 문자열 처리
	/* 맨 앞 글자가 칸인 경우 빈칸이 아닐때까지 start를 옮기고 , 빈칸이 아닌경우 토큰 개수를증가*/
	for(i = 0; i < strlen(start); i++)
	{
		if(start[i] == ' ')  
		{
			while(start[i] == ' ') 
				i++;
			if(start[0]==' ') { 
				start += i; 
				i = 0; 
			}
			else
				row++; 
			
			i--;
		} 
		else
		{
			// 현재 토큰에 이동된 start 의 한 칸을 붙임
			strncat(tokens[row], start + i, 1); 
			// . 이 왔는데 start 이전이라면 다음문자가 빈칸이 아닐때까지 i를 증가
			if( start[i] == '.' && i<strlen(start)){ 
				while(start[i + 1] == ' ' && i < strlen(start)) 
					i++;

			}
		}
		//공백제거
		strcpy(tokens[row], ltrim(rtrim(tokens[row])));

		//lpthread 이 현옵션에 있다면 그리고 바로 앞 토큰이 - 라면 바로앞 토큰에 현 토큰을 갖다 붙이고 현 토큰을 0으로 만들어버리고 토큰개수를 줄임
		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){  
			strcat(tokens[row - 1], tokens[row]);
			memset(tokens[row], 0, sizeof(tokens[row]));
			row--;
		}
	 	else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){ // 토큰이 1개이상이면서 현재 토큰의 마지막인 문자/정수 이면서 변수명이거나 전 토큰의 마지막 문자가 정수/문자 이거나 . 이라면 while 문 마지막과정 반복
			
			if(row > 1 && strcmp(tokens[row-2],"(") == 0) 
			{
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0) 
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { 
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) 
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;
			}
		} 
	}

	//토큰이 1개 이상이라면
	if(row > 0) 
	{
		// 첫번째 토큰이 include 나 struct 라면 토큰을 토큰을 초기화하고 0번토큰에 str 그자체를 담음 (공백제거해서)
		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){
			clear_tokens(tokens); 
			strcpy(tokens[0], remove_extraspace(str)); 
		}
	}

	// 첫 토큰이 변수명이거나 extern 인 경우 토큰 개수만큼 토큰들을 2번째 토큰부터 순회하면서 마지막 토큰 이전까지 토큰들을 합치는 과정
	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){ 
		for(i = 1; i < TOKEN_CNT; i++){ 
			if(strcmp(tokens[i],"") == 0) 
				break;		       

			if(i != TOKEN_CNT -1 )
				strcat(tokens[0], " "); 
			strcat(tokens[0], tokens[i]); 
			memset(tokens[i], 0, sizeof(tokens[i])); 
		}
	}
	
	// 캐스팅이나 struct 사용 시 토큰 재검사 reset_tokens 시행

	while((p_str = find_typeSpecifier(tokens)) != -1){ 
		if(!reset_tokens(p_str, tokens))
			return false;
	}


	while((p_str = find_typeSpecifier2(tokens)) != -1){  
		if(!reset_tokens(p_str, tokens))
			return false;
	}
	
	return true;
}


// 해당 token 을 바탕으로 파스 트리 생성 (parentheses 괄호개수)
node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses) 
{
	node *cur = root;
	node *new; //임시노드 ,cur, new...
	node *saved_operator;
	node *operator;
	int fstart;
	int i;

	// 모든 토큰이 분석될때까지 반복.
	while(1)
	{
		if(strcmp(tokens[*idx], "") == 0) // 0 > 토큰이 비어있으면 종료
			break;
	
		if(!strcmp(tokens[*idx], ")")) // 1> ) : 트리생성을 종료하고 부모로 되돌리기.
			return get_root(cur);

		else if(!strcmp(tokens[*idx], ",")) // 2> : , 가 오면 트리생성 종료하고 부모 되돌리기.
			return get_root(cur);
		
		else if(!strcmp(tokens[*idx], "(")) // 3. 지금 토큰이 ( 으로 들어간다면 자식노드의 identifier(변수명) 인경우만 병렬적으로 next로 연결함
		{
			// 3-1 : 바로 앞 토큰 문자가 연산자가 아니거나 + ',' 라면 가 아니면 가장 처음값으로 인식하고 같은 (형제) 수준으로 )를 만날때까지 노드를 연결 ex> names (A | B | C)
			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0){ 
				fstart = true; 

				while(1)
				{
					*idx += 1; 

					if(!strcmp(tokens[*idx], ")")) 
						break;
					
					new = make_tree(NULL, tokens, idx, parentheses + 1);
					
					if(new != NULL){ 
						if(fstart == true){  // 노드의 header 처리와 비슷.
							cur->child_head = new; 
							new->parent = cur; 
	
							fstart = false; 
						}
						else{  // 같은 형제노드 수준에서 추가적인 identifier 연결
							cur->next = new; 
							new->prev = cur; 
						}

						cur = new; //후미 노드 초기화
					}

					if(!strcmp(tokens[*idx], ")")) // ) 만났으니까 노드 생성 종료
						break;
				}
			}
			else{ // 3-2 : ( 가 맨 처음 위치하거나 바로앞 토큰이 연산자거나 바로 앞이 , 가 아니라면,,
				*idx += 1; 
	
				new = make_tree(NULL, tokens, idx, parentheses + 1); // 재귀 트리 생성

				if(cur == NULL) // 3-2-A. ( 가 맨 처음에 위치한 경우라면 ex) (end = times ...)
					cur = new; // cur 을 맨 처음 노드로...

				// 3-2-B-a. 현재 준 답안 중에 해당되는 예시 없음.. --> 자식토큰과 현재토큰이 아래와 같은 (| || & &&) 로 동일하다면 자식 노드로 생성하는게 아니라 형제 노드로 연결
				else if(!strcmp(new->name, cur->name)){ 
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||")  
						|| !strcmp(new->name, "&") || !strcmp(new->name, "&&"))
					{
						cur = get_last_child(cur); // cur의 막내노드로 이동해서 

						if(new->child_head != NULL){ // new 노드에 자식노드가 있다면 자식노드로 이동해서 동일한 관계 (형졔관계로 변경)
							new = new->child_head; 

							new->parent->child_head = NULL; 
							new->parent = NULL; 
							new->prev = cur;
							cur->next = new;
						}
					}
					// 3-2-B-b. 괄호 앞에 +, - 가 온 경우 ex) + (  --> 새 연산자와 뒤 연산자의 우선순위를 비교해서 노드관계를 갱신한다.
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*")) 
					{
						i = 0;  // i := 다음 연산자 로 지정.

						while(1) 
						{   // 다음 토큰이 비어있으면 break.
							if(!strcmp(tokens[*idx + i], "")) 
								break;

							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0) // 다음 토큰이 연산자이고 ) 가 아니면 break. 
								break; 

							i++; 
						}
						
						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name)) // ( 안 연산자랑 앞의 연산자랑 비교해서 우선순위에 따라 자식 노드 연결
						{	// 괄호 안 연산자가 우선순위가 더 작으면 이전 연산자랑 동일관계로 노드 설정
							cur = get_last_child(cur);
							cur->next = new; 
							new->prev = cur; 
							cur = new;
						}
						else // 괄호 안 연산자가 우선순위가 더 크면 이전 토큰 연산자를 동일한 이전 연산자의 자식노드로 연결
						{
							cur = get_last_child(cur); 

							if(new->child_head != NULL){ 
								new = new->child_head;

								new->parent->child_head = NULL; 
								new->parent = NULL;
								new->prev = cur;
								cur->next = new;
							}
						}
					}// 3-2-B-c. 그 외의 경우 그냥 동일관계로 연결
					else{ 
						cur = get_last_child(cur);  
						cur->next = new; 
						new->prev = cur;
						cur = new;
					}
				}
				// 3-2-C ( 뒤에 논리연산도 아니고, 맨 처음도 아니고 이전토큰이 , 도 아니라면 그냥 형제노드로연결
				else
				{
					cur = get_last_child(cur); 

					cur->next = new;
					new->prev = cur;
	
					cur = new;
				}
			}
		}
		else if(is_operator(tokens[*idx])) // 4. ( , ) 가 아닌 일반적인 연산자가 온경우 
		{
			// 4-1. 연산자 중에서도 A 연산자 B 관계가 상관없는 연산자 (교환법칙이 성립하는 연산자) 
				// ||, && , | , & , + , *를 만난 경우 --> 형제노드로 연결하기 위한 분기문
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&") 
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*"))
			{  
				//만약 cur이 연산자고, 현재 토큰과 같은 연산자라면 operator 로 cur을 할당
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx])) 
					operator = cur; 
		
				else //아니면
				{
					// 새로운 노드를 생성해서 cur, new 연산자 중 우선순위가 높은 연산자를 operator 로 설정
					new = create_node(tokens[*idx], parentheses);
					operator = get_most_high_precedence_node(cur, new); 

					if(operator->parent == NULL && operator->prev == NULL){ // operator 노드가 부모 , 형제 아무것도 없으면 

						// (원래)opeator 연산자 > 새 연산자 라면 --> new를 operator 의 부모로 설정 (parse 트리 특성상 아래로 내려갈 수록 연산자 우선순위가 높게 설정)
						if(get_precedence(operator->name) < get_precedence(new->name)){ 
							cur = insert_node(operator, new); 
						}
						// (원래)opeator 연산자 < 새 연산자 라면 --> new를 opeartor 의 가장 하위자식으로 설정
						else if(get_precedence(operator->name) > get_precedence(new->name)) 
						{
							if(operator->child_head != NULL){ 
								operator = get_last_child(operator);   
								cur = insert_node(operator, new); 
							}
						}
						else // 연산자 우선순위가 같으면 operator 에 cur 을 할당한다.
						{
							operator = cur; 

							//operatr를 가장 왼편 operator (가장 큰 형) 노드로 이동
							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx])) // 기존 연산자가 새로운 토큰이랑 같을 때
									break;
								
								if(operator->prev != NULL) 
									operator = operator->prev; 
								else 
									break;
							}
							
							//원래 operator 와 현 토큰이 다른 연산자라면 operator를 부모로 이동.
							if(strcmp(operator->name, tokens[*idx]) != 0)
								operator = operator->parent; 

							if(operator != NULL){ // operator 가 NULL 이 아니고 operator와 현 토큰의 operator가 같으면 현재 노드를 opeartor 할당
								if(!strcmp(operator->name, tokens[*idx])) 
									cur = operator; 
							}
						}
					}

					else
						cur = insert_node(operator, new);
				}

			}
			else // 4-2. A 연산자 B 관계에서 연산자 관계가 존재하는  연산자들 --> - = <= 등등.... : 기존의 new를 old의 부모노드로 연결
			{
				// 새 토큰 생성
				new = create_node(tokens[*idx], parentheses); 
				
				// cur 노드가 비어있으면 노드 cur 노드에 new 할당
				if(cur == NULL)
					cur = new;

				else // cur 노드가 존재한다면.
				{
					//new 노드보다 연산자 우선순위가 높은 cur 부모/형제 중 가장 부모노드에 가장 근접하면서 우선순위가 높은연산자 받아옴.
					operator = get_most_high_precedence_node(cur, new);

					if(operator->parentheses > new->parentheses) // operator 가 new 보다 괄호가 많으면 new 를 operator의 부모노드로 설정하고 cur 로 할당
						cur = insert_node(operator, new);
					// opeartor 연산자가 가장 루트노드인경우 operator 연산자가 new 연산자보다 우선순위가 높으면 막내노드(맞형)로 설정
					else if(operator->parent == NULL && operator->prev ==  NULL){ 
					
						if(get_precedence(operator->name) > get_precedence(new->name))
						{
							if(operator->child_head != NULL){ 
	
								operator = get_last_child(operator); 
								cur = insert_node(operator, new);
							}
						}
					// 아래부터 new 연산자를 operator 의 부모노드로 설정.
						else	
							cur = insert_node(operator, new);
					}
	
					else
						cur = insert_node(operator, new);
				}
			}
		}
		else  // 5. 그 밖의 일반 단어의 경우. 
		{
			new = create_node(tokens[*idx], parentheses); //노드 하나 생성 

			if(cur == NULL) // 5-1. 가장 첫번째 노드라면 헤드노드로 설정
				cur = new;

			else if(cur->child_head == NULL){ // 5-2. 자식 노드가 없다면 자식노드로 설정
				cur->child_head = new; 
				new->parent = cur;

				cur = new;
			}
			else{ // 5-3. 자식노드가 있다면 막내노드로 만들고 갱신
					//가장 막내노드 가져오기
				cur = get_last_child(cur); 

				cur->next = new; 
				new->prev = cur;

				cur = new;
			}
		}

		*idx += 1;  //다음 토큰 분석..
	}

	return get_root(cur); // 만들어진 트리의 루트노드를 반환
}

// 자식노드의 형제노드들을 swaping하는 함수 (next <-> prev 변경)
node *change_sibling(node *parent)
{
	node *tmp;
	
	tmp = parent->child_head;

	// 자식 노드 포인팅
	parent->child_head = parent->child_head->next;
	parent->child_head->parent = parent;
	parent->child_head->prev = NULL;

	// prev, next 유동 변경
	parent->child_head->next = tmp; 
	parent->child_head->next->prev = parent->child_head;
	parent->child_head->next->next = NULL;
	parent->child_head->next->parent = NULL;		

	return parent;
}

// name: 학생 답안 정보, parenthesese : 괄호 개수 --> node 생성자
node *create_node(char *name, int parentheses)
{
	node *new;

	new = (node *)malloc(sizeof(node));
	new->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(new->name, name);

	new->parentheses = parentheses;
	new->parent = NULL;
	new->child_head = NULL;
	new->prev = NULL;
	new->next = NULL;

	return new;
}

// 연산자의 우선순위를 알려주는 함수 
int get_precedence(char *op) 
{
	int i;

	for(i = 2; i < OPERATOR_CNT; i++){
		if(!strcmp(operators[i].operator, op))
			return operators[i].precedence;
	}
	return false;
}

// 연산자가 위의 정의된 연산자 중에 존재하는지 체크 있으면 true, 없으면 false
int is_operator(char *op)
{
	int i;

	for(i = 0; i < OPERATOR_CNT; i++)
	{
		if(operators[i].operator == NULL)
			break;
		if(!strcmp(operators[i].operator, op)){
			return true;
		}
	}

	return false;
}


// 트리 출력함수
void print(node *cur) 
{
	//자식 노드가 존재하면 자식 노드로 재귀호출
	if(cur->child_head != NULL){ 
		print(cur->child_head); 
		printf("\n");
	}

	//형제 노드가 존재하면 형제 호출
	if(cur->next != NULL){ 
		print(cur->next);
		printf("\t");
	}
	printf("%s", cur->name); //출력 (자식노드의 가장 오른쪽부터 출력 후위순회 (Right left root))
}

// cur 노드의 형제노드가 있으면 형 노드 반환, 없으면 parent 반환;
node *get_operator(node *cur) 
{
	if(cur == NULL) 
		return cur;

	if(cur->prev != NULL)
		while(cur->prev != NULL)
			cur = cur->prev;

	return cur->parent;
}

// 트리의 루트 노드 반환
node *get_root(node *cur) 
{
	if(cur == NULL)
		return cur;

	while(cur->prev != NULL)
		cur = cur->prev;

	if(cur->parent != NULL)
		cur = get_root(cur->parent);

	return cur;
}

// cur의 형/부모 노드들 중 우선순위가 new보다 하나라도 높은 연산자 노드 return
node *get_high_precedence_node(node *cur, node *new) 
{
	if(is_operator(cur->name)) // 연산자 값이 작을 수록 연산자 우선순위가 높음 우선순위가 높은 cur return
		if(get_precedence(cur->name) < get_precedence(new->name))  
			return cur; 

	// new 연산자 우선순위가 높고 + cur의 형 노드가 존재한다면
	if(cur->prev != NULL){ 
		while(cur->prev != NULL){ // 형제노드를 하나씩 땡겨가며 형제노드 중 우선순위가 하나라도 높은게 있는지 확인.
			cur = cur->prev;
			
			return get_high_precedence_node(cur, new); // 형 노드로 재귀호출
		}


		if(cur->parent != NULL) // 형 노드의 부모노드가 존재한다면 부모노드로 재귀호출
			return get_high_precedence_node(cur->parent, new);
	}

	// cur의 부모노드가 없으면 cur return.
	if(cur->parent == NULL) 
		return cur; 
}

// cur 노드 중 가장 부모노드쪽에 있으면서 new 노드모다 연산자 우선순위가 높은 노드 return
node *get_most_high_precedence_node(node *cur, node *new) 
{
	node *operator = get_high_precedence_node(cur, new); // new보다 연산자가 높은 cur의 형/부모 노드 를 operator 할당
	node *saved_operator = operator; 

	while(1) // cur 노드의 형제/부모들 중 가장 우선순위가 높은 연산자 노드 return
	{
		//cur 노드의 더이상 부모가 없다면.. break;
		if(saved_operator->parent == NULL)
			break; 

		if(saved_operator->prev != NULL)  
			operator = get_high_precedence_node(saved_operator->prev, new); //형제 노드 비교

		else if(saved_operator->parent != NULL) 
			operator = get_high_precedence_node(saved_operator->parent, new); //부모 노드 비교

		saved_operator = operator; 
	}
	
	return saved_operator; 
}

// new를 old 의 부모노드로 연결
node *insert_node(node *old, node *new)
{
	// old 형제 노드가 존재한다면, new를 형제관계로 연결
	if(old->prev != NULL){ 
		new->prev = old->prev;
		old->prev->next = new;
		old->prev = NULL;
	}

	new->child_head = old; 
	old->parent = new; 

	return new;
}

// 노드의 막내의 가장 오른편 노드를 return
node *get_last_child(node *cur) 
{
	if(cur->child_head != NULL)
		cur = cur->child_head;

	while(cur->next != NULL)
		cur = cur->next;

	return cur;
}

// 자기 자신을 미포함한 형제노드 개수 반환.
int get_sibling_cnt(node *cur)
{
	int i = 0;

	while(cur->prev != NULL)
		cur = cur->prev;

	while(cur->next != NULL){
		cur = cur->next;
		i++;
	}

	return i;
}

// cur 노드 기준으로 내려가면서 모두 free 하는 함수
void free_node(node *cur) 
{
	if(cur->child_head != NULL)
		free_node(cur->child_head);

	if(cur->next != NULL)
		free_node(cur->next);

	if(cur != NULL){
		cur->prev = NULL;
		cur->next = NULL;
		cur->parent = NULL;
		cur->child_head = NULL;
		free(cur);
	}
}


// 영문자나, 정수이면 true return
int is_character(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// 타입 종류에 대해 알려주는 함수. (0:타입이 존재하는 문자열, 1: 아무것도 아닌 것(괄호가 캐스팅용), 2:해당타입으로 시작하는 문장)
int is_typeStatement(char *str) 
{ 
	char *start;
	char str2[BUFLEN] = {0}; 
	char tmp[BUFLEN] = {0}; 
	char tmp2[BUFLEN] = {0}; 
	int i;	 
	
	start = str;
	strncpy(str2,str,strlen(str));
	remove_space(str2);

	while(start[0] == ' ')
		start += 1;
/*
	if(strstr(str2, "gcc") != NULL) 
	{
		strncpy(tmp2, start, strlen("gcc"));
		if(strcmp(tmp2,"gcc") != 0)
			return 0;
		else
			return 2;
	}
*/	
	//* 모든 데이터 리스트 중에 str2 가 해당 타입이 있는지 확인 (토큰 분리를 위해)
	for(i = 0; i < DATATYPE_SIZE; i++) 
	{
		if(strstr(str2,datatype[i]) != NULL) // str2 에 데이터타입이 존재한다면
		{	
			strncpy(tmp, str2, strlen(datatype[i])); // tmp 배열에 str2 저장 (가공을 위해 임시저장)
			strncpy(tmp2, start, strlen(datatype[i])); 
			
			if(strcmp(tmp, datatype[i]) == 0) // 임시 확인
				if(strcmp(tmp, tmp2) != 0) // 분할된 문자열과 토큰이 완전히 일치하는지 여부
					return 0;  // 타입이 포함된 문자열같은 경우
				else
					return 2; // 해당 타입으로 시작하는 문장 (변수타입 같은경우인지 확인)		
		}
	}
	return 1; // 아무것도 아니거나 괄호가 단지 캐스팅용으로 쓰인경우

}

// 타입을 순회하며 (타입) 이후 연산자가 오거나 문자가 온다면 (?캐스팅이라면) 해당 토큰 번호 반환 (없으면 -1)
int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN])
{
	int i, j;

	for(i = 0; i < TOKEN_CNT; i++) 
	{
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			if(strstr(tokens[i], datatype[j]) != NULL && i > 0)
			{
				if(!strcmp(tokens[i - 1], "(") && !strcmp(tokens[i + 1], ")") 
						&& (tokens[i + 2][0] == '&' || tokens[i + 2][0] == '*' 
							|| tokens[i + 2][0] == ')' || tokens[i + 2][0] == '(' 
							|| tokens[i + 2][0] == '-' || tokens[i + 2][0] == '+' 
							|| is_character(tokens[i + 2][0]))) 
					return i;
			}
		}
	}
	return -1;
}

// struct 토큰을 사용하면서 다음토큰의 마지막값이 문자나정수라면 --> 구조체 토큰이라면 return i; (없으면 -1)
int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN])
{
    int i, j;

   
    for(i = 0; i < TOKEN_CNT; i++)
    {
        for(j = 0; j < DATATYPE_SIZE; j++)
        {
            if(!strcmp(tokens[i], "struct") && (i+1) <= TOKEN_CNT && is_character(tokens[i + 1][strlen(tokens[i + 1]) - 1]))  
                    return i; 
        }
    }
    return -1;
}

// 현재 str이 모두 * 일 경우 true 아니면 false
int all_star(char *str) 
{
	int i;
	int length= strlen(str);
	
 	if(length == 0)	
		return 0;
	
	for(i = 0; i < length; i++)
		if(str[i] != '*')
			return 0;
	return 1;

}

// str 에서 영문자나 숫자가 하나라도 존재하면 rturn 1;
int all_character(char *str)  
{
	int i;

	for(i = 0; i < strlen(str); i++)
		if(is_character(str[i]))
			return 1;
	return 0;
	
}


// token에 캐스팅이나, struct 가 있는 경우 문법확인 (괄호 짝 갯수) 및 토큰 정렬.
// 함수 start : 캐스팅 , struct 번호
int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]) 
{
	//j 가 start를 움직이며 에러 체크
	int i;
	int j = start - 1; 
	int lcount = 0, rcount = 0;
	int sub_lcount = 0, sub_rcount = 0;

	if(start > -1){
		// 토큰이 struct 와 일치하는 경우 --> 다음토큰과 struct 변수명 이런식으로 합침
		if(!strcmp(tokens[start], "struct")) {		
			strcat(tokens[start], " ");
			strcat(tokens[start], tokens[start+1]); 

			//앞 뒤 토큰을 합친 후 뒷 토큰 앞으로 계속 땡겨옴 (배열의 한계..)
			for(i = start + 1; i < TOKEN_CNT - 1; i++){ 
				strcpy(tokens[i], tokens[i + 1]);
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

		//현 토큰이 unsigned 이고 다음 토큰이 바로 괄호를 닫지 않는 경우 --> 다음 토큰과 unsigned 변수명 형태로 모두 연결 exunisgned char 이런식으로 합치는 듯.
		else if(!strcmp(tokens[start], "unsigned") && strcmp(tokens[start+1], ")") != 0) { 
			strcat(tokens[start], " "); 
			strcat(tokens[start], tokens[start + 1]);	     
			strcat(tokens[start], tokens[start + 2]);

			//앞 뒤 토큰을 합친 후 뒷 토큰 앞으로 계속 땡겨옴 (배열의 한계..)
			for(i = start + 1; i < TOKEN_CNT - 1; i++){ 
				strcpy(tokens[i], tokens[i + 1]);
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

		j = start + 1;           
		//현 토큰 뒤의 괄호 닫기 ) 횟수 측정
		while(!strcmp(tokens[j], ")")){ 
				rcount ++;
				if(j==TOKEN_CNT)
					break;
				j++;
		}
	
		j = start - 1;
		//현 토큰 앞의 괄호 열기 ( 횟수 측정
		while(!strcmp(tokens[j], "(")){
        	        lcount ++;
                	if(j == 0)
						break;
               		j--;
		}

		if( (j!=0 && is_character(tokens[j][strlen(tokens[j])-1]) ) || j==0)
			lcount = rcount; 

		//괄호 개수가 다르다면 false return
		if(lcount != rcount) 
			return false;

		//현 토큰 괄호 ( 앞에 sizeof가 존재하면 true return 하고 종료
		if( (start - lcount) >0 && !strcmp(tokens[start - lcount - 1], "sizeof")){
			return true; 
		}
		
		// unsinged 나 struct 뒤에 바로 괄호 ) 가 오면 필요없는 괄호로 생각하고 괄호제거
		else if((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start+1], ")")) { 
			strcat(tokens[start - lcount], tokens[start]);
			strcat(tokens[start - lcount], tokens[start + 1]);
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]);
		 
		 	//괄호 제거 후 토큰 앞으로 땡겨오고 남는 토큰들은 초기화
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount; i++) { 
				strcpy(tokens[i], tokens[i + lcount + rcount]);
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0]));
			}


		}
 		else{ // 그 외의 괄호처리의 경우
			// 현 토큰의 2번째 뒤에 ( 가 온 경우 그 뒤에 안에 계속 ( ) 괄호가 존재하는 경우
			if(tokens[start + 2][0] == '('){ 
				j = start + 2;
				// 지금 토큰으로부터 ( 괄호가 멈출때까지 j 증가
				while(!strcmp(tokens[j], "(")){ 
					sub_lcount++;
					j++;
				} 	
				// j 한칸 뒤에 ) 가 오지않으면 false return ) 가 온다면 ) 가 닫힐때까지 계속 sub_rcount 증가
				if(!strcmp(tokens[j + 1],")")){ 
					j = j + 1;
					while(!strcmp(tokens[j], ")")){
						sub_rcount++;
						j++;
					}
				}
				else 
					return false;


				// 현 토큰 내부의 내부 ( ) 개수가 맞지 않으면 false return
				if(sub_lcount != sub_rcount)
					return false;
				
				// start 2칸 이후의 내부의 괄호들 모두 제거하고 토큰 앞으로땡기고 초기화
				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]);
				for(int i = start + 3; i<TOKEN_CNT; i++)
					memset(tokens[i], 0, sizeof(tokens[0])); 

			}
			// 괄호 제거 진행 --> 제거한만큼 토큰 정리
			strcat(tokens[start - lcount], tokens[start]);
			strcat(tokens[start - lcount], tokens[start + 1]);
			strcat(tokens[start - lcount], tokens[start + rcount + 1]);
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount -1; i++) { 
				strcpy(tokens[i], tokens[i + lcount + rcount +1]);
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0])); 

			}
		}
	}
	return true;
}

// 토큰을 모두 비우는 (초기화) 하는 함수
void clear_tokens(char tokens[TOKEN_CNT][MINLEN])
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i]));
}

// _str 오른쪽 공백 제거
char *rtrim(char *_str) 
{
	char tmp[BUFLEN];
	char *end;

	strcpy(tmp, _str);
	end = tmp + strlen(tmp) - 1;
	while(end != _str && isspace(*end))
		--end;

	*(end + 1) = '\0';
	_str = tmp;
	return _str;
}

// _str 왼쪽 공백 제거
char *ltrim(char *_str)
{
	char *start = _str;

	while(*start != '\0' && isspace(*start))
		++start;
	_str = start;
	return _str;
}

// 잔여 공백 제거
char* remove_extraspace(char *str)
{
	int i;
	char *str2 = (char*)malloc(sizeof(char) * BUFLEN);
	memset(str2, '\0', BUFLEN); // --> 쓰레기값 들어가서 이상한 답 나옴 디버깅용. (배준형)
	char *start, *end;
	char temp[BUFLEN] = "";
	int position;

	if(strstr(str,"include<")!=NULL){ // 답안 내용에 include<가 있는지 확인
		start = str; // 시작위치
		end = strpbrk(str, "<"); // <부터 위치 저장
		position = end - start;
	
		strncat(temp, str, position); // #include
		strcat(temp, " "); // #include + ' '
		strncat(temp, str + position, strlen(str) - position + 1); // #include + ' ' + <filename>

		str = temp;		
	}
	
	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] ==' ') // 문자가 공백일 경우
		{
			if(i == 0 && str[0] ==' ') // 문자열 전의 공백일 경우
				while(str[i + 1] == ' ') // 인덱스 증가
					i++;	
			else{
				if(i > 0 && str[i - 1] != ' ') // 문자열 내부의 공백일 경우
					str2[strlen(str2)] = str[i];
				while(str[i + 1] == ' ') // 연속적 공백일 경우
					i++;
			} 
		}
		else // 공백이 없을 경우
			str2[strlen(str2)] = str[i];
	}

	return str2; // 불필요한 공백이 제거된 문자열 리턴
}


// str 공백 제거
void remove_space(char *str)
{
	char* i = str;
	char* j = str;
	
	while(*j != 0)
	{
		*i = *j++;
		if(*i != ' ')
			i++;
	}
	*i = 0;
}

// ( ) 개수 짝이 맞으면 1, 틀리면 0
int check_brackets(char *str) 
{
	char *start = str;
	int lcount = 0, rcount = 0;
	
	while(1){
		if((start = strpbrk(start, "()")) != NULL){
			if(*(start) == '(')
				lcount++;
			else
				rcount++;

			start += 1; 		
		}
		else
			break;
	}

	if(lcount != rcount)
		return 0;
	else 
		return 1;
}


// 총 토큰 개수 return
int get_token_cnt(char tokens[TOKEN_CNT][MINLEN])
{
	int i;
	
	for(i = 0; i < TOKEN_CNT; i++)
		if(!strcmp(tokens[i], ""))
			break;

	return i;
}
