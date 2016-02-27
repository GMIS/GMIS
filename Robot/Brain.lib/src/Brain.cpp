// Brain.cpp: implementation of the CBrain class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)


#include "Brain.h"
#include "UserLinkerPipe.h"
#include "Msg.h"
#include "BrainObject.h"
#include "LogicThread.h"
#include "LogicTask.h"
#include "NotifyMsgDef.h"
#include "LogicDialog.h"
#include "GUIMsgDefine.h"
#include "BrainWebsocket.h"
#include <stdio.h>

void CreateBrainMsg(CMsg& NewMsg,ePipeline& Receiver,ePipeline& MsgData,int64 EventID){
	assert(MsgData.GetID() !=0);
	CMsg m(Receiver,MSG_FROM_BRAIN,EventID);
	ePipeline& Letter = m.GetLetter();
	Letter.PushInt(MsgData.GetID());
	Letter<<MsgData;

	NewMsg.Reset(m.Release());
}

void CreateBrainMsg(CMsg& NewMsg,int64 ReceiverID,ePipeline& Msg,int64 EventID){
	ePipeline Receiver;
	Receiver.PushInt(ReceiverID);
	CreateBrainMsg(NewMsg,Receiver,Msg,EventID);
};


CBrainThreadWorker::CBrainThreadWorker(int64 ID,Model* Parent,int32 Type)
	:CThreadWorker(ID,Parent,Type)
{
	assert(Type == BRAIN_WEBSOCKET_IO_WORK_TYPE || Type == BRAIN_EVENT_WORK_TYPE  );
}
CBrainThreadWorker::~CBrainThreadWorker(){

}

bool CBrainThreadWorker::Do(Energy* E){
	if (m_WorkType == BRAIN_WEBSOCKET_IO_WORK_TYPE)
	{
		WebsocketWorkProc();
	}else if (m_WorkType == BRAIN_EVENT_WORK_TYPE)
	{
		EventProc();	
	}
	return true;
}

void CBrainThreadWorker::EventProc(){
	CBrain* Brain = (CBrain*)m_Parent;
	Brain->GetBrainData()->EventProc(Brain);
}
void CBrainThreadWorker::WebsocketWorkProc(){
	CBrain* Brain = (CBrain*)m_Parent;
	try{
		    char buf[MODEL_IO_BUFFER_SIZE];
			while(IsAlive() && Brain->IsAlive()){

				int64 SourceID = 0;

				CLinker Linker;
				Brain->m_WebsocketClientList.PopLinker(Linker);

				
				if ( Linker.IsValid())
				{
					Linker().ThreadIOWorkProc(buf,MODEL_IO_BUFFER_SIZE);
					Brain->m_WebsocketClientList.ReturnLinker(Linker);
				}else{
					SLEEP_MILLI(20);
				}			
			}
		}
		/*
		catch(TimeoutException& e){
			AnsiString s = e.displayText();
			ePipeline Data;
			Data.PushString(s);
			m_Parent->NotifySysState(MNOTIFY_EXCEPTION_OCCURRED,&Data);
			//CLockedSystemData* LockedData = m_Parent->GetSystemData();
			//LockedData->DeleteIOWork(m_ID);
		}
		*/
		catch(...){
			ePipeline NotifyData;
			NotifyData.PushString(_T("An exception occurred, WebsocketIOWork Closed"));
			Brain->NotifySysState(NOTIFY_EXCEPTION_OCCURRED,NULL,&NotifyData);
		}

		ePipeline NotifyData;
		NotifyData.PushInt(m_ID);
		Brain->NotifySysState(NOTIFY_BRAIN_SCENE,NTID_IO_WORKER_CLOSED,&NotifyData);
}


void CWebsocketLinkerList::CreateLinker(CLinker& Linker,CBrain* Parent,int64 SourceID,const WebSocket& ws){
	_CLOCK(&m_Mutex);
	deque<CLinkerPipe*>::iterator it = m_LinkerPool.begin();
	if (it != m_LinkerPool.end())
	{
		CWebsocketLinkerPipe* UserPipe = (CWebsocketLinkerPipe*)*it;
		UserPipe->Reset();
		UserPipe->SetSourceID(SourceID);
		UserPipe->m_Socket = ws;
		Linker.Reset(UserPipe);
		m_LinkerPool.erase(it);
		return;
	}
	CWebsocketLinkerPipe* LinkPtr = new CWebsocketLinkerPipe(&m_ClientSitMutex,Parent,SourceID,ws);
	Linker.Reset(LinkPtr);	
};


void CNotifyDialogState::Notify(CLogicDialog* Dialog){	
	Dialog->m_Brain->NoitfyDialogState(Dialog,this);
    assert(Size()==0);
}


//CLockedBrainData
//////////////////////////////////////////////////////////////////////////
CBrain::CLockedBrainData::CLockedBrainData(CBrain* Brain)
{
   CLogicDialog* CentralDialog = CreateNewDialog(Brain,SYSTEM_SOURCE,DEFAULT_DIALOG,NO_PARENT,_T("System"),_T("System"),
		DIALOG_SYSTEM_MAIN,TASK_OUT_DEFAULT);

   CentralDialog->m_bEditValid = false;  //神经中枢不允许人工输入


   ePipeline ClientExePipe;
   ePipeline ClientAddress(SYSTEM_SOURCE);
   
   ClientAddress.PushInt(DEFAULT_DIALOG);

#ifdef _DEBUG
   tstring memo = Format1024(_T("%s %d"),_T(__FILE__),__LINE__);
   PushBrainEvent(DEFAULT_DIALOG,0,ClientExePipe,ClientAddress,TIME_SEC,true,memo);
#else
   PushBrainEvent(DEFAULT_DIALOG,0,ClientExePipe,ClientAddress,TIME_SEC,true);
#endif

}

CBrain::CLockedBrainData::~CLockedBrainData(){


};

void CBrain::CLockedBrainData::Clear(){
	_CLOCK(&m_BrainMutex);

	map<int64,CLogicThread*>::iterator  It_int64_thread = m_LogicThreadList.begin();
	while (It_int64_thread != m_LogicThreadList.end())
	{
		CLogicThread* Think = It_int64_thread->second;
		It_int64_thread->second = NULL;
		delete Think;
		It_int64_thread++;
	}
	m_LogicThreadList.clear();

	deque<CLogicThread*>::iterator It_thread = m_LogicThreadPool.begin();  
	while(It_thread != m_LogicThreadPool.end()){
		CLogicThread* Think = *It_thread;
		*It_thread = NULL;
		delete Think;
		It_thread++;
	}
	m_LogicThreadPool.clear();

	map<int64,CLogicTask*>::iterator  It_int64_braintask = m_LogicTaskList.begin();
	while (It_int64_braintask != m_LogicTaskList.end())
	{
		CLogicTask* Task = It_int64_braintask->second;
		delete Task;
		It_int64_braintask++;
	}
	m_LogicTaskList.clear();

	deque<CLogicTask*>::iterator It_braintask = m_LogicTaskPool.begin();  
	while(It_braintask != m_LogicTaskPool.end()){
		CLogicTask* Task = *It_braintask;
		*It_braintask = NULL;
		delete Task;
		It_braintask++;
	}
	m_LogicTaskPool.clear();

	map<int64,CLogicDialog*>::iterator  It_int64_taskdialog = m_DialogList.begin();
	while (It_int64_taskdialog != m_DialogList.end())
	{
		CLogicDialog* Dialog = It_int64_taskdialog->second;
		It_int64_taskdialog->second = NULL;
		delete Dialog;
		It_int64_taskdialog++;
	}
	m_DialogList.clear();

	deque<CLogicDialog*>::iterator It_taskdialog = m_DialogPool.begin();  
	while(It_taskdialog != m_DialogPool.end()){
		CLogicDialog* Dialog = *It_taskdialog;
		*It_taskdialog = NULL;
		Dialog->SetTaskState(TASK_DELELTE);
		delete Dialog;
		It_taskdialog++;
	}
	m_DialogPool.clear();
}

#ifdef _DEBUG

void CBrain::CLockedBrainData::PushBrainEvent(int64 EventID,int64 ClientEventID,ePipeline& ExePipe,ePipeline& LocalAddress,int64 EventInterval, bool bEventOnce,tstring& Memo){
	_CLOCK(&m_EventMutex);
	CBrainEvent& Event = m_EventList[EventID];

	Event.m_Interval = EventInterval<MIN_EVENT_INTERVAL? MIN_EVENT_INTERVAL:EventInterval;
	Event.m_ClientEventID = ClientEventID;
	Event.m_ClientAddress =  LocalAddress;
	Event.m_bOnce = bEventOnce;
	Event.m_ClientExePipe = ExePipe;

	Event.m_Memo = Memo;
	assert(Memo.size());
	assert(Event.m_ClientAddress.Size());
};

#else

