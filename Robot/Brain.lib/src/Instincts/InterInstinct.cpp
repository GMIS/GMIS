// BrainObject.cpp: implementation of the CBrainObject class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "..\Brain.h"
#include "..\LogicDialog.h"
#include "InterInstinct.h"
#include "..\GUIMsgDefine.h"
#include "..\InstinctDefine.h"
//#include "Arm.h"

//////////////////////////////////////////////////////////////////////////

CInterInstinct::CInterInstinct()
:CElement(0,_T("BrainObject")){

};

CInterInstinct::CInterInstinct(int64 ID,tstring Name,int64 InstinctID,ePipeline& Param)
:CElement(ID,Name.c_str()),m_InstinctID(InstinctID)
{
	m_Param << Param;

}

CInterInstinct::~CInterInstinct(){

}


bool  CInterInstinct::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg)
{
	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);

	bool ret = false;
	switch(m_InstinctID){
	case INSTINCT_THINK_LOGIC:
		ret = DoThinkLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_RUN_TASK:
		ret = DoRunTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_DEBUG_TASK: 
		ret = DoDebugTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_TEST_TASK:
		ret = DoTestTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_STOP_TASK:
		ret = DoStopTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_PAUSE_TASK:
		ret = DoPauseTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_STEP_TASK:
		ret = DoStepTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_GOTO_TASK:
		break;
	case INSTINCT_CLOSE_DIALOG:
		ret = DoCloseDialog(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_CREATE_ACCOUNT:
		ret = DoCreateAccount(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_DELETE_ACCOUNT:
		ret = DoDeleteAccount(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_SET_LOGIC_ADDRESS:
		ret = DoSetLogicAddress(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_GET_LOGIC_ADDRESS:
		ret = DoGetLogicAddress(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_SET_LOGIC_BREAKPOINT:
		ret = DoSetLogicBreakpoint(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_REMOVE_LOGIC:
		ret = DoRemoveLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_INSERT_LOGIC:
		ret = DoInsertLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_REMOVE_TEMP_LOGIC:
		ret = DoRemoveTempLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_CLEAR_TEMP_LOGIC:
		ret = DoClearTempLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_WORD:
		ret = DoLearnWord(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_TEXT:
		ret = DoLearnText(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_LOGIC:
		ret = DoLearnLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_OBJECT:
		ret = DoLearnObject(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_ACTION:
		ret = DoLearnAction(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_LANGUAGE:
		ret = DoLearnLanguage(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_FIND:
		ret = DoFind(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_FIND_LOGIC:
		ret = DoFindLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_FIND_OBJECT:
		ret = DoFindObject(Dialog,ExePipe,LocalAddress,Msg);
		break;
	default:
		assert(0);
	}	
	return ret;
}

bool  CInterInstinct::DoCloseDialog(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	TASK_STATE  State = Dialog->GetTaskState();

	if (Dialog->m_DialogType == DIALOG_SYSTEM)
	{
		return true;
	}
	if (Dialog->m_DialogType == DIALOG_TASK)
	{
		CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
		nf.PushInt(DL_DEL_DIALOG);
		nf.Notify(Dialog);

		GetBrain()->GetBrainData()->DeleteDialog(Dialog->m_SourceID,Dialog->m_DialogID);

		return true;
	}
	return true;
}

bool  CInterInstinct::DoCreateAccount(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if ((Dialog->m_SourceID != LOCAL_GUI_SOURCE && Dialog->m_SourceID != SYSTEM_SOURCE) || (Dialog->m_DialogType != DIALOG_SYSTEM && Dialog->m_DialogType != DIALOG_TASK ))
	{
		return ExeError(ExePipe,_T("this dialog not authorized to perform this operation"));
	}
		
	if (!ExePipe.HasTypeAB(PARAM_TYPE3(TYPE_STRING,TYPE_STRING,TYPE_INT)))
	{
		return ExeError(ExePipe,_T("parameter is not valid"));
	}
	tstring Name = ExePipe.PopString();
	tstring Password = ExePipe.PopString();
	int64   DialogType = ExePipe.PopInt();

	tstring CrypStr = Name + Password;

	ePipeline UserInfo;
	if(GetBrain()->GetBrainData()->FindRegisterUserInfo(Name,CrypStr,UserInfo)){
		return ExeError(ExePipe,Format1024(_T("User Name '%s' has exist"),Name.c_str()));
	};

	if (DialogType ==DIALOG_SYSTEM)
	{
		GetBrain()->GetBrainData()->RegisterUserAccount(Name,CrypStr,DIALOG_SYSTEM);
	}else{
		return ExeError(ExePipe,_T("DialogType is not valid"));
	}
	return true;
};
bool  CInterInstinct::DoDeleteAccount(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if (Dialog->m_SourceID != LOCAL_GUI_SOURCE && Dialog->m_DialogType != DIALOG_SYSTEM)
	{
		return ExeError(ExePipe,_T("this dialog not authorized to perform this operation"));
	}

	if (!ExePipe.HasTypeAB(PARAM_TYPE2(TYPE_STRING,TYPE_STRING)))
	{
		return ExeError(ExePipe,_T("parameter is not valid"));
	}
	tstring Name     = ExePipe.PopString();
	tstring Password = ExePipe.PopString();
	tstring CrypStr = Name + Password;
	int64 SourceID  = GetBrain()->GetBrainData()->DeleteUserAccount(Name,CrypStr);
	if (SourceID>0)
	{
		int32 n = GetBrain()->GetBrainData()->DeleteDialogOfSource(SourceID);
	}
	return true;
};

bool  CInterInstinct::DoGetLogicAddress(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if(Dialog->m_DialogType == DIALOG_EVENT){
		CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
		if (ParentDlg==NULL)
		{
			return ExeError(ExePipe,_T("not find parent dialog"));
		}

		ePipeline& LogicAddress = ParentDlg->m_LogicAddress;
		if(LogicAddress.Size()){
			Energy* e = LogicAddress.GetLastEnergy();
			ExePipe.Push_Directly(e->Clone());
		}
	}else {
		ePipeline& LogicAddress = Dialog->m_LogicAddress;
		if(LogicAddress.Size()){
			Energy* e = LogicAddress.GetLastEnergy();
			ExePipe.Push_Directly(e->Clone());
		}
	}	
	return true;
}

bool  CInterInstinct::DoRemoveLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if(Dialog->m_DialogType != DIALOG_EVENT){
		return ExeError(ExePipe,_T("'remove logic' command is available in execution mode")); 
	}

	int64 EventID = Dialog->m_DialogID;
	CBrainEvent EventInfo;
	bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,false);
	if (!ret) 
	{
		return true;
	}

	if (EventInfo.m_InstinctID == INSTINCT_PAUSE_TASK)
	{
		return ExeError(ExePipe,_T("'remove logic' command is invalid in pause branch"));
	}

	CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		return ExeError(ExePipe,_T("not find parent dialog"));
	}

	if(ParentDlg->GetTaskState() != TASK_PAUSE){
		return ExeError(ExePipe,_T("'remove logic' command is available in task pause state"));   
	}

	ePipeline LogicAddress = ParentDlg->m_LogicAddress;

	if(LogicAddress.Size()==0){
		return ExeError(ExePipe,_T("the  logic address where will be remove object is empty"));
	}


	int64 LastItemID = *(int64*)LogicAddress.GetLastData();
	
	//排除欲删除目标在已经暂停的某个路径上
	ePipeline PauseList;
	ParentDlg->GetPauseEventIDList(PauseList);
	while(PauseList.Size()){
		int64 PauseEventID = PauseList.PopInt();
		CBrainEvent EventInfo;
		bool ret = GetBrain()->GetBrainData()->GetEvent(PauseEventID,EventInfo,false);
		if(ret){
			ePipeline& PauseAddress = EventInfo.m_ClientAddress;
			for(int i=0; i<PauseAddress.Size(); i++){
				int64 MassID = *(int64*)PauseAddress.GetData(i);
				if(MassID==LastItemID){  //欲删除的目标在当前pause路径上，则不允许删除
					return ExeError(ExePipe,_T("can't remove the object because the  logic address  is on the path of pause"));
				}
			}

		}
	}
	
	
	LogicAddress.EraseBackEnergy();

	CMsg EltMsg(Dialog->m_SourceID,LogicAddress,MSG_ELT_REMOVE_LOGIC,DEFAULT_DIALOG,0);
	ePipeline& Letter = EltMsg.GetLetter(false);
	Letter.PushInt(LastItemID); //避免条目自己删除自己的情况

	GetBrain()->PushNerveMsg(EltMsg,false,false);
	return true;

};

bool  CInterInstinct::DoInsertLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){

	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	tstring  LogicNameOrLogicText = m_Param.PopString();

	if(Dialog->m_DialogType != DIALOG_EVENT){
		return ExeError(ExePipe,_T("'insert logic' command is available in execution mode"));   //忽略
	}

	int64 EventID = Dialog->m_DialogID;
	CBrainEvent EventInfo;
	bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,false);
	if (!ret) 
	{
		return true;
	}

	if (EventInfo.m_InstinctID == INSTINCT_PAUSE_TASK)
	{
		return ExeError(ExePipe,_T("'insert logic' command is invalid in pause branch"));   //忽略
	}

	CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		return ExeError(ExePipe,_T("not find parent dialog"));
	}

	if(ParentDlg->GetTaskState() != TASK_PAUSE){
		return ExeError(ExePipe,_T("'Insert logic' command is available in task pause state"));   //忽略
	}

	ePipeline LogicAddress = ParentDlg->m_LogicAddress;

	if(LogicAddress.Size()==0){
		return ExeError(ExePipe,_T("the  logic address where will be inserted logic is empty"));
	}

	int64 LastItemID = *(int64*)LogicAddress.GetLastData();
	LogicAddress.EraseBackEnergy();

	CMsg EltMsg(Dialog->m_SourceID,LogicAddress,MSG_ELT_INSERT_LOGIC,DEFAULT_DIALOG,0);
	ePipeline& Letter = EltMsg.GetLetter(false);
	Letter.PushInt(LastItemID);
	Letter.PushString(LogicNameOrLogicText);

	GetBrain()->PushNerveMsg(EltMsg,false,false);
	
	return true;
};

bool  CInterInstinct::DoRemoveTempLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	tstring LogicName = m_Param.PopString();

	if(Dialog->m_DialogType == DIALOG_EVENT){
		CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
		if (ParentDlg==NULL)
		{
			return ExeError(ExePipe,_T("not find parent dialog"));
		}

		ParentDlg->DeleteLogic(LogicName);
	}else{
		Dialog->DeleteLogic(LogicName);
	}

	return true;
}
bool  CInterInstinct::DoClearTempLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if(Dialog->m_DialogType == DIALOG_EVENT){
		CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
		if (ParentDlg==NULL)
		{
			return ExeError(ExePipe,_T("not find parent dialog"));
		}

		ParentDlg->ClearLogicSence();
	}else{
		Dialog->ClearLogicSence();
	}
	return true;
}	

bool  CInterInstinct::DoThinkLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){	
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if(Dialog->m_DialogType == DIALOG_EVENT){
		int64 EventID = Dialog->m_DialogID;
		CBrainEvent EventInfo;
		bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,false);
		if (!ret) 
		{
			return true;
		}
	
		CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
		if (ParentDlg==NULL)
		{
			return ExeError(ExePipe,_T("not find parent dialog"));
		}

		if (EventInfo.m_InstinctID == INSTINCT_PAUSE_TASK)
		{
			ParentDlg->RuntimeOutput(0,_T("cannot execute this command in pause dialog"));
			return true;   //忽略
		}

		ParentDlg->m_CurLogicName = m_Param.PopString();
		ParentDlg->SetWorkMode(WORK_THINK);
	}else{
		Dialog->m_CurLogicName = m_Param.PopString();
		Dialog->SetWorkMode(WORK_THINK);
	}
	return true;
}


bool  CInterInstinct::DoDebugTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	
	Dialog->SetWorkMode(WORK_DEBUG);
	return true;	
};

