### beego中form相关 
- post接口chrome 刷新时，当输入过一次后，再刷新每次都需要选择确认 
  同时触发post接口函数的响应 
  
- 原因： 
- 	//	this.TplName = "testbootstrap.tpl"    // 原来使用的方式 
- this.Ctx.Redirect(302, "testbootstrap")  // 修改后无该现象，但原理却还不知道

###　TplName　与　Redirect　区别　　
－　this.TplName只是重新渲染页面，并不执行任何方法。　//实际实验，刷新页面时是不执行方法，但是会弹出提示框，一旦确定如果form绑定是post则进行post中的方法
－　this.Redirect()跳回本页面时执行Get绑定的方法，一般不绑定就执行controller中的Get()方法。
