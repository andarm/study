/********************************************************************
	created:	2013/12/03
	created:	3:12:2013   10:26
	filename: 	TSrv.h
	file base:	HTSrv
	file ext:	h
	author:		liujia
	
	purpose:	Arm9 HTҵ��ģ��
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
// CAN״̬
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

// ����������豸��(0x7f)
#define MAX_DEV_NUM		(0x7f) 
// CAN�����豸�������
#define MAX_CAN_CHECK_TIME		(3)
#define MAX_DEV_CHECK_TIME      (15)
#define MAX_NOTIFY_TIME         (3)

// CAN���ߵ�ַ����
#define CAN_ADDR_MASK			(0x7f)
// PC��CAN��ַ
#define LOCAL_PC_ADDR	        (0x01)
// HT��ַ
#define  LOCAL_HT_ADDR          (0x6f)
// ���ص�ַ
#define GWCANADDR               (0x02)
// CAN�㲥��ַ
#define CANBRAODCASTADDR		(0x00)


// CANͨ����
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
    CAN_STATE stat;  // ״̬
    __UINT32 dwflow; // ����
}_CAN_STST;

// HT��CAN״̬
typedef struct HT_CAN_STAT
{
    _CAN_STST CAN_STAT[MAX_CAN_CHANNEL];
	__INT16  nCanTimeCheckCount;// CAN��ʱ��������
}HT_CAN_STAT;

// �������豸CAN״̬
typedef struct DEV_CAN_STAT
{
	_CAN_STST CAN1;
	_CAN_STST CAN2;
	__INT16  nCan1TimeCheckCount;// CAN1��ʱ������
	__INT16  nCan2TimeCheckCount;// CAN2��ʱ������
}DEV_CAN_STAT;


class  CHTSrv:public ILogic,CLogicMsgMng
{
public:

    CHTSrv(void);
    virtual ~CHTSrv(void);
    // ��ʼ������,һ�㴫�������ָ��. ʧ��:0 �ɹ�:��0
    virtual	int	Init(void *pMainServer);
    // �ͷŸ�ģ���Լ��������Դ
    virtual void	Fini();
    // ��Ϣ����ӿ�
    virtual long  OnHandle(const LogicMsg *pMsg,void *Para = NULL);
protected:
    virtual void OnMessage(LogicMsg *pMsg);
    bool SendMsg(const ILogic *pLogic,const char *pData,const __UINT32 &dwSize);
private:
    // �ӿ����ӳɹ�
    void OnConnect(const int &nIndex,void *Para);
    // ��½�����(TCPģʽ��Ч)
    void LoginSrv();
    // �ӿڶ���
    void OnDisConnect(const int &nIndex,void *Para);
    // ��ʱ���ص�����
    long OnTimer(void *Para);
	// ���������豸CAN״̬��
	void UpdateDevCanState(const LogicMsg *pMsg);
    // �������ݵ� CAN����
    void TransDataToCAN(const char *data,const __UINT32 &dwLen,const CANCHANNEL &ch,const __UINT16 &Addr = CANBRAODCASTADDR);
	// ��������豸CAN״̬��
	void CheckDevCanState(void);
	// ���HT��CAN״̬
	void CheckHtCanState(void);
	// ��ȡCAN״̬,��Ϊ����״̬�����³�ʼ��CAN�˿�
	__INT32 CheckCanStateOrRestart(const CANCHANNEL &CanChannel, CAN_STATE *pCanState);
    // �㲥 CAN����������Ϣ
    void BroadcastCANInfo();
    //  ֪����λ�����߽ڵ�״̬
    void NotifyCANInfoToSrv();
	// ��ʼ�������豸״̬�б�
	void InitDevCanStateList(void);
    // ��ȡ�������豸·��
    const CANCHANNEL GetLocalBusRoute(const __UINT16 &Addr);
    // ��ȡ�������豸·��
    const CANCHANNEL GetCrossBusRoute();
    // �������ݰ���֡��
    const __UINT32 CalcPackFrames(const __UINT32 &nSize);
    
	// HT����Э��
    // ɨ��Ӧ��
    void OnScanHTResp(LogicMsg *pMsg);
    // HT����
    void OnSetHTCFG(LogicMsg *pMsg);
    // ����HT MAC��ַ
    void OnSetMacAddr(LogicMsg *pMsg);
private:
    // ����CAN���ߺ�
    __INT32			m_nLocalCAN_NO;
    // ���߼������ʱ��
    __INT32         m_dwCheckCANTime;
    // �豸�������ʱ��
    __INT32         m_dwCheckDEVTime;
    // ��ʱ�ϱ�״̬ʱ����
    __INT32         m_dwNotifyTime;

    IMainInterface *m_pMainServer;
    // CAN�ӿ�
    IIOLayer       *m_pCAN1;
    IIOLayer       *m_pCAN2;
    IIOLayer       *m_pCAN3;
    IIOLayer       *m_pCAN4;
    // HT����Ⱥ
    IIOLayer       *m_pIOSRV;
    // ���ýӿ�(UDP)
    IIOLayer       *m_pIOCFG;
    // �ӿ�����
    __UINT32       m_dwIndex_IOSRVIF;    // HT ��Ⱥ����ӿ�����
    __UINT32       m_dwIndex_CFG_WEB;    // web���ÿͻ��˽ӿ�����
    __UINT32       m_dwIndex_CFG_BR;     // ���ù㲥�ӿ� 
    // ������ʱ��
    __UINT32       m_dwTimerID;
	// HT��CAN״̬
	HT_CAN_STAT		m_HTCanStat; 
	// �����豸CAN״̬
	DEV_CAN_STAT  	m_DevCanState[MAX_DEV_NUM];
};

