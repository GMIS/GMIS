/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _BRAIN_H__
#define _BRAIN_H__

#include "AbstractSpace.h"
#include "PhysicSpace.h"
#include <process.h>

#include "MsgList.h"
#include "LogDatabase.h"
#include "NotifyMsgDef.h"
//#include "Arm.h"

enum TASK_OUT_TYPE{
	TASK_OUT_DEFAULT, //缺省输出行为结果
	TASK_OUT_THINK,   //输出逻辑思考结果
};

enum MsgProcState{ RETURN_DIRECTLY,CONTINUE_TASK};


#define LOGIC_TASK 1

//NOTE:时间戳的最小单位是百纳秒
#define TIME_SEC    10*1000*1000  //= 1百纳秒*10*1000*1000
#define TIME_MILlI  10*1000   

#define MIN_EVENT_INTERVAL 10*1000*20  //最小事件间隔为20毫秒

class CTaskDialog;
class CObjectData;
class CLogicThread;
class CBrainTask;
class CElement;

//生成一个MSG_FROM_BRAIN信息，注意：Msg.m_ID = 子消息ID，Msg则包含可能有的数据
void  CreateBrainMsg(CMsg& NewMsg,ePipeline& Receiver,ePipeline& Msg,int64 EventID=0);
void  CreateBrainMsg(CMsg& NewMsg,int64 ReceiverID,ePipeline& MsgData,int64 EventID=0);


//helper class 简单调用Brain->NotifyDialogState(...)
class CNotifyState: public ePipeline{
public:
	CNotifyState(int64 NotifyID):ePipeline(NotifyID){ };
	~CNotifyState(){};
	void Notify(CTaskDialog* Dialog); 
};

struct _ACCOUNT{
	tstring Name;
	tstring Password;
	tstring Confirm;
	tstring CrypText;
	int64   ip;
};


enum{
	EVENT_INTERVAL =0,
	EVENT_TICK_COUNT,
	EVENT_ADDRESS,
	EVENT_PIPE
};


class CBrain : public System  
{

public:
	tstring     m_ErrorInfo;

//	static CArm m_LeftArm;

	class CLockedBrainData
	{
	private:
		CUserMutex					   m_BrainMutex;

		map<int64,CTaskDialog*>        m_DialogList; 
		deque<CTaskDialog*>            m_DialogPool;           //内存池，同时在多线程下保证有一定的存活期缓冲，避免直接删除带来奔溃
		
		map<int64,CLogicThread*>       m_LogicThreadList;      //当前使用的LogicThread
		deque<CLogicThread*>           m_LogicThreadPool;      //内存池，避免频繁的内存new/del

	    map<int64,CBrainTask*>         m_LogicTaskList;        //当前使用的LogicTask
		deque<CBrainTask*>             m_LogicTaskPool;        //内存池
 
		bool                           m_bCheckPool;           //用于BrainIdleProc()，避免多个线程执行此步骤


		CUserMutex					   m_EventMutex;           //由于需要不停的遍历循环，所以设置一个专用锁  
		map<int64, ePipeline>          m_EventList;            //对话交由大脑中枢等待回复的事物列表

		bool                           m_bCheckEvent;          //用于EventProc避免多个线程执行此步骤

		set<int64>                     m_GuiSourceList;        //用户控制界面源ID列表
		ePipeline                      m_ExtraData;	           //保留数据
	public:
		CLockedBrainData();
        ~CLockedBrainData();


        CTaskDialog* CreateNewDialog(CBrain* Frame,int64 SourceID,int64 DialogID,
			                     int64 ParentDialogID,tstring SourceName,tstring DialogName,
								 DIALOG_TYPE Type,int64 OutputSourceID,TASK_OUT_TYPE TaskType);

		CTaskDialog* GetDialog(int64 DialogID);
        
		void    DeleteDialog(int64 DialogID);
		int     DeleteDialogOfSource(int64 SourceID);  //属于这个SourceID的所有对话，返回个数
		void    DeleteAllDialog();

		//得到指定对话的对话历史，初始只给出最近20个对话信息
		void    GetFocusDialogData(int64 SourceID,int64 DialogID,ePipeline& Pipe,bool bIncludeDebugInfo);		
		
		//在GetDialogLog()的基础上要求获得更多对话历史，一次也是20个
		void    GetMoreLog(int64 SourceID, int64 DialogID,int64 LastItemID,ePipeline& Pipe);

		//得到所有Dialog列表
		void    GetAllDialogListInfo(ePipeline& LinkerInfo);

		

		//事件TICK的间隔单位为百纳秒，最小间隔=MIN_EVENT_INTERVAL,也就是20毫秒，缺省为一秒
		void    PushBrainEvent(int64 EventID,ePipeline& ExePipe,ePipeline& LocalAddress,int64 EventInterval=TIME_SEC);
		bool    PopBrainEvent(int64 EventID,ePipeline& ObjectInfo);
        void    ResetEventTickCount(int64 EventID);
	