bool CInterInstinct::DoTestTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	Dialog->SetWorkMode(WORK_TEST);
	return true;	
}

bool  CInterInstinct::DoPauseTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){

	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}


	int64 EventID = Dialog->m_DialogID;
	CBrainEvent EnventInfo;
	bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EnventInfo,false);
	if (!ret) 
	{
		return true;
	}


	CMsg Msg1(SYSTEM_SOURCE,EnventInfo.m_ClientAddress,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = Msg1.GetLetter(false);
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_PAUSE);

	GetBrain()->PushNerveMsg(Msg1,false,false);

    return true;
};

bool  CInterInstinct::DoRunTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if(Dialog->m_DialogType != DIALOG_EVENT){
		return ExeError(ExePipe,_T("'run' command is available in pause state"));   
	}

	int64 EventID = Dialog->m_DialogID;
	CBrainEvent EventInfo;
	bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,false);
	if (!ret) 
	{
		return true;
	}

	CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		return ExeError(ExePipe,_T("not find parent dialog"));
	}

	if(!ParentDlg->IsPaused()){
		return ExeError(ExePipe,_T("the task is running"));
	}
	//如果是在预生成的暂停分支执行此命令
	if (EventInfo.m_InstinctID == INSTINCT_PAUSE_TASK)
	{
		int64 PauseID = *(int64*)EventInfo.m_ClientAddress.GetLastData();

		CMsg Msg1(SYSTEM_SOURCE,ParentDlg->m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = Msg1.GetLetter(false);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN);
		Letter.PushInt(PauseID);

		GetBrain()->PushNerveMsg(Msg1,false,false);
		return true;
	}

	ePipeline&  LogicAddress = ParentDlg->m_LogicAddress;
	if(LogicAddress.Size()==0){
		CMsg Msg1(SYSTEM_SOURCE,EventInfo.m_ClientAddress,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = Msg1.GetLetter(false);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN);
		Letter.PushInt(0);

		GetBrain()->PushNerveMsg(Msg1,false,false);
		return true;
	}else{
		int64 PauseID = *(int64*)LogicAddress.GetLastData();
		CMsg Msg1(SYSTEM_SOURCE,EventInfo.m_ClientAddress,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = Msg1.GetLetter(false);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN);
		Letter.PushInt(PauseID);

		GetBrain()->PushNerveMsg(Msg1,false,false);
		return true;
	}

	return true;		
};
bool  CInterInstinct::DoStopTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){

	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if(Dialog->m_DialogType != DIALOG_EVENT){
		return ExeError(ExePipe,_T("'stop' command is available in execution mode"));   //忽略
	}

	int64 EventID = Dialog->m_DialogID;
	CBrainEvent EnventInfo;
	bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EnventInfo,false);
	if (!ret) 
	{
		return true;
	}

	int64 ReceiverID = Dialog->m_ParentDialogID;

	CMsg Msg1(SYSTEM_SOURCE,ReceiverID,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = Msg1.GetLetter(false);
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_STOP);

	GetBrain()->PushNerveMsg(Msg1,false,false);

	return true;	
};
bool  CInterInstinct::DoStepTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if(Dialog->m_DialogType != DIALOG_EVENT){
		return ExeError(ExePipe,_T("'step' command is available in execution mode"));   //忽略
	}

	int64 EventID = Dialog->m_DialogID;
	CBrainEvent EventInfo;
	bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,false);
	if (!ret) 
	{
		return true;
	}


	CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		return ExeError(ExePipe,_T("not find parent dialog"));
	}

	//如果是在预生成的暂停分支执行此命令
	if (EventInfo.m_InstinctID == INSTINCT_PAUSE_TASK)
	{
		int64 PauseID = *(int64*)EventInfo.m_ClientAddress.GetLastData();

		CMsg Msg1(SYSTEM_SOURCE,ParentDlg->m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = Msg1.GetLetter(false);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_DEBUG_STEP);
		Letter.PushInt(PauseID);

		GetBrain()->PushNerveMsg(Msg1,false,false);
		return true;
	}

	ePipeline&  LogicAddress = ParentDlg->m_LogicAddress;
	if(LogicAddress.Size()==0){
		CMsg Msg1(SYSTEM_SOURCE,EventInfo.m_ClientAddress,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = Msg1.GetLetter(false);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_DEBUG_STEP);
		Letter.PushInt(0);

		GetBrain()->PushNerveMsg(Msg1,false,false);
		return true;
	}else{
		int64 PauseID = *(int64*)LogicAddress.GetLastData();
		CMsg Msg1(SYSTEM_SOURCE,EventInfo.m_ClientAddress,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = Msg1.GetLetter(false);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_DEBUG_STEP);
		Letter.PushInt(PauseID);

		GetBrain()->PushNerveMsg(Msg1,false,false);
		return true;
	}

	return true;		
};


