//
//  main.cpp
//  wordAnalysis
//
//  Created by Ivy on 2017/11/18.
//  Copyright ? 2017年 Ivy. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string.h>
using namespace std;
#define max 200

#define norw 13       /* 保留字个数 */
#define txmax 100     /* 符号表容量 */
#define nmax 14       /* 数字的最大位数 */
#define al 10         /* 标识符的最大长度 */
#define maxerr 30     /* 允许的最多错误数 */
#define amax 2048     /* 地址上界*/
#define levmax 3      /* 最大允许过程嵌套声明层数*/
#define cxmax 200     /* 最多的虚拟机代码数 */
#define stacksize 500 /* 运行时数据栈元素最多为500个 */

int toIndex = 1;
char DigitTab[max][20];
char VarTab[max][20];
int DigIndex = 1;
int VarIndex = 1;
int cx; /* 假想机代码指针 */
int err; /* 错误个数 */
int aa = 0;
int else_mark[10] = {0};
int flagg = 0;
int canshu_s;
int canshu_n = 0;


const char oneOp[] ={    '+','-','*','/','=','<','>'}; /*说明单字运算符 */

const char* twoOp[] = { "<>","<=",">=",":=","++","--" }; /*说明双字运算符*/
const char* const KeyWord[] =
{   "","program","if","then","else","while","do","call","read","write",
    "odd","var","producer","begin","end","procedure","const"};

const char End[] ={',','{','}',';','[',']','(',')'};/*说明界符*/

enum object{ constant,variable,procedur,}; /*说明种类的定义*/

enum fct /* 指令 */
{
    lit,     opr,     lod,
    sto,     cal,     ini,
    jmp,     jpc,
};


typedef struct Token /*定义sym*/
{
    char text[20];
    int num;
    int type; //0标识符 1运算符 2界符 3常量 4基本字
    int line;
    char *index;//常数／标识符表指针
    //0 无error 1变量非法定义（数字开头） 2关键字不存在
}token;

typedef struct symtable/*符号表结构*/
{
    char name[20];
    enum object kind;
    int val;/*值*/
    int level;/*层次*/
    int adr;/*地址*/
    int size;/*存储空间*/
    int canshu = 0;/*形参个数*/
};

typedef struct instruction
{
    enum fct f;/* 伪操作码 */
    int l;/* 调用层与说明层的层差值 */
    int a;/* 相对地址 */
};

struct symtable table[max];
char line[81];      /* 读取行缓冲区 */
char a[al+1];       /* 临时符号，多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
char word[norw][al];        /* 保留字 */
char instru_name[8][5];   /* 假想代码指令名称 */

void tokenWord(token *t,char strToken[],int type,int line,char *tabIndex)
{
    if ( type == 3)
    {
        t[toIndex].num = atof(strToken);
    }
    else
        t[toIndex].num = -1;
    strcpy(t[toIndex].text, strToken);
    t[toIndex].type = type;
    t[toIndex].line = line;
    t[toIndex].index = tabIndex;
    toIndex++;
    //*strToken = NULL;
}

//0 无error 1变量非法定义（数字开头） 2关键字不存在
void Error(int type,char ch[],int line)
{
    if(type == 1)
    {
        cout << "Error:"<<"(Line"<<line<<")"<<ch<<"非法定义,不允许数字开头"<<endl;
    }
    else if(type == 2)
    {
        cout << "Error:"<<"(Line"<<line<<")"<<ch<<"不是关键字"<<endl;
    }
}
int Reserve(char ch[])
{
    for(int i = 1; i <= 16;i++)
    {
        if((strcmp(ch, KeyWord[i]) == 0)) return i;
    }
    return 0;
}
char* InsertId(char str[])
{
    for(int i = 0;str[i] != '\0';i++)
        VarTab[VarIndex][i] = str[i];
    return VarTab[VarIndex];
}

char* InserConst(char str[])
{
    for(int i = 0;str[i] != '\0';i++)
        DigitTab[DigIndex][i] = str[i];
    return DigitTab[DigIndex];
}

void ProcEro(int Line)
{
    cout << "第" << Line << "行输入有误"<<endl;
}

bool isLetter(char ch)
{
    if ((ch <= 'z' && ch >= 'a' ) || (ch <= 'Z' && ch >= 'A'))
        return true;
    else  return false;
}


