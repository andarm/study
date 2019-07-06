# web上显示时间，同时实时刷新时间的显示
``` 
function myclick(){

		// document.write(show);
		var myname =new Date(); //获取 时间 Sun Mar 18 2018 16:45:42 GMT+0800 (中国标准时间)
	    var show="val:"+myname
		var getTmp = document.getElementById("myinput");
		getTmp.value=show;     //从元素中更新
		setTimeout(myclick,1000);  //定时更新时间，这里是1S 
	}
```  
