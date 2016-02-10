/*
*author: ZhangHongBing(hongbing75@gmail.com)  
*  
* A encapsulation of ePipeline can be used as a thread safe queue for caching the input and output data
*
* With an emergency data pipeline, if there are emergency data will be gived priority to process
*/

#ifndef _CLockPIPE_H
#define _CLockPIPE_H


#include "Typedef.h"
#include "Pipeline.h"
#include "ABMutex.h"
#include <string>
#include <assert.h>

namespace ABSTRACT{

 
class   CLockPipe: public ePipeline
{

protected:
	CABMutex*            m_Mutex;       // To ensure thread safe
	ePipeline            m_UrgenceMsg;
	int64                m_LastPopTimeStamp;
public:	

	CLockPipe(CABMutex* m,const wchar_t* Name=_T("Unkown"),uint64 ID=0)
		:ePipeline(Name,ID),
		 m_Mutex(m),
		 m_UrgenceMsg(_T("UrgenceMsg"),0),
		 m_LastPopTimeStamp(0)
	{};
	virtual ~CLockPipe(){
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

	void ClearData(){
		CLock lk(m_Mutex,this);
		Clear();
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
	//the string format is：type@len{type@len@data1,type@len@data2 ... type@len@dataN}
	bool   FromString(AnsiString& s,uint32& pos){ 
		CLock lk(m_Mutex,this);

		ePipeline Pipe;
		bool ret = Pipe.FromString(s,pos);
        if(!ret)return false;

		eElectron UrgPipe, NormalPipe;
		Pipe.Pop(&UrgPipe);
		Pipe.Pop(&NormalPipe);
		
        m_UrgenceMsg.Clear();
		m_UrgenceMsg<<(*(ePipeline*)UrgPipe.Value());

		Clear();
		(*this)<<(*(ePipeline*)NormalPipe.Value());
		return true;
	}
    /*
	Note: because using Push_Directly(),so the Data must be from memory heap
	*/
	int64 Push(ePipeline* Data){
		assert(Data != NULL);
		CLock lk(m_Mutex,this);
		Push_Directly(Data);
		return m_LastPopTimeStamp--; 
	};
	
	//the emergency data will be gived priority to process
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
	void Pop(CMsg& Msg ){
		CLock lk(m_Mutex,this);
		if(m_UrgenceMsg.Size()){
			m_LastPopTimeStamp = CreateTimeStamp();
			m_UrgenceMsg.PopMsg(Msg);
			return;
		}
		else if(Size()){
			m_LastPopTimeStamp = CreateTimeStamp();
			ePipeline::PopMsg(Msg);
			return;
		}
		Msg.Reset(NULL);
	}
    /*
      NOTE: the data may not be valid, users must check it before use
   
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
