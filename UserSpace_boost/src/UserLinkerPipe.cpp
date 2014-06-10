// UserLinkerPipe.cpp: implementation of the CUserLinkerPipe class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include "UserLinkerPipe.h"
#include "UserMutex.h"
#include "UserSpaceMutex.h"
//#include "MsgList.h"
#include "UserModel.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserLinkerPipe::CUserLinkerPipe(Model* Parent,int64 SourceID,tstring Name)
:CLinkerPipe(NULL,Parent,FALSE,SourceID,Name)
{
	m_RevBuf.resize(1024);
	m_Mutex = new CUserMutex;
	m_bDeleteMutex = TRUE;

#ifdef _DEBUG
	SetStateOutputLevel(LIGHT_LEVEL);
#endif
}

CUserLinkerPipe::CUserLinkerPipe(CSpaceMutex* Mutex,Model* Parent,int64 SourceID)
:CLinkerPipe(Mutex,Parent,TRUE,SourceID,_T("Child"))
{
	m_RevBuf.resize(1024);
	assert(Mutex);
	m_bDeleteMutex = FALSE;

#ifdef _DEBUG
	SetStateOutputLevel(LIGHT_LEVEL);
#endif
}

CUserLinkerPipe::~CUserLinkerPipe()
{
    if (m_Mutex && m_bDeleteMutex)
    {
		delete m_Mutex;
		m_Mutex = NULL;
    }
}

