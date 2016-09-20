#include "..\Brain.h"
#include "..\LogicDialog.h"
#include "..\GUIMsgDefine.h"
#include "UseObjectElt.h"


CStartObject::CStartObject(int64 ID,ePipeline* Obj)
	:CElement(ID,_T("StartObject")),m_Obj(Obj)
{

};

CStartObject::~CStartObject(){

}


bool  CStartObject::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){ 

	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);

	if(!Msg.IsReaded()){
		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_TASK_RESULT)
		{	
			int64 EventID = Msg.GetEventID();
			if (EventID != GetEventID())
			{
				return true;
			}

			ePipeline& Letter = Msg.GetLetter(true);
			ePipeline  OldExePipe;
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(0);

			if (!NewExePipe->IsAlive())
			{
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}

			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{

				Dialog->CloseEventDialog(EventID,OldExePipe,ExePipe);
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}

			int64 InstanceID = NewExePipe->PopInt();

			assert(m_Obj->Size()==5);
			m_Obj->SetID(InstanceID);

			CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
			nf.PushInt(INSTANCE_OBJECT);
			nf.PushInt(CREATE_INSTANCE);
			nf.Push_Directly(m_Obj->Clone());
			nf.Notify(Dialog);


			ExePipe.Clear();
			ExePipe << *NewExePipe;

			Dialog->CloseEventDialog(EventID,OldExePipe,ExePipe);

			Dialog->AddObjectInstance(InstanceID,*m_Obj);
			Dialog->m_ObjectFocus = InstanceID;

			return true;
		}else{
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			if(ret == RETURN_DIRECTLY){
				return true;
			}
		}	
	}

	//如果没有启动外部器官，则启动
	CLinker Linker;
	GetBrain()->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		return ExeError(ExePipe,_T("Space disconnected."));
	}
	assert(ExePipe.IsAlive());

	//准备好一个请求事件
	UpdateEventID();

	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = _T("Waiting for starting external object");
	tstring Title = Format1024(_T("StartObject(%I64ld)"),m_ID);
	CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),Title,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,false,false,false);
	if(!Dlg){
		ExeError(ExePipe,_T("create event dialog fail"));
		return true;
	}
	Dlg->SaveDialogItem(DialogText,Dialog->m_DialogName,0);

	//把请求发给对方
	Reset();

	CMsg NewMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_ROBOT_START_OBJECT,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter(false);
	Letter.Push_Directly(m_Obj->Clone());
	Letter.Push_Directly(ExePipe.Clone());

	Linker().PushMsgToSend(NewMsg);
	return true;
};

//////////////////////////////////////////////////////////////////////////

bool CFocusObject::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){		

	tstring ObjectName = ExePipe.PopString();
	int64 InstanceID = Dialog->m_NamedObjectList.GetInstanceID(ObjectName);
	Dialog->m_ObjectFocus = InstanceID;
	if (InstanceID==0)
	{
		return ExeError(ExePipe,Format1024(_T("%I64ld: the object instance %s is not exist"),m_ID,ObjectName.c_str()));
	}

	return true;
}


bool  CFocusObject_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	int64 InstanceID = Dialog->m_NamedObjectList.GetInstanceID(m_FocusName);
	Dialog->m_ObjectFocus = InstanceID;
	if (InstanceID==0)
	{
		return ExeError(ExePipe,Format1024(_T("%I64ld: the object isntance %s is not exist"),m_ID,m_FocusName.c_str()));
	}

	return true;
};


bool  CFocusObject_Inter::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	Dialog->m_ObjectFocus = m_InstanceID;

	return true;
};

//////////////////////////////////////////////////////////////////////////

CNameObject::CNameObject(int64 ID)
	:CElement(ID,_T("Name object"))
{

}

CNameObject::~CNameObject(){

};

bool CNameObject::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	ePipeline* ObjectInstance = Dialog->FindObjectInstance(Dialog->m_ObjectFocus);
	if (ObjectInstance==NULL)
	{
		return false;
	}

	tstring SrcObjectName = *(tstring*)ObjectInstance->GetData(0);

	tstring InstanceName = ExePipe.PopString();

	if (Dialog->m_NamedObjectList.HasName(Dialog,InstanceName))
	{
		return false;
	}
	Dialog->m_NamedObjectList.RegisterNameByTask(InstanceName,Dialog->m_TaskID,Dialog->m_ObjectFocus);

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_OBJECT);
	nf.PushInt(NAME_INSTANCE);
	nf.PushString(SrcObjectName);
	nf.PushInt(Dialog->m_ObjectFocus);
	nf.PushString(InstanceName);
	nf.Notify(Dialog);

	return true;
}

CNameObject_Static::CNameObject_Static(int64 ID,tstring Name)
	:CElement(ID,_T("Name object")),m_Name(Name)
{

}

CNameObject_Static::~CNameObject_Static(){

};

