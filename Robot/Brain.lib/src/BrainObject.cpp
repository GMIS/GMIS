// BrainObject.cpp: implementation of the CBrainObject class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)
#include "Brain.h"
#include <winsock2.h>
#include "BrainMemory.h"
#include "BrainObject.h"
#include "LogicDialog.h"
#include "UserLinkerPipe.h"
#include "LogicThread.h"
#include "GUIMsgDefine.h"
//#include "Arm.h"
//////////////////////////////////////////////////////////////////////////

MsgProcState CWaitSecond_Static::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_EVENT_TICK)
	{	
		if(!ExePipe.IsAlive()){
			CBrainEvent EventInfo;
			bool ret = Dialog->m_Brain->GetBrainData()->PopBrainEvent(GetEventID(),EventInfo);
			if (!ret) 
			{
				ExePipe.GetLabel() = Format1024(_T("Error: (%I64ld)%s event lost"),m_ID,GetName().c_str());
				ExePipe.SetID(RETURN_ERROR);
				return RETURN_DIRECTLY;
			}

			ePipeline& OldExePipe = EventInfo.m_ClientExePipe;
			
			Dialog->CloseChildDialog(GetEventID(),OldExePipe,ExePipe);
			
			return CONTINUE_TASK;
		}
		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID()) //调试暂停后也会持续收到此信息
		{
			ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			return ret;
		}

		
		ePipeline& Letter = Msg.GetLetter();
		
		int64 TimeStamp = Letter.PopInt();
		
		float64 t = (float64)(TimeStamp-m_StartTimeStamp);
		
		t /=(float64)(TIME_SEC);

		
		if(t<m_Second){
			Dialog->m_Brain->GetBrainData()->ResetEventTickCount(EventID);
			if (ExePipe.GetID() == RETURN_BREAK)
			{
				m_bPause = true; //等待期间的收到暂停信号不处理，直到等待时间结束
			}

			ExePipe.SetID(RETURN_WAIT); 
		}else{
			
			Dialog->m_Brain->OutputLog(LOG_MSG_RUNTIME_TIP,_T("%I64ld waited %.3f second"),m_ID,t);

			CBrainEvent EventInfo;
			bool ret = Dialog->m_Brain->GetBrainData()->PopBrainEvent(EventID,EventInfo);
			if (!ret) 
			{
				ExePipe.GetLabel() = Format1024(_T("Error: (%I64ld)%s event lost"),m_ID,GetName().c_str());
				ExePipe.SetID(RETURN_ERROR);
				return RETURN_DIRECTLY;
			}
			ePipeline& OldExePipe = EventInfo.m_ClientExePipe;
			
			ExePipe.Clear();
			ExePipe<<OldExePipe;
			
			if(m_bPause){
				ExePipe.SetID(RETURN_BREAK);
				m_bPause = false;
			}
			
			Dialog->CloseChildDialog(GetEventID(),OldExePipe,ExePipe);
		}
	}else{

		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
		
	}
	return RETURN_DIRECTLY;
}

bool CWaitSecond_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	m_StartTimeStamp = AbstractSpace::CreateTimeStamp();
	
	UpdateEventID();
	
	tstring DialogTitle = Format1024(_T("Wait %.1f second"),m_Second);
	Dialog->StartChildDialog(GetEventID(),DialogTitle,_T("Please wait timer ends"),TASK_OUT_DEFAULT,ExePipe,LocalAddress,MIN_EVENT_INTERVAL,false,false);
	
	return true;
}


bool CWaitSecond::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	m_StartTimeStamp = AbstractSpace::CreateTimeStamp();

	if (ExePipe.Size()==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: Wait second lost param"));
		return false;
	}
	eElectron E;
	ExePipe.Pop(&E);

	if (E.EnergyType() == TYPE_INT)
	{
		m_Second = E.Int64();
	}else if(E.EnergyType() == TYPE_FLOAT){
		m_Second = E.Float64();
	}else{
		ExePipe.GetLabel() = Format1024(_T("Error: Wait second  param must be int or float"));
		return false;
	}

	UpdateEventID();

	tstring DialogTitle = Format1024(_T("Wait %.1f second"),m_Second);
	Dialog->StartChildDialog(GetEventID(),DialogTitle,_T("Please wait timer ends"),TASK_OUT_DEFAULT,ExePipe,LocalAddress,MIN_EVENT_INTERVAL,false,false);

	return true;
}
//////////////////////////////////////////////////////////////////////////

MsgProcState CGotoLabel::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_EVENT_TICK)
	{	

		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID()) //调试暂停后也会持续收到此信息
		{
			ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			return ret;
		}
		
		ePipeline& Letter = Msg.GetLetter(); //注意：虽然用不到，但必须使用一次，表明此信读过，避免随后被使用

		CBrainEvent ObjectInfo;
		bool ret = Dialog->m_Brain->GetBrainData()->PopBrainEvent(EventID,ObjectInfo);
		if (!ret) 
		{
			ExePipe.GetLabel() = Format1024(_T("Error: got label '%s' lost event"),m_GotoLabel.c_str());
			ExePipe.SetID(RETURN_ERROR);
			return RETURN_DIRECTLY;
		}
	

		ExePipe.Clear();
		ExePipe<<ObjectInfo.m_ClientExePipe;;

		int64 ID = ObjectInfo.m_ClientExePipe.GetID();
		if (ID == RETURN_BREAK || ExePipe.GetID()==RETURN_BREAK)
		{
			ExePipe.SetID(RETURN_DEBUG_GOTO_LABEL);
		} 
		else
		{
			ExePipe.SetID(RETURN_GOTO_LABEL);
			Dialog->SetTaskState(TASK_RUN);
			Dialog->NotifyTaskState();
		}
		
		ExePipe.SetLabel(m_GotoLabel.c_str());

		

	}else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
		
	}
	return RETURN_DIRECTLY;

}

bool CGotoLabel::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
		
	UpdateEventID();
	int64 EventID = GetEventID();

#ifdef _DEBUG
	tstring memo = Format1024(_T("%s %d"),_T(__FILE__),__LINE__);
	Dialog->m_Brain->GetBrainData()->PushBrainEvent(EventID,EventID,ExePipe,LocalAddress,MIN_EVENT_INTERVAL,false,memo); 