void analysis(char ch[],int chLen,token *t,int Line )
{
    char strToken[20];
    int strIndex = 0;
    int chIndex = 0;
    int code;
    int error_type = 0;
    while(chIndex < chLen)
    {
        while( ch[chIndex] == ' ' || ch[chIndex] == '\t'){ chIndex++; }
        while( ch[chIndex] == 13 || ch[chIndex] == 10 ){ return; }
        if(isLetter(ch[chIndex]))
        {
            strIndex = 0;
            while (isLetter(ch[chIndex]) || isdigit(ch[chIndex])) {
                strToken[strIndex++] = ch[chIndex++];
            }
            strToken[strIndex] = '\0';
            code = Reserve(strToken);
            if(code == 0)
                tokenWord(t, strToken, 0, Line, InsertId(strToken));
            else
                tokenWord(t, strToken, 4, Line, NULL);
        }
        else if (isdigit(ch[chIndex]))
        {
            strIndex = 0;
            while (isdigit(ch[chIndex]) && ch[chIndex] != ' ')
            {
                strToken[strIndex++] = ch[chIndex++];
                strToken[strIndex] = '\0';
            }
            if(isLetter(ch[chIndex]))
            {
                error_type = 1;
                while( isLetter(ch[chIndex]) || isdigit(ch[chIndex]))
                {
                    strToken[strIndex++] = ch[chIndex++];
                }
                strToken[strIndex] = '\0';
                tokenWord(t, strToken, 0, Line, InsertId(strToken));
                Error(error_type, strToken, Line);
            }
            else
                tokenWord(t, strToken, 3, Line,InserConst(strToken));
        }
        else if(ch[chIndex] == '+' || ch[chIndex] == '-'||
                ch[chIndex] == '*' || ch[chIndex] == '/'||
                ch[chIndex] == '=' || ch[chIndex] == '<'||
                ch[chIndex] == '>' || ch[chIndex] == ':')
        {
            strIndex = 0;
            if(ch[chIndex+1] ==  ' ' || ch[chIndex+1] == '\t'|| isLetter(ch[chIndex+1]) || isdigit(ch[chIndex+1]))//单目运算符
            {
                int i = 0,flag = false; //flag为true时，确认为单目运算符
                while(i<7)
                {
                    if( ch[chIndex] == oneOp[i])
                    {
                        strToken[strIndex] = oneOp[i];
                        strToken[++strIndex] = '\0';
                        tokenWord(t, strToken, 1, Line, NULL);
                        chIndex++;
                        flag  = true;
                        break;
                    }
                    i++;
                }
                if(flag)
                    continue;
                strToken[strIndex] = ch[chIndex++];
                strToken[strIndex+1] = '\0';
                error_type = 2;
                Error(error_type, strToken, Line);
                continue;
            }
            //超前搜索
            //双目运算符
            strToken[strIndex] = ch[chIndex];
            chIndex++;
            if( ch[chIndex-1] == '<')
            {
                if(ch[chIndex] == '>' || ch[chIndex] == '=')
                {
                    strToken[++strIndex] = ch[chIndex];
                    strToken[++strIndex] = '\0';
                    chIndex++;
                }
            }
            else if (ch[chIndex-1] == ':'||ch[chIndex-1] == '>' )
            {
                if(ch[chIndex] == '=' )
                {
                    strToken[++strIndex] = ch[chIndex];
                    strToken[++strIndex] = '\0';
                    chIndex++;
                }
            }
            else
                strToken[strIndex+1] = '\0';
            //            while(ch[chIndex]!= ' '&&ch[chIndex]!= '\t'&& !isLetter(ch[chIndex]) && !isdigit(ch[chIndex]))
            //            {
            //                error_type = 2;
            //                strToken[strIndex++] = ch[chIndex++];
            //            }
            //            if(error_type == 2 )
            //            {
            //                strToken[strIndex] = '\0';
            //                Error(error_type, strToken, Line);
            //            }
            //            else
            tokenWord(t, strToken, 1, Line, NULL);
        }
        else if(/* DISABLES CODE */ (1))
        {
            for(int i = 0;i<8;i++)
            {
                if( End[i] == ch[chIndex])
                {
                    strIndex = 0;
                    strToken[strIndex] = End[i];
                    strToken[strIndex+1] = '\0';
                    tokenWord(t,strToken, 2, Line, NULL);
                    chIndex++;
                    break;
                }
            }
        }
        else
        {
            strIndex = 0;
            while( ch[chIndex] == ' ' || ch[chIndex] == '\t' )
            {
                strToken[strIndex++] = ch[chIndex++];
            }
            strToken[strIndex] = '\0';
            error_type = 2;
            Error(error_type, strToken, Line);
        }
        error_type = 0;
    }
}



void  ReadFile(token *t)
{
    fstream file;
    char filename1[] = "test.txt";
    char ch[max];
    int Len = 1;
    file.open(filename1);
    
    while(!file.eof())
    {
        file.getline(ch,max);
        analysis(ch,strlen(ch),t,Len);
        Len++;
    }
    for(int i = 1;i < toIndex;i++)
    {
//        cout << setw(9) << t[i].text << setw(4) << t[i].type << setw(4)<< t[i].line << endl;
    }
    cout << "-------------- 词法分析结束 --------------"<<endl;
}


// ----------------------------------------- 以上是词法分析器

int yfIndex = 1;
void prog(token *);
void proc(token *,int*,int*,int,int);
void block(token *,int,int);
void constt(token *,int*,int,int*);
void vardecl(token *,int*,int,int*);
void condecl(token *,int*,int,int*);
void body(token *,int*,int);
void statement(token *,int *,int);
void  exp(token *,int*,int);
int lexp(token *,int *,int);
int lop(token *);
int aop(token *);
void term(token *,int*,int);
void factor(token *,int*,int);


int mop(token *);

void error0(int line)
{
    err++;
    cout << "第" << line <<"行缺少分号"<<endl;
}

void error1(int line)
{
    err++;
    cout << "第" << line <<"行标识符有误"<<endl;
}

void error2(int line)
{
    err++;
    cout << "第" << line <<"行program拼写有误"<<endl;
}

void error3(int line)
{
    err++;
    cout << "第" << line <<"行常数输入有误"<<endl;
}

void error4(int line)
{
    err++;
    cout << "第" << line <<"行const拼写有误"<<endl;
}

void error5(int line)
{
    err++;
    cout << "第" << line <<"行赋值符号有误"<<endl;
}

void error6(int line)
{
    err++;
    cout << "第" << line <<"行var拼写有误"<<endl;
}

void error7(int line)
{
    err++;
    cout << "第" << line <<"行缺少右括号 ）"<<endl;
}

void error8(int line)
{
    err++;
    cout << "第" << line <<"行缺少左括号 ( "<<endl;
}

void error9(int line,token *t)
{
    err++;
    cout << "第" << line <<"行procdeure拼写有误"<<endl;
}

void error10(int line)
{
    err++;
    cout << "第" << line <<"行end拼写有误"<<endl;
}

void error11(int line)
{
    err++;
    cout << "第" << line <<"行begin拼写有误"<<endl;
}

void error12(int line)
{
    err++;
    cout << "第" << line <<"行if拼写有误"<<endl;
}
void error13(int line)
{
    err++;
    cout << "第" << line <<"行then拼写有误"<<endl;
}

void error14(int line)
{
    err++;
    cout << "第" << line <<"行call拼写有误"<<endl;
}

void error15(int line)
{
    err++;
    cout << "第" << line <<"行while拼写有误"<<endl;
}

void error16(int line)
{
    err++;
    cout << "第" << line <<"行do拼写有误"<<endl;
}

void error17(int line)
{
    err++;
    cout << "第" << line <<"行do拼写有误"<<endl;
}

void error18(int line)
{
    err++;
    cout << "第" << line <<"行odd拼写有误"<<endl;
}

void error19(int line)
{
    err++;
    cout << "第" << line <<"行缺少运算符"<<endl;
}

void error20(int line)
{
    err++;
    cout << "第" << line <<"行有多余分号"<<endl;
}

void error21(int line)
{
    err++;
    cout << "第" << line <<"行缺少关键字then"<<endl;
}

