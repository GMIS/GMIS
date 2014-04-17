// UserSpaceMutex.cpp: implementation of the CUserSpaceMutex class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "UserSpaceMutex.h"
#include "UserMutex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserSpaceMutex::CUserSpaceMutex()
{
#ifdef WIN32
    ::InitializeCriticalSection(&m_Mutex);
#else
    (void)pthread_once(&sMutexAttrInit, MutexAttrInit);
    (void)pthread_mutex_init(&m_Mutex, sMutexAttr);
#endif
}

CUserSpaceMutex::~CUserSpaceMutex()
{
#ifdef WIN32
	::DeleteCriticalSection(&m_Mutex);
#else
	pthread_mutex_destroy(&m_Mutex);
#endif
}


SeatMutex* CUserSpaceMutex::CreateSeat(){
	
	SeatMutex*  m = new SeatMutex;
	if(m==NULL)return NULL;
	m->Mutex = new CUserMutex;
	if (m->Mutex == NULL)
	{
		delete m;
		return NULL;
	}
	return m;
}