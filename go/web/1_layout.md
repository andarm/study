### 建立layout模板 
``` 
<div class="row">
        <div id="footer" class="col-xs-12 col-md-12 text-center">
            <p class="help-block"> Copyright &copy; xxxx.com</p>
        </div>
 </div>


<script>
function setTime() {
    var http = new XMLHttpRequest;
    http.open("HEAD", ".", false);
    http.send(null);
    datetime.innerText = new Date(http.getResponseHeader("Date")).toUTCString();
    setTimeout("setTime()", 1000);
}
setTime();
$('.dropdown-toggle').dropdown();
</script>
``` 
### 显示时间 html 
``` 
    <div class="row" id="head_row">
        <div class="col-xs-2 col-md-2" id="logo"></div>
        <div id="datetime" class="col-xs-4 col-md-4 text-left"></div>
    </div>
``` 

### 显示时间 CSS 
``` 
#datetime {
    color: sandybrown;
    font-family: Verdana, Geneva, sans-serif;
	font-size: 16px;
	font-style: italic;
    top: 55px;
}

``` 

### 页脚显示html  
``` 
    <div class="row">
        <div id="footer" class="col-xs-12 col-md-12 text-center">
            <p class="help-block"> Copyright &copy; 2016 andarm.com</p>
        </div>
    </div>
``` 
### 页脚显示css  
``` 
   #footer{
    background:#2E363F;
    width: 100%;
}
``` 
