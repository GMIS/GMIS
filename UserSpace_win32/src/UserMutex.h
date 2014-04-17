// UserMutex.h: interface for the CUserMutex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERMUTEX_H__76B5425A_CBB1_4E98_B229_FF4E86EA61CF__INCLUDED_)
#define AFX_MUTEX_H__76B5425A_CBB1_4E98_B229_FF4E86EA61CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ABMutex.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <pthread.h>
#endif

using namespace ABSTRACT;

class CUserMutex : public CABMutex  
{
private:
#ifdef WIN32
	uint32            m_ThreadID;
    CRITICAL_SECTION  m_Mutex;                    
#else
    pthread_mutex_t   m_Mutex;      
#endif      

public:
	CUserMutex();
	virtual ~CUserMutex();

    virtual void Acquire(){
#ifdef WIN32
		::EnterCriticalSection(&m_Mutex);
		m_ThreadID = GetCurrentThreadId();
#else
		(void)pthread_mutex_lock(&m_Mutex);
#endif
	};


	virtual void Release(){
#ifdef WIN32
		::LeaveCriticalSection(&m_Mutex);
		m_ThreadID = 0;
#else
		pthread_mutex_unlock(&m_Mutex);
#endif	
	};
};


#endif // !defined(AFX_USERMUTEX_H__76B5425A_CBB1_4E98_B229_FF4E86EA61CF__INCLUDED_)
