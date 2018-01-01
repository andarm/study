  ``` 
  /*8.分割以某字符隔开的字符串*/    
  string = "android-iphone-wp7";  
  /*     **字符串取道'-'为止,后面还需要跟着分隔符'-',     **起到过滤作用,有点类似于第7点     */   
  sscanf(string, "%[^-]-%[^-]-%[^-]", buf1, buf2, buf3);   
  printf("8.string=%s\n", string);    
  printf("8.buf1=%s, buf2=%s, buf3=%s\n\n", buf1, buf2, buf3);
  ``` 
  
