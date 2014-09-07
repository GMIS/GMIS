/*
* author: ZhangHongBing(hongbing75@gmail.com)  
*  
*说明：
*Model比Object更独立，拥有从外部获得信息的能力，通常这指网络连接，当然也可能是其他串口之类。
*它的应用目标是一个具有信息输入输出能力的程序框架。从程序员角度看，Model是一个典型的Client。
*
*
*因此，Model必须负责建立连接，然后把通过连接的信息转换成本地格式信息，其后的信息处理则与Object
*相同。这一切对于用户是透明的，至于具体选择何种物理连接方式，由用户后期实现，这里避免提前对系统
*依赖。
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
	    list<CLinkerPipe*>          m_DelLinkerList; //等待被物理删除的列表
    
		CLockedLinkerList(){};
	public:
		CLockedLinkerList(CABMutex* mutex);
		~CLockedLinkerList();	


		//由于多线程使用，不直接删，先放入m_DelLinkerList中，当下一个被删除时，检查前一个删除对象是否可以删除
		bool          DeleteLinker(int64 ID);  
	
		void	      GetLinker(int64 ID,CLinker& Linker);
		void          GetNextLinker(int64 SourceID,CLinker& Linker); //得到SourceID的下一个，用于遍历列表
		void          AddLinker(CLinkerPipe* Linker); 
	
		void		  DeleteAllLinker(); //慎用，只在关闭整个系统时使用，并且确保不会有其它线程使用CLinkerPipe
	
		bool          SetLinkerID(int64 OldID,int64 NewID);

		int32         GetLinkerNum();
		int32         GetDelLinkerNum();
	};


	//独立线程，处理对外网络IO，实际使用需要继承实现线程启动
	class CModelIOWork : public Object{
	protected:	
		Model*     m_Parent;
	public:
		CModelIOWork(int64 ID,Model* Parent);
		virtual ~CModelIOWork();
		virtual bool Do(Energy* E);

	};
		
	//独立线程，负责中枢神经信息的处理，实际使用需要继承实现线程启动
	class CCentralNerveWork : public Object{
	protected:			
		Model*	    m_Parent;
		int32       m_IdleCount;  //无信息可处理时的循环计数
	public:
		CCentralNerveWork(int64 ID,Model* Parent);
		virtual ~CCentralNerveWork();
		virtual bool Do(Energy* E);
	};

	class CLockedModelData{
	protected:
		CABMutex*					     m_pMutex;
		uint32                           m_MaxNerveWorkerNum;      //允许最大中枢神经处理线程数目，default=20
		uint32                           m_NerveMsgMaxNumInPipe;   //神经中枢中如果保留超过此数的信息考虑生成新的处理线程,default = 10
		int64                            m_NerveMsgMaxInterval;    //神经中枢最近取出的信息时间超过此数考虑生成新的处理线程,default=10*1000*1000(单位是百纳秒，也就是1秒)
		uint32                           m_NerveIdleMaxCount;	   //中枢神经处理线程如果空闲计数超过此数则退出，default=30
		uint32							 m_NerveWorkingNum;        

		map<int64,CModelIOWork*>		 m_ModelIOWorkList;        //处理与服务器的连接
		map<int64,CCentralNerveWork*>    m_CentralNerveWorkList;  
		list<Object*>                    m_DelThreadWorkList;      //等待被物理删除的列表(避免线程体自己执行自己的delete),包括ModelIOWork和CentralNerveWork

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
		
		bool    RequestCreateNewCentralNerveWork(uint32 MsgNum,int64 Interval,uint32& Reason); //返回理由 
	};

    //代替Model初始化列表，当用户实例化时承继此类即可增加参数，避免参数表越来越长
	class CModelInitData{
	public:
		tstring					  m_Name;
		CTimeStamp*               m_Timer;
		CAbstractSpacePool*       m_Pool;
		CLockedModelData*         m_ModelData;
		CABMutex*                 m_ModelListMutex; 
		CLockPipe*                m_CentralNerve;  
		int32                     m_nCPU;   //CPU数，有助于判断线程使用数目，非当前必须
	public:
		CModelInitData();
		virtual ~CModelInitData();
	};
private: 
	//中枢神经，所有信息进入此管道后才被处理，完全私有，防止派生类直接存取信息，因为存取信息时涉及到处理线程的调度	
	CLockPipe*           m_CentralNerve;

	//由于在多线程环境下，需要锁定，因此不鼓励直接引用，而是调用GetModelData()得到锁定后的引用;
	CLockedLinkerList    m_SuperiorList;               
    CLockedModelData*    m_ModelData;

	
	//检测状态决定是否需要增加Worker(线程），由PushCentralNerveMsg调用;
	//NewMsgPushTime=压入新信息后的当前时间，LastMsgPopTime则是最后取出信息的时间
	virtual BOOL  CentralNerveWorkStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime);
protected:

	uint32         m_LogFlag;  //default=0
	
	/*
	像系统申请线程，Reason目前有两种:
	一种是时间因素：管道里的等候的信息虽然数量不多，但最近一个已经超过预期时间没有取出，申请一个新worker去处理
	一个是数量因素：管道里等待的信息数量太多，现有线程不够用，申请一个新的
	通常,对于一般客服端可以不考虑reason,就是一个简单的线程产生函数
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

	//把信息压入中枢神经后调用CentralNerveWorkStrategy()
	void			   PushCentralNerveMsg(CMsg& Msg,bool bUrgenceMsg=FALSE);
	void			   PopCentralNerveMsg(CMsg& Msg);
	int32			   GetCentralNerveMsgNum();
	void			   GetCentralNerveMsgList(ePipeline& Pipe);


	//系统内部任何组成（主要是线程）直接通过此函数通知系统一些事情，DATA为通知的数据内容
	virtual void       NotifySysState(int64 NotifyID,ePipeline* Data = NULL)=0;

	//用户应该在派生类中实现具体的处理，并#define MY_MAIN_FRAME 派生类名，注意：Linker已经处于上锁状态
	virtual void       NotifyLinkerState(CLinkerPipe* Linker,int64 NotifyID,ePipeline& Data)=0;

     /* 连接指定地址，地址数据压入Pipe中，根据连接类型而不同
	
	  Client和Server的握手程序：
	  
		Client            ---Connect---->              Server
		|                                              | 连接成功
		|             <--MSG_WHO_ARE_YOU-----          | 询问对方
		|                                              |  
		|             ------MSG_I_AM -------->         | 得到Client的验证信息
		|                                              |
		|             <----MSG_CONNECT_OK-------       | 接受
		|				<----MSG_DICCONNECT--------	   | 或者拒绝    	
    */
	//virtual BOOL Connect(ePipeline* Pipe)=0;
	
  	
	/*承继自Object的工作线程,用户必须重载这个函数,它等于Model的中枢神经处理
	  用户的业务逻辑在这里实现，如果单线程不足以完成，则根据情况压入m_Nerver,
	  由NerverWork线程处理。

      这里应该处理最优先的信息，并负责调度其他组织一起合作工作

      注意：有信息处理返回TRUE,没有应该返回FALSE,以便通知系统关掉多余的线程
    */
	virtual bool Do(Energy* E=NULL){
		assert(E);

		//如果有对外网络连接，那么缺省应该处理以下信息（这里只是给出提示，没有处理）
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
