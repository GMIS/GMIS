/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _BRAIN_H__
#define _BRAIN_H__

#include "AbstractSpace.h"
#include "PhysicSpace.h"

#include "LogicDialog.h"
#include "BrainMemory.h"
#include "MsgList.h"
#include "LogDatabase.h"
#include "NotifyMsgDef.h"
#include "Element.h"
#include "Poco/Net/HTTPServer.h"
#include "BrainWebsocket.h"

using namespace PHYSIC;

#define LOGIC_TASK 1
#define  BRAIN_MEMORY_CHECH_DIALOG_ID 1

class CLogicDialog;
class CObjectData;
class CLogicThread;
class CLogicTask;
class CElement;

//生成一个MSG_FROM_BRAIN信息，注意：Msg.m_ID = 子消息ID，Msg则包含可能有的数据
void  CreateBrainMsg(CMsg& NewMsg,ePipeline& Receiver,ePipeline& Msg,int64 EventID=0);
void  CreateBrainMsg(CMsg& NewMsg,int64 ReceiverID,ePipeline& MsgData,int64 EventID=0);


//helper class 简单调用Brain->NotifyDialogState(...)
class CNotifyDialogState: public ePipeline{
public:
	CNotifyDialogState(int64 NotifyID):ePipeline(NotifyID){ };
	~CNotifyDialogState(){};
	void Notify(CLogicDialog* Dialog); 
};

class CWebsocketLinkerList:public CClientLinkerList
{
private:
	virtual void   CreateLinker(CLinker& Linker,Model* Parent,int64 ID,ePipeline& Param){};
public:
	CWebsocketLinkerList(){};
	~CWebsocketLinkerList(){};	

	void   CreateLinker(CLinker& Linker,CBrain* Parent,int64 ID,const WebSocket& ws);
};

class CBrainEvent{
public:
	int64       m_TimeStamp;    
	int64		m_Interval;      //事件间隔
	int64		m_TickCount;     //间隔计数 
	bool        m_bOnce;         //是否一次性事件(注意：不考虑用Interval=0表示一次性事件，因为存在特定事件间隔的一次性事件）  
	int64       m_ClientEventID;
	ePipeline   m_ClientAddress; //事件客户的地址
	ePipeline   m_ClientExePipe; //临时保存事件客户的执行管道数据
#ifdef _DEBUG
	tstring     m_Memo;
#endif

public:
	CBrainEvent():m_Interval(TIME_SEC),m_TickCount(0),m_bOnce(false),m_ClientEventID(0){
		m_TimeStamp = AbstractSpace::CreateTimeStamp();		
	};
	virtual ~CBrainEvent(){};

	void ToPipeline(ePipeline& Pipe){
		Pipe.PushInt(m_TimeStamp);
		Pipe.PushInt(m_Interval);
		Pipe.PushInt(m_TickCount);
		Pipe.PushInt(m_bOnce);
		Pipe.PushInt(m_ClientEventID);
		Pipe.PushPipe(m_ClientAddress);
		Pipe.PushPipe(m_ClientExePipe);
	};

	void FromPipeline(ePipeline& Pipe){
		m_TimeStamp = Pipe.PopInt();
		m_Interval  = Pipe.PopInt();
		m_TickCount = Pipe.PopInt();
		m_bOnce     = Pipe.PopInt()!=0;
		m_ClientEventID = Pipe.PopInt();
		eElectron e;
		Pipe.Pop(&e);
		assert(e.EnergyType()== TYPE_PIPELINE);
		ePipeline* tempPipe = (ePipeline*)e.Value();
		m_ClientAddress << *tempPipe;
		Pipe.Pop(&e);
		tempPipe = (ePipeline*)e.Value();
		m_ClientExePipe << *tempPipe;
	}
};

class COrgan{
public:
	tstring		   m_Name;
	tstring        m_CryptTxt;
	int64          m_SourceID;
	DIALOG_TYPE    m_DialogType;
public:
	COrgan():m_DialogType(DIALOG_NULL),m_SourceID(0){};
	COrgan(tstring Name,tstring CryptTxt,int64 SourceID,DIALOG_TYPE Type)
		:m_Name(Name),m_CryptTxt(CryptTxt),m_SourceID(SourceID),m_DialogType(Type){};
	virtual ~COrgan(){};
};