#else
	Dialog->m_Brain->GetBrainData()->PushBrainEvent(EventID,EventID,ExePipe,LocalAddress,MIN_EVENT_INTERVAL,false); 
#endif
	int64 ID = ExePipe.GetID();
	if (ID == RETURN_BREAK)
	{
		ExePipe.SetID(RETURN_DEBUG_WAIT);
	}else {
		ExePipe.SetID(RETURN_WAIT);
	}	
	return true;	
}


bool COutputInfo::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	
	tstring Info = ExePipe.PopString();
	Dialog->RuntimeOutput(m_ID,Info);
	return true;
};
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

	switch(m_InstinctID){
	case INSTINCT_THINK_LOGIC:
		DoThinkLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_RUN_TASK:
		DoRunTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_DEBUG_TASK: 
		DoDebugTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_STOP_TASK:
		DoStopTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_PAUSE_TASK:
		DoPauseTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_STEP_TASK:
		DoStepTask(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_GOTO_TASK:
		break;
	case INSTINCT_SET_GLOBLELOGIC:
		break;
	case INSTINCT_CLOSE_DIALOG:
		DoCloseDialog(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_CREATE_ACCOUNT:
		DoCreateAccount(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_DELETE_ACCOUNT:
		DoDeleteAccount(Dialog,ExePipe,LocalAddress,Msg);
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
		DoLearnWord(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_TEXT:
		DoLearnText(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_LOGIC:
		DoLearnLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_OBJECT:
		DoLearnObject(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_LEARN_ACTION:
		DoLearnAction(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_FIND_SET_STARTTIME:
		break;
	case INSTINCT_FIND_SET_ENDTIME:
		break;
	case INSTINCT_FIND_SET_PRICISION:
		DoSetFindPricision(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_FIND:
		DoFind(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_FIND_LOGIC:
		DoFindLogic(Dialog,ExePipe,LocalAddress,Msg);
		break;
	case INSTINCT_FIND_OBJECT:
		DoFindObject(Dialog,ExePipe,LocalAddress,Msg);
		break;
	default:
		assert(0);
		return false;	
	}	
	return true;
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
	if (Dialog->m_DialogType == DIALOG_SYSTEM_MAIN)
	{
		return true;
	}
	if (Dialog->m_DialogType == DIALOG_SYSTEM_CHILD || Dialog->m_DialogType == DIALOG_OTHER_CHILD)
	{
		CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
		nf.PushInt(DL_DEL_DIALOG);
		nf.Notify(Dialog);

		Brain->GetBrainData()->DeleteDialog(Dialog->m_SourceID,Dialog->m_DialogID);

		return true;
	}
	
	if (Dialog->m_DialogType == DIALOG_OTHER_MAIN)
	{
		CLinker Linker;
		Brain->GetLinker(Dialog->m_SourceID,Linker);
		Linker().Close();
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

	if ((Dialog->m_SourceID != LOCAL_GUI_SOURCE && Dialog->m_SourceID != SYSTEM_SOURCE) || (Dialog->m_DialogType != DIALOG_SYSTEM_MAIN && Dialog->m_DialogType != DIALOG_SYSTEM_CHILD ))
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

	if (DialogType > DIALOG_NULL && DialogType < DIALOG_OTHER_CHILD)
	{
		Dialog->m_Brain->GetBrainData()->RegisterUserAccount(Name,CrypStr,DIALOG_SYSTEM_MAIN);
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

	if (Dialog->m_SourceID != LOCAL_GUI_SOURCE && Dialog->m_DialogType != DIALOG_SYSTEM_MAIN)
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
	tstring CrypStr = Name + Password;
	int64 SourceID = Dialog->m_Brain->GetBrainData()->DeleteUserAccount(Name,CrypStr);
	if (SourceID>0)
	{
		int32 n = Dialog->m_Brain->GetBrainData()->DeleteDialogOfSource(SourceID);
	}
	return true;
};

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
	CBrainEvent ObjectInfo;
	bool ret = Dialog->m_Brain->GetBrainData()->PopBrainEvent(EventID,ObjectInfo);
	if (!ret) 
	{
		return true;
	}
				
	CMsg EltMsg(ObjectInfo.m_ClientAddress,MSG_ELT_TASK_CTRL,0);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushInt(CMD_STOP);
	Letter.PushPipe(ObjectInfo.m_ClientExePipe);
				
	//给Element发信息
	Dialog->m_Brain->PushNerveMsg(EltMsg,false,false);

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
	CBrainEvent ObjectInfo;
	bool ret = Dialog->m_Brain->GetBrainData()->PopBrainEvent(EventID,ObjectInfo);
	if (!ret) 
	{
		return true;
	}
	
	CMsg EltMsg(ObjectInfo.m_ClientAddress,MSG_ELT_TASK_CTRL,0);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushInt(CMD_PAUSE);
	Letter.PushPipe(ObjectInfo.m_ClientExePipe);
				
	//给Element发信息
	Dialog->m_Brain->PushNerveMsg(EltMsg,false,false);

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
		ePipeline ReceiverInfo;
		ReceiverInfo.PushInt(ParentDialg->m_DialogID);
		
		ePipeline ChildMsg(GUI_TASK_CONTROL);
		ChildMsg.PushInt(CMD_RUN);
		ChildMsg.PushInt(0);
		
		CMsg NewMsg;
		CreateBrainMsg(NewMsg,ReceiverInfo,ChildMsg,Msg.GetEventID());
		ParentDialg->Do(NewMsg);		
	}
	else{ //意味着用户在暂停对话中输入了"run"命令，这时候可以直接取得事件
		int64 EventID = Dialog->m_DialogID;
		CBrainEvent ObjectInfo;
		bool ret = Dialog->m_Brain->GetBrainData()->PopBrainEvent(EventID,ObjectInfo);
		if (!ret)
		{
			return true;
		}
		
		CMsg EltMsg(ObjectInfo.m_ClientAddress,MSG_ELT_TASK_CTRL,EventID);
		ePipeline& Letter = EltMsg.GetLetter();
		Letter.PushInt(CMD_RUN);
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
	CBrainEvent ObjectInfo;
	bool ret = Dialog->m_Brain->GetBrainData()->PopBrainEvent(EventID,ObjectInfo);
	if (!ret)
	{
		return true;
	}
								
	CMsg EltMsg(ObjectInfo.m_ClientAddress,MSG_ELT_TASK_CTRL,EventID);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushInt(CMD_DEBUG_STEP);
	Letter.PushPipe(ObjectInfo.m_ClientExePipe);
				
	//给Element发信息
	Dialog->m_Brain->PushNerveMsg(EltMsg,false,false);
   
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
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}
			
			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
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

				bool ret = Dialog->StartChildDialog(GetEventID(),_T("Ask"),DialogText,TASK_OUT_THINK,*OldExePipe,LocalAddress);				
				if (!ret)
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
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
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

		bool ret = Dialog->StartChildDialog(GetEventID(),_T("Input Dialog"),DialogText,TASK_OUT_THINK,ExePipe,LocalAddress);
		if (!ret)
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
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}
			
			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
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
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				return true;
			}
			
			int64 LogicID = m_Param.GetID();
			int64 ID = Think->CommentMemory(Text,LogicID);		
			if(ID ==0){
				tstring Answer = Format1024(_T("learn logic memo fail"));
				Dialog->RuntimeOutput(Answer);
			}
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			
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

	bool ret = Dialog->StartChildDialog(GetEventID(),_T("Input Dialog"),DialogText,TASK_OUT_THINK,ExePipe,LocalAddress);
	if (!ret)
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
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}
			
			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
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

				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				return true;
			}
			
			int64 ObjectID = m_Param.GetID();
			int64 ID = Think->CommentMemory(Text,ObjectID);		
			if(ID ==0){
				tstring Answer = Format1024(_T("learn object memo fail"));
				Dialog->RuntimeOutput(Answer);
			}
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			
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
					Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
					ExePipe.Break();
					return true;
				}
				
				int64 retTask = NewExePipe->GetID();
				if (retTask == RETURN_ERROR)
				{
					Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
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
					Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
					return true;	
				}
				
				
				m_Param.Clear();
				m_Param.PushInt(ID);
				m_Param.SetID(2);
				
				tstring DialogText = _T("Please input action memo : (or empty)");

				UpdateEventID();
				bool ret = Dialog->StartChildDialog(GetEventID(),_T("Input Dialog"),DialogText,TASK_OUT_THINK,*OldExePipe,LocalAddress);
				if (!ret)
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
					Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
					ExePipe.Break();
					return true;
				}
				
				int64 retTask = NewExePipe->GetID();
				if (retTask == RETURN_ERROR)
				{
					Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
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
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				
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
	bool ret = Dialog->StartChildDialog(GetEventID(),_T("Input Dialog"),DialogText,TASK_OUT_THINK,ExePipe,LocalAddress);
	if (!ret)
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
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}

			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
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
				bool ret = Dialog->StartChildDialog(GetEventID(),_T("Input word"),DialogText,TASK_OUT_THINK,*OldExePipe,LocalAddress);				
				if (!ret)
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

//////////////////////////////////////////////////////////////////////////


CInputElement::CInputElement(int64 ID,tstring Tip,bool bInputNum)
:CElement(ID,Tip.c_str()),m_bInputNum(bInputNum)
{
}

CInputElement::~CInputElement(){
	
};

MsgProcState CInputElement::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	int64 MsgID = Msg.GetMsgID();
	if( MsgID == MSG_TASK_RESULT){
		
		ePipeline& Letter = Msg.GetLetter();
		ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
		ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
		
		if (!NewExePipe->IsAlive())
		{	
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			return RETURN_DIRECTLY; //不在继续执行TaskProc()
		}
	
		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.Break();
			ExePipe.SetID(retTask);
			ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
			return RETURN_DIRECTLY;
		}
		
		int64 ThinkID = NewExePipe->PopInt();
		CLogicThread* Think = Dialog->m_Brain->GetBrainData()->GetLogicThread(ThinkID);
		assert(Think);
		tstring Text = Think->GetUserInput();
	    Text = TriToken(Text);

		Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);	

		if (m_bInputNum)
		{
			NumType Type = IsNum(Text);
			if (Type == NOT_NUM)
			{
				//再次压入事件，要求用户重新输入
				tstring DialogText = _T("Input error: not a num,Please input again.");
				
				UpdateEventID();
				bool ret = Dialog->StartChildDialog(GetEventID(),_T("Input Num"),DialogText,TASK_OUT_THINK,*OldExePipe,LocalAddress);				
				if (!ret)
				{
					tstring Answer = _T("Start child dialog fail.");
					ExePipe.SetLabel(Answer.c_str());
					ExePipe.SetID(RETURN_ERROR);
					return CONTINUE_TASK;
				}
				
				ExePipe.SetID(OldExePipe->GetID());
			}else if(Type == INT_NUM){
				ExePipe.Clear();
				ExePipe<<*OldExePipe;
				int64 i = _ttoi64(Text.c_str());
				ExePipe.PushInt(i);
			}else {
				ExePipe.Clear();
				ExePipe<<*OldExePipe;
				float64 f = _tcstod(Text.c_str(),NULL);
				ExePipe.PushFloat(f);
			}
		}else{
			ExePipe.Clear();
			ExePipe<<*OldExePipe;
			ExePipe.PushString(Text);
		}
		
	}else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
	}	
	return RETURN_DIRECTLY;
}
bool CInputElement::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	
	UpdateEventID();
	bool ret = Dialog->StartChildDialog(GetEventID(),_T("Input Dialog"),m_Name,TASK_OUT_THINK,ExePipe,LocalAddress);
	return ret;
}




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
//////////////////////////////////////////////////////////////////////////


