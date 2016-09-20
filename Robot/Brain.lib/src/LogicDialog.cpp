// TaskDialog.cpp: implementation of the CTaskDialog class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning (disable:4786)

#include "Brain.h"
#include "LogicDialog.h"

#include "Element.h"

#include "InstinctDefine.h"
#include "NotifyMsgDef.h"
#include "MsgList.h"
#include "GUIMsgDefine.h"




bool CNameList::HasName(CLogicDialog* Dialog,tstring& Name){
	map<tstring,CNameUser>::iterator it = m_NameList.find(Name);
	if (it == m_NameList.end()){
		return false;
	}
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
bool CNameList::HasMemoryInstance(CLogicDialog* Dialog,tstring& Name){
	map<tstring,CNameUser>::iterator it = m_NameList.find(Name);
	if (it == m_NameList.end()){
		return false;
	}

	CNameUser& NameUser = it->second;
	if(Dialog->FindTempMemory(NameUser.m_InstanceID)){
		return true;
	}

	return true;
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
	m_NameList.erase(it);
};

void CNameList::UnregisterNameByTask(int64 InstanceID){
	map<tstring, CNameUser>::iterator it = m_NameList.begin();
	while(it != m_NameList.end()){
		CNameUser& User = it->second;
		if(User.m_InstanceID == InstanceID){
			User.m_TaskID = 0;
			m_NameList.erase(it);
			return;
		}
		it++;
	};
};


int64 CNameList::GetInstanceID(tstring& Name){
	map<tstring, CNameUser>::iterator it = m_NameList.find(Name);
	if(it != m_NameList.end()){
		CNameUser& User = it->second;
		return User.m_InstanceID;
	};
	return 0;
};
CNameUser& CNameList::GetNameUser(tstring& Name){
	static CNameUser Empty;
	map<tstring, CNameUser>::iterator it = m_NameList.find(Name);
	if(it != m_NameList.end()){
		CNameUser& User = it->second;
		return User;
	};
	return Empty;
}
tstring CNameList::GetInstanceName(int64 InstanceID){
	map<tstring, CNameUser>::iterator it = m_NameList.begin();
	while(it != m_NameList.end()){
		CNameUser& User = it->second;
		if(User.m_InstanceID == InstanceID){
			return it->first;
		}
		it++;
	};
	return _T("");
}

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

CLogicDialog::CLogicDialog(int64 SourceID,int64 DialogID,int64 ParentDialogID,tstring SourceName,tstring DialogName,
						 DIALOG_TYPE Type,TASK_OUT_TYPE TaskType)
:m_ElementMsgList(&m_DialogMutex),m_TaskDialogCount(0)
{
	m_DialogHistory.SetLabel(_T("Histroy"));  //设置名字便于调试
	m_RuntimeOutput.SetLabel(_T("RuntimeOutput"));
	m_SearchOutput.SetLabel(_T("SearchOutput"));

	Reset(SourceID,DialogID,ParentDialogID,SourceName,DialogName,Type,TaskType);
}

CLogicDialog::~CLogicDialog()
{

	ClearLogicSence();
	ClearObject();
	m_FindResultList.clear();
	m_FindSeedList.clear();

	m_TaskID = 0;

}

void CLogicDialog::Reset(int64 SourceID,int64 DialogID,int64 ParentDialogID,tstring SourceName,tstring DialogName,
		   DIALOG_TYPE Type,TASK_OUT_TYPE TaskType)

{
 	    m_SourceName = SourceName;
		m_DialogName = DialogName;
		m_SourceID = SourceID;
		m_DialogID = DialogID;
		m_ParentDialogID = ParentDialogID;
		m_LastDebugTimeStamp = 0;

		m_DialogType = Type;
		m_TaskOutType = TaskType;
		m_SysProcCounter = 0;
		m_ThinkState   = THINK_IDLE;
		m_TaskState = TASK_STOP;
		m_WorkMode = WORK_TASK;

		m_ItemNumPerPage   = 10;
		m_FindInterval = 5000; //*TIME_SEC;

		m_bEditValid = true;
		m_EditText = _T("");
		
		m_DialogHistory.Clear();
		m_RuntimeOutput.Clear();
		m_SearchOutput.Clear();
		m_LogicItemTree.Clear();

		m_ChildEventList.clear();
		m_MemoryAddress.Clear();
		m_LogicAddress.Clear();

		m_ControlDialogID = 0;
			
		while (m_ElementMsgList.DataNum())
		{
			CMsg m;
			m_ElementMsgList.Pop(m);
		};

		m_ObjectFocus =0;
		m_MemoryFocus =0;
		m_RequestFocus =0;

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
		Pipe->PushInt(m_ThinkState);

		Pipe->PushInt(m_TaskDialogCount);

		ePipeline PauseEventList;
		map<int64,int64>::iterator it = m_ChildEventList.begin();
		while(it != m_ChildEventList.end())
		{
			PauseEventList.PushInt(it->first);
			PauseEventList.PushInt(it->second);
			it++;
		}
		Pipe->PushPipe(PauseEventList);

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
		Pipe->PushInt(m_MemoryFocus);
		Pipe->PushInt(m_RequestFocus);

		Pipe->PushPipe(m_MemoryAddress);
		Pipe->PushPipe(m_LogicAddress);

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
			ePipeline* Item = ob->Clone();
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
		List.Clear();

		map<int64, ePipeline>::iterator ite = m_MemoryInstanceList.begin();
		while (ite != m_MemoryInstanceList.end())
		{
			int64 ID = ite->first;
			ePipeline& Instance = ite->second;
			
			List.PushInt(ID);
			List.PushPipe(Instance);
			ite++;
		}
		Pipe->PushList(List);
		List.Clear();

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
		List.Clear();

		map<int64, ePipeline>::iterator itg = m_RequestInstanceList.begin();
		while (itg != m_RequestInstanceList.end())
		{
			int64 ID = itg->first;
			ePipeline& Instance = itg->second;

			List.PushInt(ID);
			List.PushPipe(Instance);
			itg++;
		}
		Pipe->PushList(List);

		Energy* e1 = m_NamedObjectList.ToEnergy();
		if(!e1){
			return NULL;
		}
		Pipe->Push_Directly(e1);

		Energy* e2 = m_NamedMemoryList.ToEnergy();
		if(!e2){
			return NULL;
		}
		Pipe->Push_Directly(e2);

		if (m_TaskID!=0)
		{
			CLogicTask* Task  = GetBrain()->GetBrainData()->GetLogicTask(m_TaskID,false); 
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
			CLogicThread* Think = GetBrain()->GetBrainData()->GetLogicThread(m_ThinkID,false); 
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
	m_ThinkState = (THINK_STATE)Pipe->PopInt();
	m_TaskDialogCount = Pipe->PopInt();

	eElectron E;
	Pipe->Pop(&E);
	ePipeline& PauseEventList = *(ePipeline*)E.Get();

	while(PauseEventList.Size())
	{ 
		int64 MassID  = PauseEventList.PopInt();
		int64 EventID = PauseEventList.PopInt();
		m_ChildEventList[MassID]=EventID;
	}

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
    m_MemoryFocus = Pipe->PopInt();
	m_RequestFocus= Pipe->PopInt();
	
	Pipe->Pop(&E);
	m_MemoryAddress = *(ePipeline*)E.Get();

	Pipe->Pop(&E);
	m_LogicAddress = *(ePipeline*)E.Get();

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
		
		m_MemoryInstanceList[ID] = *Instance;
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

	//与其它机器人对话实例
	Pipe->Pop(&E);
	List = (ePipeline*)E.Get();
	while (List->Size())
	{
		int64 ID = List->PopInt();
		eElectron E1;
		List->Pop(&E1);
		ePipeline* Instance = (ePipeline*)E1.Get();

		m_RequestInstanceList[ID] = *Instance;
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
    if (!m_NamedMemoryList.FromEnergy(List))
    {
		return false;
    }

	if (m_TaskID!=0)
	{
		Pipe->Pop(&E);
		CLogicTask* Task = GetBrain()->GetBrainData()->CreateLogicTask(m_DialogID,m_TaskID);
		if(!Task){
			return false;
		}
		Task->FromEnergy(this,E.Get());
	}

	if (m_ThinkID !=0)
	{
		Pipe->Pop(&E);
		CLogicThread* Think = GetBrain()->GetBrainData()->CreateLogicThread(m_DialogID,m_ThinkID);
		if(!Think){
			return false;
		}
		Think->FromEnergy(E.Get());
	}
	return true;
}


void CLogicDialog::NotifyPause(ePipeline& ExePipe,ePipeline& Address){  
	if(Address.Size()==0){
		return;
	}
	int64 PauseID = *(int64*)Address.GetLastData();
	int64 EventID = AbstractSpace::CreateTimeStamp();

	m_LogicAddress = Address;

	//输出当前数据管道里的数据
	CPipeView PipeView(&ExePipe);
	tstring s = PipeView.GetString();				
	RuntimeOutput(0,s);

	//最后更改执行管道返回值
	ExePipe.SetID(RETURN_PAUSE);

	tstring DialogName = Format1024(_T("Pause%I64ld"),PauseID);
	CLogicDialog* ChilidDialog = StartEventDialog(EventID,DialogName,TASK_OUT_DEFAULT,ExePipe,Address,TIME_SEC,false,true,false,INSTINCT_PAUSE_TASK);
	if(!ChilidDialog){
		return;
	}
	tstring DialogText = _T("Paused, you need input 'run' or 'step' or 'stop' ");
	ChilidDialog->SaveDialogItem(DialogText,_T("System"),0);
	_CLOCK2(&m_DialogMutex,this);
	m_ChildEventList[PauseID] = EventID;
};

void CLogicDialog::ClosePauseDialog(int64 PauseEventID)
{
	_CLOCK2(&m_DialogMutex,this);
	map<int64,int64>::iterator it = m_ChildEventList.begin();
	while (it != m_ChildEventList.end())
	{
		if (it->second == PauseEventID)
		{		
			CLogicDialog* Dlg = GetBrain()->GetBrainData()->GetDialog(m_SourceID,PauseEventID);
			if (Dlg)
			{
				CBrainEvent EventInfo;
				GetBrain()->GetBrainData()->GetEvent(PauseEventID,EventInfo,true);

				CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
				nf.PushInt(DL_DEL_DIALOG);
				nf.Notify(Dlg);

				int64 PauseItemID = it->first;
				assert(PauseItemID != PauseEventID);

				CNotifyDialogState nf1(NOTIFY_DEBUG_VIEW);
				nf1.PushInt(DEBUG_DISABLE_PAUSE);
				nf1.PushInt(PauseItemID);
				nf1.Notify(this);


				GetBrain()->GetBrainData()->DeleteDialog(m_SourceID,PauseEventID);	
			}

			m_ChildEventList.erase(it);
			m_ChildEventList.erase(PauseEventID); //因为pause事件会被登记两次
			return;
		}
		
		it++;
	}
}

void CLogicDialog::StopPause(int64 PauseID,int64 CmdID){
	_CLOCK2(&m_DialogMutex,this);
	if (PauseID ==0)
	{
		ePipeline PauseEventList;
		map<int64,int64>::iterator it = m_ChildEventList.begin();

		//先一次性取得所有暂停事件列表
		while(it != m_ChildEventList.end() )
		{
			if(it->first != it->second){  
				PauseEventList.PushInt(it->second);	
			}else{
				break;  //真正的pause事件总是排在前面，如果没找到，后面肯定就不是了
			}
			it++;		
		}

		while(PauseEventList.Size())
		{
			int64 EventID = PauseEventList.PopInt();
			CBrainEvent EventInfo;
			bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,true);
			if (!ret)
			{
				return;
			}

			CMsg EltMsg(m_SourceID,EventInfo.m_ClientAddress,MSG_ELT_TASK_CTRL,DEFAULT_DIALOG,EventID);
			ePipeline& Letter = EltMsg.GetLetter(false);
			Letter.PushInt(CmdID);
			Letter.PushPipe(EventInfo.m_ClientExePipe);

			//给Element发信息
			GetBrain()->PushNerveMsg(EltMsg,false,false);

		}
		SetWorkMode(WORK_TASK);
	}else{
		map<int64,int64>::iterator it = m_ChildEventList.find(PauseID);
		if (it == m_ChildEventList.end() )
		{
			RuntimeOutput(0,_T("not find pause address:%I64ld"),PauseID);
			return;
		}
		if(it->first == it->second){
			RuntimeOutput(0,_T("the pause address:%I64ld is invalid"),PauseID);
			return;
		}

		int64 EventID = it->second;

		CBrainEvent EventInfo;
		bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,true);
		if (!ret)
		{
			RuntimeOutput(0,_T("the pause event:%I64ld is non-existent"),PauseID);
			return;
		}

		CMsg EltMsg(m_SourceID,EventInfo.m_ClientAddress,MSG_ELT_TASK_CTRL,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = EltMsg.GetLetter(false);
		Letter.PushInt(CmdID);
		Letter.PushPipe(EventInfo.m_ClientExePipe);

		//给Element发信息
		GetBrain()->PushNerveMsg(EltMsg,false,false);

	}
}

void CLogicDialog::GetPauseMassIDList(ePipeline& List){
	_CLOCK2(&m_DialogMutex,this);
	map<int64,int64>::iterator it = m_ChildEventList.begin();
	while(it != m_ChildEventList.end()){
		if(it->first != it->second){  //相等是普通事件对话记录
			List.PushInt(it->first);
		}else{
			break;  //找到相等的意味着后面的都不是，因为MassID通常不是很大，会排在最前面
		}
		it++;
	}
};


void CLogicDialog::GetPauseEventIDList(ePipeline& List){
	_CLOCK2(&m_DialogMutex,this);
	map<int64,int64>::iterator it = m_ChildEventList.begin();
	while(it != m_ChildEventList.end()){
		if(it->first != it->second){  //相等是普通事件对话记录
			List.PushInt(it->second);
		}else{
			break;
		}
		it++;
	}
};

bool CLogicDialog::IsPaused(){
	_CLOCK2(&m_DialogMutex,this);
	map<int64,int64>::iterator  it = m_ChildEventList.begin();
	if(it != m_ChildEventList.end()){
		if(it->first != it->second){
			return true;
		}
	};
	return false;
}

void CLogicDialog::UpdateDebugTree(){

	m_LogicItemTree.Clear();

	assert(m_TaskID!=0);
	CLogicTask* Task = GetTask();

	Task->GetDebugItem(m_LogicItemTree);

	ePipeline PauseIDList;
	GetPauseMassIDList(PauseIDList);

	CNotifyDialogState nf(NOTIFY_DEBUG_VIEW);
	nf.PushInt(DEBUG_RESET);
	nf.Push_Directly(m_LogicItemTree.Clone());
	nf.PushPipe(PauseIDList);
	nf.Notify(this);

}

CLogicDialog* CLogicDialog::StartEventDialog(int64 EventID,tstring DialogName,TASK_OUT_TYPE OutType,ePipeline& ClientExePipe,ePipeline& ClientAddress,int64 EventInterval,bool bFocus,bool bEditValid,bool bOnce,int64 InstinctID/*=0*/){

	assert(EventID !=0);

	CLogicDialog* NewDialog = NULL;

	assert((NewDialog = GetBrain()->GetBrainData()->GetDialog(m_SourceID,EventID))==NULL);
		
	CLogicDialog* ChildDialog = GetBrain()->GetBrainData()->CreateNewDialog(m_SourceID,EventID,m_DialogID,m_DialogName,DialogName,DIALOG_EVENT,OutType);
	if (!ChildDialog)
	{
		return NULL;
	}
	
	m_DialogMutex.AcquireThis(this);
	m_ChildEventList[EventID] = EventID; //通用对话
	m_DialogMutex.ReleaseThis(this);

	ChildDialog->m_bEditValid = bEditValid;
	ClientAddress.SetID(m_SourceID);

	//生成一个与任务对话对应的事件
	GetBrain()->GetBrainData()->RegisterEvent(EventID,EventID,ClientExePipe,ClientAddress,EventInterval,bOnce,InstinctID,false);

	CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
	nf.PushInt(DL_ADD_DIALOG);
	nf.PushInt(bFocus); //是否设置为当前对话
	nf.Notify(ChildDialog);

	int64 ID = ClientExePipe.GetID();
	if (ID == RETURN_BREAK)
	{
		ClientExePipe.SetID(RETURN_DEBUG_WAIT);
	}else {
		ClientExePipe.SetID(RETURN_WAIT);
	}

	return ChildDialog;
	
};

void CLogicDialog::CloseEventDialog(int64 EventID,ePipeline& OldExePipe,ePipeline& ExePipe){

	assert(EventID !=0);

	m_DialogMutex.AcquireThis(this);
	map<int64,int64>::iterator it = m_ChildEventList.find(EventID); //通用对话
	if(it!=m_ChildEventList.end())m_ChildEventList.erase(it);

	m_DialogMutex.ReleaseThis(this);

	CBrainEvent Info;
	bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,Info,true);
	assert(ret);

	OldExePipe = Info.m_ClientExePipe;

	CLogicDialog* Dlg = GetBrain()->GetBrainData()->GetDialog(m_SourceID,EventID);
	if (Dlg)
	{
		
		CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
		nf.PushInt(DL_DEL_DIALOG);
		nf.Notify(Dlg);

		GetBrain()->GetBrainData()->DeleteDialog(m_SourceID,EventID);
	}

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
}



ePipeline&  CLogicDialog::SaveDialogItem(tstring Info,tstring Speaker,int32 State){
	
	int64 ID= AbstractSpace::CreateTimeStamp();
	tstring Time = AbstractSpace::GetTimer()->GetHMSM(ID);

	ePipeline* Item = new ePipeline(Speaker.c_str(),ID);
 
    Item->PushInt(State);
	Item->PushString(Info);
	Item->PushString(Time);  //javascript的64位整数运算能力还不够，所以直接发送字符串
	
	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(DIALOG_INFO);
	nf.Push_Directly(Item);
	nf.Notify(this);

	return *(ePipeline*)m_DialogHistory.GetLastData();
}


void CLogicDialog::ClearEltMsgList()
{
	m_ElementMsgList.ClearData();
};

void CLogicDialog::FeedbackToBrain(){

	int64 DialogID = m_DialogID;
    
	assert(DialogID != DEFAULT_DIALOG);
	
	
	CMsg rMsg(m_SourceID,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,DialogID);		
	ePipeline& rLetter = rMsg.GetLetter(false);
	rLetter.PushPipe(m_ExePipe);	
		
	GetBrain()->PushCentralNerveMsg(rMsg,false,false);	
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

	int64 MsgID = Msg.GetMsgID();

	ePipeline& TaskAddress = Msg.GetReceiver();
	int64 ID = TaskAddress.PopInt();

	assert(ID == m_DialogID);

	uint32 LogFlag = GetBrain()->GetLogFlag();
	if (TaskAddress.Size()==0)  //系统对话
	{
		if (LogFlag & LOG_MSG_PROC_BEGIN)
		{	
			if (MsgID != MSG_EVENT_TICK) //这个信息太频繁，忽略之
			{
				int64 EventID = Msg.GetEventID();

				tstring CmdStr = GetBrain()->MsgID2Str(MsgID);
				tstring& Tip = Msg.GetMsg().GetLabel();
				
				GetBrain()->OutputLog(LOG_MSG_PROC_BEGIN,Format1024(_T("-->Dialog(%I64ld) Pop Msg:%s EventID:%I64ld %s"),m_DialogID,CmdStr.c_str(),EventID,Tip.c_str()).c_str());
			}
		}

	    DialogMsgProc(&m_ExePipe,Msg);
		
	}
	//原则上已经设置停止标志的对话是不处理任何信息的，之所以MSG_TASK_RESULT例外是因为有一些自我委托的任务
	//需要接收这个信息后实现真正的停止。
    else if(MsgID==MSG_TASK_RESULT || GetTaskState() != TASK_STOP){ 
		
		if(MsgID == MSG_TASK_RESULT){
			m_ExePipe.Reuse();
		}

		if (LogFlag & LOG_MSG_TASK_PUSH)
		{
			if (MsgID != MSG_EVENT_TICK) //这个信息太频繁，忽略之
			{
				int64 EventID = Msg.GetEventID();
				tstring CmdStr = GetBrain()->MsgID2Str(MsgID);
				GetBrain()->OutputLog(LOG_MSG_TASK_PUSH,Format1024(_T("Push msg to task queue:%s EventID:%I64ld"),CmdStr.c_str(),EventID).c_str());
			}
		}		
		m_ElementMsgList.Push(Msg.Release());
		
	}
   
	AutoSysProcCounter  Counter(this);
  
	int32 n = Counter.GetProcNum();
    if (n>1) //只允许一个线程执行逻辑任务，如果有其它线程在处理会接着处理m_MsgList的信息
    {
		return;
    }
	
	while (m_ElementMsgList.DataNum())
	{
#ifdef NDEBUG
		try
		{	
#endif
			CMsg EltMsg;
			m_ElementMsgList.Pop(EltMsg);
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

					tstring CmdStr = GetBrain()->MsgID2Str(MsgID);
					tstring& Tip = EltMsg.GetMsg().GetLabel();
					GetBrain()->OutputLog(LOG_MSG_PROC_BEGIN,Format1024(_T("-->Task Pop Msg:%s EventID:%I64ld %s"),CmdStr.c_str(),EventID,Tip.c_str()).c_str());
					
				}
			}

			CLogicTask* CurTask = GetTask();

			ePipeline& EltAddress = EltMsg.GetReceiver();
			
			ePipeline  LocalAddress(m_SourceID);
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

					CLogicDialog* TargetDialog = this;
					if(m_DialogType == DIALOG_EVENT){
						TargetDialog = GetParentDialog();
						assert(TargetDialog);
					}
					if(TargetDialog){
						TargetDialog->RuntimeOutput(0,Error);
					}
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

			FeedbackToBrain();//目前有任何例外都视为当前任务结束
			continue;
		}
#endif
		//处理完后检查任务状态
		TASK_STATE NewState = GetTaskState();
		

		if (NewState == TASK_RUN )  //如果是RUN状态下运行到这步，则表示任务运行完毕
		{
            FeedbackToBrain();
		}		
	}
	
}


