### input button 元素
- 通过js 直接响应表单 
``` 
<form>
<input type="button" id="button1" onclick="alertMsg()"
value="Button 1" />
</form>
``` 
注意： alertMsg()就是需要相应的js 代码 

- 通过后天执行
``` 
<form action="form_action.asp" method="get">
  <p>First name: <input type="text" name="fname" /></p>
  <p>Last name: <input type="text" name="lname" /></p>
  <input type="submit" value="Submit" />
</form>
``` 



