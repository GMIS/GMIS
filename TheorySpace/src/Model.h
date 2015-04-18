/*
Model is more independent than the Object,it have the ability to obtain information from external which usually refers to a network connection or  other serial ports and so on.

Model's application target is a program framework that has an information input/output capability,  From the programmer's point of view, Model is a typical client.

Therefore, the Model must be responsible for establishing the connection and convert the information received from connection into local format. The information's using same to Object.

All of this is transparent to users, as for the specific physical connection method should be implemented later by user,here avoiding  beforehand the advanced system dependency.

author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _MODEL_H__
#define _MODEL_H__


#include "LinkerPipe.h"
#include "Object.h"
#include <map>
#include <list>

using namespace std;


namespace ABSTRACT{


#define  SPACE_PORT   911
#define  ROBOT_PORT   912

#define    BUFFER_SIZE  1024*64

//log flag

	
enum Log_Flag{
	LOG_MSG_IO_PUSH           =0x00000001, 
	LOG_MSG_I0_RECEIVED       =0x00000002,
	LOG_MSG_IO_SENDED         =0x00000004,
	LOG_MSG_IO_REMOTE_RECEIVED=0x00000008,
	LOG_MSG_CENTRL_NERVE_PUSH =0x00000010,
	LOG_MSG_NERVE_PUSH        =0x00000020,
	LOG_MSG_TASK_PUSH         =0x00000040,
	LOG_MSG_PROC_BEGIN        =0x00000100,
	LOG_MSG_PROC_PATH         =0x00000200,
	LOG_MSG_RUNTIME_TIP       =0x00000400,
	LOG_ERROR                 =0x00001000,
	LOG_WARNING               =0x00002000,
	LOG_TIP                   =0x00004000
};

//NotifyModelState()
#define MNOTIFY_CENTRAL_NERVE_MSG_NUM       10000
#define MNOTIFY_CENTRAL_NERVE_THREAD_JOIN   10001
#define MNOTIFY_CENTRAL_NERVE_THREAD_CLOSE  10002
#define MNOTIFY_CENTRAL_NERVE_THREAD_LIMIT  10003
#define MNOTIFY_CENTRAL_NERVE_THREAD_FAIL   10004
#define MNOTIFY_CONNECT_FAIL                10005
#define MNOTIFY_IO_WORK_THREAD_CLOSE        10006
#define MNOTIFY_EXCEPTION_OCCURRED          10007
#define MNOTIFY_ILLEGAL_MSG                 10008

#define REASEON_ALWAYS      0x0000
#define REASON_REFUSE       0x0002
#define REASON_TIME_OUT     0x0003
#define REASON_MSG_TOO_MUCH 0x0004
#define REASON_LIMIT        0x0005
#define REASON_WORKER_BUSY  0X0006
///////////////////////////////////////////////////////////////////////////


class  Model : public Object  
{
public:	
	class CLockedLinkerList
	{
	private:
		CABMutex*                   m_pMutex;
		map<int64,CLinkerPipe*>     m_LinkerList;     	
	    list<CLinkerPipe*>          m_DelLinkerList; //Waiting to be physically deleted 
    
		CLockedLinkerList(){};
	public:
		CLockedLinkerList(CABMutex* mutex);
		~CLockedLinkerList();	


		//Because using multithreaded, it not directly delete the linker but put it in the m_DelLinkerList. the linker in the m_DelLinkerList will be really removed when the next one is deleted.
		bool          DeleteLinker(int64 ID);  
	
		void	      GetLinker(int64 ID,CLinker& Linker);
		void          GetNextLinker(int64 SourceID,CLinker& Linker); //Get the next of the SourceID to traverse the list
		void          AddLinker(CLinkerPipe* Linker); 
	
		void		  DeleteAllLinker(); //Caution: used only when shutting down the system, and ensure that no other thread using CLinkerPipe

	
		bool          SetLinkerID(int64 OldID,int64 NewID);

		int32         GetLinkerNum();
		int32         GetDelLinkerNum();
	};


	//Separate thread, handling for external network IO, in actually using, it needs to  inherit this class to implement  thread  enabling.
	class CModelIOWork : public Object{
	protected:	
		Model*     m_Parent;
	public:
		CModelIOWork(int64 ID,Model* Parent);
		virtual ~CModelIOWork();
		virtual bool Do(Energy* E);

	};
		
	//Separate thread, handling for central nervous message, in actually using, it needs to  inherit this class to implement  thread  enabling.
	class CCentralNerveWork : public Object{
	protected:			
		Model*	    m_Parent;
		int32       m_IdleCount; 
	public:
		CCentralNerveWork(int64 ID,Model* Parent);
		virtual ~CCentralNerveWork();
		virtual bool Do(Energy* E);
	};

	class CLockedModelData{
	protected:
		CABMutex*					     m_pMutex;
		uint32                           m_MaxNerveWorkerNum;      //Allowed maximum number of central nervous system processing threads，default=20
		uint32                           m_NerveMsgMaxNumInPipe;   //If more than this amount of messages were not handled,considering to generate a new processing thread,default = 10
		int64                            m_NerveMsgMaxInterval;    //if the time interval  of the last popped message was greater than this number,considering to generate a new processing thread,default=10*1000*1000(the unit is hundred of nanoseconds, or 1 second)
		uint32                           m_NerveIdleMaxCount;	   //If the count of idle threads exceeded this number, the extra thread will be exit，default=30
		uint32							 m_NerveWorkingNum;        

		map<int64,CModelIOWork*>		 m_ModelIOWorkList;        //for connection to the server
		map<int64,CCentralNerveWork*>    m_CentralNerveWorkList;  
		list<Object*>                    m_DelThreadWorkList;      //Pointer list waiting to be physically deleted ( avoiding the thread delete itself), including ModelIOWork and CentralNerveWork
		CLockedModelData(){};
	public:
		CLockedModelData(CABMutex* mutex);
		~CLockedModelData();
       
		virtual bool Activation();
		virtual void Dead();

		void    IncreNerveWorkCount();
		void    DecreNerveWorkCount();

		int64   GetNerveMsgInterval();
		void    SetNerveMsgInterval(int32 n);

		int32	GetNerveMaxIdleCount();
		void	SetNerveMaxIdleCount(int32 n);
		
		int32   GetCentralNerveWorkNum();
        int32   GetBusyNerveWorkNum();

		int32   GetIOWorkNum();
		int32   AddIOWork(CModelIOWork* Work);
		int32   DeleteIOWork(int64 ID);
	 

		int32   AddCentralNerveWork(CCentralNerveWork* Work);
        int32   DeleteCentralNerveWork(int64 ID);
		
		bool    RequestCreateNewCentralNerveWork(uint32 MsgNum,int64 Interval,uint32& Reason); 
	};

    //For Model initialization,  we can add parameters through inheriting this class to  avoid the parameter table too longer
	class CModelInitData{
	public:
		tstring					  m_Name;
		CTimeStamp*               m_Timer;
		CAbstractSpacePool*       m_Pool;
		CLockedModelData*         m_ModelData;
		CABMutex*                 m_ModelListMutex; 
		CLockPipe*                m_CentralNerve;  
		int32                     m_nCPU;   //cpu number
	public:
		CModelInitData();
		virtual ~CModelInitData();
	};
private: 
	/*The central nerve, all information coming in the pipeline before being processed, 
	  totally private, preventing the derived classes to access it directly,
	  because that comes to dealing with thread scheduling.
	*/
	CLockPipe*           m_CentralNerve;

	//Because in a multi-threaded environment, it need to lock, so we should call GetModelData () got a locked  reference  instead of direct use.
	CLockedLinkerList    m_SuperiorList;               
    CLockedModelData*    m_ModelData;

	
	/*
	Detecting the state to determine whether system need to increase Workers (threads)，
	called by the PushCentralNerveMsg;
	NewMsgPushTime is the timestamp of last pushed message，
	LastMsgPopTime is the timestamp of last popped message
	*/
	virtual BOOL  CentralNerveWorkStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime);
