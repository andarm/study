#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include <map>

using namespace std; 

vector<string> vt ; 
queue<int *> que ;
typedef map<int,string> map_Obj; 
typedef map<int,string>::iterator map_ITER;
typedef pair<map_ITER,bool> pr_ret; 



void if_del()
{
    char *aa = "abcd";
    char buf[10] = {0};
    
    string str = "1234";
    for(int i=0;i<10;i++)
    {
	vt.push_back(str);
    }

    
}
void test_Q()
{
    int *i  = new int(3) ; 
    que.push(i);
}

void test_pair()
{
    map_Obj mp ; 
    pr_ret ret = mp.insert(make_pair<int,string>(1,"abc")); 


}

int main(void)
{

    test_Q();
    int *pt = que.front();
    cout<<"i:"<<*pt<<endl;;
    if_del() ;
    vector<string>::iterator it = vt.begin();
    for(;it!=vt.end();it++)
    cout<<"vt:"<<vt.front()<<endl; 
    string strA="abc|{AA}+{23}";
    cout<<"show:"<<strA.substr(3,7)<<endl;
    string::size_type pos_A(0);
    string::size_type pos_A_1(0);
    string::size_type pos_B(0);

    int i = 0 ; 
    do
    {
	    pos_A=strA.find('{',pos_B);
	    pos_A_1 = strA.find('{',pos_A+1);
	    pos_B = strA.find('}',pos_A);
	    if(pos_B != string::npos && pos_A_1 < pos_B)
	    {
		pos_B=pos_A_1 ; 
		
		cout<<"begin:"<<strA.substr(pos_A,pos_B)<<endl;
		continue; 
	    }
	    if(pos_B!=string::npos)
	    {

		cout<<i++<<"=end:"<<strA.substr(pos_A,pos_B-pos_A+1)<<endl;
	    }
   // }while(0);
    }while(pos_B !=string::npos);
}
