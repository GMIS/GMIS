/*
Model is more independent than the Object,it have the ability to obtain information from external which usually refers to a network connection or  other serial ports and so on.

Model's application target is a program framework that has an information input/output capability,  From the programmer's point of view, Model is a typical client.

Therefore, the Model must be responsible for establishing the connection and convert the information received from connection into local format. The information's using same to Object.

All of this is transparent to users, as for the specific physical connection method should be implemented later by user,here avoiding  beforehand the advanced system dependency.

author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _MODEL_H__
#define _MODEL_H__



#if defined(USING_POCO)

#include "Poco/Thread.h"
using namespace Poco;

#elif defined(USING_WIN32)

#include <process.h>

#else 
#error "Lack of specified platform #define(eg.USING_POCO or USING_WIN32 or other UserSpace implementation relied on your OS,default using USING_POCO because of it is cross-platform"

#endif

#include "LinkerPipe.h"
#include "Object.h"
#include "UserMutex.h"
#include "UserTimer.h"
#include "UserSpacePool.h"
#include "NotifyCodes.h"

#include <map>
#include <list>

using namespace std;


namespace PHYSIC{


#define    SPACE_PORT   911
#define    ROBOT_PORT   912

#define    MODEL_IO_BUFFER_SIZE  1024*64

//log flag

enum Log_Flag{
	LOG_MSG_IO_PUSH           =0x00000001, 
	LOG_MSG_I0_RECEIVED       =0x00000002,
	LOG_MSG_IO_SENDED         =0x00000004,
	LOG_MSG_IO_REMOTE_RECEIVED=0x00000008,
	LOG_MSG_CENTRL_NERVE_PUSH =0x00000010,
	LOG_MSG_NERVE_PUSH        =0x00000020,
	LOG_MSG_TASK_PUSH         =0x00000040,
	LOG_MSG_PROC_BEGIN        =0x00000080,
	LOG_MSG_PROC_PATH         =0x00000100,
	LOG_MSG_RUNTIME_TIP       =0x00000200,
	LOG_ERROR                 =0x00000400,
	LOG_WARNING               =0x00000800,
	LOG_TIP                   =0x00001000,
	LOG_EXCEPTION             =0x00002000
};


#define REASON_ALWAYS      0x0000
#define REASON_REFUSE       0x0002
#define REASON_TIME_OUT     0x0003
#define REASON_MSG_TOO_MUCH 0x0004
#define REASON_LIMIT        0x0005
#define REASON_WORKER_BUSY  0X0006
///////////////////////////////////////////////////////////////////////////

enum { 
	MODEL_IO_WORK_TYPE =1, 
	MODEL_CENTRAL_NEVER_WORK_TYPE,
	MODEL_TEST_WORK_TYPE
};

class CThreadWorker : public Object{
public:	
	Model*     m_Parent;
	int32      m_WorkType;
	int64      m_IdleCount;

protected:
	//handling for  network IO
	void ModelIOWorkProc();

	//handling for central nerve message
	void CentralNerveWorkProc();

	void UnitTest();
public:
	CThreadWorker(int64 ID,Model* Parent,int32 Type);
	virtual ~CThreadWorker();
	virtual bool Do(Energy* E);

};

class CSuperiorLinkerList
{
protected:
	CUserMutex                  m_Mutex;
	list<CLinkerPipe*>          m_LinkerList;
	list<CLinkerPipe*>          m_ActivelyLinker; //using by io thread
public:
	CSuperiorLinkerList();
	~CSuperiorLinkerList();	

	int32  GetLinkerNum();
	virtual void CreateLinker(CLinker& Linker,Model* Parent,int64 SourceID,ePipeline& Param);

	void   AddLinker(CLinkerPipe* Linker); 
    
	virtual bool   DeleteLinker(int64 SourceID);  
	
	bool   HasLinker(const AnsiString& Address,const int32 Port);

	void   GetLinker(int64 SourceID,CLinker& Linker);

	virtual void   PopLinker(CLinker& Linker);   //pop one ClinkerPipe using by io thread
	virtual void  ReturnLinker(CLinker& Linker);  //the io thread exited and return the CLinkerPipe to LinkerList

};

class  Model : public Object  
{
public:	
	
	class CLockedModelData{
	protected:
		CUserMutex					     m_Mutex;
		uint32                           m_NerveMsgMaxNumInPipe;   //if the amount of  messages in pipe more than it,considering to generate a new processing thread,default = 10
		int64                            m_NerveMsgMaxInterval;    //if the time interval of the last popped message was greater than it,considering to generate a new processing thread,default=10*1000*1000(the unit is hundred of nanoseconds, or 1 second)
		

		map<int64,CThreadWorker*>        m_CentralNerveWorkerList;  
		uint32                           m_MaxNerveWorkerNum;      //Allowed maximum number of threads to process central nerve msg， default=20
		uint32                           m_NerveIdleMaxCount;	   //If the count of idle of the thread exceed this number, the thread will exit,default=30

		map<int64,CThreadWorker*>		 m_ModelIOWorkerList;       //for connecting to the server

		deque<CThreadWorker*>            m_ThreadWorkerPool;
	public:
		CLockedModelData();
		~CLockedModelData();
       
		void    Clear();

		int64   GetNerveMsgInterval();
		void    SetNerveMsgInterval(int32 n);

		int32	GetNerveMaxIdleCount();
		void	SetNerveMaxIdleCount(int32 n);
		
		int32   GetCentralNerveWorkerNum();
								
		int32   GetIOWorkerNum();

		CThreadWorker* CreateThreadWorker(int64 ID,Model* Parent,int32 Type);
		void   DeleteThreadWorker(Model* Parent,int64 ID,int32 Type);
		
		void    WaitAllWorkerThreadClosed(Model* Parent);
		bool    RequestCreateNewCentralNerveWorker(uint32 MsgNum,int64 Interval,uint32& Reason); 
	};

protected: 
	/*The central nerve, all information coming in the pipeline before being processed, 
	  totally private, preventing the derived classes to access it directly,
	  because that comes to dealing with thread scheduling.
	*/
    CUserMutex           m_CentralNerveMutex;
	CLockPipe            m_CentralNerve;

	//Because in a multi-threaded environment, it need to lock, so we should call GetModelData () got a locked  reference  instead of direct use.
	CSuperiorLinkerList  m_SuperiorList;               
	CLockedModelData     m_ModelData;
	
	int32          m_nCPU;     //default = 2
	uint32         m_LogFlag;  //default = 0
	

	/*
	Detecting the state to determine whether system need to increase Workers (threads)，
	called by the PushCentralNerveMsg;
	*/
	virtual BOOL  CreateCentralNerveWorkerStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime);

