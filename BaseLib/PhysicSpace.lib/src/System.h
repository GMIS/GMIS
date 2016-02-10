/*
* Used to implement the highest Mass' instance
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _SYSTEM_H__
#define _SYSTEM_H__

#include "UserLinkerPipe.h"
#include "Model.h"
#include <vector>
#include <map>
#include "ABMutex.h"
#include "SpaceMutex.h"



namespace PHYSIC{

	
enum { 
	SYSTEM_IO_WORK_TYPE = 100, 
	SYSTEM_NEVER_WORK_TYPE,
};

class CSysThreadWorker : public CThreadWorker{

protected:
	void SystemIOWorkProc();
	void NerveWorkProc();
public:
	CSysThreadWorker(int64 ID,Model* Parent,int32 Type);
	virtual ~CSysThreadWorker();
	virtual bool Do(Energy* E);

};

class CClientLinkerList:public CSuperiorLinkerList
{
protected:
	deque<CLinkerPipe*> m_LinkerPool;
	CSpaceMutex         m_ClientSitMutex;
	int32               m_PoolMaxRestSize; //default = 20;
public:
	CClientLinkerList();
	~CClientLinkerList();	

	virtual void   CreateLinker(CLinker& Linker,Model* Parent,int64 ID,ePipeline& Param);
	virtual bool   DeleteLinker(int64 SourceID);  
	virtual void   PopLinker(CLinker& Linker); 
	virtual void   ReturnLinker(CLinker& Linker);
};


class  System : public Model  
{
public:

	class CNetListenWorker: public Object{

	public:
		System*       m_Parent;
		int32		  m_Port;
#if defined(USING_POCO)
		ServerSocket  m_Socket;

#elif defined(USING_WIN32)
		SOCKET        m_Socket;
#endif

	public:
		CNetListenWorker(System* Parent, int32 Port);
		~CNetListenWorker();
		virtual bool Activation();
		virtual void Dead();
		virtual bool Do(Energy* E=NULL);
	};


	class CLockedSystemData{
	protected:
 		CUserMutex				   m_Mutex;
		uint32                     m_MaxNerveWorkerNum;      //Allowed maximum number of central nervous system processing threads，default=20
		uint32                     m_NerveMsgMaxNumInPipe;   //If more than this amount of messages were not handled,considering to generate a new processing thread,default = 10
		int64                      m_NerveMsgMaxInterval;    //if the interval  of the last popped message was greater than this number,considering to generate a new processing thread,default=10*1000*1000( 1 second)
		int32                      m_NerveIdleMaxCount;	     //If the count of idle threads exceeded this number, the extra thread will be exit，default=30

		uint32                     m_NerveWorkingNum;        
		
		map<int32,CNetListenWorker*> m_AcceptorList;
		deque<CNetListenWorker*>     m_AcceptorPool;

		map<int64,CThreadWorker*>  m_SystemIOWorkerList;  
		map<int64,CThreadWorker*>  m_NerveWorkerList;
		deque<CThreadWorker*>      m_ThreadWorkerPool;

	public:
		CLockedSystemData();
		virtual ~CLockedSystemData();
		void    Clear();

		void    IncreNerveWorkerCount();
        void    DecreNerveWorkerCount();

		int64   GetNerveMsgInterval();
		void    SetNerveMsgInterval(int32 n);
		
		int32	GetNerveMaxIdleCount();
		void	SetNerveMaxIdleCount(int32 n);
		
		int32   GetIOWorkerNum();

		CThreadWorker* CreateThreadWorker(int64 ID,System* Parent,int32 Type);
		void   DeleteThreadWorker(System* Parent,int64 ID,int32 Type);

		int32   GetNerveWorkerNum();
		int32   GetBusyNerveWorkerNum();
        int32   GetIdleWorkerNum(); //GetNerveWorkerNum()-GetBusyNerveWorkerNum();
	
		bool    RequestCreateNewNerveWorker(uint32 MsgNum,int64 Interval,uint32& Reason); 

		CNetListenWorker* CreateAcceptor(System* Parent,int32 Port);
		bool  HasAcceptor(int32 Port);
		void  DelAcceptor(int32 Port,bool bWaitDead);
		void  DelAllAcceptor();

		void  WaitAllWorkThreadClosed(System* Parent); //include CSystemIOWork CNerveWork CNetListenWork

	};

protected:
	//Some messages if the central nerve system does not handle , just leave it to nerve processing
	CUserMutex                      m_NerveMutex;
	CLockPipe                       m_Nerve;

	CClientLinkerList               m_ClientLinkerList;
	CLockedSystemData               m_SystemData;



	virtual BOOL CreateNerveWorkerStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime);
	
	virtual void CreateClientLinkerWorkerStrategy(int32 Port);

	//virtual void BroadcastMsg(set<int64>& SourceList,int64 BCS_ID,ePipeline& MsgData);


public:
	System(CUserTimer* Timer,CUserSpacePool* Pool);
	virtual MASS_TYPE  MassType(){ return MASS_SYSTEM;};
	virtual ~System();
	
    virtual bool Activation();
	virtual void Dead();

	//General nerve message processing function that need users to implement
	virtual void		NerveMsgProc(CMsg& Msg)=0;

	CClientLinkerList*    GetClientLinkerList();
	CLockedSystemData*    GetSystemData();

	virtual void       NotifySysState(int64 NotifyType,int64 NotifyID,ePipeline* Data);

    void	     PopNerveMsg(CMsg& Msg);
	int32		 GetNerveMsgNum();
	void		 GetNerveMsgList(ePipeline& Pipe);

	//bDirectly meant directly return when the msg pushed the queue without checking and starting work thread
	void		 PushNerveMsg(CMsg& Msg,bool bUrgenceMsg,bool bDirectly);

	//open a port and listen, allow to  open several ports at same time, the repeated port will be ignored
	bool OpenPort(int32 Port,tstring& error,bool bIP6);
	void ClosePort(int32 Port);

};

} //namespace ABSTRACT

#endif // !defined(_SYSTEM_H__)
