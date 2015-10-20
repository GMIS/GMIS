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


#define SNOTIFY_NERVE_MSG_NUM               20000
#define SNOTIFY_NERVE_THREAD_JOIN			20001
#define SNOTIFY_NERVE_THREAD_CLOSE          20002
#define SNOTIFY_NERVE_THREAD_LIMIT          20003
#define SNOTIFY_NERVE_THREAD_FAIL           20004
#define SNOTIFY_LISTEN_FAIL                 20005
#define SNOTIFY_IO_WORK_THREAD_CLOSE        20006


namespace PHYSIC{

class  System : public Model  
{
public:

	class CNetListenWork: public Object{

	protected:
		System*       m_Parent;
		int32		  m_Port;
#if defined(USING_POCO)
		ServerSocket  m_Socket;

#elif defined(USING_WIN32)
		SOCKET        m_Socket;

#endif

	public:
		CNetListenWork(System* Parent, int32 Port);
		~CNetListenWork();
		virtual bool Activation();
		virtual void Dead();
		virtual bool Do(Energy* E=NULL);
	};


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
 		CUserMutex				   m_Mutex;
		uint32                     m_MaxNerveWorkerNum;      //Allowed maximum number of central nervous system processing threads，default=20
		uint32                     m_NerveMsgMaxNumInPipe;   //If more than this amount of messages were not handled,considering to generate a new processing thread,default = 10
		int64                      m_NerveMsgMaxInterval;    //if the time interval  of the last popped message was greater than this number,considering to generate a new processing thread,default=10*1000*1000(the unit is hundred of nanoseconds, or 1 second)
		int32                      m_NerveIdleMaxCount;	     //If the count of idle threads exceeded this number, the extra thread will be exit，default=30

		uint32                     m_NerveWorkingNum;        //Number of threads used by task logic
		map<int64,CSystemIOWork*>  m_SystemIOWorkList;  
		map<int64,CNerveWork*>     m_NerveWorkList;
		map<int32,CNetListenWork*> m_AcceptorList;

		bool                       m_bClosed;	
	public:
		CLockedSystemData();
		virtual ~CLockedSystemData();

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
		

		bool    RequestCreateNewNerveWork(uint32 MsgNum,int64 Interval,uint32& Reason); 

		CNetListenWork* FindAcceptor(int32 Port);
		void  AddAcceptor(int32 Port,CNetListenWork* Acceptor);
		void  DelAcceptor(int32 Port);
		void  DelAllAcceptor();

		void    CloseAllWorkThread(); //include CSystemIOWork CNerveWork CNetListenWork
	};

    friend class CSystemIOWork;
	friend class CNerveWork;
private:
	//Some messages if the central nerve system does not handle , just leave it to nerve processing
	CUserMutex                      m_NerveMutex;
	CLockPipe                       m_Nerve;

	CLockedLinkerList               m_ClientList;
	CLockedSystemData               m_SystemData;

protected:	

	CSpaceMutex 					m_ClientSitMutex;  //Child CLinkerPipe common lock

    virtual void		NerveProc(CMsg& Msg){;//General nerve message processing function that need users to implement
	
	};

	virtual BOOL		NerveWorkStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime);
	virtual CNerveWork* CreateNerveWorker(int64 ID,System* Parent,uint32 Reason);

	//used for Accept()
	CUserLinkerPipe* CreateClientLinkerPipe();
	
	virtual void BroadcastMsg(set<int64>& SourceList,int64 BCS_ID,ePipeline& MsgData);
	
	virtual void NotifySysState(int64 NotifyID,ePipeline* Data = NULL);

public:
	System(CUserTimer* Timer,CUserSpacePool* Pool);
	virtual MASS_TYPE  MassType(){ return MASS_SYSTEM;};
	virtual ~System();
	
    virtual bool Activation();
	virtual void Dead();

	CSpaceMutex*          GetSpaceMutex();
	CLockedLinkerList*    GetClientLinkerList();
	CLockedSystemData*    GetSystemData();

	void GetLinker(int64 ID,CLinker& Linker );

    void	     PopNerveMsg(CMsg& Msg);
	int32		 GetNerveMsgNum();
	void		 GetNerveMsgList(ePipeline& Pipe);
	//bDirectly meant directly return when the msg pushed the queue without checking and starting work thread
	void		 PushNerveMsg(CMsg& Msg,bool bUrgenceMsg,bool bDirectly);

	//Allows to connect several servers at same time, if address and port were the same then to ignore
	bool Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock);

	//open a port and listen, allow to  open several ports at same time, if port repeated then to ignore
	bool OpenPort(int32 Port,tstring& error,bool bIP6);
	void ClosePort(int32 Port);

};

} //namespace ABSTRACT

#endif // !defined(_SYSTEM_H__)