void CBrain::CLockedBrainData::PushBrainEvent(int64 EventID,int64 ClientEventID,ePipeline& ExePipe,ePipeline& LocalAddress,int64 EventInterval, bool bEventOnce){
	_CLOCK(&m_EventMutex);

	CBrainEvent& Event = m_EventList[EventID];

	Event.m_Interval = EventInterval<MIN_EVENT_INTERVAL? MIN_EVENT_INTERVAL:EventInterval;
	Event.m_ClientEventID = ClientEventID;
	Event.m_ClientAddress =  LocalAddress;
	Event.m_bOnce = bEventOnce;
	Event.m_ClientExePipe = ExePipe;
	assert(Event.m_ClientAddress.Size());
};
#endif

bool CBrain::CLockedBrainData::PopBrainEvent(int64 EventID,CBrainEvent& EventInfo){
	_CLOCK(&m_EventMutex);
	
	map<int64,CBrainEvent>::iterator it = m_EventList.find(EventID);
	if (it != m_EventList.end())
	{
		CBrainEvent& Obj = it->second;

		EventInfo = Obj;
		m_EventList.erase(it);
		return true;
	}	
	return false;
};

void CBrain::CLockedBrainData::ResetEventTickCount(int64 EventID){
	_CLOCK(&m_EventMutex);
	map<int64, CBrainEvent>::iterator it = m_EventList.find(EventID);
    if (it != m_EventList.end())
    {
		CBrainEvent& EventInfo = it->second;
	    EventInfo.m_TickCount = 0;
	}
}

CLogicThread* CBrain::CLockedBrainData::CreateLogicThread(int64 UserID,int64 ThreadID /*=0*/){

	if(ThreadID==0){
		ThreadID = AbstractSpace::CreateTimeStamp();
	}
	_CLOCK(&m_BrainMutex);

	deque<CLogicThread*>::iterator it = m_LogicThreadPool.begin();
	if (it != m_LogicThreadPool.end())
	{   
		CLogicThread* Thread = *it;
		m_LogicThreadList[ThreadID] = Thread;
		Thread->Reset(ThreadID,UserID);
		m_LogicThreadPool.pop_front();
		return Thread;
	}
	CLogicThread* Thread = new CLogicThread(ThreadID,UserID);
	m_LogicThreadList[ThreadID] = Thread;
	return Thread;
};

CLogicThread* CBrain::CLockedBrainData::GetLogicThread(int64 ThreadID,bool bRequirLock /* = true*/){
    if (!bRequirLock)
    {
		map<int64,CLogicThread*>::iterator it = m_LogicThreadList.find(ThreadID);
		assert(it != m_LogicThreadList.end());
		CLogicThread* LogicThread =  it->second;
		return LogicThread;
    }
	_CLOCK(&m_BrainMutex);
	map<int64,CLogicThread*>::iterator it = m_LogicThreadList.find(ThreadID);
	assert(it != m_LogicThreadList.end());
    CLogicThread* LogicThread =  it->second;
	return LogicThread;
};

void    CBrain::CLockedBrainData::DeleteLogicThread(int64 ThreadID){
	_CLOCK(&m_BrainMutex);
	map<int64,CLogicThread*>::iterator it = m_LogicThreadList.find(ThreadID);
	assert(it != m_LogicThreadList.end());
    CLogicThread* LogicThread =  it->second;
    LogicThread->Reset(0,0);
    m_LogicThreadList.erase(it);
	m_LogicThreadPool.push_back(LogicThread);
};

void  CBrain::CLockedBrainData::SetLogicThreadUser(int64 ThreadID,int64 UserID){
	if (ThreadID==0)
	{
		return;
	}
	_CLOCK(&m_BrainMutex);
	map<int64,CLogicThread*>::iterator it = m_LogicThreadList.find(ThreadID);
	assert(it != m_LogicThreadList.end());
    if(it == m_LogicThreadList.end())return;
	CLogicThread* LogicThread =  it->second;
	LogicThread->m_UserDialogID = UserID;
}


CLogicTask*   CBrain::CLockedBrainData::CreateLogicTask(int64 UserID,int64 TaskID/*=0*/){
	if (TaskID==0)
	{
		TaskID = AbstractSpace::CreateTimeStamp();
	}

	_CLOCK(&m_BrainMutex);
	deque<CLogicTask*>::iterator it = m_LogicTaskPool.begin();
	if (it != m_LogicTaskPool.end())
	{   
		CLogicTask* Task = *it;
		m_LogicTaskList[TaskID] = Task;
        Task->Reset(TaskID,UserID);
		m_LogicTaskPool.pop_front();
		return Task;
	}

	CLogicTask* Task = new CLogicTask(TaskID,UserID);
    m_LogicTaskList[TaskID] = Task;
	return Task;
};

CLogicTask* CBrain::CLockedBrainData::GetLogicTask(int64 TaskID,bool bRequirLock /*= true*/){
	if (!bRequirLock)
	{
		map<int64,CLogicTask*>::iterator it = m_LogicTaskList.find(TaskID);
		assert(it != m_LogicTaskList.end());
		CLogicTask* Task =  it->second;
		return Task;
	}
    _CLOCK(&m_BrainMutex);
	map<int64,CLogicTask*>::iterator it = m_LogicTaskList.find(TaskID);
	assert(it != m_LogicTaskList.end());
    CLogicTask* Task =  it->second;
	return Task;
};

void  CBrain::CLockedBrainData::DeleteLogicTask(int64 TaskID){
	
	_CLOCK(&m_BrainMutex);
	map<int64,CLogicTask*>::iterator it = m_LogicTaskList.find(TaskID);
	assert(it != m_LogicTaskList.end());
    CLogicTask* LogicTask =  it->second;
    LogicTask->Reset(0,-1);
    m_LogicTaskList.erase(it);
	m_LogicTaskPool.push_back(LogicTask);
};

void  CBrain::CLockedBrainData::SetLogicTaskUser(int64 TaskID,int64 UserID){
	if (TaskID==0)
	{
		return;
	}
	_CLOCK(&m_BrainMutex);
	map<int64,CLogicTask*>::iterator it = m_LogicTaskList.find(TaskID);
	assert(it != m_LogicTaskList.end());
    if(it == m_LogicTaskList.end())return;
	CLogicTask* Task =  it->second;
	Task->m_UserDialogID = UserID;
}



CLogicDialog* CBrain::CLockedBrainData::CreateNewDialog(CBrain* Frame,int64 SourceID,int64 DialogID,int64 ParentDialogID,
						  tstring SourceName,tstring DialogName,DIALOG_TYPE Type,TASK_OUT_TYPE TaskType
						  )
{

	CLogicDialog* NewDialog = NULL;
	_CLOCK(&m_BrainMutex);
	deque<CLogicDialog*>::iterator it = m_DialogPool.begin();
	while(it != m_DialogPool.end()){
		CLogicDialog* Old = *it;
		if (Old->GetSysProcNum()==0){
			NewDialog = Old;
			NewDialog->Reset(Frame,SourceID,DialogID,ParentDialogID,SourceName,DialogName,Type,TaskType);
			m_DialogPool.erase(it);
			break;
		}
		it++;
	}

	if (NewDialog==NULL)
	{
		NewDialog = new CLogicDialog(Frame,SourceID,DialogID,ParentDialogID,SourceName,DialogName,Type,TaskType);
		if (NewDialog == NULL)
		{
			return NULL;
		}
	}

	DialogID = DialogID==DEFAULT_DIALOG?SourceID:DialogID;
	assert(m_DialogList.find(DialogID) == m_DialogList.end());
	m_DialogList[DialogID] = NewDialog;

	return NewDialog;
}

CLogicDialog* CBrain::CLockedBrainData::GetDialog(int64 SourceID,int64 DialogID){
	DialogID = DialogID==DEFAULT_DIALOG? SourceID:DialogID;

	_CLOCK(&m_BrainMutex);
	CLogicDialog* Dialog = NULL;
	map<int64,CLogicDialog*>::iterator it = m_DialogList.find(DialogID);
	if (it != m_DialogList.end())
	{
		Dialog =  it->second;

	}
	return Dialog;
}

CLogicDialog* CBrain::CLockedBrainData::Interal_GetDialog(int64 SourceID,int64 DialogID){
	DialogID = DialogID==DEFAULT_DIALOG? SourceID:DialogID;
	CLogicDialog* Dialog = NULL;
	map<int64,CLogicDialog*>::iterator it = m_DialogList.find(DialogID);
	if (it != m_DialogList.end())
	{
		Dialog =  it->second;

	}
	return Dialog;
}

CLogicDialog* CBrain::CLockedBrainData::FindDialog(int64 DialogID){

	_CLOCK(&m_BrainMutex);
	CLogicDialog* Dialog = NULL;
	map<int64,CLogicDialog*>::iterator it = m_DialogList.find(DialogID);
	if (it != m_DialogList.end())
	{
		Dialog =  it->second;
	}
	return Dialog;
}
void   CBrain::CLockedBrainData::DeleteDialog(int64 SourceID,int64 DialogID){
	DialogID = DialogID==DEFAULT_DIALOG? SourceID:DialogID;
	m_BrainMutex.Acquire();

	map<int64,CLogicDialog*>::iterator it= m_DialogList.find(DialogID);
	if (it == m_DialogList.end())
	{
		m_BrainMutex.Release();
		return;
	}
	CLogicDialog* Dialog = it->second;	
	m_DialogList.erase(it);				
	m_DialogPool.push_back(Dialog);	
	m_BrainMutex.Release();	

	//以下会调用m_DialogMutex，所以必须小心排除在m_BrianMutex锁定范围之外
	Dialog->SetTaskState(TASK_DELELTE);
	Dialog->NotifyTaskState();
}

