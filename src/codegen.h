/* 命令語のコード */
typedef enum codes {
    lit,
    opr,
    lod,
    sto,
    cal,
    ret,
    ict,
    jmp,
    jpc,
} OpCode;

/* 演算命令のコード */
typedef enum ops {
    neg,
    add,
    sub,
    mul,
    div,
    odd,
    eq,
    ls,
    gr,
    neq,
    lseq,
    greq,
    wrt,
    wrl
} Operator;

/* 命令語の生成、アドレス部にv */
int genCodeV(OpCode op, int v);
/* 命令語の生成、アドレスは名前表から */
int genCodeT(OpCode op, int ti);
/* 命令語の生成、アドレス部に演算命令 */
int genCodeO(Operator p);
/* ret命令語の生成 */
int genCodeR();
/* 命令語のバックパッチ（次の番地を） */
void backPatch(int i);
/* 次の命令語のアドレスを返す */
int nextCode();
/* 目的コード（命令語）のリスティング */
void listCode();
/* 目的コード（命令語）の実行 */
void execute();