		//负责发出事件心跳反馈，并且去掉无效的事件
		void    EventProc(CBrain* Brain);

		CLogicThread* CreateLogicThread(int64 UserID,int64 ThreadID=0);
		CLogicThread* GetLogicThread(int64 ThreadID,bool bRequirLock = true);  //极少数情况会要求不锁，避免嵌套锁
		void          DeleteLogicThread(int64 ThreadID);
		void          SetLogicThreadUser(int64 ThreadID,int64 UserID);

		CBrainTask*   CreateLogicTask(int64 UserID,int64 TaskID=0); 
		CBrainTask*   GetLogicTask(int64 TaskID,bool bRequirLock = true);
		void          DeleteLogicTask(int64 TaskID);
		void          SetLogicTaskUser(int64 TaskID,int64 UserID);

		//资源回收，把无效的CLogicThread或者CBrainTask归还内存池，并限制内存池大小
		void    BrainIdleProc(CBrain* Brain);

		//登记一个经过验证的用户界面
		void    AddGuiSourceID(int64 ID);
		bool    IsAllowedSourceID(int64 ID);

		Energy* ToEnergy(); 
		bool    FromEnergy(CBrain* Brain,Energy* E);

		void  GetBrainInfo(ePipeline& Info);
	};

//////////////////////////////////////////////////////////////////////////
private:
	CLockedBrainData               m_BrainData;

	int64                          m_CurDebugTimeStamp;
protected:
 	CLogDatabase                   m_LogDB;
	
	_ACCOUNT                       m_Account;
/////////////////////////////////////////////////////////
public:
	CBrain(CSystemInitData* InitData);
	
	virtual ~CBrain();
	
	virtual tstring MsgID2Str(int64 MsgID);

	virtual bool      Activation();
	virtual void      Dead();

	virtual Energy*   ToEnergy(); 
	virtual bool      FromEnergy(Energy* E); 

	void OutputLog(uint32 Flag,TCHAR* Format, ...);
	virtual void  OutputLog(uint32 Flag,const TCHAR* text);

	//向用户界面发送消息,这是Brain唯一与用户界面打交道的接口,缺省什么也不做
	virtual void      SendMsgToGUI(int64 GuiID,CMsg& GuiMsg);   


	_ACCOUNT* GetAccount(){
		return &m_Account;
	}
//继承的接口
//////////////////////////////////////////////////////////
	//神经中枢处理总入口，由主线程调用
	
	virtual bool Do(Energy* E); 
	virtual void NerveProc(CMsg& Msg);
	
	//在Brain，这两个会产生博弈，因此要求生成其中一个会考虑另一方状态而决定是否同意
	virtual System::CNerveWork*	      CreateNerveWorker(int64 ID,System* Parent,uint32 Reason);
	virtual Model::CCentralNerveWork* CreateCentralNerveWorker(int64 ID, Model* Parent,uint32 Reason);
	

//通用函数
//////////////////////////////////////////////////////////////////////////
	
	CLockedBrainData* GetBrainData();
	
	void	SendMsgToSpace(CMsg& Msg);
	void    ConnectSpace(CTaskDialog* Dialog,tstring& ip);

	//输出信息至系统缺省对话
	void	OutSysInfo(tstring s);
    void	OutSysInfo(const TCHAR* s);
	
	//适合更复杂信息
	void	OutSysStatus(ePipeline& Data);
	


