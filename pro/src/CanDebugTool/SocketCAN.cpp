#include "SocketCAN.h"
#include "SaftopCAN.h"
#ifndef WIN32  // only support linux
#include "can.h"
#include "log.h"
#include "error.h"
#include <sys/ioctl.h>
//#include <linux/if.h>
#include <net/if.h>

using namespace ST_SYS;

#ifndef INVALID_FD
#define INVALID_FD  (-1)
#endif


#define CAN_FRAME_DATA_SIZE    (8)

#define DEFAULT_CAN_INDEX    (0)
const int CAN_FRAME_SIZE = sizeof(struct can_frame);
    const int NO_LEN = 0;
    const int ERR_DATA = -1;
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

// 最大发送次数
const __UINT32 MAX_SEND_COUNT = 50;

CSocketCAN::CSocketCAN(void):m_FD(INVALID_FD),m_bActive(false)
{
#ifdef _DEBUG
    ST_LOG_LEVEL(LOG_DEBUG);
#endif
    memset(m_szName,0,MAX_CAN_NAME);
}


CSocketCAN::~CSocketCAN(void)
{
}

int CSocketCAN::Init( void *pMainServer )
{
#if  0 
    m_pMainServer = (IMainInterface*)pMainServer;
    m_pCoder = m_pMainServer->GetCoderByIO(GetIndex());
    pLogic = m_pMainServer->GetLogicsByIO(GetIndex(),m_nLogicCount);
#endif 

    //CSaftopCAN saftopcan;
    printf("11\n");
    m_pCoder = new CSaftopCAN();
    printf("22\n");
    Start();
    return TRUE;
}

void CSocketCAN::Fini()
{
    m_Exit.Notify();
    int ret = Join(2000);
}

// CAN数据包发送，对于数据长度超过8字节的数据，分包发送
int CSocketCAN::Write( const char *pData,const int &nLen, const int &index /*= 0*/ )
{
    if( INVALID_FD != m_FD )
    {
        int nDstLen = MAX_CAN_BUF_LEN;
        char DstBuf[MAX_CAN_BUF_LEN];
        // 加密成功
        if ( GetCoder()->Code((char*)pData,nLen,DstBuf,nDstLen) )
        {
            if( 0 == ( nDstLen % sizeof(can_frame)) ) // 编码后待发送的数据一定得为can_frame的整数倍
            {
                int nCount = 0;
                int nSendCount = 0;
                do 
                {
                    int ret = SendMsg(DstBuf+nCount,sizeof(can_frame));
                    if( ret == sizeof(can_frame) )
                    {
                         nCount += sizeof(can_frame);
                    }
                    else
                    {   // 解决发送太快,底层缓存不够，丢包问题
                        if( nSendCount++ > MAX_SEND_COUNT)
                        {
                            ST_LOG(LOG_DEBUG,"CAN[%s] send data error[%d].\n",m_szName,errno);
                            return false;
                        }
                        SLEEP(10);
                    }
                   
                } while (nCount < nDstLen);
                return nCount;
            }
        }
    }
    return  false;
}

int CSocketCAN::Open( const char *args )
{
    printf("open now\n");
    if( args )
    {
        int len = min(strlen(args),MAX_CAN_NAME);
        memcpy(m_szName,args,len);
        m_szName[len] = 0;
	printf("show name:%s\n",m_szName);
        return true;
    }
    return false;
}

int CSocketCAN::Close( const char *args )
{
    if( INVALID_FD != m_FD )
    {
        close(m_FD);
        m_FD = INVALID_FD;
    }
    return true;
}

int CSocketCAN::IOCtrl( int nAction, void *pActionStruct, void *pRetData )
{
    return false;
}

void CSocketCAN::Setopts( int s, int loopback, int recv_own_msgs )
{
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_LOOPBACK,
        &loopback, sizeof(loopback));
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS,
        &recv_own_msgs, sizeof(recv_own_msgs));
}

