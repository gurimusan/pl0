#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getSource.h"

#define MAXLINE 120
#define MAXERROR 30
#define MAXNUM 14
#define TAB 5

/* ソースファイル */
static FILE *fpi;
/* LaTex出力ファイル */
static FILE *fptex;
/* 1行分の入力バッファー */
static char line[MAXLINE];
/* 次に読む文字の位置 */
static int lineIndex;
/* 最後に読んだ文字 */
static char ch;

/* 最後に読んだトークン */
static Token cToken;
/* 現トークン（Id）の種類 */
static KindT idKind;
/* そのトークンの前のスペースの個数 */
static int spaces;
/* その前のCRの個数 */
static int CR;
/* トークンは印字済みか */
static int printed;

/* 出力したエラーの数 */
static int errorNo = 0;
/* 次の文字を読む関数 */
static char nextChar();
/* tは記号か？ */
static int isKeySym(KeyId k);
/* tは予約語か？ */
static int isKeyWd(KeyId k);
/* 予約語の検索 */
static int findKeyWd(char *m);
/* トークンの前のスペースの印字 */
static void printSpaces();
/* トークンの印字 */
static void printcToken();

/* 予約語や記号と名前（KeyId） */
struct keyWd {
    char *word;
    KeyId keyId;
};

/* 予約語や記号と名前（KeyId）の表 */
static struct keyWd KeyWdT[] = {
    {"begin", Begin},
    {"const", Const},
    {"do", Do},
    {"end", End},
    {"function", Func},
    {"if", If},
    {"odd", Odd},
    {"return", Ret},
    {"then", Then},
    {"var", Var},
    {"while", While},
    {"write", Write},
    {"writeln", WriteLn},
    {"$dummy1", end_of_KeyWd},
    {"+", Plus},
    {"-", Minus},
    {"*", Mult},
    {"/", Div},
    {"(", Lparen},
    {")", Rparen},
    {"=", Equal},
    {"<", Lss},
    {">", Gtr},
    {"<>", NotEq},
    {"<=", LssEq},
    {">=", GtrEq},
    {",", Comma},
    {".", Period},
    {";", Semicolon},
    {":=", Assign},
    {"$dummy2", end_of_KeySym},
};

/* キーkは予約語か？ */
int isKeyWd(KeyId k)
{
    return (k < end_of_KeyWd);
}

/* キーkは記号か？ */
int isKeySym(KeyId k)
{
    if (k < end_of_KeyWd) {
        return 0;
    }
    return (k < end_of_KeySym);
}

/* 文字の種類を示す表にする */
static KeyId charClassT[256];

/* 文字の種類を示す表を作る関数 */
static void initCharClassT()
{
    int i;
    for (i=0; i<256; ++i) {
        charClassT[i] = others;
    }
    for (i='0'; i<='9'; ++i) {
        charClassT[i] = digit;
    }
    for (i='A'; i<='Z'; ++i) {
        charClassT[i] = letter;
    }
    for (i='a'; i<='z'; ++i) {
        charClassT[i] = letter;
    }
    charClassT['+'] = Plus;
    charClassT['-'] = Minus;
    charClassT['*'] = Mult;
    charClassT['/'] = Div;
    charClassT['('] = Lparen;
    charClassT[')'] = Rparen;
    charClassT['='] = Equal;
    charClassT['<'] = Lss;
    charClassT['>'] = Gtr;
    charClassT[','] = Comma;
    charClassT['.'] = Period;
    charClassT[';'] = Semicolon;
    charClassT[':'] = colon;
}

int openSource(char fileName[])
{
    char fileNameO[30];
    if ((fpi = fopen(fileName, "r")) == NULL) {
        printf("can't open %s\n", fileName);
        return 0;
    }
    strcpy(fileNameO, fileName);
    strcat(fileNameO, ".tex");
    if ((fptex = fopen(fileNameO, "w")) == NULL) {
        printf("can't open %s\n", fileNameO);
        return 0;
    }
    return 1;
}

void closeSource()
{
    fclose(fpi);
    fclose(fptex);
}

