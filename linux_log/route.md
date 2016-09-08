## linux 下添加路由
- route add -net 0.0.0.0 gw 192.168.0.254
	
	注意： ifconfig eth0 192.168.0.1 up  
	1、route 的网关地址必须的本网段的， 
	2、末尾255作为网关地址写入也是失败的


