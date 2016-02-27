// UserMutex.cpp: implementation of the CUserMutex class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "UserMutex.h"
#include "Pipeline.h"
#include "Poco/Exception.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserMutex::CUserMutex()
{
#if defined(USING_WIN32)
	::InitializeCriticalSection(&m_Mutex);
#endif
}

CUserMutex::~CUserMutex()
{
#if defined(USING_WIN32)
	::DeleteCriticalSection(&m_Mutex);
#endif
}

void CUserMutex::Acquire(){
#if defined(USING_POCO)
#ifdef _CHECK_MUTEX_DEADLOCK_
try{
	m_Mutex.lock(1000);
	//m_Mutex.lock();
}
catch(TimeoutException& e){
	AnsiString s = e.displayText();
	tstring& PreAcuire = m_Memo;
	assert(0);
}
#else
	m_Mutex.lock();
#endif

#elif defined(USING_WIN32)
	::EnterCriticalSection(&m_Mutex);
#endif
};


void CUserMutex::Release(){
#if defined(USING_POCO)
	m_Mutex.unlock();
#elif defined(USING_WIN32)
	::LeaveCriticalSection(&m_Mutex);
#endif
};