/********************************************************************
	created:	2013/12/04
	created:	4:12:2013   14:45
	filename: 	SaftopCAN.h
	file base:	SaftopCAN
	file ext:	h
	author:		liujia
	
	purpose:	Saftop CAN协议编解码
*********************************************************************/
#pragma once
#include "../comm/frame/interface.h"
#include "../comm/sys/type.h"

/* Saftop CAN协议
*  =================================================================================================================================================
*  |                                仲裁域                                   |                                  数据域                        |
*  =================================================================================================================================================
*  | Bit28 |             Bit27 ~ Bit14       |          Bit13 ~ Bit0         |                Data[0]                       |   Data[1] ~ Data[7]  |
*  =================================================================================================================================================
*  |优先级| 目标地址 总线号(7) 设备地址(7) | 源地址 总线号(7) 设备地址(7) |  类型码(3):0:起始 1:中间 2:结束  | Index(5) |         数据区       |
*  =================================================================================================================================================
*  注:最大传输长度:   224 Byte = 2^5 * 7
*/

#define  FRAME_BEG   (0)  // 起始帧
#define  FRAME_MID   (1)  // 中间帧
#define  FRAME_END   (2)  // 结束帧

// CAN 地址屏蔽码
#define  CANADDRESSMASK    (0x3fff)

// 一个数据包所能包含的最大帧数，最大 2^5 * 7 字节
#define  MAXRXFRAMES     (32)
// 每帧包含的最大有效字节数
#define  FRAMESIZE       (7)
// 接收帧之间的最大间隔时间
#define  MAXRXSPACETIME  (200)
// 最大缓存大小,即可以同时解码这些个设备地址的数据
#define MAXCANRXBUFFS     (32)
// 非法CAN地址
#define INVALID_ADDR      (-1)

/*
* 扩展格式识别符由 29 位组成。其格式包含两个部分：11 位基本 ID、18 位扩展 ID。
* Controller Area Network Identifier structure
*
* bit 0-28     : CAN识别符 (11/29 bit)
* bit 29     : 错误帧标志 (0 = data frame, 1 = error frame)
* bit 30     : 远程发送请求标志 (1 = rtr frame)
* bit 31     :帧格式标志 (0 = standard 11 bit, 1 = extended 29 bit)
*/


// CAN帧结构
typedef struct __can_frame {
    __UINT32 can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    __UINT8    can_dlc; /* data length code: 0 .. 8 */
#ifndef WIN32
    __UINT8    data[8] __attribute__((aligned(8)));
#else
    __INT8     PADDING[3]; // 填充区域
    __UINT8    data[8];
#endif
}CANFRAME;

// CAN 接收缓冲区
typedef struct {
    bool    used;							// TRUE 已经被占用
    __UINT32   OSTime;						    // 上一帧接收时间
    __UINT32   address;						    // 地址，首帧时占用
    __UINT8    index;
    __UINT8    data[MAXRXFRAMES][FRAMESIZE];
}CANRXBUFF;

class  CSaftopCAN:public ICoder
{
public:
    CSaftopCAN(void);
    ~CSaftopCAN(void);
    // 初始化函数,一般传入主框架指针. 失败:0 成功:非0
     virtual int Init(void *pMainServer);
    // 释放该模块自己申请的资源
    virtual void Fini();
    // 拆分一个数据包，并返回包的长度,若数据没有收全,返回0
    virtual int SplitPacket(const char *InBuf, const int &Inlen);
    // 把缓存数据InBuf解码到OutBuf中,解密成功返回true,解密失败或无需解密返回false
    virtual	bool DeCode( const char *InBuf, const int &InLen,char *OutBuf,int &OutLen);
    // 把缓存数据InBuf编码到OutBuf中,加密成功返回true,加密失败或无需加密返回false
    virtual bool Code(  const char *InBuf, const int &InLen,char *OutBuf,int &OutLen);
private:
    // 查找该设备地址已经存在的未接收完整的帧
    CANRXBUFF* SearchCANRxBuff(	CANRXBUFF * pbuff,const __UINT32 &address);
    // 申请一个空闲的缓存区
    CANRXBUFF* GetCANRxBuff(CANRXBUFF * pbuff);
    // 插入CAN帧到缓存
    bool  PutCANRxBuff(CANRXBUFF  * pbuff,CANFRAME *pframe);
    // 释放缓存
    void FreeCANRxBuff(CANRXBUFF * pbuff);
    // 回收超时数据包占用的缓存
    bool RecCANRxBuff(	CANRXBUFF * pbuff);

private:
    CANRXBUFF m_CANBUFF[MAXCANRXBUFFS]; // 接收缓存区 
};

