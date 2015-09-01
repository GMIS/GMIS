// Brain.cpp: implementation of the CBrain class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)


#include "Brain.h"
#include "UserLinkerPipe.h"
#include "Msg.h"
#include "BrainObject.h"
#include "LogicThread.h"
#include "BrainTask.h"
#include "NotifyMsgDef.h"
#include "TaskDialog.h"
#include "GUIMsgDefine.h"

//CArm  CBrain::m_LeftArm;

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



void CNotifyState::Notify(CTaskDialog* Dialog){	
	Dialog->m_Brain->NoitfyDialogState(Dialog,this);
    assert(Size()==0);
}


//CLockedBrainData
//////////////////////////////////////////////////////////////////////////
CBrain::CLockedBrainData::CLockedBrainData(){
   m_bCheckPool = false;
   m_bCheckEvent = false;
}

CBrain::CLockedBrainData::~CLockedBrainData(){

	map<int64,CLogicThread*>::iterator  It_int64_thread = m_LogicThreadList.begin();
	while (It_int64_thread != m_LogicThreadList.end())
	{
		CLogicThread* Think = It_int64_thread->second;
		delete Think;
		It_int64_thread++;
	}
	m_LogicThreadList.clear();
	
	deque<CLogicThread*>::iterator It_thread = m_LogicThreadPool.begin();  
	while(It_thread != m_LogicThreadPool.end()){
		CLogicThread* Think = *It_thread;
		delete Think;
		It_thread++;
	}
    m_LogicThreadPool.clear();

	map<int64,CBrainTask*>::iterator  It_int64_braintask = m_LogicTaskList.begin();
	while (It_int64_braintask != m_LogicTaskList.end())
	{
		CBrainTask* Task = It_int64_braintask->second;
		delete Task;
		It_int64_braintask++;
	}
	m_LogicTaskList.clear();
	
	deque<CBrainTask*>::iterator It_braintask = m_LogicTaskPool.begin();  
	while(It_braintask != m_LogicTaskPool.end()){
		CBrainTask* Task = *It_braintask;
		delete Task;
		It_braintask++;
	}
	m_LogicTaskPool.clear();

	map<int64,CTaskDialog*>::iterator  It_int64_taskdialog = m_DialogList.begin();
	while (It_int64_taskdialog != m_DialogList.end())
	{
		CTaskDialog* Dialog = It_int64_taskdialog->second;
		delete Dialog;
		It_int64_taskdialog++;
	}
	m_DialogList.clear();
	
	deque<CTaskDialog*>::iterator It_taskdialog = m_DialogPool.begin();  
	while(It_taskdialog != m_DialogPool.end()){
		CTaskDialog* Dialog = *It_taskdialog;
		delete Dialog;
		It_taskdialog++;
	}
    m_DialogPool.clear();
};



void CBrain::CLockedBrainData::PushBrainEvent(int64 EventID,ePipeline& ExePipe,ePipeline& LocalAddress,int64 EventInterval){
	CLock lk(&m_EventMutex);

	ePipeline& Data = m_EventList[EventID];
	
	Data.Clear();
		
	int64 TimeStamp = AbstractSpace::CreateTimeStamp();
	Data.SetID(TimeStamp);

	if (EventInterval<MIN_EVENT_INTERVAL)
	{
		EventInterval = MIN_EVENT_INTERVAL;
	}

	Data.PushInt(EventInterval);
	Data.PushInt(0);
	Data.Push_Directly(LocalAddress.Clone());
	Data.Push_Directly(ExePipe.Clone());
};

bool CBrain::CLockedBrainData::PopBrainEvent(int64 EventID,ePipeline& ObjectInfo){
	CLock lk(&m_EventMutex);
	
	map<int64,ePipeline>::iterator it = m_EventList.find(EventID);
	if (it != m_EventList.end())
	{
		ePipeline& Obj = it->second;

		ObjectInfo << Obj;
		m_EventList.erase(it);
		return true;
	}	
	return false;
};

