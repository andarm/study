# vector �и���Ԫ�ز��ҵķ���
- �õ������Լ�һ����for ȥ����
- ʹ��find  
���磺
---
```c
vector<int> v;
int num_to_find=25;//Ҫ���ҵ�Ԫ��,����Ҫ��vector<>����һ��
for(int i=0;i<10;i++)
v.push_back(i*i);
vector<int>::iterator iter=std::find(v.begin(),v.end(),num_to_find);//���ص���һ��������ָ��
if(iter==v.end())
    cout<<"ERROR!"<<endl;
    else               //ע�������ָ�����Ԫ�صķ�ʽ��distance�÷�
    cout<<"the index of value "<<(*iter)<<" is " << std::distance(v.begin(), iter)<<std::endl;
    return 0;
```
---

