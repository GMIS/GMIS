#pragma warning (disable:4786)

#include "Brain.h"
#include "LogicDialog.h"
#include "GUIMsgDefine.h"

void CBrain::CentralNerveMsgProc(CMsg& Msg){

	int64 SourceID = Msg.GetSourceID();
	int64 DialogID = Msg.GetReceiverID();
	int64 MsgID=Msg.GetMsgID();

	if(DialogID!=DEFAULT_DIALOG){	
		//转向子神经处理，避免中枢神经阻塞
		PushNerveMsg(Msg,false,false);
		return;
	}

	CLogicDialog* MainDialog = m_BrainData.GetDialog(SourceID,DialogID);
	if (MainDialog==NULL) //对于网络登录，对方发来MSG_I_AM信息时，还没有建立对话，此时交给系统缺省对话处理
	{
		if (MsgID == MSG_I_AM)
		{
			MainDialog = m_BrainData.GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
			assert(MainDialog);
		}else{
			return ;
		}
	}

	CLogicDialog::AutoSysProcCounter  Counter(MainDialog);

	switch(MsgID)
	{
	case MSG_EVENT_TICK:
		{
			int64 EventID = Msg.GetEventID();
			m_BrainData.ResetEventTickCount(EventID);
			if(SourceID==SYSTEM_SOURCE){
				m_BrainData.BrainIdleProc(this);
			}
			//MainDialog->SaveReceiveItem(_T("ResetEventTickCount"),0);
		}
		break;
	case MSG_WHO_ARE_YOU:
		OnWhoAreYou(MainDialog,Msg);
		break;
	case MSG_I_AM:
		OnI_AM(MainDialog,Msg);
		break;
	case MSG_INIT_BRAIN:
		OnInitBrain(MainDialog,Msg);
		break;
	case MSG_APP_RUN:
		OnAppRun(MainDialog,Msg);
		break;
	case MSG_APP_PAUSE:
		OnAppPause(MainDialog,Msg);
		break;
	case MSG_CONNECT_OK:
		OnConnectOK(MainDialog,Msg);
		break;
	case MSG_GUI_TO_BRAIN:
		OnGUI2Brain(MainDialog,Msg);
		break;
	case MSG_GET_DIALOGLIST:
		OnGetDialogList(MainDialog,Msg);
		break;
	case MSG_TASK_FEEDBACK:
		OnTaskFeedback(MainDialog,Msg);
		break;
	case MSG_SPACE_CATALOG:
		OnSpaceCatalog(MainDialog,Msg);
        break;
	case MSG_SPACE_ADDED:
		OnSpaceAdded(MainDialog,Msg);
		break;
	case MSG_SPACE_DELETED:
		OnSpaceDeleted(MainDialog,Msg);
		break;
	case MSG_SPACE_SEND_OBJECT:
		OnSpaceSendObject(MainDialog,Msg);
		break;
	case MSG_SPACE_ACCESS_DENIED:
		OnSpaceAccessDenied(MainDialog,Msg);
		break;
	case MSG_OPERATOR_FEEDBACK:
		OnOperatorFeedback(MainDialog,Msg);
		break;
	case MSG_ROBOT_EXPORT_OBJECT:
		OnRobotExportObject(MainDialog,Msg);
		break;
	case MSG_TASK_REQUEST:
		OnTaskRequest(MainDialog,Msg);
		break;
/*
	case MSG_CLOSE_DIALOG:
		OnCloseDialog(MainDialog,Msg);
		break;
*/
	default:
		NotifySysState(NOTIFY_ILLEGAL_MSG, NULL,&Msg.GetMsg());
		break;
	}
}

void CBrain::OnInitBrain(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline ClientExePipe;
	ePipeline ClientAddress(SYSTEM_SOURCE);
	ClientAddress.PushInt(DEFAULT_DIALOG);

	StartSysTask(INSTINCT_BRAIN_INIT,_T("check memory"),0,ClientAddress,ClientExePipe,true);		
}

