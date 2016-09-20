#pragma warning (disable:4786)

#include "Brain.h"
#include "Element.h"
#include "GUIMsgDefine.h"
#include "InstinctDefine.h"
#include "LogicDialog.h"

MsgProcState CElement::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 MsgID = Msg.GetMsgID();

	switch(MsgID){
	case MSG_ELT_TASK_CTRL:
		return OnEltTaskControl(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		break;
	case MSG_EVENT_TICK:
		{
		   int64 EventID = Msg.GetEventID();
		   ePipeline& Letter = Msg.GetLetter(true);
		   int64 TimeStamp = Letter.PopInt();

		   GetBrain()->GetBrainData()->ResetEventTickCount(EventID);

		   //Dialog->SaveReceiveItem(_T("ResetEventTickCount"),0);

		   ExePipe.Break(); //不在继续处理,并返回系统
		   return RETURN_DIRECTLY; 
		}
		break;
	case MSG_ELT_INSERT_LOGIC:
		return OnEltInsertLogic(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
	case MSG_ELT_REMOVE_LOGIC:
		return OnEltRemoveLogic(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
	case MSG_TASK_RESULT:
		return OnEltTaskResult(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
	default:
		assert(0);
		break;
	}

	ExePipe.Break(); //不在继续处理,并返回系统
	return RETURN_DIRECTLY; 
}


MsgProcState CElement::OnEltTaskControl(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 EventID = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter(true);
	int64 Cmd = Letter.PopInt();
	
	switch (Cmd)
	{
	case TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN:
		{
			if(Dialog->m_DialogType == DIALOG_EVENT){
				int64 EventID = Dialog->m_DialogID;
				CBrainEvent EventInfo;
				bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,false);
				if (!ret) 
				{
					ExeError(ExePipe,Format1024(_T("lost event %I64ld"),EventID));
					return CONTINUE_TASK;
				}

				CLogicDialog* ParentDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,Dialog->m_ParentDialogID);
				if (ParentDlg==NULL)
				{
					ExeError(ExePipe,_T("not find parent dialog"));
					return CONTINUE_TASK;
				}

				WORK_MODE WorkMode = ParentDlg->GetWorkMode();
				if(WorkMode == WORK_THINK){
					tstring& CurLogicName = ParentDlg->m_CurLogicName;
					ParentDlg->SetWorkMode(WORK_DEBUG);

					CLogicTask* Task = Dialog->GetTask();
					Task->m_Name = CurLogicName;
					ParentDlg->m_CurLogicName = _T("");

					bool ret = ParentDlg->RegisterLogic(Task);
					if (!ret)
					{
						ParentDlg->RuntimeOutput(ParentDlg->m_CompileError);
					}	
					ExePipe.SetID(RETURN_NORMAL);	
					Dialog->SetTaskState(TASK_RUN);
					return CONTINUE_TASK;
				}
			}

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

				ExePipe.SetID(RETURN_NORMAL);	
				Dialog->SetTaskState(TASK_RUN);

			}else if (WorkMode == WORK_DEBUG)
			{
				ExePipe.Clear();
				if(EventID>0){
					Dialog->ClosePauseDialog(EventID);
					ePipeline& NewExePipe = *(ePipeline*)Letter.GetData(0);
					ExePipe << NewExePipe;
					ExePipe.SetID(RETURN_NORMAL);
				}else{
					ExePipe.SetID(RETURN_BREAK);
				}
				
				Dialog->SetTaskState(TASK_RUN);
				return CONTINUE_TASK;

			}else{
				assert(WorkMode == WORK_TASK || WorkMode ==WORK_TEST);

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

						CNotifyDialogState nf(NOTIFY_DEBUG_VIEW);
						nf.PushInt(DEBUG_RESET);
						nf.PushPipe(PauseIDList);
						nf.PushPipe(Dialog->m_LogicItemTree);
						nf.Notify(Dialog);

					}
					
				}

				ExePipe.Reuse();
				ExePipe.SetID(RETURN_NORMAL);	
				Dialog->SetTaskState(TASK_RUN);

			}
		}
		break;
	case TO_BRAIN_MSG::TASK_CONTROL::CMD_PAUSE:
		{
			ExePipe.SetID(RETURN_BREAK);
		}
		break;
	case TO_BRAIN_MSG::TASK_CONTROL::CMD_DEBUG_STEP:
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
	case TO_BRAIN_MSG::TASK_CONTROL::CMD_STOP:
		{
		    Dialog->SetTaskState(TASK_STOP); //表明任务是被主动停止	
			ExePipe.Break(); //不继续处理,并返回系统
		    return RETURN_DIRECTLY; 
		}
		break;
	case  TO_BRAIN_MSG::TASK_CONTROL::CMD_SET_BREAKPOINT:
		{
			int64 bBreak = Letter.PopInt();
			ePipeline* Path = (ePipeline*)Letter.GetData(0);

			bool ret = Dialog->SetBreakPoint(*Path,bBreak);
			ExePipe.SetID(RETURN_NORMAL);
			return RETURN_DIRECTLY; 
		}
		break;
	default:
		break;
	}
	return CONTINUE_TASK;
}