bool  CInterInstinct::DoSetLogicBreakpoint(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	int64 bBreak = m_Param.PopInt();

	if(Dialog->m_DialogType != DIALOG_EVENT){
		return ExeError(ExePipe,_T("'set logic breakpoint' command is available in execution mode"));   //忽略
	}

	int64 EventID = Dialog->m_DialogID;
	CBrainEvent EventInfo;
	bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,false);
	if (!ret) 
	{
		return true;
	}

	if (EventInfo.m_InstinctID == INSTINCT_PAUSE_TASK)
	{
		return ExeError(ExePipe,_T("'set logic breakpoint' command is invalid in pause branch"));   //忽略
	}
	CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		return ExeError(ExePipe,_T("not find parent dialog"));
	}

	if(ParentDlg->GetWorkMode() != WORK_DEBUG){
		ExePipe.GetLabel() =_T("'set logic breakpoint' command  is invalid when the task running");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}

	ePipeline LogicAddress = ParentDlg->m_LogicAddress;

	if(LogicAddress.Size()==0){
		return ExeError(ExePipe,_T("the  logic address where will be setted breakpoint is empty"));
	}

	int64 ReceiverID = Dialog->m_ParentDialogID;

	//为了和工具条的鼠标点击操作地址兼容，这里预先删除内部使用的DialogID
	int64 DialogID = LogicAddress.PopInt();

	CMsg Msg1(SYSTEM_SOURCE,ReceiverID,MSG_FROM_BRAIN,DEFAULT_DIALOG,0);
	ePipeline& Letter = Msg1.GetLetter(false);
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_SET_BREAKPOINT);
	Letter.PushInt(bBreak);
	Letter.PushPipe(LogicAddress);

	GetBrain()->PushNerveMsg(Msg1,false,false);

	return true;
}

