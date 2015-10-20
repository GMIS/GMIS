#pragma warning (disable:4786)

#include "Brain.h"
#include "LogicDialog.h"
#include "GUIMsgDefine.h"

void CBrain::BrainDialogProc(CLogicDialog* Dialog,CMsg& Msg){
	int64 MsgID=Msg.GetMsgID();
	
	switch(MsgID)
	{
	case MSG_LINKER_ERROR:
		OnLinkerError(Dialog,Msg);
		break;
	case MSG_WHO_ARE_YOU:
		OnWhoAreYou(Dialog,Msg);
		break;
	case MSG_I_AM:
		OnI_AM(Dialog,Msg);
		break;
	case MSG_INIT_BRAIN:
		OnInitBrain(Dialog,Msg);
		break;
	case MSG_APP_RUN:
		OnAppRun(Dialog,Msg);
		break;
	case MSG_APP_PAUSE:
		OnAppPause(Dialog,Msg);
		break;
	case MSG_CONNECT_OK:
		OnConnectOK(Dialog,Msg);
		break;
	case MSG_GUI_TO_BRAIN:
		OnGUI2Brain(Dialog,Msg);
		break;
	case MSG_GET_LINKERLIST:
		OnGetLinkerList(Dialog,Msg);
		break;
	case MSG_TASK_FEEDBACK:
		OnTaskFeedback(Dialog,Msg);
		break;
	case MSG_SPACE_CATALOG:
		OnSpaceCatalog(Dialog,Msg);
        break;
	case MSG_SPACE_ADDED:
		OnSpaceAdded(Dialog,Msg);
		break;
	case MSG_SPACE_DELETED:
		OnSpaceDeleted(Dialog,Msg);
		break;
	case MSG_SPACE_SEND_OBJECT:
		OnSpaceSendObject(Dialog,Msg);
		break;
	case MSG_SPACE_ACCESS_DENIED:
		OnSpaceAccessDenied(Dialog,Msg);
		break;
	case MSG_OPERATOR_FEEDBACK:
		OnOperatorFeedback(Dialog,Msg);
		break;
	case MSG_ROBOT_EXPORT_OBJECT:
		OnRobotExportObject(Dialog,Msg);
		break;
	case MSG_TASK_REQUEST:
		OnTaskRequest(Dialog,Msg);
		break;
	case MSG_SYS_MSG:
		OnSysMsg(Dialog,Msg);
		break;
	default:
		TaskDialogProc(Dialog,&Dialog->m_ExePipe,Msg);
		break;
	}
}