enum { 
	BRAIN_WEBSOCKET_IO_WORK_TYPE =200, 
	BRAIN_EVENT_WORK_TYPE    
};

class CBrainThreadWorker : public CThreadWorker{

protected:
	void EventProc();
	void WebsocketWorkProc();
public:
	CBrainThreadWorker(int64 ID,Model* Parent,int32 Type);
	virtual ~CBrainThreadWorker();
	virtual bool Do(Energy* E);

};
class CBrain : public System  
{
public:
	tstring     m_ErrorInfo;

	class CLockedBrainData
	{
	private:
		CUserMutex					   m_BrainMutex;

		map<int64,CLogicDialog*>       m_DialogList; 
		deque<CLogicDialog*>           m_DialogPool;           //内存池，同时在多线程下保证有一定的存活期缓冲，避免直接删除带来奔溃
		
		map<int64,CLogicThread*>       m_LogicThreadList;      //当前使用的LogicThread
		deque<CLogicThread*>           m_LogicThreadPool;      //内存池，避免频繁的内存new/del

	    map<int64,CLogicTask*>         m_LogicTaskList;        //当前使用的LogicTask
		deque<CLogicTask*>             m_LogicTaskPool;        //内存池
 
		CUserMutex					   m_EventMutex;           //由于需要不停的遍历循环，所以设置一个专用锁  
		map<int64, CBrainEvent>        m_EventList;            //对话交由大脑中枢等待回复的事件列表

		list<COrgan>                   m_GUI_List;             //当前在线的User SourceID List

		map<int64,CThreadWorker*>      m_WebsocketIOWorkerList;  
		deque<CThreadWorker*>          m_ThreadWorkerPool;
		
		/*
		m_UserAccountList每一个子条目是一个
		ePipeline｛
			tstring: UserName;
			tstring: CrypStr;
			int:     DialogType;
			int:     DialogID;  default=-1
		｝
		
		*/
		ePipeline                      m_UAStaticList;      //从数据库中取出所有注册的用户信息      

		CLogicDialog* Interal_GetDialog(int64 SourceID,int64 DialogID); // no using m_BrainMutex
		void          Interal_DeleteDialog(int64 SourceID,int64 DialogID);
	public:
		CLockedBrainData(CBrain* Brain);
        ~CLockedBrainData();
		void Clear();

        CLogicDialog* CreateNewDialog(CBrain* Frame,int64 SourceID,int64 DialogID,
			                     int64 ParentDialogID,tstring SourceName,tstring DialogName,
								 DIALOG_TYPE Type,TASK_OUT_TYPE TaskType);

		//尽管实际只使用其中一个参数，但如此可以显式指出具体指哪个Dialog
		CLogicDialog* GetDialog(int64 SourceID,int64 DialogID); 
        
		//仅知道DialogID的情况下找到此Dialog
		CLogicDialog* FindDialog(int64 DialogID);

		void    DeleteDialog(int64 SourceID,int64 DialogID);
		int     DeleteDialogOfSource(int64 SourceID);  //属于这个SourceID的所有对话，返回个数
		void    DeleteAllDialog();

		//得到指定对话的对话历史,初始只给出最近20个对话信息
		void    GetFocusDialogData(int64 SourceID,int64 DialogID,ePipeline& Pipe,bool bIncludeDebugInfo);		
		void    GetInvalidDialogData(ePipeline& Pipe); //当点击的焦点对话无效时，返回一个缺省的空对话数据

		//在GetDialogLog()的基础上要求获得更多对话历史，一次也是20个
		void    GetMoreLog(int64 SourceID,int64 DialogID,int64 LastItemID,ePipeline& Pipe);

		//得到所有Dialog列表
		void    GetAllDialogListInfo(ePipeline& LinkerInfo);

		//事件TICK的间隔单位为百纳秒,最小间隔=MIN_EVENT_INTERVAL,也就是20毫秒，缺省为一秒
#ifdef _DEBUG	
		void    PushBrainEvent(int64 EventID,int64 ClientEventID,ePipeline& ExePipe,ePipeline& LocalAddress,int64 EventInterval, bool bEventOnce,tstring& Memo);
#else
		void    PushBrainEvent(int64 EventID,int64 ClientEventID,ePipeline& ExePipe,ePipeline& LocalAddress,int64 EventInterval, bool bEventOnce);
#endif
		bool    PopBrainEvent(int64 EventID,CBrainEvent& EventInfo);
        void    ResetEventTickCount(int64 EventID);
	