bool CLogicDialog::SetBreakPoint(ePipeline& Path,BOOL bEnable){
	if(m_TaskID == 0){
		RuntimeOutput(0,_T("the logic task is invalid"));
		return false;
	}
	CLogicTask* CurTask  = GetTask();
	
	ePipeline Path1; //用于回信
	Path1 = Path;

	bool ret = CurTask->SetBreakPoint(Path,bEnable);

	if (ret) //反馈
	{
		//在view，为了避免64位整数看起来过长，我们认为改为=1
		int64* id = (int64*)Path1.GetData(0);
		*id = 1;

		CNotifyDialogState nf(NOTIFY_DEBUG_VIEW);
		nf.PushInt(DEBUG_SET_BREAK);
		nf.PushInt(bEnable);
		nf.PushPipe(Path1);
		nf.Notify(this);
		return true;
	}else{
		RuntimeOutput(0,_T("set breakpoint fail: logic address error"));
	}
	return false;
}

THINK_STATE CLogicDialog::SetThinkState(THINK_STATE State){
	CLock((CABMutex*)&m_DialogMutex,this);
	THINK_STATE Old = m_ThinkState;
	m_ThinkState = State;

	CNotifyDialogState nf(NOTIFY_THINK_STATE);

	nf.PushInt(State);
	nf.Notify(this);

	return Old;
}