void CBrain::OnInitBrain(CLogicDialog* Dialog,CMsg& Msg){

	ePipeline Clause;
    Clause.PushInt(INSTINCT_BRAIN_INIT);

	ePipeline Sentence;
	Sentence.PushPipe(Clause);

	Dialog->ResetTask();
	Dialog->ResetThink();

	CLogicTask* Task = Dialog->GetTask();
    Task->m_LogicText = _T("Check memory...");
    bool ret = Task->Compile(Dialog,&Sentence);
    if (!ret)
    {
		OutSysInfo(_T("Check memory....failed"));
		return;
	}

	Task->ResetTime();
    Dialog->m_LogicItemTree.Clear();
	Dialog->m_LogicItemTree.SetID(Task->m_BeginTime);

	OutSysInfo(_T("Check memory...."));

	
	//如果成功理解则准备执行
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_DialogID);
	Receiver.PushInt(Dialog->m_TaskID);
				
	CMsg TaskMsg(Receiver,MSG_ELT_TASK_CTRL,0);
	TaskMsg.GetLetter().PushInt(CMD_RUN);
				
	PushCentralNerveMsg(TaskMsg,false,false); 
				
}
void CBrain::OnLinkerError(CLogicDialog*Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	
	int64 SourceID  = Letter.PopInt();
	
    CLinker Linker;
	GetLinker(SourceID,Linker);
	
	if (!Linker.IsValid())
	{
		return;
	}
	
	if (Linker().GetRecoType()==LINKER_DEL)
	{
		return;
	}
	
	if (Linker().IsClientLinker())
	{
		GetClientLinkerList()->DeleteLinker(SourceID);
	}else{
		GetSuperiorLinkerList()->DeleteLinker(SourceID);
	}

	//得到链接失效的Main Dialog
	Dialog = GetBrainData()->GetDialog(SourceID);

	if (Dialog==NULL)
	{
		return;
	}

	if (SourceID==SPACE_SOURCE)
	{
		GetBrainData()->DeleteDialogOfSource(SourceID);
		
		tstring ip ;
		int32 GuiID = LOCAL_GUI;
		ePipeline Receiver;
		Receiver.PushInt(Dialog->m_SourceID);
		Receiver.PushInt(Dialog->m_DialogID);
		
		CMsg GuiMsg2(Receiver,MSG_BRAIN_TO_GUI,0);		
		ePipeline Cmd(GUI_CONNECT_STATE);
		Cmd.PushInt(CON_END);
		Cmd.PushString(ip);
		Cmd.PushInt(FALSE);	
		GuiMsg2.GetLetter().PushPipe(Cmd);
		
		SendMsgToGUI(GuiID,GuiMsg2);
		return;
	}
	
	ePipeline Data;
    Data.PushInt(DL_DEL_DIALOG);
	if (Letter.GetEnergy(0)->EnergyType() == TYPE_INT)
	{	
		int32 ErrorID   = Letter.PopInt();
		tstring s = Format1024(_T("Linker Closed:%d"),ErrorID);
		Data.PushString(s);
	}else {
		assert(Letter.GetEnergy(0)->EnergyType()  == TYPE_STRING);
		tstring s = Letter.PopString();
		Data.PushString(s);
	}
    
	CNotifyState nf(NOTIFY_DIALOG_LIST);
	nf<<Data;
    nf.Notify(Dialog);
	
	GetBrainData()->DeleteDialogOfSource(SourceID);

}


void CBrain::OnWhoAreYou(CLogicDialog*Dialog,CMsg& Msg){
	
	ePipeline& Sender = Msg.GetSender();
		
    int64 SenderID = Msg.GetSenderID();
	int64 SourceID = Msg.GetSourceID();
	

	CLinker Linker;
	GetSuperiorLinkerList()->GetLinker(SourceID,Linker);
	
	if(!Linker.IsValid())return;
	
#ifdef _DEBUG
	Linker().SetStateOutputLevel(LIGHT_LEVEL);
#endif

	CMsg rMsg(SenderID,MSG_I_AM,Dialog->m_DialogID,0);
	
	ePipeline& rLetter=rMsg.GetLetter();
	
	//	eINT64*  ID = new eINT64(m_Alias);
	eSTRING* Name = new eSTRING(GetName());
	eSTRING* Cryp = new eSTRING();
	//	rLetter->Push_Directly(ID);
	rLetter.Push_Directly(Name);
	rLetter.Push_Directly(Cryp);
	rLetter.PushInt(DIALOG_OTHER_MAIN);
	
	
	Linker().PushMsgToSend(rMsg);
}

