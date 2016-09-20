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
				m_BrainData.BrainIdleProc();
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
	case MSG_TASK_RUNTIME_INFO:
		OnTaskRuntimeInfo(MainDialog,Msg);
		break;
	case MSG_TASK_RUNTIME_LOGIC:
		OnTaskRuntimeLogic(MainDialog,Msg);
		break;
	case MSG_TASK_RUNTIME_RESULT:
		OnTaskRuntimeResult(MainDialog,Msg);
		break;
	case MSG_START_RESQUEST:
		OnRobotStartRequest(MainDialog,Msg);
		break;
	case MSG_REQUEST_RUNTIME:
		OnRobotRequestRuntime(MainDialog,Msg);
		break;
	case MSG_EXECUTE_REQUEST:
		OnRobotExecuteResquest(MainDialog,Msg);
		break;
	case MSG_CLOSE_REQUEST:
		OnRobotCloseRequest(MainDialog,Msg);
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
	case MSG_SPACE_ADDED:
		OnSpaceAdded(MainDialog,Msg);
		break;
	case MSG_SPACE_DELETED:
		OnSpaceDeleted(MainDialog,Msg);
		break;
	case MSG_SPACE_SEND_OBJECT:
		OnSpaceSendObject(MainDialog,Msg);
		break;
	case MSG_OPERATOR_FEEDBACK:
		OnOperatorFeedback(MainDialog,Msg);
		break;
	case MSG_ROBOT_EXPORT_OBJECT:
		OnRobotExportObject(MainDialog,Msg);
		break;
	case MSG_SPACE_CATALOG:
		OnSpaceCatalog(MainDialog,Msg);
		break;
	case MSG_SPACE_ACCESS_DENIED:
		OnSpaceAccessDenied(MainDialog,Msg);
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
	
	ePipeline& rLetter=rMsg.GetLetter(false);
	
	const COrgan& Organ = m_BrainData.GetOrgan(LOCAL_GUI_SOURCE);

	rLetter.PushInt(0);  //robot总是0
	rLetter.PushString(Organ.m_Name);
	rLetter.PushString(Organ.m_CryptTxt);

	
	Linker().PushMsgToSend(rMsg);
}

void CBrain::OnI_AM(CLogicDialog* Dialog,CMsg& Msg){
	
	int64 EventID = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter(true);
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
		OutputLog(LOG_WARNING,Format1024(_T("Linker(%I64ld) is invalid for MSG_I_AM"),SourceID).c_str());
		assert(0);
		return;
	}
#ifdef _DEBUG
	Linker().SetStateOutputLevel(LIGHT_LEVEL);
