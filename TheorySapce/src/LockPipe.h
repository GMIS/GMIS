/*
*author: ZhangHongBing(hongbing75@gmail.com)  
*  
* 对ePipeline的封装，可以看作是一个用于多线程安全队列，
* 用来缓存输入输出信息。
* 自带一个紧急数据管道，如果紧急数据管道有数据则优先使用
*/

#ifndef _CLockPIPE_H
#define _CLockPIPE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "Typedef.h"
#include "Pipeline.h"
#include "ABMutex.h"
#include <string>
#include <assert.h>

namespace ABSTRACT{

 
class   CLockPipe: public ePipeline
{

protected:
	CABMutex*            m_Mutex;       //确保线程安全
	ePipeline            m_UrgenceMsg;
	int64                m_LastPopTimeStamp; //最后取出信息的时间戳
public:	

	CLockPipe(CABMutex* m,const TCHAR* Name=_T("Unkown"),uint64 ID=0)
		:m_Mutex(m),
		 ePipeline(Name,ID),
		 m_UrgenceMsg(_T("UrgenceMsg")),
		 m_LastPopTimeStamp(0)
	{};
	virtual ~CLockPipe(){
//		CLock lk(m_Mutex);  由于m_Mutex是后期决定的，可能被承继类提前删除而失效
//      m_UrgenceMsg.Clear();
	};

	int64 GetLastPopTimeStamp(){
		CLock lk(m_Mutex,this);
		return m_LastPopTimeStamp;
	}

    bool MutexIsVaild(){
		return m_Mutex != NULL;
	}

    int32 DataNum(){
		CLock lk(m_Mutex,this);
        int32 n = m_UrgenceMsg.Size() + Size();
		return n;
	}
	void ToString(AnsiString& s){
		CLock lk(m_Mutex,this);

		ePipeline Pipe;
		Pipe.Push_Copy(&m_UrgenceMsg);
		Pipe.Push_Copy(this);
		Pipe.ToString(s);
	};

	void CopyTo(ePipeline& Pipe){
		CLock lk(m_Mutex,this);
		Pipe.Push_Copy(&m_UrgenceMsg);
		Pipe.Push_Copy(this);
	}
	//注意：输入的字符串格式为：type@len{type@len@data1,type@len@data2 ... type@len@dataN}
	uint32   FromString(AnsiString& s,uint32 pos=0){ 
		CLock lk(m_Mutex,this);

		ePipeline Pipe;

		uint32 n = Pipe.FromString(s,pos);
        if(n==0)return n;

		eElectron UrgPipe, NormalPipe;
		Pipe.Pop(&UrgPipe);
		Pipe.Pop(&NormalPipe);
		
        m_UrgenceMsg.Clear();
		m_UrgenceMsg<<(*(ePipeline*)UrgPipe.Value());

		Clear();
		(*this)<<(*(ePipeline*)NormalPipe.Value());
		return n;
	}
    /*
	   往CCLockPipe里写的数据必须是ePipeline
	   NOTE: 由于使用Push_Directly,所以Data必须取自堆
	*/
	int64 Push(ePipeline* Data){
		assert(Data != NULL);
		CLock lk(m_Mutex,this);
		Push_Directly(Data);
		//=0具有标识首次使用的作用，但如果push很快，但还没来得及pop则m_LastPopTimeStamp始终=0,
		//为了避免这种情况，我们-1，如果push很快而pop很慢，则还有加大时间间隔的作用
		return m_LastPopTimeStamp--; 
	};
	
	//紧急数据会被优先取出
	int64 PushUrgence(ePipeline* Data){
		assert(Data != NULL);
		CLock lk(m_Mutex,this);
		m_UrgenceMsg.Push_Directly(Data);
		return m_LastPopTimeStamp--;
	};
	
	void Pop(eElectron* E){	
		CLock lk(m_Mutex,this);
		if(m_UrgenceMsg.Size()){
		    m_UrgenceMsg.Pop(E);
		}
		else if(Size()){
			ePipeline::Pop(E);
		}
		m_LastPopTimeStamp = CreateTimeStamp();
	}
	CMsg Pop(){
		CLock lk(m_Mutex,this);
		if(m_UrgenceMsg.Size()){
			m_LastPopTimeStamp = CreateTimeStamp();
			return m_UrgenceMsg.PopMsg();
		}
		else if(Size()){
			m_LastPopTimeStamp = CreateTimeStamp();
			return ePipeline::PopMsg();
		}
		CMsg m(NULL);
		return m;
	}
    /*
      NOTE: 数据可能无效，用户必须自己检查
   
	eData<ePipeline> Pop(){	
		CLock lk(m_Mutex);		
		eData<ePipeline> Data;
		
		if(m_UrgenceMsg.Size()){
		    m_UrgenceMsg.Pop(&Data);
			return Data;
		}

		if(m_NormalMsg.Size()){
			m_NormalMsg.Pop(&Data);
		    return Data;
		}
		return Data;
	}
 */
};
typedef CLockPipe Nerve;

}// namespace ABSTRACT	

#endif // _CLockPIPE_H
