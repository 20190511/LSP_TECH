typedef struct pn {

    char path[MAXPATHLEN];
    struct pn *prev;
    struct pn *next;
}pathNode;
char** path_arr(char* str);
int realpathS2 (char *str);


int realpathS2 (char *str)
{
    if (str == NULL)
        return 0;
    int idx = 0;
    char* home_path = getenv("HOME");
    char cur_path[MAXPATHLEN];
    getcwd(cur_path, MAXPATHLEN);
    char *tmp_path = (char*)malloc(MAXPATHLEN);
    char *path = (char*)malloc(MAXPATHLEN);
     
    strcpy(path, str);
    if (path[0] == '~') {
        snprintf(path, MAXPATHLEN, "%s/%s", home_path, str);
    }
    else if (path[0] != '/'){
        snprintf(path, MAXPATHLEN+2, "%s/%s", cur_path, str);
    }

    char **lex = path_arr(str);
    if (lex == NULL)
        return 0;

    pathNode* head = (pathNode*)malloc(sizeof(pathNode));       //더미노드 생성.
    pathNode* cur = head;

    //설계과제1 응용: 링크드리스트로 경로 받아서 . 면 for문 passing, .. 이면 pop 형태로 제작.
    for (idx = 0 ; lex[idx] != NULL ; idx++)
    {
        if (!strcmp(lex[idx], "."))
            continue;
        else if (!strcmp(lex[idx], ".."))
        {
            cur = cur->prev;
            cur->next = NULL;
            continue;
        }

        pathNode* newNode = (pathNode*)malloc(sizeof(pathNode));
        strcpy(newNode->path, lex[idx]);
        cur->next = newNode;
        newNode->prev = cur;
        cur = newNode;
    }
    
    cur = head->next; //더미 노드이니까 하나 삭제.
    while(cur != NULL)
    {
        printf("%s\n", cur->path);
        cur = cur->next;
    }

    return 1;
}


char** path_arr(char* str)
{
    if (str == NULL)
        return NULL;
    
    char* ptr = str;
    char tok_path[MAXPATHLEN];
    int tk_cnt = 0;
    int i;
    while (*ptr != '\0')
    {        
        if (*ptr == '/')
            tk_cnt++;
        ptr++;
    }
    if (str[0] == '/')
        tk_cnt--;

    if (tk_cnt == 0)
        return NULL;
    
    char **lexeme_path = (char**)malloc(sizeof(char*) * (tk_cnt+1));
    strcpy(tok_path, str);
    
    ptr = strtok(tok_path, "/");
    for (i = 0 ; ptr != NULL ; i++)
    {
        if (ptr == NULL)
            break;
        
        lexeme_path[i] = (char*)malloc(sizeof(tok_path));
        strcpy(lexeme_path[i], ptr);
        ptr = strtok(NULL, "/");
    }
    lexeme_path[i] = NULL;
    return lexeme_path;
}