bool CNameObject_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	ePipeline* ObjectInstance = Dialog->FindObjectInstance(Dialog->m_ObjectFocus);
	if (ObjectInstance==NULL)
	{
		return false;
	}

	tstring SrcObjectName = *(tstring*)ObjectInstance->GetData(0);

	tstring& InstanceName = m_Name;

	if (Dialog->m_NamedObjectList.HasName(Dialog,InstanceName))
	{
		return false;
	}
	Dialog->m_NamedObjectList.RegisterNameByTask(InstanceName,Dialog->m_TaskID,Dialog->m_ObjectFocus);

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_OBJECT);
	nf.PushInt(NAME_INSTANCE);
	nf.PushString(SrcObjectName);
	nf.PushInt(Dialog->m_ObjectFocus);
	nf.PushString(InstanceName);
	nf.Notify(Dialog);

	return true;
}

//////////////////////////////////////////////////////////////////////////


CGetObjectDoc::CGetObjectDoc(int64 ID)
	:CElement(ID,_T("UseObjectDoc")){

};

CGetObjectDoc::~CGetObjectDoc(){

}

MsgProcState CGetObjectDoc::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_TASK_RESULT)
	{	

		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID())
		{
			return RETURN_DIRECTLY;
		}

		ePipeline& Letter = Msg.GetLetter(true);
		ePipeline  OldExePipe;
		ePipeline* NewExePipe= (ePipeline*)Letter.GetData(0);

		if (!NewExePipe->IsAlive())
		{
			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
			ExePipe.Break();
			return RETURN_DIRECTLY;
		}

		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
			ExePipe.SetID(retTask);
			ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
			return RETURN_DIRECTLY;
		}

		if (NewExePipe->Size()>0 && NewExePipe->GetDataType(0)== TYPE_STRING)
		{
			tstring Doc = NewExePipe->PopString();
			Dialog->RuntimeOutput(0,Doc);
		}

		Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);

		return RETURN_DIRECTLY; // 不在继续执行TaskProc
	}else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
	}

	return RETURN_DIRECTLY;
}


bool  CGetObjectDoc::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){


	ePipeline* ObjectInstance = Dialog->FindObjectInstance(Dialog->m_ObjectFocus);
	if (ObjectInstance==NULL)
	{
		return false;
	}

	assert(ObjectInstance->GetID() ==Dialog->m_ObjectFocus );

	//如果没有启动外部器官，则启动
	CLinker Linker;
	GetBrain()->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		return ExeError(ExePipe,_T("Space disconnected."));
	};

	//这个子对话主要起提示作用，实际任务会交给远方执行
	UpdateEventID();
	tstring DialogText = Format1024(_T("Waiting for run external object:%I64ld  EventID:%I64ld"),Dialog->m_ObjectFocus,GetEventID());
	tstring Title = Format1024(_T("UseObject(%I64ld)"),m_ID);
	CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),Title,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,false,false,false);
	if(!Dlg){
		return ExeError(ExePipe,_T("create event dialog fail"));
	}
	Dlg->SaveDialogItem(DialogText,Dialog->m_DialogName,0);

	//把请求发给对方
	CMsg NewMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_ROBOT_GET_OBJECT_DOC,LocalAddress,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter(false);
	Letter.PushInt(	Dialog->m_ObjectFocus);
	Letter.Push_Directly(ExePipe.Clone());

	Linker().PushMsgToSend(NewMsg);

	return true;
};


//////////////////////////////////////////////////////////////////////////

CUseObject::CUseObject(int64 ID)
	:CElement(ID,_T("UseObject"))
{
};

CUseObject::~CUseObject(){

}

MsgProcState CUseObject::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_TASK_RESULT)
	{	
		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID())
		{
			return RETURN_DIRECTLY;
		}

		int64 SourceID = Msg.GetSourceID();
		if(SourceID == SPACE_SOURCE){

			ePipeline& Letter = Msg.GetLetter(true);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(0);
			ePipeline OldExePipe;

			if (!NewExePipe->IsAlive())
			{
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
				ExePipe.Break();
				return RETURN_DIRECTLY;
			}

			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return RETURN_DIRECTLY;
			}


			ExePipe.Clear();
			ExePipe << *NewExePipe;

			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
		}else{
			//MSG_TASK_RUNTIME_LOGIC的执行结果

			ePipeline& Letter = Msg.GetLetter(true);
			ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);

			CLinker SpaceLinker;
			GetBrain()->GetLinker(SPACE_SOURCE,SpaceLinker);

			CMsg  rMsg(SPACE_SOURCE,m_SpaceEventID,MSG_TASK_RUNTIME_RESULT,DEFAULT_DIALOG,0);
			rMsg.GetLetter(false).PushPipe(*NewExePipe);

			SpaceLinker().PushMsgToSend(rMsg);
		}

		return RETURN_DIRECTLY; // 不在继续执行TaskProc
	}
	else if (MsgID == MSG_TASK_RUNTIME_INFO)
	{
		ePipeline& Letter = Msg.GetLetter(true);
		if(!Letter.HasTypeAB(PARAM_TYPE1(TYPE_PIPELINE))){
			Dialog->RuntimeOutput(m_ID,_T("Receive a msg(%s) that include invalid data"),GetBrain()->MsgID2Str(MsgID).c_str());
			
		}else{

			ePipeline* Info = (ePipeline*)Letter.GetData(0);
			CPipeView PipeView(Info);

			tstring RuntimeInfo = PipeView.GetString();
			Dialog->RuntimeOutput(m_ID,RuntimeInfo);
		}
	}else if (MsgID == MSG_TASK_RUNTIME_LOGIC)
	{
		ePipeline& Letter = Msg.GetLetter(true);
		if(!Letter.HasTypeAB(PARAM_TYPE1(TYPE_STRING))){
			Dialog->RuntimeOutput(m_ID,_T("Receive a msg(%s) that include invalid data"),GetBrain()->MsgID2Str(MsgID).c_str());
		}else{
			tstring DialogInfo = Letter.PopString();
			CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());
			assert(EventDlg);

			m_SpaceEventID = Msg.GetSenderID();

			EventDlg->SaveDialogItem(DialogInfo,Dialog->m_DialogName,0);

			EventDlg->ResetThink();
			EventDlg->SetThinkState(THINK_IDLE);
		}
	}	
	else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
	}

	return RETURN_DIRECTLY;
}


