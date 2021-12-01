#include<iostream>
#include<cstdio>
#include<stack>
#include<queue>
#include<vector>
#include "getword.h"
#include "symbol_table.h"
#include "Output_region.h"
#define int1 long long int
using namespace std;


int END;

bool Error = false;

bool put_top_num(stack<int>& s_num, stack<int>& s_tag, var_node& re, Output_region& out){
    if (!s_num.size()) return false;
    re = var_node(s_tag.top(), s_num.top());
    return true;
}

bool exp_stack_pop(stack<int>& s_num, stack<int>& s_tag, stack<int>& s_opt, stack<int>& s_opt_type, Output_region& out){  //弹出栈顶运算符号并运算
    int id = out.get_new_id();
    if (s_opt_type.top()){  //如果是单目运算符，弹出一个
        var_node re;
        if (!put_top_num(s_num,s_tag, re, out)) {Error = true;printf("Err at exp_stack_pop");return false;}
        s_num.pop(); s_tag.pop();
        switch(s_opt.top()) {
            case 20:out.insert_add(id, var_node(0,0), re); break;
            case 21:out.insert_sub(id, var_node(0,0), re);break;
            default:Error = true;printf("Err at exp_stack_pop");return false;
        }
        s_opt.pop(); s_opt_type.pop();
        s_num.push(id);
        s_tag.push(1);
        return true;
    }
    var_node re1, re2;
    if (!put_top_num(s_num,s_tag, re1, out)) {Error = true;printf("Err at exp_stack_pop");return false;}
    s_num.pop(); s_tag.pop();
    if (!put_top_num(s_num,s_tag, re2, out)) {Error = true;printf("Err at exp_stack_pop");return false;}
    s_num.pop(); s_tag.pop();
    switch(s_opt.top()) {
        case 20:out.insert_add(id, re2, re1);break;
        case 21:out.insert_sub(id, re2, re1);break;
        case 22:out.insert_mul(id, re2, re1);break;
        case 23:out.insert_sdiv(id, re2, re1);break;
        case 24:out.insert_srem(id, re2, re1);break;
        default:Error = true;printf("Err at exp_stack_pop");return false;
    }
    s_num.push(id); s_tag.push(1);
    s_opt.pop(); s_opt_type.pop();
    return true;
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

int Exp(int head, int& re_id, int& re_type, Output_region& out, bool is_const = false){        //表达式求值
    int pos = head, Par = 0, i = 0,is_last_num = false;
    stack<int> s_num, s_tag, s_opt, s_opt_type;
    while (true){
        i++;
        if (words[pos].id == 1){        //Ident
            symbol* p = sym_getIdent(words[pos].name,Space);
            if (p == nullptr) {Error = true; puts("Exp Ident not found"); return END;}
            if (is_const && p->is_const == false) {Error = true; puts("Should not put a var in a constexp"); return END;}
            if (!p->is_func) {
                int id = out.get_new_id();
                out.insert_load(id,var_node(2,p->id));
                s_num.push(id);
                s_tag.push(1);
                is_last_num = true;
            }
            else {            //function
                if (p->re_type == 0) {Error = true;puts("Can't put a void function in an exp"); return END;}
                if (words[++pos].id != 14) {Error = true; return END;} //(
                func_params *u = p->params;
                bool first = true;
                queue<int> param_id;
                queue<int> param_type;
                pos++;
                while (u != nullptr){
                    if (first) first = false;
                    else if (words[pos++].id != 28) {Error = true; puts("in lack of ,"); return END;} //,
                    if (u->type == 0) {   //如果是传值
                        int re_id_param,re_type_param;
                        pos = Exp(pos, re_id_param, re_type_param, out);
                        if (Error) return END;
                        param_id.push(re_id_param);
                        param_type.push(re_type_param);
                    }
                    else if (u->type == 1){ //如果是传指针
                        if (words[pos].id != 1){Error = true; puts("There must be a var"); return END;}
                        symbol* param_i = sym_getIdent(words[pos].name,Space);
                        if (param_i == nullptr) {Error = true; puts("Can't find name(exp_func)"); return END;}
                        if (param_i->is_func) {Error = true; return END;}
                        if (param_i->is_const) {Error = true; return END;}
                        //找到合法的变量
                        param_id.push(param_i->id);
                        param_type.push(2);
                        pos++;
                    }
                    else {Error = true; return END;}
                    u = u->next;
                }
                if (words[pos].id != 15) {Error = true; return END;} //)
                //@load func
                int id = out.get_new_id();
                vector<var_node> params;
                while (!param_id.empty()){
                    int p_id = param_id.front(); param_id.pop();
                    int p_type = param_type.front(); param_type.pop();
                    params.emplace_back(var_node(p_type, p_id));
                }
                out.insert_call_i32(id, p->name, params);
                s_num.push(id);
                s_tag.push(1);
                is_last_num = true;
            }
        }
        else if (words[pos].id == 2){   //num
            s_num.push(words[pos].num);
            s_tag.push(0);
            is_last_num = true;
        }
        else if (words[pos].id == 14){   //(
            s_opt.push(14);
            s_opt_type.push(0);
            is_last_num = false;
            Par++;
        }
        else if (words[pos].id == 15){   //)
            if (Par - 1 < 0) break;
            Par--;
            while (s_opt.top() != 14){  //弹出符号直至(
                if (!exp_stack_pop(s_num,s_tag,s_opt,s_opt_type, out)) return false;
            }
            s_opt.pop();
            s_opt_type.pop();
            is_last_num = true;
        }
        else if (words[pos].id >= 20 && words[pos].id <= 24){  //opt
            if (!is_last_num){
                s_opt.push(words[pos].id);
                s_opt_type.push(1);
                is_last_num = false;
            }
            else {
                while (!s_opt.empty() && (s_opt_type.top() == 1 || (s_opt.top() != 14 && opt_cmp(s_opt.top(), words[pos].id)))) {
                    if (!exp_stack_pop(s_num,s_tag,s_opt,s_opt_type, out)) return false;
                }
                s_opt.push(words[pos].id);
                s_opt_type.push(0);
                is_last_num = false;
            }
        }
        else break;
        pos++;
    }
    while (!s_opt.empty()){
        if (!exp_stack_pop(s_num,s_tag,s_opt,s_opt_type, out)) {Error = true; return END;}
    }
    if (s_num.size() != 1) {Error = true; puts("Wrong exp!"); return END;}
    if (Par) {Error = true; printf("%d %d\n",pos,Par); puts("Par not matched!"); return END;}
    re_id = s_num.top();
    re_type = s_tag.top();
    return pos;
}

int ConstExp(int head, int& re_id, int& re_type, Output_region& out){
    int pos = head;
    pos = Exp(pos,re_id,re_type, out,true);
    return pos;
}

int const_declare(int pos, Output_region& out){  //@插入常量声明
    int id = out.get_new_id();
    if (!sym_insert(words[pos].name, Space, id,true)) Error = true;
    out.insert_alloc(id);
    return id;
}

void const_var_init(int id, const int& re_id, const int& re_type, Output_region& out){  //@插入常量初始化
    out.insert_store(var_node(re_type, re_id), var_node(2, id));
}

int var_declare(int pos, Output_region& out){  //@插入变量声明
    int id = out.get_new_id();
    if (!sym_insert(words[pos].name, Space, id,false)) Error = true;
    out.insert_alloc(id);
    return id;
}

void var_modify(int id, const int& re_id, const int& re_type, Output_region& out){    //@插入变量修改
    out.insert_store(var_node(re_type, re_id), var_node(2, id));
}

int ConstDef(int head, Output_region& out){          //常量声明
    int pos = head;
    if (words[pos].id == 1){   //Ident
        int t_pos = pos++;
        if (words[pos].id == 12){ //=
            int re_id,re_type;
            pos = ConstExp(pos + 1,re_id,re_type, out); //@
            if (Error) {puts("var name already exists."); return END;}
            int ident_id = const_declare(t_pos, out);    //@
            if (Error) return END;
            const_var_init(ident_id,re_id,re_type, out);   //@
        }
        else {Error = true; return END;}
    }
    else {Error = true; return END;}
    return pos;
}

int VarDef(int head, Output_region& out){          //变量声明
    int pos = head;
    if (words[pos].id == 1){   //Ident
        int t_pos = pos;
        pos++;
        if (words[pos].id == 12){ //=
            int re_id;
            int re_type;
            pos = Exp(pos + 1, re_id, re_type, out); //@
            if (Error) return END;
            int ident_id = var_declare(t_pos, out);    //@
            if (Error) {puts("var name already exists."); return END;}
            var_modify(ident_id,re_id,re_type, out);   //@
        }
        else {
            var_declare(t_pos, out);    //@
        }
    }
    else {Error = true; printf("VarDef no Ident"); return END;}
    return pos;
}

int BType(int head, Output_region& out){             //数据类型
    int pos = head;
    if (words[pos].id != 9) {Error = true; return END;}  //int
    return pos + 1;
}

int VarDecl(int head, Output_region& out){      //变量声明
    int pos = head;
    pos = BType(pos, out);
    if (Error) return END;
    pos = VarDef(pos, out);
    if (Error) return END;
    while (words[pos].id == 28){ //,
        pos = VarDef(pos + 1, out);
        if (Error) return END;
    }
    if (words[pos].id != 13) {Error = true; return END;} //;
    pos++;
    return pos;
}

int ConstDecl(int head, Output_region& out){
    int pos = head;
    if (words[pos].id == 11){ //const
        pos++;
        pos = BType(pos, out);
        if (Error) return END;
        pos = ConstDef(pos, out);
        if (Error) return END;
        while (words[pos].id == 28){ //,
            pos = ConstDef(pos + 1, out);
            if (Error) return END;
        }
        if (words[pos].id != 13) {Error = true; return END;} //;
        pos++;
    }
    else {Error = true; return END;}
    return pos;
}

int Decl(int head, Output_region& out){
    int pos = head;
    if (words[pos].id == 11) pos = ConstDecl(pos, out); //const
    else if (words[pos].id == 9) pos = VarDecl(pos, out);  //int
    else {Error = true; return END;}
    return pos;
}

void return_value(const int& re_id, const int& re_type, Output_region& out){   //@return
    out.insert_ret(var_node(re_type, re_id));
}

int Voidfun(int head, Output_region& out){
    int pos = head;
    if (words[pos].id != 1) {Error = true; return END;}
    symbol* p = sym_getIdent(words[pos].name,Space);
    if (p == nullptr) {Error = true; puts("void fun Ident not found"); return END;}
    if (words[++pos].id != 14) {Error = true; return END;} //(
    func_params *u = p->params;
    bool first = true;
    queue<int> param_id,param_type;
    pos++;
    while (u != nullptr){
        if (first) first = false;
        else if (words[pos++].id != 28) {Error = true; return END;} //,
        if (u->type == 0) {   //如果是传值
            int re_id_param,re_type_param;
            pos = Exp(pos, re_id_param, re_type_param, out);
            if (Error) return END;
            param_id.push(re_id_param);
            param_type.push(re_type_param);
        }
        else if (u->type == 1){ //如果是传指针
            if (words[pos].id != 1){Error = true; return END;}
            symbol* param_i = sym_getIdent(words[pos].name,Space);
            if (param_i == nullptr) {Error = true; return END;}
            if (param_i->is_func) {Error = true; return END;}
            if (param_i->is_const) {Error = true; return END;}
            //找到合法的变量
            param_id.push(param_i->id);
            param_type.push(2);
        }
        else {Error = true; return END;}
        u = u->next;
    }
    if (words[pos++].id != 15) {Error = true; return END;} //)
    //@load func
    //printf("\tcall void @%s(", p->name);
    vector<var_node> params;
    while (!param_id.empty()){
        int p_id = param_id.front(); param_id.pop();
        int p_type = param_type.front(); param_type.pop();
        params.emplace_back(var_node(p_type, p_id));
    }
    out.insert_call_void(p->name, params);
    return pos;
}

int Stmt(int head, Output_region& out){
    int pos = head;
    if (words[pos].id == 8){ //return
        int re_id,re_type;
        pos = Exp(pos + 1,re_id,re_type, out);
        if (Error) return END;
        if (words[pos].id != 13) {Error = true; puts("Error at Stmt 1"); return END;}
        return_value(re_id,re_type, out); //@return
        pos++;
    }
    //赋值
    else if (words[pos].id == 1 && words[pos + 1].id == 12){
        symbol *p = sym_getIdent(words[pos].name,Space);
        if (p == nullptr) {Error = true; puts("Error at Stmt 2"); return END;}
        if (p->is_const) {Error = true; puts("Error at Stmt 2.5:modify const"); return END;}
        int ident_id = p->id;
        //printf("%d",words[pos+2].id);
        int re_id,re_type;
        pos = Exp(pos + 2,re_id,re_type, out); //@
        if (Error) return END;
        //printf("%lld\n",words[pos].id);
        var_modify(ident_id,re_id,re_type, out);   //@
        if (words[pos].id != 13) {Error = true; puts("Error at Stmt 3"); return END;} //;
        pos++;
    }
    //;
    else if (words[pos].id == 13){
        pos++;
    }
    //Exp
    else {
        if (words[pos].id == 1){
            symbol *p = sym_getIdent(words[pos].name,Space);
            if (p == nullptr) {Error = true; puts("stmt Ident not found"); return END;}
            if (p->is_func && p->re_type == 0){
                pos = Voidfun(pos, out);
                if (Error) return END;
                if (words[pos].id != 13) {Error = true; puts("Error at Stmt 4"); return END;}
                pos++;
                return pos;
            }
        }
        int re_id;
        int re_type;
        pos = Exp(pos,re_id,re_type, out);
        if (Error) return END;
        if (words[pos].id != 13) {Error = true; puts("Error at Stmt 4"); return END;}
        pos++;
    }
    return pos;
}

int BlockItem(int head, Output_region& out){
    int pos = head;
    while (words[pos].id != 17 && !Error) {   //遇到}退出
        if (words[pos].id == 11 || words[pos].id == 9) {  //const ||  int
            pos = Decl(pos, out);
            if (Error) return END;
        } else {
            pos = Stmt(pos, out);
            if (Error) return END;
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
    Output_region out;
    pos = BlockItem(pos, out);
    if (Error) return END;
    out.output(0);
    layer_cnt--;
    if (words[pos].id == 17){   //}
        printf("}\n");
    }
    else {Error = true; puts("Error at Block 2"); return END;}
    pos++;
    return pos;
}

int FuncFParams(int head){    //参数列表
    int pos = head;

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
    sym_insert("getint",0,0,false,true,1);
    printf("declare i32 @getint()\n");
    sym_insert("getch",0,0,false,true,1);
    printf("declare i32 @getch()\n");
    auto *p = new func_params();
    sym_insert("putint",0,0,false,true,0,p);
    printf("declare void @putint(i32)\n");
    sym_insert("putch",0,0,false,true,0,p);
    printf("declare void @putch(i32)\n");
    int pos = head;
    while (pos <= words_len){
        if (words[pos].id == 11) ;//pos = Decl(pos);     //const
        else if (words[pos].id == 10) pos = FuncDef(pos);   //void
        else if (words[pos].id == 9 && words[pos + 1].id == 1){   //int Ident
            if (words[pos + 2].id == 14) pos = FuncDef(pos);            //(
            //else pos = Decl(pos);
        }
        else if (words[pos].id == 100) return pos;
        else {Error = true; return END;}
    }
    return pos;
}

int main(){
    //freopen("in.txt","r",stdin);
    //freopen("out.txt","w",stdout);
    //printf("declare void @putch(i32)\ndefine i32 @main(){\n");
    get_sym();
    //printf("\tret i32 0\n}\n");
    END = words_len;
    CompUnit(1);
    if (Error) return 1;
    return 0;
}
