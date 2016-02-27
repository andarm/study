/********************************************************************
	created:	2013/12/03
	created:	3:12:2013   10:26
	filename: 	TSrv.h
	file base:	HTSrv
	file ext:	h
	author:		liujia
	
	purpose:	Arm9 HT业务模块
**************************************************************************/
#pragma once
#include "../comm/frame/interface.h"
#include "../comm/sys/logicmsgmng.h"
#ifndef _WIN32
#include "libsocketcan.h"
#include "can_netlink.h"
#endif
using namespace ST_SYS;

#ifndef _WIN32
typedef can_state  CAN_STATE;
#else
// CAN状态
enum CAN_STATE {
    CAN_STATE_ERROR_ACTIVE = 0,	// RX/TX error count < 96 
    CAN_STATE_ERROR_WARNING,	// RX/TX error count < 128 
    CAN_STATE_ERROR_PASSIVE,	// RX/TX error count < 256 
    CAN_STATE_BUS_OFF,		// RX/TX error count >= 256 
    CAN_STATE_STOPPED,		// Device is stopped 
    CAN_STATE_SLEEPING,		// Device is sleeping
    CAN_STATE_MAX
};
#endif

// 总线上最大设备数(0x7f)
#define MAX_DEV_NUM		(0x7f) 
// CAN总线设备检测周期
#define MAX_CAN_CHECK_TIME		(3)
#define MAX_DEV_CHECK_TIME      (15)
#define MAX_NOTIFY_TIME         (3)

// CAN总线地址屏蔽
#define CAN_ADDR_MASK			(0x7f)
// PC机CAN地址
#define LOCAL_PC_ADDR	        (0x01)
// HT地址
#define  LOCAL_HT_ADDR          (0x6f)
// 网关地址
#define GWCANADDR               (0x02)
// CAN广播地址
#define CANBRAODCASTADDR		(0x00)


// CAN通道号
typedef enum
{
    CAN1_CHANNEL=0,
    CAN2_CHANNEL=1,
    CAN3_CHANNEL=2,
    CAN4_CHANNEL=3,
    MAX_CAN_CHANNEL = 4
}CANCHANNEL;


typedef struct tag_CAN_STST
{ 
    CAN_STATE stat;  // 状态
    __UINT32 dwflow; // 流量
}_CAN_STST;

// HT的CAN状态
typedef struct HT_CAN_STAT
{
    _CAN_STST CAN_STAT[MAX_CAN_CHANNEL];
	__INT16  nCanTimeCheckCount;// CAN定时检测计数器
}HT_CAN_STAT;

// 总线上设备CAN状态
typedef struct DEV_CAN_STAT
{
	_CAN_STST CAN1;
	_CAN_STST CAN2;
	__INT16  nCan1TimeCheckCount;// CAN1定时计数器
	__INT16  nCan2TimeCheckCount;// CAN2定时计数器
}DEV_CAN_STAT;


class  CHTSrv:public ILogic,CLogicMsgMng
{
public:

    CHTSrv(void);
    virtual ~CHTSrv(void);
    // 初始化函数,一般传入主框架指针. 失败:0 成功:非0
    virtual	int	Init(void *pMainServer);
    // 释放该模块自己申请的资源
    virtual void	Fini();
    // 消息处理接口
    virtual long  OnHandle(const LogicMsg *pMsg,void *Para = NULL);
protected:
    virtual void OnMessage(LogicMsg *pMsg);
    bool SendMsg(const ILogic *pLogic,const char *pData,const __UINT32 &dwSize);
private:
    // 接口连接成功
    void OnConnect(const int &nIndex,void *Para);
    // 登陆服务端(TCP模式有效)
    void LoginSrv();
    // 接口断连
    void OnDisConnect(const int &nIndex,void *Para);
    // 定时器回调函数
    long OnTimer(void *Para);
	// 更新总线设备CAN状态表
	void UpdateDevCanState(const LogicMsg *pMsg);
    // 发送数据到 CAN总线
    void TransDataToCAN(const char *data,const __UINT32 &dwLen,const CANCHANNEL &ch,const __UINT16 &Addr = CANBRAODCASTADDR);
	// 检测总线设备CAN状态表
	void CheckDevCanState(void);
	// 检测HT的CAN状态
	void CheckHtCanState(void);
	// 获取CAN状态,若为故障状态则重新初始化CAN端口
	__INT32 CheckCanStateOrRestart(const CANCHANNEL &CanChannel, CAN_STATE *pCanState);
    // 广播 CAN总线流量信息
    void BroadcastCANInfo();
    //  知会上位机总线节点状态
    void NotifyCANInfoToSrv();
	// 初始化总线设备状态列表
	void InitDevCanStateList(void);
    // 获取本总线设备路由
    const CANCHANNEL GetLocalBusRoute(const __UINT16 &Addr);
    // 获取跨总线设备路由
    const CANCHANNEL GetCrossBusRoute();
    // 计数数据包的帧数
    const __UINT32 CalcPackFrames(const __UINT32 &nSize);
    
	// HT配置协议
    // 扫描应答
    void OnScanHTResp(LogicMsg *pMsg);
    // HT配置
    void OnSetHTCFG(LogicMsg *pMsg);
    // 设置HT MAC地址
    void OnSetMacAddr(LogicMsg *pMsg);
private:
    // 本地CAN总线号
    __INT32			m_nLocalCAN_NO;
    // 总线检测心跳时间
    __INT32         m_dwCheckCANTime;
    // 设备检测心跳时间
    __INT32         m_dwCheckDEVTime;
    // 定时上报状态时间间隔
    __INT32         m_dwNotifyTime;

    IMainInterface *m_pMainServer;
    // CAN接口
    IIOLayer       *m_pCAN1;
    IIOLayer       *m_pCAN2;
    IIOLayer       *m_pCAN3;
    IIOLayer       *m_pCAN4;
    // HT服务集群
    IIOLayer       *m_pIOSRV;
    // 配置接口(UDP)
    IIOLayer       *m_pIOCFG;
    // 接口索引
    __UINT32       m_dwIndex_IOSRVIF;    // HT 集群服务接口索引
    __UINT32       m_dwIndex_CFG_WEB;    // web配置客户端接口索引
    __UINT32       m_dwIndex_CFG_BR;     // 配置广播接口 
    // 基础定时器
    __UINT32       m_dwTimerID;
	// HT的CAN状态
	HT_CAN_STAT		m_HTCanStat; 
	// 总线设备CAN状态
	DEV_CAN_STAT  	m_DevCanState[MAX_DEV_NUM];
};

