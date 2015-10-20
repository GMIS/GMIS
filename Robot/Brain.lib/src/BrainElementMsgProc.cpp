#pragma warning (disable:4786)

#include "Brain.h"
#include "GUIMsgDefine.h"
#include "BrainObject.h"
#include "LogicDialog.h"

MsgProcState CBrain::ElementMsgProc(CLogicDialog* Dialog,CElement* Elt,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 MsgID = Msg.GetMsgID();

	switch(MsgID){
	case MSG_ELT_TASK_CTRL:
		return OnEltTaskControl(Dialog,Elt,ChildIndex,Msg,ExePipe,LocalAddress);
		break;
	case MSG_EVENT_TICK:
		{
		   OnEventTick(Dialog,&ExePipe,Msg);
		   ExePipe.Break(); //不在继续处理,并返回系统
		   return RETURN_DIRECTLY; 
		}
		break;
	case MSG_ELT_INSERT_LOGIC:
		return OnEltInsertLogic(Dialog,Elt,ChildIndex,Msg,ExePipe,LocalAddress);
	case MSG_ELT_REMOVE_LOGIC:
		return OnEltRemoveLogic(Dialog,Elt,ChildIndex,Msg,ExePipe,LocalAddress);
	case MSG_TASK_RESULT:
		return OnEltTaskResult(Dialog,Elt,ChildIndex,Msg,ExePipe,LocalAddress);
	default:
		assert(0);
		break;
	}


	ExePipe.Break(); //不在继续处理,并返回系统
	return RETURN_DIRECTLY; 
}


MsgProcState CBrain::OnEltTaskControl(CLogicDialog* Dialog,CElement* Elt,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 EventID = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();
	int64 Cmd = Letter.PopInt();
	
	switch (Cmd)
	{
	case CMD_RUN:
		{
			WORK_MODE WorkMode = Dialog->GetWorkMode();

			if (WorkMode == WORK_THINK)
			{				
				Dialog->SetWorkMode(WORK_TASK);

				CLogicTask* Task = Dialog->GetTask();
				bool ret = Dialog->RegisterLogic(Task);
				if (!ret)
				{
					Dialog->RuntimeOutput(Dialog->m_CompileError);
				}
				
				Dialog->m_TaskID = 0;

				ExePipe.SetID(RETURN_NORMAL);	
				Dialog->SetTaskState(TASK_RUN);
				Dialog->NotifyTaskState();

			}else if (WorkMode == WORK_DEBUG)
			{
				ExePipe.SetID(RETURN_BREAK);	
				Dialog->SetTaskState(TASK_RUN);
				Dialog->NotifyTaskState();
			}else{
				assert(WorkMode == WORK_TASK);

				if(Dialog->GetTaskState() == TASK_PAUSE){

					Dialog->ClosePauseDialog(EventID);

					ePipeline& NewExePipe = *(ePipeline*)Letter.GetData(0);					
					ExePipe.Clear();
					ExePipe << NewExePipe;
					Letter.Clear();
				}else{
					//正常调试之初应该重置DebugView
					if (Dialog->m_LogicItemTree.Size())
					{
						Dialog->m_LogicItemTree.Clear();
						Dialog->m_LogicItemTree.SetID(0);
						ePipeline PauseIDList;

						CNotifyState nf(NOTIFY_DEBUG_VIEW);
						nf.PushInt(DEBUG_RESET);
						nf.PushPipe(PauseIDList);
						nf.PushPipe(Dialog->m_LogicItemTree);
						nf.Notify(Dialog);

					}
					
				}

				ExePipe.Reuse();
				ExePipe.SetID(RETURN_NORMAL);	
				Dialog->SetTaskState(TASK_RUN);
				Dialog->NotifyTaskState();
			}
		}
		break;
	case CMD_PAUSE:
		{
			ExePipe.SetID(RETURN_BREAK);
		}
		break;
	case CMD_DEBUG_STEP:
		{
			Dialog->ClosePauseDialog(EventID);

			TASK_STATE State = Dialog->GetTaskState();
			if(State != TASK_PAUSE && State != TASK_WAIT){
				return RETURN_DIRECTLY;	
			}

			ePipeline& NewExePipe = *(ePipeline*)Letter.GetData(0);
			
			ExePipe.Clear();
			ExePipe << NewExePipe;
			ExePipe.SetID(RETURN_BREAK);
			
			return CONTINUE_TASK;
		}
		break;
	case CMD_STOP:
		{
			Dialog->SetTaskState(TASK_STOP);
			
			ExePipe.Break(); //不在继续处理,并返回系统
		    return RETURN_DIRECTLY; 
		}
		break;
	case  CMD_DEBUG_BREAK:
		{
			int64 bBreak = Letter.PopInt();
			ePipeline* Path = (ePipeline*)Letter.GetData(0);

			Dialog->SetBreakPoint(*Path,bBreak);
			return RETURN_DIRECTLY; 
		}
		break;
	default:
		break;
	}
	return CONTINUE_TASK;
}

