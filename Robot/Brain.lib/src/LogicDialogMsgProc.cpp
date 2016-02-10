#pragma warning (disable:4786)

#include "Brain.h"
#include "LogicDialog.h"
#include "GUIMsgDefine.h"
#include <iostream>
#include <fstream>
#include "Ipclass.h"
#include <winbase.h>

void CLogicDialog::DialogMsgProc(ePipeline* ExePipe,CMsg& Msg){
	

	int64 MsgID=Msg.GetMsgID();
	
	switch(MsgID)
	{
	case MSG_FROM_BRAIN:
		OnMsgFromBrain(ExePipe,Msg);
		break;
	case MSG_TASK_RESULT:
		OnTaskResult(ExePipe,Msg);
		break;
	case MSG_BROADCAST_MSG:
		OnBroadcaseMsg(ExePipe,Msg);
		break;
	case MSG_EVENT_TICK:
		OnEventTick(ExePipe,Msg);
		break;
	default:
		m_Brain->NotifySysState(NOTIFY_ILLEGAL_MSG,NULL, &Msg.GetMsg());
		break;
	}
	
}




void CLogicDialog::OnBroadcaseMsg(ePipeline* ExePipe,CMsg& Msg){
	//所谓Server暂时没有处理广播信息的必要
	assert(0);
}


void CLogicDialog::OnMsgFromBrain(ePipeline* ExePipe,CMsg& Msg)
{
	int64 SourceID = Msg.GetSourceID();
	if (SourceID != SYSTEM_SOURCE)
	{
		m_Brain->NotifySysState(NOTIFY_ILLEGAL_MSG,NULL, &Msg.GetMsg());
		return;
	}
	
    ePipeline& Letter = Msg.GetLetter();

	int64 ChildMsgID = Letter.PopInt();
	
	switch(ChildMsgID){
	case GUI_IO_INPUTING:
		{
			OnBrainTextInputing(ExePipe,Msg);	
		}
		break;
	case GUI_IO_INPUTED:
		{
			OnBrainTextInputEnd(ExePipe,Msg);
		}
		break;
	case GUI_GET_DIALOG_MORE_ITME:
        {
			OnBrainGetMoreLog(ExePipe,Msg);
		}
		break;
	case GUI_SET_FOUCUS_DIALOG:
        {
			OnBrainSetFocusDialog(ExePipe,Msg);
		}
		break;
	case GUI_TASK_CONTROL:
		{
			OnBrainTaskControl(ExePipe,Msg);
		}
		break;
	case GUI_GET_DEBUG_ITEM:
		{
			OnBrainGetDebugItem(ExePipe,Msg);
		}
		break;
	case GUI_LOGIC_OPERATE:
		{
			OnBrainLogicOperate(ExePipe,Msg);
		}
		break;
	case GUI_OBJECT_OPERATE:
		{
			OnBrainObjectOperate(ExePipe,Msg);
		}
		break;
	case GUI_MEMORY_OPERATE:
		{
			OnBrainMemoryOperate(ExePipe,Msg);
		}
		break;
	case GUI_GET_THINK_RESULT:
		{
			OnBrainGetThinkResult(ExePipe,Msg);
		}
		break;
	case GUI_GET_ANALYSE_RESULT:
		{
			OnBrainGetAnalyseResult(ExePipe,Msg);
		}
		break;
	case GUI_CLEAR_DIALOG_OUTPUT:
		{
			OnBrainClearDialogOutput(ExePipe,Msg);
		}
		break;
	case GUI_GET_FIND_REUSLT:
		{
			OnBrainGetFindResult(ExePipe,Msg);
		}
		break;
	case GUI_CLEAR_THINK:
		{
			OnBrainClearThink(ExePipe,Msg);
		}
		break;

	case GUI_CONNECT_TO:
		{
			OnBrainConnectTo(ExePipe,Msg);
		}
		break;
	case GUI_DISCONNECT:
		{
			OnBrainDisconnect(ExePipe,Msg);
		}
		break;
	case GUI_SET_LOG_FLAG:
		{
			OnBrainSetLogFlag(ExePipe,Msg);
		}
		break;
	default:
		m_Brain->NotifySysState(NOTIFY_ILLEGAL_MSG, NULL,&Msg.GetMsg());
		return;
	}
}