void  CUserLinkerPipe::AttachSocket(boost::shared_ptr<ip::tcp::socket>& Socket){
	m_Socket = Socket;
	assert(m_Socket);
	assert(m_Socket->is_open());
	try
	{
		m_Socket->async_read_some(boost::asio::buffer((void*)m_RevBuf.c_str(),m_RevBuf.size()),boost::bind(&CUserLinkerPipe::RevHandler,this,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	catch (std::exception& e)
	{
	    AnsiString s= e.what();
	}
}
boost::shared_ptr<ip::tcp::socket> CUserLinkerPipe::GetSocket(){
	return m_Socket;
};

bool   CUserLinkerPipe::IsValid(){
	return (m_Socket && m_Socket->is_open());
}

void   CUserLinkerPipe::Close(){
	if(IsValid()){
		m_Socket->close();
		CLinkerPipe::Close();
	}
}

void CUserLinkerPipe::FeedbackDirectly(ePipeline* Msg){
	m_UrgenceMsg.Push_Directly(Msg);
	//投递一个空的异步发送
	char buf[1];
	m_Socket->async_write_some(boost::asio::buffer((void*)buf,0),boost::bind(&CUserLinkerPipe::SendHandler,this,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

int64 CUserLinkerPipe::PushMsgToSend(CMsg& Msg,bool bUrgence){	
	int64 ID = CLinkerPipe::PushMsgToSend(Msg,bUrgence);

	//投递一个空的异步发送
	char* buf[1];
	m_Socket->async_write_some(boost::asio::buffer((void*)buf,0),boost::bind(&CUserLinkerPipe::SendHandler,this,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	return ID;
};
 
void CUserLinkerPipe::RevHandler(const boost::system::error_code& error,std::size_t bytes_transferred ){
	if (error)
	{
		m_RecoType = LINKER_INVALID;
		
		CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
		
		Msg.GetLetter().PushInt(m_SourceID);
		Msg.GetLetter().PushString(error.message());	
		m_Parent->PushCentralNerveMsg(Msg);				
		return ;
	}

	if (bytes_transferred)
	{
		_CInnerIOLock lk(m_Mutex,this);	
		CompileMsg((char*)m_RevBuf.c_str(), bytes_transferred); 
		
	}

	m_Socket->async_read_some(boost::asio::buffer((void*)m_RevBuf.c_str(),m_RevBuf.size()),boost::bind(&CUserLinkerPipe::RevHandler,this,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}


void CUserLinkerPipe::SendHandler(const boost::system::error_code& error, std::size_t bytes_transferred ){
	if (error)
	{
		m_RecoType = LINKER_INVALID;
		
		CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
		
		Msg.GetLetter().PushInt(m_SourceID);
		Msg.GetLetter().PushString(error.message());	
		m_Parent->PushCentralNerveMsg(Msg);				
		return ;
	}

	_CInnerIOLock lk(m_Mutex,this);

	if(m_SendState != WAIT_MSG){ 
		
		int32 SendBytes=bytes_transferred;
		if (SendBytes)
		{
			m_SendPos  += SendBytes;
			
			ePipeline Data;
			Data.PushInt(m_PendingMsgID);
			Data.PushInt(m_SendBuffer.size());
			Data.PushInt(m_SendPos);
			m_Parent->NotifyLinkerState(this,LINKER_SEND_STEP,Data);		   
		}
		
		if(m_SendBuffer.size() == m_SendPos){
			
			ePipeline Info;
			Info.Push_Directly(m_CurSendMsg.Release());
			m_Parent->NotifyLinkerState(this,LINKER_MSG_SENDED,Info);
			
			
			m_SendState = WAIT_MSG;	 				
			m_SendPos = 0;
			m_SendBuffer = "";
			m_CurSendMsg.Reset();		
		}	
		else{
			int32 n = m_SendBuffer.size() - m_SendPos; //还剩多少数据没有发送	
			assert(n>0);
			void* buf = (void*)(m_SendBuffer.c_str()+m_SendPos);					
			m_Socket->async_write_some(boost::asio::buffer(buf,n),boost::bind(&CUserLinkerPipe::SendHandler,this,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));			
		}
	}
	
	if(m_SendState == WAIT_MSG ){
		
		//准备发送一个新的信息
		ePipeline* Msg = NULL;	
		
		if (m_UrgenceMsg.Size())
		{
			Msg = (ePipeline*)m_UrgenceMsg.GetData(0);
		} 
		else if(Size())
		{
			Msg = (ePipeline*)GetData(0);
		}else{
			return;
		}
		
		int64 MsgID = Msg->GetID();
		if(MsgID>100){ //不是反馈信息
			ePipeline* Letter = GET_LETTER(Msg);
			MsgID = Letter->GetID();
		}
		
		if(MsgID<100){  //内部控制信息在任何时候都可以发送
			eElectron E;
			if (m_UrgenceMsg.Size())
			{
				m_UrgenceMsg.Pop(&E);
			} 
			else
			{
				ePipeline::Pop(&E);
			}
			E.Release();
			m_CurSendMsg.Reset(Msg);
			
			if(MsgID == LINKER_RESET)
			{   //要求向对方发接收重置信息
				while(m_SendBuffer.size()<ERROR_RESUME_LENGTH)m_SendBuffer += '@';
				m_SendState = SEND_RESET; 
				m_SendPos = 0;
				
			}else{
				assert(MsgID == LINKER_FEEDBACK);
				
				m_SendState = SEND_FEEDBACK;
				Msg->ToString(m_SendBuffer);
				m_SendPos = 0;	
				
			}			
			m_Socket->async_write_some(boost::asio::buffer((void*)m_SendBuffer.c_str(),m_SendBuffer.size()),boost::bind(&CUserLinkerPipe::SendHandler,this,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));			
		}
		else if ( m_PendingMsgID == NULL) //只有未决消息被回复后才能继续发送新消息
		{
			eElectron E;
			if (m_UrgenceMsg.Size())
			{
				m_UrgenceMsg.Pop(&E);
			} 
			else
			{
				ePipeline::Pop(&E);
			}
			E.Release();
			
			m_CurSendMsg.Reset(Msg);
			
			m_SendState = SEND_MSG; 
			Msg->ToString(m_SendBuffer);
			m_PendingMsgID = m_CurSendMsg.GetMsgID();
			m_PendMsgSenderID = m_CurSendMsg.GetSenderID();
			
			
			ePipeline Data;
			Data.PushInt(MsgID);
			Data.PushInt(m_SendBuffer.size());
			Data.PushInt(0);
			m_Parent->NotifyLinkerState(this,LINKER_SEND_STEP,Data);		   

			m_Socket->async_write_some(boost::asio::buffer((void*)m_SendBuffer.c_str(),m_SendBuffer.size()),boost::bind(&CUserLinkerPipe::SendHandler,this,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));			
		}else{
			//投递一个空的异步发送,等待未决信息结束
			char* buf[1];
			m_Socket->async_write_some(boost::asio::buffer((void*)buf,0),boost::bind(&CUserLinkerPipe::SendHandler,this,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	}
	
}

