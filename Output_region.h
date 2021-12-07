//
// Created by 97677 on 2021/11/27.
//

#ifndef COMPLIE_OUTPUT_REGION_H
#define COMPLIE_OUTPUT_REGION_H
#include<queue>
#include <utility>
#include<vector>
#include<cstring>
#include "symbol_table.h"
using namespace std;
static int opt_id_cnt;
//while循环记录
static int while_head[10005],while_out[10005],st_while[10005],while_id_cnt,while_pos;
//防止一块输出多个br
static int cnt_br;

struct var_node{
    int type{0};  //0:i32 id  1:i32 %id  2:i32* %id 3:i32 @a
    //4:i32* @a 5:[len x i32]* %id 6:[len x i32]* @a
    int id{0};
    int len{0};
    char* name{};
    var_node()= default;
    var_node(int t, int i, int l = 0){
        this->type = t;
        this->id = i;
        this->len = l;
        name = nullptr;
    }
    explicit var_node(const char* s){
        this->type = 3;
        this->name = new char[sizeof(char) * (strlen(s) + 2)];
        strcpy(this->name, s);
    }
};

struct Output_region;

struct output_unit{
    int type{0};  //0:line 1:block  2:block with
    int opt_t{}; //0:alloc 1:store 2:load 3:call void 4:call i32 5:ret
    //6:add 7:sub 8:mul 9:sdiv 10:srem 11:zext 12:icmp eq
    //13:ne 14:sgt 15:slt 16:sge 17:sle 18:br 19:label
    //20:br break  21:br continue
    //22:alloca 1D  23:alloc 2D
    //24:getelementptr 1D  25:getelementptr 2D
    //26:memset  27:define xxx @s(){
    //28:getelementptr 2D for 1D
    //29:alloc [? x i32]*   30:alloc i32*
    //31:insert_getele_ptr_1
    //32:insert_getele_ptr_2
    //33:ret void
    //icmp....
    int left_id{};
    int delta{0};
    int return_id{0};
    int out_label{0};
    char *name{nullptr};
    func_params *p{nullptr};
    vector<var_node> opt_num;
    Output_region* block{};
    output_unit()= default;
    output_unit(int type,int opt_t,int left_id = 0){
        this->type = type;
        this->opt_t = opt_t;
        this->left_id = left_id;
    }
    output_unit(int type, int opt_t, var_node a, var_node b, int left_id = 0){
        this->type = type;
        this->opt_t = opt_t;
        (this->opt_num).push_back(a);
        (this->opt_num).push_back(b);
        this->left_id = left_id;
    }
    output_unit(int type, int opt_t, var_node a, var_node b, var_node c, int left_id = 0){
        this->type = type;
        this->opt_t = opt_t;
        (this->opt_num).push_back(a);
        (this->opt_num).push_back(b);
        (this->opt_num).push_back(c);
        this->left_id = left_id;
    }
    output_unit(int type, int opt_t, var_node a, var_node b, var_node c, var_node d, int left_id = 0){
        this->type = type;
        this->opt_t = opt_t;
        (this->opt_num).push_back(a);
        (this->opt_num).push_back(b);
        (this->opt_num).push_back(c);
        (this->opt_num).push_back(d);
        this->left_id = left_id;
    }
    output_unit(int type, int opt_t, var_node a, var_node b, var_node c, var_node d, var_node e, int left_id = 0){
        this->type = type;
        this->opt_t = opt_t;
        (this->opt_num).push_back(a);
        (this->opt_num).push_back(b);
        (this->opt_num).push_back(c);
        (this->opt_num).push_back(d);
        (this->opt_num).push_back(e);
        this->left_id = left_id;
    }
    output_unit(int type, int opt_t, int left_id, var_node a){
        this->type = type;
        this->opt_t = opt_t;
        this->left_id = left_id;
        (this->opt_num).push_back(a);
    }
    output_unit(int type, int opt_t, const char* s, vector<var_node> v){
        this->type = type;
        this->opt_t = opt_t;
        this->name = new char[sizeof(char) * (strlen(s) + 2)];
        strcpy(this->name, s);
        opt_num = v;
    }
    output_unit(int type, int opt_t,int id, const char* s, vector<var_node> v){
        this->type = type;
        this->opt_t = opt_t;
        this->left_id = id;
        this->name = new char[sizeof(char) * (strlen(s) + 2)];
        strcpy(this->name, s);
        opt_num = v;
    }
    output_unit(int type,int opt_t,var_node a){
        this->type = type;
        this->opt_t = opt_t;
        (this->opt_num).push_back(a);
    }
    output_unit(int type,Output_region* b){
        this->type = type;
        this->block = b;
    }
    output_unit(int type,Output_region* b,int delta, int return_id, int out_label){
        this->type = type;
        this->block = b;
        this->delta = delta;
        this->return_id = return_id;
        this->out_label = out_label;
    }
};


