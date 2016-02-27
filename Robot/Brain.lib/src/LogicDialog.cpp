// TaskDialog.cpp: implementation of the CTaskDialog class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning (disable:4786)

#include "LogicDialog.h"
#include "Brain.h"

#include "Element.h"
#include "DefineDataOperator.h"
#include "BitwiseOperator.h"
#include "LogicElement.h"
#include "LogicOperator.h"
#include "MathematicalOperator.h"
#include "RelationalOperator.h"
#include "NotifyMsgDef.h"
#include "MsgList.h"
#include "GUIMsgDefine.h"


bool CRequestBrain::Request(CLogicDialog* Dialog){
	bool ret = Dialog->m_Brain->ProcessRequest(Dialog,this);
//	assert(Size()==0);
	return ret;
}


bool CNameList::HasName(CLogicDialog* Dialog,tstring& Name){
	map<tstring,CNameUser>::iterator it = m_NameList.find(Name);
	if (it == m_NameList.end())return false;
	CNameUser& NameUser = it->second;
	
	if (NameUser.m_TaskID!=0 && NameUser.m_TaskID == Dialog->m_TaskID)
	{
		return true;
	}
	
	if(Dialog->FindLogic(NameUser.m_LogicName)){
		return true;	
	};
	
	return false;
}

void  CNameList::RegisterNameByLogic(tstring& Name,tstring& LogicName){
	CNameUser& User = m_NameList[Name];
	User.m_LogicName = LogicName;
};

void  CNameList::RegisterNameByTask(tstring& Name,int64 TaskID,int64 InstanceID){
	CNameUser& User = m_NameList[Name];
	User.m_TaskID = TaskID;
	User.m_InstanceID =InstanceID;
};

void CNameList::UnregisterNameByTask(tstring& Name){
	map<tstring,CNameUser>::iterator it = m_NameList.find(Name);
	if (it == m_NameList.end())return;
	CNameUser& NameUser = it->second;
	NameUser.m_TaskID = 0;
};


int64 CNameList::GetInstanceID(tstring& Name){
	map<tstring, CNameUser>::iterator it = m_NameList.find(Name);
	if(it != m_NameList.end()){
		CNameUser& User = it->second;
		return User.m_InstanceID;
	};
	return 0;
};

Energy*  CNameList::ToEnergy(){
	ePipeline* Pipe = new ePipeline;

	eElectron e(Pipe);
	if (Pipe)
	{
		map<tstring, CNameUser>::iterator it = m_NameList.begin();
		while(it != m_NameList.end()){
			tstring Name = it->first;
			CNameUser& User = it->second;

			Pipe->PushString(Name);
			Pipe->PushString(User.m_LogicName);
			Pipe->PushInt(User.m_TaskID);
			Pipe->PushInt(User.m_InstanceID);

			it++;
		}	
		return e.Release();
	}
	return NULL;
};

