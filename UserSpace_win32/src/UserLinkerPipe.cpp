// UserLinkerPipe.cpp: implementation of the CUserLinkerPipe class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include <WINSOCK2.H>
#include "UserLinkerPipe.h"
#include "UserMutex.h"
#include "UserSpaceMutex.h"
#include "Msg.h"
#include "format.h"
#include "Model.h"
#include "UserSystem.h"
#include "AbstractSpacePool.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserLinkerPipe::CUserLinkerPipe(Model* Parent,int64 SourceID,tstring Name)
:CLinkerPipe(NULL,Parent,FALSE,SourceID,Name),m_Socket(INVALID_SOCKET)
{
	m_RecoType = LINKER_FRIEND;
	m_Mutex = new CUserMutex;
	m_bDeleteMutex = TRUE;

#ifdef _DEBUG
	SetStateOutputLevel(LIGHT_LEVEL);
#endif
}


CUserLinkerPipe::CUserLinkerPipe(CSpaceMutex* Mutex,CUserSystem* Parent,int64 SourceID)
:CLinkerPipe(Mutex,Parent,TRUE,SourceID,_T("Child")),m_Socket(INVALID_SOCKET)
{
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
void CUserLinkerPipe::AttachSocket(SOCKET sc){
	m_Socket = sc;
}

SOCKET  CUserLinkerPipe::GetSocket(){
	return m_Socket;
}

bool   CUserLinkerPipe::IsValid(){
	return m_Socket != NULL && m_Socket != INVALID_SOCKET;;
}

void   CUserLinkerPipe::Close(){
	if(IsValid()){
		long s = m_Socket; //可能线程正在循环检查m_Socket
        m_Socket =INVALID_SOCKET;
		closesocket(s);
		CLinkerPipe::Close();
	}
}

bool  CUserLinkerPipe::ThreadIOWorkProc(char* Buffer,uint32 BufSize){
	_CInnerIOLock lk(m_Mutex,this);
	ThreadInputProc(Buffer,BufSize);
	ThreadOutputProc(Buffer,BufSize);
	return TRUE;
}

uint32 CUserLinkerPipe::ThreadInputProc(char* Buffer,uint32 BufSize){
	
	uint32 RevBytes=0;
	bool RET = PhysicalRev(Buffer, BufSize, RevBytes,0);
	if (!RET){
		return 1;
	}
	
	if(RevBytes > 0)
	{				
		CompileMsg(Buffer, RevBytes); 
	}
	return RevBytes;
}

//从m_OutputPort中取出信息，编译成字符串形式准备输出
uint32 CUserLinkerPipe::ThreadOutputProc(char* Buffer,uint32 BufSize){
	
	uint32 SendBytes =0;
	
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
			return 1;
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
				if(MsgID != LINKER_FEEDBACK){
					return 0;
				}
				m_SendState = SEND_FEEDBACK;
				Msg->ToString(m_SendBuffer);
				m_SendPos = 0;	
				
			}	
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
			
		}
	}
	else{
		int32 n = m_SendBuffer.size() - m_SendPos; //还剩多少数据没有发送	
		if(n>0){
			--BufSize;
			uint32 SendSize = (n > BufSize)?BufSize:n;
			memcpy(Buffer,m_SendBuffer.c_str()+m_SendPos,SendSize);
			
			bool RET = PhysicalSend(Buffer, SendSize, SendBytes,0);
			if (!RET){
				return 1;
			}
						
			m_SendPos += SendBytes; //因为是非阻塞，实际送出的可能跟预计送出的不一样 	

			ePipeline Data;
			Data.PushInt(m_PendingMsgID);
			Data.PushInt(m_SendBuffer.size());
			Data.PushInt(m_SendPos);
			m_Parent->NotifyLinkerState(this,LINKER_SEND_STEP,Data);		   

			if(m_SendBuffer.size() == m_SendPos){
				
				ePipeline Info;
				Info.Push_Directly(m_CurSendMsg.Release());
				m_Parent->NotifyLinkerState(this,LINKER_MSG_SENDED,Info);

				m_SendState = WAIT_MSG;	 				
				m_SendPos = 0;
				m_SendBuffer = "";
				m_CurSendMsg.Reset();	
			}
		}
		
	}
	return SendBytes;
}

