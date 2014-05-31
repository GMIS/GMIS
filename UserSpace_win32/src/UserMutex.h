/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _USERMUTEX_H__
#define _USERMUTEX_H__


#include "ABMutex.h"
#include <windows.h>

using namespace ABSTRACT;

class CUserMutex : public CABMutex  
{
private:

    uint32            m_ThreadID;
    CRITICAL_SECTION  m_Mutex;                    

public:
	CUserMutex();
	virtual ~CUserMutex();

    virtual void Acquire(){

		::EnterCriticalSection(&m_Mutex);
		m_ThreadID = GetCurrentThreadId();

	};


	virtual void Release(){

		::LeaveCriticalSection(&m_Mutex);
		m_ThreadID = 0;

	};
};


#endif // _USERMUTEX_H__