void CBrain::OnI_AM(CLogicDialog*Dialog,CMsg& Msg){
	
	int64 EventID = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter();
	ePipeline& Sender = Msg.GetSender();
	
	int64 SourceID  = Msg.GetSourceID();
	int64 SenderID = Msg.GetSenderID();
	
	CLinker Linker;
	GetClientLinkerList()->GetLinker(SourceID,Linker);

#ifdef _DEBUG
	Linker().SetStateOutputLevel(LIGHT_LEVEL);
#endif	
	
	Letter.AutoTypeAB();	
	if(Letter.GetTypeAB() != 0x33000000 ){ //只含两个字符串
		assert(0);
		GetClientLinkerList()->DeleteLinker(Linker().GetSourceID());
		return ;
	};
	
	tstring  Name        = *(tstring*)Letter.GetData(0);  
	tstring& Cryptograhp = *(tstring*)Letter.GetData(1);


	//验证身份
	DIALOG_TYPE DialogType  = VerifyUser(Name,Cryptograhp);

	if(DialogType == DIALOG_SYSTEM_MAIN){
		//主人远程连接
		assert(GetName() == Name);
		
		//通知连接成功
		CMsg rMsg(SenderID,MSG_CONNECT_OK,NULL,EventID);
		rMsg.GetLetter().PushString(GetName());
		Linker().PushMsgToSend(rMsg);	


		//直接把当前系统对话指向此网络连接
		Dialog->m_SourceID = SourceID;

		//向此连接通知对话列表


	}else if (DialogType == DIALOG_SYSTEM_MODEL)
	{
		//器官，只允许一次性连接
	}
	else{
		//非法连接
		Linker().Close();
		GetClientLinkerList()->DeleteLinker(Linker().GetSourceID());
	}

	CLogicDialog* NewDialog = NULL;
	CLockedBrainData* LockedData = GetBrainData();


	if (DialogType == DIALOG_SYSTEM_MODEL) //一个用户界面连接，其对话的输出源ID将是其自己
	{

		//验证是否是系统模块，并得到其类型
		bool ret = Login(SourceID,Name,Cryptograhp);
		if (!ret)//说明此连接非法
		{
			Linker().Close();
			GetClientLinkerList()->DeleteLinker(Linker().GetSourceID());
		}
	}
	else{
		NewDialog = LockedData->CreateNewDialog(this,SourceID,DEFAULT_DIALOG,0,Name,_T("You"),DIALOG_OTHER_MAIN,0,TASK_OUT_DEFAULT);	
	
		assert(NewDialog);
		NewDialog->m_DialogType = (DIALOG_TYPE)DialogType;
		NewDialog->m_DialogName = Name;
		
	    CNotifyState nf(NOTIFY_DIALOG_LIST);
		nf.PushInt(DL_ADD_DIALOG);
		nf.PushInt(FALSE);
		nf.Notify(NewDialog);
	}	

	tstring s = Format1024(_T("Dialog(%s) Accepted."),Name.c_str());
	OutSysInfo(s);
    

	if(!Linker.IsValid())return;


	
}


void CBrain::OnAppRun(CLogicDialog* Dialog,CMsg& Msg){
	int64 EnventID = Msg.GetEventID();
	assert(EnventID);
	
	ePipeline& Letter = Msg.GetLetter();	
	
	int64 CmdType = Letter.PopInt();
		
	int64 DialogID  = Letter.PopInt();	
	
	tstring LogicText = Letter.PopString();	
	ePipeline* ExePipe    = (ePipeline*)Letter.GetData(0);
	
	//生成子对话	
	tstring TaskName = _T("LogicTask");
	int64 TaskID = AbstractSpace::CreateTimeStamp();	
	CLogicDialog* ChildDialog = GetBrainData()->CreateNewDialog(this,Dialog->m_SourceID,TaskID,0,Dialog->m_SourceName,TaskName,DIALOG_OTHER_TEMP_CHILD,Dialog->m_OutputSourceID,TASK_OUT_DEFAULT);
	
	if (!ChildDialog)
	{
		
		tstring text = _T("The task transferred fail");
		Dialog->SaveSendItem(text,0);				
		
		CMsg rMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EnventID);		
		ePipeline& rLetter = rMsg.GetLetter();	
		rLetter.PushInt(0);
		
		ExePipe->SetLabel(text.c_str());
		rLetter.PushPipe(*ExePipe);
		
		Dialog->SendMsg(rMsg,REMOTE);
		
		return ;
	}
	
	
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
		CMsg rMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EnventID);		
		ePipeline& rLetter = rMsg.GetLetter();	
		rLetter.PushInt(0);
		
		
		ExePipe->GetLabel()= _T("Think error!");
		rLetter.PushPipe(*ExePipe);		
		Dialog->SendMsg(rMsg,REMOTE);
		
		//关闭对话	
		return;
	}
    
	//如果成功理解则准备执行
	ePipeline ChildMsg(GUI_TASK_CONTROL);
	ChildMsg.PushInt(CMD_EXE);			
	CMsg Msg1;
	CreateBrainMsg(Msg1,ChildDialog->m_DialogID,ChildMsg,Msg.GetEventID());		

	PushNerveMsg(Msg1,false,false);	
};

