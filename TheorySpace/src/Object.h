/*
*
* Object������������и�߼��ĳ����ࡣ��һ��Massʵ��ͬ��Object��ʵ��
* ���е������߳���ִ���Լ�����Ϊ��Ҳ����˵������ˡ�������ԡ��������ֲ�
* �ܶ������ڣ�����������ĳ����̣�
*
* ͨ������ʵ������֡�������֯����һ���û��ò������ࡣ
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _OBJECT_H__
#define _OBJECT_H__

#include "AbstractSpace.h"
#include "LockPipe.h"
#include <assert.h>

namespace ABSTRACT{


class   Object : public Mass  
{
protected:
	tstring          m_Name;
	volatile bool    m_Alive;

	/*�����߳�����ѭ���ļ��ʱ�䣬ȱʡΪ50���롣
	  
	  ��������������˽���CPU��ռ���ʣ�����Ϊ��ͬ����������в�ͬ��ʱ�����жȣ�
	  ������������ƽʱ������Ӧһ�㣬��Σ��ʱ���������ܶ�һ��

      GMISʵ����Ϊ���������ԡ�����ÿһ����֯�̶߳����Լ���SleepTime,ϵͳͨ�����ʱ������
	  Э�������Ϊ�ĺ�г�����統�յ�����Ϣ����������ʱ��ϵͳ���С��Ϣ�����̵߳ļ��ʱ�䣬
	  �������Ϣ���ܵļ��ʱ�䡣��ȵ����̵߳����ȼ������ǿ����ô��Ը�ݾ���������ص㣬
	  ͨ����㷨��ʵʱ��������߳������SleepTime��
	*/

    int32			 m_SleepTime ;  //<0��ʾִֻ��һ��

	Object(CTimeStamp* Timer,CAbstractSpacePool* Pool):
		Mass(Timer,Pool),
		m_Name(_T("MainObject")),
		m_Alive(FALSE),
		m_SleepTime(50)
	{
	}
public:
		
	Object()
		:m_Name(_T("Object")),
		 m_Alive(FALSE),
		 m_SleepTime(50)
	{
	};
	Object(tstring Name)
		:m_Name(Name),
		 m_Alive(FALSE),
		 m_SleepTime(50)
	{
	};

	virtual ~Object(){
	
	};

	virtual MASS_TYPE  MassType(){ return MASS_OBJECT;};
	virtual TypeAB   GetTypeAB(){ return 0;};  

	virtual bool Activation(){
		if(m_Alive)return TRUE;
		m_Alive  = TRUE;
 		return TRUE;
	}	

	tstring GetName(){
		return m_Name;
	};

	void SetName(tstring Name){
		m_Name = Name;
	}
	virtual bool IsAlive(){ return m_Alive;};

	virtual void Dead(){ 
		m_Alive = FALSE;
	}

	void SetObjectSleepTime(int32 ms){m_SleepTime = ms;};

#ifdef _WIN32
	static unsigned int ObjectDefaultThreadFunc (void* pParam)
	{
		Object* This = reinterpret_cast<Object*>(pParam);
		assert(This != NULL);
		This->Do(NULL); 
		return 0;
	};
#else
	static void* ObjectDefaultThreadFunc (void* pParam)
	{
		Object* This = reinterpret_cast<Object*>(pParam);
		assert(This != NULL);
		This->Do(NULL); 
		return (void*)0;
	};
#endif
};

}

#endif //namespace ABSTRACT 
