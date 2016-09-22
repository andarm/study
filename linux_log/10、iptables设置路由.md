# iptables
- 如果本地主机有两块网卡，一块连接内网(eth0)，一块连接外网(eth1)，那么可以使用下面的规则将eth0的数据路由到eht1：

- iptables -A FORWARD -i eth0 -o eth1 -j ACCEPT