void CBrain::OnWhoAreYou(CLogicDialog* Dialog,CMsg& Msg){
	
	ePipeline& Sender = Msg.GetSender();
		
    int64 SenderID = Msg.GetSenderID();
	int64 SourceID = Msg.GetSourceID();
	

	CLinker Linker;
	GetSuperiorLinkerList()->GetLinker(SourceID,Linker);
	
	if(!Linker.IsValid())return;
	
#ifdef _DEBUG
	Linker().SetStateOutputLevel(LIGHT_LEVEL);
#endif

	CMsg rMsg(SYSTEM_SOURCE,SenderID,MSG_I_AM,DEFAULT_DIALOG,0);
	
	ePipeline& rLetter=rMsg.GetLetter();
	
	const COrgan& Organ = m_BrainData.GetOrgan(LOCAL_GUI_SOURCE);

	//	eINT64*  ID = new eINT64(m_Alias);
	eSTRING* Name = new eSTRING(Organ.m_Name);
	eSTRING* Cryp = new eSTRING(Organ.m_CryptTxt);
	//	rLetter->Push_Directly(ID);
	rLetter.Push_Directly(Name);
	rLetter.Push_Directly(Cryp);
	rLetter.PushInt(DIALOG_SYSTEM);
	
	
	Linker().PushMsgToSend(rMsg);
}

void CBrain::OnI_AM(CLogicDialog* Dialog,CMsg& Msg){
	
	int64 EventID = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter();
	ePipeline& Sender = Msg.GetSender();
	
	int64 SourceID  = Msg.GetSourceID();
	int64 SenderID = Msg.GetSenderID();
	
	CLinker Linker;
	m_ClientLinkerList.GetLinker(SourceID,Linker);
	if (!Linker.IsValid())
	{
		m_WebsocketClientList.GetLinker(SourceID,Linker);
	}

	if (!Linker.IsValid())
	{
		OutputLog(LOG_WARNING,_T("Linker(%I64ld) is invalid for MSG_I_AM"),SourceID);
		assert(0);
		return;
	}
#ifdef _DEBUG
	Linker().SetStateOutputLevel(LIGHT_LEVEL);
#endif	
	
	if(!Letter.HasTypeAB(0x3300000)){ //两个字符串
		if (Linker().GetLinkerType()== CLIENT_LINKER)
		{
			m_ClientLinkerList.DeleteLinker(SourceID);
		} 
		else
		{
			m_WebsocketClientList.DeleteLinker(SourceID);
		}
		
		return ;
	};
	
	tstring  Name        = *(tstring*)Letter.GetData(0);  
	tstring& Cryptograhp = *(tstring*)Letter.GetData(1);


	//验证身份
	ePipeline UserInfo;
	bool ret = m_BrainData.FindRegisterUserInfo(Name,Cryptograhp,UserInfo);
	if(!ret){
		//非法连接
		if (Linker().GetLinkerType()== CLIENT_LINKER)
		{
			m_ClientLinkerList.DeleteLinker(SourceID);
		} 
		else
		{
			m_WebsocketClientList.DeleteLinker(SourceID);
		}
		return;
	}

	DIALOG_TYPE DialogType  = (DIALOG_TYPE)(*(int64*)UserInfo.GetData(2));

	if(DialogType == DIALOG_SYSTEM){
		//主人远程连接,通知连接成功
		
		ret = Login(SourceID,Name,Cryptograhp);
		if(ret){

			tstring s = Format1024(_T("User(%s) Login."),Name.c_str());
			OutSysInfo(s.c_str());

			CMsg rMsg(SYSTEM_SOURCE,SenderID,MSG_CONNECT_OK,DEFAULT_DIALOG,EventID);
			rMsg.GetLetter().PushString(Name);

			Linker().PushMsgToSend(rMsg);	

			CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
			nf.PushInt(DL_LOGIN_ONE);
			nf.PushInt(SourceID);
			nf.Notify(Dialog);

		}else{
			Linker().Close();
			GetClientLinkerList()->DeleteLinker(Linker().GetSourceID());
		}

	}else if (DialogType == DIALOG_MODEL)
	{
		//器官，只允许一次性连接
	}
	else{
		//非法连接
		Linker().Close();
		GetClientLinkerList()->DeleteLinker(Linker().GetSourceID());
	}

}


