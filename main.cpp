#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cctype>
#include<cmath>
#include "getword.h"
#include "symbol_table.h"
#define LL long long int
#define REP(i,n) for (int i = 1; i <= (n); i++)
#define Redge(u) for (int k = h[u],to; k; k = ed[k].nxt)
#define cls(s,v) memset(s,v,sizeof(s))
using namespace std;
const int maxn = 100005;


int END;

bool Error = false;
int opt_id_cnt = 0;

node exps0[maxn],expr[maxn];
int exp_n,exp0_n;

void out_put_tabs(){
    for (int i = 1; i <= layer_cnt; i++) printf("\t");
}

//num ident +-*/%()
bool is_not_exp_end(node& u,int& Par) {
    if (u.id == 14) Par++;
    if (u.id != 15) return u.id == 1 || u.id == 2 || (u.id >= 20 && u.id <= 24) || u.id == 14;
    Par--;
    if (Par < 0) return false;
    return true;
}

bool pre_work(){   //将表达式标准化并检查语法正确性
    exp_n = 0;
    int Par = 0;
    for (int i = 1; i <= exp0_n; i++){
        if (exps0[i].id == 20 || exps0[i].id == 21){   //正负相消
            int flag = exps0[i].id == 20 ? 1 : -1;
            while (i + 1 <= exp0_n && (exps0[i + 1].id == 20 || exps0[i + 1].id == 21)){
                flag = exps0[i + 1].id == 20 ? flag : -flag;
                i++;
            }
            if (i == 1 || exps0[i - 1].id == 14){
                expr[++exp_n] = node(2,0,0);
            }
            expr[++exp_n] = flag == 1 ? node(20) : node(21);
            if (i == exp0_n || exps0[i + 1].id == 15) return false;
        }
        else {
            expr[++exp_n] = exps0[i];
            if (exps0[i].id == 14) Par++;  //括号匹配
            else if (exps0[i].id == 15){
                Par--;
                if (Par < 0) return false;
            }
            else if (exps0[i].id == 22 || exps0[i].id == 23 || exps0[i].id == 24){
                if (i == 1 || (exps0[i - 1].id != 2 && exps0[i - 1].id != 1) || i == exp0_n || exps0[i + 1].id == 15){
                    puts("pre_work ErrA");
                    return false;
                }
            }
            else if (exps0[i].id == 1 || exps0[i].id == 2){
                if (i > 1 && (exps0[i - 1].id == 1 || exps0[i - 1].id == 2)){
                    puts("pre_work ErrB");
                    return false;
                }
            }
        }
    }
    if (Par != 0) return false;
    return true;
}

LL s_num[maxn];   //数字栈
int s_tag[maxn],s_opt[maxn],top_num,top_opt;  //数字标识栈,符号栈
char num1[100],num2[100];

void put_top_num(int d){
    if (s_tag[top_num - d] == 0) printf("%lld",s_num[top_num - d]);
    else if (s_tag[top_num - d] == 1) {
        printf("%%%lld",s_num[top_num - d]);
    }
}

void exp_stack_pop(){  //弹出栈顶运算符号并运算
    opt_id_cnt++;
    out_put_tabs();
    printf("%%%d = ",opt_id_cnt);
    switch(s_opt[top_opt]) {
        case 20:printf("add");break;
        case 21:printf("sub");break;
        case 22:printf("mul");break;
        case 23:printf("sdiv");break;
        case 24:printf("srem");break;
        default:break;
    }
    printf(" i32 ");
    num1[0] = num2[0] = '\0';
    put_top_num(1);
    printf(", ");
    put_top_num(0);
    top_num -= 2;
    printf("\n");
    s_num[++top_num] = opt_id_cnt;
    s_tag[top_num] = 1;
    top_opt--;
}

int opt_cmp_chart[][5]={
        {1,1,0,0,0},
        {1,1,0,0,0},
        {1,1,1,1,1},
        {1,1,1,1,1},
        {1,1,1,1,1},
};

bool opt_cmp(int opt_a,int opt_b){
    return opt_cmp_chart[opt_a - 20][opt_b - 20];
}

