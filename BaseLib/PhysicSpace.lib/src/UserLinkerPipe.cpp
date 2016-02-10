// UserLinkerPipe.cpp: implementation of the CUserLinkerPipe class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "UserLinkerPipe.h"
#include "UserMutex.h"
#include "SpaceMutex.h"
#include "Msg.h"
#include "Model.h"
#include "System.h"
#include "ABSpacePool.h"


namespace PHYSIC{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserLinkerPipe::CUserLinkerPipe(Model* Parent,int64 SourceID,tstring Name)
:CLinkerPipe(NULL,Parent,SERVER_LINKER,SourceID,Name){
	m_RecoType = LINKER_FRIEND;
	m_Mutex = new CUserMutex;
	m_bDeleteMutex = TRUE;

#ifdef _DEBUG
	SetStateOutputLevel(LIGHT_LEVEL);
#endif
}


CUserLinkerPipe::CUserLinkerPipe(CSpaceMutex* Mutex,System* Parent,int64 SourceID)
:CLinkerPipe(Mutex,Parent,CLIENT_LINKER,SourceID,_T("Child"))
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


bool   CUserLinkerPipe::IsValid(){
	bool ret = false;
#if defined(USING_POCO)
	SocketImpl* imp = m_Socket.impl();
	ret = imp->initialized();
#elif defined(USING_WIN32)
	ret = m_Socket != NULL && m_Socket != INVALID_SOCKET;
#endif
	return ret;
}

void   CUserLinkerPipe::Close(){

#if defined(USING_POCO)
	m_Socket.close();

#elif defined(USING_WIN32)
	if(IsValid()){
		long s = m_Socket; //Maybe the thread is looping and  checking m_Socket
		m_Socket =INVALID_SOCKET;
		closesocket(s);
	}
#endif
	CLinkerPipe::Close();
}




bool  CUserLinkerPipe::PhysicalRev(char* Buf,uint32 BufSize, uint32& RevLen, uint32 flag){

	RevLen = 0;	

#if defined(USING_POCO)
	Poco::Timespan timeout(50);
	bool ret = m_Socket.poll(timeout,Socket::SELECT_READ);
	if (ret)
	{
		RevLen = m_Socket.receiveBytes(Buf,BufSize,flag);
		if (RevLen==0)
		{
			ePipeline ErrorInfo;
			m_Parent->NotifyLinkerState(this,LINKER_IO_ERROR,ErrorInfo);
			return false;
		}
		Buf[RevLen] = '\0';
	}
#elif defined(USING_WIN32)
	fd_set  ReadFDs;

	FD_ZERO(&ReadFDs);

	FD_SET(m_Socket, &ReadFDs);

	timeval TimeOut;  
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

				ePipeline ErrorInfo;
				m_Parent->NotifyLinkerState(this,LINKER_IO_ERROR,ErrorInfo);			
				return FALSE;
			}

			if(nRead == 0)   //has connected
			{ 
				int nError = WSAGetLastError();
				m_RecoType = LINKER_INVALID;

				ePipeline ErrorInfo;
				m_Parent->NotifyLinkerState(this,LINKER_IO_ERROR,ErrorInfo);		
				return FALSE;
			}

			Buf[nRead] = '\0';
			RevLen = nRead;
		}	
	}
	else if (ret == SOCKET_ERROR ) { //exit
		int nError = WSAGetLastError();

		m_RecoType = LINKER_INVALID;

		ePipeline ErrorInfo;
		m_Parent->NotifyLinkerState(this,LINKER_IO_ERROR,ErrorInfo);

		return FALSE;
	}
#endif
	return TRUE;
};

bool  CUserLinkerPipe::PhysicalSend(char* Buf,uint32 BufSize, uint32& SendLen, uint32 flag){
	SendLen = 0;
#if defined(USING_POCO)
	Poco::Timespan timeout(50);

	bool ret = m_Socket.poll(timeout,Socket::SELECT_WRITE);
	if(ret){
		SendLen = m_Socket.sendBytes(Buf,BufSize,flag);
	}

#elif defined(USING_WIN32)
	fd_set  WriteFDs; 	
	bool bError = FALSE;

	FD_ZERO(&WriteFDs);		

	FD_SET(m_Socket, &WriteFDs);		

	timeval TimeOut;  
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

				ePipeline ErrorInfo;
				m_Parent->NotifyLinkerState(this,LINKER_IO_ERROR,ErrorInfo);			
				return FALSE;
			}
			SendLen = nBytes;
			Buf[SendLen]= '\0';
		}
	}
	else if (ret == SOCKET_ERROR ) { //exit
		int nError = WSAGetLastError();

		m_RecoType = LINKER_INVALID;

		ePipeline ErrorInfo;
		m_Parent->NotifyLinkerState(this,LINKER_IO_ERROR,ErrorInfo);

		return FALSE;
	}
