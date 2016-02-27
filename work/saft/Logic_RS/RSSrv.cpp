#include "RSSrv.h"
#include <stdlib.h>  // for system
#include "../comm/sys/iniaccess.h"
#include "../comm/sys/log.h"
#include "../comm/frame/timerhelper.h"
#include "../protocol/CANProtocol.h"
#include "../protocol/HTCfgProtocol.h"
#include "version.h"

using namespace ST_SYS;

#define  MAX_CFG_DATA_LEN  (1024)

// 解码 CAN地址与设备地址
#define DECODE_EQUIP_ADDR(X)    ((X)&0x7f)     // 低7bit
#define DECODE_CAN_ADDR(X)  ((X>>7)&0x7f)  // 高7bit

// 编码CAN地址
#define CODE_HT_CAN_ADDR(X,Y)    (((X&0x7f)<<7) | (Y&0x7f))
// 命令字转换  使用网络字节序注册
#define HT_COMMAND(H,L)            (((L)<<8)+(H))  

// 默认CAN总线号
#define DEF_CAN_NUM  (1)

CHTSrv::CHTSrv( void ):m_dwCheckCANTime(MAX_CAN_CHECK_TIME)
    ,m_dwCheckDEVTime(MAX_DEV_CHECK_TIME)
    ,m_dwNotifyTime(MAX_NOTIFY_TIME)
{ 
    INIT_MOUDLE_INFO("saftop","HTSrv")
#ifdef _DEBUG
        ST_LOG_LEVEL(LOG_DEBUG);
#endif
    m_pCAN1 = NULL;
    m_pCAN2 = NULL;
    m_pCAN3 = NULL;
    m_pCAN4 = NULL;
    m_pIOSRV = NULL;
    m_pIOCFG = NULL;
    m_dwIndex_IOSRVIF = INVALID_ID;
    m_dwTimerID = INVALID_ID;
    m_dwIndex_CFG_WEB = INVALID_ID;
    m_dwIndex_CFG_BR = INVALID_ID;
}

CHTSrv::~CHTSrv( void )
{

}

int CHTSrv::Init( void *pMainServer )
{
    m_pMainServer = (IMainInterface*)pMainServer;
    // 加载配置
    CIniFile f_ini("ht.ini");

    // 读取本地CAN总线号，默认为1号总线
    f_ini.get_param_int("CAN", m_nLocalCAN_NO, DEF_CAN_NUM);
    f_ini.get_param_int("CHECKCANTIME", m_dwCheckCANTime, MAX_CAN_CHECK_TIME);
    f_ini.get_param_int("CHECKDEVTIME", m_dwCheckDEVTime, MAX_DEV_CHECK_TIME);
    f_ini.get_param_int("NOTIFYTIME", m_dwNotifyTime, MAX_NOTIFY_TIME);
    
    
    // 初始化总线设备CAN状态列表

    // CAN1参数设置
    __INT8 szCANName[MAX_STR_LEN];
    memset(szCANName,0,MAX_STR_LEN);
    f_ini.get_param("CAN1",szCANName); 

    // 启动CAN1 IO 
    m_pCAN1 = (IIOLayer*)m_pMainServer->GetInstanceByName("IO_485_1");
    if( m_pCAN1 )
    {
	ST_LOG(LOG_DEBUG,"open Serial...\n");    	
        m_pCAN1->Open(szCANName);
    }

#if 1// open other serial  
    // CAN2参数设置
    memset(szCANName,0,MAX_STR_LEN);
    f_ini.get_param("CAN2",szCANName); 
    // 启动CAN2 IO 
    m_pCAN2 = (IIOLayer*)m_pMainServer->GetInstanceByName("IO_485_2");
    if( m_pCAN2 )
    {
        m_pCAN2->Open(szCANName);
    }

    // CAN3参数设置
    memset(szCANName,0,MAX_STR_LEN);
    f_ini.get_param("CAN3",szCANName); 
    // 启动CAN3 IO 
    m_pCAN3 = (IIOLayer*)m_pMainServer->GetInstanceByName("IO_485_3");
    if( m_pCAN3 )
    {
        m_pCAN3->Open(szCANName);
    }

    // CAN4参数设置
    memset(szCANName,0,MAX_STR_LEN);
    f_ini.get_param("CAN4",szCANName); 
    // 启动CAN4 IO 
    m_pCAN4 = (IIOLayer*)m_pMainServer->GetInstanceByName("IO_485_4");
    if( m_pCAN4 )
    {
        m_pCAN4->Open(szCANName);
    }
#endif 
    // SRV参数设置
#if 0 
    __INT8 szRemoteAddr[MAX_STR_LEN];
    memset(szRemoteAddr,0,MAX_STR_LEN);

    __INT8 szTmp[MAX_STR_LEN];
    memset(szTmp,0,MAX_STR_LEN);
    if( !f_ini.get_param("REMOTEADDR",szTmp))
    {
        ST_LOG(LOG_ERROR,"Read CFG [REMOTEADDR] error.\n");
        return false;
    }
    int dwRemotePort = 0;
    f_ini.get_param_int("REMOTEPORT",dwRemotePort,9001); 

    // 远程服务端地址
    _SNPRINTF_(szRemoteAddr,MAX_STR_LEN,"%s:%d",szTmp,dwRemotePort);


    // 本地监听端口,使用UDP模式时的本地监听端口
    __INT8 szLocalPort[MAX_STR_LEN];
    memset(szLocalPort,0,MAX_STR_LEN);
    f_ini.get_param("LOCALPORT",szLocalPort); 

    // 启动SRV IO
    m_pIOSRV = (IIOLayer*)m_pMainServer->GetInstanceByName("IO_SRV");
    if( m_pIOSRV )
    {
        // UDP模式时，需要配置本地监听端口，TCP模式时无需配置
        if( szLocalPort[0] )
        {
            m_pIOSRV->Open(szLocalPort);
        }
        // UDP时为接口索引，TCP模式时为INVALID_ID
        m_dwIndex_IOSRVIF= m_pIOSRV->Open(szRemoteAddr);

    }

    // 启动网关IO
    m_pIOCFG = (IIOLayer*)m_pMainServer->GetInstanceByName("IO_GW");
    if( m_pIOCFG )
    {
        m_pIOCFG->Open("2123");
        m_dwIndex_CFG_WEB = m_pIOCFG->Open("127.0.0.1:6666");
    }

    // 启动逻辑线程
    Start();

    // 启动一个定时器
    m_dwTimerID = SetTimer(m_pMainServer,1000,MAKE_TIMER_CALLBACK<CHTSrv>(this,&CHTSrv::OnTimer));
    f_ini.set_param("VERSION",this->m_Info.szVer);
#endif 
    return true;
}