void CLogicDialog::OnBrainTextInputing(ePipeline* ExePipe,CMsg& Msg){

	GetThink()->ThinkProc(this,Msg);
};

void CLogicDialog::OnBrainTextInputEnd(ePipeline* ExePipe,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
		
	int64   Pos  = Letter.PopInt();
	tstring text = Letter.PopString();
	
	m_EditText = text;

	//Dialog->RuntimeTip(DEFAULT_DIALOG,_T("%s"),text.c_str());
	
	TASK_OUT_TYPE TaskType = GetTaskType();

	if (TaskType == TASK_OUT_DEFAULT)
	{ 
		CLogicThread* Think = GetThink();
	
		bool  ret = Think->CanBeExecute(this);
		
		if(!ret){ //不能执行,让用户继续编辑
			SetTaskState(TASK_IDLE);
			NotifyTaskState();
			return;
		};
		
		if(m_DialogType == DIALOG_SYSTEM_MAIN){
		    //系统对话不执行具体逻辑任务，而是请求大脑生成一个子对话执行	
			int64 EventID = AbstractSpace::CreateTimeStamp();
			CRequestBrain rq(REQUEST_TRANS_TASK);
			rq.PushInt(EventID);
			rq.PushInt(FALSE); //just once?
			rq.Request(this);
		}else{
			ePipeline ChildMsg(GUI_TASK_CONTROL);
			ChildMsg.PushInt(CMD_EXE);
			
			CMsg BrainMsg;
			CreateBrainMsg(BrainMsg,m_DialogID,ChildMsg,0);	
			m_Brain->PushNerveMsg(BrainMsg,false,false);
		}
	} 
	else
	{	
		ExePipe->PushInt(m_ThinkID);
		FeedbackToBrain();
	}

};


