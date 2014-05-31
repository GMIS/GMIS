/*
*author: ZhangHongBing(hongbing75@gmail.com)  
*  
* ��ePipeline�ķ�װ�����Կ�����һ�����ڶ��̰߳�ȫ���У�
* �����������������Ϣ��
* �Դ�һ��������ݹܵ�����������ݹܵ������������ʹ��
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
	CABMutex*            m_Mutex;       //ȷ���̰߳�ȫ
	ePipeline            m_UrgenceMsg;
	int64                m_LastPopTimeStamp; //���ȡ����Ϣ��ʱ���
public:	

	CLockPipe(CABMutex* m,const wchar_t* Name=_T("Unkown"),uint64 ID=0)
		:ePipeline(Name,ID),
		 m_Mutex(m),
		 m_UrgenceMsg(_T("UrgenceMsg")),
		 m_LastPopTimeStamp(0)
	{};
	virtual ~CLockPipe(){
//		CLock lk(m_Mutex);  ����m_Mutex�Ǻ��ھ����ģ����ܱ��м�����ǰɾ���ʧЧ
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
	//ע�⣺������ַ��ʽΪ��type@len{type@len@data1,type@len@data2 ... type@len@dataN}
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
	   ��CCLockPipe��д����ݱ�����ePipeline
	   NOTE: ����ʹ��Push_Directly,����Data����ȡ�Զ�
	*/
	int64 Push(ePipeline* Data){
		assert(Data != NULL);
		CLock lk(m_Mutex,this);
		Push_Directly(Data);
		//=0���б�ʶ�״�ʹ�õ����ã������push�ܿ죬����û���ü�pop��m_LastPopTimeStampʼ��=0,
		//Ϊ�˱����������������-1�����push�ܿ��pop�������мӴ�ʱ����������
		return m_LastPopTimeStamp--; 
	};
	
	//������ݻᱻ����ȡ��
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
      NOTE: ��ݿ�����Ч���û������Լ����
   
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