bool CFocusObject::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	ExePipe.AutoTypeAB();   
	uint32 Type = GetTypeAB();             		
	if( !ExePipe.HasTypeAB(Type))   
	{
		ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("%I64ld: %s input data type checking failure:(%x,%x)"),m_ID,GetName().c_str(),Type,ExePipe.GetTypeAB());
		return true;
	}		

	tstring ObjectName = ExePipe.PopString();
	int64 InstanceID = Dialog->m_NamedObjectList.GetInstanceID(ObjectName);
	Dialog->m_ObjectFocus = InstanceID;
	if (InstanceID==0)
	{
		return false;
	}

	return true;
}


bool  CFocusObject_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	
	int64 InstanceID = Dialog->m_NamedObjectList.GetInstanceID(m_FocusName);
	Dialog->m_ObjectFocus = InstanceID;
	if (InstanceID==0)
	{
		return false;
	}
	
	return true;
};


bool  CFocusObject_Inter::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	Dialog->m_ObjectFocus = m_InstanceID;
	
	return true;
};

CUseObject::CUseObject(int64 ID)
	:CElement(ID,_T("UseObject")),m_InstanceID(0)
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
		
		ePipeline& Letter = Msg.GetLetter();
		ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
		ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
		
		if (!NewExePipe->IsAlive())
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.Break();
			return RETURN_DIRECTLY;
		}
		
		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.SetID(retTask);
			ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
			return RETURN_DIRECTLY;
		}
		
		Dialog->m_ObjectFocus = m_InstanceID;

		ExePipe.Clear();
		ExePipe << *NewExePipe;
		
		Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);

		return RETURN_DIRECTLY; // 不在继续执行TaskProc
	}else{
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
		return false;
	}
	
	assert(ObjectInstance->GetID() ==Dialog->m_ObjectFocus );
	
	m_InstanceID = Dialog->m_ObjectFocus;

	//如果没有启动外部器官，则启动
	CLinker Linker;
	Dialog->m_Brain->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("Space disconnected."));			
		return true;
	};
	
	//准备好一个请求信息
	UpdateEventID();

	CRequestBrain rq(REQUEST_USE_OBJECT);
	int64 DllType = *(int64*)ObjectInstance->GetData(4);
	rq.PushInt(DllType);
	rq.PushInt(m_InstanceID);
	rq.Push_Directly(ExePipe.Clone());
	
	
	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = Format1024(_T("Waiting for run external object:%I64ld  EventID:%I64ld"),m_InstanceID,GetEventID());
	tstring Title = Format1024(_T("UseObject(%I64ld)"),m_ID);
	Dialog->StartChildDialog(GetEventID(),Title,DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress);
				
	
	//把请求发给对方
	CMsg NewMsg(MSG_TASK_REQUEST,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter();
	Letter.Push_Directly(rq.Clone());
	
	Linker().PushMsgToSend(NewMsg);
				
	
	return true;
};

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

		ePipeline& Letter = Msg.GetLetter();
		ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
		ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);

		if (!NewExePipe->IsAlive())
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.Break();
			return RETURN_DIRECTLY;
		}

		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.SetID(retTask);
			ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
			return RETURN_DIRECTLY;
		}
		
		if (NewExePipe->Size()>0 && NewExePipe->GetDataType(0)== TYPE_STRING)
		{
			tstring Doc = NewExePipe->PopString();
			Dialog->RuntimeOutput(0,Doc);
		}

		Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);

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
	Dialog->m_Brain->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("Space disconnected."));			
		return true;
	};

	//准备好一个请求信息
	UpdateEventID();

	CRequestBrain rq(REQUEST_GET_OBJECT_DOC);
	int64 DllType = *(int64*)ObjectInstance->GetData(4);
	rq.PushInt(DllType);
	rq.PushInt(	Dialog->m_ObjectFocus);
	rq.Push_Directly(ExePipe.Clone());

	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = Format1024(_T("Waiting for run external object:%I64ld  EventID:%I64ld"),Dialog->m_ObjectFocus,GetEventID());
	tstring Title = Format1024(_T("UseObject(%I64ld)"),m_ID);
	Dialog->StartChildDialog(GetEventID(),Title,DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress);


	//把请求发给对方
	CMsg NewMsg(MSG_TASK_REQUEST,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter();
	Letter.Push_Directly(rq.Clone());

	Linker().PushMsgToSend(NewMsg);


	return true;
};

