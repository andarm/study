### ajax 使用 
``` 
<script>
	var $table= $('#tcp-table') ; 

	$(function(){
		var val = {state:1,id:{{.getvalue.Id}},name:{{.getvalue.Name}}};
		$table.bootstrapTable('append',val);
	
        setInterval('myrefresh()',3000); //指定1秒刷新一次

    });
    function myrefresh()
    {
        $.ajax({url:"ACInput",success:function(result){
            var val = {state:1,id:{{.getvalue.Id}},name:{{.getvalue.Name}}};
		    $table.bootstrapTable('append',val);
        }
        });
    }

    // });
</script>
``` 
