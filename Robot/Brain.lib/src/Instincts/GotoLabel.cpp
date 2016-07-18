#include "GotoLabel.h"
#include "..\LogicDialog.h"

MsgProcState CGotoLabel::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_EVENT_TICK)
	{	

		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID()) //调试暂停后也会持续收到此信息
		{
			ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			return ret;
		}

		ePipeline& Letter = Msg.GetLetter(); //注意：虽然用不到，但必须使用一次，表明此信读过，避免随后被使用

		CBrainEvent ObjectInfo;
		bool ret = Dialog->m_Brain->GetBrainData()->GetEvent(EventID,ObjectInfo,true);
		if (!ret) 
		{
			ExePipe.GetLabel() = Format1024(_T("Error: got label '%s' lost event"),m_GotoLabel.c_str());
			ExePipe.SetID(RETURN_ERROR);
			return RETURN_DIRECTLY;
		}


		ExePipe.Clear();
		ExePipe<<ObjectInfo.m_ClientExePipe;;

		int64 ID = ObjectInfo.m_ClientExePipe.GetID();
		if (ID == RETURN_BREAK || ExePipe.GetID()==RETURN_BREAK)
		{
			ExePipe.SetID(RETURN_DEBUG_GOTO_LABEL);
		} 
		else
		{
			ExePipe.SetID(RETURN_GOTO_LABEL);
			Dialog->SetTaskState(TASK_RUN);
		}

		ExePipe.SetLabel(m_GotoLabel.c_str());



	}else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;

	}
	return RETURN_DIRECTLY;

}

bool CGotoLabel::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	UpdateEventID();
	int64 EventID = GetEventID();

	Dialog->m_Brain->GetBrainData()->CreateEvent(EventID,EventID,ExePipe,LocalAddress,MIN_EVENT_INTERVAL,false,INSTINCT_GOTO_LABEL,false); 

	int64 ID = ExePipe.GetID();
	if (ID == RETURN_BREAK)
	{
		ExePipe.SetID(RETURN_DEBUG_WAIT);
	}else {
		ExePipe.SetID(RETURN_WAIT);
	}	
	return true;	
}
