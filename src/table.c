#include <string.h>
#ifndef TABLE
#define TBL
#include "table.h"
#endif
#include "getSource.h"

/* 名前表の最大長 */
#define MAXTABLE 100
/* 名前の最大長 */
#define MAXNAME 31
/* ブロックの最大の深さ */
#define MAXLEVEL 5

/* 名前表のエントリーの型 */
typedef struct tableE {
    KindT kind;             /* 名前の種類 */
    char name[MAXNAME];     /* 名前のつづり */
    union {
        int value;          /* 定数の場合: 値 */
        struct {
            RelAddr raddr;  /* 関数の場合: 先頭アドレス */
            int pars;       /* 関数の場合: パラメタ数 */
        } f;
        RelAddr raddr;      /* 変数、パラメタの場合: アドレス */
    } u;
} TableE;

/* 名前表 */
static TableE nameTable[MAXTABLE];
/* 名前表のインデックス */
static int tIndex = 0;
/* 現在のブロックレベル */
static int level = -1;
/* bIndex[i]にはブロックレベルiの最後のインデックス */
static int bIndex[MAXLEVEL];
/* addr[i]にはブロックレベルiの最後の変数の番地 */
static int addr[MAXLEVEL];
/* 現在のブロックの最後の変数の番地 */
static int localAddr;
static int tfIndex;

/* 名前の種類の出力用関数 */
static char* kindName(KindT k)
{
    switch (k) {
        case varId: return "var";
        case parId: return "par";
        case funcId: return "func";
        case constId: return "const";
    }
    return NULL;
}

/* ブロックの始まり（最初の変数の番地）で呼ばれる */
void blockBegin(int firstAddr)
{
    /* 主ブロックの時、初期設定 */
    if (level == -1) {
        localAddr = firstAddr;
        tIndex = 0;
        ++level;
        return;
    }
    if (level == MAXLEVEL-1) {
        errorF("too many nested blocks");
    }
    bIndex[level] = tIndex;
    addr[level] = localAddr;
    localAddr = firstAddr;
    ++level;
    return;
}

/* ブロックの終りで呼ばれる */
void blockEnd()
{
    --level;
    tIndex = bIndex[level];
    localAddr = addr[level];
}

/* 現ブロックのレベルを返す */
int bLevel()
{
    return level;
}

/* 現ブロックの関数のパラメタ数を返す */
int fPars()
{
    return nameTable[bIndex[level-1]].u.f.pars;
}

/* 名前表に名前を登録 */
void enterT(char *id)
{
    if (++tIndex < MAXTABLE) {
        strcpy(nameTable[tIndex].name, id);
    } else {
        errorF("too many names");
    }
}

/* 名前表に関数名と先頭番地を登録 */
int enterTfunc(char *id, int v)
{
    enterT(id);
    nameTable[tIndex].kind = funcId;
    nameTable[tIndex].u.f.raddr.level = level;
    nameTable[tIndex].u.f.raddr.addr = v;
    nameTable[tIndex].u.f.pars = 0;
    tfIndex = tIndex;
    return tIndex;
}

/* 名前表のパラメタ名を登録 */
int enterTpar(char *id)
{
    enterT(id);
    nameTable[tIndex].kind = parId;
    nameTable[tIndex].u.raddr.level = level;
    nameTable[tfIndex].u.f.pars++;
    return tIndex;
}

/* 名前表に変数名を登録 */
int enterTvar(char *id)
{
    enterT(id);
    nameTable[tIndex].kind = varId;
    nameTable[tIndex].u.raddr.level = level;
    nameTable[tIndex].u.raddr.addr = localAddr++;
    return tIndex;
}

/* 名前表に定数名とその値を登録 */
int enterTconst(char *id, int v)
{
    enterT(id);
    nameTable[tIndex].kind = constId;
    nameTable[tIndex].u.value = v;
    return tIndex;
}

/* パラメタ宣言部の最後に呼ばれる */
void endpar()
{
    int i;
    int pars = nameTable[tIndex].u.f.pars;
    if (pars == 0) return;
    for (i=1; i<=pars; ++i) {
        nameTable[tfIndex+i].u.raddr.addr = i - 1 - pars;
    }
}

/* 名前表[ti]の値（関数の先頭番地）の変更 */
void changeV(int ti, int newVal)
{
    nameTable[ti].u.f.raddr.addr = newVal;
}

/* 名前idの名前表の位置を返す */
int searchT(char *id, KindT k)
{
    int i;
    i = tIndex;
    strcpy(nameTable[0].name, id);
    while (strcmp(id, nameTable[i].name)) {
        i--;
    }
    if (i) {
        return i;
    } else {
        errorType("undef");
        if (k == varId) return enterTvar(id);
        return 0;
    }
}

/* 名前表[i]の種類を返す */
KindT kindT(int i)
{
    return nameTable[i].kind;
}

/* 名前表[ti]のアドレスを返す */
RelAddr relAddr(int ti)
{
    return nameTable[ti].u.raddr;
}

/* 名前表[ti]のvalueを返す */
int val(int ti)
{
    return nameTable[ti].u.value;
}

/* 名前表[ti]の関数パラメタ数を返す */
int pars(int ti)
{
    return nameTable[ti].u.f.pars;
}

char* tname(int ti)
{
    return nameTable[ti].name;
}

/* そのブロックで実行時に必要とするメモリー容量 */
int frameL()
{
    return localAddr;
}
