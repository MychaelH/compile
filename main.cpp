#include<iostream>
#include<cstdio>
#include<stack>
#include<vector>
#include "getword.h"
#include "symbol_table.h"
#include "Output_region.h"
using namespace std;

int END;

bool Error;

int now_func_type;

int now_func_name_pos,is_call_self;


int opt_cmp_chart[][11]={
        {1,1,0,0,0,1,1,1,1,1,1},
        {1,1,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1},
        {0,0,0,0,0,1,1,1,1,1,1},
        {0,0,0,0,0,1,1,1,1,1,1},
        {0,0,0,0,0,1,1,1,1,1,1},
        {0,0,0,0,0,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,0,0,1,1},
        {0,0,0,0,0,0,0,0,0,1,1},
};

bool opt_cmper(int opt_a, int opt_b){
    int id_a = 0,id_b = 0;
    if (opt_a >= 20 && opt_a <= 26 || opt_a == 29) id_a = opt_a - 20;
    else if (opt_a >= 32 && opt_a <= 33) id_a = opt_a - 25;
    else if (opt_a == 34) id_a = 10;
    if (opt_b >= 20 && opt_b <= 26 || opt_b == 29) id_b = opt_b - 20;
    else if (opt_b >= 32 && opt_b <= 33) id_b = opt_b - 25;
    else if (opt_b == 34) id_b = 10;
    return opt_cmp_chart[id_a][id_b];
}

bool Const_exp_stack_pop(stack<int>& s_num, stack<int>& s_opt, stack<int>& s_opt_type){  //弹出栈顶运算符号并运算
    if (s_opt_type.top()){  //如果是单目运算符，弹出一个
        if (s_num.empty()) {Error = true;printf("Err at exp_stack_pop");return false;}
        int a = s_num.top();
        s_num.pop();
        switch(s_opt.top()) {
            case 27:a = !a; break;// !
            case 20:a = +a; break;// +
            case 21:a = -a; break;// -
            default:Error = true;printf("Err at exp_stack_pop");return false;
        }
        s_opt.pop(); s_opt_type.pop();
        s_num.push(a);
        return true;
    }
    int a, b;
    if (s_num.empty()) {Error = true;printf("Err at exp_stack_pop");return false;}
    b = s_num.top(); s_num.pop();
    if (s_num.empty()) {Error = true;printf("Err at exp_stack_pop");return false;}
    a = s_num.top(); s_num.pop();
    switch(s_opt.top()) {
        case 20:a += b; break;   // +
        case 21:a -= b; break;   // -
        case 22:a *= b; break;   // *
        case 23:a /= b; break;  // /
        case 24:a %= b; break;  // %
        case 25:a = a > b; break;  // >
        case 26:a = a < b; break;  // <
        case 32:a = a >= b; break;  // >=
        case 33:a = a <= b; break;  // <=
        case 29:a = a == b; break;  // ==
        case 34:a = a != b; break;  // !=
        default:Error = true;printf("Err at exp_stack_pop");return false;
    }
    s_num.push(a);
    s_opt.pop(); s_opt_type.pop();
    return true;
}

int ConstExp(int head, int& re_num){        //常量表达式求值
    int pos = head, Par = 0, i = 0,is_last_num = false;
    stack<int> s_num, s_opt, s_opt_type;
    while (true){
        i++;
        if (words[pos].id == 1){        //Ident
            symbol* p = sym_getIdent(words[pos].name, Space);
            if (p == nullptr) {Error = true; puts("Global Exp Ident not found"); return END;}
            if (p->is_func) {Error = true; return END;}
            if (!p->is_const) {Error = true; puts("global exp should be const"); return END;}
            if (p->dimen.dimen == 0) {
                s_num.push(p->id);
            }
            else if (p->dimen.dimen == 1){
                if (words[++pos].id != 18) {Error = true; puts("Exp [ not found"); return END;}
                int a_num;
                pos = ConstExp(pos + 1, a_num);
                if (words[pos].id != 19) {Error = true; puts("Exp ] not found"); return END;}
                s_num.push(p->nums[a_num]);
            }
            else if (p->dimen.dimen == 2){
                if (words[++pos].id != 18) {Error = true; puts("Exp [ not found"); return END;}
                int a_num;
                pos = ConstExp(pos + 1, a_num);
                if (words[pos].id != 19) {Error = true; puts("Exp ] not found"); return END;}
                if (words[++pos].id != 18) {Error = true; puts("Exp [ not found"); return END;}
                int b_num;
                pos = ConstExp(pos + 1, b_num);
                if (words[pos].id != 19) {Error = true; puts("Exp ] not found"); return END;}
                s_num.push(p->nums[a_num * p->dimen.len2 + b_num]);
            }
            else {Error = true; return END;}
            is_last_num = true;
        }
        else if (words[pos].id == 2){   //num
            s_num.push(words[pos].num);
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
                if (!Const_exp_stack_pop(s_num,s_opt,s_opt_type)) return false;
            }
            s_opt.pop();
            s_opt_type.pop();
            is_last_num = true;
        }
        else if (words[pos].id >= 20 && words[pos].id <= 24 || words[pos].id >= 25 && words[pos].id <= 26 || words[pos].id == 29 || words[pos].id >= 32 && words[pos].id <= 34){  //opt
            if (!is_last_num){
                s_opt.push(words[pos].id);
                s_opt_type.push(1);
                is_last_num = false;
            }
            else {
                while (!s_opt.empty() && (s_opt_type.top() == 1 || (s_opt.top() != 14 &&
                        opt_cmper(s_opt.top(), words[pos].id)))) {
                    if (!Const_exp_stack_pop(s_num,s_opt,s_opt_type)) return false;
                }
                s_opt.push(words[pos].id);
                s_opt_type.push(0);
                is_last_num = false;
            }
        }
        else if (words[pos].id == 27){ // !取反
            s_opt.push(words[pos].id);
            s_opt_type.push(1);
            is_last_num = false;
        }
        else break;
        pos++;
    }
    while (!s_opt.empty()){
        if (!Const_exp_stack_pop(s_num, s_opt, s_opt_type)) {Error = true; return END;}
    }
    if (s_num.size() != 1) {Error = true; puts("Wrong Global_exp!"); return END;}
    if (Par) {Error = true; printf("%d %d\n",pos,Par); puts("Par not matched!"); return END;}
    re_num = s_num.top();
    return pos;
}

void Global_const_declare(int head, int num = 0){
    if (!sym_insert(words[head].name, 0, num, d0, true)) Error = true;
    //printf("@%s = global i32 %d\n", words[head].name, num);
}

void Global_var_declare(int head, int num = 0){
    if (!sym_insert(words[head].name, 0, num, d0)) Error = true;
    printf("@%s = global i32 %d\n", words[head].name, num);
}

