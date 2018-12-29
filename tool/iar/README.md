# 
## [目录](README.md)
## IAR 常用命令 

### 工程同时输出hex和bin文件

  在工程配置中post command line 中添加下面命令 
``` 
"$TOOLKIT_DIR$\bin\ielftool.exe" --bin --verbose  "$TARGET_PATH$" "$TARGET_BPATH$.bin"
``` 
