#ifndef _ST_VERSION_H_
#define _ST_VERSION_H_
#include "../comm/sys/strtool.h"
#define ST_VERSION    "v1.0.1138"

#define	INIT_MOUDLE_INFO(AUTHOR,EXPLAIN)  \
    memset(m_Info.szAuthor,0,MAX_INTERFACE_INFO_LEN); \
    memset(m_Info.szExplain,0,MAX_INTERFACE_INFO_LEN);\
    memset(m_Info.szDate,0,MAX_INTERFACE_INFO_LEN);\
    memset(m_Info.szVer,0,MAX_INTERFACE_INFO_LEN);\
    _SNPRINTF_(m_Info.szAuthor,MAX_INTERFACE_INFO_LEN-1,AUTHOR);\
    _SNPRINTF_(m_Info.szExplain,MAX_INTERFACE_INFO_LEN-1,EXPLAIN);\
    _SNPRINTF_(m_Info.szVer,MAX_INTERFACE_INFO_LEN-1,ST_VERSION);\
    _SNPRINTF_(m_Info.szDate,MAX_INTERFACE_INFO_LEN-1,"%s %s",__DATE__,__TIME__);\
	
#endif