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

#include "Mass.h"
#include "LockPipe.h"
#include <assert.h>

#if defined(USING_POCO)

#include "Poco/Thread.h"
using namespace Poco;

#elif defined(USING_WIN32)
#include <process.h>
#else 
#error "Lack of specified platform #define(eg.USING_POCO or USING_WIN32 or other UserSpace implementation relied on your OS,default using USING_POCO because of it is cross-platform"

#endif


namespace PHYSIC{


class   Object : public Mass  
{
protected:
	tstring          m_Name;
	volatile bool    m_Alive;


#if defined(USING_POCO)
	Thread           m_ObjectDefaultThread;
#elif defined(USING_WIN32)
	HANDLE           m_hObjectDefaultThread;
	DWORD            m_hObjectDefaultThreadID;
#endif

	/*
	Control the loop interval of threads func, default is 50 milliseconds	  

	Set this variable in addition to lower CPU occupy rate, but also because different tasks may have different 
	time-sensitive, like in emergency the reaction capability of animals  is more effective than peace time.

	GMIS instance as the brain of robot, each "tissue" thread of it has its own SleepTime,the system adjust this time
	to coordinate the entire behaviors that are more harmony.For example, when there is not enough time to handle the recieved messages, tthe system will decrease the interval of message processing thread and increase the interval of 	message receiving thread.Compared with adjust the thread priority,using an algorithm that depend on the specific 	task,we can adjust the SleepTime of each thread in real-time to achieve same goal.
	*/

    int32			 m_SleepTime ;  //<0 means only executing

protected:
	Object(CABTime* Timer,CABSpacePool* Pool);

public:	
	Object();
	virtual ~Object();

	virtual MASS_TYPE  MassType(){ return MASS_OBJECT;};
	virtual TypeAB   GetTypeAB(){ return 0;};  

	virtual bool Activate();
	virtual void Dead();

	tstring GetName(){
		return m_Name;
	};
	void SetName(tstring Name){
		m_Name = Name;
	}

	virtual bool IsAlive(){ return m_Alive;};

	void SetObjectSleepTime(int32 ms){m_SleepTime = ms;};


//////////////////////////////////////////////////////////////////////////
#if defined(USING_POCO)
	static void _cdecl ObjectDefaultThreadFunc (void* pParam)
	{
		Object* This = reinterpret_cast<Object*>(pParam);
		assert(This != NULL);
		This->Do(NULL); 
	}
#elif defined(WIN32)
	static unsigned int __stdcall ObjectDefaultThreadFunc (void* pParam)
	{
		Object* This = reinterpret_cast<Object*>(pParam);
		assert(This != NULL);
		This->Do(NULL); 
		return 0;
	};
#endif
};

}

#endif //namespace ABSTRACT 
