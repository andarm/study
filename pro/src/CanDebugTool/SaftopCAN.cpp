#include "SaftopCAN.h"
#include "../protocol/CANProtocol.h"
#include "../comm/sys/timetool.h"
#include "../comm/sys/log.h"

using namespace ST_SYS;
// 命令字转换  使用网络字节序注册
#define HT_COMMAND(H,L)            (((L)<<8)+(H))  

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))


#define CAN_EFF_FLAG (0x80000000U) /* EFF/SFF is set in the MSB */

CSaftopCAN::CSaftopCAN(void)
{
    memset(m_CANBUFF,0,sizeof(CANRXBUFF)*MAXCANRXBUFFS);
}


CSaftopCAN::~CSaftopCAN(void)
{
}

int CSaftopCAN::Init( void *pMainServer )
{
    // 初始化地址
    for(int i = 0; i < MAXCANRXBUFFS; i++)
    {
        m_CANBUFF[i].address = 0xffffffff;
    }
   return true;
}

void CSaftopCAN::Fini()
{

}

int CSaftopCAN::SplitPacket( const char *InBuf, const int &Inlen )
{
    // 定长包，无需处理
    return Inlen;
}

bool CSaftopCAN::DeCode( const char *InBuf, const int &InLen,char *OutBuf,int &OutLen )
{
    CANFRAME *pFrame = (CANFRAME*)InBuf;
    if( !(pFrame->can_id & CAN_EFF_FLAG) ) // 非扩展帧，直接丢弃 
    {
        return false;
    }
    __UINT8 nType = pFrame->data[0] >> 5; // 获取帧类型 
    __UINT32 dwAddr =  pFrame->can_id&0x0fffffff;  //发送该数据帧的源
    if( FRAME_BEG == nType ) // 起始帧
    {
        CANRXBUFF * pResult = NULL;
        do {
            pResult = SearchCANRxBuff(m_CANBUFF,dwAddr);
            if (pResult) 
            {
                // 不允许同时接收同一主机发来的帧数据
                FreeCANRxBuff(pResult);// 丢弃之前帧
            }
        } while(pResult);

        // 取一个空缓冲区
        pResult = GetCANRxBuff(m_CANBUFF);
        if (!pResult)
        {
            // 缓冲区满，先回收，再分配
            if (true == RecCANRxBuff(m_CANBUFF))
            {
                pResult = GetCANRxBuff(m_CANBUFF);
            }
        }

        if (pResult) 
        {
            pResult->used    = true;
            pResult->address = dwAddr;
            PutCANRxBuff(pResult, pFrame);
        }
        else
        {
            // 缓冲满
             ST_LOG(LOG_DEBUG,"[CAN Coder] Begin buf full.\n");
        
        }
    }
    else if( FRAME_MID == nType ) // 中间帧
    {
        CANRXBUFF * pResult = SearchCANRxBuff(m_CANBUFF,dwAddr);
        if (pResult) 
        {
            PutCANRxBuff(pResult, pFrame);
        } 
        else 
        {
            // 错误帧数据
             ST_LOG(LOG_DEBUG,"[CAN Coder] Mid CAN_FRAME[ID:%d] error.\n",pFrame->can_id);
        }
    }
    else if( FRAME_END == nType) // 结束帧
    {
        if (pFrame->data[0] & 0x1f) // 多帧
        {		
            CANRXBUFF * pResult = SearchCANRxBuff(m_CANBUFF,dwAddr);
            if (pResult)
            {
                PutCANRxBuff(pResult, pFrame);
                HT_HEAD *pHead = (HT_HEAD*)OutBuf;
                pHead->nDest = ((pResult->address) >> 14) &CANADDRESSMASK;
                pHead->nOrg =  pResult->address &CANADDRESSMASK;
                const __UINT8 *pData =  &(pResult->data[0][0]);
                pHead->nFC = HT_COMMAND(pData[0],pData[1]);
                pHead->nLen = pData[2];
                __UINT32 dwLen = min(pHead->nLen+2,(MAXRXFRAMES*FRAMESIZE)); // (+2字节CRC长度)
                memcpy((char*)(OutBuf+sizeof(HT_HEAD)),(char*)(&pData[3]),dwLen);
                OutLen = sizeof(HT_HEAD) + dwLen; // 加上头长度
                // 释放缓存
                FreeCANRxBuff(pResult);
                return true;
            }
            else
            {
                // 错误帧
                ST_LOG(LOG_DEBUG,"[CAN Coder] End CAN_FRAME[ID:%d] error.\n",pFrame->can_id);
            }

        }
        else  // 单帧
        {
            HT_HEAD *pHead = (HT_HEAD*)OutBuf;
            pHead->nDest = (pFrame->can_id >> 14) &CANADDRESSMASK;
            pHead->nOrg =  pFrame->can_id &CANADDRESSMASK;
            pHead->nFC = HT_COMMAND(pFrame->data[1],pFrame->data[2]);
            pHead->nLen = pFrame->data[3];
            __UINT32 dwLen = min(pHead->nLen+2,4); // 最多剩下4字节数据了 (+2字节CRC长度)
            memcpy((char*)(OutBuf+sizeof(HT_HEAD)),(char*)(&pFrame->data[4]),dwLen); 
            OutLen = sizeof(HT_HEAD)+ dwLen;  // 加上头长度
            return true;
        }
    }
    return false;
}
// 将待发送的数据编码为CAN_FRAME格式
bool CSaftopCAN::Code( const char *InBuf, const int &InLen,char *OutBuf,int &OutLen )
{
    // 非法数据
    if( InLen < sizeof(HT_HEAD))
    {
        return false;
    }
    
    OutLen  = 0;
    HT_HEAD *pHead = (HT_HEAD*)InBuf;
    // 地址保存在CANFRAME的id中，剩下下数据存储在CANFRAME结构的data区域
    const char *pData = (InBuf+sizeof(__UINT16)+sizeof(__UINT16)); 
    CANFRAME  canframe;
    __UINT8   dwIndex = 0;  // 帧索引
    // ID值越低，报文优先级越高
    // 便于将上位机的指令数据快速发送出去
    canframe.can_id = ( (0 << 28)/*优先级*/ | (pHead->nDest << 14) /*目标地址*/| (pHead->nOrg) /*源地址*/| CAN_EFF_FLAG /*使用扩展帧*/);
    canframe.data[0] = ( 0<< 8);  // 起始帧
    __UINT32 dwSendDataLen =  InLen - sizeof(__UINT16)-sizeof(__UINT16); // 源，目标地址保存在CAN的 ID中
    for(int i = 0; i < dwSendDataLen; )
    {
        int j = 0;
        for(j=0; (j < 7) && (i < dwSendDataLen); j++)
        {
            canframe.data[j+1] = *(pData++);
    	    i++;
        }
        canframe.can_dlc = j+1;  // +1字节类型码 
         if( i >= dwSendDataLen ) // 最后一帧
         {
             canframe.data[0]  = (canframe.data[0] & 0x1f) | (2<<5);
         }
        // 编码包
         memcpy(OutBuf+OutLen,(char*)&canframe,sizeof(CANFRAME));
         OutLen += sizeof(CANFRAME);
        // 下一帧数据索引与标记
        dwIndex++;

        canframe.data[0] = (1<<5) |(dwIndex & 0x1f);  // // 中间帧 & 索引
    }
   
    return TRUE;
}

