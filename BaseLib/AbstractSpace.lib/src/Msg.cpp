// Msg.cpp: implementation of the CMsg class.
//
//////////////////////////////////////////////////////////////////////

#include "Msg.h"
#include "Pipeline.h"

namespace ABSTRACT{

ePipeline* CreateMsg(int64 MsgID,int64 EventID,ePipeline** ReceiverInfo,ePipeline** Letter,ePipeline** SenderInfo)
{
	ePipeline* MsgPtr = new ePipeline();
	if(!MsgPtr)return NULL;
	
	eElectron e(MsgPtr); //AUTO PTR

	ePipeline* ReceivePipe  = new ePipeline(EventID);
	if (!ReceivePipe)
	{
		return NULL;
	}else{
		*ReceiverInfo = ReceivePipe;
		MsgPtr->Push_Directly(ReceivePipe);
	}
	
	ePipeline* LetterPipe = new ePipeline(MsgID);
	if (!LetterPipe)
	{
		return NULL;
	}else{
		*Letter = LetterPipe;
		MsgPtr->Push_Directly(LetterPipe);
	}
	
	//发信时间戳由系统自动填充
	int64 SendTimeStamp = AbstractSpace::CreateTimeStamp();
	ePipeline* SenderPipe  = new ePipeline(SendTimeStamp);
	if (!SenderPipe)
	{
		return NULL;
	}else{
		*SenderInfo = SenderPipe;
		MsgPtr->Push_Directly(SenderPipe);	
	}
	
	return (ePipeline*)e.Release();
}

CMsg::CMsg()
	:m_MsgPtr(NULL),m_bReaded(false)
{

}
CMsg::CMsg(int64 SourceID,ePipeline& Receiver,int64 MsgID,ePipeline& Sender,int64 EventID){
	m_bReaded = false;
	ePipeline* ReceiverInfo = NULL;
	ePipeline* Letter = NULL;
	ePipeline* SenderInfo = NULL;
	m_MsgPtr = CreateMsg(MsgID,EventID,&ReceiverInfo,&Letter,&SenderInfo);
	if (m_MsgPtr)
	{
		*ReceiverInfo=Receiver;
		ReceiverInfo->SetID(EventID);
		*SenderInfo = Sender;
		m_MsgPtr->m_ID = SourceID;
	}

}
CMsg::CMsg(int64 SourceID,int64 ReceiverID,int64 MsgID,int64 SenderID,int64 EventID){
	m_bReaded = false;
	ePipeline* ReceiverInfo = NULL;
	ePipeline* Letter = NULL;
	ePipeline* SenderInfo = NULL;
	m_MsgPtr = CreateMsg(MsgID,EventID,&ReceiverInfo,&Letter,&SenderInfo);
	if (m_MsgPtr)
	{
		ReceiverInfo->PushInt(ReceiverID);
		SenderInfo->PushInt(SenderID);
		m_MsgPtr->m_ID = SourceID;
	}
}
CMsg::CMsg(int64 SourceID,ePipeline& Receiver,int64 MsgID,int64 SenderID,int64 EventID){

	m_bReaded = false;
	ePipeline* ReceiverInfo = NULL;
	ePipeline* Letter = NULL;
	ePipeline* SenderInfo = NULL;
	m_MsgPtr = CreateMsg(MsgID,EventID,&ReceiverInfo,&Letter,&SenderInfo);
	if (m_MsgPtr)
	{
		*ReceiverInfo=Receiver;
		ReceiverInfo->SetID(EventID);
		SenderInfo->PushInt(SenderID);
		m_MsgPtr->m_ID = SourceID;
	}
}

//CMsg::CMsg(int64 MsgID,int64 SenderID,int64 EventID){
//	m_bReaded = false;
//	ePipeline* ReceiverInfo = NULL;
//	ePipeline* Letter = NULL;
//	ePipeline* SenderInfo = NULL;
//	m_MsgPtr = CreateMsg(MsgID,EventID,&ReceiverInfo,&Letter,&SenderInfo);
//    if (m_MsgPtr)
//    {
//		ReceiverInfo->PushInt(DEFAULT_DIALOG);
//		SenderInfo->PushInt(SenderID);
//		
//    }
//}
//
//CMsg::CMsg(int64 ReceiverID, int64 MsgID,int64 SenderID,int64 EventID){
//	m_bReaded = false;
//	ePipeline* ReceiverInfo = NULL;
//	ePipeline* Letter = NULL;
//	ePipeline* SenderInfo = NULL;
//	m_MsgPtr = CreateMsg(MsgID,EventID,&ReceiverInfo,&Letter,&SenderInfo);
//    if (m_MsgPtr)
//    {
//		ReceiverInfo->PushInt(ReceiverID);
//		SenderInfo->PushInt(SenderID);
//    }
//}
//
//
//CMsg::CMsg(ePipeline& Receiver,int64 MsgID,int64 EventID){
//	m_bReaded = false;
//	ePipeline* ReceiverInfo = NULL;
//	ePipeline* Letter = NULL;
//	ePipeline* SenderInfo = NULL;
//	m_MsgPtr = CreateMsg(MsgID,0,&ReceiverInfo,&Letter,&SenderInfo);
//    if (m_MsgPtr)
//    {
//		*ReceiverInfo=Receiver;
//		ReceiverInfo->SetID(EventID);
//		SenderInfo->PushInt(0);
//	}
//		
//}

CMsg::CMsg(ePipeline* Msg):m_MsgPtr(Msg),m_bReaded(false){
	
};


CMsg::~CMsg(){
	Reset();
};

bool CMsg::IsValid(){ 
	return m_MsgPtr != NULL;
};
bool CMsg::IsReaded(){
	return m_bReaded;
}
void CMsg::Reset(ePipeline* Msg /*= 0*/)
{
	if(m_MsgPtr != NULL){
		delete m_MsgPtr;
	}
	m_MsgPtr = Msg;
	m_bReaded = false;

};


ePipeline* CMsg::Release(){
	ePipeline* temp = m_MsgPtr;
	m_MsgPtr = NULL;
	return temp;
};


ePipeline& CMsg::GetMsg(){
	assert(m_MsgPtr);
	return *m_MsgPtr;
};

ePipeline& CMsg::GetReceiver(){
	assert(m_MsgPtr);
	return *static_cast<ePipeline*>(m_MsgPtr->GetData(0));
};
ePipeline& CMsg::GetLetter(){
	assert(m_MsgPtr);
	m_bReaded = true;
	return *static_cast<ePipeline*>(m_MsgPtr->GetData(1));
	
};
ePipeline& CMsg::GetSender(){
	assert(m_MsgPtr);
	return *static_cast<ePipeline*>(m_MsgPtr->GetData(2));
};

void CMsg::SetSourceID(int64 ID){
	assert(m_MsgPtr);
	m_MsgPtr->SetID(ID);
}

int64 CMsg::GetSourceID(){
	assert(m_MsgPtr);
	return m_MsgPtr->GetID();
};

int64 CMsg::GetMsgID(){
	assert(m_MsgPtr);
    ePipeline* Letter = static_cast<ePipeline*>(m_MsgPtr->GetData(1));
	return Letter->GetID();
}

int64 CMsg::GetReceiverID(){
	assert(m_MsgPtr);
    ePipeline* Pipe = static_cast<ePipeline*>(m_MsgPtr->GetData(0));
    int64 ID = *(int64*)Pipe->GetData(0);
	return ID;
}

int64 CMsg::GetSenderID(){
	assert(m_MsgPtr);
    ePipeline* Pipe = static_cast<ePipeline*>(m_MsgPtr->GetData(2));
    int64 ID = *(int64*)Pipe->GetData(0);
	return ID;

}

int64 CMsg::GetEventID(){
	assert(m_MsgPtr);
    ePipeline* Pipe = static_cast<ePipeline*>(m_MsgPtr->GetData(0));
    return Pipe->GetID();	
}

int64 CMsg::GetSendTimeStamp(){
	assert(m_MsgPtr);
    ePipeline* Pipe = static_cast<ePipeline*>(m_MsgPtr->GetData(2));
    return Pipe->GetID();	
}


}//namespace