bool CSocketCAN::InitSocketCAN()
{
    if( !m_szName[0] )
    {
        return false;
    }
    if( INVALID_FD != m_FD )
    {
        close(m_FD);
        m_FD = INVALID_FD;
    }
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_filter filter;
    can_err_mask_t err_mask =CAN_ERR_MASK; /* all */
    bool bValid =false;

    if ((m_FD = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        ST_LOG(LOG_ERROR,"[CANDebug] Create CAN FD[%d] error[%d].\n",m_FD,errno);
        return false;
    }
    ST_LOG(LOG_INFO,"[CANDebug] Create CAN FD[%d] success.\n",m_FD);

    strcpy(ifr.ifr_name, m_szName);
    ioctl(m_FD, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex; 

    if (bind(m_FD, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        ST_LOG(LOG_ERROR,"[CANDebug] Bind CAN FD[%d] error[%d].\n",m_FD,errno);
        return false;
    }

    // CAN devices down
    int nErr= errno;
    if( 19 == nErr )
    {
        ST_LOG(LOG_ERROR,"[CANDebug] Bind CAN FD[%d] error[%d],no such device.\n",m_FD,nErr);
        return false;
    }
    
    ST_LOG(LOG_INFO,"[CANDebug] Bind CAN FD[%d] success[%s %d].\n",m_FD,m_szName,nErr);
    // filter.can_id   = CAN_EFF_FLAG|CAN_RTR_FLAG|CAN_ERR_FLAG;
    // filter.can_mask =CAN_SFF_MASK | CAN_EFF_FLAG | CAN_RTR_FLAG;
    //	setsockopt(canfd, SOL_CAN_RAW, CAN_RAW_FILTER,&filter, sizeof(filter));
    setsockopt(m_FD, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));
    Setopts(m_FD,1,0);	
    return true;
}

void CSocketCAN::Run()
{
    while( WAIT_OBJECT_0 != m_Exit.WaitForObject(3000) )
    {
        m_bActive= InitSocketCAN();
        if( m_bActive )
        {
            if( DrvRun() )
            {
                close(m_FD);
                return;
            }
            close(m_FD);
        }
    }
}

bool CSocketCAN::DrvRun()
{
    while( WAIT_OBJECT_0 != m_Exit.WaitForObject(0) )
    {
        fd_set rfd;
        FD_ZERO(&rfd);

        FD_SET(m_FD,&rfd);
        struct timeval tval;
        tval.tv_sec = 0;
        tval.tv_usec = 1000;
        int n = select(m_FD+1,&rfd,0,0,&tval);
        if( 0 == n ) // time out
        {
            //ST_LOG(LOG_DEBUG,"[HTSrv] Recv CAN FD[%s] data timeout.\n",m_szName);
            continue;
        }
        else if(  n < 0 ) // error
        {
            ST_LOG(LOG_ERROR,"[CANDebug] Recv CAN FD[%s] data error[%d].\n",m_szName,errno);
            return false;
        }
        else
        {
	    
            // read 
            if(FD_ISSET(m_FD,&rfd))
            {
                struct can_frame frame;
                int nbytes = 0;
                if ((nbytes = read(m_FD, &frame, CAN_FRAME_SIZE )) < 0) 
                {
                    ST_LOG(LOG_ERROR,"[CANDebug] Read CAN FD[%s] data error[%d].\n",m_szName,errno);
                    return false;
                }

                if ( nbytes < CAN_FRAME_SIZE )
                {
		    printf("11\n");
                    ST_LOG(LOG_ERROR,"[CANDebug] Recv CAN FD[%s] data size is invalid len[%d].\n",m_szName,nbytes);
                }
                else
		{
		    //处理 标准帧 扩展帧 错误帧
		    char DstBuf[MAX_CAN_BUF_LEN];
		    int nDstLen = MAX_CAN_BUF_LEN;
		    int nOrgLen = GetCoder()->SplitPacket((char*)&frame,sizeof(frame));
		    if( (ERR_DATA != nOrgLen) && (NO_LEN != nOrgLen) )
		    {
			if ( GetCoder()->DeCode((char*)&frame,nOrgLen,DstBuf,nDstLen) )
			{ // 解密成功
			    HT_HEAD *pHead =(HT_HEAD*)DstBuf;


			    char cDstBus,cDstAddr ; 
			    char cOrgBus,cOrgAddr ; 
			    cDstBus = (pHead->nDest >>7);
			    cOrgBus = (pHead->nOrg >>7);
			    cDstAddr = pHead->nDest & 0x7f ; 
			    cOrgAddr = pHead->nOrg & 0x7f ; 

			    char cmd[2]={0};
			    cmd[0]=(pHead->nFC>>8) & 0xFF;
			    cmd[1]=pHead->nFC & 0xFF;

			    ST_LOG(LOG_DEBUG,"CAN上报数目的地址：%d-%d,源地址：%d-%d, show FC : %c%c\n",cDstBus,cDstAddr,cOrgBus,cOrgAddr,cmd[1],cmd[0]);
			    string strTmp=HexToStr((const unsigned char *)DstBuf,nDstLen);
			    printf("show vlaue:%s\n",strTmp.c_str());
		//	    OnIOHandle(DEFAULT_CAN_INDEX,DstBuf,nDstLen);
			}
			else // 解密失败或无需解密
			{
			    //printf("解码失败。。\n");
			    //OnIOHandle(DEFAULT_CAN_INDEX,(char*)&frame,nOrgLen);
			}
		    }
		    else //数据包没有收全
		    {

		    }

                }
            }
        }
    }
    return true;
}


ICoder * CSocketCAN::GetCoder()
{
     return m_pCoder;
}

void CSocketCAN::OnIOHandle( const int &IFIndex/*接口索引*/ ,const char *pData,const int &nLen,const MSG_TYPE &type /*= MSG_TYPE_DATA*/ )
{
    for(int i = 0; i < m_nLogicCount; i++)
    {
        LogicMsg msg;
        msg.type = type;
        msg.pIO = this;
        msg.nIFIndex = IFIndex;
        msg.pData = (char*)pData;
        msg.nLen = nLen;
        (pLogic[i])->OnHandle(&msg);
    }
}
// CAN数据包发送
int CSocketCAN::SendMsg( const char *pData,const int &nSize )
{
    if( INVALID_FD != m_FD && m_bActive )
    {
        return write(m_FD, (can_frame*)pData, sizeof(can_frame));
    }
    return false;
}

CREATEINSTANCE(CSocketCAN)
#endif


