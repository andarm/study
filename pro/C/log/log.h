#ifndef __LOG_H 
#define __LOG_H 

#include "type.h"


#define MAX_LOG_NUM 100  //  100*256 size is 25k 
#define MAX_LOG_INFO_SIZE 128 

typedef struct tagST_LOG
{
   
    UINT16  id ; 
    UINT8 ucLogInfo[MAX_LOG_INFO_SIZE] ; 

}ST_LOG; 


typedef struct tagST_LOG_MNG
{
   BOOL bLogInitFlag ; 
   BOOL bLogWriteFlag; 
   UINT8 uiTimeOut ;
   ST_LOG ArrSTLogInfo[MAX_LOG_NUM];  
}ST_LOG_MNG; 
typedef ST_LOG_MNG g_st_log_mng ; 

void  LogInit(void);
UINT8   LogWrite(UINT8 *pInfo);
UINT8   *LogRead(UINT8 ID); 
#endif 
