#include <iostream>

using namespace std ; 
typedef struct tagST_TEST
{
    char name[20] ; 
    int age ; 
}ST_TEST;
//template <class  T>   // ok 
template <typename T> 
class CA 
{
    public:
     void T_func( void)
    {	
	int iAge  = static_cast<T*>(this)->age  ; 
	cout<<"iAge:"<<iAge << endl;
    }
     T  count(T a, T b)
    {
	return (a + b) ;   

    }

}; 

int main(void)
{
    int  a = 1 ,b = 2; 
    CA<int> ca ; 
    cout<<"int:"<<ca.count(a,b)<<endl;

    CA<ST_TEST> CST ; 
    CST.T_func();  



}
