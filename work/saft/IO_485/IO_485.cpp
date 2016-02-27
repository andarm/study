#include "IO_485.h"
#ifndef WIN32  // only support linux
//#include "can.h"
#include "log.h"
#include "error.h"
#include <sys/ioctl.h>
//#include <linux/if.h>
#include <string.h>

using namespace ST_SYS;

#ifndef INVALID_FD
#define INVALID_FD  (-1)
#endif


#define CAN_FRAME_DATA_SIZE    (256)

#define DEFAULT_CAN_INDEX    (0)
const int CAN_FRAME_SIZE = 256;
    const int NO_LEN = 0;
    const int ERR_DATA = -1;
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

// 最大发送次数
const __UINT32 MAX_SEND_COUNT = 50;

IO485::IO485(void):m_FD(INVALID_FD),m_bActive(false),m_serial(NULL)
{
#ifdef _DEBUG
    ST_LOG_LEVEL(LOG_DEBUG);
#endif
    memset(m_szName,0,MAX_CAN_NAME);
}


IO485::~IO485(void)
{

    delete m_serial;
    m_serial=NULL;
}

int IO485::Init( void *pMainServer )
{
    m_pMainServer = (IMainInterface*)pMainServer;
    m_pCoder = m_pMainServer->GetCoderByIO(GetIndex());
    pLogic = m_pMainServer->GetLogicsByIO(GetIndex(),m_nLogicCount);
   Start();
    return TRUE;
}

void IO485::Fini()
{
    m_Exit.Notify();
    int ret = Join(2000);
}

// CAN数据包发送，对于数据长度超过8字节的数据，分包发送
int IO485::Write( const char *pData,const int &nLen, const int &index /*= 0*/ )
{
    {
	ST_LOG(LOG_DEBUG,"IN Write.....Len:%d Data:%s\n",nLen,pData);
        int nDstLen = MAX_CAN_BUF_LEN;
        char DstBuf[MAX_CAN_BUF_LEN];
        // 加密成功
        if ( GetCoder()->Code((char*)pData,nLen,DstBuf,nDstLen) )
        {
	    ST_LOG(LOG_DEBUG,"send MSg...\n");
	    int ret = SendMsg(DstBuf,nDstLen);
	    
        }
    }
    return  false;
}

int IO485::Open( const char *args )
{
    ST_LOG(LOG_DEBUG,"get args from Logic :%s...\n",args);

    if( args )
    {
        int len = min(strlen(args),MAX_CAN_NAME);
        memcpy(m_szName,args,len);
        m_szName[len] = 0;
        return true;
    }
    return false;
}

int IO485::Close( const char *args )
{
    if( INVALID_FD != m_FD )
    {
        close(m_FD);
        m_FD = INVALID_FD;
    }
    return true;
}

int IO485::IOCtrl( int nAction, void *pActionStruct, void *pRetData )
{
    return false;
}

void IO485::Setopts( int s, int loopback, int recv_own_msgs )
{

}
bool IO485::InitSerial()
{
    char fname[20];
    int baud , bits ,parity , stop;
    if( !m_szName[0])
    {	
	ST_LOG(LOG_DEBUG,"m_szname error :%s...\n",m_szName);
	return false ;
    }
    if( INVALID_FD != m_FD)
    {
	ST_LOG(LOG_DEBUG,"m_FD :%d...\n",m_FD);
	close(m_FD);
	m_FD = INVALID_FD;
    }
    ST_LOG(LOG_DEBUG,"m_szname ########### :%s...\n",m_szName);
    vector<string> vt;
    _Split(m_szName,vt," ");
    strcpy(fname,vt[0].c_str());
    baud = _Atoi(vt[1].c_str());
    bits = _Atoi(vt[2].c_str());
    parity = _Atoi(vt[3].c_str());
    stop = _Atoi(vt[4].c_str());
    ST_LOG(LOG_DEBUG,"get param of serial...\n");
//    if( NULL == m_szName)
    {
	m_serial = new CSerial();
	ST_LOG(LOG_DEBUG,"log new serial ......\n");
	if(m_serial && m_serial->Init(fname,baud,bits,parity,stop) )
	{
	    ST_LOG(LOG_INFO,"[IO485 ] Init Serial port[%s %d %d %d %d] success.\r\n",fname,baud,bits,parity,stop);

	}
	else  //can not open serial 
	{
	    
	    ST_LOG(LOG_ERROR,"[IO485 ] Init Serial port[%s %d %d %d %d] error.\r\n",fname,baud,bits,parity,stop);
	}
    }

    return true ; 

}


void IO485::Run()
{
    ST_LOG(LOG_DEBUG,"lin jin ye test .....\n");
    while( WAIT_OBJECT_0 != m_Exit.WaitForObject(3000) )
    {
        m_bActive= InitSerial();
        if( m_bActive )
        {
	    ST_LOG(LOG_DEBUG,"read now :\n");
            if( DrvRun() )
            {
                return;
            }
	    ST_LOG(LOG_DEBUG,"end of FD\n");
        }
	ST_LOG(LOG_DEBUG,"test ...\n");
    }
}

bool IO485::DrvRun()
{
    char szRecBuf[1024]={0};
	ST_LOG(LOG_DEBUG,"DRv  run  ...\n");

    while( WAIT_OBJECT_0 != m_Exit.WaitForObject(0) )
    {
	int iRet = m_serial->Read(szRecBuf,1024);
	if(iRet>0)
	{
	    ST_LOG(LOG_DEBUG,"receive data :%s\n",szRecBuf);
	    //decode serial
	    // 处理帧
	    char DstBuf[MAX_CAN_BUF_LEN];
	    int nDstLen = MAX_CAN_BUF_LEN;
	    int nOrgLen =iRet;
    
	    if ( GetCoder()->DeCode((char*)&szRecBuf,nOrgLen,DstBuf,nDstLen) )
	    { // 解密成功
		OnIOHandle(DEFAULT_CAN_INDEX,DstBuf,nDstLen);
	    }
	    else // 解密失败或无需解密
	    {
		//OnIOHandle(DEFAULT_CAN_INDEX,(char*)&frame,nOrgLen);
	    }
	    ST_LOG(LOG_DEBUG,"Write data ...\n");
	    //m_serial->Write(DstBuf,nDstLen); //test send receive
//	    Write(DstBuf,nDstLen,DEFAULT_CAN_INDEX); //test send receive
	    memset(szRecBuf,0,sizeof(szRecBuf));

	}
#if 0
                {
                   if( (ERR_DATA != nOrgLen) && (NO_LEN != nOrgLen) )
                    {
            
                    }
                    else //数据包没有收全
                    {

                    }
                }
#endif 
    }
    return true;
}


ICoder * IO485::GetCoder()
{
     return m_pCoder;
}

void IO485::OnIOHandle( const int &IFIndex/*接口索引*/ ,const char *pData,const int &nLen,const MSG_TYPE &type /*= MSG_TYPE_DATA*/ )
{
#if 0 
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
#endif 
}
// CAN数据包发送
int IO485::SendMsg( const char *pData,const int &nSize )
{
    if(  m_bActive )
    {
        return m_serial->Write(pData,nSize);
    }
    return false;
}

CREATEINSTANCE(IO485)
#endif