bool  CUserLinkerPipe::PhysicalRev(char* Buf,uint32 BufSize, uint32& RevLen, uint32 flag){
	RevLen = 0;

	fd_set  ReadFDs;
	
	FD_ZERO(&ReadFDs);
	
	FD_SET(m_Socket, &ReadFDs);
	
	timeval TimeOut;  //设置1秒钟的间隔
	TimeOut.tv_sec = 0; 
	TimeOut.tv_usec = 50;
	

	int ret = select(0, &ReadFDs, NULL, NULL, &TimeOut);
	if(ret>0)
	{
		if(FD_ISSET(m_Socket,&ReadFDs)){
			int nRead = recv(m_Socket,Buf, BufSize-1, 0);
			if (nRead == SOCKET_ERROR ){
				int nError = WSAGetLastError();
				if (nError== WSAEWOULDBLOCK)return 0;

				m_RecoType = LINKER_INVALID;

				CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
				
				Msg.GetLetter().PushInt(m_SourceID);
				Msg.GetLetter().PushInt(nError);	
				m_Parent->PushCentralNerveMsg(Msg);				
				return FALSE;
			}
			
			if(nRead == 0)   //连接已经关闭
			{ 
				int nError = WSAGetLastError();
				m_RecoType = LINKER_INVALID;
				
				CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
				
				Msg.GetLetter().PushInt(m_SourceID);
				Msg.GetLetter().PushInt(nError);	
				m_Parent->PushCentralNerveMsg(Msg);				
				return FALSE;
			}
			
			Buf[nRead] = '\0';
			RevLen = nRead;
		}	
	}
	else if (ret == SOCKET_ERROR ) { //退出
		int nError = WSAGetLastError();
		
		m_RecoType = LINKER_INVALID;

		CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
		
		Msg.GetLetter().PushInt(m_SourceID);
		Msg.GetLetter().PushInt(nError);		
		m_Parent->PushCentralNerveMsg(Msg);
		
		return FALSE;
	}
	return TRUE;
};

bool  CUserLinkerPipe::PhysicalSend(char* Buf,uint32 BufSize, uint32& SendLen, uint32 flag){
	SendLen = 0;
	
	fd_set  WriteFDs; 	
	bool bError = FALSE;
	
	FD_ZERO(&WriteFDs);		
	
    FD_SET(m_Socket, &WriteFDs);		
	
	timeval TimeOut;  //设置1秒钟的间隔，是否妥当？以后再考虑
	TimeOut.tv_sec = 0; 
	TimeOut.tv_usec = 50;
	
	int ret = select(0, NULL, &WriteFDs, NULL, &TimeOut);
	if(ret>0)
	{
		if(FD_ISSET(m_Socket,&WriteFDs)){
			
			int nBytes = send(m_Socket, Buf, BufSize, 0);
			
			if (nBytes == SOCKET_ERROR) {
				int nError = WSAGetLastError();
				if (nError == WSAEWOULDBLOCK)return 0;	
				
				m_RecoType = LINKER_INVALID;

				CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
				
				Msg.GetLetter().PushInt(m_SourceID);
				Msg.GetLetter().PushInt(nError);		
				m_Parent->PushCentralNerveMsg(Msg);				
				return FALSE;
			}
			SendLen = nBytes;
            Buf[SendLen]= '\0';
		}
	}
	else if (ret == SOCKET_ERROR ) { //退出
		int nError = WSAGetLastError();

		m_RecoType = LINKER_INVALID;

		CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
		
		Msg.GetLetter().PushInt(m_SourceID);
		Msg.GetLetter().PushInt(nError);		
		m_Parent->PushCentralNerveMsg(Msg);

		return FALSE;
	}
	return TRUE;
};


CUserConnectLinkerPipe::CUserConnectLinkerPipe(Model* Parent,int64 SourceID,AnsiString Address,int32 Port,int32 TimeOut)
:CUserLinkerPipe(Parent,SourceID,_T("Connect")),m_Address(Address),m_Port(Port),m_TimeOut(TimeOut),m_bConnected(FALSE){
	assert(m_TimeOut>0 && m_TimeOut<60);

}

CUserConnectLinkerPipe::~CUserConnectLinkerPipe(){

};

