#include "LogicElt.h"
#include "..\LogicDialog.h"

bool  CFocusLogic::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{

	tstring LogicName = ExePipe.PopString();
	CElement* E = FindFocusLogic(LogicName);

	if (E==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find focus logic(%s)"),LogicName.c_str());
		return false;
	}
	Dialog->m_LogicFocus = LogicName;


	return true;
}

bool  CFocusLogic_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{

	CElement* E = FindFocusLogic(m_FocusName);

	if (E==NULL)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: not find focus logic(%s)"),m_FocusName.c_str());
		return false;
	}
	Dialog->m_LogicFocus = m_FocusName;


	return true;
}

MsgProcState CInsertLogicStatic::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_TASK_RESULT)
	{	

		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID())
		{
			return RETURN_DIRECTLY;
		}

		CBrainEvent EventInfo;
		bool ret = Dialog->m_Brain->GetBrainData()->GetEvent(EventID,EventInfo,true);
		if (!ret) 
		{
			ExePipe.GetLabel() = Format1024(_T("Error: (%I64ld)%s event lost"),m_ID,GetName().c_str());
			ExePipe.SetID(RETURN_ERROR);
			return RETURN_DIRECTLY;
		}

		ePipeline& Letter = Msg.GetLetter();
		ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
		ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);


		if (!NewExePipe->IsAlive())
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);

			tstring Answer = _T("收到的数据管道已经无效");
			ExePipe.SetLabel(Answer.c_str());
			ExePipe.SetID(RETURN_ERROR);
			return RETURN_DIRECTLY;
		}

		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);	

			ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
			ExePipe.SetID(RETURN_ERROR);
			return RETURN_DIRECTLY;
		}


		ExePipe.Clear();
		ExePipe << *OldExePipe;

		Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
		return RETURN_DIRECTLY;
	}else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
	}
	return RETURN_DIRECTLY;
};

bool CInsertLogicStatic::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	CElement* LogicInstance = FindFocusLogic(Dialog->m_LogicFocus);
	if (LogicInstance==NULL)
	{
		return false;
	}

	ePipeline InsertAddress;
	LogicInstance->GetAddress(InsertAddress);


	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = Format1024(_T("Waiting for insert logic %s"),m_InsertLogicName.c_str());
	UpdateEventID();
	Dialog->StartChildDialog(GetEventID(),_T("Insert Logic"),DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,false,false);

	int64 EventID = GetEventID();

	CMsg EltMsg(Dialog->m_SourceID,InsertAddress,MSG_ELT_INSERT_LOGIC,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushString(m_InsertLogicName);

	//直接把信息压入任务队列，并且优先处理
	Dialog->PushEltMsg(EltMsg,true);

	return true;
};


bool CRemoveLogicStatic::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	CElement* LogicInstance = FindFocusLogic(Dialog->m_LogicFocus);
	if (LogicInstance==NULL)
	{
		return false;
	}

	ePipeline RemoveAddress;
	LogicInstance->GetAddress(RemoveAddress);

	eINT* e = new eINT(Dialog->m_DialogID);
	RemoveAddress.Push_Front(e);

	assert (RemoveAddress.Size() < LocalAddress.Size());

	int i;
	for (i=0; i<RemoveAddress.Size();i++)
	{
		int64 ID1 = *(int64*)RemoveAddress.GetData(i);
		int64 ID2 = *(int64*)LocalAddress.GetData(i);
		if (ID1 != ID2)
		{
			return false;
		}
	}

	int64 ChildID = *(int64*)LocalAddress.GetData(i);

	//准备好一个请求信息
	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = Format1024(_T("Waiting for remove logic"));
	UpdateEventID();
	Dialog->StartChildDialog(GetEventID(),_T("Remove Logic"),DialogText,TASK_OUT_DEFAULT,ExePipe,RemoveAddress,TIME_SEC,false,false);

	RemoveAddress.PopInt();  //去掉dilaog ID
	int64 EventID = GetEventID();

	CMsg EltMsg(Dialog->m_SourceID,RemoveAddress,MSG_ELT_REMOVE_LOGIC,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = EltMsg.GetLetter();
	Letter.PushInt(ChildID);

	//直接把信息压入任务队列，并且优先处理
	Dialog->PushEltMsg(EltMsg,true);

	return true;
}

bool CSetLogicAddress::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	if (ExePipe.Size()==0)
	{
		Dialog->m_LogicAddress.SetID(0);
		Dialog->m_LogicAddress.Clear();
		return true;
	}
	ENERGY_TYPE Type = ExePipe.GetEnergy(0)->EnergyType();
	if(Type != TYPE_INT){
		ExePipe.GetLabel() = Format1024(_T("Error: Memory address invalid"));
		return false;
	}
	
	int64 ID = ExePipe.PopInt();

	CLogicTask* Task = Dialog->GetTask();

	ePipeline LogicAddress(ID);
	bool ret = Task->FindLogicAddress(ID,LogicAddress);

	if (!ret)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: no find th logic address of item %I64ld"),ID);
		return false;
	}

	Dialog->m_LogicAddress = LogicAddress;
	return true;
}

bool CSetLogicBreakpoint::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	return true;
}