bool  CInterInstinct::DoSetLogicAddress(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}


	if(Dialog->m_DialogType != DIALOG_EVENT){
		return ExeError(ExePipe,_T("'set logic address' command is available in execution mode"));
	}

	CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		return ExeError(ExePipe,_T("not find parent dialog"));
	}
	assert(ParentDlg->m_TaskID);
	
	int64 ItemID = 0; 
	
	if(m_Param.Size()){
		ItemID = m_Param.PopInt();
	}else{

		if(ExePipe.Size()==0){ //设置空地址
			ParentDlg->m_LogicAddress.Clear();
			return true;
		}

		if (!ExePipe.HasTypeAB(PARAM_TYPE1(TYPE_INT)))
		{
			return ExeError(ExePipe,_T("parameter is not valid"));
		}
		
		ItemID = ExePipe.PopInt();

	}

	CLogicTask* Task = ParentDlg->GetTask();

	ePipeline LogicAddress;
	LogicAddress.PushInt(ParentDlg->m_DialogID);

	bool ret = Task->FindLogicAddress(ItemID,LogicAddress);
	if(!ret){
		return ExeError(ExePipe,Format1024(_T("not find the item id=%I64ld"),ItemID));
	}
	ParentDlg->m_LogicAddress = LogicAddress;


	return true;
}	
bool  CInterInstinct::DoGotoTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	return true;		
};