void error22(int line)
{
    err++;
    cout << "第" << line <<"行有误"<<endl;
}

void error23(int line)
{
    err++;
    cout << "第" << line <<"行else有误"<<endl;
}

/* 语义分析时的错误 */
void error24(int line)
{
    err++;
    cout << "第" << line <<"标识符未定义"<<endl;
}

void error25(int line)
{
    err++;
    cout << "第" << line <<"数字越界"<<endl;
}

/*写入符号表*/
void enter (token *t,enum object k,int* tx,int lev,int *dx)
{
    (*tx)++;
    token temp,temp_2;
    if( canshu_n > 0)
    {
        strcpy(table[(*tx)].name , t[canshu_s].text);
         temp = t[canshu_s];
        temp_2 = t[canshu_s - 2];
    }
    else
    {
        strcpy(table[(*tx)].name , t[yfIndex].text);
         temp = t[yfIndex];
        temp_2 = t[yfIndex - 2];
    }
    table[(*tx)].kind = k;
    switch(k)
    {
        case constant:
            if (temp.num > amax)
            {
                /* 常数越界 */
                table[(*tx)].val = 0;
                break;
            }
            else
            {
                if( temp_2.type == 0 )
                    strcpy(table[(*tx)].name, temp_2.text);
                table[(*tx)].val = temp.num;
            }
            break;
        case variable:
            table[(*tx)].level = lev;
            table[(*tx)].adr = *dx;
            (*dx)++;
            break;
        case procedur:
            table[(*tx)].level = lev;
            break;
    }
}

/* 遇到标识符，查找符号表是否已定义
 name 标识符名
 tx 符号表指针 */
int position(char* name,int tx)
{
    int i = tx; /* 临时变量 */
    while(i)
    {
        string temp = table[i].name;
        if (strcmp(table[i].name,name) == 0)
            return i;
        else
            i--;
    }
    return i;
}

/* 假想目标机代码 */
void gen(enum fct f,int l,int a)
{
    if( cx > cxmax ) /* 指令过多 */
    {
        cout << "指令过多" << endl;
        exit(1);
    }
    //    if ( z > amax)  /* 偏移地址越界 */
    //    {
    //        cout << "偏移地址越界" << endl;
    //        exit(1);
    //    }
    code[cx].f = f;
    code[cx].l = l;
    code[cx].a = a;
    cx++;
}
void showtable(int tx)
{
    fstream file;
    char filename[] = "table.txt";
    file.open(filename);
    if (!file)
    {
        cout << "table文件不存在" <<endl;
        return;
    }
    for( int i = 1;i <= tx;i++)
    {
        switch(table[i].kind)
        {
            case constant:
                file << "const " << table[i].name << endl;
                file << "val = " << table[i].val << endl << endl;
                break;
            case variable:
                file << "var " << table[i].name << endl;
                file << "level " << table[i].level << endl;
                file << "addr = " << table[i].level << endl << endl;
                break;
            case procedur:
                file << "procedure " << table[i].name << endl;
                file << "lev = " << table[i].level << endl;
                file << "addr = " << table[i].adr << endl;
                file << "size = " << table[i].size << endl ;
                file << "canshu = "<< table[i].canshu << endl<< endl;
                break;
        }
    }
}


/* ---------------------------- 以下为BNF描述 ------------------------ */

/* <lop> → =|<>|<|<=|>|>= */
int lop(token *t)
{
    string temp = t[yfIndex].text;
    if (t[yfIndex].type == 1) {
        if (strcmp(t[yfIndex].text, "=") == 0) {
            return 0;
        }
        else if (strcmp(t[yfIndex].text, "<>") == 0) {
            return 1;
        }
        else if (strcmp(t[yfIndex].text, "<") == 0) {
            return 2;
        }
        else if (strcmp(t[yfIndex].text, "<=") == 0) {
            return 3;
        }
        else if (strcmp(t[yfIndex].text, ">") == 0) {
            return 4;
        }
        else if (strcmp(t[yfIndex].text, ">=") == 0) {
            return 5;
        }
        else
            return -1;
    }
    else
        return -1;
}
/* <aop> → +|- */
int aop(token *t)
{
    if (strcmp(t[yfIndex].text,"+") == 0) {
        return 0;
    }
    else if (strcmp(t[yfIndex].text,"-") == 0) {
        return 1;
    }
    else
        return -1;
}

/* <mop> → *|/ */
int mop(token *t)
{
    if (strcmp(t[yfIndex].text,"*") == 0) {
        return 0;
    }
    else if (strcmp(t[yfIndex].text,"/") == 0) {
        return 1;
    }
    return -1;
    
}

/* <factor>→<id>|<integer>|(<exp>) */
void factor(token *t,int *tx,int lev)
{
    int i = -2; /* 记录标识符在表中的位置 */
    int num;
    int j = strlen(t[yfIndex].text);
    char temp[j];
    strcpy(temp, t[yfIndex].text);
    if (t[yfIndex].type == 3 || t[yfIndex].type == 0)
    {
        if (t[yfIndex].type == 0)
        {
            i = position(t[yfIndex].text,*tx);
        }
        if (i == -2)   /* 为数字 */
        {
            gen(lit,0,atof(temp));
        }
        else if( i == -1 )
        {
            error24(t[yfIndex].line); /* 标识符未定义 */
        }
        else
        {
            switch(table[i].kind)
            {
                case constant:
                    gen(lit,0,table[i].val);/* 直接把常量的值入栈 */
                    break;
                case variable:
                {
                    int j = position(table[i].name, *tx);
                    gen(lod,lev - table[j].level,table[j].adr);
                }
                    break;
                case procedur:
                    error24(t[yfIndex].line);
            }
        }
        yfIndex++;
    }
    else if(t[yfIndex].type == 3){
        yfIndex++;
        if( t[yfIndex].num > amax) /* 数越界 */
        {
            error25(t[yfIndex].line);
            num = 0;
        }
        else
            num = t[yfIndex].num;
        gen(lit, 0, num);
    }
    else if(strcmp(t[yfIndex].text, "(") == 0) /* 因子为表达式 */
    {
        yfIndex++;
        exp(t,tx,lev);
        if (strcmp(t[yfIndex].text, ")") == 0) {
            yfIndex++;
        }
        else
            error7(t[yfIndex].line); // lack of )
    }
    else
        error8(t[yfIndex].line); // lack of (
}