	bool    RunExportObject(CTaskDialog* Dialog,CMsg* Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

    //向TYPE指出范围的连接转发信息,比如Type=BCS_ALL_LINKER，表示向所有连接广播此信息
    //转发会使用统一的信息MSG_BRODCAST_MSG,然后把欲转发的信息放入Letter
	

//任务对话的信息处理
//主要负责处理任务对话信息，
//原本应该放在CTaskDialog，但这样一来迫使CTaskDialog被承继，统一放在这里承继处理则更方便。
//////////////////////////////////////////////////////////////////////////
	
	virtual void TaskDialogProc(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);		
		void OnBroadcaseMsg(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);  //对方BroadcastMsg()发来的信息
		void OnCloseDialog(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
		void OnMsgFromBrain(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);    //处理用户界面发来的信息
			void OnBrainTextInputing(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainTextInputEnd(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainGetMoreLog(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainSetFocusDialog(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainTaskControl(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainGetDebugItem(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainLogicOperate(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainObjectOperate(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainMemoryOperate(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainGetThinkResult(CTaskDialog* Dialog, ePipeline* ExePipe,CMsg& Msg);
			void OnBrainGetAnalyseResult(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainClearDialogOutput(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainGetFindResult(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainClearThink(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);		
			void OnBrainConnectTo(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainDisconnect(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
			void OnBrainSetLogFlag(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg);
	   void OnTaskResult(CTaskDialog*Dialog,ePipeline* ExePipe,CMsg& Msg);
	   void OnEventTick(CTaskDialog*Dialog,ePipeline* ExePipe,CMsg& Msg);



//负责处理神经中枢信息
//与TaskDialogProc类似，只不过优先执行TaskDialogProc不会处理到的一些信息
//最终也会调用TaskDialogProc
//////////////////////////////////////////////////////////////////////////

	virtual void BrainDialogProc(CTaskDialog* Dialog,CMsg& Msg);
		void OnLinkerError(CTaskDialog* Dialog,CMsg& Msg);   //通常连接中断时发出
		void OnWhoAreYou(CTaskDialog* TASK,CMsg& Msg);       //由Brain Listen到Linker后发出
		void OnI_AM(CTaskDialog* Dialog,CMsg& Msg);          //由对方接受到MSG_WHO_ARE_YOU后发来的回复
		void OnInitBrain(CTaskDialog* Dialog,CMsg& Msg);  
		void OnAppRun(CTaskDialog* Dialog,CMsg& Msg);
		void OnAppPause(CTaskDialog* Dialog,CMsg& Msg);
		void OnConnectOK(CTaskDialog* Dialog,CMsg& Msg);
		void OnGUI2Brain(CTaskDialog* Dialog,CMsg& Msg);
		void OnGetLinkerList(CTaskDialog* Dialog,CMsg& Msg);
		void OnTaskFeedback(CTaskDialog* Dialog,CMsg& Msg);
		void OnSpaceCatalog(CTaskDialog* Dialog,CMsg& Msg);
		void OnSpaceAdded(CTaskDialog* Dialog,CMsg& Msg);
		void OnSpaceDeleted(CTaskDialog* Dialog,CMsg& Msg);
		void OnSpaceSendObject(CTaskDialog* Dialog,CMsg& Msg);
		void OnSpaceAccessDenied(CTaskDialog* Dialog,CMsg& Msg);
		void OnOperatorFeedback(CTaskDialog* Dialog,CMsg& Msg);
		void OnRobotExportObject(CTaskDialog* Dialog,CMsg& Msg);
		void OnTaskRequest(CTaskDialog* Dialog, CMsg& Msg);


		
//负责处理组织信息
////////////////////////////////////////////////////////////////////////// 
   virtual MsgProcState ElementMsgProc(CTaskDialog* Dialog,CElement* Elt,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
		MsgProcState OnEltTaskControl(CTaskDialog* Dialog,CElement* Elt,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
		MsgProcState OnEltInsertLogic(CTaskDialog* Dialog,CElement* Elt,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
		MsgProcState OnEltRemoveLogic(CTaskDialog* Dialog,CElement* Elt,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
        MsgProcState OnEltTaskResult(CTaskDialog* Dialog,CElement* Elt,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
//通知信息处理
////////////////////////////////////////////////////////////////////////// 
	
	virtual void NotifySysState(int64 NotifyID,ePipeline* Data = NULL);	
	virtual void NotifyLinkerState(CLinkerPipe* Linker,int64 NotifyID,ePipeline& Info);	
	
	//NOTE:会调用Dialog.m_Mutex锁定，因此应该仅用于信息输出，避免递归嵌套导致死锁
	//目前共用此锁的函数还有：Set\GetTaskState,Set\GetWorkMode,GetSysProcNum
	virtual void NoitfyDialogState(CTaskDialog* Dialog, ePipeline* NotifyInfo);	
		void OnNotifyDialogError(CTaskDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyDialogState(CTaskDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyDialogOutput(CTaskDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyLogicView(CTaskDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyObjectList(CTaskDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyDialogList(CTaskDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyProgressOutput(CTaskDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyDebugView(CTaskDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyBrainInit(CTaskDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyFindInfo(CTaskDialog* Dialog, ePipeline* NotifyInfo);
		void OnNotifyMemoryList(CTaskDialog* Dialog, ePipeline* NotifyInfo);

//处理任务请求
//////////////////////////////////////////////////////////////////////////
	virtual bool ProcessRequest(CTaskDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestEnd(CTaskDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestDialog(CTaskDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestUseObject(CTaskDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestTransTask(CTaskDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestExeLogic(CTaskDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestInsertLogic(CTaskDialog* Dialog,ePipeline* RequestInfo);
		bool OnRequestRemoveLogic(CTaskDialog* Dialog,ePipeline* RequestInfo);

};




#endif // !defined(_BRAIN_H__)