void CBrain::CLockedBrainData::ResetEventTickCount(int64 EventID){
	CLock lk(&m_EventMutex);
	map<int64, ePipeline>::iterator it = m_EventList.find(EventID);
    if (it != m_EventList.end())
    {
		ePipeline& EventInfo = it->second;
		int64& TickCount = *(int64*)EventInfo.GetData(EVENT_TICK_COUNT);
	    TickCount = 0;
	}
}

CLogicThread* CBrain::CLockedBrainData::CreateLogicThread(int64 UserID,int64 ThreadID /*=0*/){

	if(ThreadID==0){
		ThreadID = AbstractSpace::CreateTimeStamp();
	}
	CLock lk(&m_BrainMutex);

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
	CLock lk(&m_BrainMutex);
	map<int64,CLogicThread*>::iterator it = m_LogicThreadList.find(ThreadID);
	assert(it != m_LogicThreadList.end());
    CLogicThread* LogicThread =  it->second;
	return LogicThread;
};

void    CBrain::CLockedBrainData::DeleteLogicThread(int64 ThreadID){
	CLock lk(&m_BrainMutex);
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
	CLock lk(&m_BrainMutex);
	map<int64,CLogicThread*>::iterator it = m_LogicThreadList.find(ThreadID);
	assert(it != m_LogicThreadList.end());
    if(it == m_LogicThreadList.end())return;
	CLogicThread* LogicThread =  it->second;
	LogicThread->m_UserDialogID = UserID;
}


CBrainTask*   CBrain::CLockedBrainData::CreateLogicTask(int64 UserID,int64 TaskID/*=0*/){
	if (TaskID==0)
	{
		TaskID = AbstractSpace::CreateTimeStamp();
	}

	CLock lk(&m_BrainMutex);
	deque<CBrainTask*>::iterator it = m_LogicTaskPool.begin();
	if (it != m_LogicTaskPool.end())
	{   
		CBrainTask* Task = *it;
		m_LogicTaskList[TaskID] = Task;
        Task->Reset(TaskID,UserID);
		m_LogicTaskPool.pop_front();
		return Task;
	}

	CBrainTask* Task = new CBrainTask(TaskID,UserID);
    m_LogicTaskList[TaskID] = Task;
	return Task;
};

CBrainTask* CBrain::CLockedBrainData::GetLogicTask(int64 TaskID,bool bRequirLock /*= true*/){
	if (!bRequirLock)
	{
		map<int64,CBrainTask*>::iterator it = m_LogicTaskList.find(TaskID);
		assert(it != m_LogicTaskList.end());
		CBrainTask* Task =  it->second;
		return Task;
	}
    CLock lk(&m_BrainMutex);
	map<int64,CBrainTask*>::iterator it = m_LogicTaskList.find(TaskID);
	assert(it != m_LogicTaskList.end());
    CBrainTask* Task =  it->second;
	return Task;
};

void  CBrain::CLockedBrainData::DeleteLogicTask(int64 TaskID){
	
	CLock lk(&m_BrainMutex);
	map<int64,CBrainTask*>::iterator it = m_LogicTaskList.find(TaskID);
	assert(it != m_LogicTaskList.end());
    CBrainTask* LogicTask =  it->second;
    LogicTask->Reset(0,-1);
    m_LogicTaskList.erase(it);
	m_LogicTaskPool.push_back(LogicTask);
};

void  CBrain::CLockedBrainData::SetLogicTaskUser(int64 TaskID,int64 UserID){
	if (TaskID==0)
	{
		return;
	}
	CLock lk(&m_BrainMutex);
	map<int64,CBrainTask*>::iterator it = m_LogicTaskList.find(TaskID);
	assert(it != m_LogicTaskList.end());
    if(it == m_LogicTaskList.end())return;
	CBrainTask* Task =  it->second;
	Task->m_UserDialogID = UserID;
}


void CBrain::CLockedBrainData::AddGuiSourceID(int64 ID){
	CLock lk(&m_BrainMutex);
	m_GuiSourceList.insert(ID);
};

bool CBrain::CLockedBrainData::IsAllowedSourceID(int64 ID){
	if (ID == SYSTEM_SOURCE)
	{
		return true;
	}
	CLock lk(&m_BrainMutex);
	set<int64>::iterator it = m_GuiSourceList.find(ID);
	return it != m_GuiSourceList.end();
}