//////////////

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
				return false;
			}
			
			ePipeline& Letter = Msg.GetLetter();
			ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
			
			if (!NewExePipe->IsAlive())
			{
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}
			
			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				
				Dialog->CloseChildDialog(EventID,*OldExePipe,ExePipe);
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}
			int64 DllType = NewExePipe->PopInt();
			int64 InstanceID = NewExePipe->PopInt();
				
			assert(m_Obj->Size()==5);
			m_Obj->SetID(InstanceID);
			*(int64*)m_Obj->GetData(4) = DllType;

			CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
			nf.PushInt(INSTANCE_OBJECT);
			nf.PushInt(CREATE_INSTANCE);
			nf.Push_Directly(m_Obj->Clone());
			nf.Notify(Dialog);
			
			
			ExePipe.Clear();
			ExePipe << *NewExePipe;
			

			Dialog->CloseChildDialog(EventID,*OldExePipe,ExePipe);

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
	Dialog->m_Brain->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("Space disconnected."));			
		return true;
	}
	assert(ExePipe.IsAlive());
	
	//准备好一个请求信息
	UpdateEventID();
	
	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = _T("Waiting for starting external object");
	tstring Title = Format1024(_T("StartObject(%I64ld)"),m_ID);
	Dialog->StartChildDialog(GetEventID(),Title,DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress);
	
	
	//把请求发给对方
	Reset();
	CRequestBrain rq(REQUEST_START_OBJECT);
	rq.Push_Directly(m_Obj->Clone());
	rq.Push_Directly(ExePipe.Clone());
	
	CMsg NewMsg(MSG_TASK_REQUEST,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter();
	Letter.Push_Directly(rq.Clone());
	
	Linker().PushMsgToSend(NewMsg);
	return true;
};