/* <term> → <factor>{<mop><factor>} */
void term(token *t,int* tx,int lev)
{
    factor(t,tx,lev);
    while (mop(t) != -1) {
        int temp_t = mop(t);
        yfIndex++;
        factor(t,tx,lev);
        if( temp_t == 0 )
        {
            gen(opr,0,4); /* 乘法指令 */
        }
        else
        {
            gen(opr,0,5); /* 除法指令 */
        }
        
    }
}

/* <exp> → [+|-]<term>{<aop><term>} */
void exp(token *t,int* tx,int lev)
{
    if( (strcmp(t[yfIndex].text, "+") == 0)
       || strcmp(t[yfIndex].text, "-") == 0)
    {
        yfIndex++;
        if ( strcmp(t[yfIndex].text, "-") == 0 )
        {
            gen(opr,0,1);   /* 如果开头为负号生成取负指令 */
        }
    }
    term(t,tx,lev);
    while(aop(t) != -1) {
        int temp_t = aop(t);
        yfIndex++;
        term(t,tx,lev);
        if ( temp_t == 0)
        {
            gen(opr, 0, 2); /* 生成加法指令 */
        }
        else
        {
            gen(opr, 0, 3); /* 生成减法指令 */
        }
    }
}

/* <lexp> → <exp> <lop> <exp>|odd <exp> */
int lexp(token *t,int *tx,int lev)    /* 返回opr的数字 */
{
    if (strcmp(t[yfIndex].text, "odd") == 0) {
        yfIndex++;
        exp(t,tx,lev);
        gen(opr, 0, 6); /* 生成odd指令 */
        return 6;
    }
    else
    {
        exp(t,tx,lev);
        int i = lop(t);
        if (i != -1)
        {
            yfIndex++;
            exp(t,tx,lev);
            switch (i)
            {
                case 0:
                    gen(opr, 0, 8);
                    break;
                case 1:
                    gen(opr, 0, 9);
                    break;
                case 2:
                    gen(opr, 0, 10);
                    break;
                case 3:
                    gen(opr, 0, 11);
                    break;
                case 4:
                    gen(opr, 0, 12);
                    break;
                case 5:
                    gen(opr, 0, 13);
                    break;
            }
        }
        else
            error19(t[yfIndex].line);
        return i+8;
    }
}

/*
 <statement> → <id> := <exp>
 |if <lexp> then <statement>[else <statement>]
 |while <lexp> do <statement>
 |call <id>（[<exp>{,<exp>}]）
 |<body>
 |read (<id>{，<id>})
 |write (<exp>{,<exp>})
 */
