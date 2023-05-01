
#ifndef BLANK_H_
#define BLANK_H_

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef BUFLEN
	#define BUFLEN 1024
#endif

#define OPERATOR_CNT 24
#define DATATYPE_SIZE 35
#define MINLEN 64
#define TOKEN_CNT 50

typedef struct node{
	int parentheses;
	char *name;
	struct node *parent;
	struct node *child_head;
	struct node *prev;
	struct node *next;
}node;

typedef struct operator_precedence{
	char *operator;
	int precedence;
}operator_precedence;

void compare_tree(node *root1,  node *root2, int *result); //root1 <-> root2 parse tree 동일 판단 함수
node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses); // token 가지고 tree 만드는 함수
node *change_sibling(node *parent); // 부모의 자식 노드의 형제들을 바꾸는 함수 (< 같은 연산자에서 왼/오 바꾸기위해)
node *create_node(char *name, int parentheses); // node 생성자
int get_precedence(char *op); // 우선순위를 알려주는 함수
int is_operator(char *op); // op 에 token 존재하는지 확인용
void print(node *cur); // 트리 출력함수
node *get_operator(node *cur); 
node *get_root(node *cur); //트리의 루트노드 반환
node *get_high_precedence_node(node *cur, node *new); // cur 과 new 중 우선순위 높은 것 반환
node *get_most_high_precedence_node(node *cur, node *new); // new 보다 높은 우선순위중 가장 높은것, 없으면 new
node *insert_node(node *old, node *new); // old 에 new 삽입함수
node *get_last_child(node *cur); // 가장 막내노드 return
void free_node(node *cur); // 트리 free 전용함수
int get_sibling_cnt(node *cur); // 트리의 자식 개수 알려주는 함수 (compare_tree 에서 사용)

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN]); // lexeme 생성기 (lexical analyzer)
int is_typeStatement(char *str); // str의 type 을 알려주는 함수
int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]); // casting 체크함수
int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]); // struct + 다음토큰 마지막이 문자/정수
int is_character(char c); // c가 문자/정수인지 확인
int all_star(char *str); // str에 * 만 존재하는지 확인
int all_character(char *str); // str 에 문자/숫자 하나라도 존재하는지?
int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]); // token 문법확인 및 정렬함수
void clear_tokens(char tokens[TOKEN_CNT][MINLEN]); // 토큰을 모두 비우는 초기화함수
int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]); // 총 토큰 개수를 알려주는 함수
char *rtrim(char *_str); // 왼쪽 white space 제거
char *ltrim(char *_str); // 오른쪽 white space 제거
void remove_space(char *str); // 남은 white space 제거
int check_brackets(char *str); // 괄호 쌍이 맞는지 확인
char* remove_extraspace(char *str); // 남은 white space 제거

#endif