void CHTSrv::Fini()
{
    WaitForExit();
}

long CHTSrv::OnHandle( const LogicMsg *pMsg,void *Para /*= NULL*/ )
{
    if(pMsg->type == MSG_TYPE_DATA) // 业务数据消息异步处理
    {
        return  SendMessage(pMsg);
    }
    else if(pMsg->type == MSG_TYPE_SYS) // 接口数据同步处理,只有TCP模式才会触发该回调
    {
        IO_INFO *pInfo = (IO_INFO*)pMsg->pData;
        if( pInfo->stat == IO_STATE_ESTABLISHED )
        {
            OnConnect(pMsg->nIFIndex,pMsg->pData);
        }
        else if( pInfo->stat == IO_STATE_DISCONNECTED )
        {
            OnDisConnect(pMsg->nIFIndex,pMsg->pData);
        }
    }
    return true;
}

void CHTSrv::OnMessage( LogicMsg *pMsg )
{
    IBasicInterface *pInfo = (IBasicInterface*)pMsg->pIO;
    ST_LOG(LOG_DEBUG,"[HTSrv] OnMessage:%s MoudleIndex:%d IF:%d Len:%d.\n",pInfo->GetBaseInfo()->szName,pInfo->GetIndex(),pMsg->nIFIndex,pMsg->nLen);

    HT_HEAD *pHead = (HT_HEAD*)pMsg->pData;

    if( pMsg->pIO == m_pIOSRV ) // 收到SRV的消息，转发给本地CAN 总线
    {
        __UINT8 CanDevAddr = DECODE_EQUIP_ADDR(pHead->nDest);
        // 特殊处理广播指令
        // 解决当HT 双路CAN正常，但是控制器某路CAN异常，因为负载均衡算法引起无法扫描到控制器
        if( CANBRAODCASTADDR ==CanDevAddr 
            && m_HTCanStat.CAN_STAT[CAN1_CHANNEL].stat < CAN_STATE_ERROR_PASSIVE 
            && m_HTCanStat.CAN_STAT[CAN2_CHANNEL].stat < CAN_STATE_ERROR_PASSIVE )
        {
            TransDataToCAN(pMsg->pData,pMsg->nLen,CAN1_CHANNEL,CanDevAddr);
            TransDataToCAN(pMsg->pData,pMsg->nLen,CAN2_CHANNEL,CanDevAddr);
            return;
        }

        // 其它业务数据处理
        // 获取CAN路由
        CANCHANNEL Channel = GetLocalBusRoute(CanDevAddr);
        TransDataToCAN(pMsg->pData,pMsg->nLen,Channel,CanDevAddr);
    }
    else if( (pMsg->pIO == m_pCAN1) || (pMsg->pIO == m_pCAN2) ) // 收到本地CAN的消息，转发给SRV
    {
        // 更新总线设备状态表
        UpdateDevCanState(pMsg);

        // 负载均衡广播消息不需要处理
        if( pHead->nFC == HT_COMMAND('B','R') )
        {
            return;
        }

        // 目标地址为PC机地址(控制器回应答会将目的地址与源地址互换)
        if(LOCAL_PC_ADDR == pHead->nDest)
        {
            // 转发给集群
            m_pIOSRV->Write(pMsg->pData,pMsg->nLen,m_dwIndex_IOSRVIF);
        }
        else
        {   // 跨CAN总线联动机制:
            // 第1种情况为目标地址不为本总线号并且不是广播地址
            // 第2种情况为目标地址为广播地址
            __UINT8 CanBusNUM = DECODE_CAN_ADDR(pHead->nDest);
            if( ((CanBusNUM != m_nLocalCAN_NO) && (pHead->nDest != CANBRAODCASTADDR))
                || (CANBRAODCASTADDR == pHead->nDest) )
            {   
                CANCHANNEL Channel = GetCrossBusRoute();
                TransDataToCAN(pMsg->pData,pMsg->nLen,Channel);
            }
        }

    }
    else if( (pMsg->pIO == m_pCAN3) || (pMsg->pIO == m_pCAN4)) // 收到CAN3、CAN4总线消息
    {	
        // 更新总线设备状态表
        UpdateDevCanState(pMsg);
        // 负载均衡广播消息不需要处理
        if( pHead->nFC == HT_COMMAND('B','R') )
        {
            return;
        }

        __UINT8 CanBusNUM =   DECODE_CAN_ADDR(pHead->nDest);
        __UINT8 CanDevAddr =   DECODE_EQUIP_ADDR(pHead->nDest);
        //  目标地址为本总线消息或者是广播消息，则转发
        if( (CanBusNUM == m_nLocalCAN_NO) || (pHead->nDest == CANBRAODCASTADDR) )
        {
            // 获取CAN路由
            CANCHANNEL Channel = GetLocalBusRoute(CanDevAddr);
            TransDataToCAN(pMsg->pData,pMsg->nLen,Channel,CanDevAddr);
        }
    }
    else if( pMsg->pIO == m_pIOCFG) // 配置接口
    {
        const HT_CFG_HEAD *pCFGHead = (HT_CFG_HEAD*)pMsg->pData;
        const char *pData = pMsg->pData;
        // HT扫描应答
        if(pCFGHead->cmd == CFG_COMMAND('A',0xff))
        {
            return OnScanHTResp(pMsg);
        }
        else if(pCFGHead->cmd == CFG_COMMAND('A',0x01))
        {
            return OnSetHTCFG(pMsg);
        }
        else if(pCFGHead->cmd == CFG_COMMAND('A',0x02))
        {
            return OnSetMacAddr(pMsg);
        }
    }
}

