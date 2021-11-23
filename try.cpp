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

int main(){
    Space_pre[1] = 0;
    Space_pre[2] = 1;
    Space_pre[3] = 0;
    int a = sym_insert("a",0,0,false);
    int b = sym_insert("a",0,0,false);
    int bb = sym_insert("aa",0,0,false);
    int c = sym_insert("a",1,10,false);
    int d = sym_insert("a",2,100,false);
    int e = sym_insert("b",3,11,false);
    printf("%d %d %d %d %d %d\n",a,b,bb,c,d,e); //1 0 1 1 1 1
    symbol* u = sym_getIdent("b",0);
    if (u == nullptr) puts("Can not find");
    else printf("%s %d %d\n",u->name,u->id,u->space);
    Space = 2;
    u = sym_getIdent("a",2);
    printf("%s %d %d\n",u->name,u->id,u->space);
    return 0;
}