void   CBrain::CLockedBrainData::Interal_DeleteDialog(int64 SourceID,int64 DialogID){
	DialogID = DialogID==DEFAULT_DIALOG? SourceID:DialogID;

	map<int64,CLogicDialog*>::iterator it= m_DialogList.find(DialogID);
	if (it == m_DialogList.end())
	{
		return;
	}
	CLogicDialog* Dialog = it->second;	
	m_DialogList.erase(it);				
	m_DialogPool.push_back(Dialog);	

	//以下会调用m_DialogMutex，所以必须小心排除在m_BrianMutex锁定范围之外
	Dialog->SetTaskState(TASK_DELELTE);
	Dialog->NotifyTaskState();
}

void CBrain::CLockedBrainData::DeleteAllDialog(){
	_CLOCK(&m_BrainMutex);
	map<int64,CLogicDialog*>::iterator it= m_DialogList.begin();
	while (it != m_DialogList.end())
	{
		CLogicDialog* Dialog = it->second;
		Dialog->SetTaskState(TASK_DELELTE);
		m_DialogPool.push_back(Dialog);
		it->second = NULL;
		it = m_DialogList.erase(it);
	}				
}
int   CBrain::CLockedBrainData::DeleteDialogOfSource(int64 SourceID){
	_CLOCK(&m_BrainMutex);
	int n=0;
	map<int64,CLogicDialog*>::iterator it= m_DialogList.begin();
	while (it != m_DialogList.end())
	{
		CLogicDialog* Dialog = it->second;
		
		if (Dialog->m_SourceID == SourceID)
		{
			m_DialogPool.push_back(Dialog);			
			it->second = NULL;
			it = m_DialogList.erase(it);	
		
			n++;
		}else{
			it++;
		}
	}	
	return n;			
}


void  CBrain::CLockedBrainData::GetFocusDialogData(int64 SourceID,int64 DialogID,ePipeline& Pipe,bool bIncludeDebugInfo)
{
	DialogID = DialogID==DEFAULT_DIALOG? SourceID:DialogID;

	_CLOCK(&m_BrainMutex);	

	CLogicDialog* Dialog = NULL;
	
	map<int64,CLogicDialog*>::iterator it = m_DialogList.find(DialogID);
	assert(it != m_DialogList.end());
	if (it == m_DialogList.end())
	{
		return ;
	}
	Dialog = it->second;	
	
	assert(Dialog!=NULL);
	
	//首先装入最后20个对话数据
	ePipeline ItemList;	
	ePipeline& Log = Dialog->m_DialogHistory;
	
	int32  n=20;
	
	int32  P2 = Log.Size(); 
	int32  P1 = P2-n>0?P2-n:0;
	
	if (P1>0)
	{
		ePipeline* Empty = new ePipeline(-1);
		Empty->PushInt(P1);  //还有多少数据没显示
		ItemList.Push_Directly(Empty);
	}    
	
	while (P1<P2)
	{
		ePipeline* Item = (ePipeline*)Log.GetData(P1++);
		ePipeline* c = (ePipeline*)Item->Clone();
		ItemList.Push_Directly(c);
	}
	
	Pipe.PushPipe(ItemList);

	ItemList.Clear();

	//装入最后20个运行时记录
	ePipeline& RuntimeInfo = Dialog->m_RuntimeOutput;
	
	P2 = RuntimeInfo.Size(); 
	P1 = P2-n>0?P2-n:0;
	
	ePipeline* Empty = new ePipeline(-1);
	Empty->PushInt(P1);  //还有多少数据没显示
	ItemList.Push_Directly(Empty);    
	
	while (P1<P2)
	{
		ePipeline* Item = (ePipeline*)RuntimeInfo.GetData(P1++);
		ePipeline* c = (ePipeline*)Item->Clone();
		ItemList.Push_Directly(c);
	}	
	Pipe.PushPipe(ItemList);

	//装入临时逻辑数据
	ePipeline LogicList;
	//CLogicThread* Think = GetLogicThread(Dialog->m_ThinkID);

	Dialog->GetLocalLogicData(LogicList);
	Pipe.PushPipe(LogicList);

	//装入临时capa数据
	ePipeline CapaList;
    Dialog->GetLocalCapaData(CapaList);
	Pipe.PushPipe(CapaList);

    //装入临时indu数据
	ePipeline InduList;
	Dialog->GetLocalInduData(InduList);
	Pipe.PushPipe(InduList);

	//装入临时物体数据
	ePipeline ObjectList;
	Dialog->GetLocalObjectData(ObjectList);
	Pipe.PushPipe(ObjectList);

	ePipeline TableInstanceList;
	Dialog->GetTableInstanceData(TableInstanceList);
	Pipe.PushPipe(TableInstanceList);


	//输入窗口状态
    Pipe.PushInt(Dialog->m_bEditValid);
	Pipe.PushString(Dialog->m_EditText);

	//状态条文字
	Pipe.PushString(Dialog->m_StatusText);

	//工作模式
	Pipe.PushInt(Dialog->GetWorkMode());

	//任务状态
	TASK_STATE State = Dialog->GetTaskState();
    Pipe.PushInt(State);

	//调试模式
	if (State == TASK_PAUSE)
	{
		if (bIncludeDebugInfo)
		{
			Pipe.PushInt(1);
			Pipe.Push_Directly(Dialog->m_LogicItemTree.Clone());
			
			//debug timestamp
			Pipe.PushInt(Dialog->m_LogicItemTree.GetID());
          
			//pause item
			ePipeline PauseList;
		    Dialog->GetPauseIDList(PauseList);
			Pipe.PushPipe(PauseList);
		}else{
			Pipe.PushInt(0);
		}
	}

	//搜索数据
	Pipe.Push_Directly(Dialog->m_SearchOutput.Clone());

};
void  CBrain::CLockedBrainData::GetInvalidDialogData(ePipeline& Pipe){


	//首先装入对一个缺省对话
	ePipeline ItemList;	

	int64 ID= AbstractSpace::CreateTimeStamp();
	ePipeline* Item = new ePipeline(_T("System"),ID);

	Item->PushInt(0);
	Item->PushString(_T("you not authorized to access this dialog"));
	ItemList.Push_Directly(Item);

	Pipe.PushPipe(ItemList);

	ItemList.Clear();

	//装入空运行时记录	
	ePipeline* Empty = new ePipeline(-1);
	Empty->PushInt(0);  //还有多少数据没显示
	ItemList.Push_Directly(Empty);    
	Pipe.PushPipe(ItemList);

	//装入空临时逻辑数据
	ePipeline LogicList;
	Pipe.PushPipe(LogicList);

	//装入空临时capa数据
	ePipeline CapaList;
	Pipe.PushPipe(CapaList);

	//装入空临时indu数据
	ePipeline InduList;
	Pipe.PushPipe(InduList);

	//装入空临时物体数据
	ePipeline ObjectList;
	Pipe.PushPipe(ObjectList);

	//空临时记忆数据
	ePipeline TableInstanceList;
	Pipe.PushPipe(TableInstanceList);


	//输入窗口状态
	Pipe.PushInt(0); //不准编辑
	Pipe.PushString(_T(""));

	//状态条文字
	Pipe.PushString(_T(""));

	//工作模式
	Pipe.PushInt(WORK_TASK);

	//任务状态
	Pipe.PushInt(TASK_IDLE);

	//搜索数据
	ePipeline SearchResult;
	Pipe.PushPipe(SearchResult);
}

void  CBrain::CLockedBrainData::GetMoreLog(int64 SourceID,int64 DialogID,int64 LastItemID,ePipeline& Pipe){
	DialogID = DialogID==DEFAULT_DIALOG? SourceID:DialogID;
	_CLOCK(&m_BrainMutex);	

	map<int64,CLogicDialog*>::iterator it = m_DialogList.find(DialogID);
	assert(it != m_DialogList.end());
	if (it == m_DialogList.end())
	{
		return ;
	}
	
	CLogicDialog* Dialog = it->second;	
	assert(Dialog!=NULL);
	
	ePipeline ItemList;	
	ePipeline& Log = Dialog->m_DialogHistory;
	
    //首先找到LastItem位置
	int i;
	for (i=Log.Size()-1; i>-1;i--)
	{
		ePipeline* Child = (ePipeline*)Log.GetData(i);
		if (Child->GetID() == LastItemID)
		{
			break;
		}
	};
	
	
	//返回i之前20个数据
	int32  n=20;
	
    int32 P2 = i;
	assert(P2>0);   
	int32  P1 = P2-n>0?P2-n:0;
	
	if (P1>0)
	{
		ePipeline* Empty = new ePipeline;
		Empty->PushInt(P1);  //还有多少数据没显示
		ItemList.Push_Directly(Empty);
	}    
	
	while (P1<P2)
	{
		ePipeline* Item = (ePipeline*)Log.GetData(P1++);
		ePipeline* c = (ePipeline*)Item->Clone();
		ItemList.Push_Directly(c);
	}
	Pipe<<ItemList;
}