bool  CInterInstinct::DoLearnText(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	

	if (!ExePipe.HasTypeAB(PARAM_TYPE1(TYPE_STRING)))
	{
		return ExeError(ExePipe,_T("no text in the ExePipe"));
	}

	tstring Text = ExePipe.PopString();
	TriToken(Text);

	if(Text.size() == 0){ 
		return true;
	}

	if(Text.size()==1 && ispunct(Text[0])){
		return true;
	}
	
	int64 ID = Dialog->GetThink()->LearnText(Text);
	
	if(ID==0){
		return ExeError(ExePipe,_T("learn text fail."));
	}
	return true;	
};

bool  CInterInstinct::DoLearnLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if (!ExePipe.HasTypeAB(PARAM_TYPE2(TYPE_STRING,TYPE_STRING)))
	{
		return ExeError(ExePipe,_T("no logic name or memo in the ExePipe"));
	}

	tstring LogicName = ExePipe.PopString();
	LogicName = TriToken(LogicName);

	tstring LogicMemo = ExePipe.PopString();
	CLocalLogicCell* Logic = Dialog->FindLogic(LogicName);
	
	if(Logic == NULL){		
		return ExeError(ExePipe,Format1024(_T("no find logic \"%s\"."), LogicName.c_str()));
	}
	
	//预先记忆逻辑体
	ePipeline LogicData = Logic->m_Task.m_LogicData;
	
	CLogicThread* Think = Dialog->GetThink();
	int64 LogicID = Think->LearnLogic(Dialog,&LogicData);
	if(LogicID==0){
		return ExeError(ExePipe,Format1024(_T("learn logic \"%s\" fail."), LogicName.c_str()));
	}
	
	int64 ID = Think->CommentMemory(LogicMemo,LogicID);		
	if(ID ==0){
		tstring tip = Format1024(_T("learn logic memo fail"));
		Dialog->RuntimeOutput(tip);
	}
	return true;
};
bool  CInterInstinct::DoLearnObject(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	

	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	
	return ExeError(ExePipe,Format1024(_T("This command is currently unavailable")));	
	
	/*
	 //问题：如何记忆物体的地址还需要进一步研究
	   
	tstring ObjectName = m_Param.PopString();
	
	vector<CObjectData> ObjectList;
	
	Dialog->FindObject(ObjectName,ObjectList);
			 
	if(ObjectList.size() != 1){		
		tstring Answer = tformat(_T(" Object \"%s\" no find or more than one"), ObjectName.c_str());
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.Break();
		ExePipe.SetID(RETURN_ERROR);
		return false;	
	}

	//预先记忆逻辑体
	int64 ObjectID = 0;
	CObjectCell& Item = ObjectList.back();

	_ACCOUNT ac = GetAccount();
	uint32 Ip = ac.ip.GetUint32();
	tstring  AddressText = GetBrain()->m_MapView.GetAddressText();
	uint32 crc32 = _tcstoul(Item.m_CrcStr.c_str(),NULL,16);

	AddressText += ObjectName;
	tstring Comment = _T("");
	ObjectID = Dialog->m_Think->LearnObject(Ip,AddressText,crc32,Comment,Item.m_Type>OBJECT_SPACE);
	if(ObjectID==0){
		tstring Answer = tformat(_T("Learn Object \"%s\" fail."), ObjectName.c_str());
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.Break();
		ExePipe.SetID(RETURN_ERROR);
		return false;	
	}

	//Load Object的Memo置入当前Edit区,并转为文本模式
	StaticHandle  DLLHandle;
	tstring FileName = GetTempDir() + Item.m_Crc32Str;
	FileName +=_T(".dll");

	DLLHandle.LoadObject(FileName.c_str());
	if(!DLLHandle.IsValid()){	
		//这样得到FatherID可能会出现多线程问题，目前暂且不管
		int64 FatherID = GetBrain()->m_WorldShow->GetSpaceID();
		PhyAddress Address(FatherID,Item.m_Alias);
		if(Dialog->m_Think->LoadObject(Address)){
			DLLHandle.LoadObject(FileName.c_str());
		}
	};

	if(DLLHandle.IsValid()){	
		DLLHandle.LoadObject(FileName.c_str());
		Memo = DLLHandle.GetObjectDoc();
	}   

	tstring Memo;
	tstring Answer = tformat(_T("Please Input or edit object memo: (or empty)"));

	Dialog->RuntimeOutput(Msg,Answer);
	Dialog->m_Think->ClearAnalyse();

	ExePipe.Break();
	ExePipe.SetID(RETURN_PAUSE);


	//把Memo置入当前输入区,并转为文本输入模式
	CNotifyState nf(NOTIFY_DIALOG_STATE);
	nf.PushInt64(WORK_TASK);
	nf.PushInt64(TASK_IDLE);
	nf.PushString(Memo);
	nf.Notify(Dialog,Msg);

	m_Param.SetID(ObjectID);

	*/
	return true;

};
bool  CInterInstinct::DoLearnAction(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if (!ExePipe.HasTypeAB(PARAM_TYPE3(TYPE_STRING,TYPE_STRING,TYPE_STRING)))
	{
		return ExeError(ExePipe,_T("no logic name or command or memo in the ExePipe"));
	}

	tstring LogicName = ExePipe.PopString();
	LogicName = TriToken(LogicName);

	tstring Command = ExePipe.PopString();
	Command = TriToken(Command);

	tstring CmdMemo = ExePipe.PopString();

	//预先记忆逻辑体
	CLocalLogicCell* Logic = Dialog->FindLogic(LogicName);

	if(Logic == NULL){		
		return ExeError(ExePipe,Format1024(_T("no find logic \"%s\"."), LogicName.c_str()));
	}


	//预先记忆逻辑体
	ePipeline LogicData = Logic->m_Task.m_LogicData;
	
	CLogicThread* Think = Dialog->GetThink();
	
	int64 LogicID = Think->LearnLogic(Dialog,&LogicData);
	
	if(LogicID==0){
		return ExeError(ExePipe,_T("Learn logic fail."));
	}
	
	int64 CmdID = Think->LearnAction(Dialog,Command,LogicID);
	if (CmdID == 0)
	{
		return ExeError(ExePipe,Format1024(_T("Learn command  %s fail: %s"),Command.c_str(),Think->m_LastError.c_str()));
	}

	int64 CommentID = Think->CommentMemory(CmdMemo,CmdID);		
	if(CommentID ==0){
		tstring Info = Format1024(_T("learn action memo fail"));
		Dialog->RuntimeOutput(Info);
	}

	return true;	
};
bool  CInterInstinct::DoLearnLanguage(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{	
		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_TASK_RESULT)
		{		
			ePipeline& Letter = Msg.GetLetter(true);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(0);

			ePipeline OldExePipe;
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
				ExePipe.Break();
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}

			int64 ThinkID = NewExePipe->PopInt();
			CLogicThread* Think = GetBrain()->GetBrainData()->GetLogicThread(ThinkID);
			assert(Think);

			tstring Text = Think->GetUserInput();
			Text = TriToken(Text);

			CToken* Token = Think->JustOneToken();

			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);

			if (Text.size()==0 || !Token)
			{
				//再次压入事件，要求用户重新输入
				tstring DialogText = _T("Input error: not a token,Please input again.");
				UpdateEventID();
				CLogicDialog*  Dlg= Dialog->StartEventDialog(GetEventID(),_T("Input word"),TASK_OUT_THINK,OldExePipe,LocalAddress,TIME_SEC,true,true,true);				
				if (Dlg==NULL)
				{
					ExeError(ExePipe,_T("Start child dialog fail."));
					return CONTINUE_TASK;
				}
				Dlg->SaveDialogItem(DialogText,Dialog->m_DialogName,0);

				ExePipe.SetID(OldExePipe.GetID());

			}else {
				ExePipe.Clear();
				ExePipe<<OldExePipe;
				
				int64 WordID = m_Param.GetID();
				int64 ID = Think->CommentMemory(Text,WordID);		
				if(ID ==0){
					tstring Answer = Format1024(_T("learn word fail"));
					Dialog->RuntimeOutput(Answer);
				}
			}
			return false;
		}else{
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			if(ret == RETURN_DIRECTLY){
				return true;
			}
		}
	}

	if (!ExePipe.HasTypeAB(PARAM_TYPE3(TYPE_STRING,TYPE_STRING,TYPE_STRING)))
	{
		return ExeError(ExePipe,_T("no valid param in the ExePipe"));
	}

	tstring OldWord = ExePipe.PopString();
	OldWord = TriToken(OldWord);

	tstring PartofSpeech = ExePipe.PopString();
	PartofSpeech = TriToken(PartofSpeech);

	int32 Type = 0;
	if(PartofSpeech ==  _T("token"))
		Type = MEMORY_LAN;
	else if(PartofSpeech ==  _T("pronoun"))
		Type = MEMORY_PRONOUN;
	else if(PartofSpeech == _T("adjective"))
		Type = MEMORY_ADJECTIVE;
	else if(PartofSpeech == _T("numeral"))
		Type = MEMORY_NUMERAL;
	else if(PartofSpeech ==  _T("verb"))
		Type = MEMORY_VERB;
	else if(PartofSpeech ==  _T("adverb"))
		Type = MEMORY_ADVERB;
	else if(PartofSpeech == _T("article"))
		Type = MEMORY_ARTICLE;
	else if(PartofSpeech ==  _T("preposition"))
		Type = MEMORY_PREPOSITION;
	else if(PartofSpeech == _T("conjunction"))
		Type = MEMORY_CONJUNCTION;
	else if(PartofSpeech == _T("interjection"))
		Type = MEMORY_INTERJECTION;
	else if(PartofSpeech ==  _T("noun"))
		Type = MEMORY_NOUN;
	else {
		return ExeError(ExePipe,_T("Learn fail: the part of speech is invalid"));
	}

	tstring NewWord = ExePipe.PopString();
	NewWord = TriToken(NewWord);

	CLogicThread* Think = Dialog->GetThink();
	
	int64 WordID = Think->CheckWord(OldWord,Type);
	

	if(WordID == NULL){		
		tstring Info = Format1024(_T("do not understand  word \"%s\" as %s."), OldWord.c_str(),PartofSpeech.c_str());
		Dialog->RuntimeOutput(Info);
		return true;
	}

	
	Think->ThinkProc(Dialog,0,NewWord,false);
	CToken* Token = Think->JustOneToken();
	if(!Token ||Token->Size()==0){
		tstring Info  =Format1024(_T("the new  word \"%s\" is not a token"), NewWord.c_str());
		Dialog->RuntimeOutput(Info);
		return true;
	}

	int64 ID = Think->LearnWordFromWord(NewWord,Type,OldWord);
	if(ID ==0){
		tstring Info  =Format1024(_T("learn new  word \"%s\" fail"), NewWord.c_str());
		Dialog->RuntimeOutput(Info);
		return true;
	}
	return true;
}
bool  CInterInstinct::DoFind(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	
	tstring text;
    if(m_Param.Size()){
		text = m_Param.PopString();
	}
	else if (!ExePipe.HasTypeAB(PARAM_TYPE1(TYPE_STRING)))
	{
		return ExeError(ExePipe,_T("parameter is invalid"));
	}else{
		text = ExePipe.PopString();
	}

	CNotifyDialogState nf(NOTIFY_FIND_VIEW);
	nf.PushInt(FIND_START);
	nf.Notify(Dialog);
	
	CLogicThread* Think = Dialog->GetThink();
	Dialog->FindFirst(text);
	
	ePipeline SearchResult;
	Dialog->FindContinue(Think,0,SearchResult);
	
	Dialog->m_SearchOutput = SearchResult;

	//输出结果,0条目也要通知
	CNotifyDialogState nf1(NOTIFY_FIND_VIEW);
	nf1.PushInt(FIND_UPDATE);
	nf1.PushPipe(SearchResult);
	nf1.Notify(Dialog);

	
	//如果已经找到所有结果,通知搜索结束
	if (Dialog->m_FindSeedList.size()==0)
	{
		CNotifyDialogState nf2(NOTIFY_FIND_VIEW);
		nf2.PushInt(FIND_STOP);
		nf2.PushInt(TRUE);
		nf2.Notify(Dialog);	
		Dialog->m_SearchOutput.SetID(0); //没有更多结果
	}else{
		CNotifyDialogState nf2(NOTIFY_FIND_VIEW);
		nf2.PushInt(FIND_STOP);
        nf2.PushInt(FALSE);
		nf2.Notify(Dialog);	
		Dialog->m_SearchOutput.SetID(1); //还有更多结果
	}
	
    return true;	
};