struct Output_region{
    int label;
    bool is_jump,is_labeled;
    vector<output_unit> out;
    Output_region *pre, *p_yes, *p_no, *p_jump;
    Output_region(){
        label = -1;
        pre = p_yes = p_no = p_jump = nullptr;
        is_jump = false;
        is_labeled = false;
    }
    //设置输出块标签用以跳转
    void set_label(){  //only be used when the region is just created
        label = ++opt_id_cnt;
        is_labeled = true;
    }
    //设置模块末尾有跳转
    void set_is_jump(){
        is_jump = true;
    }
    static int get_new_id(){
        return ++opt_id_cnt;
    }
    void insert_alloc(int id){
        out.emplace_back(0,0,id);
    }
    void insert_store(var_node a,var_node b){
        out.emplace_back(output_unit(0,1,a,b));
    }
    void insert_load(int id,var_node a){
        out.emplace_back(output_unit(0,2,id,a));
    }
    void insert_call_void(const char* name,vector<var_node> v){
        out.emplace_back(output_unit(0,3,name,v));
    }
    void insert_call_i32(int id, const char* name,vector<var_node> v){
        out.emplace_back(output_unit(0,4,id,name,v));
    }
    void insert_ret(var_node a){
        out.emplace_back(output_unit(0,5,a));
    }
    void insert_add(int id,var_node a,var_node b){
        out.emplace_back(output_unit(0, 6, a, b, id));
    }
    void insert_sub(int id,var_node a,var_node b){
        out.emplace_back(output_unit(0, 7, a, b, id));
    }
    void insert_mul(int id,var_node a,var_node b){
        out.emplace_back(output_unit(0, 8, a, b, id));
    }
    void insert_sdiv(int id,var_node a,var_node b){
        out.emplace_back(output_unit(0, 9, a, b, id));
    }
    void insert_srem(int id,var_node a,var_node b){
        out.emplace_back(output_unit(0, 10, a, b, id));
    }
    void insert_zext(int id, var_node a){
        out.emplace_back(output_unit(0, 11, id, a));
    }
    void insert_icmp_eq(int id, var_node a, var_node b){
        out.emplace_back(output_unit(0, 12, a, b, id));
    }
    void insert_icmp_ne(int id, var_node a, var_node b){
        out.emplace_back(output_unit(0, 13, a, b, id));
    }
    void insert_icmp_sgt(int id, var_node a, var_node b){
        out.emplace_back(output_unit(0, 14, a, b, id));
    }
    void insert_icmp_slt(int id, var_node a, var_node b){
        out.emplace_back(output_unit(0, 15, a, b, id));
    }
    void insert_icmp_sge(int id, var_node a, var_node b){
        out.emplace_back(output_unit(0, 16, a, b, id));
    }
    void insert_icmp_sle(int id, var_node a, var_node b){
        out.emplace_back(output_unit(0, 17, a, b, id));
    }
    void insert_br(int id){
        out.emplace_back(output_unit(0, 18, id));
    }
    void insert_label(int id){
        out.emplace_back(output_unit(0, 19, id));
    }
    void insert_br_while_head(int id){
        out.emplace_back(output_unit(0, 20, id));
    }
    void insert_br_while_out(int id){
        out.emplace_back(output_unit(0, 21, id));
    }
    void insert_alloca_1(int id,var_node a){
        out.emplace_back(output_unit(0,22,id,a));
    }
    void insert_alloca_2(int id,var_node a,var_node b){
        out.emplace_back(output_unit(0, 23, a, b, id));
    }
    void insert_getele_1(int id,var_node a, var_node b, var_node pos){ //维度大小+对象+位移
        out.emplace_back(output_unit(0, 24, a, b, pos, id));
    }
    void insert_getele_2(int id, var_node a, var_node b, var_node c, var_node pos1, var_node pos2){ //维度大小+对象+位移
        out.emplace_back(output_unit(0, 25, a, b, c, pos1, pos2, id));
    }
    void insert_memset(var_node a,var_node b){
        out.emplace_back(output_unit(0,26,a,b));
    }
    void insert_def_func(int id, const char* name,vector<var_node> v){
        out.emplace_back(output_unit(0,27,id,name,v));
    }
    void insert_getele_2_to_1(int id, var_node a, var_node b, var_node c, var_node pos1){
        out.emplace_back(output_unit(0, 28, a, b, c, pos1, id));
    }
    void insert_alloca_2_star(int id,var_node a){
        out.emplace_back(output_unit(0,29,id,a));
    }
    void insert_alloca_star(int id){
        out.emplace_back(output_unit(0,30,id));
    }
    void insert_getele_ptr_1(int id, var_node a, var_node pos){
        out.emplace_back(output_unit(0, 31, a, pos, id));
    }
    void insert_getele_ptr_2(int id, var_node a, var_node b, var_node pos1, var_node pos2){
        out.emplace_back(output_unit(0, 32, a, b, pos1, pos2, id));
    }
    void insert_ret_void(){
        out.emplace_back(output_unit(0, 33));
    }
    void insert_getelepr_2_1(int id, var_node a, var_node b, var_node pos){
        out.emplace_back(output_unit(0, 34, a, b, pos, id));
    };
    void insert_block(Output_region* inside_region){
        out.emplace_back(output_unit(1, inside_region));
        inside_region->pre = this;
    }
    void insert_inline_call(int re_id, symbol* p, int& opt_cnt,int delta, const vector<var_node>& para){
        int id = 0;
        for (unsigned int j = 0; j < para.size(); j++,id++){
            const var_node& t = para[j];
            if (t.type == 0 || t.type == 1) {
                this->insert_add(id + delta, var_node(0, 0), var_node(t.type, t.id));
            }
            else if (t.type == 2){
                this->insert_getele_ptr_1(id + delta, t, var_node(0, 0));
            }
            else if (t.type == 5) {
                this->insert_getelepr_2_1(id + delta, var_node(0, t.len), t, var_node(0, 0));
            }
        }
        this->insert_add(id + delta, var_node(0, 0), var_node(0, 0));
        opt_cnt = id + delta;
        this->out.emplace_back(output_unit(2, p->out, delta, re_id - delta, p->max_id + 1));
        //printf("%d\n",p->max_id);
        opt_cnt = p->max_id + delta + 1;
        this->insert_label(opt_cnt);
        this->insert_add(++opt_cnt, var_node(0, 0), var_node(0, 0));
    }
    void output(int type = 1, int delta = 0, int return_id = 0, int out_label = 0){
        if (is_labeled && this->label != -1){
            printf("%d:\n",this->label + delta);
            opt_id_cnt++;
            cnt_br = 0;
        }
        for (auto & u : out){
            if (u.type == 1){
                (u.block)->output(type, delta, return_id, out_label);
            }
            else if (u.type == 2){
                (u.block)->output(2, u.delta + delta, u.return_id, u.out_label);
            }
            else {
                switch(u.opt_t){
                    case 0: //alloc
                        printf("\t%%%d = alloca i32\n",u.left_id + delta);
                        opt_id_cnt++;
                        break;
                    case 1:  //store
                        if (u.opt_num[0].type == 0){
                            if (u.opt_num[1].type == 3) printf("\tstore i32 %d, i32* @%s\n", u.opt_num[0].id, u.opt_num[1].name);
                            else if (u.opt_num[1].type == 1) printf("\tstore i32 %d, i32* %%%d\n", u.opt_num[0].id, u.opt_num[1].id + delta);
                            else if (u.opt_num[1].type == 2) printf("\tstore i32 %d, i32* * %%%d\n", u.opt_num[0].id, u.opt_num[1].id + delta);
                        }
                        else if (u.opt_num[0].type == 1){
                            if (u.opt_num[1].type == 3) printf("\tstore i32 %%%d, i32* @%s\n", u.opt_num[0].id + delta, u.opt_num[1].name);
                            else if (u.opt_num[1].type == 1) printf("\tstore i32 %%%d, i32* %%%d\n", u.opt_num[0].id + delta, u.opt_num[1].id + delta);
                        }
                        else if (u.opt_num[0].type == 2){
                            if (u.opt_num[1].type == 4) printf("\tstore i32* %%%d, i32* * @%s\n", u.opt_num[0].id + delta, u.opt_num[1].name);
                            else if (u.opt_num[1].type == 2) printf("\tstore i32* %%%d, i32* * %%%d\n", u.opt_num[0].id + delta, u.opt_num[1].id + delta);
                        }
                        else if (u.opt_num[0].type == 5) {
                            if (u.opt_num[1].type == 5) printf("\tstore [%d x i32]* %%%d, [%d x i32]* * %%%d\n", u.opt_num[0].len, u.opt_num[0].id + delta, u.opt_num[1].len, u.opt_num[1].id + delta);
                            else puts("Error at output1");
                        }
                        else puts("Error at output1");
                        break;
                    case 2:  //load
                        if (u.opt_num[0].type == 5) printf("\t%%%d = load [%d x i32]*, [%d x i32]* * %%%d\n",u.left_id + delta, u.opt_num[0].len, u.opt_num[0].len,u.opt_num[0].id + delta);
                        else if (u.opt_num[0].type == 2) printf("\t%%%d = load i32*, i32* * %%%d\n",u.left_id + delta,u.opt_num[0].id + delta);
                        else if (u.opt_num[0].type == 3) printf("\t%%%d = load i32, i32* @%s\n",u.left_id + delta,u.opt_num[0].name);
                        else printf("\t%%%d = load i32, i32* %%%d\n",u.left_id + delta,u.opt_num[0].id + delta);
                        opt_id_cnt++;
                        break;
                    case 3: //call void
                        printf("\tcall void @%s(",u.name);
                        for (unsigned int j = 0; j < u.opt_num.size(); j++){
                            if (j) printf(", ");
                            if (u.opt_num[j].type == 0) {
                                printf("i32 %d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("i32 %%%d", u.opt_num[j].id + delta);
                            }
                            else if (u.opt_num[j].type == 2){
                                printf("i32* %%%d", u.opt_num[j].id + delta);
                            }
                            else if (u.opt_num[j].type == 3){
                                printf("i32 @%s", u.opt_num[j].name);
                            }
                            else if (u.opt_num[j].type == 4){
                                printf("i32* @%s", u.opt_num[j].name);
                            }
                            else if (u.opt_num[j].type == 5){
                                printf("[%d x i32]* %%%d", u.opt_num[j].len, u.opt_num[j].id + delta);
                            }
                            else if (u.opt_num[j].type == 6){
                                printf("[%d x i32]* @%s", u.opt_num[j].len, u.opt_num[j].name);
                            }
                            else puts("wrong type at 3");
                        }
                        printf(")\n");
                        break;
                    case 4:
                        printf("\t%%%d = call i32 @%s(", u.left_id + delta, u.name);
                        for (unsigned int j = 0; j < u.opt_num.size(); j++){
                            if (j) printf(", ");
                            if (u.opt_num[j].type == 0) {
                                printf("i32 %d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("i32 %%%d", u.opt_num[j].id + delta);
                            }
                            else if (u.opt_num[j].type == 2){
                                printf("i32* %%%d", u.opt_num[j].id + delta);
                            }
                            else if (u.opt_num[j].type == 3){
                                printf("i32 @%s", u.opt_num[j].name);
                            }
                            else if (u.opt_num[j].type == 4){
                                printf("i32* @%s", u.opt_num[j].name);
                            }
                            else if (u.opt_num[j].type == 5){
                                printf("[%d x i32]* %%%d", u.opt_num[j].len, u.opt_num[j].id + delta);
                            }
                            else if (u.opt_num[j].type == 6){
                                printf("[%d x i32]* @%s", u.opt_num[j].len, u.opt_num[j].name);
                            }
                            else puts("wrong type at 4");
                        }
                        printf(")\n");
                        opt_id_cnt++;
                        break;
                    case 5:
                        if (type != 2) {
                            printf("\tret ");
                            if (u.opt_num[0].type == 0) {
                                printf("i32 %d\n", u.opt_num[0].id);
                            } else if (u.opt_num[0].type == 1) {
                                printf("i32 %%%d\n", u.opt_num[0].id + delta);
                            } else puts("Error at output5");
                            opt_id_cnt++;
                        }
                        else {
                            printf("\t%%%d = add i32 0, 0\n", ++opt_id_cnt);
                            if (u.opt_num[0].type == 0) {
                                printf("\tstore i32 %d, i32* %%%d\n", u.opt_num[0].id, return_id + delta);
                            } else if (u.opt_num[0].type == 1) {
                                printf("\tstore i32 %%%d, i32* %%%d\n", u.opt_num[0].id + delta, return_id + delta);
                            } else puts("Error at output5");
                            if (cnt_br < 1){
                                printf("\tbr label %%%d\n",out_label + delta);
                                cnt_br++;
                            }

                        }
                        break;
                    case 6: //add
                        printf("\t%%%d = add i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output6");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 7: //sub
                        printf("\t%%%d = sub i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output6");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 8:  //mul
                        printf("\t%%%d = mul i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output6");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 9:  //sdiv
                        printf("\t%%%d = sdiv i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output6");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 10:  //srem
                        printf("\t%%%d = srem i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output6");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 11:  //zext
                        printf("\t%%%d = zext i1 %%%d to i32\n", u.left_id + delta, u.opt_num[0].id + delta);
                        opt_id_cnt++;
                        break;
                    case 12:   //eq
                        printf("\t%%%d = icmp eq i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output12");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 13:  //ne
                        printf("\t%%%d = icmp ne i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output12");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 14: //sgt
                        printf("\t%%%d = icmp sgt i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output12");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 15:  //slt
                        printf("\t%%%d = icmp slt i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output12");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 16:  //sge
                        printf("\t%%%d = icmp sge i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output12");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 17:  //sle
                        printf("\t%%%d = icmp sle i32 ", u.left_id + delta);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id + delta);
                            }
                            else puts("Error at output12");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        opt_id_cnt++;
                        break;
                    case 18:  //br
                        if (cnt_br < 1){
                            printf("\tbr label %%%d\n",u.left_id + delta);
                            cnt_br++;
                        }
                        break;
                    case 19:
                        printf("%d:\n",u.left_id + delta);
                        opt_id_cnt++;
                        cnt_br = 0;
                        break;
                    case 20:
                        if (cnt_br < 1){
                            printf("\tbr label %%%d\n",while_head[u.left_id] + delta);
                            cnt_br++;
                        }
                        break;
                    case 21:
                        if (cnt_br < 1){
                            printf("\tbr label %%%d\n",while_out[u.left_id] + delta);
                            cnt_br++;
                        }
                        break;
                    case 22:
                        printf("\t%%%d = alloca [%d x i32]\n", u.left_id + delta, u.opt_num[0].id);
                        opt_id_cnt++;
                        break;
                    case 23:
                        printf("\t%%%d = alloca [%d x [%d x i32]]\n", u.left_id + delta, u.opt_num[0].id, u.opt_num[1].id);
                        opt_id_cnt++;
                        break;
                    case 24:
                        if (u.opt_num[1].type == 1){
                            printf("\t%%%d = getelementptr [%d x i32], [%d x i32]* %%%d, i32 0, i32 ", u.left_id + delta, u.opt_num[0].id, u.opt_num[0].id, u.opt_num[1].id + delta);
                        }
                        else if (u.opt_num[1].type == 3){
                            printf("\t%%%d = getelementptr [%d x i32], [%d x i32]* @%s, i32 0, i32 ", u.left_id + delta, u.opt_num[0].id, u.opt_num[0].id, u.opt_num[1].name);
                        }
                        else puts("Error at output24");
                        if (u.opt_num[2].type == 0){
                            printf("%d\n",u. opt_num[2].id);
                        }
                        else if (u.opt_num[2].type == 1){
                            printf("%%%d\n",u. opt_num[2].id + delta);
                        }
                        else puts("Error at output24");
                        opt_id_cnt++;
                        break;
                    case 25:
                        if (u.opt_num[2].type == 1) {
                            printf("\t%%%d = getelementptr [%d x [%d x i32]], [%d x [%d x i32]]* %%%d, i32 0, i32 ",u.left_id + delta, u.opt_num[0].id, u.opt_num[1].id, u.opt_num[0].id, u.opt_num[1].id,u.opt_num[2].id + delta);
                        }
                        else if (u.opt_num[2].type == 3) {
                            printf("\t%%%d = getelementptr [%d x [%d x i32]], [%d x [%d x i32]]* @%s, i32 0, i32 ",u.left_id + delta, u.opt_num[0].id, u.opt_num[1].id, u.opt_num[0].id, u.opt_num[1].id,u.opt_num[2].name);
                        }
                        else puts("Error at output25");
                        if (u.opt_num[3].type == 0){
                            printf("%d, i32 ",u. opt_num[3].id);
                        }
                        else if (u.opt_num[3].type == 1){
                            printf("%%%d, i32 ",u. opt_num[3].id + delta);
                        }
                        else puts("Error at output25");
                        if (u.opt_num[4].type == 0){
                            printf("%d\n",u. opt_num[4].id);
                        }
                        else if (u.opt_num[4].type == 1){
                            printf("%%%d\n",u. opt_num[4].id + delta);
                        }
                        else puts("Error at output25");
                        opt_id_cnt++;
                        break;
                    case 26:
                        printf("\tcall void @memset(i32* %%%d, i32 0, i32 %d)\n", u.opt_num[0].id + delta, u.opt_num[1].id);
                        break;
                    case 27:
                        if (type == 2) break;
                        printf("define ");
                        if (u.left_id == 0) printf("void ");
                        else printf("i32 ");
                        printf("@%s(", u.name);
                        for (unsigned int i = 0; i < u.opt_num.size(); i++){
                            if (i) printf(", ");
                            var_node& t = u.opt_num[i];
                            if (t.type == 1){
                                printf("i32 %%%d",t.id);
                            }
                            else if (t.type == 2){
                                printf("i32* %%%d",t.id);
                            }
                            else if (t.type == 3){
                                printf("[%d x i32]* %%%d", t.len, t.id);
                            }
                            else puts("wrong at 27");
                            opt_id_cnt++;
                        }
                        printf("){\n");
                        opt_id_cnt++; //不知道为什么要丢掉一个id
                        break;
                    case 28:
                        if (u.opt_num[2].type == 1) {
                            printf("\t%%%d = getelementptr [%d x [%d x i32]], [%d x [%d x i32]]* %%%d, i32 0, i32 ",u.left_id + delta, u.opt_num[0].id, u.opt_num[1].id, u.opt_num[0].id, u.opt_num[1].id,u.opt_num[2].id + delta);
                        }
                        else if (u.opt_num[2].type == 3) {
                            printf("\t%%%d = getelementptr [%d x [%d x i32]], [%d x [%d x i32]]* @%s, i32 0, i32 ",u.left_id + delta, u.opt_num[0].id, u.opt_num[1].id, u.opt_num[0].id, u.opt_num[1].id,u.opt_num[2].name);
                        }
                        else puts("Error at output28");
                        if (u.opt_num[3].type == 0){
                            printf("%d\n",u. opt_num[3].id);
                        }
                        else if (u.opt_num[3].type == 1){
                            printf("%%%d\n",u. opt_num[3].id + delta);
                        }
                        else puts("Error at output28");
                        opt_id_cnt++;
                        break;
                    case 29:
                        printf("\t%%%d = alloca [%d x i32]*\n", u.left_id + delta, u.opt_num[0].id);
                        opt_id_cnt++;
                        break;
                    case 30:
                        printf("\t%%%d = alloca i32*\n", u.left_id + delta);
                        opt_id_cnt++;
                        break;
                    case 31:
                        if (u.opt_num[1].type == 0) printf("\t%%%d = getelementptr i32, i32* %%%d, i32 %d\n", u.left_id + delta, u.opt_num[0].id + delta, u.opt_num[1].id);
                        else if (u.opt_num[1].type == 1) printf("\t%%%d = getelementptr i32, i32* %%%d, i32 %%%d\n", u.left_id + delta, u.opt_num[0].id + delta, u.opt_num[1].id + delta);
                        else puts("wrong at 31");
                        opt_id_cnt++;
                        break;
                    case 32:
                        printf("\t%%%d = getelementptr [%d x i32], [%d x i32]* %%%d, i32 \n", u.left_id + delta, u.opt_num[0].id, u.opt_num[0].id, u.opt_num[1].id + delta);
                        if (u.opt_num[2].type == 0) printf("%d, i32 ",u.opt_num[2].id);
                        else if (u.opt_num[2].type == 1) printf("%%%d, i32 ",u.opt_num[2].id + delta);
                        else puts("wrong at 32");
                        if (u.opt_num[3].type == 0) printf("%d\n",u.opt_num[3].id);
                        else if (u.opt_num[3].type == 1) printf("%%%d\n",u.opt_num[3].id + delta);
                        else puts("wrong at 32");
                        opt_id_cnt++;
                        break;
                    case 33:
                        if (type != 2) {
                            printf("\tret void\n");
                            opt_id_cnt++;
                        }
                        else {
                            printf("\t%%%d = add i32 0, 0\n", ++opt_id_cnt);
                            if (cnt_br < 1){
                                printf("\tbr label %%%d\n",out_label + delta);
                                cnt_br++;
                            }
                        }
                        break;
                    case 34:
                        printf("\t%%%d = getelementptr [%d x i32], [%d x i32]* %%%d, i32 ", u.left_id + delta, u.opt_num[0].id, u.opt_num[0].id, u.opt_num[1].id + delta);
                        if (u.opt_num[2].type == 0) printf("%d\n",u.opt_num[2].id);
                        else if (u.opt_num[2].type == 1) printf("%%%d\n",u.opt_num[2].id + delta);
                        else puts("wrong at 34");
                        opt_id_cnt++;
                        break;
                    default:puts("Error at output");break;
                }
            }
        }
        if (this->is_jump) {   //如果是带跳跃的模块
            if (this->p_jump != nullptr) {  //直接跳
                Output_region *t = this;
                while (t->p_jump != nullptr && !t->p_jump->is_labeled) {
                    t = t->p_jump;
                }
                if (t->p_jump == nullptr || !t->p_jump->is_labeled) {
                    puts("Wrong at jump???");
                    return;
                }
                if (cnt_br < 1){
                    printf("\tbr label %%%d\n", t->p_jump->label + delta);
                    cnt_br++;
                }
            }
            else {  //分支跳
                int yj, nj;
                //找到yes跳的对象
                if (this->p_yes != nullptr){
                    yj = this->p_yes->label;
                }
                else {
                    Output_region *t = this;
                    while (t != nullptr && t->p_yes == nullptr) {
                        t = t->pre;
                    }
                    if (t == nullptr || t->p_yes == nullptr) {
                        puts("Wrong at jump yes");
                        return;
                    }
                    yj = t->p_yes->label;
                }
                //找到no跳的对象
                if (this->p_no != nullptr){
                    nj = this->p_no->label;
                }
                else {
                    Output_region *t = this;
                    while (t != nullptr && t->p_no == nullptr) {
                        t = t->pre;
                    }
                    if (t == nullptr || t->p_no == nullptr) {
                        puts("Wrong at jump no");
                        return;
                    }
                    nj = t->p_no->label;
                }
                if (cnt_br < 1){
                    printf("\tbr i1 %%%d, label %%%d, label %%%d\n", opt_id_cnt, nj + delta, yj + delta);
                }
            }
        }
    }

};

#endif //COMPLIE_OUTPUT_REGION_H
