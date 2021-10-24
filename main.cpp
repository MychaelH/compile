#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cctype>
#include<cmath>
#define LL long long int
#define REP(i,n) for (int i = 1; i <= (n); i++)
#define Redge(u) for (int k = h[u],to; k; k = ed[k].nxt)
#define cls(s,v) memset(s,v,sizeof(s))
using namespace std;

const int err = 12;
char text[10005],expr[10005];
char c;
bool is_get = false,good = true;
long long int num = 0,num_expr = 0;
int pos = 0,L = 0,len_exp;

//读入 
char get_char(){
	return ++pos <= L ? text[pos] : EOF;
}

void get_next_char(){
	if (!is_get) c = get_char();
	else is_get = false;
	while (c == ' ' || c == '\n' || c == '\r' || c == '\t') c = get_char();
}

//词法处理 
void get_string(const char *s){
	int len = strlen(s);
	bool first = true;
	for (int i = 0; i < len; i++){
		if (first){
			first = false;
			get_next_char();
		}
		else c = get_char();
		if (c != s[i]) good = false;
	}
}

bool is_hex(char ch){
	return isdigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

int hex(char ch){
	return ch >= '0' && ch <= '9' ? (ch - '0') : (tolower(ch) - 'a' + 10);
}

bool is_oct(char ch){
	return ch >= '0' && ch <= '7';
}

void get_number(){
	//get_next_char();
	if (c == '0'){
		c = get_char();
		if (c == 'x' || c == 'X'){
			c = get_char();
			if (is_hex(c)){
				num = hex(c);
				c = get_char();
				while (is_hex(c)){
					num = num * 16 + hex(c);
					c = get_char();
				}
				is_get = true;
			}
			else good = false;
		}
		else if (is_oct(c)){
			num = c - '0';
			c = get_char();
			while (is_oct(c)) {
				num = num * 8 + c - '0';
				c = get_char();
			}
			is_get = true;
		}
		else {
			num = 0;
			is_get = true;	
		}
	}
	else if (isdigit(c)){
		num = c - '0';
		c = get_char();
		while (isdigit(c)) {
			num = num * 10 + c - '0';
			c = get_char();
		}
		is_get = true;
	}
	else good = false;
}

bool is_exp(char ch){
	return isdigit(c) || c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')'; 
}

int get_num_len(LL x){
	int re = 0;
	while (x) x /= 10,re++;
	return re;
}

//计算表达式的值 
int cal(int l,int r){
	int state = 0;
	for (int i = r; i >= l; i--){
		if (expr[i] == '(') state++;
		else if (expr[i] == ')') state--;
		else if (!state){
			if (expr[i] == '+' && (i == l || (expr[i - 1] != '*' && expr[i - 1] != '/'))){
				if (i + 1 > r){
					good = true;
					return 0;
				}
				return cal(l,i - 1) + cal(i + 1,r);
			}
			if (expr[i] == '-' && (i == l || (expr[i - 1] != '*' && expr[i - 1] != '/'))){
				if (i + 1 > r){
					good = true;
					return 0;
				}
				return cal(l,i - 1) - cal(i + 1,r);
			} 
		}
	}
	state = 0;
	for (int i = r; i >= l; i--){
		if (expr[i] == '(') state++;
		else if (expr[i] == ')') state--;
		else if (!state){
			if (expr[i] == '*'){
				if (l > i - 1){
					good = true;
					return 0;
				}
				if (i + 1 > r){
					good = true;
					return 0;
				}
				return cal(l,i - 1) * cal(i + 1,r);
			} 
			if (expr[i] == '/'){
				if (l > i - 1){
					good = true;
					return 0;
				}
				if (i + 1 > r){
					good = true;
					return 0;
				}
				
				return cal(l,i - 1) / cal(i + 1,r);
			}
		}
	}
	if (expr[l] == '(' && expr[r] == ')') return cal(l + 1,r - 1);
	int res = 0;
	for (int i = l; i <= r; i++) res = res * 10 + expr[i] - '0';
	return res;
}

void check_bracket(){
	int tmp = 0;
	for (int i = 1; i <= len_exp; i++){
		if (expr[i] == '(') tmp++;
		if (expr[i] == ')') tmp--;
		if (tmp < 0){
			good = false;
			return;
		}
	}
	if (tmp) good = false;
}

//读入表达式并初步检验合法性后调用求值 
void get_exp(){
	len_exp = 0;
	get_next_char();
	while (is_exp(c)){
		if (isdigit(c)){
			get_number();
			int len_num = get_num_len(num);
			LL tmp = 1;
			for (int i = 1; i < len_num; i++){
				tmp *= 10;
			}
			for (int i = 1; i <= len_num; i++){
				expr[++len_exp] = num / tmp % 10 + '0';
				tmp /= 10;
			}
		}
		else if (c == '-' || c == '+'){
			bool flag = true;
			while (c == '-' || c == '+'){
				if (c == '-') flag = !flag;
				get_next_char();
			}
			is_get = true;
			if (flag) expr[++len_exp] = '+';
			else expr[++len_exp] = '-';
		}
		else expr[++len_exp] = c;
		get_next_char();
	}
	is_get = true;
	check_bracket();
	//puts(expr + 1);
	if (good) num_expr = cal(1,len_exp);
}

//语法处理
 
void Stmt(){
	//puts("A");
	get_string("return");
	//puts("A+");
	//get_number();
	get_exp();
	//puts("AA");
	get_string(";");
	//puts("AAA");
}

void Block(){
	get_string("{");
	Stmt();
	get_string("}");
}

void Ident(){
	get_string("main");
}

void FuncType(){
	get_string("int");
}

void FuncDef(){
	FuncType();
	Ident();
	get_string("(");
	get_string(")");
	Block();
}

//输出 
void output(){
	printf("define dso_local i32 @main(){\n");
    printf("    ret i32 %lld\n",num_expr);
    printf("}");
}

//初始化 
void get_in(){
	c = getchar();
	while (c != EOF){
		text[++L] = c;
		c = getchar();
	}
}

void get_clear(){
	int head = -1,last = false;
	for (int i = 1; i <= L; i++){
		if (last && text[i] == '*'){
			head = i - 1;
			int end = i + 1,laststar = false;
			for (; end < L; end++){
				if (laststar && text[end] == '/') break;
				else if (text[end] == '*') laststar = true;
				else laststar = false;
			}
			for (int j = head; j <= end; j++) text[j] = ' ';
			i = end;
		}
		else if (text[i] == '/'){
			if (!last) last = true;
			else {
				head = i - 1;
				int end = i;
				for (; end < L; end++){
					if (text[end + 1] == '\n' || text[end + 1] == '\r') break;
				}
				for (int j = head; j <= end; j++) text[j] = ' ';
				i = end;
			}
		}
		else last = false;
	}
}

void init(){
	get_in();
	get_clear();
}

int main(){
	//freopen("in.txt","r",stdin);
	//freopen("out.txt","w",stdout);
	init();
	FuncDef();
	get_next_char();
	if (c != EOF) good = false;
	if (!good) return 1;
	output();
	return 0;
}
