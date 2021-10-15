#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cctype>
using namespace std;

char c;
bool is_get = false;
int num = 0;

void get_next_char(){
	if (!is_get) c = getchar();
	else is_get = false;
	while (c == ' ' || c == '\n' || c == '\r' || c == '\t') c = getchar();
}

void get_string(const char *s){
	int len = strlen(s);
	bool first = true;
	for (int i = 0; i < len; i++){
		if (first){
			first = false;
			get_next_char();
		}
		else c = getchar();
		if (c != s[i]) exit(1);
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
	get_next_char();
	if (c == '0'){
		c = getchar();
		if (c == 'x' || c == 'X'){
			c = getchar();
			if (is_hex(c)){
				num = hex(c);
				c = getchar();
				while (is_hex(c)){
					num = num * 16 + hex(c);
					c = getchar();
				}
				is_get = true;
			}
			else exit(1);
		}
		else if (is_oct(c)){
			num = c - '0';
			c = getchar();
			while (is_oct(c)) {
				num = num * 8 + c - '0';
				c = getchar();
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
		c = getchar();
		while (isdigit(c)) {
			num = num * 10 + c - '0';
			c = getchar();
		}
		is_get = true;
	}
	else exit(1);
}

void Stmt(){
	//puts("A");
	get_string("return");
	//puts("A+");
	get_number();
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

void output(){
	printf("define dso_local i32 @main(){\n");
    printf("\tret i32 %d\n",num);
    printf("}");
}

int main(){
	//freopen("in.txt","r",stdin);
	//freopen("out.txt","w",stdout);
	FuncDef();
	get_next_char();
	if (c != EOF) exit(1);
	output();
	return 0;
}
