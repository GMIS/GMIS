#include "..\Brain.h"
#include "..\LogicDialog.h"
#include "InputElt.h"

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

		ePipeline& Letter = Msg.GetLetter(true);
		
		ePipeline OldExePipe;
		ePipeline* NewExePipe = (ePipeline*)Letter.GetData(0);

		if (!NewExePipe->IsAlive())
		{	
			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
			return RETURN_DIRECTLY; //不在继续执行TaskProc()
		}

		int64 retTask = NewExePipe->GetID();
		if (retTask == RETURN_ERROR)
		{
			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
			ExePipe.Break();
			ExePipe.SetID(retTask);
			ExePipe.SetLabel(NewExePipe->GetLabel().c_str());
			return RETURN_DIRECTLY;
		}

		int64 ThinkID = NewExePipe->PopInt();
		CLogicThread* Think = GetBrain()->GetBrainData()->GetLogicThread(ThinkID);
		assert(Think);
		tstring Text = Think->GetUserInput();
		Text = TriToken(Text);

		Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);	

		if (m_bInputNum)
		{
			NumType Type = IsNum(Text);
			if (Type == NOT_NUM)
			{
				//再次压入事件，要求用户重新输入
				UpdateEventID();
				tstring DialogText = _T("Input error: not a num,Please input again.");
				CLogicDialog*  Dlg = Dialog->StartEventDialog(GetEventID(),_T("Input Num"),TASK_OUT_THINK,OldExePipe,LocalAddress,TIME_SEC,true,true,true,m_bInputNum?INSTINCT_INPUT_NUM:INSTINCT_INPUT_TEXT);				
				if (!Dlg)
				{
					ExeError(ExePipe,_T("Start child dialog fail."));
					return CONTINUE_TASK;
				}
				Dlg->SaveDialogItem(DialogText,Dialog->m_DialogName,0);
				ExePipe.SetID(OldExePipe.GetID());
			}else if(Type == INT_NUM){
				ExePipe.Clear();
				ExePipe<<OldExePipe;
				int64 i = _ttoi64(Text.c_str());
				ExePipe.PushInt(i);
			}else {
				ExePipe.Clear();
				ExePipe<<OldExePipe;
				float64 f = _tcstod(Text.c_str(),NULL);
				ExePipe.PushFloat(f);
			}
		}else{
			ExePipe.Clear();
			ExePipe<<OldExePipe;
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
	CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),_T("Input Dialog"),TASK_OUT_THINK,
		ExePipe,LocalAddress,TIME_SEC,true,true,true,m_bInputNum?INSTINCT_INPUT_NUM:INSTINCT_INPUT_TEXT);

	if(!Dlg){
		ExeError(ExePipe,_T("create event dialog fail"));
		return false;
	}
	Dlg->SaveDialogItem(m_Name,Dialog->m_DialogName);
	return true;
}
