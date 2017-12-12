- community public  是查询密码的意思 
snmputil，就是程序名拉，呵呵。

get，就理解成获取一个信息。

getnext，就理解成获取下一个信息。

walk，就理解成获取一堆信息（嗯，应该说所有数据库子树/子目录的信息）

agent，具体某台机器拉。

community，嗯就是那个“community strings”“查询密码”拉。

oid，这个要多说一下，这个呢，就是物件识别代码（Object Identifier）。

# OID理解
.1.3.6.1.4.1.6302.2.1.2.1.0
.1.3.6.1.4.1.6302.2.1.2.2.0
.1.3.6.1.4.1.6302.2.1.2.3.0
.1.3.6.1.4.1.6302.2.1.2.5  // 目录psBattery 
.1.3.6.1.4.1.6302.2.1.2.6.1.0  //  psBattery 目录下的元素 
.1.3.6.1.4.1.6302.2.1.2.6.2.0


例子：
Iso  (1).org(3).dod(6).internet(1).private(4).transition(868).products(2).chassis(4).card(1).slotCps(2)-  
.-cpsSlotSummary(1).cpsModuleTable(1).cpsModuleEntry(1).cpsModuleModel(3).3562.3 

# linux 下相关命令的  
- snmpget -v 1 -c public 10.169.86.113 .1.3.6.1.4.1.6302.2.1.2.1.0