public:
    Model(CUserTimer* Timer,CUserSpacePool* Pool);
	virtual ~Model();

	virtual MASS_TYPE  MassType(){ return MASS_MODEL;};
    virtual bool	   Activation();
	virtual void       Dead();
	
		
	void    SetLogFlag(uint32 flag);
	uint32  GetLogFlag();	
	int32   GetCpuNum();

	virtual void    CentralNerveMsgProc(CMsg& Msg)=0;

	//Type = OUTPUT_TEXT...if(Type&m_LogOutFlag)then output
	virtual void    OutputLog(uint32 Flag,const wchar_t* text)=0;
	
	virtual tstring MsgID2Str(int64 MsgID)=0;


	// note: Linker already in locked state
	virtual void       NotifyLinkerState(int64 SourceID,int64 NotifyID,STATE_OUTPUT_LEVEL Flag,ePipeline& Info)=0;
	
	virtual void       NotifySysState(int64 NotifyType,int64 NotifyID,ePipeline* Data);


    CSuperiorLinkerList* GetSuperiorLinkerList();
	CLockedModelData*    GetModelData();

	//it call CentralNerveWorkStrategy() after pushing the message into central nerve 
	//bDirectly meant directly return when the msg pushed the queue without checking and starting work thread
	void			   PushCentralNerveMsg(CMsg& Msg,bool bUrgenceMsg,bool bDirectly);
	void			   PopCentralNerveMsg(CMsg& Msg);

	int32			   GetCentralNerveMsgNum();
	void			   GetCentralNerveMsgList(ePipeline& Pipe);

	//allows to connect several servers at same time,  the same address and port will be ignored 
    /* Connect to a specified address, the address data that was in Pipe depend on the connection type
	
	  The handshake procedure of the Client and the Server:
	  
		Client            ---Connect---->              Server
		|                                              | Conection successful
		|             <--MSG_WHO_ARE_YOU-----          | Require authentication information
		|                                              |  
		|             ------MSG_I_AM -------->         | Get the authentication information
		|                                              |
		|             <----MSG_CONNECT_OK-------       | Accept
		|				<----MSG_DICCONNECT--------	   | Or refuse  	
    */
	bool Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock);

	//default central nerve worker
	virtual bool Do(Energy* E);
	
	//used for internal testing purposes only
	virtual void UnitTest();
};

}// namespace PHYSIC

#endif // !defined _MODEL_H__
