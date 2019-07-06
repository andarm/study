#ifndef _S3C_DO_H_
#define _S3C_DO_H_
#include <linux/types.h>

#ifdef __KERNEL__
	typedef struct
	{
		u32 cmd;
		u32 ch;
		u32 time_10ms;
	} ST_S3C_DO_CFG, *LPST_S3C_DO_CFG;
#else	
	typedef struct
	{
		__u32 cmd;
		__u32 ch;
		__u32 time_10ms;
	} ST_S3C_DO_CFG, *LPST_S3C_DO_CFG;
#endif

#define S3C_DO_NC			    8001
#define S3C_DO_NO			    8002
#define S3C_DO_PULSE_NC		8003
#define S3C_DO_PULSE_NO		8004

#endif
