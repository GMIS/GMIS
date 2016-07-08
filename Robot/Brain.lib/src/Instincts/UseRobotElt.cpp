#include "UseRobotElt.h"
#include "..\Brain.h"
#include "..\LogicDialog.h"

CUseRobot::CUseRobot(int64 ID,CLogicDialog* Dialog, int64 RobotID,ePipeline& LogicPipe)
	:CElement(ID,_T("UseObject")),m_Dialog(Dialog),m_RobotID(RobotID)
{

	m_TaskLogic<<LogicPipe;
	m_TaskLogic.SetLabel(LogicPipe.GetLabel().c_str());

};

CUseRobot::~CUseRobot(){

};

TypeAB CUseRobot::GetTypeAB(){ 
	return 0;
};

tstring CUseRobot::GetName(){ 
	return m_TaskLogic.GetLabel();
};


MsgProcState CUseRobot::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_TASK_RESULT)
	{		
		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID())
		{
			return RETURN_DIRECTLY;
		}

		ePipeline& Letter = Msg.GetLetter();
		ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
		ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);

		if (!NewExePipe->IsAlive())
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.Break();
			return CONTINUE_TASK;
		}

		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.Break();
			ExePipe.SetID(retTask);
			return CONTINUE_TASK;
		}


		ExePipe.Clear();
		ExePipe << *NewExePipe;
		Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);

	}else{
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
	}
	return RETURN_DIRECTLY;
}

bool CUseRobot::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	//return m_Dialog->m_Brain->RunExternalRobot(m_Dialog,m_EventID,m_RobotID,m_TaskLogic,ExePipe,LocalAddress);	

	//如果没有启动外部器官，则启动
	CLinker Linker;
	m_Dialog->m_Brain->GetLinker(m_RobotID,Linker);//RobotID = SourceID
	if (!Linker.IsValid())
	{		
		ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("The robot disconnect."));			
		return true;
	}
	tstring& LogicText = m_TaskLogic.GetLabel();

	//发执行信息

	ePipeline rq(REQUEST_EXE_LOGIC);
	rq.PushInt(GetEventID());
	rq.PushString(LogicText);
	rq.Push_Directly(ExePipe.Clone());

	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = _T("Waiting for run external robot");
	UpdateEventID();
	Dialog->StartChildDialog(GetEventID(),_T("UseObject"),DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,true,false);

	CMsg NewMsg(Dialog->m_SourceID,DEFAULT_DIALOG,MSG_TASK_REQUEST,DEFAULT_DIALOG,0);
	ePipeline& Letter = NewMsg.GetLetter();
	Letter.Push_Directly(rq.Clone());

	Linker().PushMsgToSend(NewMsg);

	return true;
}