bool  CInterInstinct::DoFindLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	tstring text;
	if(m_Param.Size()){
		text = m_Param.PopString();
	}
	else if (!ExePipe.HasTypeAB(PARAM_TYPE1(TYPE_STRING)))
	{
		return ExeError(ExePipe,_T("parameter is invalid"));
	}else{
		text = ExePipe.PopString();
	}
	
	CNotifyDialogState nf(NOTIFY_FIND_VIEW);
	nf.PushInt(FIND_START);
	nf.Notify(Dialog);
	
	CLogicThread* Think = Dialog->GetThink();
	Dialog->FindFirst(text,FIND_LOGIC);
	
	ePipeline SearchResult;
	Dialog->FindContinue(Think,0,SearchResult);
	
	Dialog->m_SearchOutput = SearchResult;
	
	//输出结果,0条目也要通知
	CNotifyDialogState nf1(NOTIFY_FIND_VIEW);
	nf1.PushInt(FIND_UPDATE);
	nf1.PushPipe(SearchResult);
	nf1.Notify(Dialog);
	
	
	//如果已经找到所有结果,通知搜索结束
	if (Dialog->m_FindSeedList.size()==0)
	{
		CNotifyDialogState nf2(NOTIFY_FIND_VIEW);
		nf2.PushInt(FIND_STOP);
		nf2.PushInt(TRUE);
		nf2.Notify(Dialog);	
		Dialog->m_SearchOutput.SetID(0); //没有更多结果
	}else{
		CNotifyDialogState nf2(NOTIFY_FIND_VIEW);
		nf2.PushInt(FIND_STOP);
        nf2.PushInt(FALSE);
		nf2.Notify(Dialog);	
		Dialog->m_SearchOutput.SetID(1); //还有更多结果
	}
	
    return true;	
};
bool  CInterInstinct::DoFindObject(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	
	tstring text;
	if(m_Param.Size()){
		text = m_Param.PopString();
	}
	else if (!ExePipe.HasTypeAB(PARAM_TYPE1(TYPE_STRING)))
	{
		return ExeError(ExePipe,_T("parameter is invalid"));
	}else{
		text = ExePipe.PopString();
	}
	
	CNotifyDialogState nf(NOTIFY_FIND_VIEW);
	nf.PushInt(FIND_START);
	nf.Notify(Dialog);
	
	CLogicThread* Think = Dialog->GetThink();
	Dialog->FindFirst(text,FIND_OBJECT);
	
	ePipeline SearchResult;
	Dialog->FindContinue(Think,0,SearchResult);
	
	Dialog->m_SearchOutput = SearchResult;
	
	//输出结果,0条目也要通知
	CNotifyDialogState nf1(NOTIFY_FIND_VIEW);
	nf1.PushInt(FIND_UPDATE);
	nf1.PushPipe(SearchResult);
	nf1.Notify(Dialog);
	
	
	//如果已经找到所有结果,通知搜索结束
	if (Dialog->m_FindSeedList.size()==0)
	{
		CNotifyDialogState nf2(NOTIFY_FIND_VIEW);
		nf2.PushInt(FIND_STOP);
		nf2.PushInt(TRUE);
		nf2.Notify(Dialog);	
		Dialog->m_SearchOutput.SetID(0); //没有更多结果
	}else{
		CNotifyDialogState nf2(NOTIFY_FIND_VIEW);
		nf2.PushInt(FIND_STOP);
        nf2.PushInt(FALSE);
		nf2.Notify(Dialog);	
		Dialog->m_SearchOutput.SetID(1); //还有更多结果
	}
	
    return true;			
};