#endif	
	
	if(!Letter.HasTypeAB(PARAM_TYPE2(TYPE_STRING,TYPE_STRING))){ //两个字符串
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
			rMsg.GetLetter(false).PushString(Name);

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

void CBrain::OnTaskRuntimeInfo(CLogicDialog* Dialog,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	CBrainEvent  EventInfo;
	bool ret = GetBrainData()->GetEvent(EventID,EventInfo,false);
	assert(ret);
	if(ret){
		ePipeline& ClientAddress = EventInfo.m_ClientAddress;
		ePipeline& OldExePipe   = EventInfo.m_ClientExePipe;

		int64 ClientSourceID = EventInfo.m_ClientAddress.m_ID;

		CMsg m(ClientSourceID,ClientAddress,MSG_TASK_RUNTIME_INFO,Msg.GetSender(),EventID);
		ePipeline& Letter = m.GetLetter(false);
		Letter<<Msg.GetLetter(true);	

		PushNerveMsg(m,false,false);
	}
};

void CBrain::OnTaskRuntimeLogic(CLogicDialog* Dialog,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	CBrainEvent  EventInfo;
	bool ret = GetBrainData()->GetEvent(EventID,EventInfo,false);
	assert(ret);
	if(ret){
		ePipeline& ClientAddress = EventInfo.m_ClientAddress;
		ePipeline& OldExePipe   = EventInfo.m_ClientExePipe;

		int64 ClientSourceID = EventInfo.m_ClientAddress.m_ID;

		CMsg m(ClientSourceID,ClientAddress,MSG_TASK_RUNTIME_LOGIC,Msg.GetSender(),EventID);
		ePipeline& Letter = m.GetLetter(false);
		Letter<<Msg.GetLetter(true);	

		PushNerveMsg(m,false,false);
	}
};
void CBrain::OnTaskRuntimeResult(CLogicDialog* Dialog,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	CBrainEvent  EventInfo;
	bool ret = GetBrainData()->GetEvent(EventID,EventInfo,false);
	assert(ret);
	if(ret){
		ePipeline& ClientAddress = EventInfo.m_ClientAddress;
		ePipeline& OldExePipe   = EventInfo.m_ClientExePipe;

		int64 ClientSourceID = EventInfo.m_ClientAddress.m_ID;

		CMsg m(ClientSourceID,ClientAddress,MSG_TASK_RUNTIME_RESULT,Msg.GetSender(),EventID);
		ePipeline& Letter = m.GetLetter(false);
		Letter<<Msg.GetLetter(true);	

		PushNerveMsg(m,false,false);
	}
};


void CBrain::OnConnectOK(CLogicDialog* Dialog,CMsg& Msg){
		
	int64 SourceID = Msg.GetSourceID();
	ePipeline& Letter = Msg.GetLetter(true);
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
		
		ePipeline Address(LOCAL_SPACEID);
		ePipeline Param(SYSTEM_SOURCE);
		Param.PushPipe(Address); //空地址表示访问根目录

		StartSysTask(SysTask_GotoSpace,SPACE_SOURCE,Param,_T("Goto Space"),0,true);				

		//return;
	}
	
	//CLockedBrainData* LockedData = GetBrainData();
	//
	//CLogicDialog* NewDialog = LockedData->CreateNewDialog(this,SPACE_SOURCE,DEFAULT_DIALOG,NO_PARENT,Name,_T("You"),DIALOG_SYSTEM,TASK_OUT_DEFAULT);	
	//NewDialog->m_bEditValid = false;

	//CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
	//nf.PushInt(DL_ADD_DIALOG);
	//nf.Notify(NewDialog);
   	
	OutSysInfo(_T("Connection ok!"));
}


void CBrain::OnGUI2Brain(CLogicDialog* Dialog,CMsg& Msg){
    int64 GuiSourceID = Msg.GetSourceID();
	
	if (GetBrainData()->GetOrgan(GuiSourceID).m_DialogType != DIALOG_SYSTEM)
	{
		NotifySysState(NOTIFY_ILLEGAL_MSG,NULL,&Msg.GetMsg());
		return; 
	}
	
	ePipeline& Letter = Msg.GetLetter(true);
	
	int64 SourceID = Letter.PopInt();
	int64 DialogID = Letter.PopInt();
	
	if (SourceID != GuiSourceID && SourceID != SYSTEM_SOURCE && SourceID != SPACE_SOURCE)
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
		ePipeline& Letter = NewMsg.GetLetter(false);
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
	rMsg.GetLetter(false).PushPipe(DialogListInfo);	
	
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

	ePipeline& Letter = Msg.GetLetter(true);	
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

	//如果客户是系统对话，就直接处理结果，
	if(ClientSourceID == Dialog->m_SourceID && ClientDialogID == Dialog->m_DialogID ){

		CLogicDialog* TaskDlg = GetBrainData()->GetDialog(ClientSourceID,EventID);
		if(EventInfo.m_bOnce){

			TaskDlg->SetThinkState(THINK_IDLE); //仅仅起到重置GUI界面的作用
			TaskDlg->ResetTask();

			BrainData->GetEvent(EventID,EventInfo,true);

			assert(!EventInfo.m_bPrivate);

			CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
			nf.PushInt(DL_DEL_DIALOG);
			nf.Notify(TaskDlg);

			m_BrainData.DeleteDialog(ClientSourceID,EventID);

		
		}else{ //长期对话

			CPipeView PipeView(ExePipe);
			tstring s  = PipeView.GetString();

			TaskDlg->RuntimeOutput(0,s);

			int64 ID = ExePipe->GetID();

			if (!ExePipe->IsAlive())
			{
				tstring text = _T("Stop");
				TaskDlg->SaveDialogItem(text,_T("System"),0); 
				TaskDlg->SetWorkMode(WORK_TASK);
				TaskDlg->ResetTask();
				TaskDlg->ResetThink();

			}
			else{
				tstring text = _T("ok");
				TaskDlg->SaveDialogItem(text,_T("System"),0);    

				//如果有的话，准备执行下一个逻辑任务
				if(TaskDlg->HasTask()){
					//SetTaskState(TASK_STOP);

					CMsg Msg(SYSTEM_SOURCE,TaskDlg->m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,0);
					ePipeline& Letter = Msg.GetLetter(false);
					Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
					Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE);
					ExePipe->Clear();
					Letter.PushPipe(*ExePipe);

					PushNerveMsg(Msg,false,false);
					return;
				}else{
					TaskDlg->ResetTask();
				}		
			}

		}
		

	}else{  //剩下的客户对话目前而言就是任务对话了，发信者可能是任务对话自身，也可能是其事件子对话

		if (EventInfo.m_InstinctID == INSTINCT_PAUSE_TASK)
		{ 
			//唯一预先生成并允许用户输入的只有pause对话，此时忽略其结果
			CLogicDialog* EventDlg = GetBrainData()->GetDialog(ClientSourceID,EventID);
			EventDlg->SaveDialogItem(_T("Please input 'run' or 'step' or 'stop' command to cancel pause "),Dialog->m_DialogName,0);
			EventDlg->ResetTask();
			return;
		}

		CMsg m(Msg.GetSourceID(),ClientAddress,MSG_TASK_RESULT,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = m.GetLetter(false);
		Letter.PushPipe(*ExePipe);		

		PushNerveMsg(m,false,false);
	}
}