MsgProcState CElement::OnEltInsertLogic(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	int64 EventID = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter(true);

	int64 ChildID = Letter.PopInt();
	tstring LogicNameOrLogicText = Letter.PopString();


	if(RealtionType()==SHUNT_RELATION){
		ExeError(ExePipe,Format1024(_T("can't insert logic '%s 'in  shunt-body"),LogicNameOrLogicText.c_str())); 
		RETURN_DIRECTLY;
	}

	CLocalLogicCell* lg = Dialog->FindLogic(LogicNameOrLogicText);
	
	CElement* E = NULL;

	if(lg == NULL) {		
		if(LogicNameOrLogicText.size()==0){
			ExeError(ExePipe,_T("no find availabel logic"));
			return RETURN_DIRECTLY;
		}

		TCHAR ch = *LogicNameOrLogicText.rbegin();
		if(ch !=_T(';') || ch !=_T('；')){
			LogicNameOrLogicText+=_T(";");
		}

		CLogicThread  Think;	
		Think.ThinkProc(Dialog,0,LogicNameOrLogicText,false,0);	
		bool ret = Think.CanBeExecute(Dialog);
		if(!ret){
			ExeError(ExePipe,Format1024(_T("the '%s' is not a valid logic name or command"),LogicNameOrLogicText.c_str()));
			return RETURN_DIRECTLY; 
		}
		CText& Text = Think.m_Text;
	
		if (Text.m_SentenceList.size()!=1)
		{
			ExeError(ExePipe,Format1024(_T("the '%s' is more than one sentence"),LogicNameOrLogicText.c_str()));
			return RETURN_DIRECTLY; 
		}

		CSentence* Sentence = Text.m_SentenceList[0];
	
		//序号以原任务计数增长，确保唯一性
		int64 Count = Dialog->GetTask()->m_MassCount;
		CLogicTask Task(Count);
	
		E  = Task.CompileSentence(Dialog,LogicNameOrLogicText,&Sentence->m_AnalyseResult);	
		if(E==NULL){
			ExeError(ExePipe,Format1024(_T("Error: the logic '%s' compile failed"),LogicNameOrLogicText.c_str()));
			return RETURN_DIRECTLY; 
		}

		Dialog->GetTask()->m_MassCount = Task.m_MassCount; //跟踪计数
	}		
	else if(!lg->IsValid()) {
		ExeError(ExePipe,Format1024(_T("Error: Local logic '%s' is not valid"),LogicNameOrLogicText.c_str()));
		return RETURN_DIRECTLY; 
	}else{
		ePipeline LogicPipe(lg->m_Task.m_LogicData);	
		
		//序号以原任务计数增长，确保唯一性
		int64 Count = Dialog->GetTask()->m_MassCount;
		CLogicTask Task(Count);
       
		E  = Task.CompileSentence(Dialog,LogicNameOrLogicText,&LogicPipe);
		if (!E)
		{
			ExeError(ExePipe,Format1024(_T("Compile logic '%s' failed: %s" ),LogicNameOrLogicText.c_str(),Task.m_CompileError.c_str()));
			return RETURN_DIRECTLY; 
		}
		Dialog->GetTask()->m_MassCount = Task.m_MassCount; //跟踪计数
	}


	ActomList::iterator it = E->m_ActomList.begin();
	while(it != E->m_ActomList.end()){
		Mass* m = * it++;
		if (m->MassType() == MASS_ELEMENT)
		{
			CElement* e = (CElement*)m;
			e->m_Parent = this;
		}
	}

	bool bInsert = false;

	it = m_ActomList.begin();
	while(it != m_ActomList.end()){
		Mass* m = *it;
		if(m->m_ID == ChildID){	
			m_ActomList.insert(it,E->m_ActomList.begin(),E->m_ActomList.end());	
			bInsert = true;
			break;
		}
		it++;
	}

	if(!bInsert){
		ExeError(ExePipe,Format1024(_T("the index out of range  when inserting logic '%s'" ),LogicNameOrLogicText.c_str()));
		return RETURN_DIRECTLY; 
	}

	GetBrain()->OutputLog(LOG_MSG_RUNTIME_TIP,Format1024(_T("execute inserting logic:%s"),LogicNameOrLogicText).c_str());
	
	Dialog->UpdateDebugTree();


	//ExePipe.SetID(RETURN_NORMAL);
	return RETURN_DIRECTLY; 
};

MsgProcState CElement::OnEltRemoveLogic(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 EventID = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter(true);
	int64 ChildID = Letter.PopInt();

	assert(ChildIndex==IT_SELF);

	if(RealtionType()==SHUNT_RELATION){
		ExeError(ExePipe,Format1024(_T("can't remove shunt-body's child logic:%I64ld "),ChildID));
		return RETURN_DIRECTLY; 
	}

    ActomList::iterator it = m_ActomList.begin();
	while(it != m_ActomList.end()){
		Mass* m = *it++;
		if(m->m_ID == ChildID){
			m->m_ID = -m->m_ID;
			break;
		}	
	}
  
	Dialog->UpdateDebugTree();

	return RETURN_DIRECTLY; 
}

MsgProcState CElement::OnEltTaskResult(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	int64 EventID = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter(true);
	ePipeline OldExePipe;
	ePipeline* NewExePipe= (ePipeline*)Letter.GetData(0);
	
	
	if (!NewExePipe->IsAlive())
	{
		Dialog->CloseEventDialog(EventID,OldExePipe,ExePipe);
		ExeError(ExePipe,_T("the task has been stoped"));
		return RETURN_DIRECTLY;
	}
	
	int64 retTask = NewExePipe->GetID();
	if (retTask == RETURN_ERROR)
	{
		Dialog->CloseEventDialog(EventID,OldExePipe,ExePipe);
		ExeError(ExePipe,NewExePipe->GetLabel());
		return RETURN_DIRECTLY;
	}
	
	ExePipe.Clear();
	ExePipe << OldExePipe;
	

	Dialog->CloseEventDialog(EventID,OldExePipe,ExePipe);

	return RETURN_DIRECTLY;

}