int Global_Array_Def(int head, bool is_const = false){
    int pos = head;
    if (words[pos].id != 1) {Error = true; puts("lack array ident"); return END;}
    if (is_const) printf("@%s = constant ",words[pos].name);
    else printf("@%s = global",words[pos].name);
    // [
    if (words[++pos].id != 18) {Error = true; puts("lack array ["); return END;}
    int len1;
    pos = ConstExp(pos + 1,len1);
    if (Error) return END;
    if (words[pos++].id != 19) {Error = true; puts("lack array ]"); return END;}
    if (words[pos].id == 12){ // = 一维数组初始化
        int *nums = new int[len1];
        memset(nums, 0, sizeof(int) * len1);
        printf("[%d x i32] ", len1);
        // {
        if (words[++pos].id != 16){Error = true; puts("lack array {"); return END;}
        if (words[++pos].id == 17){ // 直接跟}   0初始化
            printf("zeroinitializer\n");
        }
        else {
            printf("[");
            for (int i = 0; i < len1; i++){
                if (words[pos].id == 17){  //}
                    while (i < len1){
                        printf(", i32 0");
                        i++;
                    }
                    break;
                }
                if (i){
                    // ,
                    if (words[pos++].id != 28){Error = true; puts("lack array ,"); return END;}
                }
                int r_num;
                pos = ConstExp(pos, r_num);
                nums[i] = r_num;
                if (i) printf(", ");
                printf("i32 %d",r_num);
            }
            if (words[pos++].id != 17) {Error = true; puts("lack array }"); return END;}
            printf("]\n");
        }
        if (!sym_insert(words[head].name, Space, 0, Dimen(len1), is_const)){Error = true; puts("ident exists."); return END;}
        symbol *p = sym_getIdent(words[head].name, Space);
        p->nums = nums;
    }
    else if (words[pos].id == 18){ // [ 二维数组
        int len2;
        pos = ConstExp(pos + 1, len2);
        if (Error) return END;
        if (words[pos++].id != 19) {Error = true; puts("lack array ]"); return END;}
        int *nums = new int[len1 * len2];
        memset(nums, 0, sizeof(int) * len1 * len2);
        printf("[%d x [%d x i32]] ", len1, len2);
        if (words[pos].id == 12) { // = 二维数组初始化
            // {
            if (words[++pos].id != 16) {Error = true;puts("lack array {");return END;}
            if (words[++pos].id == 17) { // 直接跟}   0初始化
                printf("zeroinitializer\n");
            }
            else {
                printf("[");
                for (int i = 0; i < len1; i++){
                    if (words[pos].id == 17) { // 遇到}结束
                        while (i < len1) {
                            printf(", [%d x i32] zeroinitializer",len2);
                            i++;
                        }
                        break;
                    }
                    if (i){
                        //,
                        if (words[pos++].id != 28){Error = true; puts("lack array ,"); return END;}
                    }
                    if (i) printf(", ");
                    printf("[%d x i32] ",len2);
                    if (words[pos++].id != 16) {Error = true;puts("lack array {249");return END;}
                    printf("[");
                    for (int j = 0; j < len2; j++){
                        if (words[pos].id == 17){  //}
                            while (j < len2){
                                printf(", i32 0");
                                j++;
                            }
                            break;
                        }
                        if (j){
                            // ,
                            if (words[pos++].id != 28){Error = true; puts("lack array ,"); return END;}
                        }
                        int r_num;
                        pos = ConstExp(pos, r_num);
                        nums[i * len2 + j] = r_num;
                        if (j) printf(", ");
                        printf("i32 %d",r_num);
                    }
                    printf("]");
                    if (words[pos++].id != 17) {Error = true;puts("lack array }");return END;}
                }
                printf("]\n");
            }
            if (words[pos++].id != 17) {Error = true;puts("lack array }");return END;}
        }
        else { //二维数组0初始化
            printf("zeroinitializer\n");
        }
        if (!sym_insert(words[head].name, Space, 0, Dimen(len1, len2), is_const)){Error = true; puts("ident exists."); return END;}
        symbol *p = sym_getIdent(words[head].name, Space);
        p->nums = nums;
    }
    else { //一维数组0初始化
        int *nums = new int[len1];
        memset(nums, 0, sizeof(int) * len1);
        printf("[%d x i32] zeroinitializer\n", len1);
        if (!sym_insert(words[head].name, Space, 0, Dimen(len1), is_const)){Error = true; puts("ident exists."); return END;}
        symbol *p = sym_getIdent(words[head].name, Space);
        p->nums = nums;
    }
    return pos;
}

int Global_ConstDef(int head){          //常量声明
    int pos = head;
    if (words[pos].id == 1){   //Ident
        int t_pos = pos++;
        if (words[pos].id == 12){ //=
            int re_num;
            pos = ConstExp(pos + 1, re_num); //@
            if (Error) return END;
            Global_const_declare(t_pos, re_num);    //@
            if (Error) {puts("var name already exists."); return END;}
        }
        else if (words[pos].id == 18){  // [
            pos = Global_Array_Def(pos - 1, true);
        }
        else {Error = true; return END;}
    }
    else {Error = true; return END;}
    return pos;
}

int Global_VarDef(int head){          //变量声明
    int pos = head;
    if (words[pos].id == 1){   //Ident
        int t_pos = pos;
        pos++;
        if (words[pos].id == 12){ //=
            int re_num;
            pos = ConstExp(pos + 1, re_num); //@
            if (Error) return END;
            Global_var_declare(t_pos, re_num);    //@
            if (Error) {puts("var name already exists."); return END;}
        }
        else if (words[pos].id == 18){  // [
            pos = Global_Array_Def(pos - 1);
        }
        else {
            Global_var_declare(t_pos);    //@
        }
    }
    else {Error = true; printf("VarDef no Ident"); return END;}
    return pos;
}

int Global_BType(int head){             //数据类型
    int pos = head;
    if (words[pos].id != 9) {Error = true; return END;}  //int
    return pos + 1;
}

int Global_VarDecl(int head){      //变量声明
    int pos = head;
    pos = Global_BType(pos);
    if (Error) return END;
    pos = Global_VarDef(pos);
    if (Error) return END;
    while (words[pos].id == 28){ //,
        pos = Global_VarDef(pos + 1);
        if (Error) return END;
    }
    if (words[pos].id != 13) {Error = true; return END;} //;
    pos++;
    return pos;
}

int Global_ConstDecl(int head){
    int pos = head;
    if (words[pos].id == 11){ //const
        pos++;
        pos = Global_BType(pos);
        if (Error) return END;
        pos = Global_ConstDef(pos);
        if (Error) return END;
        while (words[pos].id == 28){ //,
            pos = Global_ConstDef(pos + 1);
            if (Error) return END;
        }
        if (words[pos].id != 13) {Error = true; return END;} //;
        pos++;
    }
    else {Error = true; return END;}
    return pos;
}


int Global_Decl(int head){
    int pos = head;
    if (words[pos].id == 11) pos = Global_ConstDecl(pos); //const
    else if (words[pos].id == 9) pos = Global_VarDecl(pos);  //int
    else {Error = true; return END;}
    return pos;
}
bool put_top_num(stack<int>& s_num, stack<int>& s_tag, var_node& re){
    if (s_num.empty()) return false;
    re = var_node(s_tag.top(), s_num.top());
    return true;
}

bool exp_stack_pop(stack<int>& s_num, stack<int>& s_tag, stack<int>& s_opt, stack<int>& s_opt_type, Output_region*& out){  //弹出栈顶运算符号并运算
    int id = Output_region::get_new_id(),id2;
    if (s_opt_type.top()){  //如果是单目运算符，弹出一个
        var_node re;
        if (!put_top_num(s_num,s_tag, re)) {Error = true;printf("Err at exp_stack_pop");return false;}
        s_num.pop(); s_tag.pop();
        switch(s_opt.top()) {
            case 27:
                out->insert_icmp_eq(id, re, var_node(0, 0));
                id2 = id;
                id = Output_region::get_new_id();
                out->insert_zext(id, var_node(1, id2));
                break;
            case 20:out->insert_add(id, var_node(0, 0), re); break;
            case 21:out->insert_sub(id, var_node(0, 0), re);break;
            default:Error = true;printf("Err at exp_stack_pop");return false;
        }
        s_opt.pop(); s_opt_type.pop();
        s_num.push(id);
        s_tag.push(1);
        return true;
    }
    var_node re1, re2;
    if (!put_top_num(s_num,s_tag, re1)) {Error = true;printf("Err at exp_stack_pop");return false;}
    s_num.pop(); s_tag.pop();
    if (!put_top_num(s_num,s_tag, re2)) {Error = true;printf("Err at exp_stack_pop");return false;}
    s_num.pop(); s_tag.pop();
    switch(s_opt.top()) {
        case 20:out->insert_add(id, re2, re1);break;   // +
        case 21:out->insert_sub(id, re2, re1);break;   // -
        case 22:out->insert_mul(id, re2, re1);break;   // *
        case 23:out->insert_sdiv(id, re2, re1);break;  // /
        case 24:out->insert_srem(id, re2, re1);break;  // %
        case 25: // >
            //puts(">");
            out->insert_icmp_sgt(id, re2, re1);
            id2 = id;
            id = Output_region::get_new_id();
            out->insert_zext(id, var_node(1, id2));
            break;
        case 26: // <
            out->insert_icmp_slt(id, re2, re1);
            id2 = id;
            id = Output_region::get_new_id();
            out->insert_zext(id, var_node(1, id2));
            break;
        case 32: // >=
            out->insert_icmp_sge(id, re2, re1);
            id2 = id;
            id = Output_region::get_new_id();
            out->insert_zext(id, var_node(1, id2));
            break;
        case 33: // <=
            out->insert_icmp_sle(id, re2, re1);
            id2 = id;
            id = Output_region::get_new_id();
            out->insert_zext(id, var_node(1, id2));
            break;
        case 29: // ==
            out->insert_icmp_eq(id, re2, re1);
            id2 = id;
            id = Output_region::get_new_id();
            out->insert_zext(id, var_node(1, id2));
            break;
        case 34: // !=
            out->insert_icmp_ne(id, re2, re1);
            id2 = id;
            id = Output_region::get_new_id();
            out->insert_zext(id, var_node(1, id2));
            break;
        default:Error = true;printf("Err at exp_stack_pop");return false;
    }
    s_num.push(id); s_tag.push(1);
    s_opt.pop(); s_opt_type.pop();
    return true;
}

