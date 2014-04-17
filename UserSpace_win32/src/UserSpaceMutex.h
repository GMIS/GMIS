// UserSpaceMutex.h: interface for the CUserSpaceMutex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERSPACEMUTEX_H__5F40EADA_91D9_4CC4_8B49_E9354CADBE7A__INCLUDED_)
#define AFX_USERSPACEMUTEX_H__5F40EADA_91D9_4CC4_8B49_E9354CADBE7A__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef WIN32
#include <sys/time.h>
#include <pthread.h>
#endif

#include "SpaceMutex.h"

using namespace ABSTRACT;

class CUserSpaceMutex: public CSpaceMutex
{
private:
#ifdef WIN32
    CRITICAL_SECTION  m_Mutex;                    
#else
    pthread_mutex_t   m_Mutex;      
#endif      
	
public:
	CUserSpaceMutex();
	virtual ~CUserSpaceMutex();
	
    virtual void Acquire(){
#ifdef WIN32
		::EnterCriticalSection(&m_Mutex);
#else
		(void)pthread_mutex_lock(&m_Mutex);
#endif
	};
	
	
	virtual void Release(){
#ifdef WIN32
		::LeaveCriticalSection(&m_Mutex);
#else
		pthread_mutex_unlock(&m_Mutex);
#endif	
	};
	
	virtual SeatMutex* CreateSeat();
};

#endif // !defined(AFX_USERSPACEMUTEX_H__5F40EADA_91D9_4CC4_8B49_E9354CADBE7A__INCLUDED_)
