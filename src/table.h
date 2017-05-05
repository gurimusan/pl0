/* Identifierの種類 */
typedef enum kindT {
    varId,
    funcId,
    parId,
    constId
} KindT;

/* 変数、パラメタ、関数のアドレスの型 */
typedef struct relAddr {
    int level;
    int addr;
} RelAddr;

/* ブロックの始まりで（最初の変数の番地）呼ばれる */
void blockBegin(int firstAddr);
/* ブロックの終わりで呼ばれる */
void blockEnd();
/* 現ブロックのレベルを返す */
int bLevel();
/* 現ブロックの関数のパラメタ数を返す */
int fPars();
/* 名前表に関数目と先頭番地を登録 */
int enterTfunc(char *id, int v);
/* 名前表に変数名を登録 */
int enterTvar(char *id);
/* 名前表にパラメタ名を登録 */
int enterTpar(char *id);
/* 名前表に定数目とその値を登録 */
int enterTconst(char *id, int v);
/* パラメタの宣言部の最後で呼ばれる */
void endpar();
/* 名前表[ti]の値（関数の先頭番地）の変更 */
void changeV(int ti, int newVal);
/* 名前idの名前表の位置を返す、未宣言の時エラーとする */
int searchT(char *id, KindT k);
/* 名前表[i]の種類を返す */
KindT kindT(int i);
/* 名前表[ti]のアドレスを返す */
RelAddr relAddr(int ti);
/* 名前表[ti]のvalueを返す */
int val(int ti);
/* 名前表[ti]の関数のパラメタ数を返す */
int pars(int ti);
char* tname(int ti);
/* そのブロックで実行時に必要とするメモリー容量 */
int frameL();
