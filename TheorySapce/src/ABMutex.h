/*
*author: ZhangHongBing(hongbing75@gmail.com)        
* 
* 
* 由于Mutex与具体实现系统相关，这里定义一个虚接口，以方便体系设计
*/
#ifndef  _ABMUTEX_H__
#define  _ABMUTEX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Typedef.h"

namespace ABSTRACT{


class CABMutex  
{
public:
	CABMutex(){};
	virtual ~CABMutex(){};

    virtual void Acquire()
	{
		//请根据自己的系统实现此函数
		assert(0);
	}; 

    virtual void Release()	
	{
		//请根据自己的系统实现此函数
		assert(0);
	}

	//用于共用锁须重载，参见SpaceMutex.h
	virtual bool AcquireThis(void* user){
		Acquire();
		return TRUE;
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
    CLock( CABMutex* mutex): m_Mutex(mutex),m_User(NULL){
		assert(mutex);
		m_Mutex->Acquire();
	}
    CLock( CABMutex* mutex,void* User): m_Mutex(mutex),m_User(User){
		assert(mutex);
		m_User==NULL?m_Mutex->Acquire():m_Mutex->AcquireThis(m_User);
	}
    ~CLock (){
		m_User==NULL?m_Mutex->Release():m_Mutex->ReleaseThis(m_User);
	}   
};

}

#endif // _ABMUTEX_H__