THINK_STATE CLogicDialog::GetThinkState(){
	CLock((CABMutex*)&m_DialogMutex,this);
	return m_ThinkState ;
}

void    CLogicDialog::EnableInput(bool bEnable){
	m_bEditValid = bEnable;
	CNotifyDialogState nf(NOTIFY_INPUT_VIEW);

	nf.PushInt(ENABLE_INPUT);
	nf.Notify(this);

}
TASK_STATE   CLogicDialog::SetTaskState(TASK_STATE State){
	CLock((CABMutex*)&m_DialogMutex,this);
	TASK_STATE Old = m_TaskState;
	m_TaskState = State;

	CNotifyDialogState nf(NOTIFY_TASK_STATE);

	nf.PushInt(m_WorkMode);
	nf.PushInt(State);
	nf.Notify(this);
	return Old;
}
TASK_STATE  CLogicDialog::GetTaskState(){
	CLock((CABMutex*)&m_DialogMutex,this);
	return m_TaskState ;
}


TASK_OUT_TYPE CLogicDialog::GetTaskType(){
	return m_TaskOutType;
}

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

		CTaskDialog* NewDialog = GetBrain()->GetBrainData()->GetDialog(EventID);
		if (!NewDialog)
		{
			NewDialog = GetBrain()->GetBrainData()->CreateNewDialog(this,m_SourceID,EventID,m_DialogID,_T("Dialog"),m_DialogName,DIALOG_SYSTEM_CHILD,0,OutType);	
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
	if(m_ThinkID==0)return;

	CLogicThread* Thread = GetThink();
	Thread->m_UserDialogID = -1;  //标记为待删除

	m_ThinkID = 0;
	m_bEditValid = TRUE;
	m_EditText = _T("");
	SetThinkState(THINK_IDLE);
}
void  CLogicDialog::ResetTask(){
	if(m_TaskID==0)return;
	CLogicTask* Task = GetTask();
	Task->m_UserDialogID = -1;//标记为待删除

	m_TaskID = 0;
	m_ExePipe.Reuse();
	m_ExePipe.Clear();
	SetTaskState(TASK_STOP);
	
	EnableInput(true);
	
	ClearEltMsgList();

	_CLOCK2(&m_DialogMutex,this);
	map<int64,int64>::iterator it = m_ChildEventList.begin();
	while (it != m_ChildEventList.end())
	{
		int64 PauseID = it->first;
		int64 EventID = it->second;
		
		if(PauseID==EventID){   //自动关闭普通子对话
			CLogicDialog* Dlg = GetBrain()->GetBrainData()->GetDialog(m_SourceID,EventID);
			if (Dlg)
			{
				CBrainEvent EventInfo;
				GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,true);
				GetBrain()->GetBrainData()->DeleteDialog(m_SourceID,EventID);

				CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
				nf.PushInt(DL_DEL_DIALOG);
				nf.Notify(Dlg);
			}
		}
		it++;
	}
	m_ChildEventList.clear();
	m_LogicAddress.Clear();
   
	if (m_ControlDialogID)
	{
		CLogicDialog* Dlg = GetBrain()->GetBrainData()->GetDialog(m_SourceID,m_ControlDialogID);
		if (Dlg)
		{
			GetBrain()->GetBrainData()->DeleteDialog(m_SourceID,m_ControlDialogID);
			CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
			nf.PushInt(DL_DEL_DIALOG);
			nf.Notify(Dlg);
		}
	}
	m_ControlDialogID = 0;
};



