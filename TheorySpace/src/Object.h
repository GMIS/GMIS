/*
*
* Object is a more advanced abstract class to represente the world. 
* Different from the orginary Mass instance, the object instance has a separate * thread to perform its behavior, meaning that it has a "biologically active", 
* but it does not able to exist independently (must be attached to a process)
*  
* Usually to implement a variety of "tissue", and general users don't need this * class.
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

	/*
	Control the loop interval of threads func, default is 50 milliseconds	  

	Set this variable in addition to lower CPU occupy rate, but also because different tasks may have different 
	time-sensitive, like in emergency the reaction capability of animals  is more effective than peace time.

	GMIS instance as the brain of robot, each "tissue" thread of it has its own SleepTime,the system adjust this time
	to coordinate the entire behaviors that are more harmony.For example, when there is not enough time to handle the recieved messages, tthe system will decrease the interval of message processing thread and increase the interval of 	message receiving thread.Compared with adjust the thread priority,using an algorithm that depend on the specific 	task,we can adjust the SleepTime of each thread in real-time to achieve same goal.
	*/

    int32			 m_SleepTime ;  //<0 means only executing

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
	static  unsigned int _stdcall ObjectDefaultThreadFunc (void* pParam)
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