void CHTSrv::OnConnect( const int &nIndex,void *Para )
{
    IO_INFO *pInfo = (IO_INFO*)Para;
    ST_LOG(LOG_DEBUG,"[HTSrv] Connect to %s:%d success.\n",pInfo->remote_addr,pInfo->remote_port);
    m_dwIndex_IOSRVIF = nIndex;
    // 发送登陆请求
    LoginSrv();
}

void CHTSrv::OnDisConnect( const int &nIndex,void *Para )
{
    IO_INFO *pInfo = (IO_INFO*)Para;
    ST_LOG(LOG_DEBUG,"[HTSrv] DisConnect to %s:%d,error[%d].\n",pInfo->remote_addr,pInfo->remote_port,pInfo->error);
    m_dwIndex_IOSRVIF = INVALID_ID;
}

long CHTSrv::OnTimer( void *Para )
{
    TIMER_PARA *pTimer = (TIMER_PARA*)Para;
    if( m_dwTimerID == pTimer->nTimerID )
    {
        // 上报总线状态信息，上位机实时监控
        NotifyCANInfoToSrv();
        // 广播CAN 总线信息给本地CAN设备，CAN设备负载均衡
        BroadcastCANInfo();
        // 检测HT的自身CAN状态,到达时间后归0流量统计计数
        CheckHtCanState();
        // 检测总线上设备CAN状态,到达时间后归0流量统计计数
        CheckDevCanState();
    }
    return true;
}