MsgProcState CBrain::OnEltInsertLogic(CLogicDialog* Dialog,CElement* Elt,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	int64 EventID = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter();

	tstring InsertLogicName = Letter.PopString();

	CLocalLogicCell* lg = Dialog->FindLogic(InsertLogicName);
	
	CLogicTask* LogicTask = NULL;
	if(lg == NULL) {				
		ExePipe.m_Label = Format1024(_T("Error: Not find Local logic[%s]"),InsertLogicName.c_str());
		ExePipe.SetID(RETURN_ERROR);

		CMsg rMsg(MSG_TASK_FEEDBACK,0,EventID);		
		ePipeline& rLetter = rMsg.GetLetter();
		rLetter.PushPipe(ExePipe);	
		
		Dialog->m_Brain->PushCentralNerveMsg(rMsg,false,false);	

		ExePipe.Break(); //不在继续处理,并返回系统
		return RETURN_DIRECTLY; 
	}		
	else if(!lg->IsValid()) {
		ExePipe.m_Label = Format1024(_T("Error: Local logic[%s] not valid"),InsertLogicName.c_str());
		ExePipe.SetID(RETURN_ERROR);
		
		CMsg rMsg(MSG_TASK_FEEDBACK,0,EventID);		
		ePipeline& rLetter = rMsg.GetLetter();
		rLetter.PushPipe(ExePipe);	
		
		Dialog->m_Brain->PushCentralNerveMsg(rMsg,false,false);	

		ExePipe.Break(); //不在继续处理,并返回系统
		return RETURN_DIRECTLY; 
	}else{
		LogicTask = &lg->m_Task;	
	}
	
	CLogicTask* Task = Dialog->GetTask();
	ePipeline LogicPipe(LogicTask->m_LogicData);
		
	CElement* E  = Task->CompileSentence(Dialog,InsertLogicName,&LogicPipe);
	
	if (!E)
	{

		ExePipe.m_Label = Format1024(_T("Insert logic [%s]:[%s]" ),InsertLogicName.c_str(),Task->m_CompileError.c_str());
		ExePipe.SetID(RETURN_ERROR);
		
		CMsg rMsg(MSG_TASK_FEEDBACK,0,EventID);		
		ePipeline& rLetter = rMsg.GetLetter();
		rLetter.PushPipe(ExePipe);	
		
		Dialog->m_Brain->PushCentralNerveMsg(rMsg,false,false);	
		
		ExePipe.Break(); //不在继续处理,并返回系统
		return RETURN_DIRECTLY; 
	}

	
	Elt->InsertLogic(-1,E); //插入末尾

	OutputLog(LOG_MSG_RUNTIME_TIP,_T("Execute inserting logic:1  EventID:%I64ld"),EventID);
	
	/* 改为整体重置
	ePipeline DebugItemList;
	Task->GetDebugItem(DebugItemList,E);

	assert(DebugItemList.Size()==1);
	ePipeline* DebugItem = (ePipeline*)DebugItemList.GetData(0);

	ePipeline* InsertAddress = (ePipeline*)LocalAddress.Clone();
	int64 DialogID = InsertAddress->PopInt(); //第一个对话ID,这个无需发送给GUI
    int64 TaskID   = InsertAddress->PopInt(); //第二个是任务ID也用不到

	CNotifyState nf(NOTIFY_DEBUG_VIEW);
	nf.PushInt(DEBUG_INSERT_LOGIC);
	nf.Push_Directly(InsertAddress);
	nf.PushPipe(*DebugItem);
	nf.Notify(Dialog);
	*/


	if (Dialog->m_LogicItemTree.Size())
	{
		Dialog->m_LogicItemTree.Clear();
		Task->GetDebugItem(Dialog->m_LogicItemTree);

		ePipeline PauseIDList;
		Dialog->GetPauseIDList(PauseIDList);

		CNotifyState nf(NOTIFY_DEBUG_VIEW);
		nf.PushInt(DEBUG_RESET);
		nf.Push_Directly(Dialog->m_LogicItemTree.Clone());
		nf.PushPipe(PauseIDList);
		nf.Notify(Dialog);
	}

	ExePipe.SetID(RETURN_NORMAL);

	CMsg rMsg(MSG_TASK_FEEDBACK,0,EventID);		
	ePipeline& rLetter = rMsg.GetLetter();
	rLetter.PushPipe(ExePipe);	
	Dialog->m_Brain->PushCentralNerveMsg(rMsg,false,false);		

	ExePipe.Break(); //不在继续处理,并返回系统
	return RETURN_DIRECTLY; 
};