void CLogicDialog::SetWorkMode(WORK_MODE Mode){
	_CLOCK2(&m_DialogMutex,this);
    m_WorkMode = Mode;

	CNotifyDialogState nf(NOTIFY_WORK_MODE);
	nf.PushInt(m_WorkMode);
	nf.Notify(this);
}

WORK_MODE CLogicDialog::GetWorkMode(){
	_CLOCK2(&m_DialogMutex,this);
	WORK_MODE Mode = m_WorkMode;
	return Mode;
}

CLogicThread* CLogicDialog::GetThink(){
	if(m_ThinkID ==0){
		CLogicThread* Think = GetBrain()->GetBrainData()->CreateLogicThread(m_DialogID==DEFAULT_DIALOG?m_SourceID:m_DialogID);
		m_ThinkID = Think->m_ID;
		return Think;
	}

	CLogicThread* Thread = GetBrain()->GetBrainData()->GetLogicThread(m_ThinkID);
	return Thread;
};

CLogicTask* CLogicDialog::GetTask(){
	if(m_TaskID==0){
		CLogicTask* Task = GetBrain()->GetBrainData()->CreateLogicTask(m_DialogID==DEFAULT_DIALOG?m_SourceID:m_DialogID);
		m_TaskID = Task->m_ID;
		//tstring s = Format1024(_T("Dialog:%I64ld create new task:%I64ld"),m_DialogID,Task->m_ID);
		//GetBrain()->OutSysInfo(s.c_str());
		return Task;
	};
	CLogicTask* Task = GetBrain()->GetBrainData()->GetLogicTask(m_TaskID);
	return Task;
}