void initSource()
{
    lineIndex = -1;
    ch = '\n';
    printed = 1;
    initCharClassT();
    fprintf(fptex, "\\documentstyle[12pt]{article}\n");
    fprintf(fptex, "\\begin{document}\n");
    fprintf(fptex, "\\fboxsep=0pt\n");
    fprintf(fptex, "\\def\\insert#1{$\\fbox{#1}$}\n");
    fprintf(fptex, "\\def\\delete#1{$\\fboxrule=.5mm\\fbox{#1}$}\n");
    fprintf(fptex, "\\rm\n");
}

void finalSource()
{
    if (cToken.kind == Period) {
        printcToken();
    } else {
        errorInsert(Period);
    }
    fprintf(fptex, "\n\\end{document}\n");
}

/* 通常のエラーメッセージの出力 */
void error(char *m)
{
    if (lineIndex > 0) {
        printf("%*s\n", lineIndex, "***^");
    } else {
        printf("^\n");
    }
    printf("*** error *** %s\n", m);
    ++errorNo;
    if (errorNo > MAXERROR) {
        printf("too many errors\n");
        printf("abort compilation\n");
        exit(1);
    }
}

/* エラーの個数のカウント、多すぎたら終わり */
void errorNoCheck()
{
    if (errorNo++ > MAXERROR) {
        fprintf(fptex, "too many errors\n\\end{document}\n");
        printf("abort compilation\n");
        exit(1);
    }
}

/* 型エラーを.texファイルに出力 */
void errorType(char *m)
{
    printSpaces();
    fprintf(fptex, "\\(\\stackrel{\\mbox{\\scriptsize %s}}{\\mbox{", m);
    printcToken();
    fprintf(fptex, "}}\\)");
    errorNoCheck();
}

/* keyString(k)を.texファイルに挿入 */
void errorInsert(KeyId k)
{
    if (k < end_of_KeyWd) {
        fprintf(fptex, "\\  \\insert{{\\bf %s}}", KeyWdT[k].word);
    } else {
        fprintf(fptex, "\\  \\insert{$%s$}", KeyWdT[k].word);
    }
    errorNoCheck();
}

/* 名前がないとのエラーメッセージを.texファイルに挿入 */
void errorMissingId()
{
    fprintf(fptex, "\\insert{Id}");
    errorNoCheck();
}

/* 演算子がないとのメッセージを.texファイルに挿入 */
void errorMissingOp()
{
    fprintf(fptex, "\\insert{$\\otimes$}");
    errorNoCheck();
}

/* 今読んだトークンを読み捨てる */
void errorDelete()
{
    int i = (int)cToken.kind;
    printSpaces();
    printed = 1;
    if (i < end_of_KeyWd) {             /* 予約語 */
        fprintf(fptex, "\\delete{{\\bf %s}}", KeyWdT[i].word);
    } else if (i < end_of_KeySym) {     /* 演算子か区切り記号 */
        fprintf(fptex, "\\delete{{$%s$}}", KeyWdT[i].word);
    } else if (i == (int)Id) {          /* Identifier */
        fprintf(fptex, "\\delete{%s}", cToken.u.id);
    } else if (i == (int)Num) {         /* Num */
        fprintf(fptex, "\\delete{%d}", cToken.u.value);
    }
}

/* エラーメッセージを.texファイルに出力 */
void errorMessage(char *m)
{
    fprintf(fptex, "$^{%s}$", m);
    errorNoCheck();
}

/* エラーメッセージを出力し、コンパイル終了 */
void errorF(char *m)
{
    errorMessage(m);
    fprintf(fptex, "fatal errors\n\\end{document}\n");
    if (errorNo) {
        printf("total %d errors\n", errorNo);
    }
    printf("abort compilation\n");
    exit(1);
}

/* エラーの個数を返す */
int errorN()
{
    return errorNo;
}

/* 次の1文字を返す関数 */
char nextChar()
{
    char ch;
    if (lineIndex == -1) {
        if (fgets(line, MAXLINE, fpi) != NULL) {
            puts(line);
            lineIndex = 0;
        } else {
            errorF("end of file\n");
        }
    }
    if ((ch = line[lineIndex++]) == '\n') {
        lineIndex = -1;
        return '\n';
    }
    return ch;
}