bool CUserConnectLinkerPipe::Init(tstring& error){

	if(m_Address.size() == 0){
		
		error = _T("Connect Fail: Invalid ServerName.");
		
		return FALSE;
	}
	
	m_Socket =WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);
	
	if(m_Socket==INVALID_SOCKET){
		error = _T("Connect Fail: can't create socket.");
        return FALSE;
	}
				
	struct hostent *pHost;
	if(inet_addr(m_Address.c_str())==INADDR_NONE)
	{
		pHost=gethostbyname(m_Address.c_str());
	}
	else
	{
		unsigned int addr=inet_addr(m_Address.c_str());
		pHost=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
	}

	if(pHost==NULL)
	{
		error = _T("Connect Fail: Invalid ServerName.");
		return FALSE;
	}
	
	struct sockaddr_in Server;
	Server.sin_addr.s_addr=*((unsigned long*)pHost->h_addr);
	Server.sin_family=AF_INET;
	Server.sin_port=htons(m_Port);
	
	//把m_SocketSelf改为非阻塞,以便能检测连接超时
	unsigned long ul = 1; //设置为非阻塞
	int Ret = ioctlsocket(m_Socket,FIONBIO,(unsigned long*)&ul);
    
	if(Ret == SOCKET_ERROR){
		int ErrorCode = WSAGetLastError();
		error = tformat(_T("Connect Fail: Internal socket error[code:%d]."),ErrorCode);
		return FALSE;
	}
	
	int ret = connect(m_Socket,(struct sockaddr*)&Server,sizeof(Server));
	if(ret==SOCKET_ERROR)
	{	//正常
		int ErrorCode = WSAGetLastError();
		if (ErrorCode != WSAEWOULDBLOCK){
			error = tformat( _T("Connect Fail: Internal socket error[code:%d]."),ErrorCode);
			return FALSE;
		}
		
	}
	return TRUE;
}

bool CUserConnectLinkerPipe::BlockConnect(tstring& error)
{
	fd_set  WriteFDs; 

	for (;;)
	{
		
		FD_ZERO(&WriteFDs);		
		FD_SET(m_Socket, &WriteFDs);		
		
		timeval TimeOut;  //设置1秒钟的间隔，
		TimeOut.tv_sec = 1; 
		TimeOut.tv_usec = 0;
		
		int ret = select(0,NULL, &WriteFDs, NULL, &TimeOut);
		if(ret>0)
		{
			if(FD_ISSET(m_Socket,&WriteFDs)){
				
				m_Parent->GetSuperiorLinkerList()->AddLinker(this);
				m_bConnected = TRUE;
				return TRUE;
			}
		}
		else if (ret == SOCKET_ERROR) { //退出
			error = _T("Connect Fail: Internal socket error.");	
			return FALSE;
		}
		else if(ret == 0){
			m_TimeOut--;
			if (m_TimeOut == 0)
			{
				error = _T("Connect Fail: Time Out");
				return FALSE;
			}
		}
	}
}
void CUserConnectLinkerPipe::Connect(){

	fd_set  WriteFDs; 

	FD_ZERO(&WriteFDs);		
	FD_SET(m_Socket, &WriteFDs);		
	
	timeval TimeOut;  //设置1秒钟的间隔，
	TimeOut.tv_sec = 1; 
	TimeOut.tv_usec = 0;
	
	int ret = select(0,NULL, &WriteFDs, NULL, &TimeOut);
	if(ret>0)
	{
		if(FD_ISSET(m_Socket,&WriteFDs)){

			m_Parent->GetSuperiorLinkerList()->AddLinker(this);
			m_bConnected = TRUE;
			return ;
		}
	}
	else if (ret == SOCKET_ERROR) { //退出
		tstring s = _T("Connect Fail: Internal socket error.");
		CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
		Msg.GetLetter().PushInt(m_SourceID);
		Msg.GetLetter().PushString(s);		
		m_Parent->PushCentralNerveMsg(Msg);
		return;
	}
	else if(ret == 0){
		m_TimeOut--;
		if (m_TimeOut == 0)
		{
			tstring s = _T("Connect Fail: Time Out");
			CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
			Msg.GetLetter().PushInt(m_SourceID);
			Msg.GetLetter().PushString(s);		
			m_Parent->PushCentralNerveMsg(Msg);
		}
	}
}

bool  CUserConnectLinkerPipe::ThreadIOWorkProc(char* Buffer,uint32 BufSize){
	_CInnerIOLock lk(m_Mutex,this);

	if (!m_bConnected)
	{
		Connect();
	}else{
		CUserLinkerPipe::ThreadIOWorkProc(Buffer,BufSize);
	}

	return TRUE;
}

