#include <iostream>


using namespace std ; 

class BASE 
{
    public:
    void SetName(int i)
    {
	cout<<"base:"<<i<<endl;
    }
};

class CNameBase:public BASE
{
public:
    void GetAge()
    {
	BASE::SetName(10);
    }
    void SetName(string i )
    {
	cout<<"name:"<<i<<endl;
    }
};
int main(void)
{

    CNameBase CName ; 
    CName.SetName("aaa");
    CName.GetAge(); 
}