protected:

	uint32         m_LogFlag;  //default=0
	
	/*
	Applying to the system thread, there are currently two reasons:
	One is the time factor:   if the time interval of the last popped message was greater than a certain  number,considering to generate a new processing thread	一个是数量因素：管道里等待的信息数量太多，现有线程不够用，申请一个新的
	Another is the quantity factor: if more than this amount of messages were not handled,considering to generate a new processing thread
	Generally, for general client, it just is a simple thread creater
    */
	virtual CCentralNerveWork* CreateCentralNerveWorker(int64 ID, Model* Parent,uint32 Reason)=0;

	
public:
    Model(CModelInitData* InitData);
	virtual ~Model();

	virtual MASS_TYPE  MassType(){ return MASS_MODEL;};
    virtual bool	   Activation();
	virtual void       Dead();

	void    SetLogFlag(uint32 flag);
	uint32  GetLogFlag();
	
	//Type = OUTPUT_TEXT...if(Type&m_LogOutFlag)then output
	virtual void  OutputLog(uint32 Flag,const wchar_t* text)=0;
	
	virtual tstring MsgID2Str(int64 MsgID)=0;

    CLockedLinkerList* GetSuperiorLinkerList();
	CLockedModelData*  GetModelData();

	//it call CentralNerveWorkStrategy() after pushing the message into central nerve 
	void			   PushCentralNerveMsg(CMsg& Msg,bool bUrgenceMsg=FALSE);
	void			   PopCentralNerveMsg(CMsg& Msg);
	int32			   GetCentralNerveMsgNum();
	void			   GetCentralNerveMsgList(ePipeline& Pipe);


	virtual void       NotifySysState(int64 NotifyID,ePipeline* Data = NULL)=0;

	//Users should implement it in a derived class, and #define MY_MAIN_FRAME derived class name, note: Linker already in locked state
	virtual void       NotifyLinkerState(CLinkerPipe* Linker,int64 NotifyID,ePipeline& Data)=0;

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
	//virtual BOOL Connect(ePipeline* Pipe)=0;
	
  	
	/*
	  Inherited from Object class, user must override this function,
	   which is responsible for central nerve processing of Model.
    */
	virtual bool Do(Energy* E=NULL){
		assert(E);

		//Below is just a sample
		//if(GetCentralNerveMsgNum()){
			ePipeline* Pipe = (ePipeline*)E;
			CMsg Msg(Pipe);		
/*			
		
			int64 MsgID  = Msg.GetMsgID();
			switch(MsgID)
			{
			case MSG_CONNECT_OK:
				break;
			case MSG_CONNECT_FAIL:
				break;
			case MSG_WHO_ARE_YOU:
				break;
			case MSG_RECEIVE_OK:
				//m_CentralNerve->Push(Msg);
				break;
			case MSG_RECEIVE_ERROR:
				break;
			case MSG_SEND_PROGRESS:
				break;
			case MSG_NEW_DIALOG:
				break;
			default:
				//m_CentralNerve->Push(Msg);
				break;
			}
*/
		//}
		return TRUE;	
	}
};

}// namespace ABSTRACT

#endif // !defined _MODEL_H__