void CLogicDialog::OnBrainGetMoreLog(ePipeline* ExePipe,CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter();
	int64 LastItemID = Letter.PopInt();
	
	ePipeline Pipe;
	m_Brain->GetBrainData()->GetMoreLog(m_SourceID,m_DialogID,LastItemID,Pipe);
	
	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(DIALOG_INFO_MORE);
	nf.PushPipe(Pipe);
	nf.Notify(this);
	
};
void CLogicDialog::OnBrainSetFocusDialog(ePipeline* ExePipe,CMsg& Msg){

	
	ePipeline& Letter  = Msg.GetLetter();
        
	int64 TimeStamp = m_LogicItemTree.GetID();

	bool bIncludeDebugInfo = false;
    if(TimeStamp!=m_LastDebugTimeStamp){
		bIncludeDebugInfo = true;
		m_LastDebugTimeStamp = TimeStamp;
	}

	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(INIT_FOCUS);
	nf.PushInt(m_SourceID);
	nf.PushInt(m_DialogID);
	
	//历史对话记录
	ePipeline Pipe;
	m_Brain->GetBrainData()->GetFocusDialogData(m_SourceID,m_DialogID,Pipe,bIncludeDebugInfo);
	nf<<Pipe;
	
	//运行时输出记录
    
	nf.Notify(this);
}
void CLogicDialog::OnBrainTaskControl(ePipeline* ExePipe,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();
	int64 Cmd = Letter.PopInt();
		
	switch (Cmd)
	{
	case CMD_DEBUG_STEP:
		{
			
			int64 PauseID = Letter.PopInt();
			StopPause(PauseID,CMD_DEBUG_STEP);
		}
		break;
	case CMD_RUN: //这种只可能是在调试状态下要求继续执行
		{
			WORK_MODE WorkMode = GetWorkMode();
			assert(WorkMode == WORK_DEBUG);

			SetWorkMode(WORK_TASK);

			int64 PauseID = Letter.PopInt();

			StopPause(PauseID,CMD_RUN);
		}
		break;

	case CMD_EXE:
		{
			ExePipe->Clear();
		    ClearTaskMsgList();			
			SetTaskState(TASK_EXE);			
			NotifyTaskState();
			
			bool ret =  CompileTask();
			if (!ret)
			{					
				SetTaskState(TASK_IDLE);
				NotifyTaskState();
				return ;	
			}
								
			ePipeline Receiver;
			Receiver.PushInt(m_DialogID);
			Receiver.PushInt(m_TaskID);
			
			CMsg TaskMsg(Receiver,MSG_ELT_TASK_CTRL,0);
			TaskMsg.GetLetter().PushInt(CMD_RUN);
			
			m_Brain->PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
		}
		break;
    case CMD_STOP:
		{
			if (GetTaskState() != TASK_STOP)
			{
                //注意：stop分为两步，首先发一个CMM_STOP信息给任务, 此时任务正在执行没有机会处理此信息，然后让执行管道无效，这会让正在执行的任务退出，但执行管道无效
				//不一定是任务停止，然后任务处理CMD_STOP,改变任务状态为TASK_STOP才算正式结束任务
		
				ePipeline Address;
				Address.PushInt(m_DialogID);
				Address.PushInt(m_TaskID);

				CMsg EltMsg(Address,MSG_ELT_TASK_CTRL,0);
				ePipeline& Letter = EltMsg.GetLetter();
				Letter.PushInt(CMD_STOP);
				
				//给Element发信息
				m_Brain->PushNerveMsg(EltMsg,false,false);
	
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
			Receiver.PushInt(m_DialogID);
			Receiver.PushInt(m_TaskID);
			
			CMsg TaskMsg(Receiver,MSG_ELT_TASK_CTRL,0);
			TaskMsg.GetLetter().PushInt(Cmd);
			TaskMsg.GetLetter()<<Letter;
			m_Brain->PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
		}

	}

}


void CLogicDialog::OnBrainGetDebugItem(ePipeline* ExePipe,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	int64 TaskID = Letter.PopInt();
	assert(TaskID == m_LogicItemTree.GetID());
    	
	//凡是暂停都会自动进入调试状态，先准备好调试数据
	if (m_LogicItemTree.Size()==0)
	{
		CLogicTask* Task = m_Brain->GetBrainData()->GetLogicTask(m_TaskID);
		Task->GetDebugItem(m_LogicItemTree);
	}

	m_LastDebugTimeStamp = TaskID;

	ePipeline PauseIDList;
	GetPauseIDList(PauseIDList);
	CNotifyDialogState nf(NOTIFY_DEBUG_VIEW);
	nf.PushInt(DEBUG_RESET);
	nf.Push_Directly(m_LogicItemTree.Clone());
	nf.PushPipe(PauseIDList);
	nf.Notify(this);
};

	/*
void CLogicDialog::OnBrainSetGloble(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){


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

void CLogicDialog::OnBrainLogicOperate(ePipeline* ExePipe,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
		
	int64 Type = Letter.PopInt();
	switch (Type)
	{
	case DEL_LOGIC:
        {
			tstring LogicName = Letter.PopString();
			
            DeleteLogic(LogicName);
		}
		break;
	case CLEAR_LOGIC:
		{
			ClearLogicSence();
		};
	default:
		return;
	}
	
}
void CLogicDialog::OnBrainObjectOperate(ePipeline* ExePipe,CMsg& Msg)
{
	ePipeline& Letter = Msg.GetLetter();
	
	int64 Type = Letter.PopInt();
	switch (Type)
	{
	case SELECT_OBJECT:
        {
			ePipeline* Item = (ePipeline*)Letter.GetData(0);			
			RegisterObject(*Item);
		}
		break;
	case DEL_OBJECT:
		{
			ePipeline* Item = (ePipeline*)Letter.GetData(0);			
			DeleteObject(*Item);
		}
		break;
	case CLR_OBJECT:
		{
			ClearObject();
		};
		break;
	default:
		return;
	}	
}

void CLogicDialog::OnBrainMemoryOperate(ePipeline* ExePipe,CMsg& Msg)
{

	TASK_STATE  State = GetTaskState(); 
	if (State!= TASK_IDLE && State != TASK_STOP)
	{
		RuntimeOutput(0,_T("Please execute this command in idle state"));
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
				ePipeline*  ObjectInfo = FindObjectInstance(InstanceID);
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
			CLogicTask* Task = GetTask();
			if (!Task)
			{
				return;
			}
			if (!Task->Compile(this,&Sentence))
			{
				return;
			}
			tstring CmdStr = Format1024(_T("Close %d object instance"),n);
			SaveReceiveItem(CmdStr,0);

			SetTaskState(TASK_EXE);			
			NotifyTaskState();

			ePipeline Receiver;
			Receiver.PushInt(m_DialogID);
			Receiver.PushInt(m_TaskID);

			CMsg TaskMsg(Receiver,MSG_ELT_TASK_CTRL,0);
			TaskMsg.GetLetter().PushInt(CMD_RUN);

			m_Brain->PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
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
			CLogicTask* Task = GetTask();
			if (!Task)
			{
				return;
			}
			if (!Task->Compile(this,&Sentence))
			{
				return;
			}
			tstring CmdStr = Format1024(_T("Close %d data instance"),n);
			SaveReceiveItem(CmdStr,0);

			SetTaskState(TASK_EXE);			
			NotifyTaskState();

			ePipeline Receiver;
			Receiver.PushInt(m_DialogID);
			Receiver.PushInt(m_TaskID);

			CMsg TaskMsg(Receiver,MSG_ELT_TASK_CTRL,0);
			TaskMsg.GetLetter().PushInt(CMD_RUN);

			m_Brain->PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
		}
	}

}
void CLogicDialog::OnBrainGetThinkResult( ePipeline* ExePipe,CMsg& Msg){
	
	tstring s = GetThink()->m_Text.PrintThinkResult();
	RuntimeOutput(s);
	
}


void CLogicDialog::OnBrainGetAnalyseResult(ePipeline* ExePipe,CMsg& Msg){
	tstring s = GetThink()->m_Text.PrintAnlyseResult();
	RuntimeOutput(s);
}



void CLogicDialog::OnBrainClearDialogOutput(ePipeline* ExePipe,CMsg& Msg){
	m_RuntimeOutput.Clear();
	//此处应该通知界面的，暂时简化如此
}

void CLogicDialog::OnBrainGetFindResult(ePipeline* ExePipe,CMsg& Msg){
	
	int64 n = Msg.GetLetter().PopInt();
	
	ePipeline SearchResult;
	CLogicThread* Think = GetThink();

	FindContinue(Think,(int32)n,SearchResult);
	
	m_SearchOutput = SearchResult;
	
	//输出结果,0条目也要通知
	CNotifyDialogState nf1(NOTIFY_FIND_VIEW);
	nf1.PushInt(FIND_UPDATE);
	nf1.PushPipe(SearchResult);
	nf1.Notify(this);
	
	
	//如果已经找到所有结果,通知搜索结束
	if ( m_FindSeedList.size()==0)
	{
		CNotifyDialogState nf2(NOTIFY_FIND_VIEW);
		nf2.PushInt(FIND_STOP);
		nf2.PushInt(TRUE);
		nf2.Notify(this);	
		m_SearchOutput.SetID(0); //没有更多结果
	}else{
		CNotifyDialogState nf2(NOTIFY_FIND_VIEW);
		nf2.PushInt(FIND_STOP);
        nf2.PushInt(FALSE);
		nf2.Notify(this);	
		m_SearchOutput.SetID(1); //还有更多结果
	}	
}

void CLogicDialog::OnBrainClearThink(ePipeline* ExePipe,CMsg& Msg){
	if (GetTaskState() == TASK_IDLE)
	{
		ResetThink();
		
		CNotifyDialogState nf(NOTIFY_DIALOG_STATE);
		nf.PushInt(GetWorkMode());
		nf.PushInt(TASK_IDLE);
        nf.PushString(_T(""));
		nf.Notify(this);
	}
}


void CLogicDialog::OnBrainConnectTo(ePipeline* ExePipe,CMsg& Msg){
			
	ePipeline& Letter = Msg.GetLetter();
	
	tstring ip   = Letter.PopString(); 
	int64   Port = Letter.PopInt();
	
	/*
	if (ip == _T("127.0.0.1"))
	{
		HANDLE  MutexOneInstance = ::CreateMutexW( NULL, FALSE,
			_T("SPACEPORTAL_CREATED"));

		bool AlreadyRunning = ( ::GetLastError() == ERROR_ALREADY_EXISTS || 
			::GetLastError() == ERROR_ACCESS_DENIED);
		// The call fails with ERROR_ACCESS_DENIED if the Mutex was 
		// created in a different users session because of passing
		// NULL for the SECURITY_ATTRIBUTES on Mutex creation);

		if ( !AlreadyRunning )
		{

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
#ifdef _DEBUG
			ExecuterFile += _T("\\SpacePortal_debug.exe");
#else 
			ExecuterFile += _T("\\SpacePortal.exe");
#endif
			STARTUPINFO si;
			PROCESS_INFORMATION  pi;	
			::ZeroMemory(&si, sizeof(si));	
			si.cb = sizeof(si);


			tstring CmdLine;
			CmdLine += m_Account.Name;
			CmdLine +=_T(" ");
			CmdLine += m_Account.CrypText;


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

			if (!started)
			{
				return; 
			}
			HANDLE  MutexOneInstance = ::CreateMutexW( NULL, FALSE,
				_T("SPACEPORTAL_CREATED"));

			bool AlreadyRunning = ( ::GetLastError() == ERROR_ALREADY_EXISTS || 
				::GetLastError() == ERROR_ACCESS_DENIED);
		}

	}
	*/

	tstring s = Format1024(_T("Connect to %s"),ip.c_str());
	SaveSendItem(s,0);
	
	if (Port == SPACE_PORT)
	{
		m_Brain->ConnectSpace(this,ip);
	}
}