void CBrain::OnAppPause(CLogicDialog* Dialog,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
    int64 SourceID = Msg.GetSourceID();
	
	ePipeline& Letter = Msg.GetLetter();
	tstring s = Letter.PopString();
	

	
};

void CBrain::OnConnectOK(CLogicDialog*Dialog,CMsg& Msg){
		
	int64 SourceID = Msg.GetSourceID();
	ePipeline& Letter = Msg.GetLetter();
	tstring Name;
	Letter.PopString(Name);
	
	CLinker Linker;
	GetSuperiorLinkerList()->GetLinker(SourceID,Linker);
	
	if(!Linker.IsValid()){
		return;
	}
	
	Linker().SetRecoType(LINKER_FRIEND);
	
	if (SourceID == SPACE_SOURCE)
	{
		CMsg rMsg(MSG_ROBOT_GOTO_SPACE,NULL,NULL);
		ePipeline& rLetter = rMsg.GetLetter();
		
		ePipeline Addr(LOCAL_SPACEID);
		rLetter.PushPipe(Addr);  //意味着要求获得根空间目录
        Dialog->SendMsg(rMsg,REMOTE);
		return;
	}
	
	CLockedBrainData* LockedData = GetBrainData();
	CLogicDialog* NewDialog = LockedData->CreateNewDialog(this,SourceID,DEFAULT_DIALOG,0,Name,_T("You"),DIALOG_OTHER_MAIN,LOCAL_GUI,TASK_OUT_DEFAULT);	
	
	CNotifyState nf(NOTIFY_DIALOG_LIST);
	nf.PushInt(DL_ADD_DIALOG);
	nf.Notify(NewDialog);
   	
	OutSysInfo(_T("Connection ok!"));
}


void CBrain::OnGUI2Brain(CLogicDialog* Dialog,CMsg& Msg){
    int64 GuiSourceID = Msg.GetSourceID();
	
	/*
	if (GetBrainData()->GetOrganType(GuiSourceID) != ORGAN_TYPE_GUI)
	{
		NotifySysState(MNOTIFY_ILLEGAL_MSG, &Msg.GetMsg());
		return; 
	}
	*/
	
	ePipeline& Letter = Msg.GetLetter();
	
	int64 SourceID = Letter.PopInt();
	int64 DialogID = Letter.PopInt();
	
	ePipeline& ChildMsg = *(ePipeline*)Letter.GetData(0);
    int64 ChildMsgID = ChildMsg.GetID();

	DialogID = DialogID==0?SourceID:DialogID;				
	
	CLogicDialog* TheDialog = GetBrainData()->GetDialog(DialogID);

	ePipeline ReceiverInfo;
	ReceiverInfo.PushInt(DialogID);

	if (TheDialog) //这部分或许应该转到TaskDialogMsgProc去完成，暂时放在这里
	{		
		//以下两个信息将激活事件处理子对话
		if (ChildMsgID==GUI_IO_INPUTING || ChildMsgID==GUI_IO_INPUTED)
		{
			TASK_STATE  State = TheDialog->GetTaskState();
			if (State == TASK_RUN || State == TASK_PAUSE)
			{			
				assert(TheDialog->m_ControlDialogID);
				CLogicDialog* ChildDialog  = GetBrainData()->GetDialog(TheDialog->m_ControlDialogID);
				if (ChildDialog == NULL)
				{
					ChildDialog = GetBrainData()->CreateNewDialog(
						this,SourceID,TheDialog->m_ControlDialogID,
						DialogID,TheDialog->m_SourceName,
						_T("Event"),DIALOG_SYSTEM_CHILD,GuiSourceID,TASK_OUT_DEFAULT);
					
					if (!ChildDialog)
					{
						tstring text = _T("Create Control dialog fail");
						TheDialog->RuntimeOutput(0,text);
						assert(0); //这里或许应该添加更多错误处理
						return ;
					}						
				}	
				TheDialog = ChildDialog;
				ReceiverInfo.Clear();
				ReceiverInfo.PushInt(ChildDialog->GetDialogID());
			}
		}
	};

	if(TheDialog){
		CMsg NewMsg;
		CreateBrainMsg(NewMsg,ReceiverInfo,ChildMsg,Msg.GetEventID());
		TheDialog->Do(NewMsg);		
	}else{
		NotifySysState(MNOTIFY_ILLEGAL_MSG, &Msg.GetMsg());
		return;
	}
	
};