CTaskDialog* CBrain::CLockedBrainData::CreateNewDialog(CBrain* Frame,int64 SourceID,int64 DialogID,int64 ParentDialogID,
						  tstring SourceName,tstring DialogName,DIALOG_TYPE Type,int64 OutputSourceID,TASK_OUT_TYPE TaskType
						  )
{

	CTaskDialog* NewDialog = NULL;
	CLock lk(&m_BrainMutex);
	deque<CTaskDialog*>::iterator it = m_DialogPool.begin();
	while(it != m_DialogPool.end()){
		CTaskDialog* Old = *it;
		if (Old->GetSysProcNum()==0){
			NewDialog = Old;
			NewDialog->Reset(Frame,SourceID,DialogID,ParentDialogID,SourceName,DialogName,Type,OutputSourceID,TaskType);
			m_DialogPool.erase(it);
			break;
		}
		it++;
	}

	if (NewDialog==NULL)
	{
		NewDialog = new CTaskDialog(Frame,SourceID,DialogID,ParentDialogID,SourceName,DialogName,Type,OutputSourceID,TaskType);
		if (NewDialog == NULL)
		{
			return NULL;
		}
	}

	assert(m_DialogList.find(NewDialog->GetDialogID()) == m_DialogList.end());
	m_DialogList[NewDialog->GetDialogID()] = NewDialog;

	return NewDialog;
}

CTaskDialog* CBrain::CLockedBrainData::GetDialog(int64 DialogID){
	CLock lk(&m_BrainMutex);
	CTaskDialog* Dialog = NULL;
	map<int64,CTaskDialog*>::iterator it = m_DialogList.find(DialogID);
	if (it != m_DialogList.end())
	{
		Dialog =  it->second;

	}
	return Dialog;
}


void   CBrain::CLockedBrainData::DeleteDialog(int64 DialogID){
	CLock lk(&m_BrainMutex);
	map<int64,CTaskDialog*>::iterator it= m_DialogList.find(DialogID);
	if (it != m_DialogList.end())
	{
		CTaskDialog* Dialog = it->second;
		
		Dialog->SetTaskState(TASK_DELELTE);
		Dialog->NotifyTaskState();

		m_DialogPool.push_back(Dialog);
		
		it->second = NULL;
		m_DialogList.erase(it);	
	}				
}

