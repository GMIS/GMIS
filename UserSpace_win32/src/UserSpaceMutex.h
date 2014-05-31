/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERSPACEMUTEX_H__
#define _USERSPACEMUTEX_H__


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

    CRITICAL_SECTION  m_Mutex;                    
	
public:
	CUserSpaceMutex();
	virtual ~CUserSpaceMutex();
	
    virtual void Acquire(){

		::EnterCriticalSection(&m_Mutex);
	};
	
	
	virtual void Release(){

		pthread_mutex_unlock(&m_Mutex);	
	};
	
	virtual SeatMutex* CreateSeat();
};

#endif // _USERSPACEMUTEX_H__
