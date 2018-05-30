### 动态的使用table 
``` 
<div class="table-responsive">
    <table id="moudles-table" class="table table-condense table-hover" data-toggle="table" data-show-export="true" data-pagination="true"
    data-click-to-select="true" data-toolbar="#toolbar" data-search="true" data-unique-id="id" data-page-list="[10,25,50,100,All]">
        <thead>
            <tr>
                <th data-field="state" data-checkbox="true"></th>
                <th data-field="id" data-align="center" data-visible="false">编号</th>
                <th data-field="name" data-align="center" data-sortable="true">名称</th>
                <th data-field="type" data-align="center" data-sortable="true">类型</th>
                <th data-field="interval" data-align="center">采集间隔(ms)</th>
                <th data-field="param" data-align="center">参数</th>
                <th data-field="desc" data-align="center">描述</th>
            </tr>
        </thead>
        <tbody>
            {{range .json}}
            <tr>
                <td></td>
                <td>{{.Id}}</td>
                <td>{{.Name}}</td>
                <td>{{.Type}}</td>
                <td>{{.Interval}}</td>
                <td>{{.Param}}</td>
                <td>{{.Desc}}</td>
            </tr>
            {{end}}
        </tbody>
    </table>
</div>
``` 