void CBrain::CLockedBrainData::GetAllDialogListInfo(ePipeline& DialogListInfo){

	_CLOCK(&m_BrainMutex);	

	CLogicDialog* Dialog = NULL;
	map<int64,CLogicDialog*>::iterator it = m_DialogList.begin();
	while (it != m_DialogList.end())
	{
		Dialog =  it->second;
		int64 SourceID = Dialog->m_SourceID;
		int64 DialogID = Dialog->m_DialogID;
		int64 ParentID = Dialog->m_ParentDialogID;
		tstring& Name  = Dialog->m_DialogName;
		int32 Type = Dialog->m_DialogType;
		
		DialogListInfo.PushInt(SourceID);
		DialogListInfo.PushInt(DialogID);
		DialogListInfo.PushInt(ParentID);
		DialogListInfo.PushString(Name);
		DialogListInfo.PushInt(Type);	
		it++;
	}
	
}
void CBrain::CLockedBrainData::BrainIdleProc(CBrain* Brain){

    _CLOCK(&m_BrainMutex);

	//检查Thread的有效性，无效的则归还给内存池
	if(m_LogicThreadList.size() > m_DialogList.size() ){
	   map<int64,CLogicThread*>::iterator it = m_LogicThreadList.begin();
	   while (it != m_LogicThreadList.end())
	   {
		   CLogicThread* Think = it->second;
		   int64 DialogID = Think->m_UserDialogID;
		   if (DialogID==DEFAULT_DIALOG)
		   {
			   it++;
			   continue; //中枢对话总是有效
		   }
		   map<int64,CLogicDialog*>::iterator it1 = m_DialogList.find(DialogID);
		   if (it1 == m_DialogList.end())
		   {
			   it = m_LogicThreadList.erase(it);
			   Think->Reset(0,-1);
			   if (m_LogicThreadPool.size()>20) //控制在20个
			   {
				   delete Think;
			   }else{ //放入pool中
				   m_LogicThreadPool.push_back(Think);	   
			   }
			   continue;
		   }
		   CLogicDialog* Dilaog = it1->second;
		   if(Dilaog->m_ThinkID != Think->m_ID){
			   it = m_LogicThreadList.erase(it);
			   Think->Reset(0,-1);
			   if (m_LogicThreadPool.size()>20) //控制在20个
			   {
				   delete Think;
			   }else{ //放入pool中
				   m_LogicThreadPool.push_back(Think);	   
			   }		   
		   }else{
		       it++;
		   }
	   }
	}

	//检查Task的有效性，无效的则归还给内存池
	if(m_LogicTaskList.size() > m_DialogList.size() ){
		map<int64,CLogicTask*>::iterator it = m_LogicTaskList.begin();
		while (it != m_LogicTaskList.end())
		{
			CLogicTask* Task = it->second;
			int64 DialogID = Task->m_UserDialogID;
			if (DialogID==0)
			{
				it++;
				continue;
			}

			if (Task->m_UserDialogID == 1 ) //这种情况表明被临时逻辑占用
			{
				it++;
				continue;
			}
			map<int64,CLogicDialog*>::iterator it1 = m_DialogList.find(DialogID);
			if (it1 == m_DialogList.end())
			{
				it = m_LogicTaskList.erase(it);
				Task->Reset(0,-1);
				if (m_LogicTaskPool.size()>20) //控制在20个
				{
					delete Task;
				}else{ //放入pool中
					m_LogicTaskPool.push_back(Task);	   
				}
				continue;
			}
			CLogicDialog* Dilaog = it1->second;
			if(Dilaog->m_TaskID != Task->m_ID){
				it = m_LogicTaskList.erase(it);
				Task->Reset(0,-1);
				if (m_LogicTaskPool.size()>20) //控制在20个
				{
					delete Task;
				}else{ //放入pool中
					m_LogicTaskPool.push_back(Task);	   
				}		   
			}else{
				it++;
			}
		}
	}
	
	//限制DialogPool大小
	deque<CLogicDialog*>::iterator it = m_DialogPool.begin();
	while (it != m_DialogPool.end() && m_DialogPool.size()>20) //暂停pool大小为20
	{
		CLogicDialog* Dialog = *it;
		if (Dialog->GetSysProcNum()==0)
		{
			delete Dialog;
			it = m_DialogPool.erase(it);
		}else{
			it++;
		}
	}

}

void  CBrain::CLockedBrainData::GetBrainStateInfo(ePipeline& Info){
	m_EventMutex.Acquire();
	int EventNum = m_EventList.size();
	m_EventMutex.Release();


	m_BrainMutex.Acquire();
    int DialogNum = m_DialogList.size();
	int DialogPoolSize = m_DialogPool.size();
	
	int ThreadNum = m_LogicThreadList.size();
	int ThreadPoolSize = m_LogicThreadPool.size();

	int TaskNum = m_LogicTaskList.size();
	int TaskPoolSize = m_LogicTaskPool.size();

	int GuiNum = 0;//m_OrganList.size();
    m_BrainMutex.Release();
    
	Info.PushInt(EventNum);
	Info.PushInt(DialogNum);
	Info.PushInt(DialogPoolSize);
	Info.PushInt(ThreadNum);
	Info.PushInt(ThreadPoolSize);
	Info.PushInt(TaskNum);
	Info.PushInt(TaskPoolSize);
	Info.PushInt(GuiNum);
}


void CBrain::CLockedBrainData::EventProc(CBrain* Brain){

	while (Brain->IsAlive())
	{

		int64 NewTimeStamp = AbstractSpace::CreateTimeStamp();	

		m_EventMutex.Acquire();	
		
		map<int64, CBrainEvent>::iterator it = m_EventList.begin();
		while (it != m_EventList.end())
		{
			

			int64 EventID = it->first;
			CBrainEvent& EventInfo = it->second;
			int64 OldTimeStamp = EventInfo.m_TimeStamp;
			int64 Interval = NewTimeStamp-OldTimeStamp;

			if (Interval >=EventInfo.m_Interval) //达到预定间隔时间
			{
				EventInfo.m_TimeStamp = NewTimeStamp; 

				ePipeline& Address = EventInfo.m_ClientAddress;
				int64& TickCount = EventInfo.m_TickCount;
				
				int64 DialogID = *(int64*)Address.GetData(0);

				if (TickCount==0) //事件正常，发出一个TICK
				{						
					TickCount++;
					
					CMsg Msg(Address,MSG_EVENT_TICK,EventID);
					Msg.GetLetter().PushInt(NewTimeStamp);
					Msg.SetSourceID(Address.m_ID);

					if (DialogID == DEFAULT_DIALOG) //本地事件
					{
						Brain->PushCentralNerveMsg(Msg,false,true);

					}else{ 
						Brain->PushNerveMsg(Msg,false,false);
					}
				} 
				else  //说明在事件发出TICK信息还没得到响应，检查事件地址是否依然有效
				{     //如果有效继续等待，对方一定会处理

					if (Address.m_ID < MAX_LOCAL_SOURCE_ID ) //本地事件则
					{
					
						CLogicDialog* Dialog = Interal_GetDialog(Address.m_ID,DialogID);
						if (!Dialog )
						{	
							it = m_EventList.erase(it);
							continue;
						}

						if (Address.Size()>1)
						{
							int64 TaskID   = *(int64*)Address.GetData(1);
							if ( Dialog->m_TaskID != TaskID)
							{												
								CLogicDialog* Dlg = Interal_GetDialog(Address.m_ID,EventID);
								if (Dlg)
								{
									CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
									nf.PushInt(DL_DEL_DIALOG);
									nf.Notify(Dlg);

									Interal_DeleteDialog(Address.m_ID,EventID);
								}
								it = m_EventList.erase(it);
								continue;
							}
						}

						//检查事件用户依然存在后，我们更新时间戳，避免频繁检查
						EventInfo.m_TimeStamp = NewTimeStamp; 
						//EventInfo.m_TickCount = 0;
					} 
					else
					{
						int64 DialogID = EventID;
						CLogicDialog* Dialog = Interal_GetDialog(Address.m_ID,DialogID);
						if (!Dialog)
						{
							//如果事件没有反馈对象，那么事件本身所对应的对话就应该停止，然后被删除
							ePipeline ChildMsg(GUI_TASK_CONTROL);
							ChildMsg.PushInt(CMD_STOP);

							CMsg Msg;
							CreateBrainMsg(Msg,it->first,ChildMsg,0);
							Brain->PushNerveMsg(Msg,false,true);

							it = m_EventList.erase(it);	
							continue;
						}

						//检查事件用户依然存在后，我们更新时间戳，避免频繁检查
						EventInfo.m_TimeStamp = NewTimeStamp; 
						//EventInfo.m_TickCount = 0;						
					}		
				}
			}			
			it++;
		}
		m_EventMutex.Release();	

	}
	assert(!Brain->IsAlive());
}


