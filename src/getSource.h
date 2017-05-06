#include <stdio.h>
#ifndef TBL
#define TBL
#include "table.h"
#endif

/* 名前の最大の長さ */
#define MAXNAME 31

/* キーや文字の種類（名前） */
typedef enum keys {
    Begin, End,
    If, Then, Else,
    While, Do,
    Ret, Func,
    Var, Const, Odd,
    Write, WriteLn,
    end_of_KeyWd,
    Plus, Minus,
    Mult, Div,
    Lparen, Rparen,
    Equal, Lss, Gtr,
    NotEq, LssEq, GtrEq,
    Comma, Period, Semicolon,
    Assign,
    end_of_KeySym,
    Id, Num, nul,
    end_of_Token,
    letter, digit, colon, others
} KeyId;

/* トークンの型 */
typedef struct token {
    KeyId kind;
    union {
        char id[MAXNAME];
        int value;
    } u;
} Token;

/* 次のトークンを読んで返す */
Token nextToken();
/* t.kind==kのチェック */
Token checkGet(Token t, KeyId k);
/* ソースファイルのopen */
int openSource(char fileName[]);
/* ソースファイルのclose */
void closeSource();
/* テーブルの初期化、texファイルの初期設定  */
void initSource();
/* ソースの最終チェック、texファイルの最終設定 */
void finalSource();
/* 型エラーを.texファイルに出力 */
void errorType(char *m);
/* keyString(k)を.texファイルに挿入 */
void errorInsert(KeyId k);
/* 名前がないとのメッセージを.texファイルに挿入 */
void errorMissingId();
/* 演算子がないとのメッセージを.texファイルに挿入 */
void errorMissingOp();
/* 今読んだトークンを読み捨て（.texファイルに出力） */
void errorDelete();
/* エラーメッセージを.texファイルに出力 */
void errorMessage(char *m);
/* エラーメッセージを出力し、コンパイル終了 */
void errorF(char *m);
/* エラーの個数を返す */
int errorN();
/* 現トークン（Id）の種類をセット（.texファイル出力のため） */
void setIdKind(KindT k);