void CBrain::OnSpaceAdded(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter(true);
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);

	CMsg GuiMsg(SYSTEM_SOURCE,Receiver,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);
	
	ePipeline Cmd(TO_SPACE_VIEW::ID);
	Cmd.PushInt(TO_SPACE_VIEW::ADD_ITEM);
	Cmd<<Letter;
	
    GuiMsg.GetLetter(false).PushPipe(Cmd);
	
	GetBrainData()->SendMsgToGUI(GuiMsg,-1);
};

void CBrain::OnSpaceDeleted(CLogicDialog* Dialog,CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter(true);
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);

	CMsg GuiMsg(SYSTEM_SOURCE,Receiver,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);
	
	ePipeline Cmd(TO_SPACE_VIEW::ID);
	Cmd.PushInt(TO_SPACE_VIEW::DEL_ITEM);
	Cmd<<Letter;
	
    GuiMsg.GetLetter(false).PushPipe(Cmd);
	
	GetBrainData()->SendMsgToGUI(GuiMsg,-1);
};


void CBrain::OnSpaceSendObject(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter(true);
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);

	CMsg GuiMsg(SYSTEM_SOURCE,Receiver,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);
	
	ePipeline Cmd(TO_SPACE_VIEW::ID);
	Cmd.PushInt(TO_SPACE_VIEW::SEND_OBJECT);
	Cmd<<Letter;
	
    GuiMsg.GetLetter(false).PushPipe(Cmd);
	
	GetBrainData()->SendMsgToGUI(GuiMsg,-1);
};

void CBrain::OnSpaceCatalog(CLogicDialog* Dialog,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	CBrainEvent EventInfo;
	if(!m_BrainData.GetEvent(EventID,EventInfo,false)){ 
		assert(0);
		return;
	};

	ePipeline& Address = EventInfo.m_ClientAddress;
	Msg.GetReceiver() = Address;
	PushNerveMsg(Msg,false,false);
}

void CBrain::OnSpaceAccessDenied(CLogicDialog* Dialog,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	CBrainEvent EventInfo;
	if(!m_BrainData.GetEvent(EventID,EventInfo,false)){ 
		assert(0);
		return;
	};

	ePipeline& Address = EventInfo.m_ClientAddress;
	Msg.GetReceiver() = Address;
	PushNerveMsg(Msg,false,false);
};