void CHTSrv::UpdateDevCanState(const LogicMsg * pMsg)
{
    HT_HEAD *pHead = (HT_HEAD *)(pMsg->pData);
    __UINT8 nAddrIndex = 0;
    // 获取CAN设备地址
    if( LOCAL_PC_ADDR == pHead->nOrg )
    {
        // 上位机的应答消息
        nAddrIndex = DECODE_EQUIP_ADDR(pHead->nDest);
    }
    else
    {
        // 控制器主动上报消息
        nAddrIndex = DECODE_EQUIP_ADDR(pHead->nOrg);
    }

    // CAN帧数 
    __UINT16 dwPackCount =  CalcPackFrames(pMsg->nLen);
    // 更新CAN1通道上的总线设备CAN状态	
    if(pMsg->pIO == m_pCAN1)
    {
        m_DevCanState[nAddrIndex].CAN1.stat = CAN_STATE_ERROR_ACTIVE;// CAN正常状态
        m_DevCanState[nAddrIndex].CAN1.dwflow +=  dwPackCount;
        m_DevCanState[nAddrIndex].nCan1TimeCheckCount = m_dwCheckDEVTime;// 重新计时
        m_HTCanStat.CAN_STAT[CAN1_CHANNEL].dwflow += dwPackCount;
        m_HTCanStat.CAN_STAT[CAN1_CHANNEL].stat = CAN_STATE_ERROR_ACTIVE;
    }
    // 更新CAN2通道上的总线设备CAN状态
    else if(pMsg->pIO == m_pCAN2)
    {
        m_DevCanState[nAddrIndex].CAN2.stat = CAN_STATE_ERROR_ACTIVE;// CAN正常状态
        m_DevCanState[nAddrIndex].CAN2.dwflow +=  dwPackCount;
        m_DevCanState[nAddrIndex].nCan2TimeCheckCount = m_dwCheckDEVTime;// 重新计时
        m_HTCanStat.CAN_STAT[CAN2_CHANNEL].dwflow += dwPackCount;
        m_HTCanStat.CAN_STAT[CAN2_CHANNEL].stat = CAN_STATE_ERROR_ACTIVE;
    }
    else if( pMsg->pIO == m_pCAN3)
    {
        m_HTCanStat.CAN_STAT[CAN3_CHANNEL].dwflow += dwPackCount;
        m_HTCanStat.CAN_STAT[CAN3_CHANNEL].stat = CAN_STATE_ERROR_ACTIVE;
    }
    else if( pMsg->pIO == m_pCAN4)
    {
        m_HTCanStat.CAN_STAT[CAN4_CHANNEL].dwflow += dwPackCount;
        m_HTCanStat.CAN_STAT[CAN4_CHANNEL].stat = CAN_STATE_ERROR_ACTIVE;
    }
}
void CHTSrv::CheckDevCanState(void)
{
    // 遍历检测总线上所有的设备2路CAN状态
    for(size_t i=0; i < MAX_DEV_NUM; i++)
    {		
        DEV_CAN_STAT *pDev = &(m_DevCanState[i]);
        // CAN1状态检测
        if( pDev->nCan1TimeCheckCount <= 0 )
        {
            // 设置设备CAN1停止状态
            pDev->CAN1.stat = CAN_STATE_BUS_OFF;
            pDev->CAN1.dwflow = 0;
            // 设备CAN1重新定时计数
            pDev->nCan1TimeCheckCount = m_dwCheckDEVTime;
        }
        else
        {
            // 设备CAN1定时检测计数减1
            pDev->nCan1TimeCheckCount--;
        }

        // CAN2状态检测
        if( pDev->nCan2TimeCheckCount <= 0 )
        {
            // 设置设备CAN1停止状态
            pDev->CAN2.stat = CAN_STATE_BUS_OFF;
            pDev->CAN2.dwflow = 0;
            // 设备CAN1重新定时计数
            pDev->nCan2TimeCheckCount = m_dwCheckDEVTime;
        }
        else
        {
            // 设备CAN1定时检测计数减1
            pDev->nCan2TimeCheckCount--;
        }
    }
}

void CHTSrv::CheckHtCanState(void)
{
    // 定时倒计时为0，则检测HT的CAN状态
    if( m_HTCanStat.nCanTimeCheckCount <= 0 )
    {
        CAN_STATE CanState;
        for(int i = 0; i < MAX_CAN_CHANNEL; i++)
        {
            if(0 == CheckCanStateOrRestart((CANCHANNEL)i, &CanState))
            {
                m_HTCanStat.CAN_STAT[i].stat = CanState;
            }
            else
            { 
                m_HTCanStat.CAN_STAT[i].stat = CAN_STATE_STOPPED;
            }
            m_HTCanStat.CAN_STAT[i].dwflow = 0;
        }
        // 重新定时计数
        m_HTCanStat.nCanTimeCheckCount = m_dwCheckCANTime;
    }
    else
    {
        m_HTCanStat.nCanTimeCheckCount--;
    }
}