void  CLogicDialog::Think2TaskList()
{
	CLogicThread* Think = GetThink();

	CText& Text = Think->m_Text;
	m_TaskList.clear();

	if (Text.m_SentenceList.size()==0)
	{
		return ;
	}

	for(int i=0; i<Text.m_SentenceList.size(); i++){
		CSentence* Sentence = Text.m_SentenceList[i];
		tstring SentenceText  = Text.GetSentenceText(i);		
		Sentence->m_AnalyseResult.SetLabel(SentenceText.c_str());
	}
	m_TaskList.swap(Text.m_SentenceList);

	ResetThink();
}

std::auto_ptr<CSentence> CLogicDialog::PopTask(){
	std::auto_ptr<CSentence>  p(NULL);
	if(m_TaskList.size()==0){
		return p;
	}
	CSentence* Sentence = m_TaskList.front();
	m_TaskList.pop_front();
	p.reset(Sentence);
	return p;
};

CLogicDialog& CLogicDialog::operator << (CLogicDialog& Dialog){

	if(&Dialog == this){
		return *this;
	}

	m_TaskID = Dialog.m_TaskID;
	m_ThinkID   = Dialog.m_ThinkID;

	GetBrain()->GetBrainData()->SetLogicThreadUser(m_ThinkID,m_DialogID);
	GetBrain()->GetBrainData()->SetLogicTaskUser(m_TaskID,m_DialogID);

	m_EditText = Dialog.m_EditText;
	
	Dialog.m_EditText = _T("");
	Dialog.m_TaskID = 0;
	Dialog.m_ThinkID = 0;

	m_TaskList.swap(Dialog.m_TaskList);
	return *this;
}

