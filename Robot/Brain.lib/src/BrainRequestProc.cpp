#pragma warning (disable:4786)

#include "Brain.h"
#include "LogicDialog.h"
#include "GUIMsgDefine.h"
bool CBrain::ProcessRequest(CLogicDialog* Dialog,ePipeline* RequestInfo){

	REQUEST_ITEM RequestID = (REQUEST_ITEM)RequestInfo->GetID();
	
	switch (RequestID)
	{

	case REQUEST_USE_OBJECT:
		return OnRequestUseObject(Dialog,RequestInfo);
//	case REQUEST_TRANS_TASK:
//		return OnRequestTransTask(Dialog,RequestInfo);
	case REQUEST_EXE_LOGIC:
		return OnRequestExeLogic(Dialog,RequestInfo);
	case REQUEST_INSERT_LOGIC:
		return OnRequestInsertLogic(Dialog,RequestInfo);
	case REQUEST_REMOVE_LOGIC:
		return OnRequestRemoveLogic(Dialog,RequestInfo);
	default:
		assert(0);
	}
	return false;
}

bool CBrain::OnRequestDialog(CLogicDialog* Dialog,ePipeline* RequestInfo){
	int64 EventID = RequestInfo->PopInt();
    TASK_OUT_TYPE OutType = (TASK_OUT_TYPE)RequestInfo->PopInt();
	int64  EventInterval= RequestInfo->PopInt();
	int64  bFocusDialog  = RequestInfo->PopInt();
	int64  bEditValid    = RequestInfo->PopInt();
	tstring DialogName = RequestInfo->PopString();
	tstring DialogText = RequestInfo->PopString();

	ePipeline* ClientExePipe = (ePipeline*)RequestInfo->GetData(0);
    ePipeline* ClientAddress = (ePipeline*)RequestInfo->GetData(1);
	
	CLogicDialog* NewDialog = NULL;
	
	NewDialog = GetBrainData()->GetDialog(Dialog->m_SourceID,EventID);
	if (!NewDialog)
	{
		NewDialog = Dialog->StartEventDialog(EventID,DialogName,DialogText,OutType,*ClientExePipe,*ClientAddress,EventInterval,bFocusDialog,bEditValid,true);
		if(!NewDialog)return false;
	
    }else{
		//先做个清理
		NewDialog->ResetThink();
		NewDialog->SetTaskState(TASK_STOP);
		
		//然后设置为等待
		NewDialog->SetTaskState(TASK_WAIT);

	}

	return true;
}

bool CBrain::OnRequestUseObject(CLogicDialog* Dialog,ePipeline* RequestInfo){
	return true;
}

