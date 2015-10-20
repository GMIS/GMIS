#pragma warning (disable:4786)

#include "Brain.h"
#include "LogicDialog.h"
#include "GUIMsgDefine.h"
#include <iostream>
#include <fstream>
#include "Ipclass.h"
#include <winbase.h>

void CBrain::TaskDialogProc(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
	

	int64 MsgID=Msg.GetMsgID();
	
	switch(MsgID)
	{
	case MSG_FROM_BRAIN:
		OnMsgFromBrain(Dialog,ExePipe,Msg);
		break;
	case MSG_TASK_RESULT:
		OnTaskResult(Dialog,ExePipe,Msg);
		break;
	case MSG_BROADCAST_MSG:
		OnBroadcaseMsg(Dialog,ExePipe,Msg);
		break;
	case MSG_CLOSE_DIALOG:
		OnCloseDialog(Dialog,ExePipe,Msg);
		break;
	case MSG_EVENT_TICK:
		OnEventTick(Dialog,ExePipe,Msg);
		break;
	default:
		NotifySysState(MNOTIFY_ILLEGAL_MSG, &Msg.GetMsg());
		break;
	}
	
}



//信息处理
//////////////////////////////////////////////////////////////////////////



void CBrain::OnBroadcaseMsg(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
	//所谓Server暂时没有处理广播信息的必要
	assert(0);
}


void CBrain::OnCloseDialog(CLogicDialog*Dialog,ePipeline* ExePipe,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	
	int64 SourceID  = Letter.PopInt();
	int64 DialogID    = Letter.PopInt();
	if (SourceID==SPACE_SOURCE)
	{
		CLinker Linker;
		GetLinker(SourceID,Linker);	
		Linker().CloseDialog(DialogID);
		
		CLockedLinkerList*  SuperiorList = GetSuperiorLinkerList();
		SuperiorList->DeleteLinker(SPACE_SOURCE);
		
		if(DialogID==DEFAULT_DIALOG){
			Linker().Close();         
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
		}
	}	
	if (DialogID==DEFAULT_DIALOG)
	{
		GetBrainData()->DeleteDialog(SourceID);
	}else{
		GetBrainData()->DeleteDialog(DialogID);
	}
}

