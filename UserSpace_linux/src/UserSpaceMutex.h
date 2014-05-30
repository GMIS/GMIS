/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERSPACEMUTEX_H__
#define _USERSPACEMUTEX_H__


#include <sys/time.h>
#include <pthread.h>

#include "../../TheorySpace/src/SpaceMutex.h"

using namespace ABSTRACT;

class CUserSpaceMutex: public CSpaceMutex
{
private:

    pthread_mutex_t   m_Mutex;
	
public:
	CUserSpaceMutex();
	virtual ~CUserSpaceMutex();
	
    virtual void Acquire(){

		pthread_mutex_lock(&m_Mutex);
	};
	
	
	virtual void Release(){
		pthread_mutex_unlock(&m_Mutex);
	};
	
	virtual SeatMutex* CreateSeat();
};

#endif // _USERSPACEMUTEX_H__