// 获取本身CAN状态
__INT32 CHTSrv::CheckCanStateOrRestart(const CANCHANNEL &CanChannel, CAN_STATE *pCanState)
{
    __INT8 CanName[MAX_STR_LEN];
    memset(CanName, 0, sizeof(CanName));
    if( CanChannel < MAX_CAN_CHANNEL )
    {
        _SNPRINTF_(CanName,MAX_STR_LEN, "can%d", CanChannel);
        // 获取CAN总线状态
#ifndef _WIN32
        int can_stat = can_get_state(CanName, (int *)pCanState);// libsocketcan静态库函数接口
        // 已经在 ifconfig中设置自动重新启动，不需要再手动启动了
        //         // 总线关闭则重启
        //         if( *pCanState >= CAN_STATE_BUS_OFF  )
        //         {
        //             can_do_restart(CanName);// libsocketcan静态库函数接口
        //         }
        return can_stat;
#else
        return -1;
#endif
    }
    else
    {
        return -1;
    }
}


// 初始化总线CAN设备状态列表
void CHTSrv::InitDevCanStateList(void)
{
    // 初始化HT自身CAN状态
    for( size_t i = 0; i < MAX_CAN_CHANNEL; i ++)
    {
        m_HTCanStat.CAN_STAT[i].stat = CAN_STATE_ERROR_ACTIVE;
        m_HTCanStat.CAN_STAT[i].dwflow = 0;
    }
    m_HTCanStat.nCanTimeCheckCount = 0;

    // 初始化总线设备CAN状态列表
    for(size_t i=0; i<MAX_DEV_NUM; i++)
    {   
        DEV_CAN_STAT *pDev = &(m_DevCanState[i]);
        pDev->CAN1.stat = CAN_STATE_ERROR_ACTIVE;
        pDev->CAN1.dwflow =0;
        pDev->CAN2.stat = CAN_STATE_ERROR_ACTIVE;
        pDev->CAN2.dwflow =0;
        pDev->nCan1TimeCheckCount = 0;
        pDev->nCan2TimeCheckCount = 0;
    }
}

const CANCHANNEL CHTSrv::GetLocalBusRoute( const __UINT16 &Addr )
{
    if( Addr < MAX_DEV_NUM )
    {
        if( m_HTCanStat.CAN_STAT[CAN1_CHANNEL].stat < CAN_STATE_ERROR_PASSIVE 
            && m_HTCanStat.CAN_STAT[CAN2_CHANNEL].stat < CAN_STATE_ERROR_PASSIVE )
        { // 2路物理CAN线路正常

            if( CANBRAODCASTADDR == Addr )
            {  // 广播消息

                return m_HTCanStat.CAN_STAT[CAN1_CHANNEL].dwflow <= m_HTCanStat.CAN_STAT[CAN2_CHANNEL].dwflow ? CAN1_CHANNEL:CAN2_CHANNEL;
            }
            else // 非广播
            {
                // 双路CAN都正常,优先往流量小的总线发数据
                if( m_DevCanState[(Addr&CAN_ADDR_MASK)].CAN1.stat <= CAN_STATE_ERROR_PASSIVE 
                    && m_DevCanState[(Addr&CAN_ADDR_MASK)].CAN2.stat <= CAN_STATE_ERROR_PASSIVE)
                {
                    return m_HTCanStat.CAN_STAT[CAN1_CHANNEL].dwflow > m_HTCanStat.CAN_STAT[CAN2_CHANNEL].dwflow ? CAN2_CHANNEL:CAN1_CHANNEL;
                }
                else if( m_DevCanState[(Addr&CAN_ADDR_MASK)].CAN1.stat <= CAN_STATE_ERROR_PASSIVE )
                {
                    return CAN1_CHANNEL;
                }
                else if( m_DevCanState[(Addr&CAN_ADDR_MASK)].CAN2.stat <= CAN_STATE_ERROR_PASSIVE )
                {
                    return CAN2_CHANNEL;
                }
            }
        }
        else // 有某一路CAN总线故障
        {
            return m_HTCanStat.CAN_STAT[CAN1_CHANNEL].stat <= m_HTCanStat.CAN_STAT[CAN2_CHANNEL].stat ? CAN1_CHANNEL:CAN2_CHANNEL;
        }
    }
    return CAN1_CHANNEL;
}

