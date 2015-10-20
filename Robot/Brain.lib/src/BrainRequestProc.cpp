#pragma warning (disable:4786)

#include "Brain.h"
#include "LogicDialog.h"
#include "GUIMsgDefine.h"
bool CBrain::ProcessRequest(CLogicDialog* Dialog,ePipeline* RequestInfo){

	REQUEST_ITEM RequestID = (REQUEST_ITEM)RequestInfo->GetID();
	
	switch (RequestID)
	{
	case REQUEST_END:
        return OnRequestEnd(Dialog,RequestInfo);
	case REQUEST_DIALOG:
		return OnRequestDialog(Dialog,RequestInfo);
	case REQUEST_USE_OBJECT:
		return OnRequestUseObject(Dialog,RequestInfo);
	case REQUEST_TRANS_TASK:
		return OnRequestTransTask(Dialog,RequestInfo);
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

bool CBrain::OnRequestEnd(CLogicDialog* Dialog,ePipeline* RequestInfo){
	int64 EventID = RequestInfo->PopInt();
	CLogicDialog* Dlg = GetBrainData()->GetDialog(EventID);
    if (Dlg)
    {
		CNotifyState nf(NOTIFY_DIALOG_LIST);
		nf.PushInt(DL_DEL_DIALOG);
		nf.Notify(Dlg);
		
		GetBrainData()->DeleteDialog(EventID);
    }
	return true;
}

bool CBrain::OnRequestDialog(CLogicDialog* Dialog,ePipeline* RequestInfo){
	int64 EventID = RequestInfo->PopInt();
    TASK_OUT_TYPE OutType = (TASK_OUT_TYPE)RequestInfo->PopInt();
	int64  EventInterval= RequestInfo->PopInt();
	int64  bFocusDialog  = RequestInfo->PopInt();
	int64  bEditValid    = RequestInfo->PopInt();
	tstring DialogName = RequestInfo->PopString();
	tstring DialogText = RequestInfo->PopString();

	ePipeline* ExePipe = (ePipeline*)RequestInfo->GetData(0);
    ePipeline* Address = (ePipeline*)RequestInfo->GetData(1);

	CLogicDialog* NewDialog = GetBrainData()->GetDialog(EventID);
    if (!NewDialog)
    {
		NewDialog = GetBrainData()->CreateNewDialog(this,Dialog->m_SourceID,EventID,Dialog->GetDialogID(),_T("Dialog"),DialogName,DIALOG_SYSTEM_CHILD,0,OutType);	
		if(!NewDialog)return false;
		
		NewDialog->m_bEditValid = bEditValid?true:false;

		CNotifyState nf(NOTIFY_DIALOG_LIST);
		nf.PushInt(DL_ADD_DIALOG);
		nf.PushInt(bFocusDialog);
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
	GetBrainData()->PushBrainEvent(EventID,*ExePipe,*Address,EventInterval);
	
	if (DialogText.size())
	{
		NewDialog->SaveSendItem(DialogText,0);
	}
	return true;
}

bool CBrain::OnRequestUseObject(CLogicDialog* Dialog,ePipeline* RequestInfo){
	return true;
}

bool CBrain::OnRequestExeLogic(CLogicDialog* Dialog,ePipeline* RequestInfo)
{
	int64 SourceID = Dialog->m_SourceID;
	if (SourceID == SYSTEM_SOURCE) //暂时不允许本地请求
	{
		return false;
		
	}

	int64 EventID = RequestInfo->PopInt();
	assert(EventID);	
	
	tstring LogicText = RequestInfo->PopString();	
	ePipeline* ExePipe    = (ePipeline*)RequestInfo->GetData(0);
	
	//生成子对话	
	
	tstring TaskName = _T("LogicTask");
	int64 TaskID = AbstractSpace::CreateTimeStamp();	

	CLogicDialog* ChildDialog = GetBrainData()->CreateNewDialog(this,Dialog->m_SourceID,TaskID,0,Dialog->m_SourceName,TaskName,DIALOG_OTHER_TEMP_CHILD,Dialog->m_OutputSourceID,TASK_OUT_DEFAULT);
	
	if (!ChildDialog)
	{
		
		tstring text = _T("The task transferred fail");
		Dialog->SaveSendItem(text,0);				
		
		CMsg rMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);		
		
		ePipeline& rLetter = rMsg.GetLetter();	
		ExePipe->SetID(RETURN_ERROR);	
		ExePipe->SetLabel(text.c_str());
		rLetter.PushPipe(*ExePipe);
		
		Dialog->SendMsg(rMsg,REMOTE);
		
		return false;
	}
	
	

    ePipeline Address(SourceID);
	Address.PushInt(EventID);

    GetBrainData()->PushBrainEvent(TaskID,*ExePipe,Address);
	

	tstring s = Format1024(_T("Create New Dialog[%s]: Type=%d  DialogID=%I64d \n"),
		TaskName.c_str(),ChildDialog->m_DialogType,ChildDialog->m_DialogID);	
	
	Dialog->RuntimeOutput(0,s);
	
	tstring text = _T("The task has been transferred to the sub-dialogue processing");
	Dialog->SaveSendItem(text,0);
	
	
	CNotifyState nf(NOTIFY_DIALOG_LIST);
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
		Dialog->SendMsg(rMsg,REMOTE);
		
		//关闭对话	
		return false;
	}
    
	//如果成功理解则准备执行
	ePipeline ChildMsg(GUI_TASK_CONTROL);
	ChildMsg.PushInt(CMD_EXE);			
	CMsg Msg1;
	CreateBrainMsg(Msg1,ChildDialog->m_DialogID,ChildMsg,EventID);		
	
	PushNerveMsg(Msg1,false,false);		

	return true;
}

bool CBrain::OnRequestTransTask(CLogicDialog* Dialog,ePipeline* RequestInfo){

	int64 EventID = RequestInfo->PopInt();
	
	tstring TaskName = _T("LogicTask");
		
	CLogicDialog* TaskDialog = GetBrainData()->CreateNewDialog(this,Dialog->m_SourceID,EventID,0,Dialog->m_SourceName,TaskName,DIALOG_SYSTEM_CHILD,Dialog->m_OutputSourceID,TASK_OUT_DEFAULT);
	if (!TaskDialog)
	{
		tstring text = _T("The task transferred fail");
		Dialog->SaveSendItem(text,0);

		Dialog->SetTaskState(TASK_IDLE);
		Dialog->NotifyTaskState();
		Dialog->ResetThink();
		return false;
	}
	
	//生成一个与任务对话对应的事件
	ePipeline ExePipe;
	ePipeline Address;
	Address.PushInt(EventID);

	GetBrainData()->PushBrainEvent(EventID,ExePipe,Address);

	
	if (Dialog->m_DialogType==DIALOG_OTHER_MAIN)
	{
		TaskDialog->m_DialogType = DIALOG_OTHER_CHILD;
	}
	
	tstring s = Format1024(_T("Create New Dialog[%s]: Type=%d  DialogID=%I64d\n"),
		TaskName.c_str(),TaskDialog->m_DialogType,TaskDialog->m_DialogID);	
	
	Dialog->RuntimeOutput(0,s);
	
	tstring text = _T("The task has been transferred to the sub-dialogue processing");
	
	//把当前逻辑环境转交给新的子对话
	*TaskDialog<<*Dialog;
	
	//如果是系统对话则生成子对话，系统对话一般不执行逻辑任务，转移给子对话后可以重新开放用户输入					
	Dialog->SaveReceiveItem(text,0);
	
	CNotifyState nf(NOTIFY_DIALOG_LIST);
	nf.PushInt(DL_ADD_DIALOG);
	nf.PushInt(TRUE); //是否设置为当前对话
	nf.Notify(TaskDialog);
	
	Dialog->SetTaskState(TASK_IDLE);
	Dialog->NotifyTaskState();
	Dialog->ResetThink();
	
	ePipeline ChildMsg(GUI_TASK_CONTROL);
	ChildMsg.PushInt(CMD_EXE);
	
	CMsg BrainMsg;
	CreateBrainMsg(BrainMsg,TaskDialog->m_DialogID,ChildMsg,0);	
	PushNerveMsg(BrainMsg,false,false);
	
	return true;
}

bool CBrain::OnRequestInsertLogic(CLogicDialog* Dialog,ePipeline* RequestInfo)
{
	int64 EventID = RequestInfo->PopInt();

	tstring InsertLogicName = RequestInfo->PopString();
	ePipeline* InsertAddress = (ePipeline*)RequestInfo->GetData(0); 
	
	CMsg EltMsg(*InsertAddress,MSG_ELT_INSERT_LOGIC,EventID);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushString(InsertLogicName);
	
	//直接把信息压入任务队列，并且优先处理
	Dialog->m_TaskMsgList.PushUrgence(EltMsg.Release());

	return true;
}

bool CBrain::OnRequestRemoveLogic(CLogicDialog* Dialog,ePipeline* RequestInfo)
{
	int64 EventID = RequestInfo->PopInt();
	
	ePipeline* RemoveAddress = (ePipeline*)RequestInfo->GetData(0); 
    int64 ChildID  =   	*(int64*)RequestInfo->GetData(1); 

	CMsg EltMsg(*RemoveAddress,MSG_ELT_REMOVE_LOGIC,EventID);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushInt(ChildID);
	
	//直接把信息压入任务队列，并且优先处理
	Dialog->m_TaskMsgList.PushUrgence(EltMsg.Release());

	return true;
}