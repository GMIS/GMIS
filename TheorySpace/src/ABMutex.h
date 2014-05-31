/*
*author: ZhangHongBing(hongbing75@gmail.com)        
* 
* 
* ����Mutex�����ʵ��ϵͳ��أ����ﶨ��һ����ӿڣ��Է�����ϵ���
*/
#ifndef  _ABMUTEX_H__
#define  _ABMUTEX_H__


#include "Typedef.h"

namespace ABSTRACT{


class CABMutex  
{
public:
	CABMutex(){};
	virtual ~CABMutex(){};

    virtual void Acquire()
	{
		//�����Լ���ϵͳʵ�ִ˺���
		assert(0);
	}; 

    virtual void Release()	
	{
		//�����Լ���ϵͳʵ�ִ˺���
		assert(0);
	}

	//���ڹ����������أ��μ�SpaceMutex.h
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
		if(m_User==NULL) {
			m_Mutex->Acquire();
		}else {
			m_Mutex->AcquireThis(m_User);
		}
	}
    ~CLock (){
		m_User==NULL?m_Mutex->Release():m_Mutex->ReleaseThis(m_User);
	}   
};

}

#endif // _ABMUTEX_H__