//////////////
CCloseObject::CCloseObject(int64 ID)
:CElement(ID,_T("CloseObject"))
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
			
			ePipeline& Letter = Msg.GetLetter();
			ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
			
			
			if (!NewExePipe->IsAlive())
			{
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}
			
			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}
			
			Dialog->m_NamedObjectList.UnregisterNameByTask(m_ObjectName);
			
			ExePipe.Clear();
			ExePipe << *NewExePipe;
			
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
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
	
	tstring SrcObjectName = *(tstring*)ObjectInstance->GetData(0);
	
	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_OBJECT);
	nf.PushInt(CLOSE_INSTANCE);
	nf.PushString(SrcObjectName);
	nf.PushInt(Dialog->m_ObjectFocus);
	nf.Notify(Dialog);
	

	//如果没有启动外部器官，则启动
	CLinker Linker;
	Dialog->m_Brain->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		//ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("Waring:Space disconnected."));			
		return true;
	}
	//准备好一个请求信息
	UpdateEventID();

	CRequestBrain rq(REQUEST_CLOSE_OBJECT);

	int64 DllType = *(int64*)ObjectInstance->GetData(4);
	rq.PushInt(DllType);
	rq.PushInt(ObjectInstance->GetID());
	rq.Push_Directly(ExePipe.Clone());
	
	Dialog->CloseObjectInstance(Dialog->m_ObjectFocus);

	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = _T("Waiting for start external object");
	tstring Title = Format1024(_T("CloseObject(%I64ld)"),m_ID);
	Dialog->StartChildDialog(GetEventID(),Title,DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress);
	
	
	//把请求发给对方
	CMsg NewMsg(MSG_TASK_REQUEST,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter();
	Letter.Push_Directly(rq.Clone());
	
	Linker().PushMsgToSend(NewMsg);
	return true;
};

//////////////////////////////////////////////////////////////////////////

bool  CCreateTable::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 
	
	
	int64 InstanceID = AbstractSpace::CreateTimeStamp();
	ePipeline Pipe(m_MemoryName.c_str(),0);
	Dialog->AddMemoryInstance(InstanceID,Pipe);
	Dialog->m_DataTableFocus = InstanceID;
	Dialog->m_NamedTableList.RegisterNameByTask(m_MemoryName,Dialog->m_TaskID,InstanceID);
	
	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_DATA);
	nf.PushInt(CREATE_INSTANCE);
	nf.PushString(m_MemoryName);
	nf.Notify(Dialog);
	
	return true;
}

bool  CFocusTable::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	
	tstring MemoryName = ExePipe.PopString();
	int64 InstanceID = Dialog->m_NamedTableList.GetInstanceID(MemoryName);
	Dialog->m_DataTableFocus = InstanceID;
	if (InstanceID==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find focus memory(%s)"),MemoryName.c_str());
		return false;
	}
	

	return true;
}

bool  CFocusMemory_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	
	int64 InstanceID = Dialog->m_NamedTableList.GetInstanceID(m_FocusName);
	Dialog->m_DataTableFocus = InstanceID;
	if (InstanceID==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find focus memory(%s)"),m_FocusName.c_str());
		return false;
	}
	

	return true;
}

bool  CTable_InsertData::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 
	
	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_DataTableFocus);
	if (MemoryInstance==NULL)
	{
		return false;
	}
	
	int64 Index = ExePipe.PopInt();
	
	
	if (Index<0)
	{
		Index = 0;
	}
	
	if (Index> MemoryInstance->Size())
	{
		Index =  MemoryInstance->Size();
	}
	
	ePipeline* NewData = new ePipeline;
	if (!NewData)
	{
		return false;
	}
	
	*NewData << ExePipe;
	
	
	tstring& InstanceName = MemoryInstance->GetLabel();
	
	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_DATA);
	nf.PushInt(INSERT_DATA);
	nf.PushString(InstanceName);
	nf.PushInt(Index);
	nf.Push_Directly(NewData->Clone());
	
	nf.Notify(Dialog);
	
	
	MemoryInstance->InsertEnergy(Index,NewData);
	
	
	return true;
}

bool  CTable_ImportData::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 

	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_DataTableFocus);
	if (MemoryInstance==NULL)
	{
		return false;
	}

	tstring& InstanceName = MemoryInstance->GetLabel();
	
	ePipeline DataList;
	while (ExePipe.Size())
	{
		eElectron e;
		ExePipe.Pop(&e);
		if (e.EnergyType()==TYPE_PIPELINE)
		{
			DataList.Push_Directly(e.Get()->Clone());
			MemoryInstance->Push_Directly(e.Release());
		}else{
			ePipeline* NewData = new ePipeline;
			if (!NewData)
			{
				return false;
			}

			NewData->Push_Directly(e.Release());

			DataList.Push_Directly(NewData->Clone());
			MemoryInstance->Push_Directly(NewData);
		}
	}

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_DATA);
	nf.PushInt(IMPORT_DATA);
	nf.PushString(InstanceName);
	nf.PushPipe(DataList);

	nf.Notify(Dialog);

	return true;
}
bool  CTable_ExportData::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 

	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_DataTableFocus);
	if (MemoryInstance==NULL)
	{
		return false;
	}

	while(MemoryInstance->Size()){
		eElectron e;
		MemoryInstance->Pop(&e);
		ePipeline* Line = (ePipeline*)e.Get();
		if(Line->Size()==1){
			eElectron e1;
			Line->Pop(&e1);
			ExePipe.Push_Directly(e1.Release());
		}else{
			ExePipe.Push_Directly(e.Release());
		}
	}

	tstring& InstanceName = MemoryInstance->GetLabel();

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_DATA);
	nf.PushInt(EXPORT_DATA);
	nf.PushString(InstanceName);
	nf.Notify(Dialog);

	return true;
}

bool CTable_ModifyData::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{

	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_DataTableFocus);
	if (MemoryInstance==NULL)
	{
		return false;
	}

	int64 Index = ExePipe.PopInt();

	if (Index<0 || Index> MemoryInstance->Size()-1)
	{
		ExePipe.GetLabel() = Format1024(_T("ModifData Error: Index(%I64ld) out of range(%d)"),Index,MemoryInstance->Size());
		return false;
	}

	ePipeline* NewData = new ePipeline;
	if (!NewData)
	{
		return false;
	}

	*NewData << ExePipe;


	tstring& InstanceName = MemoryInstance->GetLabel();

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_DATA);
	nf.PushInt(MODIFY_DATA);
	nf.PushString(InstanceName);
	nf.PushInt(Index);
	nf.Push_Directly(NewData->Clone());

	nf.Notify(Dialog);


	Energy* Old = MemoryInstance->ReplaceEnergy(Index,NewData);
	assert(Old);
	delete Old;

	return true;
}
bool  CTable_GetData::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 
	
	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_DataTableFocus);
	if (MemoryInstance==NULL)
	{
		return false;
	}
	
	int64 Index = ExePipe.PopInt();
	

	if (Index<0 || Index>MemoryInstance->Size())
	{
		return false;
	}
	
	ePipeline Data = *(ePipeline*)MemoryInstance->GetData(Index);
	
	ExePipe<<Data;		
	
	
	return true;
}