void statement(token *t,int* tx,int lev)
{
    int i ; /* 记录标识符在符号表中的位置 */
    int cx0,cx1; /* 保存指令地址 */
    string temp = t[yfIndex].text;
    
    // 赋值语句 <statement> → <id> := <exp>
    if (t[yfIndex].type == 0) {
        
        i = position(t[yfIndex].text,*tx);
        if( i == 0 )
        {
            error24(t[yfIndex].line);  /* 标识符未声明 */
            i = 0;
        }
        yfIndex++;
        if (strcmp(t[yfIndex].text, ":=") == 0) {
            yfIndex++;
            exp(t,tx,lev);
        }
        else
        {
            error5(t[yfIndex].line); // 缺少:=
            if (t[yfIndex+1].type == 1 || strcmp(t[yfIndex].text, "+") ||
                strcmp(t[yfIndex].text, "-") || t[yfIndex].type == 0) { //后面是运算表达式
                exp(t,tx,lev);
            }
        }
        if ( i != 0)
        {
            /* 执行sto命令完成赋值 */
            gen(sto, lev-table[i].level, table[i].adr);
        }
    }
    
    // if语句 if <lexp> then <statement>[else <statement>]
    else if (strcmp(t[yfIndex].text, "if") == 0)
    {
        int opr_num ; // 记录opr的号码
        yfIndex++;
        opr_num = lexp(t,tx,lev);
        if (strcmp(t[yfIndex].text, "then") == 0) {
            yfIndex++;
            cx0 = cx; /* 保存当前指令地址 */
            gen(jpc, 0, 0); /* 生成条件跳转指令，跳转地址未知，暂时写0 */
            statement(t,tx,lev);
            gen(jmp,0,0); /* 生成无条件挑战指令，跳转地址未知，暂时写0 */
            code[cx0].a = cx; /* 回填jpc */
            else_mark[aa++] = cx-1;
            if (strcmp(t[yfIndex].text, "else") == 0)
            {
                //else可有可无
                yfIndex++;
                if ( strcmp(t[yfIndex].text, "if") != 0)
                /* 若是else操作，则需取反；若是else if，则不用取反 */
                {
                    temp = t[yfIndex].text;
                    statement(t,tx,lev);
                    //                    code[cx2].a = cx; /* 回填jmp */
                    //                    code[cx0].a = cx; /* 回填jpc */
                }
                else
                {
                    statement(t,tx,lev);
                    /* 回填所有jmp */
                    for(int i=0;i<aa;i++)
                    {
                        code[else_mark[i]].a = cx;
                    }
                }
            }
            //查找statement的first集，判断else为写错还是无需else
            else if(strcmp(t[yfIndex].text, "if") == 0  ||
                    strcmp(t[yfIndex].text, "while") == 0 ||
                    strcmp(t[yfIndex].text, "call") == 0 ||
                    strcmp(t[yfIndex].text, "read") == 0 ||
                    strcmp(t[yfIndex].text, "write") == 0 ||
                    t[yfIndex].type == 0){
                yfIndex++;
                error23(t[yfIndex].line);//else出错
                /* 若是else操作，则需取反；若是else if，则不用取反 */
                if( strcmp(t[yfIndex].text, "if") != 0 )
                    // gen(boo, 0, 0);
                    cx0 = cx; /* 保存当前指令地址 */
                gen(jpc, 0, 0); /* 生成条件跳转指令，跳转地址未知，暂时写0 */
                statement(t,tx,lev);
                code[cx0].a = cx; /* 回填 */
            }
        }
        else
        {
            error13(t[yfIndex].line);//then出错
            //查找statement的first集，判断then为写错还是漏写了
            
            //漏写then，跳过
            if (strcmp(t[yfIndex].text, "if") == 0  ||
                strcmp(t[yfIndex].text, "while") == 0 ||
                strcmp(t[yfIndex].text, "call") == 0 ||
                strcmp(t[yfIndex].text, "read") == 0 ||
                strcmp(t[yfIndex].text, "write") == 0 ||
                (t[yfIndex].type == 0 && t[yfIndex+1].type == 1))
            {
                cx0 = cx; /* 保存当前指令地址 */
                gen(jpc, 0, 0); /* 生成条件跳转指令，跳转地址未知，暂时写0 */
                statement(t,tx,lev);
                code[cx0].a = cx; /* 回填 */
                
                if (strcmp(t[yfIndex].text, "else") == 0)
                {
                    //else可有可无
                    yfIndex++;
                    /* 若是else操作，则需取反；若是else if，则不用取反 */
                    if( strcmp(t[yfIndex].text, "if") != 0 )
                        // gen(boo, 0, 0);
                        cx0 = cx; /* 保存当前指令地址 */
                    gen(jpc, 0, 0); /* 生成条件跳转指令，跳转地址未知，暂时写0 */
                    statement(t,tx,lev);
                    code[cx0].a = cx; /* 回填 */
                }
                //查找statement的first集，判断else为写错还是无需else
                else if(strcmp(t[yfIndex].text, "if") == 0  ||
                        strcmp(t[yfIndex].text, "while") == 0 ||
                        strcmp(t[yfIndex].text, "call") == 0 ||
                        strcmp(t[yfIndex].text, "read") == 0 ||
                        strcmp(t[yfIndex].text, "write") == 0 ||
                        t[yfIndex].type == 0)
                {
                    yfIndex++;
                    error23(t[yfIndex].line);//else出错
                    /* 若是else操作，则需取反；若是else if，则不用取反 */
                    if( strcmp(t[yfIndex].text, "if") != 0 )
                        // gen(boo, 0, 0);
                        cx0 = cx; /* 保存当前指令地址 */
                    gen(jpc, 0, 0); /* 生成条件跳转指令，跳转地址未知，暂时写0 */
                    statement(t,tx,lev);
                    code[cx0].a = cx; /* 回填 */
                }
                
            }
            else
            {
                //拼写错误，不跳过
                yfIndex++;
                statement(t,tx,lev);
                if (strcmp(t[yfIndex].text, "else") == 0) {
                    //else可有可无
                    yfIndex++;
                    statement(t,tx,lev);
                }
                //查找statement的first集，判断else为写错还是无需else
                else if(strcmp(t[yfIndex].text, "if") == 0  ||
                        strcmp(t[yfIndex].text, "while") == 0 ||
                        strcmp(t[yfIndex].text, "call") == 0 ||
                        strcmp(t[yfIndex].text, "read") == 0 ||
                        strcmp(t[yfIndex].text, "write") == 0 ||
                        t[yfIndex].type == 0){
                    yfIndex++;
                    error23(t[yfIndex].line);//else出错
                    statement(t,tx,lev);
                }
            }
        }
        
        
    }
    // while语句 while <lexp> do <statement>
    else if (strcmp(t[yfIndex].text, "while") == 0)
    {
        int opr_num ; // 记录opr的号码
        cx0 = cx; /* 保存判断条件的位置 */
        yfIndex++;
        opr_num = lexp(t,tx,lev);
        cx1 = cx; /* 保存判断条件后一条的位置 */
        gen(jpc, 0, 0); /* 生成条件跳转，但跳出循环的地址未知，标记为0等待回填 */
        if (strcmp(t[yfIndex].text, "do") == 0) {
            yfIndex++;
            statement(t,tx,lev);
            gen(jmp, 0, cx0);   /* 生成条件跳转指令，跳转到前面判断条件操作的位置 */
            code[cx1].a = cx;   /* 回填跳出循环的地址 */
        }
        else error16(t[yfIndex].line);
        //        statement(t,tx,lev);  /* 循环 */
        //        gen(jmp, 0, cx0);   /* 生成条件跳转指令，跳转到前面判断条件操作的位置 */
        //        code[cx1].a = cx;   /* 回填跳出循环的地址 */
    }
    
    // call 语句 call <id>（[<exp>{,<exp>}]）
    else if (strcmp(t[yfIndex].text, "call") == 0) {
        yfIndex++;
        if (t[yfIndex].type == 0) {
            i = position(t[yfIndex].text,*tx);
            if( i == 0 )
            {
                error24(t[yfIndex].line);/* 过程名未定义 */
            }
            else
            {
                yfIndex++;
                if (strcmp(t[yfIndex].text, "(") == 0)
                {
                    yfIndex++;
                    if (strcmp(t[yfIndex].text, ")") == 0) // 无参数
                    {
                        yfIndex++;
                    }
                    else   // 有参数
                    {
                        exp(t,tx,lev);
                        while (strcmp(t[yfIndex].text, ",") == 0)
                        {
                            yfIndex++;
                            exp(t,tx,lev);
                        }
                        if (strcmp(t[yfIndex].text, ")") == 0)
                            yfIndex++;
                        else
                            error7(t[yfIndex].line); // lack of ）
                    }
                }
                else
                    error8(t[yfIndex].line); // lack of (
                if (table[i].kind == procedur)
                {
                    gen(cal, lev-table[i].level, table[i].adr);
                    /* 生成call指令 */
                }
                else
                    error25(t[yfIndex].line); /* call后面不是过程名 */

            }
            
            
            
        }
        else
            error14(t[yfIndex].line); //biaoshifu error
    }
    
    // read 语句 read (<id>{，<id>})
    else if(strcmp(t[yfIndex].text, "read") == 0){
        yfIndex++;
        if (strcmp(t[yfIndex].text, "(") == 0)
        {
            do
            {
                yfIndex++;
                if (t[yfIndex].type == 0)
                {
                    i = position(t[yfIndex].text,*tx);
                }
                else
                {
                    i = 0;
                }
                if (i == 0)
                {
                    error24(t[yfIndex].line); // biaoshifu error
                }
                else
                {
                    gen(opr,0,16);
                    gen(sto,lev-table[i].level,table[i].adr);
                }
                yfIndex++;
            }while (strcmp(t[yfIndex].text, ",") == 0);
            if (strcmp(t[yfIndex].text, ")") == 0)
            {
                yfIndex++;
            }
            else
                error7(t[yfIndex].line);  // lack of )
        }
        else
            error8(t[yfIndex].line); // lack of (
    }
    
    // write语句 write (<exp>{,<exp>})
    else if(strcmp(t[yfIndex].text, "write") == 0)
    {
        yfIndex++;
        if (strcmp(t[yfIndex].text, "(") == 0) {
            do
            {
                yfIndex++;
                exp(t,tx,lev);
                gen(opr,0,14);
                gen(opr,0,15);
            }while (strcmp(t[yfIndex].text, ",") == 0);
            if (strcmp(t[yfIndex].text, ")") == 0) {
                yfIndex++;
            }
            else
                error7(t[yfIndex].line);  // lack of )
        }
        else
            error8(t[yfIndex].line); // lack of (
    }
    
    // <body>
    else
        body(t, tx, lev );
}