Energy*  CBrain::CLockedBrainData::ToEnergy(){

	ePipeline* Pipe = new ePipeline;
	eElectron e(Pipe);

	//先通知所有对话正在运行的任务暂停
    m_BrainMutex.Acquire();
	map<int64,CLogicDialog*>::iterator it = m_DialogList.begin();
	while(it != m_DialogList.end()){
		CLogicDialog* Dialog = it->second;
		assert(Dialog);

		TASK_STATE State = Dialog->GetTaskState();
		if(State == TASK_RUN){
			Dialog->NotifySuspendTask();
		}
		it++;
	}
	m_BrainMutex.Release();

	//然后等候十秒并确认所有对话暂停
	int n = 20;
	int RunningNum;

	CLogicDialog* SystemDialog = m_DialogList[DEFAULT_DIALOG];
	do 
	{
		SLEEP_MILLI(500);
		RunningNum =0;
		m_BrainMutex.Acquire();
		map<int64,CLogicDialog*>::iterator it = m_DialogList.begin();
		while(it != m_DialogList.end()){
			CLogicDialog* Dialog = it->second;
			assert(Dialog);
			TASK_STATE State = Dialog->GetTaskState();

			if(State == TASK_RUN){
				RunningNum++;
			}
			it++;
		}
		m_BrainMutex.Release();
		n--;

		CNotifyDialogState nf(NOTIFY_PROGRESS_OUTPUT);
		nf.PushInt(COMMON_PROGRESS);
		nf.PushString(_T("Waiting for system closing..."));
		nf.PushInt(n*100/20);
		nf.Notify(SystemDialog);

	} while (n>0 && RunningNum >0);

	//十秒后对话后理论上所有对话都应该暂停
	//如果没有暂停也暂时保存当前对话信息
	assert(RunningNum==0);

	ePipeline DialogListInfo;
	m_BrainMutex.Acquire();
    it = m_DialogList.begin();
	while(it != m_DialogList.end()){
		CLogicDialog* Dialog = it->second;
		assert(Dialog);
		if (Dialog->m_DialogID != BRAIN_MEMORY_CHECH_DIALOG_ID && Dialog->m_ParentDialogID != BRAIN_MEMORY_CHECH_DIALOG_ID)
		{
			Energy* E = Dialog->ToEnergy();
			if(E){
				ePipeline* p = (ePipeline*)E;
				DialogListInfo.Push_Directly(E);
			}			
		}
		it++;
	}
	m_BrainMutex.Release();

	Pipe->PushList(DialogListInfo);

	m_EventMutex.Acquire();
	ePipeline EventList;
	map<int64, CBrainEvent>::iterator it3= m_EventList.begin();
	while(it3!=m_EventList.end()){
		int64 EventID = it3->first;
		CBrainEvent& EventInfo = it3->second;
		ePipeline TempPipe;
		EventInfo.ToPipeline(TempPipe);
		TempPipe.SetID(EventID);
		EventList.Push_Directly(TempPipe.Clone());
		it3++;
	}
	Pipe->PushList(EventList);
	m_EventMutex.Release();

	return e.Release();
}

bool  CBrain::CLockedBrainData::FromEnergy(CBrain* Brain,Energy* E){
	DeleteAllDialog();

	ePipeline* Pipe = (ePipeline*)E;
	assert(Pipe->Size()==2);

	int i;

	
	ePipeline* DialogList = (ePipeline*)Pipe->GetData(0);
	for (i=0; i<DialogList->Size();i++)
	{
		//_CLOCK(&m_BrainMutex); //会导致嵌套死锁，也无必要

		ePipeline* p = (ePipeline*)DialogList->GetData(i);

		int64 SourceID = *(int64*)p->GetData(0);
		int64 DialogID = *(int64*)p->GetData(1);

		if (SourceID>MAX_LOCAL_SOURCE_ID) //外部对话忽略
		{
			continue;
		}
		CLogicDialog* Dialog =  CreateNewDialog(Brain,SourceID,DialogID,0,_T(""),_T(""),DIALOG_SYSTEM_MAIN,TASK_OUT_DEFAULT);
		bool ret = Dialog->FromEnergy(p);
		assert(ret);
		if (!ret)
		{
			return false;
		};
		//SetWorkMode(WORK_DEBUG);
		Dialog->NotifyTaskState();
		//Dialog->ResumeTask();
	}

	m_EventMutex.Acquire();
	ePipeline* EventList = (ePipeline*)Pipe->GetData(1);
	for (i=0; i<EventList->Size(); i++)
	{
		ePipeline* EventInfo = (ePipeline*)EventList->GetData(i);
	    int64 EventID = EventInfo->GetID();
        EventInfo->SetID(0);
		CBrainEvent& Event = m_EventList[EventID];
		Event.FromPipeline(*EventInfo);
	}
	m_EventMutex.Release();
	
	return true;
}

CThreadWorker* CBrain::CLockedBrainData::CreateThreadWorker(int64 ID,System* Parent,int32 Type){
	if(!Parent->IsAlive())return NULL;

	_CLOCK(&m_BrainMutex);
	CThreadWorker* Worker = NULL;
	deque<CThreadWorker*>::iterator it = m_ThreadWorkerPool.begin();
	if (it != m_ThreadWorkerPool.end())
	{   
		Worker = *it;
		Worker->Dead();

		Worker->m_ID = ID;
		Worker->m_IdleCount =0;
		Worker->m_Parent = Parent;
		m_ThreadWorkerPool.pop_front();
	}else{
		Worker = new CBrainThreadWorker(ID,Parent,Type);
	}

	if(!Worker){
		return NULL;
	}

	if (Type == BRAIN_WEBSOCKET_IO_WORK_TYPE)
	{
		m_WebsocketIOWorkerList[ID]= Worker;
		return Worker;
	}
	assert(0);
	return NULL;
};

int32 CBrain::CLockedBrainData::GetWebIOWorkerNum(){
	_CLOCK(&m_BrainMutex);
	return m_WebsocketIOWorkerList.size();
}

void   CBrain::CLockedBrainData::DeleteThreadWorker(System* Parent,int64 ID,int32 Type){
	if(!Parent->IsAlive())return;

	_CLOCK(&m_BrainMutex);
	CThreadWorker* Worker = NULL;
	if (Type == BRAIN_WEBSOCKET_IO_WORK_TYPE)
	{
		map<int64,CThreadWorker*>::iterator it = m_WebsocketIOWorkerList.find(ID);
		assert(it != m_WebsocketIOWorkerList.end());
		if (it != m_WebsocketIOWorkerList.end())
		{
			Worker = it->second;
			m_WebsocketIOWorkerList.erase(it);
			m_ThreadWorkerPool.push_back(Worker);

		}
	}
	//限制ThreadWorkerPool大小为20	
	if(m_ThreadWorkerPool.size()>20)
	{
		deque<CThreadWorker*>::iterator it = m_ThreadWorkerPool.begin();
		CThreadWorker* Worker = *it;
		Worker->Dead();
		it = m_ThreadWorkerPool.erase(it);
		delete Worker;
	}
}

void   CBrain::CLockedBrainData::WaitAllWorkThreadClosed(){


	map<int64,CThreadWorker*>::iterator itb =  m_WebsocketIOWorkerList.begin();
	while(itb != m_WebsocketIOWorkerList.end())
	{
		CThreadWorker* Worker = itb->second;
		Worker->Dead();
		itb++;
	}

	deque<CThreadWorker*>::iterator itc = m_ThreadWorkerPool.begin();
	while(itc != m_ThreadWorkerPool.end())
	{
		CThreadWorker* Worker = *itc;
		Worker->Dead();
		itc++;
	}
}
//CBrain
//////////////////////////////////////////////////////////////////////

CBrain::CBrain(CUserTimer* Timer,CUserSpacePool* Pool)
	:System(Timer,Pool),m_BrainData(this),m_EventWorker(0,this,BRAIN_EVENT_WORK_TYPE)

{
	m_LogFlag = 0;
	m_GUI_WebServer = NULL;
}



CBrain::~CBrain()
{
	if(m_GUI_WebServer){
		m_GUI_WebServer->stopAll(true);
		delete m_GUI_WebServer;
		m_GUI_WebServer = NULL;
	}
}