void CBrain::CLockedBrainData::DeleteAllDialog(){
	CLock lk(&m_BrainMutex);
	map<int64,CTaskDialog*>::iterator it= m_DialogList.begin();
	while (it != m_DialogList.end())
	{
		CTaskDialog* Dialog = it->second;
		Dialog->SetTaskState(TASK_DELELTE);
		m_DialogPool.push_back(Dialog);
		it->second = NULL;
		it = m_DialogList.erase(it);
	}				
}
int   CBrain::CLockedBrainData::DeleteDialogOfSource(int64 SourceID){
	CLock lk(&m_BrainMutex);
	int n=0;
	map<int64,CTaskDialog*>::iterator it= m_DialogList.begin();
	while (it != m_DialogList.end())
	{
		CTaskDialog* Dialog = it->second;
		
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
	CLock lk(&m_BrainMutex);	
	int64 TaskID = DialogID;
	if (DialogID == DEFAULT_DIALOG)
	{
		TaskID = SourceID;
	}
	
	map<int64,CTaskDialog*>::iterator it = m_DialogList.find(TaskID);
	assert(it != m_DialogList.end());
	if (it == m_DialogList.end())
	{
		return ;
	}
	
	CTaskDialog* Dialog = it->second;	
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

	//装入最后20个运行时记录
	ePipeline& RuntimeInfo = Dialog->m_RuntimeOutput;
	
	P2 = RuntimeInfo.Size(); 
	P1 = P2-n>0?P2-n:0;
	
	if (P1>0)
	{
		ePipeline* Empty = new ePipeline(-1);
		Empty->PushInt(P1);  //还有多少数据没显示
		ItemList.Push_Directly(Empty);
	}    
	
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


void  CBrain::CLockedBrainData::GetMoreLog(int64 SourceID, int64 DialogID,int64 LastItemID,ePipeline& Pipe){
	CLock lk(&m_BrainMutex);	

	int64 TaskID = DialogID;
	if (DialogID == DEFAULT_DIALOG)
	{
		TaskID = SourceID;
	}
	
	map<int64,CTaskDialog*>::iterator it = m_DialogList.find(TaskID);
	assert(it != m_DialogList.end());
	if (it == m_DialogList.end())
	{
		return ;
	}
	
	CTaskDialog* Dialog = it->second;	
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

void CBrain::CLockedBrainData::GetAllDialogListInfo(ePipeline& LinkerInfo){
	CLock lk(&m_BrainMutex);	

	CTaskDialog* Dialog = NULL;
	map<int64,CTaskDialog*>::iterator it = m_DialogList.begin();
	while (it != m_DialogList.end())
	{
		Dialog =  it->second;
		int64 SourceID = Dialog->m_SourceID;
		int64 DialogID = Dialog->m_DialogID;
		int64 ParentID = Dialog->m_ParentDialogID;
		tstring& Name  = Dialog->m_DialogName;
		int32 Type = Dialog->m_SpaceType;
		
		LinkerInfo.PushInt(SourceID);
		LinkerInfo.PushInt(DialogID);
		LinkerInfo.PushInt(ParentID);
		LinkerInfo.PushString(Name);
		LinkerInfo.PushInt(Type);	
		it++;
	}
	
}
void CBrain::CLockedBrainData::BrainIdleProc(CBrain* Brain){
	if (m_bCheckPool)
	{
		return;
	}

    CLock lk(&m_BrainMutex);
	m_bCheckPool = true;

	//检查Thread的有效性，无效的则归还给内存池
	if(m_LogicThreadList.size() > m_DialogList.size() ){
	   map<int64,CLogicThread*>::iterator it = m_LogicThreadList.begin();
	   while (it != m_LogicThreadList.end())
	   {
		   CLogicThread* Think = it->second;
		   int64 DialogID = Think->m_UserDialogID;
		   map<int64,CTaskDialog*>::iterator it1 = m_DialogList.find(DialogID);
		   if (it1 == m_DialogList.end())
		   {
			   it = m_LogicThreadList.erase(it);
			   Think->Reset(0,-1);
			   if (m_LogicThreadList.size()>20) //控制在20个
			   {
				   delete Think;
			   }else{ //放入pool中
				   m_LogicThreadPool.push_back(Think);	   
			   }
			   continue;
		   }
		   CTaskDialog* Dilaog = it1->second;
		   if(Dilaog->m_ThinkID != Think->m_ID){
			   it = m_LogicThreadList.erase(it);
			   Think->Reset(0,-1);
			   if (m_LogicThreadList.size()>20) //控制在20个
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
		map<int64,CBrainTask*>::iterator it = m_LogicTaskList.begin();
		while (it != m_LogicTaskList.end())
		{
			CBrainTask* Task = it->second;
			int64 DialogID = Task->m_UserDialogID;

			if (Task->m_UserDialogID == 1 ) //这种情况表明被临时逻辑占用
			{
				it++;
				continue;
			}
			map<int64,CTaskDialog*>::iterator it1 = m_DialogList.find(DialogID);
			if (it1 == m_DialogList.end())
			{
				it = m_LogicTaskList.erase(it);
				Task->Reset(0,-1);
				if (m_LogicTaskList.size()>20) //控制在20个
				{
					delete Task;
				}else{ //放入pool中
					m_LogicTaskPool.push_back(Task);	   
				}
				continue;
			}
			CTaskDialog* Dilaog = it1->second;
			if(Dilaog->m_TaskID != Task->m_ID){
				it = m_LogicTaskList.erase(it);
				Task->Reset(0,-1);
				if (m_LogicTaskList.size()>20) //控制在20个
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
	deque<CTaskDialog*>::iterator it = m_DialogPool.begin();
	while (it != m_DialogPool.end() && m_DialogPool.size()>20) //暂停pool大小为20
	{
		CTaskDialog* Dialog = *it;
		if (Dialog->GetSysProcNum()==0)
		{
			delete Dialog;
			it = m_DialogPool.erase(it);
		}else{
			it++;
		}
	}
	m_bCheckPool=false;

}

void  CBrain::CLockedBrainData::GetBrainInfo(ePipeline& Info){
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

	int GuiNum = m_GuiSourceList.size();
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
    if (m_bCheckEvent) //避免多个线程同时处理此函数
    {
		return;
    }
	CLock lk(&m_EventMutex);
	
	m_bCheckEvent = true;

	int64 NewTimeStamp = AbstractSpace::CreateTimeStamp();
	
	map<int64, ePipeline>::iterator it = m_EventList.begin();
    while (it != m_EventList.end())
    {
		ePipeline& EventInfo = it->second;
		int64 OldTimeStamp = EventInfo.GetID();
		int64 t = NewTimeStamp-OldTimeStamp;
		
		int64 Interval = *(int64*)EventInfo.GetData(EVENT_INTERVAL);
		 
		if (t>=Interval) //达到预定间隔时间
		{
			EventInfo.SetID(NewTimeStamp); 

			ePipeline& Address = *(ePipeline*)EventInfo.GetData(EVENT_ADDRESS); 	
			int64& TickCount = *(int64*)EventInfo.GetData(EVENT_TICK_COUNT);
			
			if (TickCount==0) //事件正常，发出一个TICK
			{						
				TickCount++;
				if (Address.m_ID ==0) //本地事件
				{
					CMsg Msg(Address,MSG_EVENT_TICK,it->first);
					Msg.GetLetter().PushInt(NewTimeStamp);
					
					Brain->PushNerveMsg(Msg);
				}else{ //对于外部事件，由于地址指向外部，所以TICK只能发往事件对应的对话
					int64 DialogID = it->first;
					CMsg Msg(DialogID,MSG_EVENT_TICK,DEFAULT_DIALOG,it->first);
					Msg.GetLetter().PushInt(NewTimeStamp);
					
					Brain->PushNerveMsg(Msg);
				}
				it++;
			} 
			else  //说明在事件发出TICK信息还没得到响应，检查事件地址是否依然有效
			{     //如果有效继续等待，对方一定会处理
						
				if (Address.m_ID==0) //本地事件则
				{
					int64 DialogID = *(int64*)Address.GetData(0);
					
					CTaskDialog* Dialog = GetDialog(DialogID);
					
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
							int64 EventID = it->first;
							
							CTaskDialog* Dlg = GetDialog(EventID);
							if (Dlg)
							{
								CNotifyState nf(NOTIFY_DIALOG_LIST);
								nf.PushInt(DL_DEL_DIALOG);
								nf.Notify(Dlg);

								DeleteDialog(EventID);
							}
							it = m_EventList.erase(it);
							continue;
						}
					}
					
					//检查事件用户依然存在后，我们更新时间戳，避免频繁检查
					EventInfo.SetID(NewTimeStamp); 
					it++;
				} 
				else
				{
					int64 DialogID = it->first;
					CTaskDialog* Dialog = GetDialog(DialogID);
					if (!Dialog)
					{
						//如果事件没有反馈对象，那么事件本身所对应的对话就应该停止，然后被删除
						ePipeline ChildMsg(GUI_TASK_CONTROL);
						ChildMsg.PushInt(CMD_STOP);
						
						CMsg Msg;
						CreateBrainMsg(Msg,it->first,ChildMsg,0);
						Brain->PushNerveMsg(Msg);
						
						it = m_EventList.erase(it);	
						continue;
					}

					//检查事件用户依然存在后，我们更新时间戳，避免频繁检查
					EventInfo.SetID(NewTimeStamp); 
					it++;							
				}		
			}


		}else{
			it++;
		}
	}
	m_bCheckEvent = false;
}


Energy*  CBrain::CLockedBrainData::ToEnergy(){

		
	ePipeline* Pipe = new ePipeline;
	eElectron e(Pipe);
	

	CLock lk2(&m_EventMutex);

	ePipeline EventList;
	map<int64, ePipeline>::iterator it3= m_EventList.begin();
	while(it3!=m_EventList.end()){
		int64 EventID = it3->first;
		ePipeline& EventInfo = it3->second;
		EventInfo.SetID(EventID);
		EventList.Push_Directly(EventInfo.Clone());
		it3++;
	}
	Pipe->PushPipe(EventList);


	CLock lk(&m_BrainMutex);
	ePipeline DialogList;
	map<int64,CTaskDialog*>::iterator it = m_DialogList.begin();
	while(it != m_DialogList.end()){
		CTaskDialog* Dialog = it->second;
	    bool ret = Dialog->SuspendTask();
		assert(ret);
		if (!ret)
		{
			continue;
		}
		Energy* E = Dialog->ToEnergy();
		if(E){
			ePipeline* p = (ePipeline*)E;
			DialogList.Push_Directly(E);
		}
		it++;
	}
	Pipe->PushPipe(DialogList);
	
	return e.Release();
}

bool  CBrain::CLockedBrainData::FromEnergy(CBrain* Brain,Energy* E){
	DeleteAllDialog();

	ePipeline* Pipe = (ePipeline*)E;
	assert(Pipe->Size()==2);

	int i;
	
	m_EventMutex.Acquire();
	ePipeline* EventList = (ePipeline*)Pipe->GetData(0);
	for (i=0; i<EventList->Size(); i++)
	{
		ePipeline* EventInfo = (ePipeline*)EventList->GetData(i);
	    int64 EventID = EventInfo->GetID();
        EventInfo->SetID(0);
		m_EventList[EventID] = *EventInfo;

	}
	m_EventMutex.Release();
	
	//CLock lk2(&m_Mutex); 会导致嵌套死锁，也无必要
	ePipeline* DialogList = (ePipeline*)Pipe->GetData(1);
	for (i=0; i<DialogList->Size();i++)
    {
		ePipeline* p = (ePipeline*)DialogList->GetData(i);
		
		int64 SourceID = *(int64*)p->GetData(0);
		int64 DialogID = *(int64*)p->GetData(1);

		if (SourceID!=0) //外部对话忽略
		{
			continue;
		}
		CTaskDialog* Dialog =  CreateNewDialog(Brain,SourceID,DialogID,0,_T(""),_T(""),DIALOG_SYSTEM_MAIN,0,TASK_OUT_DEFAULT);
		bool ret = Dialog->FromEnergy(p);
		if (!ret)
		{
			return false;
		};
		//SetWorkMode(WORK_DEBUG);
		Dialog->NotifyTaskState();
		//Dialog->ResumeTask();
	}
	return true;
}

//CBrain
//////////////////////////////////////////////////////////////////////

CBrain::CBrain(CSystemInitData* InitData)
:System(InitData)
{
	m_CurDebugTimeStamp = 0;
	m_LogFlag = 0;

	m_Account.Name = _T("");
	m_Account.Password = _T("");
	m_Account.CrypText = _T("");
	m_Account.ip = 0;

}



CBrain::~CBrain()
{
	CBrainMemory::Close();
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

bool CBrain::Activation(){
    if (m_Alive)
    {
		return true;
    }

	if(!System::Activation()){
	    m_ErrorInfo = _T("系统初始化失败");
		return false;
	}
	m_Alive = TRUE;

//	m_LeftArm.OpenSerialPort("COM3");

	CLockedBrainData* LockedData = GetBrainData();
	CTaskDialog* Dialog = LockedData->GetDialog(DEFAULT_DIALOG); 
	if (!Dialog)
	{
		Dialog = LockedData->CreateNewDialog(this,SYSTEM_SOURCE,DEFAULT_DIALOG,0,_T("You"),_T("System"),DIALOG_SYSTEM_MAIN,0,TASK_OUT_DEFAULT);
		if (!Dialog)
		{
			m_ErrorInfo = _T("系统对话生成失败");
			return false;
		}
		
		ePipeline ExePipe;
		ePipeline LocalAddress;
		LocalAddress.PushInt(DEFAULT_DIALOG);
		LockedData->PushBrainEvent(0,ExePipe,LocalAddress);
	}
		
	CMsg Msg(MSG_INIT_BRAIN,DEFAULT_DIALOG,0);	
	Dialog->SendMsg(Msg,LOCAL);
		
	return true;
}


void CBrain::Dead(){

	if(m_Alive == FALSE)return;

//	m_LeftArm.CloseSerialPort();

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
	WSACleanup();


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
void  CBrain::SendMsgToGUI(int64 GuiID,CMsg& GuiMsg){

}



//继承的接口
//////////////////////////////////////////////////////////

bool CBrain::Do(Energy* E){
 
	if (E==NULL)
	{
		m_BrainData.BrainIdleProc(this);
		m_BrainData.EventProc(this);
		return true;
	}

	ePipeline* Pipe = (ePipeline*)E;
	CMsg Msg(Pipe);



	int64 DialogID = Msg.GetReceiverID();

	if (DialogID == DEFAULT_DIALOG)//中枢神经只处理缺省对话
	{
		DialogID = Msg.GetSourceID();  //缺省对话的实际ID是其信息源ID
		CTaskDialog* Dialog = m_BrainData.GetDialog(DialogID);

		if(Dialog){
			Dialog->Do(Msg);  
		}else{
			//握手对话成功之前可能还没有产生对应的系统对话，此时转交缺省系统对话处理
			int64 MsgID = Msg.GetMsgID();
			if (MsgID == MSG_I_AM || MsgID == MSG_WHO_ARE_YOU || MsgID == MSG_CONNECT_OK)
			{
				Dialog = m_BrainData.GetDialog(DEFAULT_DIALOG);
				assert(Dialog);
				Dialog->Do(Msg);  
			}
		}

	}else{ 
		//其他信息转向子神经处理，避免中枢神经阻塞
		PushNerveMsg(Msg);
	}


	return TRUE;
}


void CBrain::NerveProc(CMsg& Msg){
	
	
	int64 MsgID = Msg.GetMsgID();
	
	int64 DialogID = Msg.GetReceiverID();
	

	CTaskDialog* Dialog = m_BrainData.GetDialog(DialogID);
	
	if(Dialog){
		Dialog->Do(Msg);
	}else{
		int64 MsgID = Msg.GetMsgID();
		tstring MsgStr = MsgID2Str(MsgID);
		if (MsgID == MSG_FROM_BRAIN)
		{
			ePipeline& Letter = Msg.GetLetter();
			int64 ChildMsgID = Letter.PopInt();
			tstring s = Format1024(_T("Dialog[%I64d] not exist! %s:%I64d losted"),DialogID,MsgStr.c_str(),ChildMsgID);
			OutSysInfo(s);
		} 
		else
		{
			tstring s = Format1024(_T("Dialog[%I64d] not exist! %s losted"),DialogID,MsgStr.c_str());
			OutSysInfo(s);

		}
	}
}

Model::CCentralNerveWork* CBrain::CreateCentralNerveWorker(int64 ID,Model* Parent,uint32 Reason){
	if (Reason == REASON_MSG_TOO_MUCH)
	{
		int32 NerveMsgNum = GetNerveMsgNum();
		int32 CentralMsgNum = GetCentralNerveMsgNum();
	
		float f = (float32)(CentralMsgNum-NerveMsgNum)/(float32)CentralMsgNum*100;
		if (f<10.0f)
		{
			return NULL;  //不批准
		}
	}

	System::CCentralNerveWork* NerveWork = new System::CCentralNerveWork(ID,this);
	return NerveWork;
}

System::CNerveWork* CBrain::CreateNerveWorker(int64 ID,System* Parent,uint32 Reason){

	if (ID!=0 && Reason == REASON_MSG_TOO_MUCH)
	{
		int32 NerveMsgNum = GetNerveMsgNum();
		int32 CentralMsgNum = GetCentralNerveMsgNum();
		float f = (float32)(NerveMsgNum-CentralMsgNum)/(float32)NerveMsgNum*100;
		if (f<10.0f)
		{
			return NULL;  //不批准
		}
	};
				
	CNerveWork* NerveWork = new CNerveWork(ID,this);
	return NerveWork;
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


void CBrain::ConnectSpace(CTaskDialog* Dialog,tstring& ip){

	int32 GuiID = LOCAL_GUI;
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	//////////////////////////////////////////////////////////////////////////	
	_ACCOUNT* ac = GetAccount();
	CLinker Linker;
	GetLinker(SPACE_SOURCE,Linker);	
	if(Linker.IsValid()){
		//if( ac->ip == Linker().m_IP)return FALSE;
		Linker().Close();
		CLockedLinkerList*  SuperiorList = GetSuperiorLinkerList();
		SuperiorList->DeleteLinker(SPACE_SOURCE);
	}
	
	CBrain::CLockedBrainData* LockedData = GetBrainData();
	LockedData->DeleteDialog(SPACE_SOURCE);
	
	
	CMsg GuiMsg1(Receiver,MSG_BRAIN_TO_GUI,0);	
	ePipeline Cmd(GUI_CONNECT_STATE);
	Cmd.PushInt(CON_START);
	Cmd.PushString(ip);
	
    GuiMsg1.GetLetter().PushPipe(Cmd);
	SendMsgToGUI(GuiID,GuiMsg1);	
	
	
	//设置一个新对话
	CTaskDialog* NewDialog = LockedData->CreateNewDialog(this,SPACE_SOURCE,DEFAULT_DIALOG,0,_T("Wait connect.."),_T("You"),DIALOG_OTHER_MAIN,LOCAL_GUI,TASK_OUT_DEFAULT);	
	
	tstring Error;
	AnsiString ip1 = WStoUTF8(ip);		
	if(!Connect(SPACE_SOURCE, ip1.c_str(),SPACE_PORT,5,Error,true)){
		//不成功则删除对话
		LockedData->DeleteDialog(SPACE_SOURCE);
		
		CMsg GuiMsg2(Receiver,MSG_BRAIN_TO_GUI,0);		
		ePipeline Cmd(GUI_CONNECT_STATE);
		Cmd.PushInt(CON_END);
		Cmd.PushString(ip);
		Cmd.PushInt(FALSE);	
		GuiMsg2.GetLetter().PushPipe(Cmd);
		
		SendMsgToGUI(GuiID,GuiMsg2);
		return;
	}
	
	
	CMsg GuiMsg3(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd2(GUI_CONNECT_STATE);
	Cmd2.PushInt(CON_END);
	Cmd2.PushString(ip);
	Cmd2.PushInt(TRUE);	
    GuiMsg3.GetLetter().PushPipe(Cmd2);	
	SendMsgToGUI(GuiID,GuiMsg3);
	
}



void CBrain::OutSysInfo(tstring s){
	OutSysInfo(s.c_str());
}

void CBrain::OutSysInfo(const TCHAR* Text){
	CTaskDialog* Dialog = m_BrainData.GetDialog(SYSTEM_SOURCE);
	
	if(!Dialog)return; 
	
	int64 ID= AbstractSpace::CreateTimeStamp();
	ePipeline* Item = new ePipeline(Dialog->m_DialogName.c_str(),ID);	
    if (!Item)
    {
		return;
    }

	Item->PushInt(0);
	Item->PushString(Text);

	CNotifyState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(DIALOG_INFO);
	nf.Push_Directly(Item);
	nf.Notify(Dialog);

};


void CBrain::OutSysStatus(ePipeline& Data){
	CTaskDialog* Dialog = m_BrainData.GetDialog(SYSTEM_SOURCE);
	if(!Dialog)return; 
	
	CNotifyState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(STATUS_INFO);
	nf.PushPipe(Data);
	nf.Notify(Dialog);
};


bool CBrain::RunExportObject(CTaskDialog* Dialog,CMsg* Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	
	int64 EventID = Msg->GetEventID();
	CLockedBrainData* BrainData = GetBrainData();
	BrainData->PushBrainEvent(EventID,ExePipe,LocalAddress);
	
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