bool  CInterInstinct::DoSetGlobleLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	return true;	

};

bool  CInterInstinct::DoFindSetStartTime(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	return true;	

};
bool  CInterInstinct::DoFindSetEndTime(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	return true;
};
bool  CInterInstinct::DoSetFindPricision(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	int64 Pricision = m_Param.PopInt();
    Dialog->SetFindFocusSize(Pricision);
    return TRUE;
};

bool  CInterInstinct::DoLearnWord(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){

	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if (!ExePipe.HasTypeAB(PARAM_TYPE2(TYPE_STRING,TYPE_STRING)))
	{
		return ExeError(ExePipe,_T("parameter is invalid"));
	}

	tstring Word = ExePipe.PopString();
	tstring PartofSpeech = ExePipe.PopString();
	
	Word = TriToken(Word);
	PartofSpeech = TriToken(PartofSpeech);

	int32 Type = 0;
	if(PartofSpeech ==  _T("token"))
		Type = MEMORY_LAN;
	else if(PartofSpeech ==  _T("pronoun"))
		Type = MEMORY_PRONOUN;
	else if(PartofSpeech == _T("adjective"))
		Type = MEMORY_ADJECTIVE;
	else if(PartofSpeech == _T("numeral"))
		Type = MEMORY_NUMERAL;
	else if(PartofSpeech ==  _T("verb"))
		Type = MEMORY_VERB;
	else if(PartofSpeech ==  _T("adverb"))
		Type = MEMORY_ADVERB;
	else if(PartofSpeech == _T("article"))
		Type = MEMORY_ARTICLE;
	else if(PartofSpeech ==  _T("preposition"))
		Type = MEMORY_PREPOSITION;
	else if(PartofSpeech == _T("conjunction"))
		Type = MEMORY_CONJUNCTION;
	else if(PartofSpeech == _T("interjection"))
		Type = MEMORY_INTERJECTION;
	else if(PartofSpeech ==  _T("noun"))
		Type = MEMORY_NOUN;
	else {
		return ExeError(ExePipe,_T("Learn fail: the part of speech is invalid"));
	}
	int64 ID = Dialog->GetThink()->CheckWord(Word,Type,MEANING_SENSE_OK);
	if(ID!=0){
		tstring s = Format1024(_T("The word \"%s\" has been learned"),Word.c_str());
		Dialog->RuntimeOutput(s);										
        return true;
	} 

	ID = Dialog->GetThink()->LearnWord(Word,Type,MEANING_SENSE_OK);
	
	if(ID==0){
		return ExeError(ExePipe,Format1024(_T("Learn word \"%s\" fail"),Word.c_str()));
	}
	return true;	

};