void CLogicDialog::OnBrainDisconnect(ePipeline* ExePipe,CMsg& Msg){	
	CLinker Linker;
	m_Brain->GetLinker(SPACE_SOURCE,Linker);
	Linker().Close();
	
	m_Brain->GetBrainData()->DeleteDialogOfSource(SPACE_SOURCE);
	
	tstring ip ;

	ePipeline Receiver;
	Receiver.PushInt(m_SourceID);
	Receiver.PushInt(m_DialogID);
	
	CMsg GuiMsg2(Receiver,MSG_BRAIN_TO_GUI,0);		
	ePipeline Cmd(GUI_CONNECT_STATE);
	Cmd.PushInt(CON_END);
	Cmd.PushString(ip);
	Cmd.PushInt(FALSE);	
	GuiMsg2.GetLetter().PushPipe(Cmd);
	
	m_Brain->GetBrainData()->SendMsgToGUI(m_Brain,GuiMsg2,-1);	
	return;
	
};

void CLogicDialog::OnBrainSetLogFlag(ePipeline* ExePipe,CMsg& Msg)
{
	ePipeline& Letter = Msg.GetLetter();
	uint32 LogFlag = (uint32)Letter.PopInt();
	m_Brain->SetLogFlag(LogFlag);
}

void CLogicDialog::OnTaskResult(ePipeline* ExePipe,CMsg& Msg){	
	ePipeline& Letter = Msg.GetLetter();
	
	ePipeline* OldExePipe = (ePipeline*)Letter.GetData(0);
	ePipeline* NewExePipe = (ePipeline*)Letter.GetData(1);

	CPipeView PipeView(NewExePipe);
	tstring s  = PipeView.GetString();

	RuntimeOutput(0,s);
	
	int64 ID = NewExePipe->GetID();

	if (!NewExePipe->IsAlive())
	{
		tstring text = _T("Stop");
		SaveSendItem(text,0); 
	}
	else{
		tstring text = _T("ok");
		SaveSendItem(text,0);    
	}
	
	ResetTask();

	//继续生成一个与任务对话对应的事件
	ePipeline EmptyExePipe;
	ePipeline Address(Msg.GetSourceID());
	int64 EventID = m_DialogID;
	Address.PushInt(EventID);
	
#ifdef _DEBUG
	tstring memo = Format1024(_T("%s %d"),_T(__FILE__),__LINE__);
	m_Brain->GetBrainData()->PushBrainEvent(EventID,EventID,EmptyExePipe,Address,TIME_SEC,false,memo);
#else
	m_Brain->GetBrainData()->PushBrainEvent(EventID,EventID,EmptyExePipe,Address,TIME_SEC,false);
#endif
	//如果有的话，准备执行下一个逻辑任务
	CLogicThread* CurThink = GetThink();	
	if(CurThink->m_Text.m_SentenceList.size()==0){
		ResetThink();	
	}else{
		ePipeline ChildMsg(GUI_TASK_CONTROL);
		ChildMsg.PushInt(CMD_EXE);
		
		CMsg BrainMsg;
		CreateBrainMsg(BrainMsg,m_DialogID,ChildMsg,0);	
		m_Brain->PushNerveMsg(BrainMsg,false,false);
	}		
};

void CLogicDialog::OnEventTick(ePipeline* ExePipe,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	ePipeline& Letter = Msg.GetLetter();
	int64 TimeStamp = Letter.PopInt();

	m_Brain->GetBrainData()->ResetEventTickCount(EventID);

	//SaveReceiveItem(_T("ResetEventTickCount"),0);
}
