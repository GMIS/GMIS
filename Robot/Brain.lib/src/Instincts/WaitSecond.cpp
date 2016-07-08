#include "WaitSecond.h"
#include "..\Brain.h"
#include "..\LogicDialog.h"
MsgProcState CWaitSecond_Static::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{

	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_EVENT_TICK)
	{	
		if(!ExePipe.IsAlive()){
			CBrainEvent EventInfo;
			bool ret = Dialog->m_Brain->GetBrainData()->GetEvent(GetEventID(),EventInfo,true);
			if (!ret) 
			{
				ExePipe.GetLabel() = Format1024(_T("Error: (%I64ld)%s event lost"),m_ID,GetName().c_str());
				ExePipe.SetID(RETURN_ERROR);
				return RETURN_DIRECTLY;
			}

			ePipeline& OldExePipe = EventInfo.m_ClientExePipe;

			Dialog->CloseChildDialog(GetEventID(),OldExePipe,ExePipe);

			return CONTINUE_TASK;
		}
		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID()) //属于调试暂停放来的MSG_EVENT_TICK,做缺省处理 
		{
			ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			return ret;
		}


		ePipeline& Letter = Msg.GetLetter();
		int64 TimeStamp   = Letter.PopInt();
		float64 t = (float64)(TimeStamp-m_StartTimeStamp);

		t /=(float64)(TIME_SEC);

		if(t<m_Second){
			Dialog->m_Brain->GetBrainData()->ResetEventTickCount(EventID);
			if (ExePipe.GetID() == RETURN_BREAK)
			{
				m_bPause = true; //等待期间的收到暂停信号不处理，直到等待时间结束
			}

			ExePipe.SetID(RETURN_WAIT); 
		}else{
			tstring tip = Format1024(_T("waited %.3f second EventID:%I64ld"),t,EventID);

			Dialog->RuntimeOutput(m_ID,tip);
			
			CBrainEvent EventInfo;
			bool ret = Dialog->m_Brain->GetBrainData()->GetEvent(EventID,EventInfo,true);
			if (!ret) 
			{
				ExePipe.GetLabel() = Format1024(_T("Error: (%I64ld)%s event lost"),m_ID,GetName().c_str());
				ExePipe.SetID(RETURN_ERROR);
				return RETURN_DIRECTLY;
			}
			ePipeline& OldExePipe = EventInfo.m_ClientExePipe;

			ExePipe.Clear();
			ExePipe<<OldExePipe;

			if(m_bPause){
				ExePipe.SetID(RETURN_BREAK);
				m_bPause = false;
			}

			Dialog->CloseChildDialog(EventID,OldExePipe,ExePipe);
		}
	}else{

		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;

	}
	return RETURN_DIRECTLY;
}

bool CWaitSecond_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	m_StartTimeStamp = AbstractSpace::CreateTimeStamp();
	UpdateEventID();
	tstring DialogTitle = Format1024(_T("Wait %.1f second"),m_Second);
	Dialog->StartChildDialog(GetEventID(),DialogTitle,_T("Please wait timer to end"),TASK_OUT_DEFAULT,ExePipe,LocalAddress,MIN_EVENT_INTERVAL,false,false);

	return true;
}


bool CWaitSecond::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	m_StartTimeStamp = AbstractSpace::CreateTimeStamp();

	if (ExePipe.Size()==0)
	{
		ExePipe.GetLabel() = Format1024(_T("Error: Wait second lost param"));
		return false;
	}
	eElectron E;
	ExePipe.Pop(&E);

	if (E.EnergyType() == TYPE_INT)
	{
		m_Second = E.Int64();
	}else if(E.EnergyType() == TYPE_FLOAT){
		m_Second = E.Float64();
	}else{
		ExePipe.GetLabel() = Format1024(_T("Error: Wait second  param must be int or float"));
		return false;
	}

	UpdateEventID();
	tstring DialogTitle = Format1024(_T("Wait %.1f second"),m_Second);
	Dialog->StartChildDialog(GetEventID(),DialogTitle,_T("Please wait timer ends"),TASK_OUT_DEFAULT,ExePipe,LocalAddress,MIN_EVENT_INTERVAL,false,false);

	return true;
}