bool cal_exp(){   //前缀表达式双栈求值
    top_num = 0; top_opt = 0;
    for (int i = 1; i <= exp_n; i++){
        if (expr[i].id == 1){        //Ident
            symbol* p = sym_getIdent(expr[i].name,Space);
            if (p == nullptr) return false;
            out_put_tabs();
            printf("%%%d = load i32, i32* %%%d\n",++opt_id_cnt,p->id);
            s_num[++top_num] = opt_id_cnt;
            s_tag[top_num] = 1;
        }
        else if (expr[i].id == 2){   //num
            s_num[++top_num] = expr[i].num;
            s_tag[top_num] = 0;
        }
        else if (expr[i].id == 14){   //(
            s_opt[++top_opt] = 14;
        }
        else if (expr[i].id == 15){   //)
            while (s_opt[top_opt] != 14){  //弹出符号直至(
                exp_stack_pop();
            }
            top_opt--;
        }
        else if (expr[i].id >= 20 && expr[i].id <= 24){  //opt
            while (top_opt > 0 && s_opt[top_opt] != 14 && opt_cmp(s_opt[top_opt],expr[i].id)){
                exp_stack_pop();
            }
            s_opt[++top_opt] = expr[i].id;
        }
    }
    while (top_opt){
        exp_stack_pop();
    }
    return true;
}

int Exp(int head){        //表达式求值
    int pos = head,Par = 0;
    exp0_n = 0;
    while (is_not_exp_end(words[pos],Par)){
        exps0[++exp0_n] = words[pos++];
    }
    if (words[pos].id == 100) {Error = true; puts("Error at Exp 1"); return END;}
    if (!pre_work()) {Error = true; puts("Error at Exp 2"); return END;}
    if (!cal_exp()) {Error = true; puts("Error at Exp 3"); return END;}
    return pos;
}

int ConstExp(int head){
    int pos = head;
    pos = Exp(pos);
    return pos;
}

int const_declare(int pos){  //@插入常量声明
    sym_insert(words[pos].name,Space,++opt_id_cnt,true);
    out_put_tabs();
    printf("%%%d = alloca i32\n",opt_id_cnt);
    return opt_id_cnt;
}

void const_var_init(int id){  //@插入常量初始化
    out_put_tabs();
    if (s_tag[top_num]) printf("store i32 %%%d, i32* %%%d\n",s_num[top_num],id);
    else printf("store i32 %d, i32* %%%d\n",s_num[top_num],id);
}

int var_declare(int pos){  //@插入变量声明
    out_put_tabs();
    if (!sym_insert(words[pos].name,Space,++opt_id_cnt,false)) Error = true;
    printf("%%%d = alloca i32\n",opt_id_cnt);
    return opt_id_cnt;
}

void var_modify(int id){    //@插入变量修改
    out_put_tabs();
    if (s_tag[top_num]) printf("store i32 %%%d, i32* %%%d\n",s_num[top_num],id);
    else printf("store i32 %d, i32* %%%d\n",s_num[top_num],id);
}

int ConstDef(int head){          //常量声明
    int pos = head;
    if (words[pos].id == 1){   //Ident
        int ident_id = const_declare(pos);    //@
        pos++;
        if (words[pos].id == 12){ //=
            pos = ConstExp(pos + 1); //@
            const_var_init(ident_id);   //@
        }
        else {Error = true; return END;}
    }
    else {Error = true; return END;}
    return pos;
}

int VarDef(int head){          //变量声明
    int pos = head;
    if (words[pos].id == 1){   //Ident
        int ident_id = var_declare(pos);    //@
        if (Error) return END;
        pos++;
        if (words[pos].id == 12){ //=
            pos = Exp(pos + 1); //@
            var_modify(ident_id);   //@
        }
    }
    else {Error = true; return END;}
    return pos;
}

int BType(int head){             //数据类型
    int pos = head;
    if (words[pos].id != 9) {Error = true; return END;}  //int
    return pos + 1;
}

int VarDecl(int head){      //变量声明
    int pos = head;
    pos = BType(pos);
    pos = VarDef(pos);
    while (words[pos].id == 28){ //,
        pos = VarDef(pos + 1);
    }
    if (words[pos].id != 13) {Error = true; return END;} //;
    pos++;
    return pos;
}

int ConstDecl(int head){
    int pos = head;
    if (words[pos].id == 11){ //const
        pos++;
        pos = BType(pos);
        pos = ConstDef(pos);
        while (words[pos].id == 28){ //,
            pos = ConstDef(pos + 1);
        }
        if (words[pos].id != 13) {Error = true; return END;} //;
        pos++;
    }
    return pos;
}

