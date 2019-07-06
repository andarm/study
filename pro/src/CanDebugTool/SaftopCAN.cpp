#include "SaftopCAN.h"
#include "../protocol/CANProtocol.h"
#include "../comm/sys/timetool.h"
#include "../comm/sys/log.h"

using namespace ST_SYS;
// ������ת��  ʹ�������ֽ���ע��
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
    // ��ʼ����ַ
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
    // �����������账��
    return Inlen;
}

bool CSaftopCAN::DeCode( const char *InBuf, const int &InLen,char *OutBuf,int &OutLen )
{
    CANFRAME *pFrame = (CANFRAME*)InBuf;
    if( !(pFrame->can_id & CAN_EFF_FLAG) ) // ����չ֡��ֱ�Ӷ��� 
    {
        return false;
    }
    __UINT8 nType = pFrame->data[0] >> 5; // ��ȡ֡���� 
    __UINT32 dwAddr =  pFrame->can_id&0x0fffffff;  //���͸�����֡��Դ
    if( FRAME_BEG == nType ) // ��ʼ֡
    {
        CANRXBUFF * pResult = NULL;
        do {
            pResult = SearchCANRxBuff(m_CANBUFF,dwAddr);
            if (pResult) 
            {
                // ������ͬʱ����ͬһ����������֡����
                FreeCANRxBuff(pResult);// ����֮ǰ֡
            }
        } while(pResult);

        // ȡһ���ջ�����
        pResult = GetCANRxBuff(m_CANBUFF);
        if (!pResult)
        {
            // �����������Ȼ��գ��ٷ���
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
            // ������
             ST_LOG(LOG_DEBUG,"[CAN Coder] Begin buf full.\n");
        
        }
    }
    else if( FRAME_MID == nType ) // �м�֡
    {
        CANRXBUFF * pResult = SearchCANRxBuff(m_CANBUFF,dwAddr);
        if (pResult) 
        {
            PutCANRxBuff(pResult, pFrame);
        } 
        else 
        {
            // ����֡����
             ST_LOG(LOG_DEBUG,"[CAN Coder] Mid CAN_FRAME[ID:%d] error.\n",pFrame->can_id);
        }
    }
    else if( FRAME_END == nType) // ����֡
    {
        if (pFrame->data[0] & 0x1f) // ��֡
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
                __UINT32 dwLen = min(pHead->nLen+2,(MAXRXFRAMES*FRAMESIZE)); // (+2�ֽ�CRC����)
                memcpy((char*)(OutBuf+sizeof(HT_HEAD)),(char*)(&pData[3]),dwLen);
                OutLen = sizeof(HT_HEAD) + dwLen; // ����ͷ����
                // �ͷŻ���
                FreeCANRxBuff(pResult);
                return true;
            }
            else
            {
                // ����֡
                ST_LOG(LOG_DEBUG,"[CAN Coder] End CAN_FRAME[ID:%d] error.\n",pFrame->can_id);
            }

        }
        else  // ��֡
        {
            HT_HEAD *pHead = (HT_HEAD*)OutBuf;
            pHead->nDest = (pFrame->can_id >> 14) &CANADDRESSMASK;
            pHead->nOrg =  pFrame->can_id &CANADDRESSMASK;
            pHead->nFC = HT_COMMAND(pFrame->data[1],pFrame->data[2]);
            pHead->nLen = pFrame->data[3];
            __UINT32 dwLen = min(pHead->nLen+2,4); // ���ʣ��4�ֽ������� (+2�ֽ�CRC����)
            memcpy((char*)(OutBuf+sizeof(HT_HEAD)),(char*)(&pFrame->data[4]),dwLen); 
            OutLen = sizeof(HT_HEAD)+ dwLen;  // ����ͷ����
            return true;
        }
    }
    return false;
}
// �������͵����ݱ���ΪCAN_FRAME��ʽ
bool CSaftopCAN::Code( const char *InBuf, const int &InLen,char *OutBuf,int &OutLen )
{
    // �Ƿ�����
    if( InLen < sizeof(HT_HEAD))
    {
        return false;
    }
    
    OutLen  = 0;
    HT_HEAD *pHead = (HT_HEAD*)InBuf;
    // ��ַ������CANFRAME��id�У�ʣ�������ݴ洢��CANFRAME�ṹ��data����
    const char *pData = (InBuf+sizeof(__UINT16)+sizeof(__UINT16)); 
    CANFRAME  canframe;
    __UINT8   dwIndex = 0;  // ֡����
    // IDֵԽ�ͣ��������ȼ�Խ��
    // ���ڽ���λ����ָ�����ݿ��ٷ��ͳ�ȥ
    canframe.can_id = ( (0 << 28)/*���ȼ�*/ | (pHead->nDest << 14) /*Ŀ���ַ*/| (pHead->nOrg) /*Դ��ַ*/| CAN_EFF_FLAG /*ʹ����չ֡*/);
    canframe.data[0] = ( 0<< 8);  // ��ʼ֡
    __UINT32 dwSendDataLen =  InLen - sizeof(__UINT16)-sizeof(__UINT16); // Դ��Ŀ���ַ������CAN�� ID��
    for(int i = 0; i < dwSendDataLen; )
    {
        int j = 0;
        for(j=0; (j < 7) && (i < dwSendDataLen); j++)
        {
            canframe.data[j+1] = *(pData++);
    	    i++;
        }
        canframe.can_dlc = j+1;  // +1�ֽ������� 
         if( i >= dwSendDataLen ) // ���һ֡
         {
             canframe.data[0]  = (canframe.data[0] & 0x1f) | (2<<5);
         }
        // �����
         memcpy(OutBuf+OutLen,(char*)&canframe,sizeof(CANFRAME));
         OutLen += sizeof(CANFRAME);
        // ��һ֡������������
        dwIndex++;

        canframe.data[0] = (1<<5) |(dwIndex & 0x1f);  // // �м�֡ & ����
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
    // δ�ҵ�
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
    // �޿ջ�����
    return NULL;
}

bool CSaftopCAN::PutCANRxBuff( CANRXBUFF * pbuff,CANFRAME *pframe )
{
    __UINT8  index = pframe->data[0]&0x1f;
    if (index >= MAXRXFRAMES) {
        // ��������
        // �ͷŻ�����
        ST_LOG(LOG_DEBUG,"[CAN Coder]Index error[%d] addr[%d].\n",index,pbuff->address);
        FreeCANRxBuff(pbuff);
        return false;
    }

    pbuff->OSTime = _GET_TICK_COUNT();
    pbuff->index  = index;
    memcpy(	pbuff->data[index],(char*)(&pframe->data[1]),FRAMESIZE); // ��һ�ֽ�Ϊ�ְ�����
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
                // �ͷŹ��ڵ�����֡
                ST_LOG(LOG_DEBUG,"[CAN Coder] RecCANRxBuff addr[%d].\n",pbuff->address);
                FreeCANRxBuff(pbuff);
                result = true;
        }
        ++pbuff;
    }
    return  result;
}

//CREATEINSTANCE(CSaftopCAN)
