// BrainObject.cpp: implementation of the CBrainObject class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "BrainObject.h"
#include "..\Brain.h"
#include "..\GUIMsgDefine.h"
#include "..\InstinctDefine.h"
#include "..\LogicDialog.h"
//#include "Arm.h"

//////////////////////////////////////////////////////////////////////////

CInterBrainObject::CInterBrainObject()
:CElement(0,_T("BrainObject")){

};

CInterBrainObject::CInterBrainObject(int64 ID,tstring Name,int64 InstinctID,ePipeline& Param)
:CElement(ID,Name.c_str()),m_InstinctID(InstinctID)
{
	m_Param << Param;

}

CInterBrainObject::~CInterBrainObject(){

}


bool  CInterBrainObject::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg)
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
	case INSTINCT_SET_GLOBLELOGIC:
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
	case INSTINCT_REMOVE_TEMP_LOGIC:
		ret = DoRemoveTempLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_CLEAR_TEMP_LOGIC:
		ret = DoClearTempLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_TOKEN:
	case INSTINCT_LEARN_PRONOUN:
	case INSTINCT_LEARN_ADJECTIVE:
	case INSTINCT_LEARN_NUMERAL:
	case INSTINCT_LEARN_VERB:
	case INSTINCT_LEARN_ADVERB:
	case INSTINCT_LEARN_ARTICLE:
	case INSTINCT_LEARN_PREPOSITION:
	case INSTINCT_LEARN_CONJUNCTION:
	case INSTINCT_LEARN_INTERJECTION:
	case INSTINCT_LEARN_NOUN:
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
	case INSTINCT_FIND_SET_STARTTIME:
		break;
	case INSTINCT_FIND_SET_ENDTIME:
		break;
	case INSTINCT_FIND_SET_PRICISION:
		ret = DoSetFindPricision(Dialog,ExePipe,LocalAddress,Msg);
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

bool  CInterBrainObject::DoCloseDialog(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	TASK_STATE  State = Dialog->GetTaskState();
	CBrain* Brain = Dialog->m_Brain;
	if (Dialog->m_DialogType == DIALOG_SYSTEM)
	{
		return true;
	}
	if (Dialog->m_DialogType == DIALOG_TASK)
	{
		CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
		nf.PushInt(DL_DEL_DIALOG);
		nf.Notify(Dialog);

		Brain->GetBrainData()->DeleteDialog(Dialog->m_SourceID,Dialog->m_DialogID);

		return true;
	}
	return true;
}