#endif

	return TRUE;

};


CUserConnectLinkerPipe::CUserConnectLinkerPipe(PHYSIC::Model* Parent,int64 SourceID,AnsiString Address,int32 Port,int32 TimeOut)
:CUserLinkerPipe(Parent,SourceID,_T("Connect Pipe")),m_Address(Address),m_Port(Port),m_TimeOut(TimeOut),m_bConnected(FALSE){
	assert(m_TimeOut>0 && m_TimeOut<60);

}

CUserConnectLinkerPipe::~CUserConnectLinkerPipe(){

};

bool CUserConnectLinkerPipe::Init(tstring& error){

	if(m_Address.size() == 0){
		
		error = _T("Connect Fail: Invalid ServerName.");
		
		return FALSE;
	}
#if defined(USING_WIN32)
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


	unsigned long ul = 1; 
	int Ret = ioctlsocket(m_Socket,FIONBIO,(unsigned long*)&ul);

	if(Ret == SOCKET_ERROR){
		int ErrorCode = WSAGetLastError();
		error = Format1024(_T("Connect Fail: Internal socket error[code:%d]."),ErrorCode);
		return FALSE;
	}

	int ret = connect(m_Socket,(struct sockaddr*)&Server,sizeof(Server));
	if(ret==SOCKET_ERROR)
	{	//normal
		int ErrorCode = WSAGetLastError();
		if (ErrorCode != WSAEWOULDBLOCK){
			error = Format1024( _T("Connect Fail: Internal socket error[code:%d]."),ErrorCode);
			return FALSE;
		}

	}
#endif
	return TRUE;
}

bool CUserConnectLinkerPipe::BlockConnect(tstring& error)
{
#if defined(USING_POCO)
	try{
		const Poco::Net::SocketAddress Address(m_Address,m_Port);
		const Poco::Timespan  TimeOut(5,0);

		m_Socket.connect(Address,TimeOut);

		m_Parent->GetSuperiorLinkerList()->AddLinker(this);
		m_bConnected = TRUE;

	}catch(Poco::TimeoutException& TimeOutError){
		error = UTF8toWS(TimeOutError.displayText());
		return false;
	}catch(Poco::Net::NetException& NetError)
	{
		error = UTF8toWS(NetError.displayText());
		return false;

	}
#elif defined(USING_WIN32)
	fd_set  WriteFDs; 

	for (;;)
	{

		FD_ZERO(&WriteFDs);		
		FD_SET(m_Socket, &WriteFDs);		

		timeval TimeOut;  
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
		else if (ret == SOCKET_ERROR) { //exit
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
#endif
	return true;
}
bool CUserConnectLinkerPipe::Connect(tstring& error){

#if defined(USING_POCO)
	const Poco::Net::SocketAddress Address(m_Address,m_Port);
	const Poco::Timespan  TimeOut(m_TimeOut,0);

	try{
		m_Socket.connect(Address,TimeOut);
	}catch(Poco::TimeoutException& TimeOutError){
		error = UTF8toWS(TimeOutError.displayText());

		return false;;
	}catch(Poco::Net::NetException& NetError)
	{
		error = UTF8toWS(NetError.displayText());
		return false;

	}
		
	m_bConnected = TRUE;

#elif defined(USING_WIN32)
	fd_set  WriteFDs; 

	FD_ZERO(&WriteFDs);		
	FD_SET(m_Socket, &WriteFDs);		

	timeval TimeOut;  
	TimeOut.tv_sec = 1; 
	TimeOut.tv_usec = 0;

	int ret = select(0,NULL, &WriteFDs, NULL, &TimeOut);
	if(ret>0)
	{
		if(FD_ISSET(m_Socket,&WriteFDs)){

			m_Parent->GetSuperiorLinkerList()->AddLinker(this);
			m_bConnected = TRUE;
		}
	}
	else if (ret == SOCKET_ERROR) { //exit
		error = _T("Connect Fail: Internal socket error.");
		return false;
	}
	else if(ret == 0){
		m_TimeOut--;
		if (m_TimeOut == 0)
		{
			error = _T("Connect Fail: Time Out");
			return false;
		}
	}
#endif
	return true;
}

bool  CUserConnectLinkerPipe::ThreadIOWorkProc(char* Buffer,uint32 BufSize){
	CLock lk(m_Mutex,this);

	if (!m_bConnected)
	{
		tstring Error;
		bool ret = Connect(Error);
		if (!ret)
		{
			ePipeline ErrorInfo;
			ErrorInfo.PushString(Error);
			m_Parent->NotifyLinkerState(this,LINKER_CONNECT_ERROR,ErrorInfo);
		}
	}else{
		CUserLinkerPipe::ThreadIOWorkProc(Buffer,BufSize);
	}

	return TRUE;
}

}//end namespace PHYSIC
