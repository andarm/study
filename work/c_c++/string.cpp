#include <iostream>

using namespace std; 

int main(void)
{

    
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
