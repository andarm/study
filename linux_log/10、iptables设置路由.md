# iptables
- �����������������������һ����������(eth0)��һ����������(eth1)����ô����ʹ������Ĺ���eth0������·�ɵ�eht1��

- iptables -A FORWARD -i eth0 -o eth1 -j ACCEPT


