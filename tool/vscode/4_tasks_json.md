### vscode 中html 通过chrome浏览器打开
``` 
  "tasks": [
        {
            "label": "html show",
            "type": "process",                  //运行的是一个程序  ； 如果是调用系统命令，则为shell 
            "command": "chrome.exe",            //这里这个不知否？？？ 暂时还没搞懂其作用      
            "args": [
                "${file}"                       //args 是command中调用生成的目标文件
            ],
            "windows": {
                "command": "C:/Program Files (x86)/Google/Chrome/Application/chrome.exe"  //执行使用的工具 ;由于我在window 下执行的这个才有用
            },
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": [
                "$gcc"
            ]
        }
    ]
``` 
