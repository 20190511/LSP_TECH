
int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN]) // �־��� ���ڿ��� ��ūȭ, TOKEN_CNT:50, MINLEN:64
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
	
	clear_tokens(tokens); // ��ū �ʱ�ȭ

	start = str; // ���� ���ڿ� ���� ������ ����
	
	if(is_typeStatement(str) == 0) // �߸��� �ۼ��� ���, gcc üũ
		return false;	


	while(1)
	{
		// strpbrk(): �ι�° ���� ���ڿ��� ���ڵ� �� �ϳ��� ù��° ���ڿ��� �����Ѵٸ� �ش� ���ڰ� ��ġ�� �ι�°�� �ּ� ����
		if((end = strpbrk(start, op)) == NULL) // start���� op�� ���ڵ� �� �ϳ��� ���ٸ� break
			break;

		if(start == end){ // ���۰� ���� ���ڰ� ���ٸ�
 
			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)) { // �տ� �α��ڰ� ���� ������������ ���
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4)) // �߸��� ���������� ���
					return false;

				if(is_character(*ltrim(start + 2))){ // ���� ���� ���� ��, ���� ���������� �� ���ڰ� �Դٸ�
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])) // ex. abc-- abc;�� ���� ���
						return false; 

					end = strpbrk(start + 2, op); // ���� �����ڱ��� ��ū Ž��
					if(end == NULL) // ��ū�� ���� Ž������ ������ ���
						end = &str[strlen(str)];
					while(start < end) { //  ���� �����ں��� ���� �����ڱ���
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1])) // start �� ���ڰ� �����̰� ���� ��ū�� ������ ���ڰ� ������ ���
							return false;
						else if(*start != ' ') // ������ �ƴϸ� ��ū�� ����
							strncat(tokens[row], start, 1); // ���� ��ū�� 1����Ʈ�� ����
						start++;	
					}
				}
				
				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ // �� ��ū�� ������ ���ڰ� ������ ���
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL) // �� ��ū�� ���� �������� ���
						return false;

					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp, start, 2); // tmp�� start���� 2����Ʈ ����
					strcat(tokens[row - 1], tmp); // tmp�� �� ��ū ���� ����
					start += 2; // start�� �ε����� 2����Ʈ �̵�
					row--; //���� ��ū ���� ����
				}
				else{ // �� ���� ���
					memset(tmp, 0, sizeof(tmp)); // ��ū�� 2����Ʈ�� ����
					strncpy(tmp, start, 2);
					strcat(tokens[row], tmp);
					start += 2;
				}
			}

			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2) 
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){ // �� �������� ���

				strncpy(tokens[row], start, 2); // ������ ����ũ��(2����Ʈ) ��ŭ ����
				start += 2; // 2����Ʈ �̵�
			}
			else if(!strncmp(start, "->", 2)) // ����ü ������ ���� �������� ���
			{
				end = strpbrk(start + 2, op); // ���� �����ڰ� ���ö� ���� Ž��

				if(end == NULL) // �����ڰ� ������ �ʾ��� ���
					end = &str[strlen(str)]; // str�� �������� ������ ����

				while(start < end){ // ���� �����ڱ��� Ž��
					if(*start != ' ') // ������ �ƴϸ�
						strncat(tokens[row - 1], start, 1); // �� ��ū�� �̾����
					start++;
				}
				row--; // ���� ��ū ���� ����
			}
			else if(*end == '&') // �ּ� ������ �������� ���
			{
				
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){ // ù ��ū�̰ų� �� ��ū�� �����ڰ� ���� ���
					end = strpbrk(start + 1, op); // ���� ������ Ž��
					if(end == NULL) // ���� �����ڰ� ������
						end = &str[strlen(str)]; // str�� �������� end�� ����
					
					strncat(tokens[row], start, 1); // ���� ��ū�� start�� 1����Ʈ('&')�� ����
					start++; // ���� �����̵�

					while(start < end){ // ���� �����ڰ� ���ö� ���� Ž��
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&') // start 1����Ʈ ���� �����̰� ���� ��ū�� ������ ���ڰ� '&'�� ���
							return false;
						else if(*start != ' ') // ������ �ƴ� ���
							strncat(tokens[row], start, 1); // 1����Ʈ�� ���� ��ū�� �̾� ����
						start++; // ���� �����̵�
					}
				}
				
				else{
					strncpy(tokens[row], start, 1); // 1����Ʈ �̾���̱�
					start += 1;
				}
				
			}
		  	else if(*end == '*') // '*' �̸�
			{
				isPointer=0; // isPointer �ʱ�ȭ, '*'�� ������ �����Ͱ� �ƴ� ���� �����Ƿ�

				if(row > 0) // ��ū�� �Ѱ� �̻��̸�
				{
					
					for(i = 0; i < DATATYPE_SIZE; i++) { // ���̾�Ÿ�� ������ŭ
						if(strstr(tokens[row - 1], datatype[i]) != NULL){ // �� ��ū�� ������Ÿ���� ������
							strcat(tokens[row - 1], "*"); // �� ��ū�� '*'���� (������ ������)
							start += 1; // ���� ���� �̵�	
							isPointer = 1; // �����ͷ� üũ
							break;
						}
					}
					if(isPointer == 1) // �������� ���
						continue;
					if(*(start+1) !=0) // ���� ���ڰ� �ι��ڰ� �ƴϸ�
						end = start + 1; // end�� ��ĭ �̵�

					// ���� ������?
					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){ // ��ū�� 2�� �̻��̰� 2�� �� ��ū�� '*'�̸�, �� ��ū�� ��� '*'�̸�
						strncat(tokens[row - 1], start, end - start); // �� ��ū��
						row--; // ��ū ���� ����
					}
					
					
					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){  // �� ��ū�� ������ ���ڰ� ���ڸ�
						strncat(tokens[row], start, end - start); // ���� ��ū�� ���� ������ ������ ��� �Է�
					}

					
					else if(strpbrk(tokens[row - 1], op) != NULL){ // �� ��ū�� �����ڰ� ������
						strncat(tokens[row] , start, end - start); // ���� ��ū�� ���� ������ ������ ��� �Է�
							
					}
					else // �׿��� ���
						strncat(tokens[row], start, end - start); // ���� ��ū�� ���� ������ ������ ��� �Է�

					start += (end - start); // ���� �����ڱ��� ���� �̵�
				}

			 	else if(row == 0) // ù ��ū�� ���
				{
					if((end = strpbrk(start + 1, op)) == NULL){ // ���� �����ڰ� ������
						strncat(tokens[row], start, 1); // ��ū�� 1����Ʈ('*')�� ���̰�
						start += 1; // ���� ���� �̵�
					}
					else{ // �����ڰ� ������
						while(start < end){ // ���� �����ڱ���
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1])) // ���� ���ڰ� �����̰� ���� ��ū�� ������ ���ڰ� ������ ���
								return false;
							else if(*start != ' ') // ������ ���
								strncat(tokens[row], start, 1); // ���� ��ū�� ���� �̾���� 
							start++; // ���� ���� �̵�	
						}
						if(all_star(tokens[row])) // ���� ��ū�� ��� '*'���ڷ� �̷���� ���� ��� ��ū ���� ����
							row--;
						
					}
				}
			}
			else if(*end == '(') // ���� ��ȣ�� ���
			{
				lcount = 0;
				rcount = 0;
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){ // ��ū�� 1�� �̻��̰�, �� ��ū�� '&' | '*' �� ��� 
					while(*(end + lcount + 1) == '(') // ��ȣ ���� ���Ŀ� ���� ��ȣ�� �� ���� ���
						lcount++; 
					start += lcount; // ���� ���� ���� ��ȣ�� �����̵�

					end = strpbrk(start + 1, ")"); // �ݴ� ��ȣ�� ������ ������ Ž��

					if(end == NULL) // �ݴ� ��ȣ�� ���� ���
						return false;
					else{
						while(*(end + rcount +1) == ')') // �ݴ� ��ȣ ���� üũ
							rcount++;
						end += rcount;

						if(lcount != rcount) // ��ȣ�� ¦�� �ȸ��� ���
							return false;

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){  // ��ū�� 2�� �̻��̰�, 2�� �� ��ū�� ������ ���ڰ� ���ڰ� �ƴϰų� ��ū�� 1���� ���
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1); // �� ��ū�� �ݴ� ��ȣ ������ ��� �̾����
							row--; // ��ū ���� ����
							start = end + 1; //���� �ٱ��� �ݴ� ��ȣ �������� �̵�
						}
						else{ // ���� ��ū�� 1����Ʈ �̾����
							strncat(tokens[row], start, 1);
							start += 1;
						}
					}
						
				}
				else{ // �� �ܿ� '(' ����
					strncat(tokens[row], start, 1);
					start += 1;
				}

			}
			else if(*end == '\"') // ����ǥ��
			{
				end = strpbrk(start + 1, "\""); // ���� ����ǥ�� ��ġ�� Ž��
				
				if(end == NULL) // ���� ����ǥ�� ���� ���
					return false;

				else{ // ���� ��ū�� ����ǥ ������ ������ ����
					strncat(tokens[row], start, end - start + 1);
					start = end + 1;
				}

			}

			else{ // �� ���� ����
				
				if(row > 0 && !strcmp(tokens[row - 1], "++")) // ��ū�� 1�� �̻��̰� �� ��ū�� ++�� ���
					return false;

				
				if(row > 0 && !strcmp(tokens[row - 1], "--")) // ��ū�� 1�� �̻��̰� �� ��ū�� --�� ���
					return false;
	
				strncat(tokens[row], start, 1); // ���� ��ū�� 1����Ʈ�� ����
				start += 1; // ���� �̵�
				
			
				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){ // ���� ��ū�� ���� �������� ���

					if(row == 0) // ù ��ū�̸� row ����
						row--; 

					
					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ // �� ��ū�� ������ ���ڰ� ���ڰ� �ƴ� ���
					
						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL) // �� ��ū�� ���� ���� �����ڰ� ������ row ����
							row--;
					}
				}
			}
		}
		else{ // start�� end�� �ٸ���(�ܾ��� ���)
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) //���������� �˻�(row > 2)
				row--;				

			if(all_star(tokens[row - 1]) && row == 1) // ���������� �˻�(row = 1)
				row--;	

			for(i = 0; i < end - start; i++){ // ���� ��ū ���ڰ� ���� ������ Ž��
				if(i > 0 && *(start + i) == '.'){ // ����ü �����̸�
					strncat(tokens[row], start + i, 1); // '.'�� �ٷ� �ٿ���

					while( *(start + i +1) == ' ' && i< end - start ) 
						i++; 
				}
				else if(start[i] == ' '){ // ������ ��� ����
					while(start[i] == ' ')
						i++;
					break;
				}
				else // �� �ܸ� ��ū�� ����
					strncat(tokens[row], start + i, 1);
			}

			if(start[0] == ' '){ // ������ ��� ����
				start += i;
				continue;
			}
			start += i; // ���� ��ū ���� ���ڱ��� �ε��� ����
		}
			
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); // �¿� ���� ���� �� �ٽ� ����

		// ��ū�� 1�� �̻��̰� ���� ��ū�� �������� �����̰� �� ��ū�� ������ Ÿ���̰ų�(���� ����)
		// �� ��ū�� ������ ���ڰ� �����̰ų� �� ��ū�� ������ ���ڰ� '.'�̸�
		 if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){

			if(row > 1 && strcmp(tokens[row - 2],"(") == 0) // ��ū�� 2�� �̻� �ְ� 2�� ��  ��ū�� ���� ��ȣ�� ��
			{
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0) // �� ��ū�� struct�� unsigned�� �ƴ� ���
					return false; 
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { // ��ū�� �Ѱ��̰�, ���� ��ū�� ������ ���ڰ� ������ ���
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) // ù ��ū�� extern�� �ƴϰ�, unsigned�� �ƴϰ�, ��������Ÿ���� �ƴ� ���
					return false;
			} 
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){ // ��ū�� 2�� �̻� �ְ�, ���� ��ū�� ������Ÿ���� ���
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;
			}
			
		}

		if((row == 0 && !strcmp(tokens[row], "gcc")) ){ // ù ��ū�� gcc�� ���
			clear_tokens(tokens); // ��ū�� ����
			strcpy(tokens[0], str);	// ù ��ū�� ���� ��ü�� �־��ְ� ����
			return 1;
		} 

		row++;
	} // �ݺ��� ����

	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) // �� ��ū�� ���̰� 2�� �� ��ū �������� ���ڰ� �ƴ� ���
		row--;				
	if(all_star(tokens[row - 1]) && row == 1) // �� ��ū�� '*'�̸�
		row--;	

	for(i = 0; i < strlen(start); i++) // start�� ���� ���� ó��
	{
		if(start[i] == ' ')  // ������ ���
		{
			while(start[i] == ' ') // ������ �ƴҶ����� i ����
				i++;
			if(start[0]==' ') { // ù ���ڰ� �����̸�
				start += i; // start�� ���� ��ġ i���� �ű��
				i = 0; // i �ʱ�ȭ
			}
			else
				row++; // ��ū ���� ����
			
			i--;
		} 
		else
		{
			strncat(tokens[row], start + i, 1); // ���� ��ū�� 1����Ʈ�� ����
			if( start[i] == '.' && i<strlen(start)){ // ���� '.'�̰� ��ġ�� start�� ���� �ʾ�����
				while(start[i + 1] == ' ' && i < strlen(start)) // ���� ���ڰ� ������ �ƴҶ����� i����
					i++;

			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); // �յ� ���� ���� �� ��ū�� �߰�

		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){  // -lpthread �ɼ��� ������
			strcat(tokens[row - 1], tokens[row]); // �ΰ��� �ٿ��� �� ��ū�� �ְ�
			memset(tokens[row], 0, sizeof(tokens[row]));
			row--; // ��ū ���� ����
		}
	 	else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){ // ��ū�� 1�� �̻��̰� ���� ��ū�� ������ ���ڰ� �����̰� ������ Ÿ���̰ų� �� ��ū�� ������ ���ڰ� �����̰ų� '.'
			
			if(row > 1 && strcmp(tokens[row-2],"(") == 0) // ��ū�� 2���̻��̰� 2�� �� ��ū�� ���� ��ȣ�� ���
			{
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0) // �� ��ū�� struct�� �ƴϰų� unsigned�� �ƴ� ���
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { // ��ū�� 2���̰� ���� ��ū�� ������ ���ڰ� ������ ���
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) // 0��° ��ū�� extern�� �ƴϰų� unsigned�� �ƴϰų� ������Ÿ���� �ƴ� ���
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){ // ��ū�� 2�� �̻��̰� �� ��ū�� ������Ÿ���� ���
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0) // �ΰ� �� ��ū�� unsigned�� �ƴϰ� extern�� �ƴ� ���
					return false;
			}
		} 
	}


	if(row > 0) // ��ū�� 1�� �̻��� ���
	{

		
		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){  // ù��° ��ū�� #include�̰ų� include�̰ų� struct�� ��� 
			clear_tokens(tokens); // ��ū�� �����ϰ�
			strcpy(tokens[0], remove_extraspace(str)); // ������ ���� ����
		}
	}

	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){ // ù ��ū�� ������Ÿ���̰ų� extern�� �椷
		for(i = 1; i < TOKEN_CNT; i++){ // ��ū ������ŭ ��ȸ
			if(strcmp(tokens[i],"") == 0)  // ��ū���� ������ ���� ���
				break;		       

			if(i != TOKEN_CNT -1 ) // ��ū�� �������� �ƴҰ��
				strcat(tokens[0], " "); // ù ��ū�� ������ ����
			strcat(tokens[0], tokens[i]); // ù ��ū�� i��° ��ū�� ���̰�
			memset(tokens[i], 0, sizeof(tokens[i])); // i��° ��ū �޸� �ʱ�ȭ
		}
	}
	
	
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
