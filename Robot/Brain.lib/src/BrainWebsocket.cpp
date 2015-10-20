#include "BrainWebsocket.h"
#include "Brain.h"

void WebSocketRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

	try
	{
	
		WebSocket ws(request, response);

		CWebsocketLinkerPipe* Linker = m_Brain->CreateWebSocketLinkerPipe(ws);

		m_Brain->OutSysInfo(_T("One WebSocket connection established."));

		if(Linker== NULL){
			   
				AnsiString Add = request.clientAddress().toString();
				tstring  AddW = UTF8toWS(Add);

				tstring s = Format1024(_T("Accept %s Fail: can't create new WebsocketLinkerPipe "),AddW.c_str());

				CMsg Msg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_LINKER_ERROR,DEFAULT_DIALOG);
				Msg.GetLetter().PushInt(0);
				Msg.GetLetter().PushString(s);		
				m_Brain->PushCentralNerveMsg(Msg,false,false);
				ws.close();
				return;
		}


		System::CLockedLinkerList* ClientList= m_Brain->GetClientLinkerList();
		ClientList->AddLinker(Linker);

		CMsg Msg(MSG_WHO_ARE_YOU,DEFAULT_DIALOG,0);
		Linker->PushMsgToSend(Msg);
	}
	catch (WebSocketException& exc)
	{
		AnsiString Info = exc.what();
		tstring wInfo = UTF8toWS(Info);
		m_Brain->OutSysInfo(wInfo.c_str());

		switch (exc.code())
		{
		case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
			response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
			// fallthrough
		case WebSocket::WS_ERR_NO_HANDSHAKE:
		case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
		case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
			response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
			response.setContentLength(0);
			response.send();
			break;
		}
	}
}


HTTPRequestHandler* BrainRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	AnsiString info = "Request from " 
		+ request.clientAddress().toString()
		+ ": "
		+ request.getMethod()
		+ " "
		+ request.getURI()
		+ " "
		+ request.getVersion();
	tstring wInfo = UTF8toWS(info);
	//m_Brain->OutSysInfo(wInfo);

	for (HTTPServerRequest::ConstIterator it = request.begin(); it != request.end(); ++it)
	{
		info = it->first + ": " + it->second;
		wInfo = UTF8toWS(info);
		//m_Brain->OutSysInfo(wInfo);
	}
	if(request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
		return new WebSocketRequestHandler(m_Brain);
	else
		return NULL;
}


CWebsocketLinkerPipe::CWebsocketLinkerPipe(CSpaceMutex* Mutex,System* Parent,int64 SourceID,const Socket& socket)
	:CLinkerPipe(Mutex,Parent,TRUE,SourceID,_T("Child")),m_Socket(socket)
{
	assert(Mutex);
	m_bDeleteMutex = FALSE;

#ifdef _DEBUG
	SetStateOutputLevel(LIGHT_LEVEL);
#endif
}

CWebsocketLinkerPipe::~CWebsocketLinkerPipe(){
	if (m_Mutex && m_bDeleteMutex)
	{
		delete m_Mutex;
		m_Mutex = NULL;
	}
}



bool   CWebsocketLinkerPipe::IsValid(){
	bool ret = false;
#if defined(USING_POCO)
	SocketImpl* imp = m_Socket.impl();
	ret = imp->initialized();
#else
   #error "Current impletement only support poco c++ lib"
#endif
	return ret;
}

void   CWebsocketLinkerPipe::Close(){

#if defined(USING_POCO)
	m_Socket.close();

#else
	#error "Current impletement only support poco c++ lib"
#endif
	CLinkerPipe::Close();
}

bool  CWebsocketLinkerPipe::PhysicalRev(char* Buf,uint32 BufSize, uint32& RevLen, uint32 flag){

	RevLen = 0;	

#if defined(USING_POCO)
	Poco::Timespan timeout(50);
	bool ret = m_Socket.poll(timeout,Socket::SELECT_READ);
	if (ret)
	{
		int32 flag=0;
		RevLen = m_Socket.receiveFrame(Buf,BufSize,flag);
		if((flag & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_CLOSE){
			Poco::Net::NetException e("websocket closed by remote");
			throw e;
		}

		Buf[RevLen] = '\0';
	}
#else
#error "Current implemention only support poco c++ lib";
#endif
	return TRUE;
};

bool  CWebsocketLinkerPipe::PhysicalSend(char* Buf,uint32 BufSize, uint32& SendLen, uint32 flag){
	SendLen = 0;
#if defined(USING_POCO)
	Poco::Timespan timeout(50);

	bool ret = m_Socket.poll(timeout,Socket::SELECT_WRITE);
	if(ret){
		SendLen = m_Socket.sendFrame(Buf,BufSize,WebSocket::FRAME_TEXT);
	}

#else
	#error "Current implemention only support poco c++ lib";
#endif

	return TRUE;

};