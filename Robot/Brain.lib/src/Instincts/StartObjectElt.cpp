#include "StartObjectElt.h"
#include "..\LogicDialog.h"

CStartObject::CStartObject(int64 ID,ePipeline* Obj)
	:CElement(ID,_T("StartObject")),m_Obj(Obj)
{

};

CStartObject::~CStartObject(){

}


bool  CStartObject::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){ 

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
				Dialog->CloseEventDialog(GetEventID(),*OldExePipe,ExePipe);
				ExePipe.Break();
				return true;
			}

			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{

				Dialog->CloseEventDialog(EventID,*OldExePipe,ExePipe);
				ExePipe.SetID(retTask);
				ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
				return true;
			}
			int64 DllType = NewExePipe->PopInt();
			int64 InstanceID = NewExePipe->PopInt();

			assert(m_Obj->Size()==5);
			m_Obj->SetID(InstanceID);
			*(int64*)m_Obj->GetData(4) = DllType;

			CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
			nf.PushInt(INSTANCE_OBJECT);
			nf.PushInt(CREATE_INSTANCE);
			nf.Push_Directly(m_Obj->Clone());
			nf.Notify(Dialog);


			ExePipe.Clear();
			ExePipe << *NewExePipe;


			Dialog->CloseEventDialog(EventID,*OldExePipe,ExePipe);

			Dialog->AddObjectInstance(InstanceID,*m_Obj);
			Dialog->m_ObjectFocus = InstanceID;

			return true;
		}else{
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			if(ret == RETURN_DIRECTLY){
				return true;
			}
		}	
	}

	//如果没有启动外部器官，则启动
	CLinker Linker;
	Dialog->m_Brain->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		ExePipe.SetID(RETURN_ERROR);
		ExePipe.GetLabel() = Format1024(_T("Space disconnected."));			
		return true;
	}
	assert(ExePipe.IsAlive());

	//准备好一个请求信息
	UpdateEventID();
	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = _T("Waiting for starting external object");
	tstring Title = Format1024(_T("StartObject(%I64ld)"),m_ID);
	Dialog->StartEventDialog(GetEventID(),Title,DialogText,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,false,false,true);


	//把请求发给对方
	Reset();
	ePipeline rq(REQUEST_START_OBJECT);
	rq.Push_Directly(m_Obj->Clone());
	rq.Push_Directly(ExePipe.Clone());

	CMsg NewMsg(Dialog->m_SourceID,DEFAULT_DIALOG,MSG_TASK_REQUEST,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter();
	Letter.Push_Directly(rq.Clone());

	Linker().PushMsgToSend(NewMsg);
	return true;
};
