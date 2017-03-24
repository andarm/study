# vector和queue 
- 函数用法 ： 1、queue有vector没有，push_front(),pop_front() ; 
- vector有而deque无的：capacity(), reserve();


# list  
- find 
- list中要拍戏，可以使用自带函数sort()



#**STL时间复杂度**
- 根据查询，插入，删除等操作需要多少机器时间来计算。
- 如果是循环遍历那么时间复杂度是O(N)
- map,set,时间复杂度O(logN)


# map或者其他容器删除需要注意 
``` 
stl之map erase方法的正确使用
STL的map表里有一个erase方法用来从一个map中删除掉指令的一个节点，不存在任何问题。
如果删除多一个节点时，需要使用正确的调用方法。比如下面的方法是有问题：
for(ITER iter=mapTest.begin();iter!=mapTest.end();++iter)
{
cout<<iter->first<<":"<<iter->second<<endl;
mapTest.erase(iter);
}
这是一种错误的写法,会导致程序行为不可知.究其原因是map 是关联容器,对于关联容器来说，如果某一个元素已经被删除，那么其对应的迭代器就失效了，不应该再被使用；否则会导致程序无定义的行为。

**正确的使用方法:**
- 1).使用删除之前的迭代器定位下一个元素。STL建议的使用方式
for(ITER iter=mapTest.begin();iter!=mapTest.end();)
{
cout<<iter->first<<":"<<iter->second<<endl;
mapTest.erase(iter++);
}
``` 
## erase 还有一种方法
``` 
iter=mapTest.erase(iter);  //原因是erase返回的是下一个元素的迭代器
``` 