bool CBrain::Activation(){
    if (m_Alive)
    {
		return true;
    }

	//从长期记忆中恢复大脑当前场景

	m_BrainMemory.Open();
	

	AnsiString DialogInfo = CBrainMemory::GetSystemItem(SYS_DIALOG);
	//DialogInfo=""; //仅用于测试时忽略

	if (DialogInfo.size())
	{
		ePipeline Pipe;
		uint32 pos=0;
		if(Pipe.FromString(DialogInfo,pos) && FromEnergy(&Pipe))
		{	
			OutputLog(LOG_TIP,_T("Retrieve Brain Logic Scene...OK"));
		}else{
			OutputLog(LOG_TIP,_T("Retrieve Brain Logic Scene...Fail"));
		}
	}
	m_BrainData.LoadUserAccountList();
	
	if(!System::Activation()){
		m_ErrorInfo = _T("System activate fail.");
		return false;
	}

	if(!m_EventWorker.Activation()){
		m_ErrorInfo = Format1024(_T("Event proc thread started fail."));
		return false;
	}

	//	m_LeftArm.OpenSerialPort("COM3");

	m_GUI_WebServer= new HTTPServer(new BrainRequestHandlerFactory(this),80);
	m_GUI_WebServer->start();

	return true;
}


void CBrain::Dead(){

	if(m_Alive == FALSE)return;

	m_GUI_WebServer->stop();

//	m_Alive = FALSE;

	Energy* E = NULL; 
	ePipeline* Pipe = NULL;
	try{
		E = ToEnergy();
		if (E)
		{
			ePipeline* Pipe = (ePipeline*)E;
			AnsiString s;
			Pipe->ToString(s);
			CBrainMemory::SetSystemItem(SYS_DIALOG,s);
			delete Pipe;
		}
	}catch(...){
		AnsiString s;
		CBrainMemory::SetSystemItem(SYS_DIALOG,s);
		if (E)delete E;
	}

	System::Dead();

	m_BrainData.WaitAllWorkThreadClosed();
	m_BrainData.Clear();

	WSACleanup();
	m_BrainMemory.Close();
}


//目前为了简单起见，仅为演示目的做最小备份
Energy*  CBrain::ToEnergy(){
	ePipeline* Pipe = new ePipeline;
	eElectron e(Pipe);
	if (Pipe)
	{
		//全局逻辑
		Pipe->Push_Directly(CBrainMemory::m_GlobalLogicList.Clone());
		
		//所有对话
		CLockedBrainData* BrainData = GetBrainData();
		Energy* E = BrainData->ToEnergy();
        if (!E)
        {
			return NULL;
        }
		Pipe->Push_Directly(E);
		
		return e.Release();
	}
	
    return NULL;
}

bool  CBrain::FromEnergy(Energy* e){
    ePipeline* Pipe = (ePipeline*)e;
    eElectron E;
	Pipe->Pop(&E);
	CBrainMemory::m_GlobalLogicList << *(ePipeline*)E.Get();
	
	Pipe->Pop(&E);
    CLockedBrainData* BrainData = GetBrainData();
	bool ret = BrainData->FromEnergy(this,E.Get());
	
	return ret;
}

tstring CBrain::MsgID2Str(int64 MsgID){
	static map<int64,tstring>  MsgTextList;
	if (MsgTextList.size()==0)
	{
		int i;
		int n = ( sizeof( (_MsgTextList) ) / sizeof( (_MsgTextList[0]) ) ) ;

		for (i=0; i<n; i++)
		{
			MsgTextList[_MsgTextList[i].msg_id] =_MsgTextList[i].msg_text;
		}

		n = ( sizeof( (_SpaceMsgTextList) ) / sizeof( (_SpaceMsgTextList[0]) ) ) ;

		for (i=0; i<n; i++)
		{
			MsgTextList[_SpaceMsgTextList[i].msg_id] =_SpaceMsgTextList[i].msg_text;
		}		
	}
	tstring MsgStr;
	map<int64,tstring>::iterator it = MsgTextList.find(MsgID);
	if(it!=MsgTextList.end()){
		MsgStr = it->second;		
	}else
	{

		MsgStr = Format1024(_T("%I64ld"),MsgID);
	}
	return MsgStr;
};


void CBrain::OutputLog(uint32 Flag,TCHAR* Format, ...){
	if (m_LogFlag & Flag)
	{
		TCHAR Buffer [512] ;
		va_list ArgList ;		
		va_start (ArgList, Format) ;
		_vsntprintf(Buffer, 256, Format, ArgList) ;		
		va_end (ArgList) ;

		int64 TimeStamp = AbstractSpace::CreateTimeStamp();
		m_LogDB.WriteItem(TimeStamp,_T("Brain"),Buffer);
	}

};

void CBrain::OutputLog(uint32 Flag,const TCHAR* s){
	if (m_LogFlag & Flag)
	{
		int64 TimeStamp = AbstractSpace::CreateTimeStamp();
		m_LogDB.WriteItem(TimeStamp,_T("Brain"),s);
	}
}

void CBrain::GetLinker(int64 SourceID,CLinker& Linker){
	GetClientLinkerList()->GetLinker(SourceID,Linker);
	if (!Linker.IsValid())
	{
		GetSuperiorLinkerList()->GetLinker(SourceID,Linker);
		if(!Linker.IsValid()){
			m_WebsocketClientList.GetLinker(SourceID,Linker);
		}
	}
}

void  CBrain::CLockedBrainData::LoadUserAccountList(){
	_CLOCK(&m_BrainMutex);
	AnsiString UserListText = CBrainMemory::GetSystemItem(SYS_ACCOUNT);
	uint32 pos=0; 
	m_UAStaticList.FromString(UserListText,pos);
}

int CBrain::CLockedBrainData::GetUserAccountNum(){
	_CLOCK(&m_BrainMutex);
	return m_UAStaticList.Size();
}

bool CBrain::Login(int64 SourceID,tstring UserName, tstring CrypStr)
{
	//根据此用户名寻找之前的对话，如果对话存在直接绑定（短时网络中断），如果不存在则生成新的对话
	ePipeline UserInfo;
	if(!m_BrainData.FindRegisterUserInfo(UserName,CrypStr,UserInfo)){
		return false;
	};

	assert(UserInfo.Size()==4);
	int64 DialogType =  *(int64*)UserInfo.GetData(2);
	if (DialogType != DIALOG_SYSTEM_MAIN){
		return false;
	}

	m_BrainData.RegisterOrgan(UserName,CrypStr,SourceID,DIALOG_SYSTEM_MAIN);

	//如果用户对应的对话还活着就直接使用(这样临时网络断线后可以直接恢复）
	CLogicDialog* Dialog = NULL;

	if (SourceID > MAX_LOCAL_SOURCE_ID)
	{
		int64 HistoryDialogID = *(int64*)UserInfo.GetData(3);
		if(HistoryDialogID != -1 && HistoryDialogID == SourceID){
			Dialog = m_BrainData.FindDialog(HistoryDialogID); 
			Dialog->m_SourceID = SourceID;
		}
	} 
	else
	{
		//可能已经存在
		Dialog = m_BrainData.FindDialog(SourceID);
	}

	if (!Dialog)
	{	
		tstring SourceName;
		if (SourceID == LOCAL_GUI_SOURCE)
		{
			SourceName = _T("System(Local)"); 
			UserName+= _T("(Local)");
		}else{
			SourceName = _T("System");
		}
		Dialog = m_BrainData.CreateNewDialog(this,SourceID,DEFAULT_DIALOG,NO_PARENT,SourceName,UserName,DIALOG_SYSTEM_MAIN,TASK_OUT_DEFAULT);

		if (!Dialog)
		{
			m_ErrorInfo = _T("Create dialog fail");
			return false;
		}

		m_BrainData.UpdateUserDialogID(UserName,CrypStr,SourceID);

		ePipeline ExePipe;
		ePipeline LocalAddress(SourceID);
		LocalAddress.PushInt(Dialog->m_DialogID);

#ifdef _DEBUG
	
		tstring memo = Format1024(_T("%s %d"),_T(__FILE__),__LINE__);
		m_BrainData.PushBrainEvent(SourceID,SourceID,ExePipe,LocalAddress,TIME_SEC,false,memo);
#else
		m_BrainData.PushBrainEvent(SourceID,SourceID,ExePipe,LocalAddress,TIME_SEC,false);
#endif
	}

	return true;
};

void CBrain::CheckMemory(){
	CMsg Msg(MSG_INIT_BRAIN,DEFAULT_DIALOG,0);	
	PushCentralNerveMsg(Msg,false ,false);	
}

void CBrain::CLockedBrainData::RegisterUserAccount(tstring Name,tstring CrypStr,DIALOG_TYPE DialogType){
	_CLOCK(&m_BrainMutex);

	bool bChanged = false;
	//修改
	for(uint32 i=0; i<m_UAStaticList.Size();i++){
		ePipeline* Item = (ePipeline*)m_UAStaticList.GetData(i);
		tstring srcName = *(tstring*)Item->GetData(0);
		if (srcName == Name)
		{
			tstring srcCrypText = *(tstring*)Item->GetData(1);
			if (srcCrypText == CrypStr)
			{
				int64& Type = *(int64*)Item->GetData(2);
				if(DialogType != Type){
					Type = DialogType;
					bChanged = true;
				}
			}
		}
	}

	if (!bChanged)
	{
		ePipeline NewItem;
		NewItem.PushString(Name);
		NewItem.PushString(CrypStr);
		NewItem.PushInt(DialogType);
		NewItem.PushInt(-1);
		m_UAStaticList.PushPipe(NewItem);
	}

	AnsiString SaveText;
	m_UAStaticList.ToString(SaveText);
	if (SaveText.size())
	{
		CBrainMemory::SetSystemItem(SYS_ACCOUNT,SaveText);
	}
}