void CLogicDialog::RuntimeOutput(tstring s){
		

	ePipeline* Item = new ePipeline(0);
	if (!Item)
	{
		return;
	}
	Item->m_Label = AbstractSpace::GetTimer()->GetHMSM(AbstractSpace::CreateTimeStamp()); //时间戳
	Item->PushString(s);
		
	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(RUNTIME_INFO);
	nf.Push_Directly(Item);
	nf.Notify(this);
}

void CLogicDialog::RuntimeOutput(int64 MassID,tstring s){

	ePipeline* Item = new ePipeline(MassID);
	if (!Item)
	{
		return;
	}
	Item->m_Label = AbstractSpace::GetTimer()->GetHMSM(AbstractSpace::CreateTimeStamp()); //时间戳
	tstring str = Format1024(_T("(%I64ld):\ns%"),MassID,s.c_str());
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
			

	m_LogicInstanceList.clear();
	
	m_TaskID = 0;

	CLogicTask* Task = GetTask();
	
	if (Task == NULL)
	{
		return false;
	}

	std::auto_ptr<CSentence> Sentence = PopTask();
	if (Sentence.get()==NULL)
	{
		m_CompileError = _T("no any sentence to be compiled");
		return false;
	}

	Task->ResetTime();
				
	m_LogicItemTree.Clear();
	m_LogicItemTree.SetID(Task->m_BeginTime); //通过标记新时间来确定之前的调试树是否有效
					
	//if(m_TaskOutType==TASK_OUT_OTHER){  //是输入给对方，让对方执行
	//	assert(Task->m_ActomList.size()==0);
	//	CCallRobot*   UseRobot = new CCallRobot(0,this,m_SourceID,Sentence->m_AnalyseResult);
	//	if (!UseRobot)
	//	{
	//		return false;
	//	}
	//	
	//	Task->m_LogicData =Sentence->m_AnalyseResult;
	//	Task->m_ActomList.push_back(UseRobot);
 //       Task->m_Name = _T("Use robot");
	//	
	//	SaveReceiveItem(Task->m_LogicText,0);

	//	return true;
	//}

	Task->m_Name = m_CurLogicName.size() ?m_CurLogicName:_T("Task");
	Task->m_LogicText = Sentence->m_AnalyseResult.GetLabel();
	Task->m_State = Sentence->m_State;

	//如果当前状态是学习逻辑，那么此任务会被登记为逻辑,而不是执行
	if (GetWorkMode() == WORK_THINK)
	{						
		Task->m_LogicData = Sentence->m_AnalyseResult;				
		
		m_CurLogicName = _T("");
		
		SaveDialogItem(Task->m_LogicText,m_DialogName,0);
		return true;
	}
	else if(m_DialogType == DIALOG_EVENT){
		CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(m_SourceID,m_ParentDialogID);
		assert(ParentDlg);
		if(ParentDlg->GetWorkMode()== WORK_THINK){
			Task->m_LogicData = Sentence->m_AnalyseResult;				

			m_CurLogicName = _T("");

			SaveDialogItem(Task->m_LogicText,m_DialogName,0);
			return true;
		}
	}

	SaveDialogItem(Task->m_LogicText,m_DialogName,0);
	if (!Task->Compile(this,&Sentence->m_AnalyseResult))
	{

		m_CompileError = Task->m_CompileError;
		SetTaskState(TASK_COMPILE_ERROR);
		return false;	
	}	
	return true;
}


void CLogicDialog::SentenceCompileProgress(int32 ClauseNum, int32 Completed){

	int32 per = Completed/ClauseNum;
	if(ClauseNum>20){
		CNotifyDialogState nf(NOTIFY_PROGRESS_OUTPUT);
		nf.PushInt(COMPILE_PROGRESS);
		nf.PushInt(per);
		nf.Notify(this);
	}	
}
