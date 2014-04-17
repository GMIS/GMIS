/*
* 对现实世界最高层次的抽象，具有完整的输出输出能力
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _SYSTEM_H__
#define _SYSTEM_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Model.h"
#include <vector>
#include <map>
#include "ABMutex.h"
#include "SpaceMutex.h"


#define SNOTIFY_NERVE_MSG_NUM               20000
#define SNOTIFY_NERVE_THREAD_JOIN			20001
#define SNOTIFY_NERVE_THREAD_CLOSE          20002
#define SNOTIFY_NERVE_THREAD_LIMIT          20003
#define SNOTIFY_NERVE_THREAD_FAIL           20004
#define SNOTIFY_LISTEN_FAIL                 20005
#define SNOTIFY_IO_WORK_THREAD_CLOSE        20006

namespace ABSTRACT{
class  System : public Model  
{
public:
	class CSystemIOWork : public Object{
	public:
		System*      m_Parent;
	public:
		CSystemIOWork(int64 ID,System* Parent);
		virtual ~CSystemIOWork();
		virtual bool Do(Energy* E);
	};

	class CNerveWork : public Object{
	protected:			
		System*	     m_Parent;
		int32        m_IdleCount;
	public:
		CNerveWork(int64 ID,System* Parent);
		virtual ~CNerveWork();
		
		virtual bool Do(Energy* E);
	};

	class CLockedSystemData{
	protected:
 		CABMutex*				   m_pMutex;
		int32                      m_MaxNerveWorkerNum;      //允许最大中枢神经处理线程数目，default=100
		int32                      m_NerveMsgMaxNumInPipe;   //神经中枢中如果保留超过此数的信息考虑生成新的处理线程,default = 10
		int64                      m_NerveMsgMaxInterval;    //神经中枢最近取出的信息时间超过此数考虑生成新的处理线程,default=10*1000*1000(单位是百纳秒，1秒)
		int32                      m_NerveIdleMaxCount;	     //中枢神经处理线程如果空闲计数超过此数则退出，default=30

		int32                      m_NerveWorkingNum;        //正在工作的，也就是被工作逻辑占用的
		map<int64,CSystemIOWork*>  m_SystemIOWorkList;  
		map<int64,CNerveWork*>     m_NerveWorkList;

		list<Object*>              m_DelThreadWorkList;      //等待被物理删除的列表(避免线程体自己执行自己的delete),包括CSystemIOWork和CNerveWork

		CLockedSystemData(){};
	public:
		CLockedSystemData(CABMutex* mutex);
		virtual ~CLockedSystemData();

		virtual bool Activation();
		virtual void Dead();

		void    IncreNerveWorkCount();
        void    DecreNerveWorkCount();

		int64   GetNerveMsgInterval();
		void    SetNerveMsgInterval(int32 n);
		
		int32	GetNerveMaxIdleCount();
		void	SetNerveMaxIdleCount(int32 n);
		
		int32   GetNerveWorkNum();
		int32   GetBusyNerveWorkNum();
        
		int32   GetIOWorkNum();
		int32   AddIOWork(CSystemIOWork* Work);
		int32   DeleteIOWork(int64 ID);
		
		int32   AddNerveWork(CNerveWork* Work);
        int32   DeleteNerveWork(int64 ID);
			
		bool    RequestCreateNewNerveWork(int32 MsgNum,int64 Interval,uint32& Reason); //返回理由 

	};

    //代替System初始化列表，当用户实例化时承继此类即可增加参数，避免参数表越来越长
	class CSystemInitData:public Model::CModelInitData{
	public:
		CLockedSystemData*  m_SystemData;
		CABMutex*		    m_SystemListMutex;
		CSpaceMutex*		m_ClientSitMutex;
		CLockPipe*          m_Nerve;
		
	public:
		CSystemInitData();
		virtual ~CSystemInitData();
	};

    friend CSystemIOWork;
	friend CNerveWork;
private:
	//一般神经，中枢神经如果不直接处理信息就把信息交给一般神经，然后有多线程负责信息的处理,
	CLockPipe*                      m_Nerve;
	CLockedLinkerList               m_ClientList;
	CLockedSystemData*              m_SystemData;

protected:	

	CSpaceMutex*					m_ClientSitMutex;  //Child CLinkerPipe 公用锁

    virtual void		NerveProc(CMsg& Msg)=0;//一般神经对信息的处理函数，用户需要自己实现
	virtual BOOL		NerveWorkStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime);
	virtual CNerveWork* CreateNerveWorker(int64 ID,System* Parent,uint32 Reason)=0;


	virtual void BroadcastMsg(set<int64>& SourceList,int64 BCS_ID,ePipeline& MsgData);
	

public:
//	System(CLockedSystemData* SystemData,CABMutex* ModelMutex, CABMutex* ModelListMutex,CABMutex* SysMutex, CABMutex* SystemListMutex, CSpaceMutex* ChildSitMutex, CLockPipe* CentralNerve, CLockPipe* Nerve,const TCHAR* Name);
	System(CSystemInitData* InitData);
	virtual MASS_TYPE  MassType(){ return MASS_SYSTEM;};
	virtual ~System();
	
    virtual bool Activation();
	virtual void Dead();

	CLockedLinkerList*    GetClientLinkerList();
	CLockedSystemData*    GetSystemData();

	CLinker GetLinker(int64 ID);

	CMsg		 PopNerveMsg();
	int32		 GetNerveMsgNum();
	void		 GetNerveMsgList(ePipeline& Pipe);
	void		 PushNerveMsg(CMsg& Msg,bool bUrgenceMsg=FALSE);
};

} //namespace ABSTRACT

#endif // !defined(AFX_SYSTEM_H__C57907E3_4F12_11DA_8739_F810D83B8278__INCLUDED_)