MsgProcState CBrain::OnEltRemoveLogic(CLogicDialog* Dialog,CElement* Elt,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 EventID = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();
	
	int64 ChildID = Letter.PopInt();

 	//递归查找所有Element实例，确保其在实例列表中的登记项目被删除
	//Mass* E = Elt->FindMass(ChildID);
	//Dialog->RemoveLogicInstance(E);

	bool ret = Elt->RemoveLoigc(ChildID);
	//assert(ret);

	OutputLog(LOG_MSG_RUNTIME_TIP,_T("Executer removing logic:%d EventID:%I64ld "),ret,EventID);

	if (!ret)
	{
		ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("Remove logic fail EventID:%I64ld"),EventID);
		CMsg rMsg(MSG_TASK_FEEDBACK,0,EventID);		
		ePipeline& rLetter = rMsg.GetLetter();
		rLetter.PushPipe(ExePipe);	
		Dialog->m_Brain->PushCentralNerveMsg(rMsg,false,false);		

		ExePipe.Break(); //不在继续处理,并返回系统
		return RETURN_DIRECTLY; 
	}
	/*改为整体重置
	ePipeline* RemoveAddress = (ePipeline*)LocalAddress.Clone();
	int64 DialogID = RemoveAddress->PopInt(); //第一个对话ID,这个无需发送给GUI
    int64 TaskID   = RemoveAddress->PopInt(); //第二个是任务ID也用不到


	CNotifyState nf(NOTIFY_DEBUG_VIEW);
	nf.PushInt(DEBUG_REMOVE_LOGIC);
	nf.Push_Directly(RemoveAddress);
	nf.PushInt(ChildID);
	nf.Notify(Dialog);
	*/

	if (Dialog->m_LogicItemTree.Size())
	{
		CLogicTask* Task = Dialog->GetTask();
		Dialog->m_LogicItemTree.Clear();
		Task->GetDebugItem(Dialog->m_LogicItemTree);

		ePipeline PauseIDList;
		Dialog->GetPauseIDList(PauseIDList);

		CNotifyState nf(NOTIFY_DEBUG_VIEW);
		nf.PushInt(DEBUG_RESET);
		nf.Push_Directly(Dialog->m_LogicItemTree.Clone());
		nf.PushPipe(PauseIDList);
		nf.Notify(Dialog);
	}
	
	CMsg rMsg(MSG_TASK_FEEDBACK,0,EventID);		
	ePipeline& rLetter = rMsg.GetLetter();
	rLetter.PushPipe(ExePipe);	
	Dialog->m_Brain->PushCentralNerveMsg(rMsg,false,false);		
	

	ExePipe.Break(); //不在继续处理,并返回系统
	return RETURN_DIRECTLY; 
}

MsgProcState CBrain::OnEltTaskResult(CLogicDialog* Dialog,CElement* Elt,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	int64 EventID = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();
	ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
	ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);
	
	
	if (!NewExePipe->IsAlive())
	{
		Dialog->CloseChildDialog(EventID,*OldExePipe,ExePipe);
		
		tstring Answer = _T("收到的数据管道已经无效");
		ExePipe.SetLabel(Answer.c_str());
		ExePipe.SetID(RETURN_ERROR);
		return RETURN_DIRECTLY;
	}
	
	int64 retTask = NewExePipe->GetID();
	if (retTask == RETURN_ERROR)
	{
		Dialog->CloseChildDialog(EventID,*OldExePipe,ExePipe);
		ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
		ExePipe.SetID(RETURN_ERROR);
		return RETURN_DIRECTLY;
	}
	
	ExePipe.Clear();
	ExePipe << *OldExePipe;
	
	Dialog->CloseChildDialog(EventID,*OldExePipe,ExePipe);

	return CONTINUE_TASK;

}