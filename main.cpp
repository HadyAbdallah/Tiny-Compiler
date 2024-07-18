#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <bits/stdc++.h>
using namespace std;


////////////////////////////////////////////////////////////////////////////////////
// Strings /////////////////////////////////////////////////////////////////////////

bool Equals(const char* a, const char* b)
{
    return strcmp(a, b)==0;
}

bool StartsWith(const char* a, const char* b)
{
    int nb=strlen(b);
    return strncmp(a, b, nb)==0;
}

void Copy(char* a, const char* b, int n=0)
{
    if(n>0) {strncpy(a, b, n); a[n]=0;}
    else strcpy(a, b);
}

void AllocateAndCopy(char** a, const char* b)
{
    if(b==0) {*a=0; return;}
    int n=strlen(b);
    *a=new char[n+1];
    strcpy(*a, b);
}

////////////////////////////////////////////////////////////////////////////////////
// Input and Output ////////////////////////////////////////////////////////////////

#define MAX_LINE_LENGTH 10000

struct InFile
{
    FILE* file;
    int cur_line_num;

    char line_buf[MAX_LINE_LENGTH];
    int cur_ind, cur_line_size;

    InFile(const char* str) {file=0; if(str) file=fopen(str, "r"); cur_line_size=0; cur_ind=0; cur_line_num=0;}
    ~InFile(){if(file) fclose(file);}

    void SkipSpaces()
    {
        while(cur_ind<cur_line_size)
        {
            char ch=line_buf[cur_ind];
            if(ch!=' ' && ch!='\t' && ch!='\r' && ch!='\n') break;
            cur_ind++;
        }
    }

    bool SkipUpto(const char* str)
    {
        while(true)
        {
            SkipSpaces();
            while(cur_ind>=cur_line_size) {if(!GetNewLine()) return false; SkipSpaces();}

            if(StartsWith(&line_buf[cur_ind], str))
            {
                cur_ind+=strlen(str);
                return true;
            }
            cur_ind++;
        }
        return false;
    }

    bool GetNewLine()
    {
        cur_ind=0; line_buf[0]=0;
        if(!fgets(line_buf, MAX_LINE_LENGTH, file)) return false;
        cur_line_size=strlen(line_buf);
        if(cur_line_size==0) return false; // End of file
        cur_line_num++;
        return true;
    }

    char* GetNextTokenStr()
    {
        SkipSpaces();
        while(cur_ind>=cur_line_size) {if(!GetNewLine()) return 0; SkipSpaces();}
        return &line_buf[cur_ind];
    }

    void Advance(int num)
    {
        cur_ind+=num;
    }
};

struct OutFile
{
    FILE* file;
    OutFile(const char* str) {file=0; if(str) file=fopen(str, "w");}
    ~OutFile(){if(file) fclose(file);}

