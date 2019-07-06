/********************************************************************
	created:	2013/12/04
	created:	4:12:2013   14:45
	filename: 	SaftopCAN.h
	file base:	SaftopCAN
	file ext:	h
	author:		liujia
	
	purpose:	Saftop CANЭ������
*********************************************************************/
#pragma once
#include "../comm/frame/interface.h"
#include "../comm/sys/type.h"

/* Saftop CANЭ��
*  =================================================================================================================================================
*  |                                �ٲ���                                   |                                  ������                        |
*  =================================================================================================================================================
*  | Bit28 |             Bit27 ~ Bit14       |          Bit13 ~ Bit0         |                Data[0]                       |   Data[1] ~ Data[7]  |
*  =================================================================================================================================================
*  |���ȼ�| Ŀ���ַ ���ߺ�(7) �豸��ַ(7) | Դ��ַ ���ߺ�(7) �豸��ַ(7) |  ������(3):0:��ʼ 1:�м� 2:����  | Index(5) |         ������       |
*  =================================================================================================================================================
*  ע:����䳤��:   224 Byte = 2^5 * 7
*/

#define  FRAME_BEG   (0)  // ��ʼ֡
#define  FRAME_MID   (1)  // �м�֡
#define  FRAME_END   (2)  // ����֡

// CAN ��ַ������
#define  CANADDRESSMASK    (0x3fff)

// һ�����ݰ����ܰ��������֡������� 2^5 * 7 �ֽ�
#define  MAXRXFRAMES     (32)
// ÿ֡�����������Ч�ֽ���
#define  FRAMESIZE       (7)
// ����֮֡��������ʱ��
#define  MAXRXSPACETIME  (200)
// ��󻺴��С,������ͬʱ������Щ���豸��ַ������
#define MAXCANRXBUFFS     (32)
// �Ƿ�CAN��ַ
#define INVALID_ADDR      (-1)

/*
* ��չ��ʽʶ����� 29 λ��ɡ����ʽ�����������֣�11 λ���� ID��18 λ��չ ID��
* Controller Area Network Identifier structure
*
* bit 0-28     : CANʶ��� (11/29 bit)
* bit 29     : ����֡��־ (0 = data frame, 1 = error frame)
* bit 30     : Զ�̷��������־ (1 = rtr frame)
* bit 31     :֡��ʽ��־ (0 = standard 11 bit, 1 = extended 29 bit)
*/


// CAN֡�ṹ
typedef struct __can_frame {
    __UINT32 can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    __UINT8    can_dlc; /* data length code: 0 .. 8 */
#ifndef WIN32
    __UINT8    data[8] __attribute__((aligned(8)));
#else
    __INT8     PADDING[3]; // �������
    __UINT8    data[8];
#endif
}CANFRAME;

// CAN ���ջ�����
typedef struct {
    bool    used;							// TRUE �Ѿ���ռ��
    __UINT32   OSTime;						    // ��һ֡����ʱ��
    __UINT32   address;						    // ��ַ����֡ʱռ��
    __UINT8    index;
    __UINT8    data[MAXRXFRAMES][FRAMESIZE];
}CANRXBUFF;

class  CSaftopCAN:public ICoder
{
public:
    CSaftopCAN(void);
    ~CSaftopCAN(void);
    // ��ʼ������,һ�㴫�������ָ��. ʧ��:0 �ɹ�:��0
     virtual int Init(void *pMainServer);
    // �ͷŸ�ģ���Լ��������Դ
    virtual void Fini();
    // ���һ�����ݰ��������ذ��ĳ���,������û����ȫ,����0
    virtual int SplitPacket(const char *InBuf, const int &Inlen);
    // �ѻ�������InBuf���뵽OutBuf��,���ܳɹ�����true,����ʧ�ܻ�������ܷ���false
    virtual	bool DeCode( const char *InBuf, const int &InLen,char *OutBuf,int &OutLen);
    // �ѻ�������InBuf���뵽OutBuf��,���ܳɹ�����true,����ʧ�ܻ�������ܷ���false
    virtual bool Code(  const char *InBuf, const int &InLen,char *OutBuf,int &OutLen);
private:
    // ���Ҹ��豸��ַ�Ѿ����ڵ�δ����������֡
    CANRXBUFF* SearchCANRxBuff(	CANRXBUFF * pbuff,const __UINT32 &address);
    // ����һ�����еĻ�����
    CANRXBUFF* GetCANRxBuff(CANRXBUFF * pbuff);
    // ����CAN֡������
    bool  PutCANRxBuff(CANRXBUFF  * pbuff,CANFRAME *pframe);
    // �ͷŻ���
    void FreeCANRxBuff(CANRXBUFF * pbuff);
    // ���ճ�ʱ���ݰ�ռ�õĻ���
    bool RecCANRxBuff(	CANRXBUFF * pbuff);

private:
    CANRXBUFF m_CANBUFF[MAXCANRXBUFFS]; // ���ջ����� 
};