int Decl(int head){
    int pos = head;
    if (words[pos].id == 11) pos = ConstDecl(pos); //const
    else if (words[pos].id == 9) pos = VarDecl(pos);  //int
    else {Error = true; return END;}
    return pos;
}

void return_value(){   //@return
    out_put_tabs();
    if (s_tag[top_num]) printf("ret i32 %%%d\n",s_num[top_num]);
    else printf("ret i32 %d\n",s_num[top_num]);
}

int Stmt(int head){
    int pos = head;
    if (words[pos].id == 8){ //return
        pos = Exp(pos + 1);
        if (words[pos].id != 13) {Error = true; puts("Error at Stmt 1"); return END;}
        return_value(); //@return
        pos++;
    }
    //赋值
    else if (words[pos].id == 1 && words[pos + 1].id == 12){
        symbol *p = sym_getIdent(words[pos].name,Space);
        if (p == nullptr) {Error = true; puts("Error at Stmt 2"); return END;}
        if (p->is_const) {Error = true; puts("Error at Stmt 2.5:modify const"); return END;}
        int ident_id = p->id;
        //printf("%d",words[pos+2].id);
        pos = Exp(pos + 2); //@
        var_modify(ident_id);   //@
        if (words[pos].id != 13) {Error = true; puts("Error at Stmt 3"); return END;}
        pos++;
    }
    //Exp
    else {
        pos = Exp(pos);
        if (words[pos].id != 13) {Error = true; puts("Error at Stmt 4"); return END;}
        pos++;
    }
    return pos;
}

int BlockItem(int head){
    int pos = head;
    while (words[pos].id != 17 && !Error) {   //遇到{退出
        if (words[pos].id == 11 || words[pos].id == 9) {  //const  int
            pos = Decl(pos);
        } else {
            pos = Stmt(pos);
        }
    }
    return pos;
}

int Block(int head){
    int pos = head;
    if (words[pos].id == 16){   //{
        printf("{\n");
    }
    else {Error = true; puts("Error at Block 1"); return END;}
    pos++;
    layer_cnt++;
    pos = BlockItem(pos);
    layer_cnt--;
    if (words[pos].id == 17){   //}
        out_put_tabs();
        printf("}\n");
    }
    else {Error = true; puts("Error at Block 2"); return END;}
    pos++;
    return pos;
}

int FuncFParams(int head){    //参数列表

    return 0;
}

int FuncDef(int head){
    int pos = head;
    printf("define ");
    if (words[pos].id == 10){
        printf("void ");
    }
    else if (words[pos].id == 9){
        printf("i32 ");
    }
    else {Error = true; return END;}
    pos++;
    if (words[pos].id == 1){
        printf("@%s",words[pos].name);
    }
    else {Error = true; return END;}
    pos++;
    if (words[pos].id == 14){   //(
        printf("(");
    }
    else {Error = true; return END;}
    pos++;
    if (words[pos].id == 15){   //)
        printf(")");
    }
    else {
        pos = FuncFParams(pos);  //参数列表
        if (words[pos].id != 15) {Error = true; return END;}
        printf(")");
        pos++;
    }
    Space_cnt++;
    Space_pre[Space_cnt] = Space;
    Space = Space_cnt;
    pos = Block(pos + 1);
    return pos;
}

int CompUnit(int head){
    int pos = head;
    while (pos <= words_len){
        if (words[pos].id == 11) pos = Decl(pos);     //const
        else if (words[pos].id == 10) pos = FuncDef(pos);   //void
        else if (words[pos].id == 9 && words[pos + 1].id == 1){   //int Ident
            if (words[pos + 2].id == 14) pos = FuncDef(pos);            //(
            else pos = Decl(pos);
        }
        else if (words[pos].id == 100) return pos;
        else {Error = true; return END;}
    }
}

int main(){
    //freopen("in.txt","r",stdin);
    //freopen("out.txt","w",stdout);
    get_sym();
    /*for (int i = 1; i <= words_len; i++){
        printf("%d ",words[i].id);
    }*/
    END = words_len;
    CompUnit(1);
    if (Error) return 1;
    return 0;
}