		//负责发出事件心跳反馈,并且去掉无效的事件
		void    EventProc(CBrain* Brain);

		CLogicThread* CreateLogicThread(int64 UserID,int64 ThreadID=0);
		CLogicThread* GetLogicThread(int64 ThreadID,bool bRequirLock = true);  //极少数情况会要求不锁，避免嵌套锁
		void          DeleteLogicThread(int64 ThreadID);
		void          SetLogicThreadUser(int64 ThreadID,int64 UserID);

		CLogicTask*   CreateLogicTask(int64 UserID,int64 TaskID=0); 
		CLogicTask*   GetLogicTask(int64 TaskID,bool bRequirLock = true);
		void          DeleteLogicTask(int64 TaskID);
		void          SetLogicTaskUser(int64 TaskID,int64 UserID);

		//资源回收，把无效的CLogicThread或者CBrainTask归还内存池，并限制内存池大小
		void    BrainIdleProc(CBrain* Brain);

		void	LoadUserAccountList();
		int		GetUserAccountNum();
		void	RegisterUserAccount(tstring Name,tstring CrypStr,DIALOG_TYPE DialogType);
		int64   DeleteUserAccount(tstring Name,tstring CrypStr);    //返回对应的SourceID
		bool	FindRegisterUserInfo(tstring Name,tstring Cryptograhp,ePipeline& UserInfo);
		void	UpdateUserDialogID(tstring Name,tstring Cryptograhp,int64 DialogID);

		void    RegisterOrgan(tstring UserName,tstring CryptTxt,int64 SourceID,DIALOG_TYPE Type);
		void    DelOrgan(int64 SourceID);
		const COrgan&  GetOrgan(int64 SourceID);

		void    SendMsgToGUI(CBrain* Brain,int64 SourceID,CMsg& Msg);
		void    SendMsgToGUI(CBrain* Brain,CMsg& Msg,int64 ExcludeSourceID);

		int32   GetWebIOWorkerNum();
		CThreadWorker* CreateThreadWorker(int64 ID,System* Parent,int32 Type);
		void   DeleteThreadWorker(System* Parent,int64 ID,int32 Type);
		void    WaitAllWorkThreadClosed();

		Energy* ToEnergy(); 
		bool    FromEnergy(CBrain* Brain,Energy* E);

		void    GetBrainStateInfo(ePipeline& Info);
	};

//////////////////////////////////////////////////////////////////////////
private:
	CLockedBrainData               m_BrainData;
	CBrainThreadWorker             m_EventWorker;
	HTTPServer*                    m_GUI_WebServer;  //to suppose websocket for web GUI	
 	CLogDatabase                   m_LogDB;
	CBrainMemory                   m_BrainMemory;
public:
	CWebsocketLinkerList           m_WebsocketClientList;
/////////////////////////////////////////////////////////
public:
	CBrain(CUserTimer* Timer,CUserSpacePool* Pool);
	
	virtual ~CBrain();
   
	virtual bool      Activate();
	virtual void      Dead();

	virtual Energy*   ToEnergy(); 
	virtual bool      FromEnergy(Energy* E); 

	void			  OutputLog(uint32 Flag,TCHAR* Format, ...);
	virtual void	  OutputLog(uint32 Flag,const TCHAR* text);

	void GetLinker(int64 SourceID,CLinker& Linker);

	virtual tstring	  MsgID2Str(int64 MsgID);

	//向本地用户界面发送消息，取决于之后具体的GUI实现，缺省什么也不做
	virtual void      SendMsgToLocalGUI(CMsg& GuiMsg){};   
	
	virtual void  NerveMsgProc(CMsg& Msg);
	