bool  CInterBrainObject::DoCreateAccount(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
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
		ExePipe.GetLabel() =_T("this dialog not authorized to perform this operation");
		ExePipe.SetID(RETURN_ERROR);
		return false;
	}
		
	if (!ExePipe.HasTypeAB(0x33100000))
	{
		ExePipe.GetLabel() =_T("parameter is not valid");
		ExePipe.SetID(RETURN_ERROR);
		return false;
	}
	tstring Name = ExePipe.PopString();
	tstring Password = ExePipe.PopString();
	int64   DialogType = ExePipe.PopInt();

	tstring CrypStr = Name + Password;

	ePipeline UserInfo;
	if(Dialog->m_Brain->GetBrainData()->FindRegisterUserInfo(Name,CrypStr,UserInfo)){
		tstring s = Format1024(_T("User Name '%s' has exist"),Name.c_str());
		ExePipe.GetLabel() = s;
		ExePipe.SetID(RETURN_ERROR);
		return false;
	};

	if (DialogType ==DIALOG_SYSTEM)
	{
		Dialog->m_Brain->GetBrainData()->RegisterUserAccount(Name,CrypStr,DIALOG_SYSTEM);
	}else{
		ExePipe.GetLabel() =_T("DialogType is not valid");
		ExePipe.SetID(RETURN_ERROR);
		return false;
	}
	return true;
};
bool  CInterBrainObject::DoDeleteAccount(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
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
		ExePipe.GetLabel() =_T("this dialog not authorized to perform this operation");
		ExePipe.SetID(RETURN_ERROR);
		return false;
	}

	if (!ExePipe.HasTypeAB(0x33000000))
	{
		ExePipe.GetLabel() =_T("parameter is not valid");
		ExePipe.SetID(RETURN_ERROR);
		return false;
	}
	tstring Name     = ExePipe.PopString();
	tstring Password = ExePipe.PopString();
	tstring CrypStr = Name + Password;
	int64 SourceID  = Dialog->m_Brain->GetBrainData()->DeleteUserAccount(Name,CrypStr);
	if (SourceID>0)
	{
		int32 n = Dialog->m_Brain->GetBrainData()->DeleteDialogOfSource(SourceID);
	}
	return true;
};
bool  CInterBrainObject::DoSetLogicAddress(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}


	if(Dialog->m_DialogType != DIALOG_EVENT){
		ExePipe.GetLabel() =_T("'set logic address' command is available in execution mode");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}

	CLogicDialog* ParentDlg = Dialog->m_Brain->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		ExePipe.GetLabel() =_T("not find parent dialog");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}
	assert(ParentDlg->m_TaskID);

	if(ExePipe.Size()==0){ //设置空地址
		ParentDlg->m_LogicAddress.Clear();
		ParentDlg->m_LogicAddress.SetID(0);
		return true;
	}

	if (!ExePipe.HasTypeAB(0x10000000))
	{
		ExePipe.GetLabel() =_T("parameter is not valid");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}
	int64 ItemID = ExePipe.PopInt();

	CLogicTask* Task = ParentDlg->GetTask();

	ePipeline LogicAddress(ItemID);
	bool ret = Task->FindLogicAddress(ItemID,LogicAddress);
	if(!ret){
		ExePipe.GetLabel() = Format1024(_T("not find the item id=%I64ld"),ItemID);
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}
	ParentDlg->m_LogicAddress = LogicAddress;


	return true;
}	
bool  CInterBrainObject::DoGetLogicAddress(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	if(Dialog->m_DialogType == DIALOG_EVENT){
		CLogicDialog* ParentDlg = Dialog->m_Brain->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
		if (ParentDlg==NULL)
		{
			ExePipe.GetLabel() =_T("not find parent dialog");
			ExePipe.SetID(RETURN_ERROR);
			return true;
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

bool  CInterBrainObject::DoSetLogicBreakpoint(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	if (!ExePipe.HasTypeAB(0x10000000))
	{
		ExePipe.GetLabel() =_T("parameter is not valid");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}

	int64 bBreak = ExePipe.PopInt();

	assert(Dialog->m_DialogType == DIALOG_EVENT);
	CLogicDialog* ParentDlg = Dialog->m_Brain->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		ExePipe.GetLabel() =_T("not find parent dialog");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}

	ePipeline LogicAddress = ParentDlg->m_LogicAddress;

	CMsg Msg1(Dialog->m_SourceID,LogicAddress,MSG_ELT_REMOVE_LOGIC,DEFAULT_DIALOG,0);
	ePipeline& Letter = Msg1.GetLetter();
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_DEBUG_BREAK);
	Letter.PushInt(bBreak);
	Letter.PushPipe(LogicAddress);

	Dialog->m_Brain->PushNerveMsg(Msg1,false,false);

	return true;
}
bool  CInterBrainObject::DoRemoveLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	assert(Dialog->m_DialogType == DIALOG_EVENT);
	CLogicDialog* ParentDlg = Dialog->m_Brain->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		ExePipe.GetLabel() =_T("not find parent dialog");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}

	ePipeline LogicAddress = ParentDlg->m_LogicAddress;

	CMsg EltMsg(Dialog->m_SourceID,LogicAddress,MSG_ELT_REMOVE_LOGIC,DEFAULT_DIALOG,0);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushInt(LogicAddress.GetID());

	Dialog->m_Brain->PushNerveMsg(EltMsg,false,false);
	return true;

};

bool  CInterBrainObject::DoInsertLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){

	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	if (!ExePipe.HasTypeAB(0x30000000))
	{
		ExePipe.GetLabel() =_T("parameter is not valid");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}
	tstring LogicName = ExePipe.PopString();

	assert(Dialog->m_DialogType == DIALOG_EVENT);
	CLogicDialog* ParentDlg = Dialog->m_Brain->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		ExePipe.GetLabel() =_T("not find parent dialog");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}

	ePipeline LogicAddress = ParentDlg->m_LogicAddress;

	CMsg EltMsg(Dialog->m_SourceID,LogicAddress,MSG_ELT_INSERT_LOGIC,DEFAULT_DIALOG,0);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushString(LogicName);

	Dialog->m_Brain->PushNerveMsg(EltMsg,false,false);
	return true;
};

