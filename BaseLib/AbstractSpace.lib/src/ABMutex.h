/*
*author: ZhangHongBing(hongbing75@gmail.com)        
* 
* 
*  Here just define a virtual mutex interface. 
*  It should be implemented later according to the specific operating system
*/

#ifndef  _ABMUTEX_H__
#define  _ABMUTEX_H__


#include "Typedef.h"

namespace ABSTRACT{


class CABMutex  
{
public:

#ifdef _DEBUG
public:
	tstring m_Memo; //for deadlock detection;
#endif

public:
	CABMutex(){};
	virtual ~CABMutex(){};

    virtual void Acquire()
	{
		//#pragma message("!!! Please re-implement this function according to your selected  platform or C++ lib")
		assert(0);
	}; 

    virtual void Release()	
	{
		//#pragma message ("!!! Please re-implement this function according to your selected  platform or C++ lib")
		assert(0);
	}

	//Share lock, overload it before using,see SpaceMutex.h 
	virtual bool AcquireThis(void* user){
		Acquire();
		return true;
	}; 
	virtual void ReleaseThis(void* user){
		Release();
	};
};


class  CLock
{
private:
	CABMutex*  m_Mutex;
	void*      m_User;
public:

#ifdef _DEBUG
	CLock( CABMutex* mutex,tstring s): m_Mutex(mutex),m_User(NULL){
		assert(mutex);
		m_Mutex->Acquire();
		m_Mutex->m_Memo = s;
	}

	CLock( CABMutex* mutex,void* User,tstring s): m_Mutex(mutex),m_User(User){
		assert(mutex);
		if(m_User==NULL) {
			m_Mutex->Acquire();
		}else {
			bool ret = m_Mutex->AcquireThis(m_User);
			assert(ret);
		}
		m_Mutex->m_Memo = s;
	}
#endif
    CLock( CABMutex* mutex): m_Mutex(mutex),m_User(NULL){
		assert(mutex);
		m_Mutex->Acquire();
	}

    CLock( CABMutex* mutex,void* User): m_Mutex(mutex),m_User(User){
		assert(mutex);
		if(m_User==NULL) {
			m_Mutex->Acquire();
		}else {
			bool ret = m_Mutex->AcquireThis(m_User);
			assert(ret);
		}
	}

    ~CLock (){
		m_User==NULL?m_Mutex->Release():m_Mutex->ReleaseThis(m_User);
	}   
};

#ifdef _CHECK_MUTEX_DEADLOCK_
#define _CLOCK(m) \
	CLock lk(m,Format1024(_T("%s %d"),_T(__FILE__),__LINE__));
#define _CLOCK2(m,u)\
	CLock lk(m,u,Format1024(_T("%s %d"),_T(__FILE__),__LINE__));
#else
#define _CLOCK(m) \
	CLock lk(m);
#define _CLOCK2(m,u)\
	CLock lk(m,u);
#endif

}

#endif // _ABMUTEX_H__