void CBrain::OnAppRun(CLogicDialog* Dialog,CMsg& Msg){
	/*
	int64 EnventID = Msg.GetEventID();
	assert(EnventID);
	
	ePipeline& Letter = Msg.GetLetter();	
	
	int64 CmdType = Letter.PopInt();
		
	int64 DialogID  = Letter.PopInt();	
	
	tstring LogicText  = Letter.PopString();	
	ePipeline* ExePipe = (ePipeline*)Letter.GetData(0);
	
	//生成子对话	
	tstring TaskName = _T("LogicTask");
	int64 NewDialogID = AbstractSpace::CreateTimeStamp();	

	CLogicDialog* ChildDialog = GetBrainData()->CreateNewDialog(this,SYSTEM_SOURCE,NewDialogID,0,_T("System"),TaskName,DIALOG_OTHER_TEMP_CHILD,TASK_OUT_DEFAULT);
	if (!ChildDialog)
	{
		tstring text = _T("The task transferred fail");
		CLogicDialog* SystemDialog = m_BrainData.GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
		SystemDialog->SaveSendItem(text,0);				
		
		CMsg rMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EnventID);		
		ePipeline& rLetter = rMsg.GetLetter();	
		rLetter.PushInt(0);
		
		ExePipe->SetLabel(text.c_str());
		rLetter.PushPipe(*ExePipe);
		
		CLinker Linker;
		GetLinker(Msg.GetSourceID(),Linker);
		if (Linker.IsValid())
		{
			Linker().PushMsgToSend(rMsg);
		}
		return ;
	}
	
	
	tstring s = Format1024(_T("Create New Dialog[%s]: Type=%d  DialogID=%I64d \n"),
		TaskName.c_str(),ChildDialog->m_DialogType,ChildDialog->m_DialogID);	
	
	CLogicDialog* SystemDialog = m_BrainData.GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
	SystemDialog->RuntimeOutput(0,s);
	
	tstring text = _T("The task has been transferred to the sub-dialogue processing");
	SystemDialog->SaveSendItem(text,0);
	
	
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
		CMsg rMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EnventID);		
		ePipeline& rLetter = rMsg.GetLetter();	
		rLetter.PushInt(0);
		
		
		ExePipe->GetLabel()= _T("Think error!");
		rLetter.PushPipe(*ExePipe);	


		CLinker Linker;
		GetLinker(Msg.GetSourceID(),Linker);
		if (Linker.IsValid())
		{
			Linker().PushMsgToSend(rMsg);
		}

		return;
	}
    
	
	//如果成功理解则生成事件，准备执行
	ePipeline ClientAddress(Msg.GetSourceID());
	ClientAddress.PushInt(NewDialogID);

	m_BrainData.CreateEvent(DialogID,EnventID,*ExePipe,ClientAddress,TIME_SEC,true);


	ePipeline ChildMsg(TO_BRAIN_MSG::TASK_CONTROL::ID);
	ChildMsg.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE);			
	CMsg Msg1;
	CreateBrainMsg(Msg1,ChildDialog->m_DialogID,ChildMsg,Msg.GetEventID());		

	PushNerveMsg(Msg1,false,false);	
	*/
};

void CBrain::OnAppPause(CLogicDialog* Dialog,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
    int64 SourceID = Msg.GetSourceID();
	
	/*
	//如果成功理解则生成事件，准备执行
	ePipeline ClientAddress;
	ePipeline ClientExePipe;
	m_BrainData.PushBrainEvent(DialogID,EventID,ClientExePipe,ClientAddress,MIN_EVENT_INTERVAL,true);


	ePipeline ChildMsg(GUI_TASK_CONTROL);
	ChildMsg.PushInt(CMD_PAUSE);			
	CMsg Msg1;
	CreateBrainMsg(Msg1,ChildDialog->m_DialogID,ChildMsg,Msg.GetEventID());		

	PushNerveMsg(Msg1,false,false);	
	*/
};

void CBrain::OnConnectOK(CLogicDialog* Dialog,CMsg& Msg){
		
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
		//发起一个内部任务，从空间获得目录，然后转发给所有登录用户

		ePipeline ClientExePipe;
		ePipeline ClientAddress((int64)SYSTEM_SOURCE);
		StartSysTask(INSTINCT_GET_SPACECATALOG,_T("Get space catalog"),0,ClientAddress,ClientExePipe,true);				

		return;
	}
	
	CLockedBrainData* LockedData = GetBrainData();
	CLogicDialog* NewDialog = LockedData->CreateNewDialog(this,SPACE_SOURCE,DEFAULT_DIALOG,NO_PARENT,Name,_T("You"),DIALOG_SYSTEM,TASK_OUT_DEFAULT);	
	NewDialog->m_bEditValid = false;

	CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
	nf.PushInt(DL_ADD_DIALOG);
	nf.Notify(NewDialog);
   	
	OutSysInfo(_T("Connection ok!"));
}