bool  CInterBrainObject::DoRemoveTempLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	if (!ExePipe.HasTypeAB(0x30000000))
	{
		ExePipe.GetLabel() =_T("parameter is not valid");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}
	tstring LogicName = ExePipe.PopString();

	assert(Dialog->m_DialogType == DIALOG_EVENT);
	CLogicDialog* ParentDlg = Dialog->m_Brain->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		ExePipe.GetLabel() =_T("not find parent dialog");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}

	ParentDlg->DeleteLogic(LogicName);
	return true;
}
bool  CInterBrainObject::DoClearTempLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	assert(Dialog->m_DialogType == DIALOG_EVENT);
	CLogicDialog* ParentDlg = Dialog->m_Brain->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
	if (ParentDlg==NULL)
	{
		ExePipe.GetLabel() =_T("not find parent dialog");
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}
	
	ParentDlg->ClearLogicSence();
	return true;
}	

bool  CInterBrainObject::DoThinkLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){	
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	Dialog->m_CurLogicName = m_Param.PopString();
	Dialog->SetWorkMode(WORK_THINK);
	return true;
}


bool  CInterBrainObject::DoDebugTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
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

bool CInterBrainObject::DoTestTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
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
bool  CInterBrainObject::DoStopTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
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
	bool ret = Dialog->m_Brain->GetBrainData()->GetEvent(EventID,EnventInfo,false);
	if (!ret) 
	{
		return true;
	}
	
	CMsg Msg1(SYSTEM_SOURCE,EnventInfo.m_ClientAddress,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = Msg1.GetLetter();
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_STOP);

	Dialog->m_Brain->PushNerveMsg(Msg1,false,false);


	//CMsg EltMsg(Dialog->m_SourceID,ObjectInfo.m_ClientAddress,MSG_ELT_TASK_CTRL,DEFAULT_DIALOG,0);
	//ePipeline& Letter = EltMsg.GetLetter();
	//Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_STOP);
	//Letter.PushPipe(ObjectInfo.m_ClientExePipe);
	//			
	////给Element发信息
	//Dialog->m_Brain->PushNerveMsg(EltMsg,false,false);

	return true;	
};
bool  CInterBrainObject::DoPauseTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){

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
	bool ret = Dialog->m_Brain->GetBrainData()->GetEvent(EventID,EnventInfo,false);
	if (!ret) 
	{
		return true;
	}


	CMsg Msg1(SYSTEM_SOURCE,EnventInfo.m_ClientAddress,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = Msg1.GetLetter();
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_PAUSE);

	Dialog->m_Brain->PushNerveMsg(Msg1,false,false);

    return true;
};

bool  CInterBrainObject::DoRunTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	CLogicDialog* ParentDialg = Dialog->m_Brain->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);

	assert(ParentDialg);
	if (!ParentDialg)
	{
		return false;
	}

	if(ParentDialg->GetControlEventID() == Dialog->m_DialogID) //用户在缺省的控制对话中输入"run"，意味着将重启全部暂停
	{

		CMsg Msg1(SYSTEM_SOURCE,ParentDialg->m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,GetEventID());
		ePipeline& Letter = Msg1.GetLetter();
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN);
		Letter.PushInt(0);

		ParentDialg->Do(Msg1);

	}
	else{ //意味着用户在暂停对话中输入了"run"命令，这时候可以直接取得事件
		int64 EventID = Dialog->m_DialogID;
		CBrainEvent ObjectInfo;
		bool ret = Dialog->m_Brain->GetBrainData()->GetEvent(EventID,ObjectInfo,true);
		if (!ret)
		{
			return true;
		}
		
		CMsg EltMsg(Dialog->m_SourceID,ObjectInfo.m_ClientAddress,MSG_ELT_TASK_CTRL,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = EltMsg.GetLetter();
		Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN);
		Letter.PushPipe(ObjectInfo.m_ClientExePipe);
		
		//给Element发信息
		Dialog->m_Brain->PushNerveMsg(EltMsg,false,false);
	}

	 return true;
};

