/*
*
* Object用来表达世界中更高级的抽象类。与一般Mass实例不同，Object的实例
* 具有单独的线程来执行自己的行为，也就是说它具有了“生物活性”，但它又不
* 能独立存在（必须依附于某个进程）
*
* 通常用来实现其各种“生物组织”，一般用户用不到此类。
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _OBJECT_H__
#define _OBJECT_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "AbstractSpace.h"
#include <process.h>
#include "LockPipe.h"
#include <assert.h>

namespace ABSTRACT{


class   Object : public Mass  
{
protected:

	volatile bool    m_Alive;

	/*控制线程任务循环的间隔时间，缺省为50毫秒。
	  
	  设置这个变量除了降低CPU的占用率，还因为不同的任务可能有不同的时间敏感度，
	  就象所有生物平时动作反应一般，但危急时刻则灵敏很多一样。

      GMIP实例作为机器“大脑”，其每一个组织线程都有自己的SleepTime,系统通过这个时间来来
	  协调整个行为的和谐，比如当收到的信息来不及处理时，系统会调小信息处理线程的间隔时间，
	  而调大信息接受的间隔时间。相比调整线程的优先级，我们可以让大脑根据具体的任务特点，
	  通过简单算法来实时调整各个线程所需的SleepTime。
	*/

    int32			 m_SleepTime ;  //<0表示只执行一次

	tstring          m_Name;

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

	static unsigned __stdcall ObjectDefaultThreadFunc (LPVOID pParam)
	{
		Object* This = reinterpret_cast<Object*>(pParam);
		assert(This != NULL);
		This->Do(NULL); 
		return 1;
	};
};

}

#endif //namespace ABSTRACT 