# vector 中跟着元素查找的方法
- 用迭代器自己一个个for 去查找
- 使用find  
例如：
---
```c
vector<int> v;
int num_to_find=25;//要查找的元素,类型要与vector<>类型一致
for(int i=0;i<10;i++)
v.push_back(i*i);
vector<int>::iterator iter=std::find(v.begin(),v.end(),num_to_find);//返回的是一个迭代器指针
if(iter==v.end())
    cout<<"ERROR!"<<endl;
    else               //注意迭代器指针输出元素的方式和distance用法
    cout<<"the index of value "<<(*iter)<<" is " << std::distance(v.begin(), iter)<<std::endl;
    return 0;
```
---