const CANCHANNEL CHTSrv::GetCrossBusRoute()
{
    if( m_HTCanStat.CAN_STAT[CAN3_CHANNEL].stat < CAN_STATE_ERROR_PASSIVE 
        && m_HTCanStat.CAN_STAT[CAN4_CHANNEL].stat < CAN_STATE_ERROR_PASSIVE )
    { // 物理2路CAN线路正常,往流量小的总线发数据
        return m_HTCanStat.CAN_STAT[CAN3_CHANNEL].dwflow > m_HTCanStat.CAN_STAT[CAN4_CHANNEL].dwflow ? CAN4_CHANNEL:CAN3_CHANNEL;
    }
    // 有某一路总线坏了
    if(  m_HTCanStat.CAN_STAT[CAN3_CHANNEL].stat <= m_HTCanStat.CAN_STAT[CAN4_CHANNEL].stat )
    {
        return CAN3_CHANNEL;
    }
    else
    {
        return CAN4_CHANNEL;
    }
}

void CHTSrv::TransDataToCAN(const char *data,const __UINT32 &dwLen,const CANCHANNEL &ch,const __UINT16 &Addr)
{
    if( ch < MAX_CAN_CHANNEL )
    {
        // 计算帧数
        __UINT32 dwPackCount =  CalcPackFrames(dwLen);
        m_HTCanStat.CAN_STAT[ch].dwflow +=dwPackCount;

        // 如果是CAN1, CAN2,则需要同步更新本总线设备的流量计数
        if( ch == CAN1_CHANNEL )
        {
            m_DevCanState[Addr&CAN_ADDR_MASK].CAN1.dwflow += dwPackCount;
            if( m_pCAN1 )
            {
                m_pCAN1->Write(data,dwLen);
            }

        }
        else if( ch == CAN2_CHANNEL )
        {
            m_DevCanState[Addr&CAN_ADDR_MASK].CAN2.dwflow += dwPackCount;
            if( m_pCAN2 )
            {
                m_pCAN2->Write(data,dwLen);
            }

        }
        else if(ch == CAN3_CHANNEL && m_pCAN3)
        {
            m_pCAN3->Write(data,dwLen);
        }
        else if(ch == CAN4_CHANNEL && m_pCAN4)
        {
            m_pCAN4->Write(data,dwLen);
        }
    }
}

void CHTSrv::BroadcastCANInfo()
{ 
    if( m_HTCanStat.nCanTimeCheckCount <= 0 )
    {
        CMD_BROADCAST_CAN_INFO SendData;
        SendData.head.nDest = CANBRAODCASTADDR;
        SendData.head.nOrg  = LOCAL_HT_ADDR;// 111
        SendData.head.nFC = HT_COMMAND('B','R');
        SendData.flow1 = m_HTCanStat.CAN_STAT[CAN1_CHANNEL].dwflow/m_dwCheckCANTime+1; // 窗口流速
        SendData.flow2 = m_HTCanStat.CAN_STAT[CAN2_CHANNEL].dwflow/m_dwCheckCANTime+1; // 窗口流速
        CODE_HT_DATA(SendData);
        TransDataToCAN((char*)&SendData,sizeof(SendData),CAN1_CHANNEL,CANBRAODCASTADDR);
        TransDataToCAN((char*)&SendData,sizeof(SendData),CAN2_CHANNEL,CANBRAODCASTADDR);
        TransDataToCAN((char*)&SendData,sizeof(SendData),CAN3_CHANNEL,CANBRAODCASTADDR);
        TransDataToCAN((char*)&SendData,sizeof(SendData),CAN4_CHANNEL,CANBRAODCASTADDR);
    }
}

void CHTSrv::NotifyCANInfoToSrv()
{
    static int nCount = 0;
    if( nCount++ > m_dwNotifyTime )
    {
        nCount = 0;
        __UINT32 dwWindows = m_dwCheckCANTime > m_HTCanStat.nCanTimeCheckCount ? (m_dwCheckCANTime - m_HTCanStat.nCanTimeCheckCount+1):1;
        CMD_NOTIFY_CAN_INFO SendData;
        SendData.head.nDest = LOCAL_PC_ADDR;
        SendData.head.nOrg  = CODE_HT_CAN_ADDR(m_nLocalCAN_NO,LOCAL_HT_ADDR);
        SendData.head.nFC = HT_COMMAND('Y','Y');
        SendData.CAN[CAN1_CHANNEL] = m_HTCanStat.CAN_STAT[CAN1_CHANNEL].stat;
        SendData.CAN[CAN2_CHANNEL] = m_HTCanStat.CAN_STAT[CAN2_CHANNEL].stat;
        SendData.CAN[CAN3_CHANNEL] = m_HTCanStat.CAN_STAT[CAN3_CHANNEL].stat;
        SendData.CAN[CAN4_CHANNEL] = m_HTCanStat.CAN_STAT[CAN4_CHANNEL].stat;
        SendData.flow[CAN1_CHANNEL] = m_HTCanStat.CAN_STAT[CAN1_CHANNEL].dwflow/dwWindows;
        SendData.flow[CAN2_CHANNEL] = m_HTCanStat.CAN_STAT[CAN2_CHANNEL].dwflow/dwWindows;
        SendData.flow[CAN3_CHANNEL] = m_HTCanStat.CAN_STAT[CAN3_CHANNEL].dwflow/dwWindows;
        SendData.flow[CAN4_CHANNEL] = m_HTCanStat.CAN_STAT[CAN4_CHANNEL].dwflow/dwWindows;
        for(int i = 1; i < LOCAL_HT_ADDR ; i++)
        {
            __UINT8 stat = 0;
            if( m_DevCanState[i].CAN1.stat <= CAN_STATE_ERROR_PASSIVE )
            {
                stat |= 0x01;
            }
            if( m_DevCanState[i].CAN2.stat <= CAN_STATE_ERROR_PASSIVE )
            {
                stat |= 0x02;
            }
            SendData.DEV[i-1] = stat;
        }
        CODE_HT_DATA(SendData);

        if( m_pIOSRV )
        {
            m_pIOSRV->Write((char*)&SendData,sizeof(SendData),m_dwIndex_IOSRVIF);
        }

        if( m_pIOCFG )
        {
            m_pIOCFG->Write((char*)&SendData,sizeof(SendData),m_dwIndex_CFG_WEB);
        }
    }

}