/* <body> → begin <statement>{;<statement>}end */
void body(token *t,int* tx,int lev)
{
    string temp1 = t[yfIndex].text;
    bool flag = false;
    if (strcmp(t[yfIndex].text, "begin") == 0) {
        yfIndex++;
        statement(t,tx,lev);
        while(1){
            //缺少分号，并且还有statement，查找statement的first集，则跳过
            while ( strcmp(t[yfIndex].text, ";") == 0 ||
                   strcmp(t[yfIndex].text, "if") == 0  ||
                   strcmp(t[yfIndex].text, "while") == 0 ||
                   strcmp(t[yfIndex].text, "call") == 0 ||
                   strcmp(t[yfIndex].text, "read") == 0 ||
                   strcmp(t[yfIndex].text, "write") == 0 ||
                   t[yfIndex].type == 0){
                if (strcmp(t[yfIndex].text, ";") == 0  &&
                    strcmp(t[yfIndex+1].text, "end") == 0) {
                    //最后一行不能有分号
                    error20(t[yfIndex].line);
                    yfIndex++;
                    break;
                }
                if(strcmp(t[yfIndex].text, ";") == 0){
                    yfIndex++;
                    statement(t,tx,lev);
                }
                else{
                    error0(t[yfIndex-1].line);//缺少分号
                    statement(t,tx,lev);
                }
            }
            
            while (strcmp(t[yfIndex].text,"end") != 0 &&
                   strcmp(t[yfIndex].text,";") != 0) {
                error22(t[yfIndex].line);
                yfIndex++;
            }
            if (strcmp(t[yfIndex].text, "end") == 0) {
                yfIndex++;
            }
            else if(strcmp(t[yfIndex].text, ";") == 0){
                yfIndex++;
                flag = true;
                statement(t,tx,lev);
            }
            else
                error10(t[yfIndex].line); // end error
            if ( flag == true){
                flag = false;
                continue;
            }
            else
                break;
        }
    }
    else
    {
        error11(t[yfIndex].line); // begin error
        while(1){
            yfIndex++;
            
            //查找下一个非终结符的first集,跳过begin
            if (strcmp(t[yfIndex].text, "if") == 0  ||
                strcmp(t[yfIndex].text, "while") == 0 ||
                strcmp(t[yfIndex].text, "call") == 0 ||
                strcmp(t[yfIndex].text, "read") == 0 ||
                strcmp(t[yfIndex].text, "write") == 0 ||
                (t[yfIndex].type == 0 && t[yfIndex+1].type == 1)) {
                statement(t,tx,lev);
                while (strcmp(t[yfIndex].text, ";") == 0) {
                    yfIndex++;
                    statement(t,tx,lev);
                }
                if (strcmp(t[yfIndex].text, "end") == 0) {
                    yfIndex++;
                }
                
            }
        }
    }
    
}

/* <proc> → procedure <id>（[<id>{,<id>]}）;<block>{;<proc>} */
void proc(token *t,int* tx,int lev,int *dx)
{
    if (strcmp(t[yfIndex].text, "procedure") == 0)
    {
        yfIndex++;
        if (t[yfIndex].type == 0)
        {
            enter(t,procedur,tx,lev,dx);   /* 将过程名写入符号表 */
            yfIndex++;
            if ( strcmp(t[yfIndex].text,"(") == 0)
            {
                yfIndex++;
                if (t[yfIndex].type == 0) // 带参数
                {
                    canshu_n++;
                    canshu_s = yfIndex;
                    table[(*tx)].canshu++;
                    yfIndex++;
                    while( strcmp(t[yfIndex].text,",") == 0)
                    {
                        canshu_n++;
                        table[(*tx)].canshu++;
                        yfIndex++;
                        if (t[yfIndex].type == 0) {
                            /* 检查参数是否定义 */
                            int i = position(t[yfIndex].text, *tx);
                            if (i != 0)
                            {
                                yfIndex++;
                            }
                            else
                            {
                                error24(t[yfIndex].line);
                                exit(1);
                            }
                            //                            enter(t,procedur,tx,lev,dx);
                            
                            //                            yfIndex++;
                        }
                        else
                            error1(t[yfIndex].line); //lack of ;
                    }
                    
                }
                // 不带参数，顺序执行
                if (strcmp(t[yfIndex].text, ")") == 0) {
                    yfIndex++;
                    if (strcmp(t[yfIndex].text, ";") == 0) {
                        //                        yfIndex++;
                        block(t,lev+1,*tx);
                        while (strcmp(t[yfIndex].text, ";") == 0) {
                            yfIndex++;
                            proc(t,tx,lev,dx);
                        }
                    }
                    else
                        error0(t[yfIndex].line);
                }
                else
                    error7(t[yfIndex].line); // lack of )
            }
            else
                error8(t[yfIndex].line); // lack of (
        }
        else
            error1(t[yfIndex].line); // 标识符错误
    }
    else error9(t[yfIndex].line,t); // procdure拼写错误
    gen(opr,0,0);  /* 每个过程出口都要使用的释放数据段指令 */
}