void CBrain::OnGUI2Brain(CLogicDialog* Dialog,CMsg& Msg){
    int64 GuiSourceID = Msg.GetSourceID();
	
	if (GetBrainData()->GetOrgan(GuiSourceID).m_DialogType != DIALOG_SYSTEM)
	{
		NotifySysState(NOTIFY_ILLEGAL_MSG,NULL,&Msg.GetMsg());
		return; 
	}
	
	ePipeline& Letter = Msg.GetLetter();
	
	int64 SourceID = Letter.PopInt();
	int64 DialogID = Letter.PopInt();
	
	if (SourceID != GuiSourceID && SourceID != SYSTEM_SOURCE)
	{

		ePipeline DialogInfo;
		m_BrainData.GetInvalidDialogData(DialogInfo);
		CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
		nf.PushInt(INIT_FOCUS);
		nf.PushInt(SourceID);
		nf.PushInt(DialogID);

		nf<<DialogInfo;

		nf.Notify(Dialog);
		return;
	}

	ePipeline& ChildMsg = *(ePipeline*)Letter.GetData(0);
    int64 ChildMsgID = ChildMsg.GetID();

	CLogicDialog* TheDialog = m_BrainData.GetDialog(SourceID,DialogID);

	if(TheDialog){
	
		ePipeline ReceiverInfo;
		ReceiverInfo.PushInt(DialogID);

		CMsg NewMsg(SYSTEM_SOURCE,ReceiverInfo,MSG_FROM_BRAIN,DEFAULT_DIALOG,Msg.GetEventID());
		ePipeline& Letter = NewMsg.GetLetter();
		Letter.PushInt(ChildMsgID);
		Letter<<ChildMsg;

		TheDialog->Do(NewMsg);		
	}else{
		NotifySysState(NOTIFY_ILLEGAL_MSG,NULL, &Msg.GetMsg());
		return;
	}
	
};


void CBrain::OnGetDialogList(CLogicDialog* Dialog,CMsg& Msg){
    ePipeline DialogListInfo;

	GetBrainData()->GetAllDialogListInfo(DialogListInfo);
	
	int64 SenderID = Msg.GetSenderID();
	CMsg rMsg(Msg.GetSourceID(),SenderID,MSG_SEND_LINKERLIST,DEFAULT_DIALOG,0);
	rMsg.GetLetter().PushPipe(DialogListInfo);	
	
	int64 SourceID = Msg.GetSourceID();
	if (SourceID == SYSTEM_SOURCE)
	{
		CLinker Linker;
		GetLinker(SourceID,Linker);
		Linker().PushMsgToSend(Msg);
	}else{
		assert(0);
	}

}


