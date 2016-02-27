/*************     Copyright (C) 2013 SAFTOP.COM     ****************
created:	2013/05/11
created:	11:5:2013   9:13
filename: 	IO_485.h
file base:	IO_485
file ext:	h
author:		linjinye

purpose:	Serial 485接口
*********************************************************************/
#pragma once
#include "interface.h"
#include "../comm/sys/thread.h"
#include "../comm/sys/Serial.h"
using namespace ST_SYS;

#define MAX_CAN_NAME     (64)
#define MAX_CAN_BUF_LEN  (1024)
class  IO485:public IIOLayer,CThread
{
public:
    IO485(void);
    ~IO485(void);
    // 初始化函数,一般传入主框架指针. 失败:0 成功:非0
    virtual int Init(void *pMainServer);
    // 释放该模块自己申请的资源,最后面调用delete this，防止不同模块释放不同堆空间数据时异常
    virtual void Fini();

    virtual int Write(const char *pData,const int &nLen, const int &index = 0);
    // 打开IO模块,通过args传参
    virtual int Open( const char *args);
    // 关闭IO模块,通过args传参
    virtual int Close(const char *args);
    // config设置与获取
    virtual	int IOCtrl(int	nAction, void *pActionStruct, void *pRetData);
protected:
    virtual void Run();
private:
    ICoder *GetCoder();
    // 回调方式处理IO层数据
    void OnIOHandle(const int &IFIndex/*接口索引*/ ,const char *pData,const int &nLen,const MSG_TYPE &type = MSG_TYPE_DATA);
    void Setopts(int s, int loopback, int recv_own_msgs);
    bool InitSerial();
    bool DrvRun();
    // CAN数据包发送
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
    CSerial *m_serial;   
};