const __UINT32 CHTSrv::CalcPackFrames( const __UINT32 &nSize )
{
    // 计算帧数
    if( nSize < (sizeof(HT_HEAD)+sizeof(CRC)) ) // 非法数据包
    {
        return 0;
    }

    __INT32 dwCount = (nSize - 4) / 7;
    if( dwCount )
    {
        return (((nSize - 4) % 7) > 0) ? (dwCount+1):dwCount;
    }
    else
    {
        return 1;
    }
    return 0;
}

void CHTSrv::OnScanHTResp(LogicMsg *pMsg)
{
    CIniFile f_ini("ht.ini");
    HT_SCAN_RESP resp;
    memset((char*)&resp,0,sizeof(resp));
    resp.head.cmd = CFG_COMMAND('A',0xff);

    char szTmp[MAX_STR_LEN];
    memset(szTmp,0,MAX_STR_LEN);
    f_ini.get_param("SYS_MAC",szTmp); 
    string strMac =string(szTmp);
    strMac =   _Replace(strMac,":","",true);
    resp.macaddr[0] = _HexToDec((strMac.substr(0,2)).c_str());
    resp.macaddr[1] = _HexToDec((strMac.substr(2,2)).c_str());
    resp.macaddr[2] = _HexToDec((strMac.substr(4,2)).c_str());
    resp.macaddr[3] = _HexToDec((strMac.substr(6,2)).c_str());
    resp.macaddr[4] = _HexToDec((strMac.substr(8,2)).c_str());
    resp.macaddr[5] = _HexToDec((strMac.substr(10,2)).c_str());
    memset(szTmp,0,MAX_STR_LEN);
    f_ini.get_param("SYS_IP",szTmp); 
    memcpy(resp.ipaddr,szTmp,MIN(strlen(szTmp),size_t(15)));
    memset(szTmp,0,MAX_STR_LEN);
    f_ini.get_param("SYS_MASK",szTmp); 
    memcpy(resp.maskaddr,szTmp,MIN(strlen(szTmp),size_t(15)));
    memset(szTmp,0,MAX_STR_LEN);
    f_ini.get_param("SYS_GW",szTmp); 
    memcpy(resp.gwaddr,szTmp,MIN(strlen(szTmp),size_t(15)));
    memset(szTmp,0,MAX_STR_LEN);
    f_ini.get_param("REMOTEADDR",szTmp); 
    memcpy(resp.remoteaddr,szTmp,MIN(strlen(szTmp),size_t(15)));
    memset(szTmp,0,MAX_STR_LEN);
    f_ini.get_param("SYS_IP",szTmp); 
    memcpy(resp.ipaddr,szTmp,MIN(strlen(szTmp),size_t(15)));
    memset(szTmp,0,MAX_STR_LEN);
    f_ini.get_param("NAME",szTmp); 
    memcpy(resp.name,szTmp,MIN(strlen(szTmp),size_t(20)));
    resp.version = 9;
    resp.type = 0;
    int nPort = 0;
    f_ini.get_param_int("LOCALPORT",nPort,0); 
    resp.localport = nPort;
    f_ini.get_param_int("REMOTEPORT",nPort,0); 
    resp.remoteport = nPort;
    resp.can = m_nLocalCAN_NO;
    if( m_pIOCFG )
    {
        m_pIOCFG->Write((char*)&resp,sizeof(resp),pMsg->nIFIndex);
    }
}

