#include "CloseObjectElt.h"
#include "..\LogicDialog.h"

CCloseObject::CCloseObject(int64 ID)
	:CElement(ID,_T("CloseObject"))
{
}

CCloseObject::~CCloseObject(){

}

bool  CCloseObject::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){ 

	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);

	if(!Msg.IsReaded()){

		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_TASK_RESULT)
		{	
			int64 EventID = Msg.GetEventID();
			if (EventID != GetEventID())
			{
				return false;
			}

			ePipeline& Letter = Msg.GetLetter();
			ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);


			if (!NewExePipe->IsAlive())
			{
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}

			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}

			Dialog->m_NamedObjectList.UnregisterNameByTask(m_ObjectName);

			ExePipe.Clear();
			ExePipe << *NewExePipe;

			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			return true;
		}else{
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			if(ret == RETURN_DIRECTLY){
				return true;
			}
		}	
	}

	ePipeline* ObjectInstance = Dialog->FindObjectInstance(Dialog->m_ObjectFocus);
	if (ObjectInstance==NULL)
	{
		return false;
	}

	assert(ObjectInstance->GetID() ==Dialog->m_ObjectFocus );

	tstring SrcObjectName = *(tstring*)ObjectInstance->GetData(0);

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_OBJECT);
	nf.PushInt(CLOSE_INSTANCE);
	nf.PushString(SrcObjectName);
	nf.PushInt(Dialog->m_ObjectFocus);
	nf.Notify(Dialog);


	//如果没有启动外部器官，则启动
	CLinker Linker;
	Dialog->m_Brain->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		//ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("Waring:Space disconnected."));			
		return true;
	}
	
	//准备好一个请求信息
	UpdateEventID();
	ePipeline rq(REQUEST_CLOSE_OBJECT);

	int64 DllType = *(int64*)ObjectInstance->GetData(4);
	rq.PushInt(DllType);
	rq.PushInt(ObjectInstance->GetID());
	rq.Push_Directly(ExePipe.Clone());

	Dialog->CloseObjectInstance(Dialog->m_ObjectFocus);

	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = _T("Waiting for start external object");
	tstring Title = Format1024(_T("CloseObject(%I64ld)"),m_ID);
	Dialog->StartChildDialog(GetEventID(),Title,DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,false,false);


	//把请求发给对方
	CMsg NewMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_TASK_REQUEST,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter();
	Letter.Push_Directly(rq.Clone());

	Linker().PushMsgToSend(NewMsg);
	return true;
};
