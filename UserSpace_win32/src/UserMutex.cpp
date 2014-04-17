// UserMutex.cpp: implementation of the CUserMutex class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "UserMutex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserMutex::CUserMutex()
{
#ifdef WIN32
    ::InitializeCriticalSection(&m_Mutex);
#else
    (void)pthread_once(&sMutexAttrInit, MutexAttrInit);
    (void)pthread_mutex_init(&m_Mutex, sMutexAttr);
#endif
}

CUserMutex::~CUserMutex()
{
#ifdef WIN32
	 ::DeleteCriticalSection(&m_Mutex);
#else
	  pthread_mutex_destroy(&m_Mutex);
#endif
}

