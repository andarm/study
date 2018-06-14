### 前端 
- 通过ajax异步机制从后台获取表数据，数据格式是json格式的字符串 
- 然后对相关的字符串转换成表格对应的json对象。  
- 注意： json 对象需要自己初始化table <th> head 的id 是一样的 
``` 
function OnBtnPost(){
    console.log("post data");
    $.ajax({
            url: "test/getjson",
            type: "get",
            dataType : "json",
            data: {},
            success : function(data){ 
                // var val = {"Id":1,"Name":"Reds","Price":"$1"};
                var val = JSON.parse(data);
                //  $('#item_table').bootstrapTable('append',val);
                $("#item_table").bootstrapTable('load',val);
            },
            error:function(data){
                alert("查询失败，请联系管理员");
            }
    }); 
``` 
### bootstrapTable 中的load方法是实现  
- 使用refresh 发现数据是没有更新
