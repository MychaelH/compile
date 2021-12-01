//
// Created by 97677 on 2021/11/27.
//

#ifndef COMPLIE_OUTPUT_REGION_H
#define COMPLIE_OUTPUT_REGION_H
#include<queue>
#include <utility>
#include<vector>
#include<cstring>

struct var_node{
    int type{};  //0:i32 id  1:i32 %id  2:i32* %id
    int id{};
    var_node()= default;
    var_node(int t,int i){
        this->type = t;
        this->id = i;
    }
};

struct Output_region;

struct output_unit{
    int type{0};  //0:line 1:block
    int opt_t{}; //0:alloc 1:store 2:load 3:call void 4:call i32 5:ret
    //6:add 7:sub 8:mul 9:sdiv 10:srem
    //icmp....
    int left_id{};
    char *name{nullptr};
    vector<var_node> opt_num;
    Output_region* block{};
    output_unit()= default;
    output_unit(int type,int opt_t,int left_id){
        this->type = type;
        this->opt_t = opt_t;
        this->left_id = left_id;
    }
    output_unit(int type,int opt_t,var_node a,var_node b){
        this->type = type;
        this->opt_t = opt_t;
        (this->opt_num).push_back(a);
        (this->opt_num).push_back(b);
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
};


struct Output_region{
    int opt_id_cnt;
    vector<output_unit> out;
    Output_region *pre;
    Output_region(){
        pre = nullptr;
        opt_id_cnt = 0;
    }
    explicit Output_region(int start){
        pre = nullptr;
        opt_id_cnt = start;
    }
    int get_new_id(){
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
        out.emplace_back(output_unit(0, 6, a, b));
    }
    void insert_sub(int id,var_node a,var_node b){
        out.emplace_back(output_unit(0, 7, a, b));
    }
    void insert_mul(int id,var_node a,var_node b){
        out.emplace_back(output_unit(0, 8, a, b));
    }
    void insert_sdiv(int id,var_node a,var_node b){
        out.emplace_back(output_unit(0, 9, a, b));
    }
    void insert_srem(int id,var_node a,var_node b){
        out.emplace_back(output_unit(0, 10, a, b));
    }
    void insert_block(Output_region& inside_region){
        out.emplace_back(output_unit(1, &inside_region));
        inside_region.pre = this;
        this->opt_id_cnt += inside_region.opt_id_cnt;
    }
    int output(int head){
        int opt_cnt = head;
        for (unsigned int i = 0; i < out.size(); i++){
            output_unit& u = out[i];
            if (u.type == 1){
                opt_cnt = (u.block)->output(opt_cnt);
            }
            else {
                switch(u.opt_t){
                    case 0: //alloc
                        printf("\t%%%d = alloca i32\n",++opt_cnt);
                        break;
                    case 1:
                        if (u.opt_num[0].type == 0){
                            printf("\tstore i32 %d, i32* %%%d\n", u.opt_num[0].id, u.opt_num[1].id);
                        }
                        else if (u.opt_num[0].type == 1){
                            printf("\tstore i32 %%%d, i32* %%%d\n", u.opt_num[0].id, u.opt_num[1].id);
                        }
                        else puts("Error at output1");
                        break;
                    case 2:
                        printf("\t%%%d = load i32* %%%d\n",++opt_cnt,u.opt_num[0].id);
                        break;
                    case 3:
                        printf("\tcall void @%s(",u.name);
                        for (unsigned int j = 0; j < u.opt_num.size(); j++){
                            if (j) printf(", ");
                            if (u.opt_num[j].type == 0) {
                                printf("i32 %d");
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("i32 %%%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 2){
                                printf("i32* %%%d", u.opt_num[j].id);
                            }
                        }
                        printf(")\n");
                        break;
                    case 4:
                        printf("\t%%%d = call i32 @%s(", ++opt_cnt, u.name);
                        for (unsigned int j = 0; j < u.opt_num.size(); j++){
                            if (j) printf(", ");
                            if (u.opt_num[j].type == 0) {
                                printf("i32 %d");
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("i32 %%%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 2){
                                printf("i32* %%%d", u.opt_num[j].id);
                            }
                        }
                        printf(")\n");
                        break;
                    case 5:
                        printf("\tret ");
                        if (u.opt_num[0].type == 0){
                            printf("i32 %d\n", u.opt_num[0].id);
                        }
                        else if (u.opt_num[0].type == 1){
                            printf("i32 %%%d\n", u.opt_num[0].id);
                        }
                        else puts("Error at output5");
                        break;
                    case 6:
                        printf("\t%%%d = add i32 ", ++opt_cnt);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id);
                            }
                            else puts("Error at output6");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        break;
                    case 7:
                        printf("\t%%%d = sub i32 ", ++opt_cnt);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id);
                            }
                            else puts("Error at output6");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        break;
                    case 8:
                        printf("\t%%%d = mul i32 ", ++opt_cnt);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id);
                            }
                            else puts("Error at output6");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        break;
                    case 9:
                        printf("\t%%%d = sdiv i32 ", ++opt_cnt);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id);
                            }
                            else puts("Error at output6");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        break;
                    case 10:
                        printf("\t%%%d = srem i32 ", ++opt_cnt);
                        for (int j = 0; j <= 1; j++){
                            if (u.opt_num[j].type == 0){
                                printf("%d", u.opt_num[j].id);
                            }
                            else if (u.opt_num[j].type == 1){
                                printf("%%%d", u.opt_num[j].id);
                            }
                            else puts("Error at output6");
                            if (!j) printf(", ");
                        }
                        printf("\n");
                        break;
                    default:puts("Error at output");break;
                }
            }
        }
        return opt_cnt;
    }

};

#endif //COMPLIE_OUTPUT_REGION_H