void CBrain::OnMsgFromBrain(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg)
{
	int64 SourceID = Msg.GetSourceID();
	if (SourceID != SYSTEM_SOURCE)
	{
		NotifySysState(MNOTIFY_ILLEGAL_MSG, &Msg.GetMsg());
		return;
	}
	
    ePipeline& Letter = Msg.GetLetter();

	int64 ChildMsgID = Letter.PopInt();
	
	switch(ChildMsgID){
	case GUI_IO_INPUTING:
		{
			OnBrainTextInputing(Dialog,ExePipe,Msg);	
		}
		break;
	case GUI_IO_INPUTED:
		{
			OnBrainTextInputEnd(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_GET_DIALOG_MORE_ITME:
        {
			OnBrainGetMoreLog(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_SET_FOUCUS_DIALOG:
        {
			OnBrainSetFocusDialog(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_TASK_CONTROL:
		{
			OnBrainTaskControl(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_GET_DEBUG_ITEM:
		{
			OnBrainGetDebugItem(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_LOGIC_OPERATE:
		{
			OnBrainLogicOperate(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_OBJECT_OPERATE:
		{
			OnBrainObjectOperate(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_MEMORY_OPERATE:
		{
			OnBrainMemoryOperate(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_GET_THINK_RESULT:
		{
			OnBrainGetThinkResult(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_GET_ANALYSE_RESULT:
		{
			OnBrainGetAnalyseResult(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_CLEAR_DIALOG_OUTPUT:
		{
			OnBrainClearDialogOutput(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_GET_FIND_REUSLT:
		{
			OnBrainGetFindResult(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_CLEAR_THINK:
		{
			OnBrainClearThink(Dialog,ExePipe,Msg);
		}
		break;

	case GUI_CONNECT_TO:
		{
			OnBrainConnectTo(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_DISCONNECT:
		{
			OnBrainDisconnect(Dialog,ExePipe,Msg);
		}
		break;
	case GUI_SET_LOG_FLAG:
		{
			OnBrainSetLogFlag(Dialog,ExePipe,Msg);
		}
		break;
	default:
		NotifySysState(MNOTIFY_ILLEGAL_MSG, &Msg.GetMsg());
		return;
	}
}

void CBrain::OnBrainTextInputing(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){

	Dialog->GetThink()->ThinkProc(Dialog,Msg);
};

void CBrain::OnBrainTextInputEnd(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
		
	int64   Pos  = Letter.PopInt();
	tstring text = Letter.PopString();
	
	Dialog->m_EditText = text;

	//Dialog->RuntimeTip(DEFAULT_DIALOG,_T("%s"),text.c_str());
	
	TASK_OUT_TYPE TaskType = Dialog->GetTaskType();

	if (TaskType == TASK_OUT_DEFAULT)
	{ 
		CLogicThread* Think = Dialog->GetThink();
	
		bool  ret = Think->CanBeExecute(Dialog);
		
		if(!ret){ //不能执行,让用户继续编辑
			Dialog->SetTaskState(TASK_IDLE);
			Dialog->NotifyTaskState();
			return;
		};
		
		if(Dialog->m_DialogID == DEFAULT_DIALOG){
		    //系统对话不执行具体逻辑任务，而是请求大脑生成一个子对话执行	
			int64 EventID = AbstractSpace::CreateTimeStamp();
			CRequestBrain rq(REQUEST_TRANS_TASK,EventID);
			rq.Request(Dialog);
		}else{
			ePipeline ChildMsg(GUI_TASK_CONTROL);
			ChildMsg.PushInt(CMD_EXE);
			
			CMsg BrainMsg;
			CreateBrainMsg(BrainMsg,Dialog->m_DialogID,ChildMsg,0);	
			PushNerveMsg(BrainMsg,false,false);
		}
	} 
	else
	{	
		ExePipe->PushInt(Dialog->m_ThinkID);
		Dialog->FeedbackToBrain();
	}

};


void CBrain::OnBrainGetMoreLog(CLogicDialog*Dialog,ePipeline* ExePipe,CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter();
	int64 LastItemID = Letter.PopInt();
	
	int64 SourceID = Dialog->m_SourceID;
	int64 DialogID = Dialog->m_DialogID;
	
	
	ePipeline Pipe;
	GetBrainData()->GetMoreLog(SourceID,DialogID,LastItemID,Pipe);
	
	CNotifyState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(DIALOG_INFO_MORE);
	nf.PushPipe(Pipe);
	nf.Notify(Dialog);
	
};
void CBrain::OnBrainSetFocusDialog(CLogicDialog*Dialog,ePipeline* ExePipe,CMsg& Msg){
	ePipeline& Letter  = Msg.GetLetter();
        


	int64 SourceID = Dialog->m_SourceID;
	int64 DialogID = Dialog->m_DialogID;
	
	int64 TimeStamp = Dialog->m_LogicItemTree.GetID();

	bool bIncludeDebugInfo = false;
    if(TimeStamp!=m_CurDebugTimeStamp){
		bIncludeDebugInfo = true;
		m_CurDebugTimeStamp = TimeStamp;
	}

	CNotifyState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(INIT_FOCUS);
	nf.PushInt(SourceID);
	nf.PushInt(DialogID);
	
	//历史对话记录
	ePipeline Pipe;
	GetBrainData()->GetFocusDialogData(SourceID,DialogID,Pipe,bIncludeDebugInfo);
	nf<<Pipe;
	
	//运行时输出记录
    
	nf.Notify(Dialog);
}
void CBrain::OnBrainTaskControl(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();
	int64 Cmd = Letter.PopInt();
		
	switch (Cmd)
	{
	case CMD_DEBUG_STEP:
		{
			
			int64 PauseID = Letter.PopInt();
			Dialog->StopPause(PauseID,CMD_DEBUG_STEP);
		}
		break;
	case CMD_RUN: //这种只可能是在调试状态下要求继续执行
		{
			WORK_MODE WorkMode = Dialog->GetWorkMode();
			assert(WorkMode == WORK_DEBUG);

			Dialog->SetWorkMode(WORK_TASK);

			int64 PauseID = Letter.PopInt();

			Dialog->StopPause(PauseID,CMD_RUN);
		}
		break;

	case CMD_EXE:
		{
			ExePipe->Clear();
		    Dialog->ClearTaskMsgList();			
			Dialog->SetTaskState(TASK_EXE);			
			Dialog->NotifyTaskState();
			
			bool ret =  Dialog->CompileTask();
			if (!ret)
			{					
				Dialog->SetTaskState(TASK_IDLE);
				Dialog->NotifyTaskState();
				return ;	
			}
								
			ePipeline Receiver;
			Receiver.PushInt(Dialog->m_DialogID);
			Receiver.PushInt(Dialog->m_TaskID);
			
			CMsg TaskMsg(Receiver,MSG_ELT_TASK_CTRL,0);
			TaskMsg.GetLetter().PushInt(CMD_RUN);
			
			PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
		}
		break;
    case CMD_STOP:
		{
			if (Dialog->GetTaskState() != TASK_STOP)
			{
                //注意：stop分为两步，首先发一个CMM_STOP信息给任务, 此时任务正在执行没有机会处理此信息，然后让执行管道无效，这会让正在执行的任务退出，但执行管道无效
				//不一定是任务停止，然后任务处理CMD_STOP,改变任务状态为TASK_STOP才算正式结束任务
		
				ePipeline Address;
				Address.PushInt(Dialog->GetDialogID());
				Address.PushInt(Dialog->m_TaskID);

				CMsg EltMsg(Address,MSG_ELT_TASK_CTRL,0);
				ePipeline& Letter = EltMsg.GetLetter();
				Letter.PushInt(CMD_STOP);
				
				//给Element发信息
				PushNerveMsg(EltMsg,false,false);
	
				ExePipe->Break();		
			}
			
		}
		break;
	case CMD_PAUSE:
		{
			//if (Dialog->GetTaskState() == TASK_RUN)
			{
				ExePipe->SetID(RETURN_BREAK);
			}
		}
		break;
	default:
		{
			ePipeline Receiver;
			Receiver.PushInt(Dialog->m_DialogID);
			Receiver.PushInt(Dialog->m_TaskID);
			
			CMsg TaskMsg(Receiver,MSG_ELT_TASK_CTRL,0);
			TaskMsg.GetLetter().PushInt(Cmd);
			TaskMsg.GetLetter()<<Letter;
			PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
		}

	}

}


void CBrain::OnBrainGetDebugItem(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	int64 TaskID = Letter.PopInt();
	assert(TaskID == Dialog->m_LogicItemTree.GetID());
    	
	//凡是暂停都会自动进入调试状态，先准备好调试数据
	if (Dialog->m_LogicItemTree.Size()==0)
	{
		CLogicTask* Task = GetBrainData()->GetLogicTask(Dialog->m_TaskID);
		Task->GetDebugItem(Dialog->m_LogicItemTree);
	}

	m_CurDebugTimeStamp = TaskID;

	ePipeline PauseIDList;
	Dialog->GetPauseIDList(PauseIDList);
	CNotifyState nf(NOTIFY_DEBUG_VIEW);
	nf.PushInt(DEBUG_RESET);
	nf.Push_Directly(Dialog->m_LogicItemTree.Clone());
	nf.PushPipe(PauseIDList);
	nf.Notify(Dialog);
};

	/*
void CBrain::OnBrainSetGloble(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){


	ePipeline& Letter = Msg.GetLetter();

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	int64 Type = Letter.PopInt();
	switch (Type)
	{
	case GLOBAL_OBJECT:
        {
			ePipeline* ItemData = (ePipeline*)Letter.GetData(0);
					
			CBrainMemory::RegisterGlobalObject(*ItemData);

			ePipeline Cmd(GUI_OBJECT_OUTPUT);
			Cmd.PushInt(GLOBAL_OBJECT);
			Cmd.PushInt(ADD_OBJECT);
			Cmd.Push_Directly(ItemData->Clone());
			GuiMsg.GetLetter().PushPipe(Cmd);
		}
		break;
	case GLOBAL_LOGIC:
		{
			tstring LogicName = Letter.PopString();
			if(CBrainMemory::FindGlobalLogic(LogicName)){
				//MessageBox(GetHwnd(),_T("The logic exist"),_T("Warning"),MB_OK);
				return ;
			}
			
			CLocalLogicCell* LogicCell = Dialog->FindLogic(LogicName);
            
			if (LogicCell == NULL)
			{
				return;
			}
			
			CBrainTask* Task = GetBrainData()->GetLogicTask(LogicCell->m_Task.m_ID);

			CBrainMemory::RegisterGlobalLogic(Task->m_Name,Task->m_LogicText,Task->m_LogicData,Task->m_State);
			
			ePipeline* LogicData = LogicCell->GetLogicItem();
			
			ePipeline Cmd(GUI_LOGIC_OUTPUT);
			Cmd.PushInt(GLOBAL_LOGIC);
			Cmd.PushInt(ADD_LOGIC);
			Cmd.Push_Directly(LogicData);
			GuiMsg.GetLetter().PushPipe(Cmd);
		}
		break;
	default:
		return;
	}
	
	int32 GuiID = LOCAL_GUI;
	SendMsgToGUI(GuiID,GuiMsg);

}
	*/

void CBrain::OnBrainLogicOperate(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
		
	int64 Type = Letter.PopInt();
	switch (Type)
	{
	case DEL_LOGIC:
        {
			tstring LogicName = Letter.PopString();
			
            Dialog->DeleteLogic(LogicName);
		}
		break;
	case CLEAR_LOGIC:
		{
			Dialog->ClearLogicSence();
		};
	default:
		return;
	}
	
}
void CBrain::OnBrainObjectOperate(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg)
{
	ePipeline& Letter = Msg.GetLetter();
	
	int64 Type = Letter.PopInt();
	switch (Type)
	{
	case SELECT_OBJECT:
        {
			ePipeline* Item = (ePipeline*)Letter.GetData(0);			
			Dialog->RegisterObject(*Item);
		}
		break;
	case DEL_OBJECT:
		{
			ePipeline* Item = (ePipeline*)Letter.GetData(0);			
			Dialog->DeleteObject(*Item);
		}
		break;
	case CLR_OBJECT:
		{
			Dialog->ClearObject();
		};
		break;
	default:
		return;
	}	
}

void CBrain::OnBrainMemoryOperate(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg)
{

	TASK_STATE  State = Dialog->GetTaskState(); 
	if (State!= TASK_IDLE && State != TASK_STOP)
	{
		Dialog->RuntimeOutput(0,_T("Please execute this command in idle state"));
		return ;
	};

	ePipeline& Letter = Msg.GetLetter();

	int64 Type = Letter.PopInt();
	int64 Cmd  = Letter.PopInt();
	if (Type == INSTANCE_OBJECT)
	{
		if (Cmd == CLOSE_INSTANCE)
		{
			int n=0;
			ePipeline Sentence;
		    while (Letter.Size())
		    {
				int64 InstanceID = Letter.PopInt();
				ePipeline*  ObjectInfo = Dialog->FindObjectInstance(InstanceID);
				if (ObjectInfo)
				{	

					ePipeline Clause1;
					Clause1.PushInt(INSTINCT_FOCUS_OBJECT);
					Clause1.PushInt(InstanceID);

					ePipeline Clause2;
					Clause2.PushInt(INSTINCT_CLOSE_OBJECT);

					Sentence.PushPipe(Clause1);
					Sentence.PushPipe(Clause2);

					n++;
				}				
		    }
			CLogicTask* Task = Dialog->GetTask();
			if (!Task)
			{
				return;
			}
			if (!Task->Compile(Dialog,&Sentence))
			{
				return;
			}
			tstring CmdStr = Format1024(_T("Close %d object instance"),n);
			Dialog->SaveReceiveItem(CmdStr,0);

			Dialog->SetTaskState(TASK_EXE);			
			Dialog->NotifyTaskState();

			ePipeline Receiver;
			Receiver.PushInt(Dialog->m_DialogID);
			Receiver.PushInt(Dialog->m_TaskID);

			CMsg TaskMsg(Receiver,MSG_ELT_TASK_CTRL,0);
			TaskMsg.GetLetter().PushInt(CMD_RUN);

			PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
		}
	}else if(Type == INSTANCE_DATA){
		if (Cmd == CLOSE_INSTANCE)
		{
			int n=0;
			ePipeline Sentence;
			while (Letter.Size())
			{
				tstring InstanceName = Letter.PopString();

				ePipeline Clause1;
				Clause1.PushInt(INSTINCT_TABLE_FOCUS);
				Clause1.PushString(InstanceName);

				ePipeline Clause2;
				Clause2.PushInt(INSTINCT_TABLE_CLOSE);

				Sentence.PushPipe(Clause1);
				Sentence.PushPipe(Clause2);

				n++;

			}
			CLogicTask* Task = Dialog->GetTask();
			if (!Task)
			{
				return;
			}
			if (!Task->Compile(Dialog,&Sentence))
			{
				return;
			}
			tstring CmdStr = Format1024(_T("Close %d data instance"),n);
			Dialog->SaveReceiveItem(CmdStr,0);

			Dialog->SetTaskState(TASK_EXE);			
			Dialog->NotifyTaskState();

			ePipeline Receiver;
			Receiver.PushInt(Dialog->m_DialogID);
			Receiver.PushInt(Dialog->m_TaskID);

			CMsg TaskMsg(Receiver,MSG_ELT_TASK_CTRL,0);
			TaskMsg.GetLetter().PushInt(CMD_RUN);

			PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
		}
	}

}
void CBrain::OnBrainGetThinkResult(CLogicDialog* Dialog, ePipeline* ExePipe,CMsg& Msg){
	
	tstring s = Dialog->GetThink()->m_Text.PrintThinkResult();
	Dialog->RuntimeOutput(s);
	
}


void CBrain::OnBrainGetAnalyseResult(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
	tstring s = Dialog->GetThink()->m_Text.PrintAnlyseResult();
	Dialog->RuntimeOutput(s);
}



void CBrain::OnBrainClearDialogOutput(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
	Dialog->m_RuntimeOutput.Clear();
	//此处应该通知界面的，暂时简化如此
}

void CBrain::OnBrainGetFindResult(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
	
	int64 n = Msg.GetLetter().PopInt();
	
	ePipeline SearchResult;
	CLogicThread* Think = Dialog->GetThink();

	Dialog->FindContinue(Think,(int32)n,SearchResult);
	
	Dialog->m_SearchOutput = SearchResult;
	
	//输出结果,0条目也要通知
	CNotifyState nf1(NOTIFY_FIND_VIEW);
	nf1.PushInt(FIND_UPDATE);
	nf1.PushPipe(SearchResult);
	nf1.Notify(Dialog);
	
	
	//如果已经找到所有结果,通知搜索结束
	if (Dialog->m_FindSeedList.size()==0)
	{
		CNotifyState nf2(NOTIFY_FIND_VIEW);
		nf2.PushInt(FIND_STOP);
		nf2.PushInt(TRUE);
		nf2.Notify(Dialog);	
		Dialog->m_SearchOutput.SetID(0); //没有更多结果
	}else{
		CNotifyState nf2(NOTIFY_FIND_VIEW);
		nf2.PushInt(FIND_STOP);
        nf2.PushInt(FALSE);
		nf2.Notify(Dialog);	
		Dialog->m_SearchOutput.SetID(1); //还有更多结果
	}	
}

void CBrain::OnBrainClearThink(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
	if (Dialog->GetTaskState() == TASK_IDLE)
	{
		Dialog->ResetThink();
		
		CNotifyState nf(NOTIFY_DIALOG_STATE);
		nf.PushInt(Dialog->GetWorkMode());
		nf.PushInt(TASK_IDLE);
        nf.PushString(_T(""));
		nf.Notify(Dialog);
	}
}


void CBrain::OnBrainConnectTo(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){
			
	ePipeline& Letter = Msg.GetLetter();
	
	tstring ip   = Letter.PopString(); 
	int64   Port = Letter.PopInt();
	
	
	TCHAR buf[512];
	GetModuleFileNameW(   
		GetModuleHandle(NULL),         
		buf,       
		512                 
		);   
	tstring ExecuterFile = buf;
	int n = ExecuterFile.find_last_of(_T("\\"),tstring::npos);
	if(n!=-1){
		ExecuterFile=ExecuterFile.substr(0,n);
	}
	
	ExecuterFile += _T("\\SpacePortal.exe");
		
	STARTUPINFO si;
	PROCESS_INFORMATION  pi;	
	::ZeroMemory(&si, sizeof(si));	
	si.cb = sizeof(si);
	
	assert(0); //这里注释掉了命令行，需要同时修改SpacePortal部分
	tstring CmdLine;
	CmdLine +=_T(" "); 
//	CmdLine += m_Account.Name;
	CmdLine +=_T(" ");
//	CmdLine += m_Account.CrypText;
  

	TCHAR CmdLineBuf[512];
    
	 _tcscpy(CmdLineBuf,CmdLine.c_str());

	//spaceportal.exe具有防止重复启动的能力
	BOOL started = CreateProcessW(ExecuterFile.c_str(),        // command is part of input string
		CmdLineBuf,        // (writeable) command string
		NULL,        // process security
		NULL,        // thread security
		FALSE,       // inherit handles flag
		0,           // flags
		NULL,        // inherit environment
		NULL,        // inherit directory
		&si,    // STARTUPINFO
		&pi);   // PROCESS_INFORMATION
		

	SLEEP_MILLI(1000);

	tstring s = Format1024(_T("Connect to %s"),ip.c_str());
	Dialog->SaveSendItem(s,0);
	
	if (Port == SPACE_PORT)
	{
		ConnectSpace(Dialog,ip);
	}
}

void CBrain::OnBrainDisconnect(CLogicDialog*Dialog,ePipeline* ExePipe,CMsg& Msg){	
	CLinker Linker;
	GetLinker(SPACE_SOURCE,Linker);
	Linker().Close();
	
	GetBrainData()->DeleteDialogOfSource(SPACE_SOURCE);
	
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
	
};

void CBrain::OnBrainSetLogFlag(CLogicDialog* Dialog,ePipeline* ExePipe,CMsg& Msg)
{
	ePipeline& Letter = Msg.GetLetter();
	uint32 LogFlag = (uint32)Letter.PopInt();
	m_LogFlag = LogFlag;
}

void CBrain::OnTaskResult(CLogicDialog*Dialog,ePipeline* ExePipe,CMsg& Msg){	
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline* OldExePipe = (ePipeline*)Letter.GetData(0);
	ePipeline* NewExePipe = (ePipeline*)Letter.GetData(1);

	CPipeView PipeView(NewExePipe);
	tstring s  = PipeView.GetString();
	
	Dialog->RuntimeOutput(0,s);
	
	
	int64 ID = NewExePipe->GetID();

	if (!NewExePipe->IsAlive())
	{
		tstring text = _T("Stop");
		Dialog->SaveSendItem(text,0); 
	}
	else{
		tstring text = _T("ok");
		Dialog->SaveSendItem(text,0);    
	}
	
	Dialog->ResetTask();

	//继续生成一个与任务对话对应的事件
	ePipeline EmptyExePipe;
	ePipeline Address;
	int64 EventID = Dialog->GetDialogID();
	Address.PushInt(EventID);
	
	GetBrainData()->PushBrainEvent(EventID,EmptyExePipe,Address);

	//如果有的话，准备执行下一个逻辑任务
	CLogicThread* CurThink = Dialog->GetThink();	
	if(CurThink->m_Text.m_SentenceList.size()==0){
		Dialog->ResetThink();	
	}else{
		ePipeline ChildMsg(GUI_TASK_CONTROL);
		ChildMsg.PushInt(CMD_EXE);
		
		CMsg BrainMsg;
		CreateBrainMsg(BrainMsg,Dialog->m_DialogID,ChildMsg,0);	
		PushNerveMsg(BrainMsg,false,false);
	}		
};

void CBrain::OnEventTick(CLogicDialog*Dialog,ePipeline* ExePipe,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	ePipeline& Letter = Msg.GetLetter();
	int64 TimeStamp = Letter.PopInt();

	GetBrainData()->ResetEventTickCount(EventID);

	//Dialog->RuntimeOutput(0,_T("ResetEventTickCount"));
}