/* <vardecl> → var <id>{,<id>}; */
void vardecl(token *t,int* tx,int lev,int* dx)
{
    if ( strcmp(t[yfIndex].text, "var") == 0)
    {
        yfIndex++;
        if( t[yfIndex].type == 0)
        {
            /* 写入符号表 */
            enter(t,variable,tx,lev,dx);
            yfIndex++;
            
            while ( strcmp(t[yfIndex].text, ",") == 0) {
                yfIndex++;
                if (t[yfIndex].type == 0) {
                    /* 写入符号表 */
                    enter(t,variable,tx,lev,dx);
                    yfIndex++;
                }
            }
            
            if ( strcmp(t[yfIndex].text,"procedure") == 0 ||
                strcmp(t[yfIndex].text,"begin") == 0 )
            {
                return;
            }
        }
        else
            error1(t[yfIndex].line);
    }
    else
        error6(t[yfIndex].line);
    
}


/* <const> → <id>:=<integer> */
void constt(token *t,int* tx,int lev,int* dx)
{
    if(t[yfIndex].type == 0)
    {
        yfIndex++;
        if( strcmp(t[yfIndex].text, ":=") == 0)
        {
            yfIndex++;
            if( t[yfIndex].type == 3)
            {
                enter(t,constant,tx,lev,dx); /* 写入符号表 */
                yfIndex++;
            }
            else
                error3(t[yfIndex].line);
        }
        else
            error5(t[yfIndex].line);
    }
    else
        error1(t[yfIndex].line);
}
/* <condecl> → const <const>{,<const>}; */
void condecl(token *t,int* tx,int lev,int* dx)
{
    if( strcmp(t[yfIndex].text, "const")==0 )
    {
        yfIndex++;
        constt(t,tx,lev,dx);
        while(strcmp(t[yfIndex].text, ",") == 0)
        {
            yfIndex++;
            constt(t,tx,lev,dx);
        }
        string temp = t[yfIndex].text;
        
        if ( strcmp(t[yfIndex].text,"var") == 0)
        {
            return;
        }
        else
        {
            error22(t[yfIndex].line);
        }
        
        
    }
    else
    {
        error4(t[yfIndex].line);
    }
}

void rewrite(token *t,int tx0,int cx,int dx,int* cx0)
{
    code[table[tx0].adr].a  = cx; /* 前面跳转指令的位置为当前位置 */
    table[tx0].adr = cx; /* 当前代码地址 */
    table[tx0].size = dx;
    cx0 = &cx;
    gen(ini,0,dx); /* 生成指令，此指令执行时在数据栈中为被调用的过程开辟dx个单元的数据区 */
}

/* <block>  → [<condecl常数>][<vardecl变量>][<proc>]<body> */
/*
 * lev:    当前分程序所在层
 * tx:     符号表的指针
 * fsys:   当前模块后继符号集合
 */
void block(token *t,int lev,int tx)
{
    int dx;         /* 计算每个变量在运行栈中相对本过程基地址的偏移量，
                     放在table的adr中河code的a中 */
    /* tx符号表的下标指针 */
    int tx0 = tx;   /* 初始tx */
    int cx0 = cx;   /* 初始cx */
    dx = 3;         /* 静态链SL、动态链DL和返回地址RA */
    table[tx].adr = cx;
    gen(jmp,0,0);   /* 跳转目的地址未知*/
    
    if( lev > levmax )
    {
        
        cout << "警告：嵌套层数过多" << endl;
    }
    /* 直接把常量的值入栈 */
    yfIndex++;
    bool flag_b = false;
    while(1){
        if( strcmp(t[yfIndex].text, "const") == 0)
        {
            condecl(t,&tx,lev,&dx);
        }
        
        while (canshu_n > 0) {
            enter(t, variable, &tx, lev, &dx);
            canshu_n--;
        }
        
        if( strcmp(t[yfIndex].text, "var") == 0)
        {
            vardecl(t,&tx,lev,&dx);
        }
        
                if(strcmp(t[yfIndex].text, "procedure") == 0)
        {
            proc(t,&tx,lev,&dx);
        }

        rewrite(t, tx0, cx, dx, &cx0);
        
        if( strcmp(t[yfIndex].text, "begin") == 0)
        {
            flag_b = true;
            body(t,&tx,lev);
        }
        
        if (yfIndex >= toIndex ) // 跳出分析器
        {
            break;
        }
        //const var begin 均不匹配时
        if (flag_b == true)
            break;
        else
        {
            error22(t[yfIndex].line);
            yfIndex++;
        }
        flag_b = false;
    }
    
    showtable(tx);
    return;
}


/* <prog> → program <id标识符>；<block> */
void prog(token *t)
{
    if( strcmp(t[yfIndex].text,"program") == 0)
    {
        yfIndex++;
        if( t[yfIndex].type == 0)
        {
            yfIndex++;
            if( strcmp(t[yfIndex].text, ";") == 0)
            {
                block(t,0,0);
            }
            else
            {
                error0(t[yfIndex].line);
                //缺少分号，跳过
                yfIndex--;
                block(t,0,0);
            }
        }
        else
            error1(t[yfIndex].line);//缺少分号
    }
    else
    {
        error2(t[yfIndex].line);
        //program拼写错误，跳下一个
        if( t[yfIndex+1].type == 0)
        {
            yfIndex++;
            if( t[yfIndex].type == 0)
            {
                yfIndex++;
                if( strcmp(t[yfIndex].text, ";") == 0)
                {
                    //                yfIndex++;
                    block(t,0,0);
                }
                else
                    error0(t[yfIndex].line);
            }
            else
                error1(t[yfIndex].line);
        }
        //program漏写
        else if( strcmp(t[yfIndex+1].text, ";"))
        {
            block(t,0,0);
        }
    }
    gen(opr, 0, 0);
    cout << "-------------- 语法分析结束 --------------" << endl;
}

/* 计算地址时的基地址 */
/* instruction.l stack base */
int base(int l,int s[],int b)
{
    int temp = b;
    while(l > 0)
    {
        temp = s[temp];
        return temp;
    }
    return temp;
}
void showcode(int cx0)
{
    for(int i = cx0; i<cx; i++)
    {
        cout << instru_name[code[i].f] << "  " << code[i].l << "  " << code[i].a << endl;
    }
}