int Exp(int head, int& re_id, int& re_type, Output_region*& out){        //表达式求值
    //printf("%d\n",words[head].id);
    int pos = head, Par = 0, i = 0,is_last_num = false;
    stack<int> s_num, s_tag, s_opt, s_opt_type;
    while (true){
        i++;
        if (words[pos].id == 1){        //Ident
            symbol* p = sym_getIdent(words[pos].name,Space);
            if (p == nullptr) {Error = true; puts("Exp Ident not found"); return END;}
            if (p->dimen.dimen == 1){ //一维数组取值
                if (words[++pos].id != 18) {Error = true; puts("Exp [ not found"); return END;}
                int a_id, a_type;
                pos = Exp(pos + 1, a_id, a_type, out);
                if (words[pos].id != 19) {Error = true; puts("Exp ] not found"); return END;}
                int t_id = Output_region::get_new_id();
                int id = Output_region::get_new_id();
                if (p->dimen.len1) {
                    //printf("dimen.len1:  %d\n", p->dimen.len1);
                    if (p->space) out->insert_getele_1(t_id, var_node(0, p->dimen.len1), var_node(1, p->id), var_node(a_type, a_id));
                    else out->insert_getele_1(t_id, var_node(0, p->dimen.len1), var_node(p->name), var_node(a_type, a_id));
                    out->insert_load(id, var_node(1, t_id));
                    s_num.push(id);
                }
                else {
                    int c_id = Output_region::get_new_id();
                    out->insert_load(t_id, var_node(2, p->id));
                    out->insert_getele_ptr_1(id, var_node(1, t_id), var_node(a_type, a_id));
                    out->insert_load(c_id, var_node(1, id));
                    s_num.push(c_id);
                }
                s_tag.push(1);
                is_last_num = true;
            }
            else if (p->dimen.dimen == 2) { //二维数组取值
                if (words[++pos].id != 18) {Error = true; puts("Exp [ not found"); return END;}
                int a_id, a_type;
                pos = Exp(pos + 1, a_id, a_type, out);
                if (words[pos].id != 19) {Error = true; puts("Exp ] not found"); return END;}
                if (words[++pos].id != 18) {Error = true; puts("Exp [ not found"); return END;}
                int b_id, b_type;
                pos = Exp(pos + 1, b_id, b_type, out);
                if (words[pos].id != 19) {Error = true; puts("Exp ] not found"); return END;}
                int t_id = Output_region::get_new_id();
                int id = Output_region::get_new_id();
                if (p->dimen.len1) {
                    if (p->space) out->insert_getele_2(t_id, var_node(0, p->dimen.len1), var_node(0, p->dimen.len2), var_node(1, p->id), var_node(a_type, a_id), var_node(b_type, b_id));
                    else out->insert_getele_2(t_id, var_node(0, p->dimen.len1), var_node(0, p->dimen.len2), var_node(p->name), var_node(a_type, a_id), var_node(b_type, b_id));
                    out->insert_load(id, var_node(1, t_id));
                    s_num.push(id);
                }
                else {
                    int c_id = Output_region::get_new_id();
                    out->insert_load(t_id, var_node(5, p->id, p->dimen.len2));
                    out->insert_getele_ptr_2(id, var_node(0, p->dimen.len2), var_node(1, t_id), var_node(a_type, a_id), var_node(b_type, b_id));
                    out->insert_load(c_id, var_node(1, id));
                    s_num.push(c_id);
                }
                s_tag.push(1);
                is_last_num = true;
            }
            else if (p->is_const) {
                s_num.push(p->id);
                s_tag.push(0);
                is_last_num = true;
            }
            else if (!p->is_func) {  //变量
                int id = Output_region::get_new_id();
                if (p->space) out->insert_load(id, var_node(1, p->id));
                else out->insert_load(id, var_node(p->name));
                s_num.push(id);
                s_tag.push(1);
                is_last_num = true;
            }
            else {            //function
                //printf("%s: %d\n",p->name,p->re_type);
                if (!strcmp(p->name,words[now_func_name_pos].name)) is_call_self = true;
                if (p->re_type == 0) {Error = true;puts("Can't put a void function in an exp"); return END;}
                if (words[++pos].id != 14) {Error = true; return END;} //(
                func_params *u = p->params;
                bool first = true;
                vector<var_node> param_id;
                pos++;
                while (u != nullptr){
                    if (first) first = false;
                    else if (words[pos++].id != 28) {Error = true; return END;} //,
                    if (u->type == 0) {   //如果是传值
                        int re_id_param,re_type_param;
                        pos = Exp(pos, re_id_param, re_type_param, out);
                        if (Error) return END;
                        param_id.emplace_back(var_node(re_type_param, re_id_param));
                    }
                    else if (u->type == 1){ //如果是传一维数组
                        if (words[pos].id != 1){Error = true; return END;}
                        symbol* param_i = sym_getIdent(words[pos].name,Space);
                        if (param_i == nullptr) {Error = true; return END;}
                        if (param_i->is_func) {Error = true; return END;}
                        if (param_i->is_const) {Error = true; return END;}
                        //找到合法的变量
                        if (param_i->dimen.dimen == 1){
                            if (param_i->dimen.len1) {
                                if (param_i->space) {
                                    int id = Output_region::get_new_id();
                                    out->insert_getele_1(id, var_node(0, param_i->dimen.len1), var_node(1, param_i->id),var_node(0, 0));
                                    param_id.emplace_back(var_node(2, id));
                                } else {
                                    int id = Output_region::get_new_id();
                                    out->insert_getele_1(id, var_node(0, param_i->dimen.len1), var_node(param_i->name),var_node(0, 0));
                                    param_id.emplace_back(var_node(2, id));
                                }
                            }
                            else {
                                int id = Output_region::get_new_id();
                                out->insert_load(id, var_node(2, param_i->id));
                                param_id.emplace_back(var_node(2, id));
                            }
                        }
                        else if (param_i->dimen.dimen == 2){  //如果传二维数组到一维
                            if (words[++pos].id != 18) {Error = true; return END;} //[
                            int a_id,a_type;
                            pos = Exp(pos + 1, a_id, a_type, out);
                            if (words[pos].id != 19) {Error = true; return END;}
                            if (param_i->dimen.len1) {
                                if (param_i->space) {
                                    int id = Output_region::get_new_id();
                                    out->insert_getele_2(id, var_node(0, param_i->dimen.len1), var_node(0, param_i->dimen.len2), var_node(1, param_i->id), var_node(a_type, a_id), var_node(0, 0));
                                    param_id.emplace_back(var_node(2, id));
                                } else {
                                    int id = Output_region::get_new_id();
                                    out->insert_getele_2(id, var_node(0, param_i->dimen.len1), var_node(0, param_i->dimen.len2), var_node(param_i->name), var_node(a_type, a_id), var_node(0, 0));
                                    param_id.emplace_back(var_node(4, id));
                                }
                            }
                            else {
                                int id = Output_region::get_new_id();
                                out->insert_getele_ptr_2(id, var_node(0, param_i->dimen.len2), var_node(1, param_i->id), var_node(0, 0), var_node(re_type, re_id));
                                param_id.emplace_back(var_node(2, id, param_i->dimen.len2));
                            }
                        }
                        else {Error = true; return END;}
                        pos++;
                    }
                    else if (u->type == 2){ //如果传二维数组
                        if (words[pos].id != 1){Error = true; return END;}
                        symbol* param_i = sym_getIdent(words[pos].name,Space);
                        if (param_i == nullptr) {Error = true; return END;}
                        if (param_i->is_func) {Error = true; return END;}
                        if (param_i->is_const) {Error = true; return END;}
                        if (param_i->dimen.dimen != 2) {Error = true; return END;}
                        if (param_i->dimen.len1) {
                            if (param_i->space) {
                                int id = Output_region::get_new_id();
                                out->insert_getele_2_to_1(id, var_node(0, param_i->dimen.len1), var_node(0, param_i->dimen.len2), var_node(1, param_i->id), var_node(0, 0));
                                param_id.emplace_back(var_node(5, id, param_i->dimen.len2));
                            } else {
                                int id = Output_region::get_new_id();
                                out->insert_getele_2_to_1(id, var_node(0, param_i->dimen.len1), var_node(0, param_i->dimen.len2), var_node(param_i->name), var_node(0, 0));
                                param_id.emplace_back(var_node(6, id, param_i->dimen.len2));
                            }
                        }
                        else {
                            int id = Output_region::get_new_id();
                            out->insert_load(id, var_node(5, param_i->id, param_i->dimen.len2));
                            param_id.emplace_back(var_node(5, id, param_i->dimen.len2));
                        }
                        pos++;
                    }
                    else {Error = true; return END;}
                    u = u->next;
                }
                if (words[pos].id != 15) {Error = true; return END;} //)
                //@load func
                int id = Output_region::get_new_id();
                if (!p->isinline){
                    out->insert_call_i32(id, p->name, param_id);
                    s_num.push(id);
                }
                else {
                    out->insert_alloc(id);
                    out->insert_inline_call(id, p, opt_id_cnt, opt_id_cnt + 1,param_id);
                    int t_id = Output_region::get_new_id();
                    out->insert_load(t_id, var_node(1, id));
                    s_num.push(t_id);
                }

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
        else if (words[pos].id >= 20 && words[pos].id <= 24 || words[pos].id >= 25 && words[pos].id <= 26 || words[pos].id == 29 || words[pos].id >= 32 && words[pos].id <= 34){  //opt
            if (!is_last_num){
                s_opt.push(words[pos].id);
                s_opt_type.push(1);
                is_last_num = false;
            }
            else {
                while (!s_opt.empty() && (s_opt_type.top() == 1 || (s_opt.top() != 14 &&
                        opt_cmper(s_opt.top(), words[pos].id)))) {
                    if (!exp_stack_pop(s_num,s_tag,s_opt,s_opt_type, out)) return false;
                }
                s_opt.push(words[pos].id);
                s_opt_type.push(0);
                is_last_num = false;
            }
        }
        else if (words[pos].id == 27){ // !取反
            s_opt.push(words[pos].id);
            s_opt_type.push(1);
            is_last_num = false;
        }
        else break;
        pos++;
    }
    while (!s_opt.empty()){
        if (!exp_stack_pop(s_num,s_tag,s_opt,s_opt_type, out)) {Error = true; return END;}
    }
    //printf("size: %d\n",s_num.size());
    if (s_num.size() != 1) {Error = true; puts("Wrong exp!"); return END;}
    if (Par) {Error = true; printf("%d %d\n",pos,Par); puts("Par not matched!"); return END;}
    re_id = s_num.top();
    re_type = s_tag.top();
    return pos;
}

void const_declare(int pos, const int& re_num){  //@常量声明
    if (!sym_insert(words[pos].name, Space, re_num, d0, true)) Error = true;
}

int var_declare(int pos, Output_region*& out){  //@插入变量声明
    int id = Output_region::get_new_id();
    if (!sym_insert(words[pos].name, Space, id, d0, false)) Error = true;
    out->insert_alloc(id);
    return id;
}

void var_modify(int id, const int& re_id, const int& re_type, Output_region*& out){    //@插入变量修改
    out->insert_store(var_node(re_type, re_id), var_node(1, id));
}

void global_var_modify(const char* name, const int& re_id, const int& re_type, Output_region*& out){    //@插入变量修改
    out->insert_store(var_node(re_type, re_id), var_node(name));
}


//常量数组声明
int  DecConstArray(int head, Output_region*& out){
    int pos = head;
    if (words[pos++].id != 1) {Error = true; puts("Array no Ident"); return END;}
    if (words[pos++].id != 18){Error = true; puts("Array no Lbra"); return END;}
    int len1;
    pos = ConstExp(pos, len1);
    if (Error) return END;
    if (len1 <= 0) {Error = true; puts("Array size should be above 0"); return END;}
    if (words[pos++].id != 19){Error = true; puts("Array no Rbra"); return END;}
    if (words[pos].id == 18){ //[  二维数组
        int len2;
        pos = ConstExp(pos + 1, len2);
        if (Error) return END;
        if (len2 <= 0) {Error = true; puts("Array size should be above 0"); return END;}
        // ]
        if (words[pos++].id != 19){Error = true; puts("Array no Rbra"); return END;}
        if (words[pos].id == 12){ //= 二维初始化
            int *nums = new int[len1 * len2];  //存常量初始化值
            int id = Output_region::get_new_id();
            out->insert_alloca_2(id, var_node(0, len1), var_node(0, len2));
            int t_id = Output_region::get_new_id();
            out->insert_getele_2(t_id, var_node(0,len1), var_node(0, len2), var_node(1, id), var_node(0, 0), var_node(0, 0));
            out->insert_memset(var_node(1, t_id), var_node(0, 4 * len1 * len2));
            // {
            if (words[++pos].id != 16) {Error = true; puts("1d init error"); return END;}
            pos++;
            for (int i = 0; i < len1; i++){
                if (words[pos].id  == 17) break;  // }
                if (i){
                    // ,
                    if (words[pos++].id != 28) {Error = true; puts("2d init error"); return END;}
                }
                // {
                if (words[pos++].id != 16) {Error = true; puts("2d 1d init error"); return END;}
                for (int j = 0; j < len2; j++){
                    if (words[pos].id  == 17) break;  // }
                    if (j){
                        // ,
                        if (words[pos++].id != 28) {Error = true; puts("2d 2d init error"); return END;}
                    }
                    int re_num;
                    pos = ConstExp(pos,re_num);
                    nums[i * len2 + j] = re_num;
                    if (Error) return END;
                    t_id = Output_region::get_new_id();
                    out->insert_getele_2(t_id, var_node(0, len1), var_node(0, len2), var_node(1, id), var_node(0, i), var_node(0, j));
                    out->insert_store(var_node(0, re_num), var_node(1, t_id));
                }
                // }
                if (words[pos++].id != 17) {Error = true; puts("2d init error"); return END;}
            }
            // }
            if (words[pos++].id != 17) {Error = true; puts("2d init error"); return END;}
            if (!sym_insert(words[head].name, Space, id, Dimen(len1, len2), true)) {Error = true; puts("Ident same name!"); return END;}
            symbol *p = sym_getIdent(words[head].name, Space);
            p->nums = nums;
        }
        else {Error = true; puts("Const array should be initialized"); return END;}
    }
    else if (words[pos].id == 12){  //=  一维初始化
        int *nums = new int[len1];
        int id = Output_region::get_new_id();
        out->insert_alloca_1(id, var_node(0, len1));
        int t_id = Output_region::get_new_id();
        out->insert_getele_1(t_id, var_node(0,len1), var_node(1, id), var_node(0, 0));
        out->insert_memset(var_node(1, t_id), var_node(0, 4 * len1));
        // {
        if (words[++pos].id != 16) {Error = true; puts("1d init error"); return END;}
        int i = 0;
        pos++;
        // }
        while (words[pos].id != 17){
            if (i){
                // ,
                if (words[pos++].id != 28) {Error = true; puts("1d init error"); return END;}
            }
            int re_num;
            pos = ConstExp(pos,re_num);
            nums[i] = re_num;
            if (Error) return END;
            t_id = Output_region::get_new_id();
            out->insert_getele_1(t_id, var_node(0,len1), var_node(1, id), var_node(0, i));
            out->insert_store(var_node(0, re_num), var_node(1, t_id));
            i++;
            if (i == len1) break;
        }
        // }
        if (words[pos++].id != 17) {Error = true; puts("1d init error"); return END;}
        if (Error) return END;
        if (!sym_insert(words[head].name, Space, id, Dimen(len1))) {Error = true; puts("Ident same name!"); return END;}
        symbol *p = sym_getIdent(words[head].name, Space);
        p->nums = nums;
    }
    else {Error = true; puts("Const array should be initialized"); return END;}
    return pos;
}

//数组声明
int  DecArray(int head, Output_region*& out){
    int pos = head;
    if (words[pos++].id != 1) {Error = true; puts("Array no Ident"); return END;}
    if (words[pos++].id != 18){Error = true; puts("Array no Lbra"); return END;}
    int len1;
    pos = ConstExp(pos, len1);
    if (Error) return END;
    if (len1 <= 0) {Error = true; puts("Array size should be above 0"); return END;}
    if (words[pos++].id != 19){Error = true; puts("Array no Rbra"); return END;}
    if (words[pos].id == 18){ //[  二维数组
        int len2;
        pos = ConstExp(pos + 1, len2);
        if (Error) return END;
        if (len2 <= 0) {Error = true; puts("Array size should be above 0"); return END;}
        if (words[pos++].id != 19){Error = true; puts("Array no Rbra"); return END;}
        if (words[pos].id == 12){ //= 二维初始化
            int id = Output_region::get_new_id();
            out->insert_alloca_2(id, var_node(0, len1), var_node(0, len2));
            int t_id = Output_region::get_new_id();
            out->insert_getele_2(t_id, var_node(0,len1), var_node(0, len2), var_node(1, id), var_node(0, 0), var_node(0, 0));
            out->insert_memset(var_node(1, t_id), var_node(0, 4 * len1 * len2));
            // {
            if (words[++pos].id != 16) {Error = true; puts("1d init error"); return END;}
            pos++;
            for (int i = 0; i < len1; i++){
                if (words[pos].id  == 17) break;  // }
                if (i){
                    // ,
                    if (words[pos++].id != 28) {Error = true; puts("2d init error"); return END;}
                }
                // {
                if (words[pos++].id != 16) {Error = true; puts("2d 1d init error"); return END;}
                for (int j = 0; j < len2; j++){
                    if (words[pos].id  == 17) break;  // }
                    if (j){
                        // ,
                        if (words[pos++].id != 28) {Error = true; puts("2d 2d init error"); return END;}
                    }
                    int re_id, re_type;
                    pos = Exp(pos,re_id,re_type,out);
                    if (Error) return END;
                    t_id = Output_region::get_new_id();
                    out->insert_getele_2(t_id, var_node(0, len1), var_node(0, len2), var_node(1, id), var_node(0, i), var_node(0, j));
                    out->insert_store(var_node(re_type, re_id), var_node(1, t_id));
                }
                // }
                if (words[pos++].id != 17) {Error = true; puts("2d init error"); return END;}
            }
            // }
            if (words[pos++].id != 17) {Error = true; puts("2d init error"); return END;}
            if (!sym_insert(words[head].name, Space, id, Dimen(len1, len2))) {Error = true; puts("Ident same name!"); return END;}
        }
        else {  //二维不初始化
            int id = Output_region::get_new_id();
            out->insert_alloca_2(id, var_node(0, len1), var_node(0, len2));
            if (!sym_insert(words[head].name, Space, id, Dimen(len1, len2))) {Error = true; puts("Ident same name!"); return END;}
        }
    }
    else if (words[pos].id == 12){  //=  一维初始化
        int id = Output_region::get_new_id();
        out->insert_alloca_1(id, var_node(0, len1));
        int t_id = Output_region::get_new_id();
        out->insert_getele_1(t_id, var_node(0,len1), var_node(1, id), var_node(0, 0));
        out->insert_memset(var_node(1, t_id), var_node(0, 4 * len1));
        // {
        if (words[++pos].id != 16) {Error = true; puts("1d init error"); return END;}
        int i = 0;
        pos++;
        // }
        while (words[pos].id != 17){
            if (i){
                // ,
                if (words[pos++].id != 28) {Error = true; puts("1d init error"); return END;}
            }
            int re_id, re_type;
            pos = Exp(pos,re_id,re_type,out);
            if (Error) return END;
            t_id = Output_region::get_new_id();
            out->insert_getele_1(t_id, var_node(0,len1), var_node(1, id), var_node(0, i));
            out->insert_store(var_node(re_type, re_id), var_node(1, t_id));
            i++;
            if (i == len1) break;
        }
        // }
        if (words[pos++].id != 17) {Error = true; puts("1d init error"); return END;}
        if (Error) return END;
        if (!sym_insert(words[head].name, Space, id, Dimen(len1))) {Error = true; puts("Ident same name!"); return END;}
    }
    else {  //一维不初始化
        int id = Output_region::get_new_id();
        out->insert_alloca_1(id, var_node(0, len1));
        if (!sym_insert(words[head].name, Space, id, Dimen(len1))) {Error = true; puts("Ident same name!"); return END;}
    }
    return pos;
}


int ConstDef(int head, Output_region*& out){          //常量声明
    int pos = head;
    if (words[pos].id == 1){   //Ident
        int t_pos = pos++;
        if (words[pos].id == 12){ //=
            int re_num;
            pos = ConstExp(pos + 1,re_num); //@
            if (Error) {puts("var name already exists."); return END;}
            const_declare(t_pos, re_num);    //@
            if (Error) return END;
        }
        else if (words[pos].id == 18){  // [
            pos = DecConstArray(pos - 1, out);
            if (Error) return END;
        }
        else {Error = true; return END;}
    }
    else {Error = true; return END;}
    return pos;
}

int VarDef(int head, Output_region*& out){          //变量声明
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
        else if (words[pos].id == 18){  //[  数组
            pos = DecArray(pos - 1, out);
            if (Error) return END;
        }
        else {
            var_declare(t_pos, out);    //@
        }
    }
    else {Error = true; printf("VarDef no Ident"); return END;}
    return pos;
}

int BType(int head, Output_region*& out){             //数据类型
    int pos = head;
    if (words[pos].id != 9) {Error = true; return END;}  //int
    return pos + 1;
}

int VarDecl(int head, Output_region*& out){      //变量声明
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

int ConstDecl(int head, Output_region*& out){
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

int Decl(int head, Output_region*& out){
    int pos = head;
    if (words[pos].id == 11) pos = ConstDecl(pos, out); //const
    else if (words[pos].id == 9) pos = VarDecl(pos, out);  //int
    else {Error = true; return END;}
    return pos;
}

void return_value(const int& re_id, const int& re_type, Output_region*& out){   //@return
    Output_region::get_new_id();
    out->insert_ret(var_node(re_type, re_id));
}

int Voidfun(int head, Output_region*& out){
    int pos = head;
    if (words[pos].id != 1) {Error = true; return END;}
    symbol* p = sym_getIdent(words[pos].name,Space);
    if (p == nullptr) {Error = true; puts("void fun Ident not found"); return END;}
    if (!p->is_func || p->re_type) {Error = true; puts("void fun not found"); return END;}
    if (!strcmp(p->name,words[now_func_name_pos].name)) is_call_self = true;
    if (words[++pos].id != 14) {Error = true; return END;} //(
    func_params *u = p->params;
    bool first = true;
    vector<var_node> param_id;
    pos++;
    while (u != nullptr){
        if (first) first = false;
        else if (words[pos++].id != 28) {Error = true; return END;} //,
        if (u->type == 0) {   //如果是传值
            int re_id_param,re_type_param;
            pos = Exp(pos, re_id_param, re_type_param, out);
            if (Error) return END;
            param_id.emplace_back(var_node(re_type_param, re_id_param));
        }
        else if (u->type == 1){ //如果是传一维数组
            if (words[pos].id != 1){Error = true; return END;}
            symbol* param_i = sym_getIdent(words[pos].name,Space);
            if (param_i == nullptr) {Error = true; return END;}
            if (param_i->is_func) {Error = true; return END;}
            if (param_i->is_const) {Error = true; return END;}
            //找到合法的变量
            if (param_i->dimen.dimen == 1){
                if (param_i->dimen.len1) {
                    if (param_i->space) {
                        int id = Output_region::get_new_id();
                        out->insert_getele_1(id, var_node(0, param_i->dimen.len1), var_node(1, param_i->id),var_node(0, 0));
                        param_id.emplace_back(var_node(2, id));
                    } else {
                        int id = Output_region::get_new_id();
                        out->insert_getele_1(id, var_node(0, param_i->dimen.len1), var_node(param_i->name),var_node(0, 0));
                        param_id.emplace_back(var_node(4, id));
                    }
                }
                else {
                    int id = Output_region::get_new_id();
                    out->insert_load(id, var_node(2, param_i->id));
                    param_id.emplace_back(var_node(2, id));
                }
            }
            else if (param_i->dimen.dimen == 2){  //二维传一维
                if (words[++pos].id != 18) {Error = true; return END;} //[
                int re_id,re_type;
                pos = Exp(pos + 1, re_id, re_type, out);
                if (words[pos].id != 19) {Error = true; return END;}
                if (param_i->dimen.len1) {
                    if (param_i->space) {
                        int id = Output_region::get_new_id();
                        out->insert_getele_2(id, var_node(0, param_i->dimen.len1), var_node(0, param_i->dimen.len2), var_node(1, param_i->id), var_node(re_type, re_id), var_node(0, 0));
                        param_id.emplace_back(var_node(2, id));
                    } else {
                        int id = Output_region::get_new_id();
                        out->insert_getele_2(id, var_node(0, param_i->dimen.len1), var_node(0, param_i->dimen.len2), var_node(param_i->name), var_node(re_type, re_id), var_node(0, 0));
                        param_id.emplace_back(var_node(4, id));
                    }
                }
                else {
                    int id = Output_region::get_new_id();
                    out->insert_getele_ptr_2(id, var_node(0, param_i->dimen.len2), var_node(1, param_i->id), var_node(0, 0), var_node(re_type, re_id));
                    param_id.emplace_back(var_node(2, id, param_i->dimen.len2));
                }
            }
            else {Error = true; return END;}
            pos++;
        }
        else if (u->type == 2){ //如果传二维数组
            if (words[pos].id != 1){Error = true; return END;}
            symbol* param_i = sym_getIdent(words[pos].name,Space);
            if (param_i == nullptr) {Error = true; return END;}
            if (param_i->is_func) {Error = true; return END;}
            if (param_i->is_const) {Error = true; return END;}
            if (param_i->dimen.dimen != 2) {Error = true; return END;}
            if (param_i->dimen.len1) {
                if (param_i->space) {
                    int id = Output_region::get_new_id();
                    out->insert_getele_2_to_1(id, var_node(0, param_i->dimen.len1), var_node(0, param_i->dimen.len2), var_node(1, param_i->id), var_node(0, 0));
                    param_id.emplace_back(var_node(5, id, param_i->dimen.len2));
                } else {
                    int id = Output_region::get_new_id();
                    out->insert_getele_2_to_1(id, var_node(0, param_i->dimen.len1), var_node(0, param_i->dimen.len2), var_node(param_i->name), var_node(0, 0));
                    param_id.emplace_back(var_node(6, id, param_i->dimen.len2));
                }
            }
            else {
                int id = Output_region::get_new_id();
                out->insert_load(id, var_node(5, param_i->id, param_i->dimen.len2));
                param_id.emplace_back(var_node(5, id, param_i->dimen.len2));
            }
            pos++;
        }
        else {Error = true; return END;}
        u = u->next;
    }
    if (words[pos++].id != 15) {Error = true; return END;} //)
    //@load func
    if (!p->isinline) out->insert_call_void(p->name, param_id);
    else {
        out->insert_inline_call(0, p, opt_id_cnt, opt_id_cnt + 1,param_id);
    }
    return pos;
}

int Block(int head, Output_region*& out, bool add_space);
int Stmt(int head, Output_region*& out);

Output_region* do_and_exp(stack<Output_region*>& s_block){
    Output_region* B = s_block.top(); s_block.pop();
    Output_region* A = s_block.top(); s_block.pop();
    auto *re = new Output_region();
    A->p_yes = B;
    A->p_no = nullptr;
    B->p_yes = nullptr;
    B->p_no = nullptr;
    re->insert_block(A);
    re->insert_block(B);
    re->label = A->label;
    return re;
}
Output_region* do_or_exp(stack<Output_region*>& s_block){
    Output_region* B = s_block.top(); s_block.pop();
    Output_region* A = s_block.top(); s_block.pop();
    auto *re = new Output_region();
    A->p_yes = nullptr;
    A->p_no = B;
    B->p_yes = nullptr;
    B->p_no = nullptr;
    re->insert_block(A);
    re->insert_block(B);
    re->label = A->label;
    return re;
}

int Cond(int head, Output_region*& out){
    int pos = head, turn = 0;
    bool first = true;
    stack<Output_region*> s_block;
    stack<int> s_opt;
    while (true){
        if (!turn){  //创建新的表达式模块
            auto *p = new Output_region();
            p->set_is_jump();
            if (first) first = false;
            else p->set_label();
            int re_id,re_type;
            pos = Exp(pos, re_id, re_type, p);
            if (Error) return END;
            int id = Output_region::get_new_id();
            p->insert_icmp_eq(id, var_node(re_type, re_id), var_node(0, 0));
            s_block.push(p);
        }
        else if (words[pos].id == 30){   // And
            while (!s_opt.empty()){
                if (s_opt.top() == 30){  //And归约
                    Output_region *p = do_and_exp(s_block);
                    s_block.push(p);
                    s_opt.pop();
                }
                else break;
            }
            s_opt.push(30);
            pos++;
        }
        else if (words[pos].id == 31){   //Or
            while (!s_opt.empty()){
                if (s_opt.top() == 30){  //And归约
                    Output_region *p = do_and_exp(s_block);
                    s_block.push(p);
                }
                else{                    //Or归约
                    Output_region *p = do_or_exp(s_block);
                    s_block.push(p);
                }
                s_opt.pop();
            }
            s_opt.push(31);
            pos++;
        }
        else break;
        turn = !turn;
    }
    //剩下符号全部出栈
    while (!s_opt.empty()){
        if (s_opt.top() == 30){  //And归约
            Output_region *p = do_and_exp(s_block);
            s_block.push(p);
        }
        else{                    //Or归约
            Output_region *p = do_or_exp(s_block);
            s_block.push(p);
        }
        s_opt.pop();
    }
    if (s_block.size() != 1) {Error = true; puts("Error at Cond"); return END;}
    Output_region *p = s_block.top();
    out = p;
    //out->insert_block(p);
    return pos;
}

int If(int head, Output_region*& out){
    int pos = head;
    if (words[pos++].id != 3) {Error = true; puts("Error at If name"); return END;}
    if (words[pos++].id != 14) {Error = true; puts("Error at If LPar"); return END;}
    auto *cond_out = new Output_region();
    pos = Cond(pos, cond_out); //计算cond表达式，返回输出模块cond_out
    out->insert_block(cond_out);
    if (words[pos++].id != 15) {Error = true; puts("Error at If RPar"); return END;}
    //if stmt
    Output_region *t;
    auto *stmt_out1 = new Output_region();
    stmt_out1->set_is_jump();
    stmt_out1->set_label();
    cond_out->p_yes = stmt_out1;
    t = stmt_out1;
    int t_re = is_return[Space];
    pos = Stmt(pos, stmt_out1);
    is_return[Space] = t_re;
    stmt_out1 = t;
    out->insert_block(stmt_out1);
    //else stmt
    auto *stmt_out2 = new Output_region();
    stmt_out2->set_is_jump();
    stmt_out2->set_label();
    cond_out->p_no = stmt_out2;
    if (words[pos].id == 4){ //else
        t = stmt_out2;
        t_re = is_return[Space];
        pos = Stmt(pos + 1, stmt_out2);
        is_return[Space] = t_re;
        stmt_out2 = t;
    }
    out->insert_block(stmt_out2);
    //other block
    auto *other_out = new Output_region();
    other_out->set_label();
    stmt_out1->p_jump = other_out;
    stmt_out2->p_jump = other_out;
    out->insert_block(other_out);
    out = other_out;
    //printf("pos id:%d\n",words[pos].id);
    return pos;
}

int While(int head, Output_region*& out){
    int pos = head;
    if (words[pos++].id != 5) {Error = true; puts("Error at While name"); return END;}
    if (words[pos++].id != 14) {Error = true; puts("Error at While LPar"); return END;}
    //while起始标记
    st_while[++while_pos] = ++while_id_cnt;
    while_head[st_while[while_pos]] = Output_region::get_new_id();
    out->insert_br(while_head[st_while[while_pos]]);
    out->insert_label(while_head[st_while[while_pos]]);

    //计算逻辑式
    auto *cond_out = new Output_region();
    pos = Cond(pos, cond_out); //计算cond表达式，返回输出模块cond_out
    out->insert_block(cond_out);
    if (words[pos++].id != 15) {Error = true; puts("Error at While RPar"); return END;}
    //while stmt
    Output_region *t;
    auto *stmt_out = new Output_region();
    stmt_out->set_label();
    cond_out->p_yes = stmt_out;
    t = stmt_out;
    int t_re = is_return[Space];
    pos = Stmt(pos, stmt_out);
    is_return[Space] = t_re;
    stmt_out = t;
    stmt_out->insert_br(while_head[st_while[while_pos]]);
    out->insert_block(stmt_out);

    //后继模块
    auto *other_out = new Output_region();
    other_out->set_label();
    while_out[st_while[while_pos]] = other_out->label;
    cond_out->p_no = other_out;
    out->insert_block(other_out);
    out = other_out;
    while_pos--;  //退出当前while层
    return pos;
}

int Break(int head, Output_region*& out){
    int pos = head;
    if (words[pos++].id != 6) {Error = true; puts("Break Ident not found"); return END;}
    if (while_pos <= 0) {Error = true; puts("Break should be in while"); return END;}
    out->insert_br_while_out(st_while[while_pos]);
    return pos;
}

int Continue(int head, Output_region*& out){
    int pos = head;
    if (words[pos++].id != 7) {Error = true; puts("While Ident not found"); return END;}
    if (while_pos <= 0) {Error = true; puts("While should be in while"); return END;}
    out->insert_br_while_head(st_while[while_pos]);
    return pos;
}

int Stmt(int head, Output_region*& out){
    int pos = head;
    if (words[pos].id == 3){   //if
        pos = If(pos, out);
    }
    else if (words[pos].id == 5){ //while
        pos = While(pos, out);
    }
    else if (words[pos].id == 6){  //break
        pos = Break(pos, out);
    }
    else if (words[pos].id == 7){  //continue
        pos = Continue(pos, out);
    }
    //Block
    else if (words[pos].id == 16){  //{
        pos = Block(pos + 1, out, true);
        if (words[pos++].id != 17) {Error = true; puts("Error at stmt1"); return END;}
    }
    else if (words[pos].id == 8){ //return
        if (now_func_type) {
            int re_id, re_type;
            pos = Exp(pos + 1, re_id, re_type, out);
            if (Error) return END;
            if (words[pos].id != 13) {Error = true; puts("Error at Stmt 1"); return END;}
            return_value(re_id,re_type, out); //@return
            pos++;
        }
        else {
            if (words[++pos].id != 13) {Error = true; puts("Error at Stmt 1"); return END;}
            Output_region::get_new_id();
            out->insert_ret_void();
            pos++;
        }
        is_return[Space] = true;
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
        if (p->space) var_modify(ident_id,re_id,re_type, out);   //@
        else global_var_modify(p->name, re_id,re_type, out);
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
            else if (p->dimen.dimen == 1){ //一维数组
                int t_pos = pos;
                if (words[++t_pos].id != 18) {Error = true; puts("[ not found"); return END;}
                int a_id,a_type,tmp_id_cnt = opt_id_cnt;
                auto* tmp = new Output_region();
                t_pos = Exp(t_pos + 1,a_id,a_type,tmp);
                opt_id_cnt = tmp_id_cnt;
                if (Error) return END;
                if (words[t_pos++].id != 19) {Error = true; puts("] not found"); return END;}
                if (words[t_pos].id == 12){ //一维数组赋值语句
                    int re_id,re_type;
                    pos = Exp(pos + 2, a_id, a_type, out);
                    if (Error) return END;
                    pos = Exp(pos + 2, re_id, re_type, out);
                    if (Error) return END;
                    if (words[pos++].id != 13) {Error = true; puts("; not found"); return END;}
                    int id = Output_region::get_new_id();
                    if (p->dimen.len1) {
                        if (p->space) out->insert_getele_1(id, var_node(0, p->dimen.len1), var_node(1, p->id), var_node(a_type, a_id));
                        else out->insert_getele_1(id, var_node(0, p->dimen.len1), var_node(p->name), var_node(a_type, a_id));
                        out->insert_store(var_node(re_type, re_id), var_node(1, id));
                    }
                    else {
                        int t_id = Output_region::get_new_id();
                        out->insert_load(id, var_node(2, p->id));
                        out->insert_getele_ptr_1(t_id, var_node(1, id), var_node(a_type, a_id));
                        out->insert_store(var_node(re_type, re_id), var_node(1, t_id));
                    }
                    return pos;
                }
            }
            else if (p->dimen.dimen == 2){  //二维数组赋值
                int t_pos = pos;
                if (words[++t_pos].id != 18) {Error = true; puts("[ not found"); return END;}
                int a_id,a_type,tmp_id_cnt = opt_id_cnt;
                auto* tmp = new Output_region();
                t_pos = Exp(t_pos + 1,a_id,a_type,tmp);
                if (Error) return END;
                if (words[t_pos++].id != 19) {Error = true; puts("] not found"); return END;}
                if (words[t_pos].id != 18) {Error = true; puts("[ not found"); return END;}
                int b_id,b_type;
                t_pos = Exp(t_pos + 1,b_id,b_type,tmp);
                opt_id_cnt = tmp_id_cnt;
                if (Error) return END;
                if (words[t_pos++].id != 19) {Error = true; puts("] not found"); return END;}
                if (words[t_pos].id == 12){ //二维数组赋值语句
                    int re_id,re_type;
                    pos = Exp(pos + 2, a_id, a_type, out);
                    if (Error) return END;
                    pos = Exp(pos + 2, b_id, b_type, out);
                    if (Error) return END;
                    pos = Exp(pos + 2, re_id, re_type, out);
                    if (Error) return END;
                    if (words[pos++].id != 13) {Error = true; puts("; not found"); return END;}
                    int id = Output_region::get_new_id();
                    if (p->dimen.len1){
                        if (p->space) out->insert_getele_2(id, var_node(0, p->dimen.len1), var_node(0, p->dimen.len2), var_node(1, p->id), var_node(a_type, a_id), var_node(b_type, b_id));
                        else out->insert_getele_2(id, var_node(0, p->dimen.len1), var_node(0, p->dimen.len2), var_node(p->name), var_node(a_type, a_id), var_node(b_type, b_id));
                        out->insert_store(var_node(re_type, re_id), var_node(1, id));
                    }
                    else {
                        int t_id = Output_region::get_new_id();
                        out->insert_load(id, var_node(5, p->id));
                        out->insert_getele_ptr_2(t_id, var_node(0, p->dimen.len2), var_node(1, id), var_node(a_type, a_id), var_node(b_type, b_id));
                        out->insert_store(var_node(re_type, re_id), var_node(1, t_id));
                    }
                    return pos;
                }
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

int BlockItem(int head, Output_region*& out){
    int pos = head;
    while (words[pos].id != 17 && !Error) {   //遇到}退出
        if (words[pos].id == 11 || words[pos].id == 9) {  //const ||  int
            if (is_return[Space]) {
                auto *tmp = new Output_region();
                int t_opt_id = opt_id_cnt;
                pos = Decl(pos, tmp);
                opt_id_cnt = t_opt_id;
                if (Error) return END;
            }
            else {
                pos = Decl(pos, out);
                if (Error) return END;
            }
        } else {
            if (is_return[Space]) {
                auto *tmp = new Output_region();
                int t_opt_id = opt_id_cnt;
                pos = Stmt(pos, tmp);
                opt_id_cnt = t_opt_id;
                if (Error) return END;
            }
            else {
                pos = Stmt(pos, out);
                if (Error) return END;
            }
        }
    }
    return pos;
}

int Block(int head, Output_region*& out, bool add_space = true){
    int pos = head;
    if (add_space) {
        Space_cnt++;
        Space_pre[Space_cnt] = Space;
        Space = Space_cnt;
    }
    is_return[Space] = false;
    pos = BlockItem(pos, out);
    if (Error) return END;
    if (!is_return[Space] && Space_pre[Space] == 0) {
        if (now_func_type) {
            Output_region::get_new_id();
            out->insert_ret(var_node(0, 0));
        } else {
            Output_region::get_new_id();
            out->insert_ret_void();
        }
    }
    if (add_space) Space = Space_pre[Space];
    return pos;
}

int FuncFParams(int head, func_params*& p){    //参数列表
    int pos = head;
    bool first = true;
    p = new func_params();
    func_params *t = p;
    while (words[pos].id != 15) {
        func_params *u = nullptr;
        if (first) first = false;
        else {
            // ,
            if (words[pos++].id != 28) {Error = true;puts("params , not found");return true;}
        }
        if (words[pos++].id != 9) {Error = true;puts("params type not right");return true;}
        if (words[pos].id != 1) {Error = true;puts("params name not right");return true;}
        int t_pos = pos++;
        if (words[pos].id == 18) {  //[
            // ]
            if (words[++pos].id != 19) {Error = true;puts("params ] not right");return true;}
            if (words[++pos].id == 18) {  //[  二维数组
                int re_num;
                pos = ConstExp(pos + 1, re_num);
                //]
                if (words[pos++].id != 19) {Error = true;puts("params ] not right");return true;}
                u = new func_params(2, re_num);
                u->name = new char[strlen(words[t_pos].name) + 2];
                strcpy(u->name, words[t_pos].name);
                t->next = u;
                t = u;
                int id = Output_region::get_new_id();
                //if (!sym_insert(words[t_pos].name, Space, id, Dimen(0,re_num))){Error = true;puts("var name repeated");return true;}
            }
            else {  //一维数组
                u = new func_params(1);
                u->name = new char[strlen(words[t_pos].name) + 2];
                strcpy(u->name, words[t_pos].name);
                t->next = u;
                t = u;
                int id = Output_region::get_new_id();
                //if (!sym_insert(words[t_pos].name, Space, id, Dimen(0))){Error = true;puts("var name repeated");return true;}
            }
        }
        else {  //变量
            u = new func_params(0);
            u->name = new char[strlen(words[t_pos].name) + 2];
            strcpy(u->name, words[t_pos].name);
            t->next = u;
            t = u;
            int id = Output_region::get_new_id();
            //if (!sym_insert(words[t_pos].name, Space, id, Dimen())){Error = true;puts("var name repeated");return true;}
        }
    }
    p = p->next;
    return pos;
}

int FuncDef(int head){
    int pos = head;
    Space_cnt++;
    Space_pre[Space_cnt] = Space;
    Space = Space_cnt;
    auto *out = new Output_region();
    opt_id_cnt = -1;
    if (words[pos].id == 10){  //void
        now_func_type = 0;
    }
    else if (words[pos].id == 9){  //int
        now_func_type = 1;
    }
    else {Error = true; puts("unknown type"); return END;}
    pos++;
    int pos_name = pos;
    now_func_name_pos = pos; //记录当前函数名
    is_call_self = false;
    if (words[pos].id != 1) {Error = true; puts("no ident"); return END;}  //ident
    pos++;
    func_params *p = nullptr;
    if (words[pos].id != 14) {Error = true; puts("lack ("); return END;}  //(
    pos++;
    if (words[pos].id == 15){   //)
        pos++;
    }
    else {
        pos = FuncFParams(pos, p);  //参数列表
        if (Error) {puts("wrong at params define");return END;}
        if (words[pos++].id != 15) {Error = true; puts("lack )"); return END;}
    }
    //puts("params ok");
    // {
    if (words[pos++].id != 16) {Error = true; puts("Error at Block 1"); return END;}
    vector<var_node> u;
    int t_opt_id = -1;
    for (func_params *t = p; t != nullptr; t = t->next){
        int id = ++t_opt_id;
        if (t->type == 0) u.emplace_back(var_node(1, id));
        else if (t->type == 1) u.emplace_back(var_node(2, id));
        else if (t->type == 2) u.emplace_back(var_node(3, id, t->len));
        else {Error = true; puts("type not found"); return END;}
    }
    //printf("u size:%d\n",u.size());
    out->insert_def_func(now_func_type, words[pos_name].name, u);
    //将参数赋值到函数内变量中
    t_opt_id = -1;
    Output_region::get_new_id();  //不知道为什么要丢掉一个id
    for (func_params *t = p; t != nullptr; t = t->next){
        t_opt_id++;
        int id = Output_region::get_new_id();
        if (t->type == 0){  //变量
            out->insert_alloc(id);
            out->insert_store(var_node(1, t_opt_id), var_node(1, id));
            if (!sym_insert(t->name, Space, id, Dimen())) {Error = true; return END;}
        }
        else if (t->type == 1){ //一维数组
            out->insert_alloca_star(id);
            out->insert_store(var_node(2, t_opt_id), var_node(2, id));
            if (!sym_insert(t->name, Space, id, Dimen(0))) {Error = true; return END;}
        }
        else if (t->type == 2){  //二维数组
            out->insert_alloca_2_star(id, var_node(0, t->len));
            out->insert_store(var_node(5, t_opt_id, t->len), var_node(5, id, t->len));
            if (!sym_insert(t->name, Space, id, Dimen(0, t->len))) {Error = true; return END;}
        }
        else {Error = true; puts("type not found"); return END;}
    }
    //printf("re_type:%d\n",re_type);
    sym_insert(words[pos_name].name, 0, 0, Dimen(), false, true, now_func_type, p);
    pos = Block(pos, out ,false);
    if (Error){puts("something wrong"); return END;}
    // }
    if (words[pos++].id != 17) {Error = true; puts("Error at Block 2"); return END;}
    //加一个ret语句防止报错
    while (out->pre != nullptr) out = out->pre;
    //puts("pass");
    if (!strcmp(words[now_func_name_pos].name,"main") || is_call_self) {
        opt_id_cnt = -1;
        out->output(); //输出
        printf("}\n");
    }
    else {
        /*opt_id_cnt = -1;
        out->output(); //输出
        printf("}\n");*/
        symbol *self = sym_getIdent(words[now_func_name_pos].name, 0);
        self->out = out;
        self->max_id = opt_id_cnt;
        self->isinline = true;
    }
    Space = Space_pre[Space];
    return pos;
}

int CompUnit(int head){
    sym_insert("getint",0,0,d0,false,true,1);
    printf("declare i32 @getint()\n");
    sym_insert("getch",0,0,d0,false,true,1);
    printf("declare i32 @getch()\n");
    auto *p = new func_params();
    sym_insert("putint",0,0,d0,false,true,0,p);
    printf("declare void @putint(i32)\n");
    sym_insert("putch",0,0,d0,false,true,0,p);
    printf("declare void @putch(i32)\n");
    auto *p1 = new func_params(1);
    sym_insert("getarray",0,0,d0,false,true,1,p1);
    printf("declare i32 @getarray(i32*)\n");
    auto *p2 = new func_params(0,p1);
    sym_insert("putarray",0,0,d0,false,true,0,p2);
    printf("declare void @putarray(i32, i32*)\n");
    printf("declare void @memset(i32*, i32, i32)\n");
    int pos = head;
    while (pos <= words_len){
        if (words[pos].id == 11) pos = Global_Decl(pos);     //const
        else if (words[pos].id == 10) pos = FuncDef(pos);   //void
        else if (words[pos].id == 9 && words[pos + 1].id == 1){   //int Ident
            if (words[pos + 2].id == 14) pos = FuncDef(pos);            //(
            else pos = Global_Decl(pos);
        }
        else if (words[pos].id == 100) return pos;
        else {Error = true; return END;}
    }
    return pos;
}

int main(){
#ifdef LOCAL
    freopen("in.txt","r",stdin);
    //freopen("out.txt","w",stdout);
#endif
    get_sym();
    Error = false;
    END = words_len;
    CompUnit(1);
    if (Error) return 1;
    return 0;
}