	//在Brain，这两个会产生博弈，因此要求生成其中一个会考虑另一方状态而决定是否同意
	virtual BOOL  CreateCentralNerveWorkerStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime);
	virtual BOOL  CreateNerveWorkerStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime);

	virtual void  CreateWebsokectWorkerStrategy();

	bool    RunExportObject(CLogicDialog* Dialog,CMsg* Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	void    CheckMemory();
//通用
//////////////////////////////////////////////////////////////////////////
	
	CLockedBrainData* GetBrainData();
	
	void	SendMsgToSpace(CMsg& Msg);
	void    ConnectSpace(CLogicDialog* Dialog,tstring& ip);

	//输出信息至系统缺省对话
    void	OutSysInfo(const TCHAR* s);

	//适合更复杂信息
	void	OutSysStatus(ePipeline& Data);
	
	//SourceID=0 表示系统自身，<1000为本地登录，其它为远程登录
	bool	Login(int64 SourceID,tstring UserName, tstring CrypStr);

	virtual void UnitTest();
 
//////////////////////////////////////////////////////////////////////////
	virtual void CentralNerveMsgProc(CMsg& Msg);

	void OnWhoAreYou(CLogicDialog* Dialog,CMsg& Msg);     //由Brain Listen到Linker后发出
	void OnI_AM(CLogicDialog* Dialog,CMsg& Msg);          //由对方接受到MSG_WHO_ARE_YOU后发来的回复
	void OnInitBrain(CLogicDialog* Dialog,CMsg& Msg);  
	void OnAppRun(CLogicDialog* Dialog,CMsg& Msg);
	void OnAppPause(CLogicDialog* Dialog,CMsg& Msg);
	void OnConnectOK(CLogicDialog* Dialog,CMsg& Msg);
	void OnGUI2Brain(CLogicDialog* Dialog,CMsg& Msg);
	void OnGetDialogList(CLogicDialog* Dialog,CMsg& Msg);
	void OnTaskFeedback(CLogicDialog* Dialog,CMsg& Msg);
	void OnSpaceCatalog(CLogicDialog* Dialog,CMsg& Msg);
	void OnSpaceAdded(CLogicDialog* Dialog,CMsg& Msg);
	void OnSpaceDeleted(CLogicDialog* Dialog,CMsg& Msg);
	void OnSpaceSendObject(CLogicDialog* Dialog,CMsg& Msg);
	void OnSpaceAccessDenied(CLogicDialog* Dialog,CMsg& Msg);
	void OnOperatorFeedback(CLogicDialog* Dialog,CMsg& Msg);
	void OnRobotExportObject(CLogicDialog* Dialog,CMsg& Msg);
	void OnTaskRequest(CLogicDialog* Dialog, CMsg& Msg);
//	void OnCloseDialog(CLogicDialog* Dialog,CMsg& Msg);


//通知信息处理
////////////////////////////////////////////////////////////////////////// 
	
	virtual void NotifySysState(int64 NotifyType,int64 NotifyID,ePipeline* Data = NULL);	
	virtual void NotifyLinkerState(int64 SourceID,int64 NotifyID,STATE_OUTPUT_LEVEL Flag,ePipeline& Info);	
	
	//NOTE:会调用Dialog.m_Mutex锁定，因此应该仅用于信息输出，避免递归嵌套导致死锁
	//目前共用此锁的函数还有：Set\GetTaskState,Set\GetWorkMode,GetSysProcNum
	virtual void NoitfyDialogState(CLogicDialog* Dialog, ePipeline* NotifyInfo);	
		void OnNotifyDialogError(CLogicDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyDialogState(CLogicDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyDialogOutput(CLogicDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyLogicView(CLogicDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyObjectList(CLogicDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyDialogList(CLogicDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyProgressOutput(CLogicDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyDebugView(CLogicDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyBrainInit(CLogicDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyFindInfo(CLogicDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyMemoryList(CLogicDialog* Dialog, ePipeline* NotifyInfo);

//处理任务请求
//////////////////////////////////////////////////////////////////////////
	virtual bool ProcessRequest(CLogicDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestEnd(CLogicDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestDialog(CLogicDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestUseObject(CLogicDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestTransTask(CLogicDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestExeLogic(CLogicDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestInsertLogic(CLogicDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestRemoveLogic(CLogicDialog* Dialog,ePipeline* RequestInfo);

};




#endif // !defined(_BRAIN_H__)