bool  CInterBrainObject::DoStepTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
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
	bool ret = Dialog->m_Brain->GetBrainData()->GetEvent(EventID,EnventInfo,false);
	if (!ret) 
	{
		return true;
	}


	CMsg Msg1(SYSTEM_SOURCE,EnventInfo.m_ClientAddress,MSG_FROM_BRAIN,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = Msg1.GetLetter();
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_DEBUG_STEP);
	Letter.PushInt(0);

	Dialog->m_Brain->PushNerveMsg(Msg1,false,false);
   
	return true;		
};

bool  CInterBrainObject::DoGotoTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
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

bool  CInterBrainObject::DoLearnText(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	if (!Msg.IsReaded())
	{
		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_TASK_RESULT)
		{		
			ePipeline& Letter = Msg.GetLetter();
			ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
			
			if (!NewExePipe->IsAlive())
			{
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}
			
			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}
			
			
			int64 ThinkID = NewExePipe->PopInt();
			CLogicThread* Think = Dialog->m_Brain->GetBrainData()->GetLogicThread(ThinkID);
			assert(Think);
			tstring Text = Think->GetUserInput();
			
			if(Text.size() == 0){ 
				//再次压入事件，要求用户重新输入		
				tstring DialogText = _T("Please input text.");
	
				UpdateEventID();
				CLogicDialog*  Dlg = Dialog->StartEventDialog(GetEventID(),_T("Ask"),DialogText,TASK_OUT_THINK,*OldExePipe,LocalAddress,TIME_SEC,true,true,true);				
				if (Dlg==NULL)
				{
					tstring Answer = _T("Start child dialog fail.");
					ExePipe.SetLabel(Answer.c_str());
					ExePipe.SetID(RETURN_ERROR);
					return true;
				}
				
				ExePipe.SetID(OldExePipe->GetID());
				
				return true;
			}
			
			int64 ID = Dialog->GetThink()->LearnText(Text);
			
			if(ID==0){
				ExePipe.SetLabel(_T("Learn text fail."));
				ExePipe.SetID(RETURN_ERROR);
				return true;
			}
			Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.Clear();
			ExePipe<<*OldExePipe;
			
			return true;
		}else{
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			if(ret == RETURN_DIRECTLY){
				return true;
			}
		}
	} 
	
	
	tstring Text = m_Param.PopString();
	TriToken(Text);

	if(Text.size() == 0 || Text == _T(";") || Text==_T("；")){ //要求输入字符传
		
		tstring DialogText= _T("Please input  text.");
		UpdateEventID();
		CLogicDialog*  Dlg = Dialog->StartEventDialog(GetEventID(),_T("Input Dialog"),DialogText,TASK_OUT_THINK,ExePipe,LocalAddress,TIME_SEC,true,true,true);
		if (Dlg == NULL)
		{
			tstring Answer = _T("Start child dialog fail.");
			ExePipe.SetLabel(Answer.c_str());
			ExePipe.SetID(RETURN_ERROR);
		}
		
		return true;
	}
	
	int64 ID = Dialog->GetThink()->LearnText(Text);
	
	if(ID==0){
		ExePipe.SetLabel(_T("Learn text fail."));
		ExePipe.SetID(RETURN_ERROR);
	}
	return true;	
};
bool  CInterBrainObject::DoLearnLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	if (!Msg.IsReaded())
	{	
		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_TASK_RESULT)
		{		
			ePipeline& Letter = Msg.GetLetter();
			ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
			
			if (!NewExePipe->IsAlive())
			{
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}
			
			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}
			
			
			int64 ThinkID = NewExePipe->PopInt();
			CLogicThread* Think = Dialog->m_Brain->GetBrainData()->GetLogicThread(ThinkID);
			assert(Think);
			tstring Text = Think->GetUserInput();
			
			TriToken(Text);

			if(Text.size() == 0 || Text == _T(";") || Text==_T("；")){ //忽略
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				return true;
			}
			
			int64 LogicID = m_Param.GetID();
			int64 ID = Think->CommentMemory(Text,LogicID);		
			if(ID ==0){
				tstring Answer = Format1024(_T("learn logic memo fail"));
				Dialog->RuntimeOutput(Answer);
			}
			Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
			
			ExePipe.Clear();
			ExePipe<<*OldExePipe;
			
			return true;
		}else{
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			if(ret == RETURN_DIRECTLY){
				return true;
			}
		}
	}

	tstring LogicName = m_Param.PopString();
	LogicName = TriToken(LogicName);
	
	CLocalLogicCell* Logic = Dialog->FindLogic(LogicName);
	
	if(Logic == NULL){		
		tstring Answer = Format1024(_T("no find logic \"%s\"."), LogicName.c_str());
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}
	
	
	//预先记忆逻辑体
	ePipeline LogicData = Logic->m_Task.m_LogicData;
	
	CLogicThread* Think = Dialog->GetThink();
	int64 LogicID = Think->LearnLogic(&LogicData);
	if(LogicID==0){
		tstring Answer = Format1024(_T("memory logic \"%s\"."), LogicName.c_str());
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.SetID(RETURN_ERROR);
		return true;	
	}
	
	m_Param.SetID(LogicID);
	
	tstring DialogText = _T("Please input or edit logic memo: (or empty)");
	UpdateEventID();
	CLogicDialog*  Dlg = Dialog->StartEventDialog(GetEventID(),_T("Input Dialog"),DialogText,TASK_OUT_THINK,ExePipe,LocalAddress,TIME_SEC,true,true,true);
	if (Dlg==NULL)
	{
		tstring Answer = _T("Start child dialog fail.");
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.SetID(RETURN_ERROR);
	}
	return true;
};
bool  CInterBrainObject::DoLearnObject(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	

	if (!Msg.IsReaded())
	{	
		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_TASK_RESULT)
		{		
			ePipeline& Letter = Msg.GetLetter();
			ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
			
			if (!NewExePipe->IsAlive())
			{
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}
			
			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}
			
			
			int64 ThinkID = NewExePipe->PopInt();
			CLogicThread* Think = Dialog->m_Brain->GetBrainData()->GetLogicThread(ThinkID);
			assert(Think);
			tstring Text = Think->GetUserInput();
			
			TriToken(Text);

			if(Text.size() == 0 || Text == _T(";") || Text==_T("；")){ //忽略

				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				return true;
			}
			
			int64 ObjectID = m_Param.GetID();
			int64 ID = Think->CommentMemory(Text,ObjectID);		
			if(ID ==0){
				tstring Answer = Format1024(_T("learn object memo fail"));
				Dialog->RuntimeOutput(Answer);
			}
			Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
			
			ExePipe.Clear();
			ExePipe<<*OldExePipe;
			
			return true;
		}else{
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			if(ret == RETURN_DIRECTLY){
				return true;
			}
	   }
	}		
	
	tstring Answer = Format1024(_T("This command is currently unavailable"));
	ExePipe.SetLabel(Answer.c_str());
	ExePipe.SetID(RETURN_ERROR);
	
	return false;	
	
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
		int64 FatherID = GetBrain()->m_WorldShow->GetRoomID();
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
bool  CInterBrainObject::DoLearnAction(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	if (!Msg.IsReaded())
	{		
		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_TASK_RESULT)
		{		
			int64 ID = m_Param.GetID();
			if(ID == 1){
				
				ePipeline& Letter = Msg.GetLetter();
				ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
				ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
				
				if (!NewExePipe->IsAlive())
				{
					Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
					ExePipe.Break();
					return true;
				}
				
				int64 retTask = NewExePipe->GetID();
				if (retTask == RETURN_ERROR)
				{
					Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
					ExePipe.Break();
					ExePipe.SetID(retTask);
					ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
					return true;
				}
				
				int64 ThinkID = NewExePipe->PopInt();
				CLogicThread* Think = Dialog->m_Brain->GetBrainData()->GetLogicThread(ThinkID);
				assert(Think);
				tstring ClauseText = Think->GetUserInput();
				
				int64 LogicID = m_Param.PopInt();
				ID = Think->LearnAction(Dialog,ClauseText,LogicID);
				if (ID == 0)
				{
					tstring Answer = Format1024(_T("Learn action fail: %s"),ClauseText.c_str());
					ExePipe.SetLabel(Answer.c_str());
					ExePipe.SetID(RETURN_ERROR);
					Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
					return true;	
				}
				
				
				m_Param.Clear();
				m_Param.PushInt(ID);
				m_Param.SetID(2);
				
				tstring DialogText = _T("Please input action memo : (or empty)");
				UpdateEventID();
				CLogicDialog*  Dlg = Dialog->StartEventDialog(GetEventID(),_T("Input Dialog"),DialogText,TASK_OUT_THINK,*OldExePipe,LocalAddress,TIME_SEC,true,true,true);
				if (Dlg==NULL)
				{
					tstring Answer = _T("Start child dialog fail.");
					ExePipe.SetLabel(Answer.c_str());
					ExePipe.SetID(RETURN_ERROR);
				}
				return true;

			}else if(ID == 2) 
			{
				ePipeline& Letter = Msg.GetLetter();
				ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
				ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
				
				
				if (!NewExePipe->IsAlive())
				{
					Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
					ExePipe.Break();
					return true;
				}
				
				int64 retTask = NewExePipe->GetID();
				if (retTask == RETURN_ERROR)
				{
					Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
					ExePipe.Break();
					ExePipe.SetID(retTask);
					ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
					return true;
				}
				
				
				int64 ThinkID = NewExePipe->PopInt();
				CLogicThread* Think = Dialog->m_Brain->GetBrainData()->GetLogicThread(ThinkID);
				assert(Think);
				tstring Text = Think->GetUserInput();
				
				int64 ActionID = m_Param.PopInt();
				if (Text.size())
				{
					ID = Think->CommentMemory(Text,ActionID);		
					if(ID ==0){
						tstring Answer = Format1024(_T("learn action memo fail"));
						Dialog->RuntimeOutput(Answer);
					}
				}
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				
				ExePipe.Clear();
				ExePipe<<*OldExePipe;
				
			}
		}else{
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			if(ret == RETURN_DIRECTLY){
				return true;
			}
		}
		
	}


	//预先记忆逻辑体
	tstring LogicName = m_Param.PopString();
	CLocalLogicCell* Logic = Dialog->FindLogic(LogicName);

	if(Logic == NULL){		
		tstring Answer = Format1024(_T("no find logic \"%s\"."), LogicName.c_str());
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}


	//预先记忆逻辑体
	ePipeline LogicData = Logic->m_Task.m_LogicData;
	
	CLogicThread* Think = Dialog->GetThink();
	
	int64 LogicID = Think->LearnLogic(&LogicData);
	
	if(LogicID==0){
		tstring Answer = _T("Learn action fail.");
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.SetID(RETURN_ERROR);
		return true;		
	}
	
	
	m_Param.PushInt(LogicID);
	m_Param.SetID(1);

	UpdateEventID();
	tstring DialogText= _T("Please input command text:");
	CLogicDialog*  Dlg = Dialog->StartEventDialog(GetEventID(),_T("Input Dialog"),DialogText,TASK_OUT_THINK,ExePipe,LocalAddress,TIME_SEC,true,true,true);
	if (Dlg==NULL)
	{
		tstring Answer = _T("Start child dialog fail.");
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.SetID(RETURN_ERROR);
	}
	return true;	
};
bool  CInterBrainObject::DoLearnMemory(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{	
		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_TASK_RESULT)
		{		
			ePipeline& Letter = Msg.GetLetter();
			ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);

			if (!NewExePipe->IsAlive())
			{
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}

			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}


			int64 ThinkID = NewExePipe->PopInt();
			CLogicThread* Think = Dialog->m_Brain->GetBrainData()->GetLogicThread(ThinkID);
			assert(Think);

			tstring Text = Think->GetUserInput();
			Text = TriToken(Text);

			CToken* Token = Think->JustOneToken();

			if (Text.size()==0 || !Token)
			{
				//再次压入事件，要求用户重新输入
				tstring DialogText = _T("Input error: not a token,Please input again.");
				UpdateEventID();
				CLogicDialog*  Dlg= Dialog->StartEventDialog(GetEventID(),_T("Input word"),DialogText,TASK_OUT_THINK,*OldExePipe,LocalAddress,TIME_SEC,true,true,true);				
				if (Dlg==NULL)
				{
					tstring Answer = _T("Start child dialog fail.");
					ExePipe.SetLabel(Answer.c_str());
					ExePipe.SetID(RETURN_ERROR);
					return CONTINUE_TASK;
				}

				ExePipe.SetID(OldExePipe->GetID());

			}else {
				ExePipe.Clear();
				ExePipe<<*OldExePipe;
				
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

	CLogicThread* Think = Dialog->GetThink();
	/*
	tstring Word = m_Param.PopString();
	int64 WordID = Think->CheckWord(Word);

	if(WordID == NULL){		
		tstring Answer = tformat(_T("not understand  word \"%s\"."), Word.c_str());
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.SetID(RETURN_ERROR);
		return true;
	}

	m_Param.SetID(LogicID);

	tstring DialogText = _T("Please input new word: ");

	UpdateEventID();

	bool ret = Dialog->StartChildDialog(GetEventID(),_T("Input word"),DialogText,TASK_OUT_THINK,ExePipe,LocalAddress);
	if (!ret)
	{
		tstring Answer = _T("Start child dialog fail.");
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.SetID(RETURN_ERROR);
	}*/
	return true;
}
bool  CInterBrainObject::DoFind(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	tstring text = m_Param.PopString();
	
	
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

bool  CInterBrainObject::DoFindLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	tstring text = m_Param.PopString();
	
	
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
bool  CInterBrainObject::DoFindObject(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	
	tstring text = m_Param.PopString();
	
	
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

bool  CInterBrainObject::DoSetGlobleLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
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

bool  CInterBrainObject::DoFindSetStartTime(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
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
bool  CInterBrainObject::DoFindSetEndTime(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
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
bool  CInterBrainObject::DoSetFindPricision(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	int64 Pricision = m_Param.PopInt();
    Dialog->SetFindCellSize(Pricision);
    return TRUE;
};

bool  CInterBrainObject::DoLearnWord(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){

	if (!Msg.IsReaded())
	{
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}

	tstring Word = m_Param.PopString();
	TriToken(Word);

	int32 Type = 0;
	switch(m_InstinctID){
	case INSTINCT_LEARN_TOKEN:
		Type = MEMORY_LAN;
		break;
	case INSTINCT_LEARN_PRONOUN:
		Type = MEMORY_PRONOUN;
		break;
	case INSTINCT_LEARN_ADJECTIVE:
		Type = MEMORY_ADJECTIVE;
		break;
	case INSTINCT_LEARN_NUMERAL:
		Type = MEMORY_NUMERAL;
		break;
	case INSTINCT_LEARN_VERB:
		Type = MEMORY_VERB;
		break;
	case INSTINCT_LEARN_ADVERB:
		Type = MEMORY_ADVERB;
		break;
	case INSTINCT_LEARN_ARTICLE:
		Type = MEMORY_ARTICLE;
		break;
	case INSTINCT_LEARN_PREPOSITION:
		Type = MEMORY_PREPOSITION;
		break;
	case INSTINCT_LEARN_CONJUNCTION:
		Type = MEMORY_CONJUNCTION;
		break;
	case INSTINCT_LEARN_INTERJECTION:
		Type = MEMORY_INTERJECTION;
		break;
	case INSTINCT_LEARN_NOUN:
		Type = MEMORY_NOUN;
		break;
	default:
		Dialog->RuntimeOutput(0,_T("Learn fail: interal error"));
		return false;
	}

	int64 ID = Dialog->GetThink()->CheckWord(Word,Type,MEANING_SENSE_OK);
	if(ID!=0){
		tstring s = Format1024(_T("The word \"%s\" has been learned"),Word.c_str());
		Dialog->RuntimeOutput(s);										
        return true;
	} 

	ID = Dialog->GetThink()->LearnWord(Word,Type,MEANING_SENSE_OK);
	
	if(ID==0){
		tstring s = Format1024(_T("Learn word \"%s\" fail"),Word.c_str());
		ExePipe.SetLabel(s.c_str());
		ExePipe.Break();
		ExePipe.SetID(RETURN_ERROR);
		return false;	
	}
	return true;	

};


/*
CUseArm::CUseArm(int64 ID,CArm& Arm)
	:CElement(ID,_T("Arm")),m_Arm(Arm)
{

};
CUseArm::~CUseArm()
{

};

bool CUseArm::TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress){

	return m_Arm.Do(Dialog,&ExePipe);
}
*/
