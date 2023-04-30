
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