CANRXBUFF* CSaftopCAN::SearchCANRxBuff( CANRXBUFF * pbuff,const __UINT32 &address )
{
    for (int i=0; i < MAXCANRXBUFFS; i++)
    {
        if ( (true == pbuff->used ) && (pbuff->address == address))
        {
          return pbuff;
        }
        ++pbuff;
    }
    // 未找到
    return NULL;
}

CANRXBUFF* CSaftopCAN::GetCANRxBuff( CANRXBUFF * pbuff )
{
    for (int i=0; i < MAXCANRXBUFFS; i++) 
    {
        if ( false == pbuff->used )
        {
            memset(pbuff, 0x00, sizeof(CANRXBUFF));
            return pbuff;
        }
        pbuff++;
    }
    // 无空缓冲区
    return NULL;
}

bool CSaftopCAN::PutCANRxBuff( CANRXBUFF * pbuff,CANFRAME *pframe )
{
    __UINT8  index = pframe->data[0]&0x1f;
    if (index >= MAXRXFRAMES) {
        // 索引错误
        // 释放缓冲区
        ST_LOG(LOG_DEBUG,"[CAN Coder]Index error[%d] addr[%d].\n",index,pbuff->address);
        FreeCANRxBuff(pbuff);
        return false;
    }

    pbuff->OSTime = _GET_TICK_COUNT();
    pbuff->index  = index;
    memcpy(	pbuff->data[index],(char*)(&pframe->data[1]),FRAMESIZE); // 第一字节为分包数据
    return true;
}

void CSaftopCAN::FreeCANRxBuff( CANRXBUFF * pbuff )
{
    pbuff->used = false;
}

bool CSaftopCAN::RecCANRxBuff( CANRXBUFF * pbuff )
{
    bool   result = false;
    for (int i=0; i < MAXCANRXBUFFS; i++)
    {
        if ((true == pbuff->used) &&
            ((_GET_TICK_COUNT() - pbuff->OSTime) > MAXRXSPACETIME))
        {
                // 释放过期的数据帧
                ST_LOG(LOG_DEBUG,"[CAN Coder] RecCANRxBuff addr[%d].\n",pbuff->address);
                FreeCANRxBuff(pbuff);
                result = true;
        }
        ++pbuff;
    }
    return  result;
}

//CREATEINSTANCE(CSaftopCAN)