void CBrain::OnGetLinkerList(CLogicDialog*Dialog,CMsg& Msg){
    ePipeline LinkerInfo;
	
	GetBrainData()->GetAllDialogListInfo(LinkerInfo);
	
	int64 SenderID = Msg.GetSenderID();
	CMsg rMsg(Dialog->m_SourceID,SenderID,MSG_SEND_LINKERLIST,Dialog->m_DialogID);
	rMsg.GetLetter().PushPipe(LinkerInfo);	
	Dialog->SendMsg(rMsg,LOCAL);
}


void CBrain::OnTaskFeedback(CLogicDialog* Dialog,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();	
	ePipeline* ExePipe = (ePipeline*)Letter.GetData(0);
	
	CLockedBrainData* BrainData = GetBrainData();
	
	ePipeline EventInfo;
	if(!BrainData->PopBrainEvent(EventID,EventInfo)){ 
		//事件消失那么对应的对话也应该删除
		CLogicDialog* EventDlg = GetBrainData()->GetDialog(EventID);
		if (EventDlg)
		{
			CNotifyState nf(NOTIFY_DIALOG_LIST);
			nf.PushInt(DL_DEL_DIALOG);
			nf.Notify(EventDlg);
			
			GetBrainData()->DeleteDialog(EventID);
		}
		return;
	};
	
	ePipeline* EventAddress = (ePipeline*)EventInfo.GetData(EVENT_ADDRESS);
	ePipeline* OldExePipe   = (ePipeline*)EventInfo.GetData(EVENT_PIPE);

	if (EventAddress->m_ID == 0) //本地事件
	{
		CMsg m(*EventAddress,MSG_TASK_RESULT,EventID);
		ePipeline& Letter = m.GetLetter();

		Letter.PushPipe(*OldExePipe);
		Letter.PushPipe(*ExePipe);		
		PushNerveMsg(m,false,false);	
	} 
	else  //远方事件则转发给对方
	{
		int64 RemoteEventID = EventAddress->PopInt();
		int64 LinkerID = EventAddress->m_ID;
		CLinker Linker;
		GetLinker(LinkerID,Linker);
		if (Linker.IsValid())
		{
			CMsg m(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,RemoteEventID);
			m.GetLetter().PushPipe(*ExePipe);
		}
	}
}


void CBrain::OnSpaceCatalog(CLogicDialog* Dialog,CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd(GUI_SPACE_OUTPUT);
	Cmd.PushInt(SPACE_CATALOG);
	Cmd<<Letter;
	
    GuiMsg.GetLetter().PushPipe(Cmd);
	
	int32 GuiID = LOCAL_GUI;
	SendMsgToGUI(GuiID,GuiMsg);
}

void CBrain::OnSpaceAdded(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd(GUI_SPACE_OUTPUT);
	Cmd.PushInt(SPACE_ADDED);
	Cmd<<Letter;
	
    GuiMsg.GetLetter().PushPipe(Cmd);
	
	int32 GuiID = LOCAL_GUI;
	SendMsgToGUI(GuiID,GuiMsg);
};

void CBrain::OnSpaceDeleted(CLogicDialog* Dialog,CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd(GUI_SPACE_OUTPUT);
	Cmd.PushInt(SPACE_DELETED);
	Cmd<<Letter;
	
    GuiMsg.GetLetter().PushPipe(Cmd);
	
	int32 GuiID = LOCAL_GUI;
	SendMsgToGUI(GuiID,GuiMsg);
};