bool  CUseObject::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	ePipeline* ObjectInstance = Dialog->FindObjectInstance(Dialog->m_ObjectFocus);

	if (ObjectInstance==NULL)
	{
		return ExeError(ExePipe,_T("the focus object instance is not exist."));
	}

	assert(ObjectInstance->GetID() == Dialog->m_ObjectFocus );

	//如果没有启动外部器官，则启动
	CLinker Linker;
	GetBrain()->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		return ExeError(ExePipe,_T("Space disconnected."));
	};

	UpdateEventID();
	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = Format1024(_T("Waiting for run external object:%I64ld  EventID:%I64ld"),Dialog->m_ObjectFocus,GetEventID());
	tstring Title = Format1024(_T("UseObject(%I64ld)"),m_ID);
	CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),Title,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,false,false,false);
	if(!Dlg){
		return ExeError(ExePipe,_T("create event dialog fail"));
	}
	Dlg->SaveDialogItem(DialogText,Dialog->m_DialogName,0);

	//把请求发给对方
	CMsg NewMsg(Dialog->m_SourceID,DEFAULT_DIALOG,MSG_ROBOT_USE_OBJECT,LocalAddress,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter(false);
	Letter.PushInt(Dialog->m_ObjectFocus);
	Letter.Push_Directly(ExePipe.Clone());

	Linker().PushMsgToSend(NewMsg);


	return true;
};

//////////////////////////////////////////////////////////////////////////
CCloseObject::CCloseObject(int64 ID)
	:CElement(ID,_T("close object"))
{
}

CCloseObject::~CCloseObject(){

}

bool  CCloseObject::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){ 

	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);

	if(!Msg.IsReaded()){

		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_TASK_RESULT)
		{	
			int64 EventID = Msg.GetEventID();
			if (EventID != GetEventID())
			{
				return false;
			}

			ePipeline& Letter = Msg.GetLetter(true);
			ePipeline  OldExePipe;
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(0);

			if (!NewExePipe->IsAlive())
			{
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}

			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}

			Dialog->m_NamedObjectList.UnregisterNameByTask(Dialog->m_ObjectFocus);

			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);

			ExePipe.Clear();
			ExePipe << OldExePipe;

			return true;
		}else{
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			if(ret == RETURN_DIRECTLY){
				return true;
			}
		}	
	}

	ePipeline* ObjectInstance = Dialog->FindObjectInstance(Dialog->m_ObjectFocus);
	if (ObjectInstance==NULL)
	{
		return false;
	}

	assert(ObjectInstance->GetID() ==Dialog->m_ObjectFocus );

	int64 InstanceID = Dialog->m_ObjectFocus;

	tstring SrcObjectName = *(tstring*)ObjectInstance->GetData(0);

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_OBJECT);
	nf.PushInt(CLOSE_INSTANCE);
	nf.PushString(SrcObjectName);
	nf.PushInt(Dialog->m_ObjectFocus);
	nf.Notify(Dialog);

	
	Dialog->CloseObjectInstance(Dialog->m_ObjectFocus);

	//如果没有启动外部器官，则启动
	CLinker Linker;
	GetBrain()->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		Dialog->RuntimeOutput(0,_T("Space disconnected."));			
		return true;
	}

	UpdateEventID();
	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = Format1024(_T("Waiting for closing external object instance %I64ld"),Dialog->m_ObjectFocus);
	tstring Title = Format1024(_T("Close Object(%d)"),m_ID);
	CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),Title,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,false,false,false);
	if(!Dlg){
		ExeError(ExePipe,_T("create event dialog fail"));
		return true;
	}
	Dlg->SaveDialogItem(DialogText,Dialog->m_DialogName,0);

	//把请求发给对方
	CMsg NewMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_ROBOT_CLOSE_OBJECT,LocalAddress,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter(false);
	Letter.PushInt(InstanceID);
	Linker().PushMsgToSend(NewMsg);
	return true;
};