int numofcanshu(int adr)
{
    int i = 0;
    for( i = 0;i<txmax;i++)
    {
        if( table[i].kind == 2) //是procedure
        {
            if ( table[i].adr == adr ) //地址相同
            {
                return table[i].canshu;
            }
        }
    }
    return i;
}
void allcode()
{
    fstream file;
    char filename[] = "code.txt";
    file.open(filename);
    if (!file)
    {
        cout << "code文件不存在"<<endl;
        return;
    }
    for(int i = 0;i<cx;i++)
    {
        file << instru_name[code[i].f] << "  " << code[i].l << "  " << code[i].a << endl;
    }
}

void pcode() /* 类PCODE代码解释执行部件 */
{
    int sp = 0; /* 指令指针 */
    int top = 0; /* 栈顶指针 */
    int ba = 1; /* 基址指针 0号不用 */
    int s[stacksize] ;  /* 数据栈 */
    struct instruction temp; /* 临时指针变量 */
    
    s[0] = 0 ; /* 不用 */
    s[1] = 0 ; /* SL：静态链 保存该过程直接外层的活动记录首地址 */
    s[2] = 0 ; /* DL：动态链 调用者的活动记录   首地址 */
    s[3] = 0 ; /* RA：返回地址  */
    do
    {
        temp = code[sp]; /* 取当前指令解释执行 */
        sp = sp + 1;
        switch(temp.f)
        {
            case lit:   /* LIT 0 ，a 取常量a放入数据栈栈顶 */
                s[++top] = temp.a;
                break;
            case opr:  /* OPR 0 ，a 执行运算，a表示执行某种运算 */
                switch( temp.a )
            {
                case 0: /* gen(opr,0,0); 每个过程出口都要使用的释放数据段指令 */
                    top = ba - 1;
                    sp = s[top+3];
                    ba = s[top+2];
                    break;
                case 1: /* 生成取负指令 */
                    s[top] = -s[top];
                case 2:  /* 生成加法指令 */
                    top--;
                    s[top] = s[top] + s[top+1];
                    break;
                case 3:  /* 生成减法指令 次栈顶减去栈顶数据 */
                    top--;
                    s[top] = s[top] - s[top+1];
                    break;
                case 4:  /* 乘法指令 */
                    top--;
                    s[top] = s[top] * s[top+1];
                    break;
                case 5:   /* 除法指令 */
                    top--;
                    s[top] = s[top] / s[top+1];
                    break;
                case 6:   /* odd 奇偶判断  奇数为1 偶数为 0*/
                    s[top] = s[top] % 2;
                    break;
                case 8:   /* = 判断是否相等 */
                    top--;
                    s[top] = ( s[top+1] == s[top]) ? 1:0;
                    break;
                case 9:  /* <> 判断是否不等 */
                    top--;
                    s[top] = ( s[top+1] != s[top]) ? 1:0;
                    break;
                case 10:  /* < 判断是否小于 */
                    top--;
                    s[top] = ( s[top] < s[top+1]) ? 1:0;
                    break;
                case 11: /* <= 判断是否小于等于 */
                    top--;
                    s[top] = ( s[top] <= s[top+1]) ? 1:0;
                    break;
                case 12: /* > 判断是否大于 */
                    top--;
                    s[top] = ( s[top] > s[top+1]) ? 1:0;
                    break;
                case 13: /* >= 判断是否大于等于 */
                    top--;
                    s[top] = ( s[top] >= s[top+1]) ? 1:0;
                    break;
                case 14: /* write 输出一个值 */
                    cout << s[top--] ;
                    break;
                case 15: /* 换行 */
                    cout << endl;
                    top--;
                    break;
                case 16: /* read 读取一个值 */
                    cin >> s[++top];
                    break;
            } //end switch
                break;
            case lod:
                /* 将变量送到运行栈S的栈顶，这时A段为变量所在说明层中的相对位置 */
                s[++top] = s[base(temp.l,s,ba) + temp.a];
                break;
            case sto:
                /* 将运行栈S的栈顶内容送入某个变量单元中，A段为变量所在说明层中的相对位置。 */
                s[base(temp.l,s,ba) + temp.a] = s[top];
                break;
            case jmp:
                /* 无条件转移，这时A段为转向地址（目标程序） */
                sp = temp.a;
                break;
            case jpc:
                /* 条件转移，当运行栈S的栈顶的布尔值为假（0）时，
                 则转向A段所指目标程序地址；否则顺序执行 */
                if( s[top--] == 0 )
                    sp = temp.a;
                break;
            case cal:
                /* 调用过程，这时A段为被调用过程的过程体
                 （过程体之前一条指令）在目标程序区的入口地址。*/
            {
                int i = numofcanshu(temp.a);  // 参数个数
                flagg = i;
                int top0 = top;
                if(i > 0)
                {
                    top = top + 3;
                }
                while( i > 0)
                {
                    s[top+i] = s[top0-i+1];
                    i--;
                }
                top = top0;
                s[top+1] = base(temp.l,s,ba); /* 建立静态链 */
                s[top+2] = ba; /* 建立动态链 */
                s[top+3] = sp; /* 保存返回地址 */
                ba = top+1; /* 调用，更新新的基地址 */
                sp =  temp.a; /* 跳转 */
            }
                break;
            case ini:
                /* 在数据栈中为被调用的过程开辟a个单元的数据区 */
                top += temp.a;
                break;
        }
    }while (sp != 0);
}
void init()  /* 初始化程序 */
{
    err = 0;
    strcpy(instru_name[lit],"lit");
    strcpy(instru_name[opr],"opr");
    strcpy(instru_name[lod],"lod");
    strcpy(instru_name[sto],"sto");
    strcpy(instru_name[cal],"cal");
    strcpy(instru_name[ini],"ini");
    strcpy(instru_name[jmp],"jmp");
    strcpy(instru_name[jpc],"jpc");
}

int main()
{
    token *t = new token[max];
    init();
    ReadFile(t);  /*  读文件 词法分析 */
    prog(t);      /* 语法 语义分析 输出符号表 生成代码 */
    if ( !err )
    {
        allcode(); /* 输出假想代码 */
//        showcode(0);
        cout << "-------------  生成假想代码  ---------------" << endl;
        pcode();
        cout << "-------------  Run Over  ---------------" << endl;
    }
    else
    {
        cout << "存在错误"<<endl;
        cout << "-------------  Run Over  ---------------" << endl;
    }
    return 0;
}