void CBrain::OnRobotExecuteResquest(CLogicDialog* Dialog,CMsg& Msg){
	int64 SourceID = Msg.GetSourceID();
	assert(SourceID == SPACE_SOURCE); //此消息只接受空间传来
	if(SourceID != SPACE_SOURCE){
		return;
	}

	int64 EventID = Msg.GetEventID();
	assert(EventID!=0);
	if(EventID==0)return;

	CBrainEvent EventInfo;

	if(!m_BrainData.GetEvent(EventID,EventInfo,false)){
		return;
	}

	ePipeline& Receiver = Msg.GetReceiver();
	Receiver.Clear();
	Receiver << EventInfo.m_ClientAddress;

	PushNerveMsg(Msg,false,false);
}
void CBrain::OnRobotStartRequest(CLogicDialog* Dialog,CMsg& Msg){

	int64 SourceID = Msg.GetSourceID();
	assert(SourceID == SPACE_SOURCE); //此消息只接受空间传来
	if(SourceID != SPACE_SOURCE){
		return;
	}
	int64 SpaceEventID = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter(true);
	if(!Letter.HasTypeAB(PARAM_TYPE2(TYPE_STRING,TYPE_STRING))){  
		CLinker Linker;
		GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);

		FeedbackError(Linker,SpaceEventID,_T("param error"),NULL);
		return;
	}

	tstring Caller = Letter.PopString();

	ePipeline Param;
	Param.PushInt(SpaceEventID);
	Param.PushString(Caller);
	Param <<  Letter;

	StartSysTask(SysTask_RobotRequest,Dialog->m_SourceID,Param,Caller,0,true);				

}

void CBrain::OnRobotRequestRuntime(CLogicDialog* Dialog,CMsg& Msg){
	
	int64 SourceID = Msg.GetSourceID();
	assert(SourceID == SPACE_SOURCE); //此消息只接受空间传来
	if(SourceID != SPACE_SOURCE){
		return;
	}
	int64 EventID = Msg.GetEventID();
	CBrainEvent EventInfo;
	if(!m_BrainData.GetEvent(EventID,EventInfo,false)){

		return;
	};

	ePipeline& Address = EventInfo.m_ClientAddress;
	ePipeline& Letter = Msg.GetLetter(true);
	
	CMsg rMsg(SYSTEM_SOURCE,Address,MSG_REQUEST_RUNTIME,DEFAULT_DIALOG,EventID);	
	ePipeline& rLetter = rMsg.GetLetter(false);
	rLetter<<Letter;
	
	PushNerveMsg(rMsg,false,false);
}

void CBrain::OnRobotCloseRequest(CLogicDialog* Dialog,CMsg& Msg){
	
	int64 SourceID = Msg.GetSourceID();
	assert(SourceID == SPACE_SOURCE); //此消息只接受空间传来
	if(SourceID != SPACE_SOURCE){
		return;
	}
	int64 EventID = Msg.GetEventID();
	CBrainEvent EventInfo;
	if(!m_BrainData.GetEvent(EventID,EventInfo,false)){
		return;
	};

	ePipeline& Receiver = Msg.GetReceiver();
	Receiver.Clear();
	Receiver << EventInfo.m_ClientAddress;

	PushNerveMsg(Msg,false,false);
	
}
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
	
	ePipeline& Letter1 = Msg1.GetLetter(false);
    Letter1<<Msg.GetLetter(true);
	

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
	
	ePipeline& Letter1 = Msg1.GetLetter(false);
    Letter1<<Msg.GetLetter(true);
	

	PushNerveMsg(Msg1,false,false);	
}


void CBrain::OnTaskRequest(CLogicDialog* Dialog,CMsg& Msg)
{
	int64 SourceID = Msg.GetSourceID();
	if (SourceID == SYSTEM_SOURCE)
	{
		ePipeline& Letter = Msg.GetLetter(true);
		ePipeline& RequestInfo = *(ePipeline*)Letter.GetData(0);
		
		//CRequestBrain Rq(RequestInfo);
		//Rq.Request(m_BrainData.GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG));
	}else{
		assert(0); //暂时不接受外部请求
	}
}



/*
void CBrain::OnCloseDialog(CLogicDialog* Dialog,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter(true);

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
		GuiMsg2.GetLetter(false).PushPipe(Cmd);

		GetBrainData()->SendMsgToGUI(this,SourceID,GuiMsg2);		
	}	

	GetBrainData()->DeleteDialog(SourceID,DialogID);
}
*/
