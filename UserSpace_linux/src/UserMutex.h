/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _USERMUTEX_H__
#define _USERMUTEX_H__


#include "../../TheorySpace/src/ABMutex.h"

#include <sys/time.h>
#include <pthread.h>

using namespace ABSTRACT;

class CUserMutex : public CABMutex  
{
private:

    pthread_mutex_t   m_Mutex;      
public:
	CUserMutex();
	virtual ~CUserMutex();

    virtual void Acquire(){

		(void)pthread_mutex_lock(&m_Mutex);
	};


	virtual void Release(){
		pthread_mutex_unlock(&m_Mutex);
	};
};


#endif // _USERMUTEX_H__