bool  CTable_RemoveData::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 
	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_DataTableFocus);
	if (MemoryInstance==NULL)
	{
		return false;
	}
	
	int64 Index = ExePipe.PopInt();
	
	
	if (Index<0 || Index>MemoryInstance->Size())
	{
		return false;
	}
	
	MemoryInstance->EraseEnergy(Index,1);
	
	tstring& InstanceName = MemoryInstance->GetLabel();
	
	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_DATA);
	nf.PushInt(REMOVE_DATA);
	nf.PushString(InstanceName);
	nf.PushInt(Index);
	nf.Notify(Dialog);
	
	return true;
}

bool  CTable_GetSize::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 
	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_DataTableFocus);
	if (MemoryInstance==NULL)
	{
		return false;
	}
	int64 Size = MemoryInstance->Size();
	ExePipe.PushInt(Size);
	
	
	return true;
}

bool  CCloseTable::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){ 
		
	ePipeline* MemoryInstance = Dialog->FindTempMemory(Dialog->m_DataTableFocus);
	if (MemoryInstance==NULL)
	{
		return true;
	}
	
	tstring InstanceName = MemoryInstance->GetLabel();
	
	Dialog->CloseMemoryInstance(Dialog->m_DataTableFocus);
	
	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_DATA);
	nf.PushInt(CLOSE_INSTANCE);
	nf.PushString(InstanceName);
	nf.Notify(Dialog);
	
	
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool  CFocusLogic::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	
	tstring LogicName = ExePipe.PopString();
	CElement* E = FindFocusLogic(LogicName);
	
	if (E==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find focus logic(%s)"),LogicName.c_str());
		return false;
	}
	Dialog->m_LogicFocus = LogicName;
	

	return true;
}

bool  CFocusLogic_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{

	CElement* E = FindFocusLogic(m_FocusName);
	
	if (E==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find focus logic(%s)"),m_FocusName.c_str());
		return false;
	}
	Dialog->m_LogicFocus = m_FocusName;
		

	return true;
}


MsgProcState CInserLogic::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_TASK_RESULT)
	{	
		
		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID())
		{
			return RETURN_DIRECTLY;
		}
		
		ePipeline& Letter = Msg.GetLetter();
		ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
		ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
		
		
		if (!NewExePipe->IsAlive())
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			
			tstring Answer = _T("收到的数据管道已经无效");
			ExePipe.SetLabel(Answer.c_str());
			ExePipe.SetID(RETURN_ERROR);
			return RETURN_DIRECTLY;
		}
		
		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);	

			ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
			ExePipe.SetID(RETURN_ERROR);
			return RETURN_DIRECTLY;
		}
		
		
		ExePipe.Clear();
		ExePipe << *OldExePipe;
		
		Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
		return RETURN_DIRECTLY;
	}else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
	}
	return RETURN_DIRECTLY;
};

bool CInserLogic::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	CElement* LogicInstance = FindFocusLogic(Dialog->m_LogicFocus);
	if (LogicInstance==NULL)
	{
		return false;
	}
	
	ePipeline InsertAddress;
	LogicInstance->GetAddress(InsertAddress);
	
	//准备好一个请求信息
	
	UpdateEventID();

	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = Format1024(_T("Waiting for insert logic %s"),m_InsertLogicName.c_str());
	Dialog->StartChildDialog(GetEventID(),_T("Insert Logic"),DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress);
	

	CRequestBrain rq(REQUEST_INSERT_LOGIC);
	rq.PushInt(GetEventID());
	rq.PushString(m_InsertLogicName);
	rq.PushPipe(InsertAddress);
	rq.Request(Dialog);

	return true;
};




//////////////////////////////////////////////////////////////////////////

bool CRemoveLogic::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	CElement* LogicInstance = FindFocusLogic(Dialog->m_LogicFocus);
	if (LogicInstance==NULL)
	{
		return false;
	}
	
	ePipeline RemoveAddress;
	LogicInstance->GetAddress(RemoveAddress);
	
	eINT* e = new eINT(Dialog->m_DialogID);
	RemoveAddress.Push_Front(e);
	
	assert (RemoveAddress.Size() < LocalAddress.Size());
	
	int i;
	for (i=0; i<RemoveAddress.Size();i++)
	{
		int64 ID1 = *(int64*)RemoveAddress.GetData(i);
		int64 ID2 = *(int64*)LocalAddress.GetData(i);
		if (ID1 != ID2)
		{
			return false;
		}
	}
	
	int64 ChildID = *(int64*)LocalAddress.GetData(i);
	
	//准备好一个请求信息
	UpdateEventID();

	//Dialog->m_Brain->GetBrainData()->PushBrainEvent(GetEventID(),ExePipe,RemoveAddress,TIME_SEC); 

	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = Format1024(_T("Waiting for remove logic"));
	Dialog->StartChildDialog(GetEventID(),_T("Remove Logic"),DialogText,TASK_OUT_DEFAULT,ExePipe,RemoveAddress);

	RemoveAddress.PopInt();  //去掉dilaog ID
	CRequestBrain rq(REQUEST_REMOVE_LOGIC);
	rq.PushInt(GetEventID());
	rq.PushPipe(RemoveAddress);
	rq.PushInt(ChildID);
	rq.Request(Dialog);

	return true;
}


//////////////	

CUseRobot::CUseRobot(int64 ID,CLogicDialog* Dialog, int64 RobotID,ePipeline& LogicPipe)
:CElement(ID,_T("UseObject")),m_Dialog(Dialog),m_RobotID(RobotID)
{

	m_TaskLogic<<LogicPipe;
	m_TaskLogic.SetLabel(LogicPipe.GetLabel().c_str());
	
};

CUseRobot::~CUseRobot(){
	
};

TypeAB CUseRobot::GetTypeAB(){ 
	return 0;
};

tstring CUseRobot::GetName(){ 
	return m_TaskLogic.GetLabel();
};