void CHTSrv::OnSetHTCFG( LogicMsg *pMsg )
{
    HT_CFG *pData = (HT_CFG*)pMsg->pData;
    CIniFile f_ini("ht.ini");

    char szOldMac[MAX_STR_LEN];
    memset(szOldMac,0,MAX_STR_LEN);
    f_ini.get_param("SYS_MAC",szOldMac);

    char szMac[MAX_STR_LEN];
    memset(szMac,0,MAX_STR_LEN);
    _SNPRINTF_(szMac,MAX_STR_LEN-1,"%02X:%02X:%02X:%02X:%02X:%02X",\
        pData->macaddr[0],pData->macaddr[1],pData->macaddr[2],\
        pData->macaddr[3],pData->macaddr[4],pData->macaddr[5]\
        );

    // Mac地址不匹配
    if(memcmp(szMac,_Trim(szOldMac).c_str(),strlen(szMac)) != 0)
    {
        return;
    }

    f_ini.set_param("SYS_IP",(char*)(string((char*)&pData->ip[0],15)).c_str());
    f_ini.set_param("SYS_MASK",(char*)(string((char*)&pData->maskaddr[0],15)).c_str());
    f_ini.set_param("SYS_GW",(char*)(string((char*)&pData->gwaddr[0],15)).c_str());
    f_ini.set_param("NAME",(char*)(string((char*)&pData->name[0],20)).c_str());
    f_ini.set_param("REMOTEADDR",(char*)(string((char*)&pData->remoteaddr[0],15)).c_str());
    f_ini.set_param_int("LOCALPORT",(int)pData->localport);
    f_ini.set_param_int("REMOTEPORT",(int)pData->remoteport);
    f_ini.set_param_int("CAN",(int)pData->can);
    // 更新系统IP信息
    WRITE_FILE("/etc/sysconfig/network-scripts/ifcfg-eth0","#!/bin/sh\nifconfig eth0 %s up netmask %s\nroute add default gw %s eth0\n",\
        (char*)(string((char*)&pData->ip[0],15)).c_str(),(char*)(string((char*)&pData->maskaddr[0],15)).c_str(),\
        (char*)(string((char*)&pData->gwaddr[0],15)).c_str()
        );
    ST_LOG(LOG_INFO,"Set HT config.\n");
    system("reboot");
}

void CHTSrv::OnSetMacAddr( LogicMsg *pMsg )
{
    HT_CFG_MAC *pData = (HT_CFG_MAC*)pMsg->pData;

  
    CIniFile f_ini("ht.ini");

    char szOldMac[MAX_STR_LEN];
    memset(szOldMac,0,MAX_STR_LEN);
    f_ini.get_param("SYS_MAC",szOldMac);

    char szMac[MAX_STR_LEN];
    memset(szMac,0,MAX_STR_LEN);
    _SNPRINTF_(szMac,MAX_STR_LEN-1,"%02X:%02X:%02X:%02X:%02X:%02X",\
        pData->oldmac[0],pData->oldmac[1],pData->oldmac[2],\
        pData->oldmac[3],pData->oldmac[4],pData->oldmac[5]\
        );

    // Mac地址不匹配
    if(memcmp(szMac,_Trim(szOldMac).c_str(),strlen(szMac)) != 0)
    {
        return;
    }

    memset(szMac,0,MAX_STR_LEN);
    _SNPRINTF_(szMac,MAX_STR_LEN-1,"%02X:%02X:%02X:%02X:%02X:%02X",\
        pData->mac[0],pData->mac[1],pData->mac[2],\
        pData->mac[3],pData->mac[4],pData->mac[5]\
        );

    f_ini.set_param("SYS_MAC",szMac);
    // 更新系统IP信息
    WRITE_FILE("/etc/sysconfig/network-scripts/ifcfg-eth0-hw","#!/bin/sh\nifconfig eth0 down\nifconfig eth0 hw ether %s\nifconfig eth0 up\n",szMac);
    ST_LOG(LOG_INFO,"Set HT mac addr.\n");
    system("reboot");
}

void CHTSrv::LoginSrv()
{
    CMD_HT_LOGIN SendData;
    SendData.head.nDest = LOCAL_PC_ADDR;
    SendData.head.nOrg  = CODE_HT_CAN_ADDR(m_nLocalCAN_NO,LOCAL_HT_ADDR);
    SendData.head.nFC = HT_COMMAND('Y','E');
    SendData.CANID = m_nLocalCAN_NO;
    CODE_HT_DATA(SendData);
    if( m_pIOSRV )
    {
        m_pIOSRV->Write((char*)&SendData,sizeof(SendData),m_dwIndex_IOSRVIF);
    }
}


CREATEINSTANCE(CHTSrv)