void CBrain::OnTaskFeedback(CLogicDialog* Dialog,CMsg& Msg){
	//只有系统对话能得到此反馈
	assert(Dialog->m_DialogID ==0);

	int64 EventID = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();	
	ePipeline* ExePipe = (ePipeline*)Letter.GetData(0);
	
	CLockedBrainData* BrainData = GetBrainData();
	
	CBrainEvent EventInfo;
	if(!BrainData->GetEvent(EventID,EventInfo,false)){ 
		//assert(0);
		return;
	};
	
	ePipeline& ClientAddress = EventInfo.m_ClientAddress;
	ePipeline& OldExePipe   = EventInfo.m_ClientExePipe;

	int64 ClientSourceID = EventInfo.m_ClientAddress.m_ID;
	int64 ClientDialogID = *(int64*)ClientAddress.GetData(0);

	//如果客户是系统对话，就直接处理结果，否则转发给客户地址对应的对话处理
	if(ClientSourceID == Dialog->m_SourceID && ClientDialogID == Dialog->m_DialogID ){
		
		CLogicDialog* TaskDlg = GetBrainData()->GetDialog(ClientSourceID,EventID);

		if(EventInfo.m_bOnce){
			BrainData->GetEvent(EventID,EventInfo,true);

			assert(!EventInfo.m_bPrivate);

			TaskDlg->SetThinkState(THINK_IDLE);

			CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
			nf.PushInt(DL_DEL_DIALOG);
			nf.Notify(TaskDlg);

			m_BrainData.DeleteDialog(ClientSourceID,EventID);
		}else{ //长期对话交给任务对话自己处理

			CMsg m(ClientSourceID,TaskDlg->m_DialogID,MSG_TASK_RESULT,DEFAULT_DIALOG,EventInfo.m_ClientEventID);
			ePipeline& Letter = m.GetLetter();
			Letter.PushPipe(OldExePipe);
			Letter.PushPipe(*ExePipe);		

			PushNerveMsg(m,false,false);
		}
		
	}else if (ClientSourceID == SPACE_SOURCE)  //如果是客户地址是空间，那把结果发给它
	{
		CLinker Linker;
		GetLinker(ClientSourceID,Linker);
		if (Linker.IsValid())
		{
			CMsg m(ClientSourceID,ClientAddress,MSG_TASK_RESULT,DEFAULT_DIALOG,EventInfo.m_ClientEventID);
			ePipeline& Letter = m.GetLetter();
			Letter.PushPipe(OldExePipe);
			Letter.PushPipe(*ExePipe);		
			Linker().PushMsgToSend(m);
		}
		assert(EventInfo.m_bOnce==0); //空间发来的任务一定是一次性的
	
		CLogicDialog* TaskDlg = GetBrainData()->GetDialog(ClientSourceID,EventID);	
		BrainData->GetEvent(EventID,EventInfo,true);
		CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
		nf.PushInt(DL_DEL_DIALOG);
		nf.Notify(TaskDlg);

		m_BrainData.DeleteDialog(ClientSourceID,EventID);
		
	}else{  //其它对话则处理完事件，把结果发给客户

		if (EventInfo.m_bOnce)
		{
			BrainData->GetEvent(EventID,EventInfo,true);

			if(!EventInfo.m_bPrivate){
				CLogicDialog* TaskDlg = GetBrainData()->GetDialog(ClientSourceID,EventID);
				assert(TaskDlg);
				if(TaskDlg){			

					CLogicDialog* ParentDlg = m_BrainData.GetDialog(ClientSourceID,TaskDlg->m_ParentDialogID);
					if(ParentDlg){
						//事件对话执行完不发MSG_TASK_RESULT给父对话，避免干扰它原来任务的执行
						//直接把它的执行管道信息在客户的运行时view显示
						CPipeView PipeView(ExePipe);
						tstring s  = PipeView.GetString();
						ParentDlg->RuntimeOutput(0,s);
					}
					CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
					nf.PushInt(DL_DEL_DIALOG);
					nf.Notify(TaskDlg);

					m_BrainData.DeleteDialog(TaskDlg->m_SourceID,TaskDlg->m_DialogID);
				}
			}
			return; 
		}		

		CMsg m(ClientSourceID,ClientAddress,MSG_TASK_RESULT,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = m.GetLetter();
		Letter.PushPipe(OldExePipe);
		Letter.PushPipe(*ExePipe);		

		PushNerveMsg(m,false,false);
	}
}


void CBrain::OnSpaceCatalog(CLogicDialog* Dialog,CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(SYSTEM_SOURCE,Receiver,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);
	
	ePipeline Cmd(TO_SPACE_VIEW::ID);
	Cmd.PushInt(TO_SPACE_VIEW::CATALOG);
	Cmd<<Letter;
	
    GuiMsg.GetLetter().PushPipe(Cmd);
	
	GetBrainData()->SendMsgToGUI(this,GuiMsg,-1);
}

void CBrain::OnSpaceAdded(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);

	CMsg GuiMsg(SYSTEM_SOURCE,Receiver,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);
	
	ePipeline Cmd(TO_SPACE_VIEW::ID);
	Cmd.PushInt(TO_SPACE_VIEW::ADD_ITEM);
	Cmd<<Letter;
	
    GuiMsg.GetLetter().PushPipe(Cmd);
	
	GetBrainData()->SendMsgToGUI(this,GuiMsg,-1);
};

void CBrain::OnSpaceDeleted(CLogicDialog* Dialog,CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);

	CMsg GuiMsg(SYSTEM_SOURCE,Receiver,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);
	
	ePipeline Cmd(TO_SPACE_VIEW::ID);
	Cmd.PushInt(TO_SPACE_VIEW::DEL_ITEM);
	Cmd<<Letter;
	
    GuiMsg.GetLetter().PushPipe(Cmd);
	
	GetBrainData()->SendMsgToGUI(this,GuiMsg,-1);
};