    void Out(const char* s)
    {
        fprintf(file, "%s\n", s); fflush(file);
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Compiler Parameters /////////////////////////////////////////////////////////////

struct CompilerInfo
{
    InFile in_file;
    OutFile out_file;
    OutFile debug_file;

    CompilerInfo(const char* in_str, const char* out_str, const char* debug_str)
                : in_file(in_str), out_file(out_str), debug_file(debug_str)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner /////////////////////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40

enum TokenType{
                IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
                ASSIGN, EQUAL, LESS_THAN,
                PLUS, MINUS, TIMES, DIVIDE, POWER,
                SEMI_COLON,
                LEFT_PAREN, RIGHT_PAREN,
                LEFT_BRACE, RIGHT_BRACE,
                ID, NUM,
                ENDFILE, ERROR
              };

// Used for debugging only /////////////////////////////////////////////////////////
const char* TokenTypeStr[]=
            {
                "If", "Then", "Else", "End", "Repeat", "Until", "Read", "Write",
                "Assign", "Equal", "LessThan",
                "Plus", "Minus", "Times", "Divide", "Power",
                "SemiColon",
                "LeftParen", "RightParen",
                "LeftBrace", "RightBrace",
                "ID", "Num",
                "EndFile", "Error"
            };

struct Token
{
    TokenType type;
    char str[MAX_TOKEN_LEN+1];

    Token(){str[0]=0; type=ERROR;}
    Token(TokenType _type, const char* _str) {type=_type; Copy(str, _str);}
};

const Token reserved_words[]=
{
    Token(IF, "if"),
    Token(THEN, "then"),
    Token(ELSE, "else"),
    Token(END, "end"),
    Token(REPEAT, "repeat"),
    Token(UNTIL, "until"),
    Token(READ, "read"),
    Token(WRITE, "write")
};
const int num_reserved_words=sizeof(reserved_words)/sizeof(reserved_words[0]);

// if there is tokens like < <=, sort them such that sub-tokens come last: <= <
// the closing comment should come immediately after opening comment
const Token symbolic_tokens[]=
{
    Token(ASSIGN, ":="),
    Token(EQUAL, "="),
    Token(LESS_THAN, "<"),
    Token(PLUS, "+"),
    Token(MINUS, "-"),
    Token(TIMES, "*"),
    Token(DIVIDE, "/"),
    Token(POWER, "^"),
    Token(SEMI_COLON, ";"),
    Token(LEFT_PAREN, "("),
    Token(RIGHT_PAREN, ")"),
    Token(LEFT_BRACE, "{"),
    Token(RIGHT_BRACE, "}")
};
const int num_symbolic_tokens=sizeof(symbolic_tokens)/sizeof(symbolic_tokens[0]);

inline bool IsDigit(char ch){return (ch>='0' && ch<='9');}
inline bool IsLetter(char ch){return ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z'));}
inline bool IsLetterOrUnderscore(char ch){return (IsLetter(ch) || ch=='_');}

void GetNextToken(CompilerInfo* pci, Token* ptoken)
{
    ptoken->type=ERROR;
    ptoken->str[0]=0;

    int i;
    char* s=pci->in_file.GetNextTokenStr();
    if(!s)
    {
        ptoken->type=ENDFILE;
        ptoken->str[0]=0;
        return;
    }

    for(i=0;i<num_symbolic_tokens;i++)
    {
        if(StartsWith(s, symbolic_tokens[i].str))
            break;
    }

    if(i<num_symbolic_tokens)
    {
        if(symbolic_tokens[i].type==LEFT_BRACE)
        {
            pci->in_file.Advance(strlen(symbolic_tokens[i].str));
            if(!pci->in_file.SkipUpto(symbolic_tokens[i+1].str)) return;
            return GetNextToken(pci, ptoken);
        }
        ptoken->type=symbolic_tokens[i].type;
        Copy(ptoken->str, symbolic_tokens[i].str);
    }
    else if(IsDigit(s[0]))
    {
        int j=1;
        while(IsDigit(s[j])) j++;

        ptoken->type=NUM;
        Copy(ptoken->str, s, j);
    }
    else if(IsLetterOrUnderscore(s[0]))
    {
        int j=1;
        while(IsLetterOrUnderscore(s[j])) j++;

        ptoken->type=ID;
        Copy(ptoken->str, s, j);

        for(i=0;i<num_reserved_words;i++)
        {
            if(Equals(ptoken->str, reserved_words[i].str))
            {
                ptoken->type=reserved_words[i].type;
                break;
            }
        }
    }

    int len=strlen(ptoken->str);
    if(len>0) pci->in_file.Advance(len);
}

////////////////////////////////////////////////////////////////////////////////////
// Parser //////////////////////////////////////////////////////////////////////////

// program -> stmtseq
// stmtseq -> stmt { ; stmt }
// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
// ifstmt -> if exp then stmtseq [ else stmtseq ] end
// repeatstmt -> repeat stmtseq until expr
// assignstmt -> identifier := expr
// readstmt -> read identifier
// writestmt -> write expr
// expr -> mathexpr [ (<|=) mathexpr ]
// mathexpr -> term { (+|-) term }    left associative
// term -> factor { (*|/) factor }    left associative
// factor -> newexpr { ^ newexpr }    right associative
// newexpr -> ( mathexpr ) | number | identifier

enum NodeKind{
                IF_NODE, REPEAT_NODE, ASSIGN_NODE, READ_NODE, WRITE_NODE,
                OPER_NODE, NUM_NODE, ID_NODE
             };

// Used for debugging only /////////////////////////////////////////////////////////
const char* NodeKindStr[]=
            {
                "If", "Repeat", "Assign", "Read", "Write",
                "Oper", "Num", "ID"
            };

enum ExprDataType {VOID, INTEGER, BOOLEAN};

// Used for debugging only /////////////////////////////////////////////////////////
const char* ExprDataTypeStr[]=
            {
                "Void", "Integer", "Boolean"
            };

#define MAX_CHILDREN 3

struct TreeNode
{
    TreeNode* child[MAX_CHILDREN];
    TreeNode* sibling; // used for sibling statements only

    NodeKind node_kind;
    string id;
    union{TokenType oper; int num; }; // defined for expression/int/identifier only;
    ExprDataType expr_data_type; // defined for expression/int/identifier only

    int line_num;

    TreeNode() {int i; for(i=0;i<MAX_CHILDREN;i++) child[i]=0; sibling=0; expr_data_type=VOID;}
};

struct ParseInfo
{
    Token next_token;
};

void PrintTree(TreeNode* node, int sh=0)
{
    int i, NSH=3;
    for(i=0;i<sh;i++) printf(" ");

    printf("[%s]", NodeKindStr[node->node_kind]);

    if(node->node_kind==OPER_NODE) printf("[%s]", TokenTypeStr[node->oper]);
    else if(node->node_kind==NUM_NODE) printf("[%d]", node->num);
    else if(node->node_kind==ID_NODE || node->node_kind==READ_NODE || node->node_kind==ASSIGN_NODE) printf("[%s]", node->id.c_str());

    if(node->expr_data_type!=VOID) printf("[%s]", ExprDataTypeStr[node->expr_data_type]);

    printf("\n");

    for(i=0;i<MAX_CHILDREN;i++) if(node->child[i]) PrintTree(node->child[i], sh+NSH);
    if(node->sibling) PrintTree(node->sibling, sh);
}

////////////////////////////////////////////////////////////////////////////////////
// Analyzer ////////////////////////////////////////////////////////////////////////

const int SYMBOL_HASH_SIZE=10007;

struct LineLocation
{
    int line_num;
    LineLocation* next;
};

struct VariableInfo
{
    char* name;
    int memloc;
    LineLocation* head_line; // the head of linked list of source line locations
    LineLocation* tail_line; // the tail of linked list of source line locations
    VariableInfo* next_var; // the next variable in the linked list in the same hash bucket of the symbol table
};

struct SymbolTable
{
    int num_vars;
    VariableInfo* var_info[SYMBOL_HASH_SIZE];

    SymbolTable() {num_vars=0; int i; for(i=0;i<SYMBOL_HASH_SIZE;i++) var_info[i]=0;}

    int Hash(const char* name)
    {
        int i, len=strlen(name);
        int hash_val=11;
        for(i=0;i<len;i++) hash_val=(hash_val*17+(int)name[i])%SYMBOL_HASH_SIZE;
        return hash_val;
    }

    VariableInfo* Find(const char* name)
    {
        int h=Hash(name);
        VariableInfo* cur=var_info[h];
        while(cur)
        {
            if(Equals(name, cur->name)) return cur;
            cur=cur->next_var;
        }
        return 0;
    }

    void Insert(const char* name, int line_num)
    {
        LineLocation* lineloc=new LineLocation;
        lineloc->line_num=line_num;
        lineloc->next=0;

        int h=Hash(name);
        VariableInfo* prev=0;
        VariableInfo* cur=var_info[h];

        while(cur)
        {
            if(Equals(name, cur->name))
            {
                // just add this line location to the list of line locations of the existing var
                cur->tail_line->next=lineloc;
                cur->tail_line=lineloc;
                return;
            }
            prev=cur;
            cur=cur->next_var;
        }

        VariableInfo* vi=new VariableInfo;
        vi->head_line=vi->tail_line=lineloc;
        vi->next_var=0;
        vi->memloc=num_vars++;
        AllocateAndCopy(&vi->name, name);

        if(!prev) var_info[h]=vi;
        else prev->next_var=vi;
    }

    void Print()
    {
        int i;
        for(i=0;i<SYMBOL_HASH_SIZE;i++)
        {
            VariableInfo* curv=var_info[i];
            while(curv)
            {
                printf("[Var=%s][Mem=%d]", curv->name, curv->memloc);
                LineLocation* curl=curv->head_line;
                while(curl)
                {
                    printf("[Line=%d]", curl->line_num);
                    curl=curl->next;
                }
                printf("\n");
                curv=curv->next_var;
            }
        }
    }

    void Destroy()
    {
        int i;
        for(i=0;i<SYMBOL_HASH_SIZE;i++)
        {
            VariableInfo* curv=var_info[i];
            while(curv)
            {
                LineLocation* curl=curv->head_line;
                while(curl)
                {
                    LineLocation* pl=curl;
                    curl=curl->next;
                    delete pl;
                }
                VariableInfo* p=curv;
                curv=curv->next_var;
                delete p;
            }
            var_info[i]=0;
        }
    }
};


class Parser{
public:
    deque<pair<TokenType,pair<int,string>>>Tokens;
    pair<TokenType,pair<int,string>> currentToken;
    bool checkErrorsExist=false;
    TreeNode* syntaxTree;
    Parser(deque<pair<TokenType,pair<int,string>>> &Tokens)
    {
        this->Tokens=Tokens;
    }
    void parseTheCode(){
        this->syntaxTree=program();
    }
    pair<TokenType,pair<int,string>> getNextToken(){
        if (Tokens.empty())
            exit(0);
        auto token = Tokens.front(); Tokens.pop_front();
        return token;
    }

    bool matchGetNextToken(TokenType currentToken){
        if (this->currentToken.first==currentToken){
            this->currentToken = getNextToken();
            return true;
        }
        else
            return false;
    }
    // program -> stmtseq
    TreeNode* program(){
        TreeNode* root;
        currentToken = getNextToken();
        root=stmtseq();
        return root;
    }
    // stmtseq -> stmt { ; stmt }
    TreeNode* stmtseq(){
        TreeNode* firstStmt=stmt();
        TreeNode* ptr=firstStmt;
        while (currentToken.first!=ENDFILE && currentToken.first!=END && currentToken.first!=UNTIL && currentToken.first!=ELSE){
            matchGetNextToken(SEMI_COLON);
            TreeNode* nextStmt=stmt();
            if(ptr!=nullptr){
                ptr->sibling=nextStmt;
                ptr=nextStmt;
            }
            else firstStmt=nextStmt;
        }
        return firstStmt;
    }
    TreeNode* stmt(){
        TreeNode* stmtNode=nullptr;
        if(currentToken.first==IF)
            stmtNode=ifstmt();
        else if(currentToken.first==REPEAT)
            stmtNode=repeatstmt();
        else if(currentToken.first==ID)
            stmtNode=assignstmt();
        else if(currentToken.first==READ)
            stmtNode=readstmt();
        else if(currentToken.first==WRITE)
            stmtNode=writestmt();
        else if(currentToken.first==NUM)
        {
            cout<<"Error in line "<<currentToken.second.first<<": "<<"Variable Name not correct you should begin with _ or any Character"<<endl;
            checkErrorsExist=true;
        }
        else
            matchGetNextToken(currentToken.first);
        return stmtNode;
    }

    // ifstmt -> if exp then stmtseq [ else stmtseq ] end
    TreeNode* ifstmt(){
        auto* ifStmtNode=new TreeNode;
        ifStmtNode->node_kind=IF_NODE;
        ifStmtNode->expr_data_type = VOID;
        ifStmtNode->line_num=currentToken.second.first;
        matchGetNextToken(IF);

        ifStmtNode->child[0]=exp();

        if(currentToken.first!=THEN) {
            cout<<"Error in line "<<currentToken.second.first<<": "<<"THEN stmt expected after IF Condition"<<endl;
            checkErrorsExist=true;
        }

        matchGetNextToken(THEN);

        ifStmtNode->child[1]=stmtseq();

        if (currentToken.first==ELSE)
        {
            matchGetNextToken(ELSE);
            ifStmtNode->child[2]=stmtseq();
        }
        if(currentToken.first!=END) {
            cout<<"Error in line "<<currentToken.second.first<<": "<<"END stmt expected after IF Condition"<<endl;
            checkErrorsExist=true;
        }

        matchGetNextToken(END);
        return ifStmtNode;
    }
    // repeatstmt -> repeat stmtseq until expr
    TreeNode* repeatstmt(){
        auto* repeatStmtNode=new TreeNode;
        repeatStmtNode->node_kind=REPEAT_NODE;
        repeatStmtNode->expr_data_type = VOID;
        repeatStmtNode->line_num=currentToken.second.first;
        matchGetNextToken(REPEAT);

        repeatStmtNode->child[0]=stmtseq();


        if(currentToken.first!=UNTIL) {
            cout<<"Error in line "<<currentToken.second.first<<": "<<"UNTIL condition expected after repeat stmt"<<endl;
            checkErrorsExist=true;
        }

        matchGetNextToken(UNTIL);
        repeatStmtNode->child[1]=exp();

        return repeatStmtNode;
    }

    // assignstmt -> identifier := expr
    TreeNode* assignstmt(){
        auto* assignNode=new TreeNode;
        assignNode->node_kind=ASSIGN_NODE;
        assignNode->line_num=currentToken.second.first;


        assignNode->id=currentToken.second.second;
        matchGetNextToken(ID);
        if(currentToken.first!=ASSIGN) {
            cout<<"Error in line "<<currentToken.second.first<<": "<<"Assignment(:=) operator expected"<<endl;
            checkErrorsExist=true;
        }

        matchGetNextToken(ASSIGN);
        assignNode->child[0]=exp();

        return assignNode;
    }
    // readstmt -> read identifier
    TreeNode* readstmt(){
        auto* readStmtNode=new TreeNode;
        readStmtNode->node_kind=READ_NODE;
        readStmtNode->expr_data_type = VOID;
        readStmtNode->line_num=currentToken.second.first;
        matchGetNextToken(READ);

        if(currentToken.first!=ID) {
            cout<<"Error in line "<<currentToken.second.first<<": "<<"Identifier expected after read stmt, variable name may be wrong"<<endl;
            checkErrorsExist=true;
        }

        readStmtNode->id = currentToken.second.second;
        matchGetNextToken(ID);

        return readStmtNode;
    }
    // writestmt -> write expr
    TreeNode* writestmt(){
        auto* writeStmtNode=new TreeNode;
        writeStmtNode->node_kind=WRITE_NODE;
        writeStmtNode->expr_data_type = VOID;
        writeStmtNode->line_num=currentToken.second.first;
        matchGetNextToken(WRITE);

        writeStmtNode->child[0]=exp();

        return writeStmtNode;
    }
    // expr -> mathexpr [ (<|=) mathexpr ]
    TreeNode* exp(){
        TreeNode* expNode=mathexpr();
        if (currentToken.first==LESS_THAN || currentToken.first==EQUAL){
            auto* newExpNode=new TreeNode;
            newExpNode->node_kind=OPER_NODE;
            newExpNode->expr_data_type = BOOLEAN;
            newExpNode->oper=currentToken.first;
            newExpNode->line_num=currentToken.second.first;
            newExpNode->child[0]=expNode;
            matchGetNextToken(currentToken.first);
            newExpNode->child[1]=mathexpr();
            return newExpNode;
        }
        return expNode;
    }
    // mathexpr -> term { (+|-) term }    left associative
    TreeNode* mathexpr(){
        TreeNode* mathExprNode=term();
        while (currentToken.first==PLUS ||currentToken.first==MINUS){
            auto* newMathExprNode=new TreeNode;
            newMathExprNode->node_kind=OPER_NODE;
            newMathExprNode->expr_data_type = INTEGER;
            newMathExprNode->oper=currentToken.first;
            newMathExprNode->line_num=currentToken.second.first;

            newMathExprNode->child[0]=mathExprNode;
            matchGetNextToken(currentToken.first);
            newMathExprNode->child[1]=mathexpr();
            mathExprNode=newMathExprNode;
        }
        return mathExprNode;
    }
    // term -> factor { (*|/) factor }    left associative
    TreeNode* term(){
        TreeNode* termNode=factor();
        while (currentToken.first==TIMES || currentToken.first==DIVIDE){
            auto* newTermNode=new TreeNode;
            newTermNode->node_kind=OPER_NODE;
            newTermNode->expr_data_type = INTEGER;
            newTermNode->oper=currentToken.first;
            newTermNode->line_num=currentToken.second.first;

            newTermNode->child[0]=termNode;
            matchGetNextToken(currentToken.first);
            newTermNode->child[1]=term();
            termNode=newTermNode;
        }
        return termNode;
    }
    // factor -> newexpr { ^ newexpr }    right associative
    TreeNode* factor(){
        TreeNode* factorNode=newexpr();
        while (currentToken.first==POWER){
            auto* newFactorNode=new TreeNode;
            newFactorNode->node_kind=OPER_NODE;
            newFactorNode->expr_data_type = INTEGER;
            newFactorNode->oper=currentToken.first;
            newFactorNode->line_num=currentToken.second.first;

            newFactorNode->child[0]=factorNode;
            matchGetNextToken(currentToken.first);
            newFactorNode->child[1]=factor();
            return newFactorNode;
        }
        return factorNode;
    }
    // newexpr -> ( mathexpr ) | number | identifier
    TreeNode* newexpr(){
        auto* newExprNode=new TreeNode;
        if (currentToken.first==LEFT_PAREN){
            matchGetNextToken(LEFT_PAREN);
            newExprNode=mathexpr();
            matchGetNextToken(RIGHT_PAREN);
        }
        else if(currentToken.first==NUM){
            newExprNode->node_kind=NUM_NODE;
            newExprNode->expr_data_type = INTEGER;
            newExprNode->num= stoi(currentToken.second.second);
            newExprNode->line_num=currentToken.second.first;
            matchGetNextToken(NUM);
        }
        else if(currentToken.first==ID){
            newExprNode->node_kind=ID_NODE;
            newExprNode->expr_data_type = INTEGER;
            newExprNode->id=currentToken.second.second;
            newExprNode->line_num=currentToken.second.first;
            matchGetNextToken(ID);
        }
        else{
            cout<<"Error in line "<<currentToken.second.first<<": "<<"Unexpected Error"<<endl;
            checkErrorsExist=true;
        }
        return newExprNode;
    }
    void printParseTree(){
        if(!checkErrorsExist)
            PrintTree(syntaxTree);
    }
};

void programGeneration(TreeNode *currNode, SymbolTable *symbolTable) {

    // generate Symbol Table with variables of code
    if (currNode->node_kind == ASSIGN_NODE || currNode->node_kind == READ_NODE || currNode->node_kind == ID_NODE)
    {
        symbolTable->Insert(currNode->id.c_str(), currNode->line_num);
    }


    if (currNode->child[0] != nullptr) {
        for (auto &child: currNode->child) {
            if (child != nullptr)
                programGeneration(child, symbolTable);
        }
    }


    if(currNode->node_kind==IF_NODE && currNode->child[0]->expr_data_type!=BOOLEAN) {
        cerr<<"Error in line "<<currNode->line_num<<": Expected boolean condition with if stmt"<<endl;
        exit(0);
    }
    if(currNode->node_kind==REPEAT_NODE && currNode->child[1]->expr_data_type!=BOOLEAN) {
        cerr<<"Error in line "<<currNode->line_num<<": Expected boolean condition with until stmt"<<endl;
        exit(0);
    }
    if(currNode->node_kind==WRITE_NODE && currNode->child[0]->expr_data_type!=INTEGER) {
        cerr<<"Error in line "<<currNode->line_num<<": Expected integer with write stmt"<<endl;
        exit(0);
    }
    if(currNode->node_kind==ASSIGN_NODE && currNode->child[0]->expr_data_type!=INTEGER) {
        cerr<<"Error in line "<<currNode->line_num<<": Expected integer with Assign stmt"<<endl;
        exit(0);
    }


    if (currNode->sibling != nullptr)
        programGeneration(currNode->sibling, symbolTable);
}


int calc(TokenType tokenType, int left, int right) {
    if (tokenType == POWER) return (int) pow(left, right);
    else if  (tokenType == TIMES) return left*right;
    else if  (tokenType == DIVIDE) return left/right;
    else if (tokenType == PLUS) return left+right;
    else if  (tokenType == MINUS) return left-right;
    else if  (tokenType == EQUAL) return left==right;
    else if  (tokenType == LESS_THAN) return left<right;
    return 0;
}


int calcExpr(TreeNode* currNode, SymbolTable* symbolTable, vector<int> &variablesTable){
    if(currNode->node_kind==ID_NODE)
        return variablesTable[symbolTable->Find(currNode->id.c_str())->memloc];
    if(currNode->node_kind==NUM_NODE)
        return currNode->num;

    int left=calcExpr(currNode->child[0], symbolTable, variablesTable);
    int right=calcExpr(currNode->child[1], symbolTable, variablesTable);

    return calc(currNode->oper,left,right);
}


void runProgram(TreeNode *currNode, SymbolTable *symbolTable, vector<int> &variablesTable) {

    if(currNode->node_kind == WRITE_NODE){
        if (currNode->child[0]->node_kind == ID_NODE)
            cout<<currNode->child[0]->id.c_str()<<": "<<
            variablesTable[symbolTable->Find((currNode->child[0]->id.c_str()))->memloc]<<endl;

        else if (currNode->child[0]->node_kind == NUM_NODE)
            cout<<"val: "<<currNode->child[0]->num<<endl;
        else {
            int val=calcExpr(currNode->child[0], symbolTable, variablesTable);
            cout<<"val: "<<val<<endl;
        }
    }

    else if (currNode->node_kind == READ_NODE){
        int idx=symbolTable->Find(currNode->id.c_str())->memloc;
        cout<<"Enter "<<currNode->id.c_str()<<": ";
        cin>>variablesTable[idx];
        if(!std::cin.good()) {
            cerr<<"Run Time Error in line "<<currNode->line_num<<": Expected integer with read stmt"<<endl;
            exit(0);
        }
    }

    else if (currNode->node_kind == IF_NODE) {
        int left = 0, right = 0;

        if(currNode->child[0]->child[0]->node_kind==ID_NODE)
            left = variablesTable[symbolTable->Find(currNode->child[0]->child[0]->id.c_str())->memloc];
        else if(currNode->child[0]->child[0]->node_kind==NUM_NODE)
            left = currNode->child[0]->child[0]->num;


        if(currNode->child[0]->child[1]->node_kind==ID_NODE)
            right = variablesTable[symbolTable->Find(currNode->child[0]->child[1]->id.c_str())->memloc];
        else if(currNode->child[0]->child[1]->node_kind==NUM_NODE)
            right = currNode->child[0]->child[1]->num;

        bool conditionCheck = bool(calc(currNode->child[0]->oper,left,right));

        if(conditionCheck)
            runProgram(currNode->child[1],symbolTable,variablesTable);
        else if (currNode->child[2])
            runProgram(currNode->child[2],symbolTable,variablesTable);
    }

    else if (currNode->node_kind == ASSIGN_NODE) {
        int value=0;
        if(currNode->child[0]->node_kind==ID_NODE)
            value = variablesTable[symbolTable->Find(currNode->child[0]->id.c_str())->memloc];

        else if(currNode->child[0]->node_kind==NUM_NODE)
            value = currNode->child[0]->num;
        else
            value=calcExpr(currNode->child[0], symbolTable, variablesTable);

        variablesTable[symbolTable->Find(currNode->id.c_str())->memloc] = value;
    }


    else if (currNode->node_kind == REPEAT_NODE) {
        int value=0;
        do
        {
            runProgram(currNode->child[0], symbolTable, variablesTable);

            value=0;

            if(currNode->child[1]->node_kind==ID_NODE)
                value = variablesTable[symbolTable->Find(currNode->child[1]->id.c_str())->memloc];

            else if(currNode->child[1]->node_kind==NUM_NODE)
                value = currNode->child[1]->num;

            else {
                int left = 0, right = 0;

                if(currNode->child[1]->child[0]->node_kind==ID_NODE)
                    left = variablesTable[symbolTable->Find(currNode->child[1]->child[0]->id.c_str())->memloc];
                else if(currNode->child[1]->child[0]->node_kind==NUM_NODE)
                    left = currNode->child[1]->child[0]->num;


                if(currNode->child[1]->child[1]->node_kind==ID_NODE)
                    right = variablesTable[symbolTable->Find(currNode->child[1]->child[1]->id.c_str())->memloc];
                else if(currNode->child[1]->child[1]->node_kind==NUM_NODE)
                    right = currNode->child[1]->child[1]->num;

                value = calc(currNode->child[1]->oper,left,right);
            }
        }while(!value);
    }

    if (currNode->sibling!= nullptr) runProgram(currNode->sibling, symbolTable, variablesTable);
}

int main()
{
    CompilerInfo *compiler = new CompilerInfo("input.txt","output.txt","debug.txt");
    Token *token = new Token();
    bool checkError=false;
    deque<pair<TokenType,pair<int,string>>>Tokens;
    pair<TokenType,pair<int,string>> currentToken;
    while (token->type!=ENDFILE)
    {
        GetNextToken(compiler,token);

        if(token->type==ERROR) {
            currentToken={token->type,{compiler->in_file.cur_line_num,token->str}};
            checkError=true;
            break;
        }
        compiler->out_file.Out(token->str);
        Tokens.push_back({token->type,{compiler->in_file.cur_line_num,token->str}});
    }
    if(checkError){
        cout<<"Error in line "<<currentToken.second.first<<endl;
        return 0;
    }
    Parser myParser=Parser(Tokens);
    myParser.parseTheCode();
    if(!myParser.checkErrorsExist)
    {
        SymbolTable *symbolTable = new SymbolTable();
        programGeneration(myParser.syntaxTree, symbolTable);
        cout << "The Symbol Table:-" << endl;
        symbolTable->Print();
        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

        // Print The Syntax Tree
        cout << "The Syntax Tree:-" << endl;
        PrintTree(myParser.syntaxTree);
        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

        vector<int> variablesTable(symbolTable->num_vars);
        cout << "Running Program" << endl;
        runProgram(myParser.syntaxTree, symbolTable, variablesTable);

    }

}
