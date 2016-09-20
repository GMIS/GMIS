#include "..\Brain.h"
#include "..\LogicDialog.h"
#include "TimeOperator.h"



CGetTime::CGetTime(int64 ID)
	:Mass(ID){

};
CGetTime::~CGetTime(){};

tstring CGetTime::GetName(){
	return _T("Get Time");
};

TypeAB  CGetTime::GetTypeAB(){ 
	return 0x00000000;
}

bool CGetTime::Do(Energy* E){
	int64 CurentTime = AbstractSpace::CreateTimeStamp();
	uint32  Year,Month,Day, Hour,Minute,Second,Milli,Micro,Nanos;

	AbstractSpace::GetTimer()->Int64ToTime(CurentTime,Year,Month,Day,Hour,Minute,Second,Milli,Micro,Nanos);
	ePipeline* Pipe = (ePipeline*)E;
	Pipe->PushInt(Year);
	Pipe->PushInt(Month);
	Pipe->PushInt(Day);
	Pipe->PushInt(Hour);
	Pipe->PushInt(Minute);
	Pipe->PushInt(Second);
	Pipe->PushInt(Milli);
	Pipe->PushInt(Micro);
	Pipe->PushInt(Nanos);

	return true;
};

//////////////////////////////////////////////////////////////////////////

CPrintPipeMass::CPrintPipeMass(int64 ID):Mass(ID){

};
CPrintPipeMass::~CPrintPipeMass(){

};

tstring  CPrintPipeMass::GetName(){ 
	return _T("Print pipe");
};
TypeAB  CPrintPipeMass::GetTypeAB(){
	return 0x00000000;
} 
bool CPrintPipeMass::Do(Energy* E){
	
	ePipeline* Pipe = (ePipeline*)E;
	
	tstring s;
	while (Pipe->Size())
	{
		eElectron e;
		Pipe->Pop(&e);
		switch(e.EnergyType()){
		case TYPE_NULL:
			{
				s+=_T("[null]");
			}
			break;
		case TYPE_INT: 
			{
				int64 n = e.Int64();
				s+= Format1024(_T("%I64ld"),n);
			}
			break;
		case TYPE_FLOAT:
			{
				float64 f=e.Float64();
				s += Format1024(_T("%.6f"),f);
			}
			break;
		case TYPE_STRING:
			{
				s += e.String();
			 }
			break;
		case TYPE_PIPELINE:
			{
				s+=_T("[pipe]");
			}
			break;
		case TYPE_BLOB:
			{
				s+=_T("[blob]");
			}
			break;
		default:
			assert(0);
			break;

		}

	}
	Pipe->PushString(s);
	return true;
};


//////////////////////////////////////////////////////////////////////////

MsgProcState CWaitTime_Static::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{

	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_EVENT_TICK)
	{	
		if(!ExePipe.IsAlive()){	
			ePipeline OldExePipe;
			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);

			return RETURN_DIRECTLY;
		}
		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID()) //属于调试暂停放来的MSG_EVENT_TICK,做缺省处理 
		{
			ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			return ret;
		}


		ePipeline& Letter = Msg.GetLetter(true);
		int64 TimeStamp   = Letter.PopInt();
		float64 t = (float64)(TimeStamp-m_StartTimeStamp);

		t /=(float64)(TIME_SEC);

		if(t<m_Second){
			GetBrain()->GetBrainData()->ResetEventTickCount(EventID);
			if (ExePipe.GetID() == RETURN_BREAK)
			{
				m_bPause = true; //等待期间的收到暂停信号不处理，直到等待时间结束
			}

			ExePipe.SetID(RETURN_WAIT); 
		}else{
			tstring tip = Format1024(_T("waited %.3f second EventID:%I64ld"),t,EventID);

			Dialog->RuntimeOutput(m_ID,tip);
			
			ePipeline OldExePipe;
			Dialog->CloseEventDialog(EventID,OldExePipe,ExePipe);

			ExePipe.Clear();
			ExePipe<<OldExePipe;

			if(m_bPause){
				ExePipe.SetID(RETURN_BREAK);
				m_bPause = false;
			}

		}
	}else{

		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;

	}
	return RETURN_DIRECTLY;
}

bool CWaitTime_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	m_StartTimeStamp = AbstractSpace::CreateTimeStamp();
	UpdateEventID();
	tstring DialogTitle = Format1024(_T("Wait %.1f second"),m_Second);
	CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),DialogTitle,TASK_OUT_DEFAULT,ExePipe,LocalAddress,MIN_EVENT_INTERVAL,false,false,true);
	if(!Dlg){
		ExeError(ExePipe,_T("create event dialog fail"));
		return false;
	}
	Dlg->SaveDialogItem(_T("Please waiting for timer to end"),Dialog->m_DialogName);
	return true;
}


bool CWaitTime::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
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

	CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),DialogTitle,TASK_OUT_DEFAULT,ExePipe,LocalAddress,MIN_EVENT_INTERVAL,false,false,true);
	if(!Dlg){
		ExeError(ExePipe,_T("create event dialog fail"));
		return false;
	}
	Dlg->SaveDialogItem(_T("Please waiting for timer to end"),Dialog->m_DialogName);
	return true;
}