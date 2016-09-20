#pragma warning (disable:4786)

#include "Brain.h"
#include "LogicDialog.h"
#include "GUIMsgDefine.h"
bool CBrain::ProcessRequest(CLogicDialog* Dialog,ePipeline* RequestInfo){

	int64 RequestID = RequestInfo->GetID();
	
//	switch (RequestID)
//	{
//
////		return OnRequestTransTask(Dialog,RequestInfo);
//	/*case REQUEST_EXE_LOGIC:
//		return OnRequestExeLogic(Dialog,RequestInfo);
//	case REQUEST_INSERT_LOGIC:
//	return OnRequestInsertLogic(Dialog,RequestInfo);
//	case REQUEST_REMOVE_LOGIC:
//	return OnRequestRemoveLogic(Dialog,RequestInfo);*/
//	default:
//		assert(0);
//	}
	return false;
}



bool CBrain::OnRequestInsertLogic(CLogicDialog* Dialog,ePipeline* RequestInfo)
{
	int64 EventID = RequestInfo->PopInt();

	tstring InsertLogicName = RequestInfo->PopString();
	ePipeline* InsertAddress = (ePipeline*)RequestInfo->GetData(0); 
	
	CMsg EltMsg(Dialog->m_SourceID,*InsertAddress,MSG_ELT_INSERT_LOGIC,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = EltMsg.GetLetter(false);
	Letter.PushString(InsertLogicName);
	
	//直接把信息压入任务队列，并且优先处理
	Dialog->m_ElementMsgList.PushUrgence(EltMsg.Release());

	return true;
}

bool CBrain::OnRequestRemoveLogic(CLogicDialog* Dialog,ePipeline* RequestInfo)
{
	int64 EventID = RequestInfo->PopInt();
	
	ePipeline* RemoveAddress = (ePipeline*)RequestInfo->GetData(0); 
    int64 ChildID  =   	*(int64*)RequestInfo->GetData(1); 

	CMsg EltMsg(Dialog->m_SourceID,*RemoveAddress,MSG_ELT_REMOVE_LOGIC,DEFAULT_DIALOG,EventID);
	ePipeline& Letter = EltMsg.GetLetter(false);
	Letter.PushInt(ChildID);
	
	//直接把信息压入任务队列，并且优先处理
	Dialog->m_ElementMsgList.PushUrgence(EltMsg.Release());

	return true;
}