MsgProcState CUseRobot::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_TASK_RESULT)
	{		
		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID())
		{
			return RETURN_DIRECTLY;
		}
		
		ePipeline& Letter = Msg.GetLetter();
		ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
		ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
		
		if (!NewExePipe->IsAlive())
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.Break();
			return CONTINUE_TASK;
		}
		
		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.Break();
			ExePipe.SetID(retTask);
			return CONTINUE_TASK;
		}
		
		
		ExePipe.Clear();
		ExePipe << *NewExePipe;
		Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
		
	}else{
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
	}
	return RETURN_DIRECTLY;
}

bool CUseRobot::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	//return m_Dialog->m_Brain->RunExternalRobot(m_Dialog,m_EventID,m_RobotID,m_TaskLogic,ExePipe,LocalAddress);	
	
	//如果没有启动外部器官，则启动
	CLinker Linker;
	m_Dialog->m_Brain->GetLinker(m_RobotID,Linker);//RobotID = SourceID
	if (!Linker.IsValid())
	{		
		ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("The robot disconnect."));			
		return true;
	}
	tstring& LogicText = m_TaskLogic.GetLabel();
	
	//发执行信息
	UpdateEventID();

	CRequestBrain rq(REQUEST_EXE_LOGIC);
	rq.PushInt(GetEventID());
	rq.PushString(LogicText);
	rq.Push_Directly(ExePipe.Clone());
	
	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = _T("Waiting for run external robot");
	Dialog->StartChildDialog(GetEventID(),_T("UseObject"),DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress);
	
	
	
	CMsg NewMsg(MSG_TASK_REQUEST,DEFAULT_DIALOG,0);
	ePipeline& Letter = NewMsg.GetLetter();
	Letter.Push_Directly(rq.Clone());
	
	Linker().PushMsgToSend(NewMsg);

	return true;
}


/////////////////

bool CConnectSocket::Do(ePipeline* Pipe,ePipeline* LocalAddress){

	assert(Pipe->Size()==3);
	
    tstring  s;
	Pipe->PopString(s);
    
	AnsiString ServerName = WStoUTF8(s);

	int32 Port = Pipe->PopInt();
	int32 Time = Pipe->PopInt();
	
	m_StopTime = AbstractSpace::CreateTimeStamp();
	m_StopTime = AbstractSpace::GetTimer()->AddTime(m_StopTime,Time);

		
	if(ServerName[0] == '\0'){
	    Pipe->GetLabel()= Format1024(_T("Connect Fail: Invalid ServerName."));
		return false;
	}
	

	m_Socket=WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);
	
	if(m_Socket==INVALID_SOCKET){
		Pipe->GetLabel()= Format1024(_T("Connect Fail: can't create socket."));
		return TRUE;
	}
				
	struct hostent *pHost;
	if(inet_addr(ServerName.c_str())==INADDR_NONE)
	{
		pHost=gethostbyname(ServerName.c_str());
	}
	else
	{
		unsigned int addr=inet_addr(ServerName.c_str());
		pHost=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
	}
	if(pHost==NULL)
	{
		Pipe->GetLabel()= Format1024(_T("Connect Fail: Invalid ServerName."));
		return TRUE;
	}
	
	struct sockaddr_in Server;
	Server.sin_addr.s_addr=*((unsigned long*)pHost->h_addr);
	Server.sin_family=AF_INET;
	Server.sin_port=htons(Port);
	
	//把m_SocketSelf改为非阻塞,以便能检测连接超时
	unsigned long ul = 1; //设置为非阻塞
	int Ret = ioctlsocket(m_Socket,FIONBIO,(unsigned long*)&ul);
    
	if(Ret == SOCKET_ERROR){
        int ErrorCode = WSAGetLastError();
		Pipe->GetLabel()= Format1024(_T("Connect Fail: Internal socket error[code:%d]."),ErrorCode);
		return TRUE;
	}
	
	int ret = connect(m_Socket,(struct sockaddr*)&Server,sizeof(Server));
	if(ret==SOCKET_ERROR)
	{	//正常
		int ErrorCode = WSAGetLastError();
		if (ErrorCode != WSAEWOULDBLOCK){
			Pipe->GetLabel()= Format1024(_T("Connect Fail: Internal socket error[code:%d]."),ErrorCode);
			return TRUE;
		}
		
	}
	return CheckConnected(Pipe,LocalAddress);
};


