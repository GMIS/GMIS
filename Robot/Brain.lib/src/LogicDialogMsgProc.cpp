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
		GetBrain()->NotifySysState(NOTIFY_ILLEGAL_MSG,NULL, &Msg.GetMsg());
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
	if (SourceID != SYSTEM_SOURCE)  //注意此消息的源必须是SYSTEM_SOURCE
	{
		GetBrain()->NotifySysState(NOTIFY_ILLEGAL_MSG,NULL, &Msg.GetMsg());
		return;
	}
	
    ePipeline& Letter = Msg.GetLetter(true);

	int64 ChildMsgID = Letter.PopInt();
	
	switch(ChildMsgID){
	case TO_BRAIN_MSG::GUI_IO_INPUTING:
		{
			OnBrainTextInputing(ExePipe,Msg);	
		}
		break;
	case TO_BRAIN_MSG::GUI_IO_INPUTED:
		{
			OnBrainTextInputEnd(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_GET_DIALOG_MORE_ITME:
        {
			OnBrainGetMoreLog(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_SET_FOUCUS_DIALOG:
        {
			OnBrainSetFocusDialog(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::TASK_CONTROL::ID:
		{
			OnBrainTaskControl(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_GET_DEBUG_ITEM:
		{
			OnBrainGetDebugItem(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_LOGIC_OPERATE::ID:
		{
			OnBrainLogicOperate(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_OBJECT_OPERATE::ID:
		{
			OnBrainObjectOperate(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_MEMORY_OPERATE:
		{
			OnBrainMemoryOperate(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_GET_THINK_RESULT:
		{
			OnBrainGetThinkResult(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_GET_ANALYSE_RESULT:
		{
			OnBrainGetAnalyseResult(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_CLEAR_DIALOG_OUTPUT:
		{
			OnBrainClearDialogOutput(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_GET_FIND_REUSLT:
		{
			OnBrainGetFindResult(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_CLEAR_THINK:
		{
			OnBrainClearThink(ExePipe,Msg);
		}
		break;

	case TO_BRAIN_MSG::GUI_CONNECT_TO:
		{
			OnBrainConnectTo(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_DISCONNECT:
		{
			OnBrainDisconnect(ExePipe,Msg);
		}
		break;
	case TO_BRAIN_MSG::GUI_SET_LOG_FLAG:
		{
			OnBrainSetLogFlag(ExePipe,Msg);
		}
		break;
	default:
		GetBrain()->NotifySysState(NOTIFY_ILLEGAL_MSG, NULL,&Msg.GetMsg());
		return;
	}
}

void CLogicDialog::OnBrainTextInputing(ePipeline* ExePipe,CMsg& Msg){

	GetThink()->ThinkProc(this,Msg);
};

void CLogicDialog::OnBrainTextInputEnd(ePipeline* ExePipe,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter(true);
		
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
			SetThinkState(THINK_ERROR);
			return;
		};	
	
		if(m_DialogType == DIALOG_SYSTEM){
			Think2TaskList();

		    //系统对话不执行具体逻辑任务，而是生成一个任务对话执行	
			int64 DialogID = AbstractSpace::CreateTimeStamp();

			tstring DialogName = Format1024(_T("Logic%I64ld"),m_TaskDialogCount++);

			CLogicDialog* TaskDialog = GetBrain()->GetBrainData()->CreateNewDialog(m_SourceID,DialogID,m_DialogID,m_DialogName,DialogName,DIALOG_TASK,TASK_OUT_DEFAULT);
			if (!TaskDialog)
			{
				tstring text = _T("The task transferred fail");
				SaveDialogItem(text,m_DialogName,0);
				return;
			}
			ePipeline  ClientExePipe;
			ePipeline  ClientAddress(m_SourceID);
			ClientAddress.PushInt(m_DialogID);

			bool bFocus = true;
			bool bOnce  = false;

			//生成一个与任务对话对应的事件
			GetBrain()->GetBrainData()->RegisterEvent(DialogID,DialogID,ClientExePipe,ClientAddress,TIME_SEC,bOnce,0,false);

			CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
			nf.PushInt(DL_ADD_DIALOG);
			nf.PushInt(bFocus); //是否设置为当前对话
			nf.Notify(TaskDialog);

			
			//把当前逻辑环境转交给新的子对话
			*TaskDialog<<*this;

			CMsg Msg(SYSTEM_SOURCE,TaskDialog->m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,0);
			ePipeline& Letter = Msg.GetLetter(false);
			Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
			Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE);

			Letter.PushPipe(ClientExePipe);

			GetBrain()->PushNerveMsg(Msg,false,false);

			ResetThink();			
		}
		else
		{
			if(m_TaskID==0){  //还没有逻辑任务，则自己执行

				Think2TaskList();

				CMsg Msg(SYSTEM_SOURCE,m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,0);
				ePipeline& Letter = Msg.GetLetter(false);
				Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
				Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE);

				ExePipe->Clear();
				Letter.PushPipe(*ExePipe);
				GetBrain()->PushNerveMsg(Msg,false,false);
			}
			else 
			{	//已经有任务在身，则生成一个子对话，委托给它执行		

				tstring EventName = m_EditText;
				assert(EventName.size()!=0);

				EventName.erase(EventName.size()-1); //delete ";"

				if(EventName.size()>10){
					EventName = EventName.substr(0,7)+_T("...");
				}

				//生成一个临时对话和事件
				int64 EventID = AbstractSpace::CreateTimeStamp();

				ePipeline ClientExePipe;
				ePipeline ClientAddress(m_SourceID);
				ClientAddress.PushInt(m_DialogID);
				CLogicDialog* ChildDialog = StartEventDialog(EventID,EventName,TASK_OUT_DEFAULT,ClientExePipe,ClientAddress,TIME_SEC,false,false,true);


				if(!ChildDialog){
					RuntimeOutput(0,_T("Start event  '%s' failed: can't create dialog"),EventName.c_str());		
					ResetThink();
					return;
				}else{
					RuntimeOutput(0,_T("Start event  '%s' "),EventName.c_str());
				}

				ChildDialog->m_ThinkID = m_ThinkID;
				GetBrain()->GetBrainData()->SetLogicThreadUser(m_ThinkID,EventID);
				ChildDialog->m_EditText = m_EditText;

				ChildDialog->Think2TaskList();

				ResetThink();
			
				//SaveSendItem(ChildDialog->m_EditText,0);

				CMsg Msg (SYSTEM_SOURCE,ChildDialog->m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,0);
				ePipeline& Letter = Msg.GetLetter(false);
				Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
				Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE);
				Letter.PushPipe(ClientExePipe);
				GetBrain()->PushNerveMsg(Msg,false,false);
			}
		}
	} 
	else
	{	
		assert(m_ThinkID!=0);

		ExePipe->Clear();
		ExePipe->PushInt(m_ThinkID);

		if(m_DialogType == DIALOG_EVENT){
			CBrainEvent EventInfo;
			bool ret = GetBrain()->GetBrainData()->GetEvent(m_DialogID,EventInfo,false);
			if (ret) 
			{
				CMsg Msg1(SYSTEM_SOURCE,EventInfo.m_ClientAddress,MSG_TASK_RESULT,DEFAULT_DIALOG,m_DialogID);
				ePipeline& Letter = Msg1.GetLetter(false);
				Letter.PushPipe(*ExePipe);	
				GetBrain()->PushNerveMsg(Msg1,false,false);
				
			}
		}
	}
};


void CLogicDialog::OnBrainGetMoreLog(ePipeline* ExePipe,CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter(true);
	int64 LastItemID = Letter.PopInt();
	
	ePipeline Pipe;
	GetBrain()->GetBrainData()->GetMoreLog(m_SourceID,m_DialogID,LastItemID,Pipe);
	
	CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
	nf.PushInt(DIALOG_INFO_MORE);
	nf.PushPipe(Pipe);
	nf.Notify(this);
	
};
void CLogicDialog::OnBrainSetFocusDialog(ePipeline* ExePipe,CMsg& Msg){

	
	ePipeline& Letter  = Msg.GetLetter(true);
        
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
	GetBrain()->GetBrainData()->GetFocusDialogData(m_SourceID,m_DialogID,Pipe,bIncludeDebugInfo);
	nf<<Pipe;
	
	//运行时输出记录 
	nf.Notify(this);
}
void CLogicDialog::OnBrainTaskControl(ePipeline* ExePipe,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter(true);
	int64 Cmd = Letter.PopInt();
		
	switch (Cmd)
	{
	case TO_BRAIN_MSG::TASK_CONTROL::CMD_DEBUG_STEP:
		{	
			int64 PauseID = Letter.PopInt();
			StopPause(PauseID,TO_BRAIN_MSG::TASK_CONTROL::CMD_DEBUG_STEP);
		}
		break;
	case TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN: //这种只可能是在调试状态下要求继续执行
		{
			WORK_MODE WorkMode = GetWorkMode();
			int64 PauseID = Letter.PopInt();

			StopPause(PauseID,TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN);
		}
		break;

	case TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE:
		{
			ExePipe->Clear();
		    ClearEltMsgList();			
			SetTaskState(TASK_COMPILE);			
			

			ePipeline* InputExePipe = (ePipeline*)Letter.GetData(0);
			*ExePipe<<*InputExePipe;

			bool ret =  CompileTask();
			if (!ret)
			{		
				
				ExePipe->Break();
				if(m_DialogType == DIALOG_EVENT){
					CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(m_SourceID,m_ParentDialogID);
					assert(ParentDlg);
					if(ParentDlg){
						ParentDlg->RuntimeOutput(0,m_CompileError);
					}
					
				}
				FeedbackToBrain();
				return ;	
			}



			ePipeline Receiver;
			Receiver.PushInt(m_DialogID);
			Receiver.PushInt(m_TaskID);
			
			CMsg TaskMsg(m_SourceID,Receiver,MSG_ELT_TASK_CTRL,DEFAULT_DIALOG,0);
			TaskMsg.GetLetter(false).PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN);
			
			GetBrain()->PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
		}
		break;
    case TO_BRAIN_MSG::TASK_CONTROL::CMD_STOP:
		{
			if (GetTaskState() != TASK_STOP)
			{
				/* 注意：Stop分为两步，首先发一个CMM_STOP信息给任务, 此时任务正在执行没有机会
				   处理此信息，然后让执行管道无效，这会让正在执行的任务退出，但执行管道无效不
				   一定是任务停止，然后任务处理CMD_STOP,改变任务状态为TASK_STOP才算正式结束任务
				*/
		
				ePipeline Address;
				Address.PushInt(m_DialogID);
				Address.PushInt(m_TaskID);

				CMsg EltMsg(m_SourceID,Address,MSG_ELT_TASK_CTRL,DEFAULT_DIALOG,0);
				ePipeline& Letter = EltMsg.GetLetter(false);
				Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_STOP);
				
				//给Element发信息
				GetBrain()->PushNerveMsg(EltMsg,false,false);
	
				ExePipe->Break();		
			}
		}
		break;
	case TO_BRAIN_MSG::TASK_CONTROL::CMD_PAUSE:
		{
			ExePipe->SetID(RETURN_BREAK);	
		}
		break;
	case TO_BRAIN_MSG::TASK_CONTROL::CMD_SET_BREAKPOINT:
		{
			if (GetTaskState() != TASK_STOP){
				ePipeline Receiver;
				Receiver.PushInt(m_DialogID);
				Receiver.PushInt(m_TaskID);

				CMsg TaskMsg(m_SourceID,Receiver,MSG_ELT_TASK_CTRL,DEFAULT_DIALOG,0);
				TaskMsg.GetLetter(false).PushInt(Cmd);
				TaskMsg.GetLetter(false)<<Letter;
				GetBrain()->PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
			}
		}
		break;
	default:
		{
			assert(0);
			GetBrain()->NotifySysState(NOTIFY_ILLEGAL_MSG,NULL, &Msg.GetMsg());
		}

	}

}


void CLogicDialog::OnBrainGetDebugItem(ePipeline* ExePipe,CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter(true);
	int64 TaskID = Letter.PopInt();
	assert(TaskID == m_LogicItemTree.GetID());
    	
	//凡是暂停都会自动进入调试状态，先准备好调试数据
	if (m_LogicItemTree.Size()==0)
	{
		CLogicTask* Task = GetBrain()->GetBrainData()->GetLogicTask(m_TaskID);
		if(Task){
			Task->GetDebugItem(m_LogicItemTree);
		}
	}

	m_LastDebugTimeStamp = TaskID;

	ePipeline PauseIDList;
	GetPauseMassIDList(PauseIDList);
	CNotifyDialogState nf(NOTIFY_DEBUG_VIEW);
	nf.PushInt(DEBUG_RESET);
	nf.Push_Directly(m_LogicItemTree.Clone());
	nf.PushPipe(PauseIDList);
	nf.Notify(this);
};

/*
void CLogicDialog::OnBrainSetGloble(CTaskDialog* Dialog,ePipeline* ExePipe,CMsg& Msg){


	ePipeline& Letter = Msg.GetLetter(true);

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
			GuiMsg.GetLetter(false).PushPipe(Cmd);
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
			GuiMsg.GetLetter(false).PushPipe(Cmd);
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
	ePipeline& Letter = Msg.GetLetter(true);
		
	int64 Type = Letter.PopInt();
	switch (Type)
	{
	case TO_BRAIN_MSG::GUI_LOGIC_OPERATE::DEL_LOGIC:
        {
			tstring LogicName = Letter.PopString();
			
            DeleteLogic(LogicName);
		}
		break;
	case TO_BRAIN_MSG::GUI_LOGIC_OPERATE::CLEAR_LOGIC:
		{
			ClearLogicSence();
		};
	default:
		return;
	}
	
}
void CLogicDialog::OnBrainObjectOperate(ePipeline* ExePipe,CMsg& Msg)
{
	ePipeline& Letter = Msg.GetLetter(true);
	
	int64 Type = Letter.PopInt();
	switch (Type)
	{
	case TO_BRAIN_MSG::GUI_OBJECT_OPERATE::SELECT_OBJECT:
        {
			ePipeline* Item = (ePipeline*)Letter.GetData(0);			
			RegisterObject(*Item);
		}
		break;
	case TO_BRAIN_MSG::GUI_OBJECT_OPERATE::DELETE_OBJECT:
		{
			ePipeline* Item = (ePipeline*)Letter.GetData(0);			
			DeleteObject(*Item);
		}
		break;
	case TO_BRAIN_MSG::GUI_OBJECT_OPERATE::CLR_OBJECT:
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
	if (State != TASK_STOP)
	{
		assert(0);  //鼠标操作需要避免和任务自身冲突
		RuntimeOutput(0,_T("Please execute this command in stop state"));
		return ;
	};

	ePipeline& Letter = Msg.GetLetter(true);

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
			SaveDialogItem(CmdStr,m_DialogName,0);


			ePipeline Receiver;
			Receiver.PushInt(m_DialogID);
			Receiver.PushInt(m_TaskID);

			CMsg TaskMsg(m_SourceID,Receiver,MSG_ELT_TASK_CTRL,DEFAULT_DIALOG,0);
			TaskMsg.GetLetter(false).PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN);

			GetBrain()->PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
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
				Clause1.PushInt(INSTINCT_FOCUS_MEMORY);
				Clause1.PushString(InstanceName);

				ePipeline Clause2;
				Clause2.PushInt(INSTINCT_CLOSE_MEMORY);

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
			SaveDialogItem(CmdStr,m_DialogName,0);

//			SetTaskState(TASK_EXE);			

			ePipeline Receiver;
			Receiver.PushInt(m_DialogID);
			Receiver.PushInt(m_TaskID);

			CMsg TaskMsg(m_SourceID,Receiver,MSG_ELT_TASK_CTRL,DEFAULT_DIALOG,0);
			TaskMsg.GetLetter(false).PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN);

			GetBrain()->PushNerveMsg(TaskMsg,false,false); //由于可能改变了任务内部，导致迭代器失效，所以必须经由系统分发信息
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
	
	int64 n = Msg.GetLetter(true).PopInt();
	
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
	ResetThink();
}


void CLogicDialog::OnBrainConnectTo(ePipeline* ExePipe,CMsg& Msg){
			
	ePipeline& Letter = Msg.GetLetter(true);
	
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
	SaveDialogItem(s,m_DialogName,0);
	
	if (Port == SPACE_PORT)
	{
		GetBrain()->ConnectSpace(this,ip);
	}
}

void CLogicDialog::OnBrainDisconnect(ePipeline* ExePipe,CMsg& Msg){	
	CLinker Linker;
	GetBrain()->GetLinker(SPACE_SOURCE,Linker);
	Linker().Close();
		
	return;
	
};

void CLogicDialog::OnBrainSetLogFlag(ePipeline* ExePipe,CMsg& Msg)
{
	ePipeline& Letter = Msg.GetLetter(true);
	uint32 LogFlag = (uint32)Letter.PopInt();
	GetBrain()->SetLogFlag(LogFlag);
}




void CLogicDialog::OnTaskResult(ePipeline* ExePipe,CMsg& Msg){	
	 
	//缺省处理只显示信息
	ePipeline& Letter = Msg.GetLetter(true);

	ePipeline* NewExePipe = (ePipeline*)Letter.GetData(0);

	CPipeView PipeView(NewExePipe);
	tstring s  = PipeView.GetString();

	RuntimeOutput(0,s);

	int64 EventID = Msg.GetEventID();
	assert(EventID>0);

	CLogicDialog* ChildDlg = GetBrain()->GetBrainData()->GetDialog(m_SourceID,EventID);
	if(ChildDlg){			
		if(ChildDlg->HasTask()){

			CMsg Msg(m_SourceID,ChildDlg->m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,0);
			ePipeline& Letter = Msg.GetLetter(false);
			Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
			Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE);
			NewExePipe->Clear();
			Letter.PushPipe(*NewExePipe);

			GetBrain()->PushNerveMsg(Msg,false,false);
			return;
		}

		ChildDlg->SetThinkState(THINK_IDLE); //仅仅起到重置GUI界面的作用
		CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
		nf.PushInt(DL_DEL_DIALOG);
		nf.Notify(ChildDlg);

		GetBrain()->GetBrainData()->DeleteDialog(ChildDlg->m_SourceID,ChildDlg->m_DialogID);
	}

	CBrainEvent EventInfo;
	GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,true);
};

void CLogicDialog::OnEventTick(ePipeline* ExePipe,CMsg& Msg){
	int64 EventID = Msg.GetEventID();
	ePipeline& Letter = Msg.GetLetter(true);
	int64 TimeStamp = Letter.PopInt();

	GetBrain()->GetBrainData()->ResetEventTickCount(EventID);

	//SaveReceiveItem(_T("ResetEventTickCount"),0);
}