int64    CBrain::CLockedBrainData::DeleteUserAccount(tstring Name,tstring CrypStr){
	_CLOCK(&m_BrainMutex);
	for(uint32 i=0; i<m_UAStaticList.Size();i++){
		ePipeline* Item = (ePipeline*)m_UAStaticList.GetData(i);
		tstring srcName = *(tstring*)Item->GetData(0);
		if (srcName == Name)
		{
			tstring srcCrypText = *(tstring*)Item->GetData(1);
			if (srcCrypText == CrypStr)
			{
				int64 SourceID = *(int64*)Item->GetData(3);

				m_UAStaticList.EraseEnergy(i,1);

				AnsiString SaveText;
				m_UAStaticList.ToString(SaveText);
				if (SaveText.size())
				{
					CBrainMemory::SetSystemItem(SYS_ACCOUNT,SaveText);
				}
				return SourceID;
			}
		}
	}
	return 0;
}

void CBrain::CLockedBrainData::UpdateUserDialogID(tstring Name,tstring Cryptograhp,int64 DialogID){
	_CLOCK(&m_BrainMutex);

	for(uint32 i=0; i<m_UAStaticList.Size();i++){
		ePipeline* Item = (ePipeline*)m_UAStaticList.GetData(i);
		tstring srcName = *(tstring*)Item->GetData(0);
		if (srcName == Name)
		{
			tstring srcCrypText = *(tstring*)Item->GetData(1);
			if (srcCrypText == Cryptograhp)
			{
				int64& HistoryDialogID = *(int64*)Item->GetData(3);
				HistoryDialogID = DialogID;
				return;
			}
		}
	}
}
bool CBrain::CLockedBrainData::FindRegisterUserInfo(tstring Name,tstring Cryptograhp,ePipeline& UserInfo){
	_CLOCK(&m_BrainMutex);

    for(uint32 i=0; i<m_UAStaticList.Size();i++){
		ePipeline* Item = (ePipeline*)m_UAStaticList.GetData(i);
		tstring srcName = *(tstring*)Item->GetData(0);
		if (srcName == Name)
		{
			tstring srcCrypText = *(tstring*)Item->GetData(1);
			if (srcCrypText == Cryptograhp)
			{
				int64 DialogType = *(int64*)Item->GetData(2);
				UserInfo = *Item;
				return true;
			}
		}
	}
	return false;
}


void CBrain::CLockedBrainData::RegisterOrgan(tstring UserName,tstring CryptTxt,int64 SourceID,DIALOG_TYPE Type){
	_CLOCK(&m_BrainMutex);
	COrgan User(UserName,CryptTxt,SourceID,Type);
	m_GUI_List.push_back(User);
}

void CBrain::CLockedBrainData::DelOrgan(int64 SourceID){
	_CLOCK(&m_BrainMutex);
	list<COrgan>::iterator it = m_GUI_List.begin();
	while(it != m_GUI_List.end()){
		COrgan& User = *it;
		if (User.m_SourceID == SourceID)
		{
			m_GUI_List.erase(it);
			break;
		}
		it++;
	}
}
const COrgan&  CBrain::CLockedBrainData::GetOrgan(int64 SourceID){
	static COrgan DefaultOrgan;
	_CLOCK(&m_BrainMutex);
	list<COrgan>::iterator it = m_GUI_List.begin();
	while(it != m_GUI_List.end()){
		COrgan& Organ = *it;
		if (Organ.m_SourceID == SourceID)
		{
			return Organ;
		}
		it++;
	}
	return DefaultOrgan;
};

void CBrain::CLockedBrainData::SendMsgToGUI(CBrain* Brain,int64 SourceID,CMsg& Msg){
	if (SourceID<1000)
	{
		Brain->SendMsgToLocalGUI(Msg);
		return;
	}

	CLinker Linker;
	Brain->GetLinker(SourceID,Linker);
	if(Linker.IsValid()){
		Linker().PushMsgToSend(Msg);
	}
}

void CBrain::CLockedBrainData::SendMsgToGUI(CBrain* Brain,CMsg& Msg,int64 ExcludeSourceID){
	_CLOCK(&m_BrainMutex);
	
	CLinker Linker;
	list<COrgan>::iterator it = m_GUI_List.begin();
	while(it != m_GUI_List.end()){
		COrgan& Organ = *it;

		CMsg m((ePipeline*)Msg.GetMsg().Clone());
		if (Organ.m_SourceID != ExcludeSourceID)
		{
			if (Organ.m_SourceID==LOCAL_GUI_SOURCE)
			{
				Brain->SendMsgToLocalGUI(m);
			}else{
				Brain->GetLinker(Organ.m_SourceID,Linker);
				if(Linker.IsValid()){
					Linker().PushMsgToSend(m);
				};
			}
		}
		it++;
	}
}

//继承的接口
//////////////////////////////////////////////////////////

void CBrain::NerveMsgProc(CMsg& Msg){
	int64 MsgID = Msg.GetMsgID();
	int64 DialogID = Msg.GetReceiverID();
	
	CLogicDialog* Dialog = m_BrainData.GetDialog(Msg.GetSourceID(),DialogID);
	
	if(Dialog){
		Dialog->Do(Msg);
	}else{
		int64 MsgID = Msg.GetMsgID();
		tstring MsgIDName = MsgID2Str(MsgID);
		if (MsgID == MSG_FROM_BRAIN)
		{
			ePipeline& Letter = Msg.GetLetter();
			int64 ChildMsgID = Letter.PopInt();
			tstring s = Format1024(_T("Dialog[%I64d] is not exist! %s:%I64d losted"),DialogID,MsgIDName.c_str(),ChildMsgID);
			OutSysInfo(s.c_str());
		} 
		//else if(MsgID != MSG_EVENT_TICK)
		else {
			tstring s = Format1024(_T("Dialog[%I64d] is not exist! %s losted"),DialogID,MsgIDName.c_str());
			OutSysInfo(s.c_str());
		}
	}
}

BOOL  CBrain::CreateCentralNerveWorkerStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime){

	int32 CentralMsgNum = GetCentralNerveMsgNum();
	ePipeline Data;
	Data.PushInt(CentralMsgNum);
	NotifySysState(NOTIFY_MODEL_SCENE,NTID_NERVE_MSG_NUM,&Data);

	int64 t = NewMsgPushTime-LastMsgPopTime; //convert into second

	//If more than 10 messages in the central nerve or there is message waiting for being handled more than 2 seconds, create a new thread
	uint32 Reason ;
	bool ret = m_ModelData.RequestCreateNewCentralNerveWorker(CentralMsgNum,t,Reason);
	if (!ret)
	{
		if (Reason == REASON_LIMIT)
		{
			NotifySysState(NOTIFY_MODEL_SCENE,NTID_NERVE_THREAD_LIMIT,NULL);
		}
		return FALSE;
	}

	if (Reason == REASON_MSG_TOO_MUCH )
	{
		if(CentralMsgNum==0)return FALSE;

		int32 NerveMsgNum = GetNerveMsgNum();
		float f = (float32)(CentralMsgNum-NerveMsgNum)/(float32)CentralMsgNum*100;
		if (f<10.0f)
		{
			return FALSE;  //不批准
		}
	}

	CThreadWorker* CentralNerveWork =m_ModelData.CreateThreadWorker(NewMsgPushTime,this,MODEL_CENTRAL_NEVER_WORK_TYPE);
	if (!CentralNerveWork)
	{
		return FALSE;
	}
	if (CentralNerveWork->Activation())
	{
		int n =m_ModelData.GetCentralNerveWorkerNum();
		ePipeline Data;
		Data.PushInt(n);
		Data.PushInt(CentralNerveWork->m_ID);
		NotifySysState(NOTIFY_MODEL_SCENE,NTID_NERVE_THREAD_JOIN,&Data);

		return TRUE;
	}else{
		m_ModelData.DeleteThreadWorker(this,CentralNerveWork->m_ID,MODEL_CENTRAL_NEVER_WORK_TYPE);
		NotifySysState(NOTIFY_MODEL_SCENE,NTID_NERVE_THREAD_FAIL,NULL);
		return FALSE;
	}


	return FALSE;
}