/* 次のトークンを読んで返す関数 */
Token nextToken()
{
    int i = 0;
    int num;
    KeyId cc;
    Token temp;
    char ident[MAXNAME];

    /* 前のトークンを印字 */
    printcToken();

    /* 次のトークンまでの空白や改行をカウント */
    spaces = 0;
    CR = 0;
    while (1) {
        if (ch == ' ') {
            ++spaces;
        } else if (ch == '\t') {
            spaces += TAB;
        } else if (ch == '\n') {
            spaces = 0;
            ++CR;
        } else {
            break;
        }
        ch = nextChar();
    }

    switch (cc = charClassT[ch]) {
        case letter:    /* identifier */
            do {
                if (i < MAXNAME) {
                    ident[i] = ch;
                }
                ++i;
                ch = nextChar();
            } while (charClassT[ch] == letter || charClassT[ch] == digit);
            if (i >= MAXNAME) {
                errorMessage("too long");
                i = MAXNAME - 1;
            }
            ident[i] = '\0';
            /* 予約語の場合 */
            if ((i = findKeyWd(ident)) != -1) {
                temp.kind = KeyWdT[i].keyId;
                cToken = temp;
                printed = 0;
                return temp;
            }
            /* ユーザの宣言した名前の場合 */
            temp.kind = Id;
            strcpy(temp.u.id, ident);
            break;
        case digit:     /* number */
            num = 0;
            do {
                num = 10*num + (ch - '0');
                ++i;
                ch = nextChar();
            } while (charClassT[ch] == digit);
            if (i > MAXNUM) {
                errorMessage("too large");
            }
            temp.kind = Num;
            temp.u.value = num;
            break;
        case colon:
            if ((ch = nextChar()) == '=') {
                ch = nextChar();
                temp.kind = Assign;
            } else {
                temp.kind = nul;
            }
            break;
        case Lss:
            if ((ch = nextChar()) == '=') {
                ch = nextChar();
                temp.kind = LssEq;
            } else if (ch == '>') {
                ch = nextChar();
                temp.kind = NotEq;
            } else {
                temp.kind = Lss;
            }
            break;
        case Gtr:
            if ((ch = nextChar()) == '=') {
                ch = nextChar();
                temp.kind = GtrEq;
            } else {
                temp.kind = Gtr;
            }
            break;
        default:
            temp.kind = cc;
            ch = nextChar();
            break;
    }
    cToken = temp;
    printed = 0;
    return temp;
}

/* t==k のチェック */
Token checkGet(Token t, KeyId k)
{
    if (t.kind == k) {
        return nextToken();
    }
    if ((isKeyWd(k) && isKeyWd(t.kind)) || (isKeySym(k) && isKeySym(t.kind))) {
        errorDelete();
        errorInsert(k);
        return nextToken();
    }
    errorInsert(k);
    return t;
}

void setIdKind(KindT k)
{
    idKind = k;
}

static int findKeyWd(char *m)
{
    int i;
    int cmp;
    int lft = 0;
    int rgt = end_of_KeyWd;
    do {
        i = lft + (rgt - lft) / 2;
        cmp = strcmp(m, KeyWdT[i].word);
        if(cmp == 0) {
            return i;
        } else if ((rgt - lft) <= 1) {
            break;
        } else if (cmp < 0) {
            rgt = i;
        } else if (cmp > 0) {
            lft = i;
        }
    } while (i > 0 && i < end_of_KeyWd);
    return -1;
}

/* 空白や改行の印字 */
static void printSpaces()
{
    while (CR-->0) {
        fprintf(fptex, "\\ \\par\n");
    }
    while (spaces-->0) {
        fprintf(fptex, "\\ ");
    }
    CR = 0;
    spaces = 0;
}

/* 現在のトークンの印字 */
void printcToken()
{
    int i = (int)cToken.kind;
    if (printed) {
        printed = 0;
        return;
    }

    /* トークンの前の空白や改行印字 */
    printSpaces();

    /* 予約語 */
    if (i < end_of_KeyWd) {
        fprintf(fptex, "{\\bf %s}", KeyWdT[i].word);
    }
}