void CBrain::OnSpaceSendObject(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);

	CMsg GuiMsg(SYSTEM_SOURCE,Receiver,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);
	
	ePipeline Cmd(TO_SPACE_VIEW::ID);
	Cmd.PushInt(TO_SPACE_VIEW::SEND_OBJECT);
	Cmd<<Letter;
	
    GuiMsg.GetLetter().PushPipe(Cmd);
	
	GetBrainData()->SendMsgToGUI(this,GuiMsg,-1);
};

void CBrain::OnSpaceAccessDenied(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);

	CMsg GuiMsg(SYSTEM_SOURCE,Receiver,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);
	ePipeline Cmd(TO_SPACE_VIEW::ID);
	Cmd.PushInt(TO_SPACE_VIEW::ACESS_DENIED);
	Cmd<<Letter;
	
    GuiMsg.GetLetter().PushPipe(Cmd);
	
	GetBrainData()->SendMsgToGUI(this,GuiMsg,-1);
};



void CBrain::OnOperatorFeedback(CLogicDialog* Dialog,CMsg& Msg){
	
	int64 SourceID = Msg.GetSourceID();
	
	int64 EventID = Msg.GetEventID();
	

	
	CBrainEvent EventInfo;
	if(!m_BrainData.GetEvent(EventID,EventInfo,true)){
		return;
	};
	ePipeline& ObjectAddress = EventInfo.m_ClientAddress;
	
	CMsg Msg1(SourceID,ObjectAddress.GetID(),MSG_OPERATOR_FEEDBACK,DEFAULT_DIALOG,EventID);
	ePipeline& Receiver = Msg1.GetReceiver();
	Receiver<< ObjectAddress;
	
	ePipeline& Letter1 = Msg1.GetLetter();
    Letter1<<Msg.GetLetter();
	

	PushNerveMsg(Msg1,false,false);	
};

void CBrain::OnRobotExportObject(CLogicDialog* Dialog,CMsg& Msg){
	
	int64 SourceID = Msg.GetSourceID();
	int64 EventID = Msg.GetEventID();
	
	CBrainEvent EventInfo;
	if(!m_BrainData.GetEvent(EventID,EventInfo,true)){
		return;
	};
	
	ePipeline& ObjectAddress = EventInfo.m_ClientAddress;
	
	CMsg Msg1(SourceID,ObjectAddress.GetID(),MSG_ROBOT_EXPORT_OBJECT,DEFAULT_DIALOG,EventID);
	ePipeline& Receiver = Msg1.GetReceiver();
	Receiver<<ObjectAddress;
	
	ePipeline& Letter1 = Msg1.GetLetter();
    Letter1<<Msg.GetLetter();
	

	PushNerveMsg(Msg1,false,false);	
}


void CBrain::OnTaskRequest(CLogicDialog* Dialog,CMsg& Msg)
{
	int64 SourceID = Msg.GetSourceID();
	if (SourceID == SYSTEM_SOURCE)
	{
		ePipeline& Letter = Msg.GetLetter();
		ePipeline& RequestInfo = *(ePipeline*)Letter.GetData(0);
		
		//CRequestBrain Rq(RequestInfo);
		//Rq.Request(m_BrainData.GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG));
	}else{
		assert(0); //暂时不接受外部请求
	}
}
/*
void CBrain::OnCloseDialog(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();

	int64 SourceID  = Letter.PopInt();
	int64 DialogID    = Letter.PopInt();
	if (SourceID==SPACE_SOURCE)
	{
		CLinker Linker;
		GetLinker(SourceID,Linker);	
		Linker().CloseDialog(DialogID);

		CSuperiorLinkerList*  SuperiorList = GetSuperiorLinkerList();
		SuperiorList->DeleteLinker(SPACE_SOURCE);

		assert(DialogID==DEFAULT_DIALOG);

		Linker().Close();         
		tstring ip ;
		ePipeline Receiver;
		Receiver.PushInt(SourceID);
		Receiver.PushInt(DialogID);

		CMsg GuiMsg2(Receiver,MSG_BRAIN_TO_GUI,0);		
		ePipeline Cmd(GUI_CONNECT_STATE); 
		Cmd.PushInt(CON_END);
		Cmd.PushString(ip);
		Cmd.PushInt(FALSE);	
		GuiMsg2.GetLetter().PushPipe(Cmd);

		GetBrainData()->SendMsgToGUI(this,SourceID,GuiMsg2);		
	}	

	GetBrainData()->DeleteDialog(SourceID,DialogID);
}
*/