BOOL CBrain::CreateNerveWorkerStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime){
	int32 NerveMsgNum = GetNerveMsgNum();

	ePipeline Data;
	Data.PushInt(NerveMsgNum);
	NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_NERVE_MSG_NUM,&Data);

	int64 t = NewMsgPushTime-LastMsgPopTime;

	//If more than 10 messages in the central nerve or there is message waiting for being handled more than 2 seconds, create a new thread
	uint32 Reason ;
	bool ret = m_SystemData.RequestCreateNewNerveWorker(NerveMsgNum,t,Reason);
	if (!ret)
	{
		if (Reason == REASON_LIMIT)
		{

			NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_NERVE_THREAD_LIMIT,NULL);
		}
		return FALSE;
	}

	if (Reason == REASON_MSG_TOO_MUCH)
	{
		if(NerveMsgNum==0)return NULL;

		int32 CentralMsgNum = GetCentralNerveMsgNum();
		float f = (float32)(NerveMsgNum-CentralMsgNum)/(float32)NerveMsgNum*100;
		if (f<10.0f)
		{
			return NULL;  //不批准
		}
	};

	CThreadWorker* NerveWork = m_SystemData.CreateThreadWorker(NewMsgPushTime,this,SYSTEM_NEVER_WORK_TYPE);
	if (!NerveWork){
		return FALSE;
	}

	if(NerveWork->Activation())
	{
		int n = m_SystemData.GetNerveWorkerNum();
		ePipeline Data;
		Data.PushInt(n);
		Data.PushInt(NerveWork->m_ID);
		NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_NERVE_THREAD_JOIN,&Data);
		return TRUE;
	}else{
		m_SystemData.DeleteThreadWorker(this,NerveWork->m_ID,SYSTEM_NEVER_WORK_TYPE);
		NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_NERVE_THREAD_FAIL,NULL);
	}
	return FALSE;
}

void CBrain::CreateWebsokectWorkerStrategy(){
	int32 WorkerNum = m_BrainData.GetWebIOWorkerNum();
	int32 LinkerNum = m_WebsocketClientList.GetLinkerNum();
	if(WorkerNum==0 || (LinkerNum >10 && WorkerNum < GetCpuNum())){
		int64 ID = AbstractSpace::CreateTimeStamp();
		CThreadWorker* IOWork = m_BrainData.CreateThreadWorker(ID,this,BRAIN_WEBSOCKET_IO_WORK_TYPE);
		if(!IOWork || !IOWork->Activation()){
			if(IOWork)m_BrainData.DeleteThreadWorker(this,ID,BRAIN_WEBSOCKET_IO_WORK_TYPE);
			tstring s= Format1024(_T("Can not start websocket io worker thread"));
			OutSysInfo(s.c_str());
		}
	}
}

//通用函数
//////////////////////////////////////////////////////////////////////////
CBrain::CLockedBrainData* CBrain::GetBrainData(){
    return &m_BrainData;
}


void CBrain::SendMsgToSpace(CMsg& Msg){
	CLinker Linker;
	GetLinker(SPACE_SOURCE,Linker);
	if (Linker.IsValid())
	{
		Linker().PushMsgToSend(Msg);
	}
}


void CBrain::UnitTest(){
	/*测试单个本能命令
	define int 23;
	define float 45.4;
	define string "hello world";
	define int 2,define int 4,use operator +;
	*/

	/*测试简单逻辑
	think logic a;
	define int 3,define int 345,use operator +;
	use logic a;
	*/
	
	/*测试基本循环

	循环输出“hello world";

	think logic a; 
	define int 0, use cp cp; 

	think logic b11;
	use rs 0, and reference cp cp; 

	think logic b1;
	reference cp cp , define int 0, use operator +,use logic b11,define int 2, use operator <; 

	think logic b; 
	use logic b1, and define string "hello world"; 

	think logic c1; 
	use dd 1, view pipe, use rs 1000,goto label label1 ; 

	think logic c2;
	use dd 0, use rs 1000; 

	think logic c; 
	use logic c1, and use logic c2; 

	think logic abc; 
	use logic a, set label label1, use logic b, use logic c; 


	版本2：采用逐步细化的方式构建逻辑

	think logic lg0; 
	use logic lg1, use logic lg2; 

	think logic lg1; 
	define int 0, use cp cp; 

	think logic lg2; 
	set label cycle,use logic lg21, and define string "hello world", use logic lg3; 

	think logic lg21;
	reference cp cp , define int 1, use operator +,use logic lg22,define int 6, use operator <; 

	think logic lg22;
	use rs 0, and reference cp cp; 

	think logic lg3; 
	use logic lg31, and use logic lg32; 

	think logic lg31; 
	use dd 1, view pipe, use rs 1000,goto label cycle; 

	think logic lg32;
	use dd 0, use rs 1000; 

	*/

	/*测试调试
	用以上循环逻辑单步测试
	*/

	/*编码和解码数据
	think logic lg1;
	define int32 29,define string "hello",encode pipe,view pipe,decode capacitor cp1,view pipe;
	use logic lg1;
	*/
	/*测试外部物体

	*/

	/*动态改变逻辑

	*/
}

void CBrain::ConnectSpace(CLogicDialog* Dialog,tstring& ip){

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	//////////////////////////////////////////////////////////////////////////	

	CLinker Linker;
	GetLinker(SPACE_SOURCE,Linker);	
	if(Linker.IsValid()){
		//if( ac->ip == Linker().m_IP)return FALSE;
		Linker().Close();
		CSuperiorLinkerList*  SuperiorList = GetSuperiorLinkerList();
		SuperiorList->DeleteLinker(SPACE_SOURCE);
	}
	
	CBrain::CLockedBrainData* LockedData = GetBrainData();
	LockedData->DeleteDialog(SPACE_SOURCE,DEFAULT_DIALOG);
	
	
	CMsg GuiMsg1(Receiver,MSG_BRAIN_TO_GUI,0);	
	ePipeline Cmd(GUI_CONNECT_STATE);
	Cmd.PushInt(CON_START);
	Cmd.PushString(ip);
	
    GuiMsg1.GetLetter().PushPipe(Cmd);

	GetBrainData()->SendMsgToGUI(this,GuiMsg1,-1); //通知所有GUI
	
	//设置一个新对话
	CLogicDialog* NewDialog = LockedData->CreateNewDialog(this,SPACE_SOURCE,DEFAULT_DIALOG,NO_PARENT,_T("Wait connect.."),_T("You"),DIALOG_OTHER_MAIN,TASK_OUT_DEFAULT);	
	NewDialog->m_bEditValid = false;

	tstring Error;
	AnsiString ip1 = WStoUTF8(ip);		
	if(!Connect(SPACE_SOURCE, ip1.c_str(),SPACE_PORT,5,Error,true)){
		//不成功则删除对话
		LockedData->DeleteDialog(SPACE_SOURCE,DEFAULT_DIALOG);
		
		CMsg GuiMsg2(Receiver,MSG_BRAIN_TO_GUI,0);		
		ePipeline Cmd(GUI_CONNECT_STATE);
		Cmd.PushInt(CON_END);
		Cmd.PushString(ip);
		Cmd.PushInt(FALSE);	
		GuiMsg2.GetLetter().PushPipe(Cmd);
		
		GetBrainData()->SendMsgToGUI(this,GuiMsg2,-1);
		return;
	}
	
	CMsg GuiMsg3(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd2(GUI_CONNECT_STATE);
	Cmd2.PushInt(CON_END);
	Cmd2.PushString(ip);
	Cmd2.PushInt(TRUE);	
    GuiMsg3.GetLetter().PushPipe(Cmd2);	
	GetBrainData()->SendMsgToGUI(this,GuiMsg3,-1);
}


void CBrain::OutSysInfo(const TCHAR* Text){
	CLogicDialog* Dialog = m_BrainData.GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
	
	if(!Dialog)return; 
	
	int64 ID= AbstractSpace::CreateTimeStamp();
	ePipeline* Item = new ePipeline(Dialog->m_DialogName.c_str(),ID);	
    if (!Item)
    {
		return;
    }

	Item->PushInt(0);
	Item->PushString(Text);

	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(DIALOG_INFO);
	nf.Push_Directly(Item);
	nf.Notify(Dialog);

};


void CBrain::OutSysStatus(ePipeline& Data){
	CLogicDialog* Dialog = m_BrainData.GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
	if(!Dialog)return; 
	
	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(STATUS_INFO);
	nf.PushPipe(Data);
	nf.Notify(Dialog);
};


bool CBrain::RunExportObject(CLogicDialog* Dialog,CMsg* Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	
	int64 EventID = Msg->GetEventID();
	CLockedBrainData* BrainData = GetBrainData();

#ifdef _DEBUG
	tstring memo = Format1024(_T("%s %d"),_T(__FILE__),__LINE__);
	BrainData->PushBrainEvent(EventID,EventID,ExePipe,LocalAddress,TIME_SEC,true,memo);
#else
	BrainData->PushBrainEvent(EventID,EventID,ExePipe,LocalAddress,TIME_SEC,true);
#endif

	CLinker Linker;
	GetLinker(SPACE_SOURCE,Linker);
	if(!Linker.IsValid()){
		return false;
	}
	
	//发执行信息
	CMsg rMsg(MSG_ROBOT_EXPORT_OBJECT,NULL,EventID);
	ePipeline& Letter = rMsg.GetLetter();
	Letter<<Msg->GetLetter();
	rMsg.GetSender() = LocalAddress;
	Linker().PushMsgToSend(rMsg);
	
	ExePipe.SetID(RETURN_WAIT);
    ExePipe.GetLabel() = Format1024(_T("Waiting for export external object"));
		
	return true;
}