//CUserAcceptLinkerPipe
//////////////////////////////////////////////////////////////////////////
CUserAcceptLinkerPipe::CUserAcceptLinkerPipe(CUserSystem* Parent,int64 SourceID,int32 Port)
:CUserLinkerPipe(Parent,SourceID,_T("Acceptor")),m_Port(Port)
{

	
};

CUserAcceptLinkerPipe::~CUserAcceptLinkerPipe(){

};

bool CUserAcceptLinkerPipe::Init(tstring& error){
	sockaddr_in Local;
	
	Local.sin_family=AF_INET; //Address family
	Local.sin_addr.s_addr=INADDR_ANY; //Wild card IP address
	Local.sin_port=htons((u_short)m_Port); //port to use
	
	m_Socket =socket(AF_INET,SOCK_STREAM,0);
	
	
	if(m_Socket==INVALID_SOCKET){
		int ErrorCode = WSAGetLastError();
        error = tformat(_T("Listen Fail: Internal socket error[code:%d]."),ErrorCode);

		return FALSE;
	}
	
	int Ret = bind(m_Socket,(sockaddr*)&Local,sizeof(Local));
	if(Ret !=0 ){
		error = _T("Listen Fail: Socket or port has been binded");
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	}
	
	if(listen(m_Socket,10)!=0){	
		error = _T("Listen Fail: listen() fail");
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	}
	
	unsigned long ul = 1; //ul非零则设置为非阻塞
	Ret = ioctlsocket(m_Socket,FIONBIO,(unsigned long*)&ul);
	assert(Ret != SOCKET_ERROR);
	
    return TRUE;
}

void CUserAcceptLinkerPipe::Accept(){
		
	SOCKET Client;
	sockaddr_in ClientAddress;
	int len=sizeof(ClientAddress);	

	Client=accept(m_Socket,(struct sockaddr*)&ClientAddress,&len);
	if(Client == INVALID_SOCKET){
		if(WSAEWOULDBLOCK == WSAGetLastError()){
			return;				
		}
		else{
			int ErrorCode = WSAGetLastError();
			tstring s = tformat(_T("Listen Fail: Accept() failed [code:%d]"),ErrorCode);
			CMsg Msg(m_SourceID,DEFAULT_DIALOG,MSG_LINKER_ERROR,DEFAULT_DIALOG);
			Msg.GetLetter().PushInt(m_SourceID);
			Msg.GetLetter().PushString(s);		
			m_Parent->PushCentralNerveMsg(Msg);
			return;
		}
	}
		
	unsigned long ul = 1; //ul非零则设置为非阻塞
	int Ret = ioctlsocket(Client,FIONBIO,(unsigned long*)&ul);
	assert(Ret != SOCKET_ERROR);
	
	//AnsiString ip = (const char*)inet_ntoa(ClientAddress.sin_addr);

	CUserSystem* System = (CUserSystem*)m_Parent;

	CUserLinkerPipe* Linker = System->CreateClientLinkerPipe();
	
	if(Linker== NULL){
		closesocket(Client);
		return ;
	}

	Linker->AttachSocket(Client);
	
	System::CLockedLinkerList* ClientList= System->GetClientLinkerList();
	ClientList->AddLinker(Linker);
	
	/*先不忙产生显式对话，而是要求对方发送认证信息，认证后才生成对应的对话（这样可以把物理连接的处理限制在UserSystem）
		  由于已经加入了连接表，此时连接处于陌生人状态，循环读取时如果超过一定时间没有改变状态则删除
		  tstring s = tformat(_T("The new linker(%s) coming in"),Linker->GetLabel().c_str());
		  OutSysInfo(s);
		  
			int64 ID = Linker->GetSourceID();
			CSysDialog* ChildTask = new CSysDialog(this,ID,DIALOG_OTHER_MAIN);
			
			  m_BrainData.AddDialog(ChildTask);
	*/
	
	CMsg Msg(MSG_WHO_ARE_YOU,DEFAULT_DIALOG,0);
	Linker->PushMsgToSend(Msg);
	
};

bool  CUserAcceptLinkerPipe::ThreadIOWorkProc(char* Buffer,uint32 BufSize){
	_CInnerIOLock lk(m_Mutex,this);
	Accept();
	return TRUE;
}
