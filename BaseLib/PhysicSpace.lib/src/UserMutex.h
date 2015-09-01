/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _USERMUTEX_H__
#define _USERMUTEX_H__


#include "ABMutex.h"

#if defined(USING_POCO)

#include "Poco/Mutex.h"
using namespace Poco;

#elif defined(USING_WIN32)
#include <windows.h>

#else 
#error "Lack of specified platform #define(eg.USING_POCO or USING_WIN32 or other UserSpace implementation relied on your OS,default using USING_POCO because of it is cross-platform"

#endif

using namespace ABSTRACT;

class CUserMutex : public CABMutex  
{
private:

#if defined(USING_POCO)
    Mutex  m_Mutex;                    
#elif defined(USING_WIN32)
	CRITICAL_SECTION  m_Mutex;                    
#endif

public:
	CUserMutex();
	virtual ~CUserMutex();

    virtual void Acquire();
	virtual void Release();
};


#endif // _USERMUTEX_H__
