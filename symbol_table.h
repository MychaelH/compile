#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cctype>
#include<cmath>
#define LL long long int
#define REP(i,n) for (int i = 1; i <= (n); i++)
#define Redge(u) for (int k = h[u],to; k; k = ed[k].nxt)
#define cls(s,v) memset(s,v,sizeof(s))
#define ULL unsigned long long int
using namespace std;

const int P = 100007;

int Space_cnt = 0; //环境总数
int Space = 0;   //当前环境编号
int Space_pre[P]; //父亲环境
int layer_cnt = 0;

struct func_params{
    int type;   //0:常数  1:指针
    func_params *next;
    func_params(){type = 0; next = nullptr;}
    func_params(int a){type = a; next = nullptr;}
    func_params(int a,func_params *b){type = a; next = b;}
};

struct symbol{
    char* name;   //变量名
    int space;   //作用域
    int id;		//IR变量编号
    bool is_const;  //是否常数
    bool is_func;
    int re_type;   //0void    1:i32
    func_params *params;
    symbol(){}
    symbol(const char* name, int space, int id, bool is_const, bool is_func, int re_type, func_params* params){
        this->name = new char[sizeof(char) * (strlen(name) + 3)];
        strcpy(this->name,name);
        this->space = space;
        this->id = id;
        this->is_const = is_const;
        this->is_func = is_func;
        this->re_type = re_type;
        this->params = params;
    }
};

int symbol_siz = 0;

struct s_node{
    symbol* u;
    s_node* next;
};

s_node* table_head[100010] = {nullptr};

int get_code(const char* s){
    ULL x = 0;
    for (int i = 0; s[i] != '\0'; i++){
        x = x * 37 + s[i];
    }
    return (int)(x % P);
}


symbol* sym_getIdent(const char* name,int space){
    //puts(name);
    int code = get_code(name);
    int t_space = space;
    while (true) {
        //puts("finding");
        s_node* u = table_head[code];
        while (u != nullptr && (strcmp(name, u->u->name) != 0 || u->u->space != t_space)) {
            //puts("next");
            u = u->next;
        }
        if (u != nullptr) return u->u;
        if (!t_space) break;
        t_space = Space_pre[t_space];
    }
    return nullptr;
}

bool sym_insert(const char* name,int space,int id,bool is_const = false,bool is_func = false,int re_type = 0,func_params *params = nullptr){
    //puts(name);
    symbol* t = sym_getIdent(name,space);
    if (t != nullptr && t->space == space) return false;
    int code = get_code(name);
    auto *p_s = new symbol(name,space,id,is_const,is_func,re_type,params);
    auto* v = new s_node;
    v->u = p_s; v->next = nullptr;
    if (table_head[code] == nullptr){
        table_head[code] = v;
        return true;
    }
    s_node* u = table_head[code];
    while (u->next != nullptr) u = u->next;
    u->next = v;
    return true;
}

bool sym_modify(const char* name,int space,int id){
    symbol *u = sym_getIdent(name,space);
    if (u->is_const) return false;
    u->id = id;
    return true;
}


#endif //SYMBOL_TABLE_H




