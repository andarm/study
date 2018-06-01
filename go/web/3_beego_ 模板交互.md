### map 数据类型交互
- 可以通过下表直接获取  
<pre> 
{{index 变量名 .map_value/key}} 
</pre> 

### 通过结构体struct交互  
<pre>
type AA struct {
  Id string 
  Name string 
}
ss := AA{
  "1",
  "aa",
}
this.Data[getvalue] = ss 
</pre> 
--- 

html 中交互 
<pre> 
<p>{{.getvalue.Id}} </p>
<p>{{.getvalue.Name}} </p>
</pre> 

###  通过数组指针  
<pre> 
type AA struct {
  Id string 
  Name string 
}
ss := []AA{
  {
  "1",
  "aa",
  },
}
{{range .getvalue}}
    <p>循环显示数组指针里面的内容{{.Id}}</p>
{{end}}