void CBrain::OnSpaceSendObject(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	;
	
	ePipeline Cmd(GUI_SPACE_OUTPUT);
	Cmd.PushInt(SPACE_SEND_OBJECT);
	Cmd<<Letter;
	
    GuiMsg.GetLetter().PushPipe(Cmd);
	
	int32 GuiID = LOCAL_GUI;
	SendMsgToGUI(GuiID,GuiMsg);
};

void CBrain::OnSpaceAccessDenied(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd(GUI_SPACE_OUTPUT);
	Cmd.PushInt(SPACE_ACESS_DENIED);
	Cmd<<Letter;
	
    GuiMsg.GetLetter().PushPipe(Cmd);
	
	int32 GuiID = LOCAL_GUI;
	SendMsgToGUI(GuiID,GuiMsg);
};



void CBrain::OnOperatorFeedback(CLogicDialog* Dialog,CMsg& Msg){
	
	int64 SourceID = Msg.GetSourceID();
	
	int64 EventID = Msg.GetEventID();
	
	
	CLockedBrainData* BrainData = GetBrainData();
	
	ePipeline ObjectInfo;
	if(!BrainData->PopBrainEvent(EventID,ObjectInfo)){
		return;
	};
	
	ePipeline* ObjectAddress = (ePipeline*)ObjectInfo.GetData(EVENT_ADDRESS);
	
	
	CMsg Msg1(ObjectAddress->GetID(),MSG_OPERATOR_FEEDBACK,0,EventID);
	ePipeline& Receiver = Msg1.GetReceiver();
	Receiver<<*ObjectAddress;
	
	ePipeline& Letter1 = Msg1.GetLetter();
    Letter1<<Msg.GetLetter();
	
	PushNerveMsg(Msg1,false,false);	
};

void CBrain::OnRobotExportObject(CLogicDialog* Dialog,CMsg& Msg){
	
	int64 SourceID = Msg.GetSourceID();
	int64 EventID = Msg.GetEventID();
	
	
	CLockedBrainData* BrainData = GetBrainData();
	
	ePipeline ObjectInfo;
	if(!BrainData->PopBrainEvent(EventID,ObjectInfo)){
		return;
	};
	
	ePipeline* ObjectAddress = (ePipeline*)ObjectInfo.GetData(EVENT_ADDRESS);
	
	
	CMsg Msg1(ObjectAddress->GetID(),MSG_ROBOT_EXPORT_OBJECT,0,EventID);
	ePipeline& Receiver = Msg1.GetReceiver();
	Receiver<<*ObjectAddress;
	
	ePipeline& Letter1 = Msg1.GetLetter();
    Letter1<<Msg.GetLetter();
	
	PushNerveMsg(Msg1,false,false);	
}


void CBrain::OnTaskRequest(CLogicDialog* Dialog, CMsg& Msg)
{
	int64 SourceID = Msg.GetSourceID();
	if (SourceID == SYSTEM_SOURCE)
	{
		ePipeline& Letter = Msg.GetLetter();
		ePipeline& RequestInfo = *(ePipeline*)Letter.GetData(0);
		
		CRequestBrain Rq(RequestInfo);
		Rq.Request(Dialog);
	}else{
		assert(0); //暂时不接受外部请求
	}

}

void CBrain::OnSysMsg(CLogicDialog* Dialog, CMsg& Msg){
	int64 SourceID = Msg.GetSourceID();
	if (SourceID == SYSTEM_SOURCE)
	{

		ePipeline& Letter = Msg.GetLetter();
		
		int64 SysMsgID = Letter.PopInt();
		switch(SysMsgID){
		case NOTIFY_SYS_STATE:
			{
				int64 NotifyID = Letter.PopInt();
				ePipeline* NotifyData = (ePipeline*)Letter.GetData(0);
				NotifySysState(NotifyID,NotifyData);
			}	
		}
	}
}