bool     CNameList::FromEnergy(Energy* E){

	if (!E)
	{
		return false;
	}
	ePipeline* Pipe = (ePipeline*)E;

	assert(Pipe->Size()%4==0); //4的倍数

	while(Pipe->Size())
	{
		tstring Name = Pipe->PopString();
		CNameUser& User = m_NameList[Name];
		User.m_LogicName = Pipe->PopString();
		User.m_TaskID  = Pipe->PopInt();
		User.m_InstanceID = Pipe->PopInt();
	}
	return true;
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSpaceMutex  CLogicDialog::m_DialogMutex;

CLogicDialog::CLogicDialog(CBrain* Brain,int64 SourceID,int64 DialogID,int64 ParentDialogID,tstring SourceName,tstring DialogName,
						 DIALOG_TYPE Type,TASK_OUT_TYPE TaskType)
:m_TaskMsgList(&m_DialogMutex)
{
	m_DialogHistory.SetLabel(_T("Histroy"));  //设置名字便于调试
	m_RuntimeOutput.SetLabel(_T("RuntimeOutput"));
	m_SearchOutput.SetLabel(_T("SearchOutput"));

	Reset(Brain,SourceID,DialogID,ParentDialogID,SourceName,DialogName,Type,TaskType);
}

CLogicDialog::~CLogicDialog()
{

	ClearLogicSence();
	ClearObject();
	m_FindResultList.clear();
	m_FindSeedList.clear();

	ResetTask();

}

void CLogicDialog::Reset(CBrain* Brain,int64 SourceID,int64 DialogID,int64 ParentDialogID,tstring SourceName,tstring DialogName,
		   DIALOG_TYPE Type,TASK_OUT_TYPE TaskType)

{
 	    m_SourceName = SourceName;
		m_DialogName = DialogName;
		m_Brain = Brain;
		m_SourceID = SourceID;
		m_DialogID = DialogID;
		m_ParentDialogID = ParentDialogID;
		m_LastDebugTimeStamp = 0;

		m_DialogType = Type;
		m_TaskOutType = TaskType;
		m_SysProcCounter = 0;
		m_TaskState = TASK_IDLE;
		m_WorkMode = WORK_TASK;

		m_ItemNumPerPage   = 10;
		m_Interval = 5;

		m_bEditValid = true;
		m_EditText = _T("");
		
		m_DialogHistory.Clear();
		m_RuntimeOutput.Clear();
		m_SearchOutput.Clear();
		m_LogicItemTree.Clear();
		
		m_FocusPauseItemID = 0;
		m_PauseEventList.clear();

		m_ControlDialogID = 0;
			
		while (m_TaskMsgList.DataNum())
		{
			CMsg m;
			m_TaskMsgList.Pop(m);
		};

		
		m_ThinkID = 0;
		m_TaskID = 0;

		m_ExePipe.Clear();
		m_ExePipe.Reuse();
};

Energy*  CLogicDialog::ToEnergy(){

	_CLOCK2(&m_DialogMutex,this);

	ePipeline* Pipe = new ePipeline;
	eElectron e(Pipe);
	if (Pipe)
	{
		Pipe->PushInt(m_SourceID);
		Pipe->PushInt(m_DialogID);

		Pipe->PushString(m_SourceName);
		Pipe->PushString(m_DialogName);

		Pipe->PushInt(m_DialogType);
	    Pipe->PushInt(m_TaskOutType);
		
		Pipe->PushInt(m_ParentDialogID);
		Pipe->PushInt(m_LastDebugTimeStamp-1); //使其与m_LogicItemTree.GetID()不相等，这样恢复的时候才能发送调试数据;

		Pipe->PushInt(m_WorkMode);
		Pipe->PushInt(m_TaskState);

		ePipeline PauseEventList;
		map<int64,int64>::iterator it = m_PauseEventList.begin();
		while(it != m_PauseEventList.end())
		{
			PauseEventList.PushInt(it->first);
			PauseEventList.PushInt(it->second);
			it++;
		}
		Pipe->PushPipe(PauseEventList);

		Pipe->PushInt(m_FocusPauseItemID);
		Pipe->PushInt(m_ControlDialogID);

		Pipe->PushString(m_CompileError);


		Pipe->PushInt(m_SysProcCounter);

		Pipe->PushPipe(m_ExePipe);

		Pipe->PushInt(m_ThinkID);
		Pipe->PushInt(m_TaskID);		

		Pipe->PushString(m_CurLogicName);


		Pipe->PushPipe(m_DialogHistory);
		Pipe->PushPipe(m_RuntimeOutput);
		Pipe->PushPipe(m_SearchOutput);

		Pipe->PushInt(m_bEditValid);
		Pipe->PushString(m_EditText);
		Pipe->PushString(m_StatusText);

		Pipe->PushPipe(m_LogicItemTree);
	
		Pipe->PushInt(m_ObjectFocus);
		Pipe->PushInt(m_DataTableFocus);
		Pipe->PushString(m_LogicFocus);


		ePipeline  List;
		vector<CLocalLogicCell*>::iterator It = m_LogicList.begin();
		while(It!=m_LogicList.end()){
			CLocalLogicCell*  Item = *It;
			Energy* e = Item->ToEnergy();
			assert(e);
			if (e)
			{
				List.Push_Directly(e);
			}
			
			It++;
		}
		Pipe->PushPipe(List);
		List.Clear();

		vector<CElementCell*>::iterator Ita = m_CapaList.begin();
		while(Ita!=m_CapaList.end()){
			CElementCell* Item = *Ita;
			Energy* e = Item->ToEnergy();
			assert(e);
			if (e)
			{
				List.Push_Directly(e);
			}
			Ita++;
		}
		Pipe->PushPipe(List);
		List.Clear();

		vector<CElementCell*>::iterator Itb = m_InduList.begin();
		while(Itb!=m_InduList.end()){
			CElementCell* Item = *Itb;
			Energy* e = Item->ToEnergy();
			assert(e);
			if (e)
			{
				List.Push_Directly(e);
			}
			Itb++;
		}
		Pipe->PushPipe(List);
		List.Clear();

		vector<CObjectData*>::iterator Itc = m_ObjectList.begin();
		while(Itc != m_ObjectList.end())
		{
			CObjectData* ob = *Itc;
			ePipeline* Item = ob->GetItemData();
			assert(Item);
			if (Item)
			{
				List.Push_Directly(Item);
			}
			Itc++;
		}
		Pipe->PushPipe(List);
		List.Clear();

		map<int64, ePipeline>::iterator itd = m_ObjectInstanceList.begin();
		while (itd != m_ObjectInstanceList.end())
		{
			int64 ID = itd->first;
			ePipeline& Instance = itd->second;

			List.PushInt(ID);
			List.PushPipe(Instance);
			itd++;
		}
		Pipe->PushList(List);

		map<int64, ePipeline>::iterator ite = m_TableInstanceList.begin();
		while (ite != m_TableInstanceList.end())
		{
			int64 ID = ite->first;
			ePipeline& Instance = ite->second;
			
			List.PushInt(ID);
			List.PushPipe(Instance);
			ite++;
		}
		Pipe->PushList(List);

		map<int64, CElement*>::iterator itf = m_LogicInstanceList.begin();
		while (itf != m_LogicInstanceList.end())
		{
			int64 ID = itf->first;
			CElement* Instance = itf->second;
			Energy* E = ToEnergy();
			if(!E){
				return NULL;
			}

			List.PushInt(ID);
			List.Push_Directly(E);
			itf++;
		}
		Pipe->PushList(List);


		Energy* e1 = m_NamedObjectList.ToEnergy();
		if(!e1){
			return NULL;
		}
		Pipe->Push_Directly(e1);

		Energy* e2 = m_NamedTableList.ToEnergy();
		if(!e2){
			return NULL;
		}
		Pipe->Push_Directly(e2);

		if (m_TaskID!=0)
		{
			CLogicTask* Task  = m_Brain->GetBrainData()->GetLogicTask(m_TaskID,false); 
			Energy* E = Task->ToEnergy();
			if (!E)
			{
				return NULL;	
			}
			Pipe->Push_Directly(E);
		}

		if (m_ThinkID!=0)
		{
			//NOTE: 这里在得到thread时不能上锁，否则会引起嵌套锁而死机
			CLogicThread* Think = m_Brain->GetBrainData()->GetLogicThread(m_ThinkID,false); 
			Energy* E = Think->ToEnergy();
			if (!E)
			{
				return NULL;	
			}
			Pipe->Push_Directly(E);
		}

		return e.Release();
	}
	return NULL;
}
bool   CLogicDialog::FromEnergy(Energy* e){
	
	_CLOCK2(&m_DialogMutex,this);

	ePipeline* Pipe = (ePipeline*)e;

	m_SourceID = Pipe->PopInt();
	m_DialogID = Pipe->PopInt();
	
	m_SourceName = Pipe->PopString();
	m_DialogName = Pipe->PopString();
	
	m_DialogType = (DIALOG_TYPE)Pipe->PopInt();
	m_TaskOutType = (TASK_OUT_TYPE)Pipe->PopInt();

	
	m_ParentDialogID = Pipe->PopInt();
	m_LastDebugTimeStamp = Pipe->PopInt();

	m_WorkMode = (WORK_MODE)Pipe->PopInt();	
	m_TaskState =  (TASK_STATE)Pipe->PopInt();
	

	eElectron E;
	Pipe->Pop(&E);
	ePipeline& PauseEventList = *(ePipeline*)E.Get();

	while(PauseEventList.Size())
	{ 
		int64 MassID  = PauseEventList.PopInt();
		int64 EventID = PauseEventList.PopInt();
		m_PauseEventList[MassID]=EventID;
	}

	m_FocusPauseItemID = Pipe->PopInt();
    m_ControlDialogID = Pipe->PopInt();
	m_CompileError    = Pipe->PopString();

	m_SysProcCounter  = Pipe->PopInt();

	Pipe->Pop(&E);
	m_ExePipe = *(ePipeline*)E.Get();

	m_ThinkID = Pipe->PopInt();
	m_TaskID  = Pipe->PopInt();

	m_CurLogicName = Pipe->PopString();


	Pipe->Pop(&E);
	m_DialogHistory << *(ePipeline*)E.Get();
	Pipe->Pop(&E);
	m_RuntimeOutput << *(ePipeline*)E.Get();
	Pipe->Pop(&E);
	m_SearchOutput << *(ePipeline*)E.Get();


	m_bEditValid  = Pipe->PopInt()!=0;
	m_EditText    = Pipe->PopString();
	m_StatusText  = Pipe->PopString();

	Pipe->Pop(&E);
	m_LogicItemTree = *(ePipeline*)E.Get();
 
	m_ObjectFocus = Pipe->PopInt();
    m_DataTableFocus = Pipe->PopInt();
    m_LogicFocus  = Pipe->PopString();
	
	Pipe->Pop(&E);
	
	//临时逻辑
	ePipeline* List = (ePipeline*)E.Get();
    while (List->Size())
    {
		eElectron E1;
		List->Pop(&E1);
		ePipeline* Logic = (ePipeline*)E1.Get();
		CLocalLogicCell* Cell = new CLocalLogicCell();
		assert(Cell);
		if(!Cell || !Cell->FromEnergy(Logic)){
			if(Cell)delete Cell;
			return false;
		}
		
		m_LogicList.push_back(Cell);
    }
	
	//电容
	Pipe->Pop(&E);
	List = (ePipeline*)E.Get();
    while (List->Size())
    {
		eElectron E1;
		List->Pop(&E1);
		ePipeline* CellData = (ePipeline*)E1.Get();
		CElementCell* Cell = new CElementCell(_T(""),_T(""));
		assert(Cell);
        if(!Cell || !Cell->FromEnergy(CellData)){
			if(Cell)delete Cell;
			return false;
		}
		m_CapaList.push_back(Cell);
    }
	
	//电感
	Pipe->Pop(&E);
	List = (ePipeline*)E.Get();
    while (List->Size())
    {
		eElectron E1;
		List->Pop(&E1);
		ePipeline* CellData = (ePipeline*)E1.Get();
		CElementCell* Cell = new CElementCell(_T(""),_T(""));
		assert(Cell);
        if(!Cell || !Cell->FromEnergy(CellData)){
			if(Cell)delete Cell;
			return false;
		}
		m_InduList.push_back(Cell);
    }
	
	//临时物体
	Pipe->Pop(&E);
	List = (ePipeline*)E.Get();
    while (List->Size())
    {
		eElectron E1;
		List->Pop(&E1);
		ePipeline* Ob = (ePipeline*)E1.Get();
		CObjectData* ObjectData = new CObjectData(*Ob);
		m_ObjectList.push_back(ObjectData);
    }

	//外部物体实例
	Pipe->Pop(&E);
	List = (ePipeline*)E.Get();
    while (List->Size())
    {
		int64 ID = List->PopInt();
		eElectron E1;
		List->Pop(&E1);
		ePipeline* Instance = (ePipeline*)E1.Get();

		m_ObjectInstanceList[ID] = *Instance;
    }

	//数据实例
	Pipe->Pop(&E);
	List = (ePipeline*)E.Get();
    while (List->Size())
    {
		int64 ID = List->PopInt();
		eElectron E1;
		List->Pop(&E1);
		ePipeline* Instance = (ePipeline*)E1.Get();
		
		m_TableInstanceList[ID] = *Instance;
    }

	//逻辑实例
	Pipe->Pop(&E);
	List = (ePipeline*)E.Get();
    while (List->Size())
    {
		int64 ID = List->PopInt();
		eElectron E1;
		List->Pop(&E1);
		ePipeline* Instance = (ePipeline*)E1.Get();
		
		CElement* Elt = new CElement(0,_T(""));
		assert(Elt);
		if (!Elt || !Elt->FromEnergy(Instance))
		{
			delete Elt;
			return false;
		}
		m_LogicInstanceList[ID] = Elt;
    }

	//物体命名列表
	Pipe->Pop(&E);
	List = (ePipeline*)E.Get();
    if (!m_NamedObjectList.FromEnergy(List))
    {
		return false;
    }

	//记忆命名列表
	Pipe->Pop(&E);
	List = (ePipeline*)E.Get();
    if (!m_NamedTableList.FromEnergy(List))
    {
		return false;
    }

	if (m_TaskID!=0)
	{
		Pipe->Pop(&E);
		CLogicTask* Task = m_Brain->GetBrainData()->CreateLogicTask(m_DialogID,m_TaskID);
		if(!Task){
			return false;
		}
		Task->FromEnergy(this,E.Get());
	}

	if (m_ThinkID !=0)
	{
		Pipe->Pop(&E);
		CLogicThread* Think = m_Brain->GetBrainData()->CreateLogicThread(m_DialogID,m_ThinkID);
		if(!Think){
			return false;
		}
		Think->FromEnergy(E.Get());
	}
	return true;
}


void CLogicDialog::NotifyPause(ePipeline& ExePipe,ePipeline& Address){  
	
	int64 PauseID = *(int64*)Address.GetLastData();
	int64 EventID = AbstractSpace::CreateTimeStamp();

	m_PauseEventList[PauseID] = EventID;
		

	//输出当前数据管道里的数据
	CPipeView PipeView(&ExePipe);
	tstring s = PipeView.GetString();				
	RuntimeOutput(0,s);

	//最后更改执行管道返回值
	ExePipe.SetID(RETURN_PAUSE);

	tstring DialogName = Format1024(_T("Pause(%I64ld)"),PauseID);
	tstring FirstDialog = _T("Paused, you need input 'run' or 'step' or 'stop' ");
	
	CRequestBrain rq(REQUEST_DIALOG);
	rq.PushInt(EventID);
    rq.PushInt(TASK_OUT_DEFAULT);
    rq.PushInt(TIME_SEC);
	rq.PushInt(FALSE);
	rq.PushInt(TRUE);
	rq.PushString(DialogName);
    rq.PushString(FirstDialog);
	rq.PushPipe(ExePipe);
    rq.Push_Directly(Address.Clone()); //Address里的数据不能改变
	   
	bool ret = rq.Request(this);

	/*
	if (ret)
	{
		CTaskDialog* PauseDialog = m_Brain->GetBrainData()->GetDialog(EventID);
		assert(PauseDialog);
		PauseDialog->SetTaskState(TASK_PAUSE);
		PauseDialog->NotifyTaskState();
	}
	*/
	SetFocusPauseItemID(PauseID);

};

void CLogicDialog::ClosePauseDialog(int64 PauseEventID)
{
	map<int64,int64>::iterator it = m_PauseEventList.begin();
	while (it != m_PauseEventList.end())
	{
		if (it->second == PauseEventID)
		{		
			CRequestBrain rq(REQUEST_END);
			rq.PushInt(it->second);
			rq.Request(this);
			m_PauseEventList.erase(it);
			return;
		}
		
		it++;
	}
}

int64 CLogicDialog::GetFocusPauseItemID(){
	map<int64,int64>::iterator it = m_PauseEventList.find(m_FocusPauseItemID);
	if(it!= m_PauseEventList.end())
	{
		return m_FocusPauseItemID;
	}
	
	m_FocusPauseItemID = 0;
	if (m_PauseEventList.size())
	{
		m_FocusPauseItemID = m_PauseEventList.begin()->first;
	}
	return m_FocusPauseItemID;	
};

void CLogicDialog::GetPauseIDList(ePipeline& List){
	map<int64,int64>::iterator it = m_PauseEventList.begin();
	while(it != m_PauseEventList.end()){
		List.PushInt(it->first);
		it++;
	}
};

void CLogicDialog::StopPause(int64 PauseID,int64 CmdID){
	if (PauseID ==0)
	{
		ePipeline PauseEventList;
		map<int64,int64>::iterator it = m_PauseEventList.begin();
		
		//先一次性取得所有暂停事件ID，否则会引发m_PauseEventList线程同步问题
		while(it != m_PauseEventList.end() )
		{
			PauseEventList.PushInt(it->second);
			it++;
		}
		
        while(PauseEventList.Size())
		{
			int64 EventID = PauseEventList.PopInt();
			CBrainEvent EventInfo;
			bool ret = m_Brain->GetBrainData()->PopBrainEvent(EventID,EventInfo);
			if (!ret)
			{
				return;
			}
			
			CMsg EltMsg(EventInfo.m_ClientAddress,MSG_ELT_TASK_CTRL,EventID);
			ePipeline& Letter = EltMsg.GetLetter();
			Letter.PushInt(CmdID);
			Letter.PushPipe(EventInfo.m_ClientExePipe);
			
			//给Element发信息
			m_Brain->PushNerveMsg(EltMsg,false,false);
		}
		
	}else{
		map<int64,int64>::iterator it = m_PauseEventList.find(PauseID);
		if (it == m_PauseEventList.end())
		{
			return;
		}

		int64 EventID = it->second;

		CBrainEvent EventInfo;
		bool ret = m_Brain->GetBrainData()->PopBrainEvent(EventID,EventInfo);
		if (!ret)
		{
			return;
		}
		
		
		CMsg EltMsg(EventInfo.m_ClientAddress,MSG_ELT_TASK_CTRL,EventID);
		ePipeline& Letter = EltMsg.GetLetter();
		Letter.PushInt(CmdID);
		Letter.PushPipe(EventInfo.m_ClientExePipe);
		
		//给Element发信息
		m_Brain->PushNerveMsg(EltMsg,false,false);
	}
}

bool CLogicDialog::StartChildDialog(int64 EventID,tstring DialogName,tstring FirstDialog,TASK_OUT_TYPE OutType,ePipeline& ExePipe,ePipeline& Address,int64 EventInterval/*=TIME_SEC*/ ,bool bFocus /*=false*/,bool bEditValid /*=true*/){

	assert(EventID !=0);

    CRequestBrain rq(REQUEST_DIALOG);
	rq.PushInt(EventID);
    rq.PushInt(OutType);
	rq.PushInt(EventInterval);
	rq.PushInt(bFocus?1:0);
	rq.PushInt(bEditValid?1:0);
	rq.PushString(DialogName);
	rq.PushString(FirstDialog);
	rq.PushPipe(ExePipe);
	rq.Push_Directly(Address.Clone()); //Address里的数据不能改变

	bool ret = rq.Request(this);

    int64 ID = ExePipe.GetID();
	if (ID == RETURN_BREAK)
	{
		ExePipe.SetID(RETURN_DEBUG_WAIT);
	}else {
		ExePipe.SetID(RETURN_WAIT);
	}

	return ret;
};

void CLogicDialog::CloseChildDialog(int64 EventID,ePipeline& OldExePipe,ePipeline& ExePipe){

	assert(EventID !=0);
	
    CRequestBrain rq(REQUEST_END);
	rq.PushInt(EventID);
    rq.Request(this);

	if(ExePipe.GetID() == RETURN_ERROR){
		return;	
	}

	if (ExePipe.GetID() == RETURN_BREAK || OldExePipe.GetID()== RETURN_BREAK)
	{
		ExePipe.SetID(RETURN_BREAK);
	}else{
		ExePipe.SetID(RETURN_NORMAL);
	}

	if(OldExePipe.IsAlive()){
		ExePipe.Reuse();
	}else{
		ExePipe.Break();
	}

    SetTaskState(TASK_RUN);
	NotifyTaskState();
}



ePipeline&  CLogicDialog::SaveReceiveItem(tstring Info,int32 State){
	
	int64 ID= AbstractSpace::CreateTimeStamp();
	ePipeline* Item = new ePipeline(m_SourceName.c_str(),ID);
 
    Item->PushInt(State);
	Item->PushString(Info);
	
	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(DIALOG_INFO);
	nf.Push_Directly(Item);
	nf.Notify(this);

	return *(ePipeline*)m_DialogHistory.GetLastData();
}

ePipeline&  CLogicDialog::SaveSendItem(tstring Info, int32 State)
{

	int64 ID= AbstractSpace::CreateTimeStamp();
	ePipeline* Item = new ePipeline(m_DialogName.c_str(),ID);

    Item->PushInt(State);
	Item->PushString(Info);
	
	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(DIALOG_INFO);
	nf.Push_Directly(Item);
	nf.Notify(this);

	return *(ePipeline*)m_DialogHistory.GetLastData();
}


void CLogicDialog::ClearTaskMsgList()
{
	m_TaskMsgList.ClearData();
};

void CLogicDialog::FeedbackToBrain(){

	int64 DialogID = m_DialogID;
    
	if(DialogID != DEFAULT_DIALOG){
		CMsg rMsg(MSG_TASK_FEEDBACK,DialogID,DialogID);		
		ePipeline& rLetter = rMsg.GetLetter();
		rLetter.PushPipe(m_ExePipe);	
		
		m_Brain->PushCentralNerveMsg(rMsg,false,false);	
	}else{
		
		//系统对话的事件ID缺省=0，为了避免伪造事件，所以系统对话不使用事件机制来回复，而是直接回复
		int64 ID = m_ExePipe.GetID();
		
		if (!m_ExePipe.IsAlive())
		{
			tstring text = _T("Stop");
			SaveSendItem(text,0); 
		}
		else{
			tstring text = _T("ok");
			SaveSendItem(text,0);    
		}	
	}
		
	ResetTask();				

	SetTaskState(TASK_STOP);
    NotifyTaskState();
}

void  CLogicDialog::Do(CMsg& Msg){

	TASK_STATE State = GetTaskState();
	if ( State == TASK_DELELTE)
	{
		return;
	}

	int64 SourceID = Msg.GetSourceID();

	/* 如果是握手信息交给缺省对话处理，SourceID!=m_SourceID很正常，
	   其它信息两者必须相等，不过信息被那个对话处理由系统决定这里没必要再检查
	if (SourceID != m_SourceID )
	{
		assert(0);
		return;
	}
	*/

	ePipeline& TaskAddress = Msg.GetReceiver();
	int64 ID = TaskAddress.PopInt();

	assert(ID == m_DialogID);

	uint32 LogFlag = m_Brain->GetLogFlag();
	if (TaskAddress.Size()==0)  //系统对话
	{
		if (LogFlag & LOG_MSG_PROC_BEGIN)
		{	
			int64 MsgID = Msg.GetMsgID();
			if (MsgID != MSG_EVENT_TICK) //这个信息太频繁，忽略之
			{
				int64 EventID = Msg.GetEventID();

				tstring CmdStr = m_Brain->MsgID2Str(MsgID);
				tstring& Tip = Msg.GetMsg().GetLabel();
				
				m_Brain->OutputLog(LOG_MSG_PROC_BEGIN,_T("-->Dialog(%I64ld) Pop Msg:%s EventID:%I64ld %s"),m_DialogID,CmdStr.c_str(),EventID,Tip.c_str());
			}
		}

	    DialogMsgProc(&m_ExePipe,Msg);
		
	}
    else{
		
		if (LogFlag & LOG_MSG_TASK_PUSH)
		{
			int64 MsgID = Msg.GetMsgID();

			if (MsgID != MSG_EVENT_TICK) //这个信息太频繁，忽略之
			{
				int64 EventID = Msg.GetEventID();
				tstring CmdStr = m_Brain->MsgID2Str(MsgID);
				m_Brain->OutputLog(LOG_MSG_TASK_PUSH,_T("Push msg to task queue:%s EventID:%I64ld"),CmdStr.c_str(),EventID);
			}
		}
		m_TaskMsgList.Push(Msg.Release());
	}
   
	AutoSysProcCounter  Counter(this);
  
	int32 n = Counter.GetProcNum();
    if (n>1) //只允许一个线程执行逻辑任务，如果有其它线程在处理会接着处理m_MsgList的信息
    {
		return;
    }
	
	while (m_TaskMsgList.DataNum())
	{
#ifdef NDEBUG
		try
		{	
#endif
			CMsg EltMsg;
			m_TaskMsgList.Pop(EltMsg);
			if (!EltMsg.IsValid())
			{
				continue;
			}

			m_CurTaskMsgID = EltMsg.GetMsgID();

			if (LogFlag & LOG_MSG_PROC_BEGIN)
			{
				int64 MsgID = m_CurTaskMsgID;

				if (MsgID != MSG_EVENT_TICK) //这个信息太频繁，忽略之
				{
					int64 EventID = EltMsg.GetEventID();

					tstring CmdStr = m_Brain->MsgID2Str(MsgID);
					tstring& Tip = EltMsg.GetMsg().GetLabel();
					m_Brain->OutputLog(LOG_MSG_PROC_BEGIN,_T("-->Task Pop Msg:%s EventID:%I64ld %s"),CmdStr.c_str(),EventID,Tip.c_str());
					
				}
			}

			CLogicTask* CurTask = GetTask();

			ePipeline& EltAddress = EltMsg.GetReceiver();
			
			ePipeline  LocalAddress;
			LocalAddress.PushInt(m_DialogID);

			//把信息根据接受者地址交给指定执行者处理。
			assert(CurTask);
			
			int64 TaskID = EltAddress.PopInt();
			if(CurTask->m_ID != TaskID){
				return ;
			}

            m_ExePipe.GetLabel() = _T("");

			CurTask->Do(this,m_ExePipe,LocalAddress,EltMsg);
			
            if (!m_ExePipe.IsAlive())
            {    
				//有两种可能：
				TASK_STATE NewState = GetTaskState();
				if (NewState == TASK_STOP) //1）被显式要求停止执行
				{
					SetWorkMode(WORK_TASK);
					ResetThink();
					FeedbackToBrain();
					return;
				}else{  //2)Element处理系统信息后直接返回，不做其他处理
					m_ExePipe.Reuse();
					continue;
				} 	 	
			}


			//如果管道有效，检查执行返回状态，更新任务状态
			int64 ID = m_ExePipe.GetID();
			
			switch(ID){
			case RETURN_NORMAL:
				break;
			case RETURN_ERROR:
				{				
					tstring& Error = m_ExePipe.GetLabel();
					RuntimeOutput(0,Error);	
					Error = _T("");
				} 
				break;

			case RETURN_GOTO_LABEL: 
			case RETURN_DEBUG_GOTO_LABEL:
				{
					tstring Error = Format1024(_T("Error: Goto Label [%s] not find"),m_ExePipe.GetLabel().c_str());
					int64 MassID = *(int64*)LocalAddress.GetLastData();
					RuntimeOutput(MassID,Error);		
				}
				break;

			case RETURN_PAUSE:
				SetWorkMode(WORK_DEBUG);
				SetTaskState(TASK_PAUSE);
				NotifyTaskState();
				break;
			case RETURN_BREAK:  //表示调试执行已经到最后一步，执行完毕
			case RETURN_DEBUG_PIPE_BREAK:
				SetTaskState(TASK_RUN);
				SetWorkMode(WORK_TASK);
				m_ExePipe.SetID(RETURN_NORMAL);
				break;
			case RETURN_WAIT:
			case RETURN_DEBUG_WAIT:		
				if (IsPaused())
				{
					SetWorkMode(WORK_DEBUG);
					SetTaskState(TASK_PAUSE);
				}else{
					SetWorkMode(WORK_TASK);
					SetTaskState(TASK_WAIT);
				}
				NotifyTaskState();
				break;
			case RETURN_PIPE_BREAK:
				SetTaskState(TASK_RUN);
				break;
			default:
				assert(ID!=RETURN_BREAK);
				assert(0);
			}
			
#ifdef NDEBUG
		}
		catch (...)
		{
			tstring s = _T("some exceptions occurred, task to terminate");	
			RuntimeOutput(0,s);
		    m_ExePipe.Break();

			SetTaskState(TASK_RUN); //目前有任何例外都视为当前任务结束
		}
#endif
		//处理完后检查任务状态
		TASK_STATE NewState = GetTaskState();
		

		if (NewState == TASK_RUN)  //如果是RUN状态下运行到这步，则表示任务运行完毕
		{
            FeedbackToBrain();
		}		
	}
	
}


void CLogicDialog::SetBreakPoint(ePipeline& Path,BOOL bEnable){
	CLogicTask* CurTask  = m_Brain->GetBrainData()->GetLogicTask(m_TaskID);
    assert(CurTask);

	ePipeline Path1; //用于回信
	Path1 = Path;
	bool ret = CurTask->SetBreakPoint(Path,bEnable);
	assert(ret);
	if (ret) //反馈
	{
		CNotifyDialogState nf(NOTIFY_DEBUG_VIEW);
		nf.PushInt(DEBUG_SET_BREAK);
		nf.PushInt(bEnable);
		nf.PushPipe(Path1);
		nf.Notify(this);
	}
}


TASK_STATE   CLogicDialog::SetTaskState(TASK_STATE State){
	CLock((CABMutex*)&m_DialogMutex,this);
	TASK_STATE Old = m_TaskState;
	m_TaskState = State;
	return Old;
}
TASK_STATE  CLogicDialog::GetTaskState(){
	CLock((CABMutex*)&m_DialogMutex,this);
	TASK_STATE State = m_TaskState ;
    return State;
}

TASK_OUT_TYPE CLogicDialog::GetTaskType(){
	return m_TaskOutType;
}
void CLogicDialog::NotifyTaskState()
{
	TASK_STATE State = m_TaskState;
	CNotifyDialogState nf(NOTIFY_DIALOG_STATE);
	
	nf.PushInt(m_WorkMode);
	nf.PushInt(State);
	nf.Notify(this);
}
/*
void CLogicDialog::NotifyStopTask(){
	if(GetTaskState() != TASK_STOP){

		ePipeline Address;
		Address.PushInt(m_DialogID);
		Address.PushInt(m_TaskID);

		CMsg EltMsg(Address,MSG_ELT_TASK_CTRL,0);
		ePipeline& Letter = EltMsg.GetLetter();
		Letter.PushInt(CMD_STOP);

		//给Element发信息
		m_Brain->PushNerveMsg(EltMsg,false,false);
	}
}
*/
void CLogicDialog::NotifySuspendTask(){
	TASK_STATE State = GetTaskState();

	if(State == TASK_RUN){
		m_ExePipe.SetID(RETURN_BREAK);
		m_ExePipe.Break();
	}
}
/*
bool CLogicDialog::ResumeTask(){

	map<int64,int64>::iterator it = m_PauseEventList.begin();
	if(m_PauseEventList.end() == it){
		return true;
	}	

	/*
	while(it != m_PauseEventList.end())
	{
		int64 PauseID = it->first;
		int64 EventID = it->second;

		CTaskDialog* NewDialog = m_Brain->GetBrainData()->GetDialog(EventID);
		if (!NewDialog)
		{
			NewDialog = m_Brain->GetBrainData()->CreateNewDialog(this,m_SourceID,EventID,m_DialogID,_T("Dialog"),m_DialogName,DIALOG_SYSTEM_CHILD,0,OutType);	
			if(!NewDialog)return false;

			CNotifyState nf(NOTIFY_DIALOG_LIST);
			nf.PushInt(DL_ADD_DIALOG);
			nf.PushInt(FALSE);
			nf.Notify(NewDialog);		

		}else{
			//先做个清理
			NewDialog->ResetThink();
			NewDialog->SetTaskState(TASK_STOP);
			NewDialog->NotifyTaskState();

			//然后设置为等待
			NewDialog->SetTaskState(TASK_WAIT);
			NewDialog->NotifyTaskState();
		}
	}
	*/
/*
	SetWorkMode(WORK_DEBUG);
	SetTaskState(TASK_PAUSE);
	return true;
}
*/
int32 CLogicDialog::GetSysProcNum(){
	CLock((CABMutex*)&m_DialogMutex,this);
	int32 n = m_SysProcCounter;
	return n;
}
void  CLogicDialog::ResetThink(){
	m_ThinkID = 0;
	m_bEditValid = TRUE;
	m_EditText = _T("");
}
void  CLogicDialog::ResetTask(){
	m_TaskID = 0;
	m_ExePipe.Reuse();
	m_ExePipe.Clear();

	map<int64,int64>::iterator it = m_PauseEventList.begin();
	while (it != m_PauseEventList.end())
	{
		int64 PauseID = it->first;
		int64 EventID = it->second;

		CRequestBrain rq(REQUEST_END);
		rq.PushInt(EventID);
		rq.Request(this);
		it++;
	}
	m_PauseEventList.clear();
	m_FocusPauseItemID = 0;
   
	if (m_ControlDialogID)
	{
		m_Brain->GetBrainData()->DeleteDialog(m_SourceID,m_ControlDialogID);
    }
	m_ControlDialogID = AbstractSpace::CreateTimeStamp();
};



void CLogicDialog::SetWorkMode(WORK_MODE Mode){
	_CLOCK2(&m_DialogMutex,this);
    m_WorkMode = Mode;
}

WORK_MODE CLogicDialog::GetWorkMode(){
	_CLOCK2(&m_DialogMutex,this);
	WORK_MODE Mode = m_WorkMode;
	return Mode;
}

CLogicThread* CLogicDialog::GetThink(){
	if(m_ThinkID ==0){
		CLogicThread* Think = m_Brain->GetBrainData()->CreateLogicThread(m_DialogID);
		m_ThinkID = Think->m_ID;
		return Think;
	}

	CLogicThread* Thread = m_Brain->GetBrainData()->GetLogicThread(m_ThinkID);
	return Thread;
};

CLogicTask* CLogicDialog::GetTask(){
	if(m_TaskID==0){
		CLogicTask* Task = m_Brain->GetBrainData()->CreateLogicTask(m_DialogID);
		m_TaskID = Task->m_ID;
		return Task;
	};
	CLogicTask* Task = m_Brain->GetBrainData()->GetLogicTask(m_TaskID);
	return Task;
}

CLogicDialog& CLogicDialog::operator << (CLogicDialog& Dialog){

	if(&Dialog == this){
		return *this;
	}

	m_TaskID = Dialog.m_TaskID;
	m_ThinkID   = Dialog.m_ThinkID;

	m_Brain->GetBrainData()->SetLogicThreadUser(m_ThinkID,m_DialogID);
	m_Brain->GetBrainData()->SetLogicTaskUser(m_TaskID,m_DialogID);

	m_EditText = Dialog.m_EditText;
	
	Dialog.m_EditText = _T("");
	Dialog.m_TaskID = 0;
	Dialog.m_ThinkID = 0;

	return *this;
}

void CLogicDialog::RuntimeOutput(tstring s){
		
	int64 ID = AbstractSpace::CreateTimeStamp();
	ePipeline* Item = new ePipeline(ID);
	if (!Item)
	{
		return;
	}

    Item->PushInt(0);
	Item->PushString(s);
		
	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(RUNTIME_INFO);
	nf.Push_Directly(Item);
	nf.Notify(this);
}

void CLogicDialog::RuntimeOutput(int64 MassID,tstring s){

	int64 ID = AbstractSpace::CreateTimeStamp();
	ePipeline* Item = new ePipeline(ID);
	if (!Item)
	{
		return;
	}
	tstring str = Format1024(_T("(%I64ld):\ns%"),MassID,s.c_str());
    Item->PushInt(MassID);
	Item->PushString(s);
		
	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(RUNTIME_INFO);
	nf.Push_Directly(Item);
	nf.Notify(this);
}


void CLogicDialog::RuntimeOutput(int64 MassID,TCHAR* Format, ...){
	TCHAR Buffer [512] ;
	va_list ArgList ;		
	va_start (ArgList, Format) ;
	_vsntprintf(Buffer, 512, Format, ArgList) ;		
	va_end (ArgList) ;
	tstring s = Buffer;
	RuntimeOutput(MassID,s);
}

//编译任务
//////////////////////////////////////////////////////////////////////////

bool CLogicDialog::CompileTask(){
			
	CLogicThread* Think = GetThink();
	
	if (Think == NULL)
	{
		return false;
	}
	
	CText& text = Think->m_Text;

	if (text.m_SentenceList.size()==0)
	{
		return false;
	}

	CSentence* Sentence = text.m_SentenceList.front();
	tstring SentenceText  = text.GetSentenceText(0);		

	Sentence->m_AnalyseResult.SetLabel(SentenceText.c_str());


	m_TaskID=0; //确保使用全新ID的Task;
 
	m_LogicInstanceList.clear();

	CLogicTask* Task = GetTask();
	
	if (Task == NULL)
	{
		return false;
	}

				
	Task->ResetTime();
				
	m_LogicItemTree.Clear();
	m_LogicItemTree.SetID(Task->m_BeginTime); //通过标记新时间来确定之前的调试树是否有效
					
	if(m_DialogType==DIALOG_OTHER_CHILD){  //是输入给对方，让对方执行
		assert(Task->m_ActomList.size()==0);
		CUseRobot*   UseRobot = new CUseRobot(0,this,m_SourceID,Sentence->m_AnalyseResult);
		if (!UseRobot)
		{
			return NULL;
		}
		
		Task->m_LogicData =Sentence->m_AnalyseResult;
		Task->m_ActomList.push_back(UseRobot);
        Task->m_Name = _T("Use robot");
		
		SaveReceiveItem(Task->m_LogicText,0);

		return true;
	}

	
	//如果当前状态是学习逻辑，那么此任务会被登记为逻辑,而不是执行
	if (GetWorkMode() == WORK_THINK)
	{						
		Task->m_Name = m_CurLogicName;
		Task->m_LogicText = SentenceText;
		Task->m_State = Sentence->m_State;
		Task->m_LogicData = Sentence->m_AnalyseResult;				
		
		m_CurLogicName = _T("");
		
		delete Sentence;		
		Think->m_Text.m_SentenceList.pop_front();
		
		SaveReceiveItem(Task->m_LogicText,0);
		return true;
	}
	
	Task->m_Name = m_CurLogicName.size() ?m_CurLogicName:_T("Task");
	Task->m_LogicText = SentenceText;
	Task->m_State = Sentence->m_State;
	
	if (!Task->Compile(this,&Sentence->m_AnalyseResult))
	{
		tstring& error = Task->m_CompileError;

		tstring EditText = Think->GetUserInput();
		
		CNotifyDialogState nf(NOTIFY_DIALOG_ERROR);
		nf.PushInt(COMPILE_ERROR);
		nf.PushString(error);
		nf.PushString(EditText);
		nf.Notify(this);
		
		return false;	
	}else{
		
		
		delete Sentence;		
		text.m_SentenceList.pop_front();
				
		SaveReceiveItem(Task->m_LogicText,0);
	}
	return true;
}


void CLogicDialog::SentenceCompileProgress(int32 ClauseNum, int32 Completed){

	int32 per = Completed/ClauseNum;
	CNotifyDialogState nf(NOTIFY_PROGRESS_OUTPUT);
	nf.PushInt(COMPILE_PROGRESS);
	nf.PushInt(per);
	nf.Notify(this);
}
