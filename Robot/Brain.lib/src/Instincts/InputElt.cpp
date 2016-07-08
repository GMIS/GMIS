#include "InputElt.h"
#include "..\LogicDialog.h"

CInputElement::CInputElement(int64 ID,tstring Tip,bool bInputNum)
	:CElement(ID,Tip.c_str()),m_bInputNum(bInputNum)
{
}

CInputElement::~CInputElement(){

};

MsgProcState CInputElement::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	int64 MsgID = Msg.GetMsgID();
	if( MsgID == MSG_TASK_RESULT){

		ePipeline& Letter = Msg.GetLetter();
		ePipeline* OldExePipe= (ePipeline*)Letter.GetData(0);
		ePipeline* NewExePipe= (ePipeline*)Letter.GetData(1);

		if (!NewExePipe->IsAlive())
		{	
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			return RETURN_DIRECTLY; //不在继续执行TaskProc()
		}

		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);
			ExePipe.Break();
			ExePipe.SetID(retTask);
			ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
			return RETURN_DIRECTLY;
		}

		int64 ThinkID = NewExePipe->PopInt();
		CLogicThread* Think = Dialog->m_Brain->GetBrainData()->GetLogicThread(ThinkID);
		assert(Think);
		tstring Text = Think->GetUserInput();
		Text = TriToken(Text);

		Dialog->CloseChildDialog(GetEventID(),*OldExePipe,ExePipe);	

		if (m_bInputNum)
		{
			NumType Type = IsNum(Text);
			if (Type == NOT_NUM)
			{
				//再次压入事件，要求用户重新输入
				UpdateEventID();
				tstring DialogText = _T("Input error: not a num,Please input again.");
				bool ret = Dialog->StartChildDialog(GetEventID(),_T("Input Num"),DialogText,TASK_OUT_THINK,*OldExePipe,LocalAddress,TIME_SEC,true,true);				
				if (!ret)
				{
					tstring Answer = _T("Start child dialog fail.");
					ExePipe.SetLabel(Answer.c_str());
					ExePipe.SetID(RETURN_ERROR);
					return CONTINUE_TASK;
				}

				ExePipe.SetID(OldExePipe->GetID());
			}else if(Type == INT_NUM){
				ExePipe.Clear();
				ExePipe<<*OldExePipe;
				int64 i = _ttoi64(Text.c_str());
				ExePipe.PushInt(i);
			}else {
				ExePipe.Clear();
				ExePipe<<*OldExePipe;
				float64 f = _tcstod(Text.c_str(),NULL);
				ExePipe.PushFloat(f);
			}
		}else{
			ExePipe.Clear();
			ExePipe<<*OldExePipe;
			ExePipe.PushString(Text);
		}

	}else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
	}	
	return RETURN_DIRECTLY;
}
bool CInputElement::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	UpdateEventID();
	bool ret = Dialog->StartChildDialog(GetEventID(),_T("Input Dialog"),m_Name,TASK_OUT_THINK,ExePipe,LocalAddress,TIME_SEC,true,true);
	return ret;
}