bool CBrain::OnRequestExeLogic(CLogicDialog* Dialog,ePipeline* RequestInfo)
{
/*
	int64 SourceID = Dialog->m_SourceID;
	if (SourceID == SYSTEM_SOURCE) //暂时不允许本地请求
	{
		return false;	
	}

	int64 EventID = RequestInfo->PopInt();
	assert(EventID);	
	
	tstring LogicText  = RequestInfo->PopString();	
	ePipeline* ExePipe = (ePipeline*)RequestInfo->GetData(0);
	
	//生成子对话		
	tstring TaskName = _T("LogicTask");
	int64 TaskID = AbstractSpace::CreateTimeStamp();	

	CLogicDialog* ChildDialog = GetBrainData()->CreateNewDialog(this,Dialog->m_SourceID,TaskID,0,Dialog->m_SourceName,TaskName,DIALOG_SYSTEM_CHILD,TASK_OUT_DEFAULT);
	
	if (!ChildDialog)
	{	
		tstring text = _T("The task transferred fail");
		Dialog->SaveSendItem(text,0);				

		CMsg rMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);		
		
		ePipeline& rLetter = rMsg.GetLetter();	
		ExePipe->SetID(RETURN_ERROR);	
		ExePipe->SetLabel(text.c_str());
		rLetter.PushPipe(*ExePipe);
		
		CLinker Linker;
		GetLinker(rMsg.GetSourceID(),Linker);
		if (Linker.IsValid())
		{
			Linker().PushMsgToSend(rMsg);
		}

		return false;
	}

    ePipeline Address(SourceID);
	Address.PushInt(TaskID);


	GetBrainData()->CreateEvent(TaskID,EventID,*ExePipe,Address,TIME_SEC,true);

	tstring s = Format1024(_T("Create New Dialog[%s]: Type=%d  DialogID=%I64d \n"),
		TaskName.c_str(),ChildDialog->m_DialogType,ChildDialog->m_DialogID);	
	
	Dialog->RuntimeOutput(0,s);
	
	tstring text = _T("The task has been transferred to the sub-dialogue processing");
	Dialog->SaveSendItem(text,0);
	
	
	CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
	nf.PushInt(DL_ADD_DIALOG);
	nf.PushInt(TRUE);
	nf.Notify(ChildDialog);
	
	
	//子任务理解逻辑文本
	CLogicThread* Think = ChildDialog->GetThink();
	Think->ThinkProc(ChildDialog,0,LogicText,false);
	int32 LogicNum = Think->CanBeExecute(ChildDialog);
	if(LogicNum ==0){ //不能执行,让用户继续编辑
		
		ChildDialog->SetTaskState(TASK_IDLE);
		ChildDialog->NotifyTaskState();
		
		//反馈结果
		CMsg rMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);		
		ePipeline& rLetter = rMsg.GetLetter();	
	
		ExePipe->SetID(RETURN_ERROR);
		ExePipe->GetLabel() = _T("Think error!");
		rLetter.PushPipe(*ExePipe);	

		CLinker Linker;
		GetLinker(rMsg.GetSourceID(),Linker);
		if (Linker.IsValid())
		{
			Linker().PushMsgToSend(rMsg);
		}		
		//关闭对话	
		return false;
	}
    
	//如果成功理解则准备执行
	ePipeline ChildMsg(TO_BRAIN_MSG::TASK_CONTROL::ID);
	ChildMsg.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE);			
	CMsg Msg1;
	CreateBrainMsg(Msg1,ChildDialog->m_DialogID,ChildMsg,EventID);		
	
	PushNerveMsg(Msg1,false,false);		
*/
	return true;
}

bool CBrain::OnRequestTransTask(CLogicDialog* Dialog,ePipeline* RequestInfo){

	int64 EventID = RequestInfo->PopInt();
	int64 bJustOnce = RequestInfo->PopInt();

	tstring TaskName = _T("LogicTask");
	tstring DialogText = _T("");

	int64 ClientEventID = 0;
	ePipeline ClientAddress(Dialog->m_SourceID);
	ClientAddress.PushInt(Dialog->m_DialogID);

	ePipeline  ClientExePipe;

	CLogicDialog* TaskDialog = Dialog->StartEventDialog(EventID,TaskName,DialogText,TASK_OUT_DEFAULT,ClientExePipe,ClientAddress,TIME_SEC,true,true,true);
	if (!TaskDialog)
	{
		tstring text = _T("The task transferred fail");
		Dialog->SaveSendItem(text,0);
		Dialog->ResetThink();
		return false;
	}
	//把当前逻辑环境转交给新的子对话
	*TaskDialog<<*Dialog;

	CMsg Msg(SYSTEM_SOURCE,TaskDialog->m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,0);
	ePipeline& Letter = Msg.GetLetter();
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
	Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE);

	PushNerveMsg(Msg,false,false);

	Dialog->ResetThink();
	return true;
}

bool CBrain::OnRequestInsertLogic(CLogicDialog* Dialog,ePipeline* RequestInfo)
{
	int64 EventID = RequestInfo->PopInt();

	tstring InsertLogicName = RequestInfo->PopString();
	ePipeline* InsertAddress = (ePipeline*)RequestInfo->GetData(0); 
	
	CMsg EltMsg(Dialog->m_SourceID,*InsertAddress,MSG_ELT_INSERT_LOGIC,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushString(InsertLogicName);
	
	//直接把信息压入任务队列，并且优先处理
	Dialog->m_ElementMsgList.PushUrgence(EltMsg.Release());

	return true;
}

bool CBrain::OnRequestRemoveLogic(CLogicDialog* Dialog,ePipeline* RequestInfo)
{
	int64 EventID = RequestInfo->PopInt();
	
	ePipeline* RemoveAddress = (ePipeline*)RequestInfo->GetData(0); 
    int64 ChildID  =   	*(int64*)RequestInfo->GetData(1); 

	CMsg EltMsg(Dialog->m_SourceID,*RemoveAddress,MSG_ELT_REMOVE_LOGIC,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushInt(ChildID);
	
	//直接把信息压入任务队列，并且优先处理
	Dialog->m_ElementMsgList.PushUrgence(EltMsg.Release());

	return true;
}