bool  CConnectSocket::CheckConnected(ePipeline* Pipe, ePipeline* LocalAddress)
{
/*
	fd_set  WriteFDs; 
	int32   SecondCount = 0;    //用于连接时间计数——一秒的倍数  
	
	switch(m_ConState)
	{
	case PHYSICAL_CON:
		{
			FD_ZERO(&WriteFDs);		
			FD_SET(m_Socket, &WriteFDs);		
			
			timeval TimeOut;  //设置1秒钟的间隔，
			TimeOut.tv_sec  = 0; 
			TimeOut.tv_usec = 0;
			
			int ret = select(0,NULL, &WriteFDs, NULL, &TimeOut);
			if(ret>0)
			{

				if(FD_ISSET(m_Socket,&WriteFDs)){
				

					CUserLinkerPipe* Linker = GetBrain()->CreateClientLinkerPipe();
					
					assert(0);//这里似乎应该移动到userspace
					//Linker->AttachSocket(m_Socket);

					GetBrain()->GetSuperiorLinkerList()->AddLinker(Linker);
					
					//Dialog->m_Brain->CreateLinkerDialog(Linker);  对话在认证环节生成

					tstring s = tformat(_T("Connect ok...wait feedback"));
                    GetBrain()->OutSysInfo(s);
									//m_AddressBar.SetConnectState(TRUE);
					m_LinkerID = Linker->GetSourceID();
					m_ConState = LOGIC_CON;
				}
			}
			else if (ret == SOCKET_ERROR) { //退出

				int ErrorCode = WSAGetLastError();
				Pipe->PrintLabel(_T("Connect Fail: Internal socket error[code:%d]."),ErrorCode);

				return TRUE;
			}
			else if(ret == 0){ //timeout
				

				int64 TimeStamp = GetBrain()->CreateTimeStamp();

				tstring t1 = AbstractSpace::GetTimer()->GetFullTime(TimeStamp);
				tstring t2 = AbstractSpace::GetTimer()->GetFullTime(m_StopTime);
				
				if(TimeStamp <m_StopTime){				
										
					CMsg Msg(MSG_SET_TIMER,0,0);

					ePipeline& Letter = Msg.GetLetter();
					TimeStamp += (int64)50*1000*10; //毫秒转换成百纳秒
					Letter.PushInt64(TimeStamp);
					
					Msg.GetSender() =  *LocalAddress;
					
					GetBrain()->PushCentralNerveMsg(Msg);

					return FALSE;
					
				}else{
					
					//如果超过了预定时间还没有连接则报错并退出
					tstring t1 = AbstractSpace::GetTimer()->GetFullTime(TimeStamp);
					tstring t2 = AbstractSpace::GetTimer()->GetFullTime(m_StopTime);
                
					Pipe->PrintLabel(_T("Connect Fail: Time out(%s > %s)"),t1.c_str(),t2.c_str());
				}
				return TRUE;
			}
	}
	case LOGIC_CON:
		{ //检查是否完成握手对话
			

			CLinker Linker = GetBrain()->GetSuperiorLinkerList()->GetLinker(m_LinkerID);
			if(!Linker.IsValid()){
				
				Pipe->PrintLabel(_T("Connect Fail: remote break"));
				return false;
			};

			if (Linker().GetRecoType() == LINKER_FRIEND)
			{
				return false ;
			}else{

				//预定时间做检查，超出时间则返回				
				int64 TimeStamp = GetBrain()->CreateTimeStamp();

				if(TimeStamp <m_StopTime){		
						
					CMsg Msg(MSG_SET_TIMER,0,0); 
					ePipeline& Letter = Msg.GetLetter();
					int64 TimeStamp = AbstractSpace::CreateTimeStamp();
					TimeStamp += (int64)50*1000*10; //毫秒转换成百纳秒
					Letter.PushInt64(TimeStamp);
					
					Msg.GetSender() =  *LocalAddress;
					
					GetBrain()->PushCentralNerveMsg(Msg);

					return false;
				}else{
					

					//如果超过了预定时间还没有连接则报错并退出

					tstring t1 = AbstractSpace::GetTimer()->GetFullTime(TimeStamp);
					tstring t2 = AbstractSpace::GetTimer()->GetFullTime(m_StopTime);

					Pipe->PrintLabel(_T("Connect Fail: Time out(%s > %s)"),t1.c_str(),t2.c_str());
					
				}
				return false;
			}
			
	}
	    break;
	default:
	    break;
	}
	return true;
*/
return TRUE;
}
void CConnectSocket::SysMsgProc(CLogicDialog* Dialog,CMsg& SysMsg,ePipeline* ExePipe,ePipeline* LocalAddress){
/*
	assert(LocalAddress);

	int64 MsgID = SysMsg.GetMsgID();
	
	switch(MsgID){
	case MSG_TIME_OUT:
		Dialog->TaskRun();
        CheckConnected(Dialog,LocalAddress);
		break;
	default:
		Element::SysMsgProc(Dialog,SysMsg,LocalAddress);
		break;
	};
	return;
*/	
}





void CPipeViewMass::PrintView(ePipeline& Result,ePipeline& Pipe)
{
	
	TCHAR buffer[1024];
    int i;
	int size = Pipe.Length();

	int Tab=++m_TabNum;

	//限制嵌套数目
	if(Tab>90){
		throw std::exception("WARNING: Nest Pipe too much, end View");
	}
    //Tab = 0;
	for(i=0; i<Tab; i++){
		buffer[i]=_T('\t');
	}
	tstring PipeLabel = Pipe.m_Label;
	if(PipeLabel.size())
	{
		_stprintf(buffer+Tab,_T("Output<%s>:\n"),PipeLabel.c_str());
	}	
	else{
        _stprintf(buffer+Tab,_T("Output:\n"));
	} 
	Result.PushString(buffer);

	for(i=0; i<size; i++){
	   uint32 Type = Pipe.GetDataType(i);
	   switch(Type){
	   case TYPE_NULL:
		   {
		   _stprintf(buffer,_T("%03d NULL:0\n"),i);
		   Result.PushString(buffer);
		   }
		   break;

	   case TYPE_INT:
		   {
		   int64* value=(int64 *)Pipe.GetData(i);
           _stprintf(buffer,_T("%03d INT64: %I64ld\n"),i, *value);
           Result.PushString(buffer);
		   }
		   break;
	   case TYPE_FLOAT:
		   {
		   float64* value=(float64 *)Pipe.GetData(i);
           _stprintf(buffer,_T("%03d FLOAT64: %f\n"),i, *value);
           Result.PushString(buffer);
		   }
		   break;
	   case TYPE_STRING:
		   {
		   tstring* value=(tstring *)Pipe.GetData(i);
           _stprintf(buffer,_T("%03d STRING:"),i);
		   //MBPrintf("test1","%s",value->c_str());
		   tstring s(buffer);
		   s +=(*value);
		   s +=_T('\n');

           Result.PushString(s);
		   //MBPrintf("test","size=%d",Result.Size());
		   }
		   break;
 /*	   case TYPE_STREAM:
		   {
			  
		   eStream* value=(eStream *)Data.Value();
           int size=value->GetDataCount();
		   for(int i=0; i<size; i++){
		      _stprintf(buffer,"%03d STREAM: \n",i);
              eSTRING s(buffer);
		      Result.Push(&s);
			  value->Output(Result);
		   }
		 
		   }
		   break;
  */	   case TYPE_PIPELINE:
		   {
		   ePipeline* value=(ePipeline *)Pipe.GetData(i);
		   //如果不用PipeNum++而用t++,导致t增大不能恢复，中间形成空格，之后打印的数据不可见
		   PrintView(Result,*value);
		   }      
		   break;
	   }
	}
    m_TabNum--;
}	
	
void CPipeViewMass::GetPipeInfo(ePipeline* Pipe,tstring& text){
	m_TabNum = -1;
	ePipeline Result;
	PrintView(Result,*Pipe);
	int len = Result.Length();
    eElectron Data;
	for(int i=0; i<len; i++){
        Result.Pop(&Data);
        text+=*(tstring*)Data.Value();
	}
}

bool CPipeViewMass::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	tstring Text;
	GetPipeInfo(&ExePipe,Text);	
	Dialog->RuntimeOutput(Text);

	return true;
}


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