/*************     Copyright (C) 2013 SAFTOP.COM     ****************
created:	2013/05/11
created:	11:5:2013   9:13
filename: 	SocketCAN.h
file base:	CSocketCAN
file ext:	h
author:		liujia

purpose:	Socket CAN�ӿ�
*********************************************************************/
#pragma once

#include "interface.h"
#include "../comm/sys/thread.h"
#include "SaftopCAN.h"
#include "../protocol/CANProtocol.h"
using namespace ST_SYS;

#define MAX_CAN_NAME     (64)
#define MAX_CAN_BUF_LEN  (1024)
class  CSocketCAN:public IIOLayer,CThread
{
public:
    CSocketCAN(void);
    ~CSocketCAN(void);
    // ��ʼ������,һ�㴫�������ָ��. ʧ��:0 �ɹ�:��0
    virtual int Init(void *pMainServer);
    // �ͷŸ�ģ���Լ��������Դ,��������delete this����ֹ��ͬģ���ͷŲ�ͬ�ѿռ�����ʱ�쳣
    virtual void Fini();

    virtual int Write(const char *pData,const int &nLen, const int &index = 0);
    // ��IOģ��,ͨ��args����
    virtual int Open( const char *args);
    // �ر�IOģ��,ͨ��args����
    virtual int Close(const char *args);
    // config�������ȡ
    virtual	int IOCtrl(int	nAction, void *pActionStruct, void *pRetData);
//protected:
    virtual void Run();
private:
    ICoder *GetCoder();
    // �ص���ʽ����IO������
    void OnIOHandle(const int &IFIndex/*�ӿ�����*/ ,const char *pData,const int &nLen,const MSG_TYPE &type = MSG_TYPE_DATA);
    void Setopts(int s, int loopback, int recv_own_msgs);
    bool InitSocketCAN();
    bool DrvRun();
    // CAN���ݰ�����
    int SendMsg( const char *pData,const int &nSize );
private:
    IMainInterface *m_pMainServer;
    unsigned int m_nLogicCount;
    ICoder  *m_pCoder;
    ILogic **pLogic;
    int     m_FD;
    char    m_szName[MAX_CAN_NAME];
    CSignal m_Exit;
    bool    m_bActive;
};

