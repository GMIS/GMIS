#include "GetObjectDocElt.h"
#include "..\LogicDialog.h"


CGetObjectDoc::CGetObjectDoc(int64 ID)
	:CElement(ID,_T("UseObjectDoc")){

};

CGetObjectDoc::~CGetObjectDoc(){

}

MsgProcState CGetObjectDoc::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
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
			Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.Break();
			return RETURN_DIRECTLY;
		}

		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.SetID(retTask);
			ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
			return RETURN_DIRECTLY;
		}

		if (NewExePipe->Size()>0 && NewExePipe->GetDataType(0)== TYPE_STRING)
		{
			tstring Doc = NewExePipe->PopString();
			Dialog->RuntimeOutput(0,Doc);
		}

		Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);

		return RETURN_DIRECTLY; // 不在继续执行TaskProc
	}else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
	}

	return RETURN_DIRECTLY;
}


bool  CGetObjectDoc::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){


	ePipeline* ObjectInstance = Dialog->FindObjectInstance(Dialog->m_ObjectFocus);
	if (ObjectInstance==NULL)
	{
		return false;
	}

	assert(ObjectInstance->GetID() ==Dialog->m_ObjectFocus );


	//如果没有启动外部器官，则启动
	CLinker Linker;
	Dialog->m_Brain->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("Space disconnected."));			
		return true;
	};

	//准备好一个请求信息
	ePipeline rq(REQUEST_GET_OBJECT_DOC);
	int64 DllType = *(int64*)ObjectInstance->GetData(4);
	rq.PushInt(DllType);
	rq.PushInt(	Dialog->m_ObjectFocus);
	rq.Push_Directly(ExePipe.Clone());

	//这个子对话主要起提示作用，实际任务会交给远方执行
	UpdateEventID();
	tstring DialogText = Format1024(_T("Waiting for run external object:%I64ld  EventID:%I64ld"),Dialog->m_ObjectFocus,GetEventID());
	tstring Title = Format1024(_T("UseObject(%I64ld)"),m_ID);
	Dialog->StartEventDialog(GetEventID(),Title,DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,false,false,true);


	//把请求发给对方
	CMsg NewMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_TASK_REQUEST,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter();
	Letter.Push_Directly(rq.Clone());

	Linker().PushMsgToSend(NewMsg);


	return true;
};
